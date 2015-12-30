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

#include "model.h"

namespace dbn {

	Model::~Model() {
		for (unsigned id = 0; id < _order; ++id) {
			delete _vars[id];
			delete _factors[id];
		}
		delete[] _vars;
		delete[] _factors;
	}

	std::ostream& operator<<(std::ostream &o, const Model &m) {
        o << "Model(order:" << m._order << ")" << std::endl;
        for (unsigned id = 0; id < m._order; ++id) {
        	o << *(m._vars[id]) << " : " << *(m._factors[id]) << std::endl;
        }

        return o;
    }

}
