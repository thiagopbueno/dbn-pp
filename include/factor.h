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

namespace dbn {

    class Factor {
    public:
        Factor(Domain *domain) : _domain(domain), _values(new double[domain->size()]), _width(domain->width()), _size(domain->size()) { };
        virtual ~Factor();

        unsigned size() const { return _size; };
        unsigned width() const { return _width; };

        double operator[](unsigned i) const;
        void set(unsigned i, double value);

        friend std::ostream &operator<<(std::ostream &o, const Factor &f);

    private:
        Domain *_domain;
        double *_values;
        unsigned _width;
        unsigned _size;
    };

}

#endif