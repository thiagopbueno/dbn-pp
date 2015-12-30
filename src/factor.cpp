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

namespace dbn {

    double &Factor::operator[](unsigned i) { 
        if (i < size()) return _values[i];
        else throw "Factor::operator[]: Index out of range.";
    }

    std::ostream &operator<<(std::ostream &o, const Factor &f) {
        o << "Factor(" << *(f._domain) << ", size:" << f.size() << ", values:[";
        unsigned i;
        for (i = 0; i < f.size()-1; ++i) { 
            o << f._values[i] << ", ";
        }
        o << f._values[i] << "])";
        return o;
    }

}
