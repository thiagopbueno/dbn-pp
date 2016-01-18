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

#include <iostream>

using namespace std;

namespace dbn {

    Domain::Domain() : _width(0), _size(1) {}

    Domain::Domain(vector<const Variable*> scope) : _scope(scope), _width(scope.size()) {
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

    Domain::Domain(const Domain &domain) : Domain(domain._scope) {}

    Domain::Domain(const Domain &domain, const unordered_map<unsigned,unsigned> &evidence) {
        for (auto it_scope : domain._scope) {
            const Variable *variable = it_scope;
            if (!evidence.count(variable->id())) {
                _scope.push_back(variable);
            }
        }
        _width = _scope.size();
        _size = 1;
        _offset.reserve(_width);
        for (int i = _width-1; i >= 0; --i) {
            _offset[i] = _size;
            _size *= _scope[i]->size();
            _var_to_index[_scope[i]->id()] = i;
        }
    }



    const Variable *Domain::operator[](unsigned i) const {
        if (i < _width) return _scope[i];
        else throw "Domain::operator[unsigned i]: Index out of range!";
    }

    unsigned Domain::operator[](const Variable* v) const {
        unordered_map<unsigned,unsigned>::const_iterator it_index = _var_to_index.find(v->id());
        if (it_index != _var_to_index.end()) return it_index->second;
        else throw "Domain::operator[const Variable*]: Invalid argument!";
    }

    int Domain::index(const unsigned id) {
        return (in_scope(id) ?  _var_to_index[id] : -1);
        // *_var_to_index.find(id)
    }

    bool Domain::in_scope(const Variable* v) const {
        unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find(v->id());
        return (it != _var_to_index.end());
    }

    bool Domain::in_scope(unsigned id) const {
        unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find(id);
        return (it != _var_to_index.end());
    }

    void Domain::modify_scope(std::unordered_map<unsigned,const Variable*> modifier) {
        for (auto it_modifier : modifier) {
            unsigned id = it_modifier.first;
            const Variable *variable = it_modifier.second;
            if (in_scope(id)) {
                unsigned index = _var_to_index[id];
                _scope[index] = variable;
                _var_to_index.erase(id);
                _var_to_index[variable->id()] = index;
            }
        }
    }

    unsigned Domain::position_instantiation(vector<unsigned> instantiation) const {
        unsigned pos = 0;
        for (int i = _width-1; i >= 0; --i) {
            pos += instantiation[i] * _offset[i];
        }
        return pos;
    }

    unsigned Domain::position_consistent_instantiation(vector<unsigned> instantiation, const Domain &domain) const {
        if (_width == 0) { return 0; }
        else {
            unsigned pos = 0;
            unsigned index = 0;
            for (auto v : _scope) {
                unordered_map<unsigned,unsigned>::const_iterator it_index = _var_to_index.find(v->id());
                unordered_map<unsigned,unsigned>::const_iterator it_index2 = domain._var_to_index.find(v->id());
                if (it_index != _var_to_index.end() && it_index2 != _var_to_index.end()) {
                    pos +=  _offset[it_index->second] * instantiation[it_index2->second];
                }
                index++;
            }
            return pos;
        }
    }

    unsigned Domain::position_consistent_instantiation(vector<unsigned> instantiation, const Domain &domain, const Variable *v, unsigned value) const {
        unsigned pos = position_consistent_instantiation(instantiation, domain);
        unordered_map<unsigned,unsigned>::const_iterator it_index = _var_to_index.find(v->id());
        if (it_index != _var_to_index.end()) {
            pos += _offset[it_index->second] * value;
        }
        return pos;
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

    void Domain::next_instantiation(vector<unsigned> &instantiation) const {
        int j;
        for (j = instantiation.size()-1; j >= 0 && instantiation[j] == _scope[j]->size()-1; --j) {
            instantiation[j] = 0;
        }
        if (j >= 0) {
            instantiation[j]++;
        }
    }

    void Domain::next_instantiation(vector<unsigned> &instantiation, const unordered_map<unsigned,unsigned> &evidence) const {
        int j;
        for (j = instantiation.size()-1; j >= 0 && (evidence.count(_scope[j]->id()) || instantiation[j] == _scope[j]->size()-1); --j) {
            if (evidence.count(_scope[j]->id())) continue;
            instantiation[j] = 0;
        }
        if (j >= 0) {
            instantiation[j]++;
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