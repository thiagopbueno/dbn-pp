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
#include "graph.h"

#include <forward_list>
#include <set>
#include <iostream>
#include <algorithm>

using namespace std;

namespace dbn {

	Factor variable_elimination(
		vector<const Variable*> &variables,
		vector<shared_ptr<Factor>> &factors) {

		// initialize result
		Factor result(1.0);

		// choose elimination ordering
		forward_list<const Variable*> ordering(variables.begin(), variables.end());
		// Graph g(factors);
		// vector<const Variable*> new_ordering = g.ordering(variables);
		// forward_list<const Variable*> ordering(new_ordering.begin(), new_ordering.end());

		// initialize buckets
		unordered_map<unsigned,set<shared_ptr<Factor>>> buckets;
		for (auto pv : ordering) {
			set<shared_ptr<Factor>> bfactors;
			buckets[pv->id()] = bfactors;
		}
		for (auto pf : factors) {
			bool in_bucket = false;
			for (auto pv : ordering) {
				if (pf->domain().in_scope(pv)) {
					buckets[pv->id()].insert(pf);
					in_bucket = true;
					break;
				}
			}
			if (!in_bucket) {
				result *= *pf;
			}
		}

		// eliminate all variables
		while (!ordering.empty()) {
			const Variable *var = ordering.front();
			ordering.pop_front();

			// eliminate var
			Factor prod(1.0);
			for (auto pf : buckets[var->id()]) {
				prod *= *pf;
			}
			shared_ptr<Factor> new_factor = make_shared<Factor>(prod.sum_out(var));

			// stop if finished
			if (buckets.empty()) {
				result *= *new_factor;
				// cout << *result << endl;
				break;
			}

			// update bucket list with new factor
			bool in_bucket = false;
			for (auto pv : ordering) {
				if (new_factor->domain().in_scope(pv)) {
					buckets[pv->id()].insert(new_factor);
					in_bucket = true;
					break;
				}
			}
			if (!in_bucket) {
				result *= *new_factor;
			}
		}

		return result;
	}

	Factor project(
		vector<shared_ptr<Factor>> &factors,
		const unordered_map<unsigned,const Variable*> &transition,
		const Factor &forward) {

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
		sum_prod_factors.push_back(make_shared<Factor>(forward));
		Factor projection = variable_elimination(ordering, sum_prod_factors);
		projection = projection.change_variables(transition);
		sum_prod_factors.pop_back();

		return projection;
	}

	Factor update(
		const Factor &projection,
		const Factor &sensor_model,
		const unordered_map<unsigned,unsigned> &evidence) {

		// add observation from time t
		Factor evidence_t = sensor_model.conditioning(evidence);

		// update projection with observation
		Factor belief_state = evidence_t * projection;

		// return move(belief_state);
		return belief_state.normalize();
	}

	vector<shared_ptr<Factor>> filtering(
		vector<const Variable*> &variables, vector<shared_ptr<Factor>> &factors,
		set<unsigned> &prior, set<unsigned> &sensor, set<unsigned> &internals,
		unordered_map<unsigned,const Variable*> &transition,
		vector<unordered_map<unsigned,unsigned>> &observations) {

		// estimates
		vector<shared_ptr<Factor>> estimates;

		// prior model
		Factor prior_model(1.0);
		for (auto id : prior) {
			prior_model = prior_model * *(factors[id]);
		}

		// (generalized) sensor model
		vector<shared_ptr<Factor>> sensor_factors;
		for (auto id : sensor) {
			sensor_factors.push_back(factors[id]);
		}
		for (auto id : internals) {
			sensor_factors.push_back(factors[id]);
		}
		vector<const Variable*> internal_variables;
		for (auto id : internals) {
			internal_variables.push_back(variables[id]);
		}
		Factor sensor_model = variable_elimination(internal_variables, sensor_factors);

		// initialize forward message
		Factor forward = prior_model;

		for (auto evidence : observations) {
			// project belief state
			Factor projection = project(factors, transition, forward);

			// update belief state
			forward = update(projection, sensor_model, evidence);

			// add new estimate to filtering list
			estimates.push_back(make_shared<Factor>(forward));
		}

		return estimates;
	}


	ADDFactor variable_elimination(
		vector<const Variable*> &variables,
		vector<shared_ptr<ADDFactor>> &factors) {

		// initialize result
		ADDFactor result;

		// choose elimination ordering
		forward_list<const Variable*> ordering(variables.begin(), variables.end());
		// Graph g(factors);
		// vector<const Variable*> new_ordering = g.ordering(variables);
		// forward_list<const Variable*> ordering(new_ordering.begin(), new_ordering.end());

		// initialize buckets
		unordered_map<unsigned,set<shared_ptr<ADDFactor>>> buckets;
		for (auto pv : ordering) {
			set<shared_ptr<ADDFactor>> bfactors;
			buckets[pv->id()] = bfactors;
		}
		for (auto pf : factors) {
			bool in_bucket = false;
			for (auto pv : ordering) {
				if (pf->domain().in_scope(pv)) {
					buckets[pv->id()].insert(pf);
					in_bucket = true;
					break;
				}
			}
			if (!in_bucket) {
				result *= *pf;
			}
		}

		// eliminate all variables
		while (!ordering.empty()) {
			const Variable *var = ordering.front();
			ordering.pop_front();

			// eliminate var
			ADDFactor prod;
			for (auto pf : buckets[var->id()]) {
				prod *= *pf;
			}
			shared_ptr<ADDFactor> new_factor = make_shared<ADDFactor>(prod.sum_out(var));

			// stop if finished
			if (buckets.empty()) {
				result *= *new_factor;
				// cout << *result << endl;
				break;
			}

			// update bucket list with new factor
			bool in_bucket = false;
			for (auto pv : ordering) {
				if (new_factor->domain().in_scope(pv)) {
					buckets[pv->id()].insert(new_factor);
					in_bucket = true;
					break;
				}
			}
			if (!in_bucket) {
				result *= *new_factor;
			}
		}

		return result;
	}

	ADDFactor project(
		vector<shared_ptr<ADDFactor>> &factors,
		const unordered_map<unsigned,const Variable*> &transition,
		const ADDFactor &forward) {

		static vector<const Variable*> ordering;
		static vector<shared_ptr<ADDFactor>> sum_prod_factors;

		if (ordering.size() == 0 && sum_prod_factors.size() == 0) {
			for (auto it_transition : transition) {
				unsigned id_prime = it_transition.first;
				const Variable *variable = it_transition.second;
				ordering.push_back(variable);
				sum_prod_factors.push_back(factors[id_prime]);
			}
		}

		sum_prod_factors.push_back(make_shared<ADDFactor>(forward));
		ADDFactor projection = variable_elimination(ordering, sum_prod_factors);
		projection = projection.change_variables(transition);
		sum_prod_factors.pop_back();
		return projection;
	}

	ADDFactor update(
		const ADDFactor &projection,
		const ADDFactor &sensor_model,
		const unordered_map<unsigned,unsigned> &evidence) {

		// add observation from time t
		ADDFactor evidence_t = sensor_model.conditioning(evidence);

		// update projection with observation
		ADDFactor belief_state = evidence_t * projection;
		return belief_state.normalize();
	}

	vector<shared_ptr<ADDFactor>>
	filtering(
		vector<const Variable*> &variables, vector<shared_ptr<ADDFactor>> &factors,
		set<unsigned> &prior, set<unsigned> &sensor, set<unsigned> &internals,
		unordered_map<unsigned,const Variable*> &transition,
		vector<unordered_map<unsigned,unsigned>> &observations)
	{

		ADDFactor::set_mgr_reordering();

		// unsigned sz = variables.size();
		// int *permutation = new int[sz];
		// for (unsigned i = 0; i < sz; ++i) {
		// 	permutation[i] = variables[i]->id();
		// }
		// ADDFactor::set_mgr_reordering(permutation);
		// delete[] permutation;

		// estimates
		vector<shared_ptr<ADDFactor>> estimates;

		// prior model
		ADDFactor prior_model;
		for (auto id : prior) {
			prior_model *= *factors[id];
		}

		// (generalized) sensor model
		vector<shared_ptr<ADDFactor>> sensor_factors;
		for (auto id : sensor) {
			sensor_factors.push_back(factors[id]);
		}
		for (auto id : internals) {
			sensor_factors.push_back(factors[id]);
		}
		vector<const Variable*> internal_variables;
		for (auto id : internals) {
			internal_variables.push_back(variables[id]);
		}
		ADDFactor sensor_model = variable_elimination(internal_variables, sensor_factors);

		// initialize forward message
		ADDFactor forward = prior_model;

		for (auto evidence : observations) {
			// project belief state
			ADDFactor projection = project(factors, transition, forward);

			// update belief state
			// forward = update(projection, internals, sensor_model, evidence);
			forward = update(projection, sensor_model, evidence);

			// add new estimate to filtering list
			estimates.push_back(make_shared<ADDFactor>(forward));
		}

		return estimates;
	}


	vector<shared_ptr<Factor>>
	unrolled_filtering(
		vector<const Variable*> variables, vector<shared_ptr<Factor>> &factors,
		set<unsigned> &prior, set<unsigned> &sensor, set<unsigned> &internals,
		unordered_map<unsigned,const Variable*> &transition,
		vector<unordered_map<unsigned,unsigned>> &observations,
		bool verbose)
	{

		vector<shared_ptr<Factor>> estimates;

		const int N = variables.size();

		unordered_map<unsigned,const Variable *> renaming;
		for (auto it_transition : transition) {
			const Variable *next_var = variables[it_transition.first];
			unsigned curr_id = it_transition.second->id();
			renaming[curr_id] = next_var;
		}

		vector<const Variable*> ordering;
		vector<shared_ptr<Factor>> unrolled_factors;
		for (auto prior_id : prior) {
			unrolled_factors.push_back(factors[prior_id]);
		}
		for (auto it_transition : transition) {
			unsigned id_next = it_transition.first;
			unrolled_factors.push_back(factors[id_next]);

			unsigned id_curr = it_transition.second->id();
			ordering.push_back(variables[id_curr]);
		}
		for (auto internal_id : internals) {
			Factor internal_factor(*factors[internal_id]);
			internal_factor = internal_factor.change_variables(renaming);
			Factor new_factor = internal_factor.conditioning(observations[0]).normalize();
			unrolled_factors.push_back(make_shared<Factor>(new_factor));

			ordering.push_back(variables[internal_id]);
		}
		for (auto sensor_id : sensor) {
			Factor sensor_factor(*factors[sensor_id]);
			sensor_factor = sensor_factor.change_variables(renaming);
			Factor new_factor = sensor_factor.conditioning(observations[0]).normalize();
			unrolled_factors.push_back(make_shared<Factor>(new_factor));
		}

		if (verbose) {
			cout << "@ t = 1" << endl;
			cout << "Renaming" << endl;
			for (auto it : renaming) {
				cout << it.first << ":" << it.second->id() << endl;
			}
			cout << "Unrolled factors:" << endl;
			for (auto const& f : unrolled_factors) {
				cout << f->domain() << endl;
			}
			cout << "Ordering" << endl;
			for (auto pv : ordering) {
				cout << pv->id() << " ";
			}
			cout << endl << endl;
		}

		Factor estimate = variable_elimination(ordering, unrolled_factors).normalize();

		unordered_map<unsigned,const Variable *> renaming_back;
		const Domain &estimate_domain = estimate.domain();
		const unsigned estimate_width = estimate_domain.width();
		for (unsigned i = 0; i < estimate_width; ++i) {
			unsigned var_id = estimate_domain[i]->id();
			for (auto it_renaming : renaming) {
				unsigned id_from = it_renaming.first;
				unsigned id_to = it_renaming.second->id();
				if (id_to == var_id) {
					renaming_back[var_id] = variables[id_from];
				}
			}
		}

		if (verbose) {
			cout << "Renaming" << endl;
			for (auto it : renaming) {
				cout << it.first << ":" << it.second->id() << endl;
			}
			cout << "Renaming back" << endl;
			for (auto it : renaming_back) {
				cout << it.first << ":" << it.second->id() << endl;
			}
		}

		estimate = estimate.change_variables(renaming_back);
		estimates.push_back(make_shared<Factor>(estimate));

		unsigned id = factors.size();
		const unsigned T = observations.size();
		for (unsigned t = 1; t < T; ++t) {

			if (verbose) {
				cout << "@ t = " << t+1 << endl;
				cout << "Renaming" << endl;
				for (auto it : renaming) {
					cout << it.first << ":" << it.second->id() << endl;
				}
			}

			for (auto it_transition : transition) {
				unsigned id_next = it_transition.first;
				unsigned id_curr = it_transition.second->id();

				const Variable *new_var = new Variable(id, variables[id_next]->size());
				renaming[id_next] = new_var;
				variables.push_back(new_var);
				id++;

				Factor *new_factor = new Factor(*factors[id_next]);
				*new_factor = new_factor->change_variables(renaming);

				unrolled_factors.emplace_back(new_factor);
				ordering.push_back(renaming[id_curr]);
			}

			for (auto it_transition : transition) {
				unsigned id_next = it_transition.first;
				unsigned id_curr = it_transition.second->id();
				renaming[id_curr] = renaming[id_next];
			}


			for (auto internal_id : internals) {
				const Variable *new_var = new Variable(id, variables[internal_id]->size());
				renaming[internal_id] = new_var;
				variables.push_back(new_var);
				id++;

				ordering.push_back(new_var);
			}
			for (auto sensor_id : sensor) {
				const Variable *new_var = new Variable(id, variables[sensor_id]->size());
				renaming[sensor_id] = new_var;
				variables.push_back(new_var);
				id++;
			}


			for (auto internal_id : internals) {
				Factor *internal_factor = factors[internal_id].get();
				Factor new_factor = internal_factor->conditioning(observations[t]).normalize();
				new_factor = new_factor.change_variables(renaming);
				unrolled_factors.push_back(make_shared<Factor>(new_factor));
			}
			for (auto sensor_id : sensor) {
				Factor *sensor_factor = factors[sensor_id].get();
				Factor new_factor = sensor_factor->conditioning(observations[t]).normalize();
				new_factor = new_factor.change_variables(renaming);
				unrolled_factors.push_back(make_shared<Factor>(new_factor));
			}

			if (verbose) {
				cout << "Unrolled factors:" << endl;
				for (auto const& f : unrolled_factors) {
					cout << f->domain() << endl;
				}
				cout << "Ordering" << endl;
				for (auto pv : ordering) {
					cout << pv->id() << " ";
				}
				cout << endl << endl;
			}

			Factor estimate = variable_elimination(ordering, unrolled_factors).normalize();
			unordered_map<unsigned,const Variable *> renaming_back;
			const Domain &estimate_domain = estimate.domain();
			const unsigned estimate_width = estimate_domain.width();
			for (unsigned i = 0; i < estimate_width; ++i) {
				unsigned var_id = estimate_domain[i]->id();
				for (auto it_renaming : renaming) {
					unsigned id_from = it_renaming.first;
					unsigned id_to = it_renaming.second->id();
					if (id_to == var_id) {
						renaming_back[var_id] = variables[id_from];
					}
				}
			}

			if (verbose) {
				cout << "Renaming" << endl;
				for (auto it : renaming) {
					cout << it.first << ":" << it.second->id() << endl;
				}
				cout << "Renaming back" << endl;
				for (auto it : renaming_back) {
					cout << it.first << ":" << it.second->id() << endl;
				}
			}

			estimate = estimate.change_variables(renaming_back);
			estimates.push_back(make_shared<Factor>(estimate));
		}

		unsigned variables_sz = variables.size();
		for (unsigned i = N; i < variables_sz; ++i) {
			delete variables[i];
		}

		return estimates;
	}

}
