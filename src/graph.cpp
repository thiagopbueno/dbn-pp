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

#include "graph.h"

using namespace std;

namespace dbn {

	Graph::Graph(const vector<shared_ptr<Factor>> &factors)
	{
		for (auto &pf : factors) {
			const Domain &domain = pf->domain();
			unsigned width = domain.width();
			if (width == 0) continue;
			for (unsigned i = 0; i < width-1; ++i) {
				for (unsigned j = i+1; j < width; ++j) {
					const Variable *v1 = domain[i];
					const Variable *v2 = domain[j];
					_adj[v1].insert(v2);
					_adj[v2].insert(v1);
				}
			}
		}
	}

	vector<const Variable*>
	Graph::ordering(const vector<const Variable*> &variables)
	{
		vector<const Variable*> vars(variables);
		vector<const Variable*> ordering;
		unordered_set<const Variable*> processed;
		while (vars.size() > 0) {
			unsigned min_index = min_fill(vars, processed);
			const Variable *next_var = vars[min_index];
			ordering.push_back(next_var);
			processed.insert(next_var);
			vars.erase(vars.begin()+min_index);
		}
		return ordering;
	}

	unsigned
	Graph::min_fill(std::vector<const Variable*> &variables, unordered_set<const Variable*> &processed)
	{
		unsigned min_index = 0;
		unsigned min_fill = _adj.size();

		unsigned nvars = variables.size();
		for (unsigned i = 0; i < nvars; ++i) {
			const Variable *var = variables[i];
			unordered_set<const Variable*> neighboors = _adj[var];

			unsigned fill_in = 0;
			for (auto const v1 : neighboors) {
				if (processed.count(v1)) continue;
				for (auto const v2 : neighboors) {
					if (processed.count(v2) || v1->id() >= v2->id()) continue;
					if (!_adj[v1].count(v2)) {
						fill_in++;
					}
				}
			}

			if (fill_in < min_fill) {
				min_index = i;
				min_fill = fill_in;
			}
		}

		return min_index;
	}

	std::ostream &
	operator<<(std::ostream &os, const Graph &g)
	{
		os << "Graph:" << endl;
		for (auto it : g._adj) {
			const Variable *v = it.first;
			unordered_set<const Variable*> neighboors = it.second;
			os << v->id() << " :";
			for (auto pv : neighboors) {
				cout << " " << pv->id();
			}
			os << endl;
		}
		os << endl;
		return os;
	}

}
