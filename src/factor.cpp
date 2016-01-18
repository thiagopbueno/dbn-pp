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

    Factor &Factor::operator=(Factor &&f) {
        if (this != &f) {
            _domain = move(f._domain);
            _values = f._values;
            _partition = f._partition;
            f._values.clear();
            f._partition = 0.0;
        }
        return *this;
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

    bool Factor::in_scope(const Variable *variable) const {
        return (_domain->in_scope(variable));
    }

    Factor Factor::sum_out(const Variable *variable) const {
        if (!in_scope(variable)) {
            Factor new_factor(*this);
            return new_factor;
        }
        else {
            vector<const Variable*> scope = _domain->scope();
            scope.erase(scope.begin() + _domain->index(variable->id()));

            Domain *new_domain = new Domain(scope);
            Factor new_factor(new_domain, 0.0);

            unsigned factor_size = new_factor.size();
            unsigned variable_size = variable->size();

            double partition = 0;

            vector<unsigned> inst(new_factor.width(), 0);
            for (unsigned i = 0; i < factor_size; ++i) {
                for (unsigned val = 0; val < variable_size; ++val) {
                    // unsigned pos = new_domain->position_consistent_instantiation(inst, *_domain, variable, val);
                    unsigned pos = _domain->position_consistent_instantiation(inst, *new_domain, variable, val);
                    double value = (*this)[pos];
                    new_factor[i] += value;
                    partition += value;
                }
                new_domain->next_instantiation(inst);
            }
            new_factor.partition() = partition;

            return new_factor;
        }
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
