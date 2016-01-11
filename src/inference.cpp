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

#include "inference.h"
#include "operations.h"

#include <forward_list>
#include <iostream>

using namespace std;

namespace dbn {

	unique_ptr<Factor> variable_elimination(
		vector<const Variable*> &variables,
		vector<shared_ptr<Factor>> &factors) {

	    forward_list<shared_ptr<Factor>> flist(factors.begin(), factors.end());
	    forward_list<shared_ptr<Factor>> bucket;

		for (auto var: variables) {

			// select all factors with var in its scope
			bucket.clear();
			unsigned b = 0; // bucket size

			forward_list<shared_ptr<Factor>>::const_iterator pf;

			forward_list<shared_ptr<Factor>> new_flist;
			for (pf = flist.begin(); pf != flist.end(); ++pf) {

				if ((*pf)->domain().in_scope(var)) {
					bucket.push_front(*pf);
					b++;
				}
				else {
					new_flist.push_front(*pf);
				}
			}

			flist = new_flist;

			// multiply all factors in bucket and eliminate variable
			if (b > 0) {
				unique_ptr<Factor> prod(new Factor(1.0));
				forward_list<shared_ptr<Factor>>::const_iterator pf = bucket.begin();

				while (b > 1) {
					prod = unique_ptr<Factor>(product(*prod, **pf));
					pf++;
					b--;
				}

				unique_ptr<Factor> p(sum_product(*prod, **pf, var));

				new_flist.push_front(move(p));
	  		}

	  		flist = new_flist;
		}

		// generate result by multiplying all remaining factors in the pool
		unique_ptr<Factor> prod(new Factor(1.0));
		forward_list<shared_ptr<Factor>>::const_iterator pf;
		for (pf = flist.begin(); pf != flist.end(); ++pf) {
			prod = unique_ptr<Factor>(product(*prod, **pf));
		}

		return prod;
	}

	unique_ptr<Factor> project(
		vector<shared_ptr<Factor>> &factors,
		const unordered_map<unsigned,const Variable*> &transition,
		const shared_ptr<Factor> &forward) {

		static vector<const Variable*> ordering;
		static vector<shared_ptr<Factor>> sum_prod_factors;

		if (ordering.size() == 0 && sum_prod_factors.size() == 0) {
			for (auto it_transition : transition) {
				unsigned id_prime = it_transition.first;
				const Variable *variable = it_transition.second;
				ordering.push_back(variable);
				sum_prod_factors.push_back(factors[id_prime]);
			}
		}

		// variable elimination
		sum_prod_factors.push_back(forward);
		unique_ptr<Factor> projection = variable_elimination(ordering, sum_prod_factors);
		projection->change_variables(transition);
		sum_prod_factors.pop_back();

		return move(projection);
	}

	unique_ptr<Factor> update(
		const Factor &projection,
		const Factor &sensor_model,
		const unordered_map<unsigned,unsigned> &evidence) {

		// add observation from time t
		unique_ptr<Factor> evidence_t = unique_ptr<Factor>(conditioning(sensor_model, evidence));

		// update projection with observation
		unique_ptr<Factor> belief_state = unique_ptr<Factor>(product(*evidence_t, projection));
		belief_state = unique_ptr<Factor>(normalization(*belief_state));

		return move(belief_state);
	}

	vector<shared_ptr<Factor>> filtering(
		vector<shared_ptr<Factor>> &factors,
		vector<unsigned> &prior,
		unordered_map<unsigned,const Variable*> &transition,
		vector<unsigned> &sensor,
		vector<unordered_map<unsigned,unsigned>> &observations) {

		// estimates
		vector<shared_ptr<Factor>> estimates;

		// prior model
		unique_ptr<Factor> prior_model = unique_ptr<Factor>(new Factor(1.0));
		for (auto id : prior) {
			prior_model = unique_ptr<Factor>(product(*prior_model, *factors[id]));
		}

		// sensor model
		unique_ptr<Factor> sensor_model = unique_ptr<Factor>(new Factor(1.0));
		for (auto id : sensor) {
			sensor_model = unique_ptr<Factor>(product(*sensor_model, *factors[id]));
		}

		// initialize forward message
		shared_ptr<Factor> forward = make_shared<Factor>(1.0);
		forward = unique_ptr<Factor>(product(*forward, *prior_model));

		for (auto evidence : observations) {

			// project belief state
			unique_ptr<Factor> projection = project(factors, transition, forward);

			// update belief state
			forward = update(*projection, *sensor_model, evidence);

			// add new estimate to filtering list
			estimates.push_back(forward);
		}

		return estimates;
	}


	vector<shared_ptr<Factor>> unrolled_filtering(
		vector<unique_ptr<Variable>> &variables,
		vector<shared_ptr<Factor>> &factors,
		vector<unsigned> &prior,
		unordered_map<unsigned,const Variable*> &transition,
		vector<unsigned> &sensor,
		vector<unordered_map<unsigned,unsigned>> &observations) {

		vector<shared_ptr<Factor>> estimates;

		vector<const Variable*> ordering;

		unordered_map<unsigned,const Variable *> renaming;
		for (auto it_transition : transition) {
			const Variable *next_var = variables[it_transition.first].get();
			unsigned curr_id = it_transition.second->id();
			renaming[curr_id] = next_var;
		}

		vector<shared_ptr<Factor>> unrolled_factors;

		for (auto prior_id : prior) {
			unrolled_factors.push_back(factors[prior_id]);
			// ordering.push_back(variables[prior_id].get());
		}

		for (auto it_transition : transition) {
			unsigned id_next = it_transition.first;
			unrolled_factors.push_back(factors[id_next]);

			unsigned id_curr = it_transition.second->id();
			ordering.push_back(variables[id_curr].get());
		}

		for (auto sensor_id : sensor) {
			factors[sensor_id]->change_variables(renaming);

			unique_ptr<Factor> new_factor = unique_ptr<Factor>(conditioning(*factors[sensor_id], observations[0]));
			new_factor = unique_ptr<Factor>(normalization(*new_factor));

			unrolled_factors.push_back(move(new_factor));
		}


		// cout << "Renaming" << endl;
		// for (auto it : renaming) {
		// 	cout << it.first << ":" << it.second->id() << endl;
		// }
		// cout << "Unrolled factors:" << endl;
		// for (auto const& f : unrolled_factors) {
		// 	cout << *f << endl;
		// }

		Factor *estimate = normalization(*variable_elimination(ordering, unrolled_factors));
		unordered_map<unsigned,const Variable *> renaming_back;
		const Domain &estimate_domain = estimate->domain();
		for (unsigned i = 0; i < estimate_domain.width(); ++i) {
			unsigned var_id = estimate_domain[i]->id();
			for (auto it_renaming : renaming) {
				unsigned id_from = it_renaming.first;
				unsigned id_to = it_renaming.second->id();
				if (id_to == var_id) {
					renaming_back[var_id] = variables[id_from].get();
				}
			}
		}
		estimate->change_variables(renaming_back);
		estimates.emplace_back(estimate);

		// cout << "Estimates[0]" << endl;
		// cout << *estimates[0] << endl << endl;

		unsigned id = factors.size();
		for (unsigned t = 1; t < observations.size(); ++t) {

			for (auto it_transition : transition) {
				unsigned id_next = it_transition.first;
				unsigned id_curr = it_transition.second->id();

				unique_ptr<Variable> new_var = unique_ptr<Variable>(new Variable(id, variables[id_next]->size()));
				renaming[id_next] = new_var.get();
				variables.push_back(move(new_var));
				id++;

				Factor *new_factor = new Factor(*factors[id_next]);
				new_factor->change_variables(renaming);

				unrolled_factors.emplace_back(new_factor);
				ordering.push_back(renaming[id_curr]);
				renaming[id_curr] = renaming[id_next];
			}

			// for (auto it_renaming : renaming) {
			// 	unsigned i = it_renaming.first;
			// 	if (i < factors.size()) {
			// 		unsigned next_id = renaming[i]->id();
			// 		if (renaming.find(next_id) != renaming.end()) {
			// 			renaming[i] = renaming[next_id];
			// 		}
			// 	}
			// }

			for (auto sensor_id : sensor) {
				unique_ptr<Variable> new_var = unique_ptr<Variable>(new Variable(id, variables[sensor_id]->size()));
				renaming[sensor_id] = new_var.get();
				variables.push_back(move(new_var));
				id++;

				unique_ptr<Factor> new_factor = unique_ptr<Factor>(conditioning(*factors[sensor_id], observations[t]));
				new_factor = unique_ptr<Factor>(normalization(*new_factor));
				new_factor->change_variables(renaming);

				unrolled_factors.push_back(move(new_factor));
			}

			// cout << "Renaming" << endl;
			// for (auto it : renaming) {
			// 	cout << it.first << ":" << it.second->id() << endl;
			// }

			// cout << "Unrolled factors:" << endl;
			// for (auto const& f : unrolled_factors) {
			// 	cout << *f << endl;
			// }

			// cout << "Ordering" << endl;
			// for (auto pv : ordering) {
			// 	cout << pv->id() << endl;
			// }

			Factor *estimate = normalization(*variable_elimination(ordering, unrolled_factors));
			unordered_map<unsigned,const Variable *> renaming_back;
			const Domain &estimate_domain = estimate->domain();
			for (unsigned i = 0; i < estimate_domain.width(); ++i) {
				unsigned var_id = estimate_domain[i]->id();
				for (auto it_renaming : renaming) {
					unsigned id_from = it_renaming.first;
					unsigned id_to = it_renaming.second->id();
					if (id_to == var_id) {
						renaming_back[var_id] = variables[id_from].get();
					}
				}
			}
			estimate->change_variables(renaming_back);
			estimates.emplace_back(estimate);

			// cout << "Estimates[" << t << "]" << endl;
			// cout << *estimates[t] << endl << endl;
		}

		return estimates;
	}

}
