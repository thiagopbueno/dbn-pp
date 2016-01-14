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

	ADDFactor::ADDFactor(Cudd &mgr, const string &output, double value) :
		_mgr(mgr),
		_dd(_mgr.constant(value)),
		_output(output) { }
			
	ADDFactor::ADDFactor(Cudd &mgr, const string &output, const Factor &factor) :
		_mgr(mgr),
		_output(output) {

		for (auto pv : factor.domain().scope()) {
			_scope.insert(pv);
		}

		_dd = _mgr.addZero();

		unsigned width = factor.width();
		vector<unsigned> inst(width, 0);

		const Domain &domain = factor.domain();
		for (unsigned l = 0; l < factor.size(); ++l) {
			unsigned pos = domain.position_instantiation(inst);
			double value = factor[pos];
			ADD line = _mgr.constant(value);

			for (unsigned i = 0; i < width; ++i) {
				ADD v = _mgr.addVar(domain[i]->id());
				line *= (inst[i] ? v : ~v);
			}

			_dd += line;
			domain.next_instantiation(inst);
		}
	}

	ADDFactor::ADDFactor(Cudd &mgr, const string &output, const ADD &dd, set<const Variable*> scope) :
		_mgr(mgr),
		_dd(dd),
		_output(output),
		_scope(scope) { }

	string ADDFactor::output() const {
		return _output;
	}

	bool ADDFactor::in_scope(const Variable *variable) {
		return (_scope.count(variable));
	}

	ADDFactor ADDFactor::sum_out(const Variable *variable) {
		unsigned index = variable->id();
		string output = "sum_out(" + _output + "," + to_string(index) + ")";

		if (!in_scope(variable)) return ADDFactor(_mgr, output, _dd, _scope);

		ADD positive = _mgr.addVar(index);
		ADD negated  = ~positive;
		ADD summed_out = _dd.Restrict(positive) + _dd.Restrict(negated);

		set<const Variable*> scope;
		for (auto pv : _scope) {
			if (pv->id() != index) {
				scope.insert(pv);
			}
		}

		return ADDFactor(_mgr, output, summed_out, scope);
	}

	int ADDFactor::dump_dot(string filename) {
		int result;
		FILE *f = fopen(filename.c_str(), "w");
		if (!f) return -1;
		DdNode *outputs[1];
		outputs[0] = _dd.getNode();
		const char *outputNames[1];
		outputNames[0] = _output.c_str();
		result = Cudd_DumpDot(_mgr.getManager(), 1, outputs, NULL, outputNames, f);
		fclose(f);
		return !result;
	}


	ostream &operator<<(ostream& os, const ADDFactor &f) {
		os << "ADDFactor: (" << f._output << ")" << endl;
		unsigned width = f._scope.size();
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

