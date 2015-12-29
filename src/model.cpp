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

	Model::Model(unsigned order, unsigned cardinality[]) : _order(order) {
		_vars = (Variable **) malloc (_order * sizeof (Variable *));
		for (unsigned id = 0; id < _order; ++id) {
			_vars[id] = new Variable(id, cardinality[id]);
		}
	}

	Model::~Model() {
		for (unsigned id = 0; id < _order; ++id) {
			delete _vars[id];
		}
		delete[] _vars;
	}

	std::ostream& operator<<(std::ostream &o, const Model &m) {
        o << "Model(order:" << m.order() << ")" << std::endl;

        o << ">> Variables:" << std::endl;
        Variable **vars = m.variables();
        for (unsigned id = 0; id < m.order(); ++id) {
        	o << *(vars[id]) << std::endl;
        }
        return o;
    }

}
