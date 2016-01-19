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
        _partition(0) { }

    Factor::Factor(Domain *domain, double value) :
        _domain(std::unique_ptr<Domain>(domain)),
        _values(std::vector<double>(domain->size(), value)),
        _partition(domain->size() * value) { }

    Factor::Factor(double value) :
        _domain(std::unique_ptr<Domain>(new Domain)),
        _values(std::vector<double>(1, value)),
        _partition(value) { }

    Factor::Factor(const Factor &f) :
        _domain(unique_ptr<Domain>(new Domain(f.domain()))),
        _values(f._values),
        _partition(f._partition) { }

    Factor::Factor(Factor &&f) {
        _domain = move(f._domain);
        _values = f._values;
        _partition = f._partition;
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

    Factor Factor::operator*(const Factor &f) {
        return product(f);
    }

    void Factor::operator*=(const Factor &f) {
        *this = product(f);
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

    Factor Factor::change_variables(std::unordered_map<unsigned,const Variable*> renaming) {
        Factor new_factor(*this);
        new_factor._domain->modify_scope(renaming);
        return new_factor;
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
            new_factor._partition = partition;

            return new_factor;
        }
    }

    Factor Factor::product(const Factor &f) const {
        const Domain &d1 = this->domain();
        const Domain &d2 = f.domain();

        Domain *new_domain = new Domain(d1, d2);
        unsigned width = new_domain->width();
        unsigned size = new_domain->size();
        Factor new_factor(new_domain, 0.0);

        vector<unsigned> inst(width, 0);
        double partition = 0;
        for (unsigned i = 0; i < size; ++i) {
            // find position in linearization of consistent instantiation
            unsigned pos1 = d1.position_consistent_instantiation(inst, *new_domain);
            unsigned pos2 = d2.position_consistent_instantiation(inst, *new_domain);

            // set product factor value
            double value = (*this)[pos1] * f[pos2];
            new_factor[i] = value;
            partition += value;

            // find next instantiation
            new_domain->next_instantiation(inst);
        }
        new_factor._partition = partition;
        return new_factor;
    }

    Factor Factor::normalize() const {
        Factor new_factor(*this);

        unsigned sz = new_factor.size();
        for (unsigned i = 0; i < sz; ++i) {
            new_factor._values[i] = new_factor._values[i]/new_factor._partition;
        }
        new_factor._partition = 1.0;

        return new_factor;
    }

    Factor Factor::conditioning(const std::unordered_map<unsigned,unsigned> &evidence) const {
        const Domain &d = domain();
        unsigned width = d.width();

        Factor new_factor(new Domain(d, evidence));

        // incorporate evidence in instantiation
        vector<unsigned> instantiation(width, 0);
        d.update_instantiation_with_evidence(instantiation, evidence);

        double partition = 0;
        unsigned new_factor_size = new_factor.size();
        for (unsigned i = 0; i < new_factor_size; ++i) {

            // update new factor
            unsigned pos = d.position_instantiation(instantiation);
            double value = (*this)[pos];
            new_factor[i] = value;
            partition += value;

            // find next instantiation
            d.next_instantiation(instantiation, evidence);
        }
        new_factor._partition = partition;

        return new_factor;
    }

    ostream &operator<<(ostream &os, const Factor &f) {

        const Domain &domain = f.domain();
        int width = domain.width();
        int size = domain.size();

        os << "Factor(";
        // os << "output = " << f._output << ", ";
        os << "width = " << width << ", ";
        os << "size = " << size << ", ";
        os << "partition = " << f.partition() << ")" << endl;

        // scope
        for (auto pf : domain.scope()) {
            os << pf->id() << " ";
        }
        os << endl;

        // values
        vector<unsigned> inst(width, 0);
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < width; ++j) {
                os << inst[j] << " ";
            }
            os << ": " << f[inst] << endl;
            domain.next_instantiation(inst);
        }

        return os;
    }

}
