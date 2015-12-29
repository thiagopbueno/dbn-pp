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

#ifndef _DBN_DOMAIN_H
#define _DBN_DOMAIN_H

#include "variable.h"

#include <unordered_map>

namespace dbn {

    class Domain {
    public:
        Domain(Variable **scope, unsigned width);
        virtual ~Domain();

        unsigned width() const { return _width; };
        unsigned size() const { return _size; };

        Variable* operator[](unsigned i) const {
            if (i < _width) return _scope[i];
            else throw "Domain::operator[]: Index out of range!";
        }
        
        unsigned position(unsigned instantiation[]);

        Domain* union_of(const Domain* d1, const Domain* d2);
        Domain* union_of(const Domain* d1, const Domain* d2, const Variable* v);

        friend std::ostream &operator<<(std::ostream &o, const Domain &v); 

    private:
        Variable **_scope;
        unsigned _width;
        unsigned _size;
        std::unordered_map<unsigned, unsigned> _var_to_index;
    };

}

#endif
