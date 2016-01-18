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
		_output(output),
		_domain(unique_ptr<Domain>(new Domain)) { }
			
	ADDFactor::ADDFactor(const string &output, const Factor &factor) :
		_output(output),
		_domain(new Domain(factor.domain())) {

		_dd = mgr.addZero();

		unsigned width = factor.width();
		vector<unsigned> inst(width, 0);

		for (unsigned l = 0; l < factor.size(); ++l) {
			unsigned pos = _domain->position_instantiation(inst);
			double value = factor[pos];
			ADD line = mgr.constant(value);

			for (unsigned i = 0; i < width; ++i) {
				ADD v = mgr.addVar((*_domain)[i]->id());
				line *= (inst[i] ? v : ~v);
			}

			_dd += line;
			_domain->next_instantiation(inst);
		}
	}

	ADDFactor::ADDFactor(const string &output, const ADD &dd, const Domain &domain) :
		_dd(dd),
		_output(output),
		_domain(unique_ptr<Domain>(new Domain(domain))) { }

	ADDFactor::ADDFactor(const ADDFactor &f) {
		_dd = f._dd;
		_output = f._output;
		_domain = unique_ptr<Domain>(new Domain(*f._domain));
	}

	ADDFactor::ADDFactor(ADDFactor &&f) {
		_dd = f._dd;
		_output = f._output;
		_domain = move(f._domain);
		f._output = "";
	}

	ADDFactor &ADDFactor::operator=(ADDFactor &&f) {
		if (this != &f) {
			_dd = f._dd;
			_output = f._output;
			_domain = move(f._domain);
			f._output = "";
		}
		return *this;
	}

	ADDFactor ADDFactor::operator*(const ADDFactor &f) {
		return product(f);
	}

	void ADDFactor::operator*=(const ADDFactor &f) {
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

	ADDFactor ADDFactor::change_variables(unordered_map<unsigned,const Variable*> renaming) {

		Domain new_domain(*_domain);
		new_domain.modify_scope(renaming);

		vector<ADD> x, y;
		for (auto it_renaming : renaming) {
			unsigned from = it_renaming.first;
			unsigned to = it_renaming.second->id();
			x.push_back(mgr.addVar(from));
			y.push_back(mgr.addVar(to));
		}
		ADD swapped = _dd.SwapVariables(x, y);

		string output = "renamed(" + _output + ")";
		return ADDFactor(output, swapped, new_domain);
	}

	bool ADDFactor::in_scope(const Variable *variable) const {
		return (_domain->in_scope(variable));
	}

	ADDFactor ADDFactor::sum_out(const Variable *variable) const {
		unsigned index = variable->id();
		string output = "sum_out(" + _output + "," + to_string(index) + ")";

		if (!in_scope(variable)) return ADDFactor(output, _dd, *_domain);

		ADD positive = mgr.addVar(index);
		ADD negated  = ~positive;
		ADD summed_out = _dd.Restrict(positive) + _dd.Restrict(negated);

		vector<const Variable*> scope;
		for (auto pv : _domain->scope()) {
			if (pv->id() != index) {
				scope.push_back(pv);
			}
		}
		Domain domain(scope);

		return ADDFactor(output, summed_out, domain);
	}

	ADDFactor ADDFactor::product(const ADDFactor &f) const {
		string output = _output + "*" + f._output;

		vector<const Variable*> scope = _domain->scope();
		for (auto pv : f.domain().scope()) {
			if (!_domain->in_scope(pv)) {
				scope.push_back(pv);
			}
		}
		Domain domain(scope);

		ADD prod = _dd * f._dd;

		return ADDFactor(output, prod, domain);
	}

	ADDFactor ADDFactor::normalize() const {
		DdManager *ddmgr = mgr.getManager();
		DdNode *partitionNode = Cudd_addConst(ddmgr, partition());
		DdNode *ddNode = Cudd_addApply(ddmgr, Cudd_addDivide, _dd.getNode(), partitionNode);
		string output = "norm(" + _output + ")";
		return ADDFactor(output, ADD(mgr, ddNode), *_domain);
	}

	ADDFactor ADDFactor::conditioning(const unordered_map<unsigned,unsigned> &evidence) const {
		string output = "cond(" + _output + ",{";
		ADD evidenceVariables = mgr.constant(1.0);
		for (auto it : evidence) {
			unsigned id = it.first;
			unsigned value = it.second;
			output += " " + to_string(id) + ":" + to_string(value);
			evidenceVariables *= (value ? mgr.addVar(id) : ~mgr.addVar(id));
		}
		output += " })";
		vector<const Variable*> scope;
		for (auto pv : _domain->scope()) {
			if (evidence.find(pv->id()) == evidence.end()) {
				scope.push_back(pv);
			}
		}
		Domain domain(scope);
		ADD conditioned = _dd.Restrict(evidenceVariables);
		return ADDFactor(output, conditioned, domain);
	}

	double ADDFactor::operator[](vector<unsigned> instantiation) const {
		DdNode *node = _dd.getNode();
		if (Cudd_IsConstant(node)) return Cudd_V(node);

		DdManager *mgr = ADDFactor::mgr.getManager();
		int N = Cudd_ReadSize(mgr);

		int *support;
		Cudd_SupportIndices(mgr, _dd.getNode(), &support);

		int *inputs = new int[N];
		for (int id = 0; id < N; ++id) {
			if (_domain->in_scope(id)) {
				inputs[id] = instantiation[_domain->index(id)];
			}
			else {
				inputs[id] = 2;
			}
		}
		delete[] support;

		DdNode *constant = Cudd_Eval(mgr, _dd.getNode(), inputs);
		delete[] inputs;

		CUDD_VALUE_TYPE value = Cudd_V(constant);
		return value;
	}

	int ADDFactor::dump_dot(string filename) const {
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

		const Domain &domain = f.domain();
		int width = domain.width();
		int size = domain.size();

		// os << "ADDFactor: " << f._output << "" << endl;
		os << "Factor(";
		os << "width = " << width << ", ";
		os << "size = " << size << ", ";
		os << "partition = " << f.partition() << ")" << endl;

		// scope
		for (auto pf : domain.scope()) {
			os << pf->id() << " ";
		}
		os << endl;

		// values
		vector<unsigned> inst(width, 0);
		for (int i = 0; i < size; ++i) {
			for (int j = 0; j < width; ++j) {
				os << inst[j] << " ";
			}
			os << ": " << f[inst] << endl;
			domain.next_instantiation(inst);
		}

		// f._dd.print(width,3);

		// DdManager *mgr = ADDFactor::mgr.getManager();

		// int *support;
		// int support_size = Cudd_SupportIndices(mgr, f._dd.getNode(), &support);
		// cout << "support: {";
		// for (int i = 0; i < support_size; ++i) {
		// 	cout << " " << support[i];
		// }
		// cout << " }" << endl;
		// delete[] support;

		// int *cube;
		// CUDD_VALUE_TYPE value;
		// DdGen *gen = Cudd_FirstCube(mgr, f._dd.getNode(), &cube, &value);

		// int N = Cudd_ReadSize(mgr);
		// while (!Cudd_IsGenEmpty(gen)) {
		// 	for (int i = 0; i < N; i++) {
		// 		os << cube[i] << " ";
		// 	}
		// 	os << ": " << value << endl;
		// 	Cudd_NextCube(gen, &cube, &value);
		// }
		// Cudd_GenFree(gen);
		// os << endl;

		return os;
	}
}

