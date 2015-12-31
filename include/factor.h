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
        Factor(Domain *domain) : _domain(std::unique_ptr<Domain>(domain)), _values(std::vector<double>(domain->size())) { };
        Factor(Domain *domain, double value) : _domain(std::unique_ptr<Domain>(domain)), _values(std::vector<double>(domain->size(), value)) { };
        Factor(double value) : _domain(std::unique_ptr<Domain>(new Domain)), _values(std::vector<double>(1, value)) { };

        const Domain &domain() const { return *_domain; };
        unsigned size()  const { return _domain->size();  };
        unsigned width() const { return _domain->width(); };

        const double &operator[](unsigned i) const;
        double &operator[](unsigned i);

        friend std::ostream &operator<<(std::ostream &o, const Factor &f);

    private:
        std::unique_ptr<Domain> _domain;
        std::vector<double> _values;
    };

}

#endif