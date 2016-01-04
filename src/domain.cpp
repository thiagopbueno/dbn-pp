// Copyright (c) 2015 Thiago Pereira Bueno
// All Rights Reserved.
//
// This file is part of DBN library.
//
// DBN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DBN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DBN.  If not, see <http://www.gnu.org/licenses/>.

#include "domain.h"

using namespace std;

namespace dbn {

    // Domain::Domain() : _width(0), _size(1) { }

    Domain::Domain(vector<const Variable*> scope, unsigned width) : _scope(scope), _width(width) {
        _size = 1;
        if (_width > 0) {
            _offset.reserve(_width);
            for (int i = _width-1; i >= 0; --i) {
                _offset[i] = _size;
                _size *= _scope[i]->size();
                _var_to_index[_scope[i]->id()] = i;
            }
        }
    }

    unsigned Domain::position(vector<unsigned> instantiation) const {
        unsigned pos = 0;
        for (int i = _width-1; i >= 0; --i) {
            pos += instantiation[i] * _offset[i];
        }
        return pos;
    }

    void Domain::consistent_instantiation(const vector<unsigned> &instantiation, const Domain& domain, vector<unsigned> &new_instantiation) const {
        if (domain.width() == 0) {
            new_instantiation.reserve(1);
            new_instantiation[0] = 0;
        }
        else {
            new_instantiation.reserve(domain.width());
            for (unsigned i = 0; i < instantiation.size(); ++i) {
                const Variable *v = domain[i];
                if (domain.in_scope(v)) {
                    new_instantiation[domain[v]] = instantiation[i];
                }
            }
        }
    }

    void Domain::update_instantiation_with_evidence(vector<unsigned> &instantiation, const unordered_map<unsigned,unsigned> &evidence) const {
        if (_width == 0) return;
        for (auto it_evidence : evidence) {
            unsigned id = it_evidence.first;
            unsigned value = it_evidence.second;

            unordered_map<unsigned,unsigned>::const_iterator it_index = _var_to_index.find(id);
            if (it_index != _var_to_index.end()) {
                instantiation[it_index->second] = value;
            }
        }
    }

    void Domain::next_instantiation(vector<unsigned> &instantiation, const unordered_map<unsigned,unsigned> &evidence) const {
        int j;
        for (j = instantiation.size()-1; j >= 0 && (evidence.count(_scope[j]->id()) || instantiation[j] == 1); --j) {
            if (evidence.count(_scope[j]->id())) continue;
            instantiation[j] = 0;
        }
        if (j >= 0) {
            instantiation[j] = 1;
        }
    }

    ostream& operator<<(ostream &o, const Domain &d) {
        unsigned width = d.width();
        if (width == 0) { o << "Domain{}"; }
        else {
            o << "Domain{";
            unsigned i;
            for (i = 0; i < width-1; ++i) {
                o << d[i]->id() << ", ";
            }
            o << d[i]->id() << "}";
        }
        return o;
    }

}