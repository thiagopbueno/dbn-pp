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

#include "domain.h"

namespace dbn {

    Domain::Domain(std::vector<const Variable*> scope, unsigned width) : _scope(scope), _width(width) {
        _offset.reserve(width);
        _size = 1;
        for (int i = _width-1; i >= 0; --i) {
            _offset[i] = _size;
            _size *= _scope[i]->size();
            _var_to_index[_scope[i]->id()] = i;
        }
    }

    unsigned Domain::position(std::vector<unsigned> instantiation) const {
        unsigned pos = 0;
        for (int i = _width-1; i >= 0; --i) {
            pos += instantiation[i] * _offset[i];
        }
        return pos;
    }

    std::ostream& operator<<(std::ostream &o, const Domain &d) {
        unsigned width = d.width();
        if (width == 0) { o << "Domain{}"; }
        else {
            o << "Domain{";
            unsigned i;
            for (i = 0; i < width-1; ++i) {
                o << d[i]->id() << ", ";
            }
            o << d[i]->id() << "}";
        }
        return o;
    }

}