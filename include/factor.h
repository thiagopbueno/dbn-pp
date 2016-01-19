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

#ifndef _DBN_FACTOR_H
#define _DBN_FACTOR_H

#include "domain.h"

#include <vector>
#include <memory>

namespace dbn {

    class Factor {
    public:
        Factor(Domain *domain);
        Factor(Domain *domain, double value);
        Factor(double value = 1.0);
        Factor(const Factor &f);
        Factor(Factor &&f);

        Factor &operator=(Factor &&f);
        Factor operator*(const Factor &f);
        void operator*=(const Factor &f);

        const Domain &domain() const { return *_domain; }
        unsigned size()        const { return _domain->size();  }
        unsigned width()       const { return _domain->width(); }
        double partition()     const { return _partition; }

        void partition(double p) { _partition = p; }

        const double &operator[](unsigned i) const;
        double &operator[](unsigned i);

        double operator[](std::vector<unsigned> instantiation) const;

        bool in_scope(const Variable *variable) const;

        Factor sum_out(const Variable *variable) const;
        Factor product(const Factor &f) const;
        Factor normalize() const;
        Factor conditioning(const std::unordered_map<unsigned,unsigned> &evidence) const;

        void change_variables(std::unordered_map<unsigned,const Variable*> renaming);

        friend std::ostream &operator<<(std::ostream &os, const Factor &f);

    private:
        std::unique_ptr<Domain> _domain;
        std::vector<double> _values;
        double _partition;
    };

}

#endif