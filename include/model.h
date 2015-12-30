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

#ifndef _DBN_MODEL_H
#define _DBN_MODEL_H

#include "variable.h"
#include "factor.h"

namespace dbn {
	
    class Model {
    public:
        Model(unsigned order, Variable **vars, Factor **factors) : _order(order), _vars(vars), _factors(factors) { };
        virtual ~Model();

        unsigned order() const { return _order; };

        Variable **variables() const { return _vars; };
        Factor **factors() const { return _factors; };

        friend std::ostream &operator<<(std::ostream &o, const Model &m);

    private:
        unsigned _order;
        Variable **_vars;
        Factor **_factors;
    };

}

#endif