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

#include "factor.h"

#include <iostream>

using namespace std;

namespace dbn {

    Factor::Factor(Domain *domain) :
        _domain(std::unique_ptr<Domain>(domain)),
        _values(std::vector<double>(domain->size())),
        _partition(0) {}

    Factor::Factor(Domain *domain, double value) :
        _domain(std::unique_ptr<Domain>(domain)),
        _values(std::vector<double>(domain->size(), value)),
        _partition(domain->size() * value) {}

    Factor::Factor(double value) :
        _domain(std::unique_ptr<Domain>(new Domain)),
        _values(std::vector<double>(1, value)),
        _partition(value) {}

    Factor::Factor(const Factor &f, bool normalization) :
        _domain(unique_ptr<Domain>(new Domain(f.domain()))),
        _values(vector<double>(f.size())) {
        unsigned sz = f.size();
        if (normalization) {
            for (unsigned i = 0; i < sz; ++i) {
                _values[i] = f._values[i]/f._partition;
            }
            _partition = 1.0;
        }
        else {
            for (unsigned i = 0; i < sz; ++i) {
                _values[i] = f._values[i];
            }
            _partition = f._partition;
        }
    }

    const double &Factor::operator[](unsigned i) const {
        if (i < size()) return _values[i];
        else throw "Factor::operator[]: Index out of range.";
    }

    double &Factor::operator[](unsigned i) { 
        if (i < size()) return _values[i];
        else throw "Factor::operator[]: Index out of range.";
    }

    double Factor::operator[](std::vector<unsigned> inst) const {
        unsigned pos = _domain->position_instantiation(inst);
        return _values[pos];
    }

    void Factor::change_variables(std::unordered_map<unsigned,const Variable*> renaming) {
        _domain->modify_scope(renaming);
    }

    ostream &operator<<(ostream &o, const Factor &f) {
        o << "Factor(" << *(f._domain);
        o << ", size:" << f.size();
        o << ", partition:" << f._partition;
        o << ", values:[";
        o.precision(3);
        o << fixed;
        unsigned i;
        for (i = 0; i < f.size()-1; ++i) { 
            o << f._values[i] << ", ";
        }
        o << f._values[i] << "])";
        return o;
    }

}
