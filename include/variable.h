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

#ifndef _DBN_VARIABLE_H
#define _DBN_VARIABLE_H

#include <ostream>

namespace dbn {

    class Variable {
    public:
        Variable(unsigned id, unsigned size) : _id(id), _size(size) { }

        unsigned id()   const { return _id;   }
        unsigned size() const { return _size; }

        friend std::ostream &operator<<(std::ostream &o, const Variable &v);

    private:
        unsigned _id;
        unsigned _size;
    };

}

#endif
