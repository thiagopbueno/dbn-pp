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

	Cudd ADDFactor::mgr(0,0);

	ADDFactor::ADDFactor(const string &output, double value) :
		_dd(mgr.constant(value)),
		_output(output) { }
			
	ADDFactor::ADDFactor(const string &output, const Factor &factor) :
		_output(output) {

		for (auto pv : factor.domain().scope()) {
			_scope.insert(pv);
		}

		_dd = mgr.addZero();

		unsigned width = factor.width();
		vector<unsigned> inst(width, 0);

		const Domain &domain = factor.domain();
		for (unsigned l = 0; l < factor.size(); ++l) {
			unsigned pos = domain.position_instantiation(inst);
			double value = factor[pos];
			ADD line = mgr.constant(value);

			for (unsigned i = 0; i < width; ++i) {
				ADD v = mgr.addVar(domain[i]->id());
				line *= (inst[i] ? v : ~v);
			}

			_dd += line;
			domain.next_instantiation(inst);
		}
	}

	ADDFactor::ADDFactor(const string &output, const ADD &dd, set<const Variable*> scope) :
		_dd(dd),
		_output(output),
		_scope(scope) { }

	ADDFactor::ADDFactor(ADDFactor &&addf) {
		_dd = addf._dd;
		_output = addf._output;
		_scope = addf._scope;
	}

	ADDFactor &ADDFactor::operator=(ADDFactor &&f) {
		if (this != &f) {
			_scope.clear();

			_dd = f._dd;
			_output = f._output;
			_scope = f._scope;

			f._output = "";
			f._scope.clear();
		}

		return *this;
	}

	ADDFactor ADDFactor::operator*(ADDFactor &f) {
		return product(f);
	}

	void ADDFactor::operator*=(ADDFactor &f) {
		*this = product(f);
	}

	string ADDFactor::output() const {
		return _output;
	}

	double ADDFactor::partition() const {
		double partition = 0;
		int *cube;
		CUDD_VALUE_TYPE value;
		DdGen *gen = Cudd_FirstCube(mgr.getManager(), _dd.getNode(), &cube, &value);
		while (!Cudd_IsGenEmpty(gen)) {
			partition += value;
			Cudd_NextCube(gen, &cube, &value);
		}
		Cudd_GenFree(gen);
		return partition;
	}

	bool ADDFactor::in_scope(const Variable *variable) {
		return (_scope.count(variable));
	}

	ADDFactor ADDFactor::sum_out(const Variable *variable) {
		unsigned index = variable->id();
		string output = "sum_out(" + _output + "," + to_string(index) + ")";

		if (!in_scope(variable)) return ADDFactor(output, _dd, _scope);

		ADD positive = mgr.addVar(index);
		ADD negated  = ~positive;
		ADD summed_out = _dd.Restrict(positive) + _dd.Restrict(negated);

		set<const Variable*> scope;
		for (auto pv : _scope) {
			if (pv->id() != index) {
				scope.insert(pv);
			}
		}

		return ADDFactor(output, summed_out, scope);
	}

	ADDFactor ADDFactor::product(const ADDFactor &f) {
		string output = _output + "*" + f._output;

		set<const Variable*> scope = _scope;
		for (auto pv : f._scope) {
			scope.insert(pv);
		}

		ADD prod = _dd * f._dd;

		return ADDFactor(output, prod, scope);
	}

	ADDFactor ADDFactor::normalize() {
		DdManager *ddmgr = mgr.getManager();
		DdNode *partitionNode = Cudd_addConst(ddmgr, partition());
		DdNode *ddNode = Cudd_addApply(ddmgr, Cudd_addDivide, _dd.getNode(), partitionNode);
		string output = "norm(" + _output + ")";
		return ADDFactor(output, ADD(mgr, ddNode), _scope);
	}

	ADDFactor ADDFactor::conditioning(const std::unordered_map<unsigned,unsigned> &evidence) {
		string output = "cond(" + _output + ",{";
		ADD evidenceVariables = mgr.constant(1.0);
		for (auto it : evidence) {
			unsigned id = it.first;
			unsigned value = it.second;
			output += " " + to_string(id) + ":" + to_string(value);
			evidenceVariables *= (value ? mgr.addVar(id) : ~mgr.addVar(id));
		}
		output += " })";
		set<const Variable*> scope;
		for (auto pv : _scope) {
			if (evidence.find(pv->id()) == evidence.end()) {
				scope.insert(pv);
			}
		}
		ADD conditioned = _dd.Restrict(evidenceVariables);
		return ADDFactor(output, conditioned, scope);
	}

	int ADDFactor::dump_dot(string filename) {
		int result;
		FILE *f = fopen(filename.c_str(), "w");
		if (!f) return -1;
		DdNode *outputs[1];
		outputs[0] = _dd.getNode();
		const char *outputNames[1];
		outputNames[0] = _output.c_str();
		result = Cudd_DumpDot(mgr.getManager(), 1, outputs, NULL, outputNames, f);
		fclose(f);
		return !result;
	}


	ostream &operator<<(ostream& os, const ADDFactor &f) {
		os << "ADDFactor: " << f._output << "" << endl;
		os << "partition = " << f.partition() << endl;
		os << "scope {";
		for (auto pf : f._scope) {
			os << " " << pf->id();
		}
		os << " }" << endl;
		unsigned width = f._scope.size();
		f._dd.print(width,3);
		int *cube;
		CUDD_VALUE_TYPE value;
		DdGen *gen = Cudd_FirstCube(ADDFactor::mgr.getManager(), f._dd.getNode(), &cube, &value);
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

