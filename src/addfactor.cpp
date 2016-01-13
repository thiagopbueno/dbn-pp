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

#include "addfactor.h"
#include "domain.h"
#include "cudd.h"

#include <cstdio>
#include <iostream>

using namespace std;

namespace dbn {

	ADDFactor::ADDFactor(Cudd &mgr, string &output, double value) :
		_mgr(mgr),
		_dd(_mgr.constant(value)),
		_domain(unique_ptr<Domain>(new Domain)),
		_output(output.c_str()) { }
			
	ADDFactor::ADDFactor(Cudd &mgr, string &output, Factor &factor) :
		_mgr(mgr),
		_domain(unique_ptr<Domain>(new Domain(factor.domain()))),
		_output(output.c_str()) {

		_dd = _mgr.addZero();

		unsigned width = factor.width();
		vector<unsigned> inst(width, 0);

		for (unsigned l = 0; l < factor.size(); ++l) {
			unsigned pos = _domain->position_instantiation(inst);
			double value = factor[pos];
			ADD line = _mgr.constant(value);

			for (unsigned i = 0; i < width; ++i) {
				ADD v = _mgr.addVar((*_domain)[i]->id());
				line *= (inst[i] ? v : ~v);
			}

			_dd += line;
			_domain->next_instantiation(inst);
		}
	}

	int ADDFactor::dump_dot(const char *filename) {
		int result;
		FILE *f = fopen(filename, "w");
		if (!f) return -1;
		DdNode *outputs[1];
	    outputs[0] = _dd.getNode();
	    const char *outputNames[1];
	    outputNames[0] = _output;
		result = Cudd_DumpDot(_mgr.getManager(), 1, outputs, NULL, outputNames, f);
	    fclose(f);
	    return !result;
	}


	ostream &operator<<(ostream& os, const ADDFactor &f) {
		os << "ADDFactor: (" << f._output << ")" << endl;
		unsigned width = f._domain->width();
		f._dd.print(width,3);
		int *cube;
		CUDD_VALUE_TYPE value;
		DdGen *gen = Cudd_FirstCube(f._mgr.getManager(), f._dd.getNode(), &cube, &value);
		while (!Cudd_IsGenEmpty(gen)) {
			for (unsigned i = 0; i < width; i++) {
				os << cube[i] << " ";
			}
			os << ": " << value << endl;
			Cudd_NextCube(gen, &cube, &value);
		}
		Cudd_GenFree(gen);
		os << endl;
		return os;
	}
}

