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
#include <iostream>
#include <algorithm>

using namespace std;

namespace dbn {

	Factor variable_elimination(
		vector<const Variable*> &variables,
		vector<shared_ptr<Factor>> &factors) {

	    forward_list<shared_ptr<Factor>> flist(factors.begin(), factors.end());
	    forward_list<shared_ptr<Factor>> bucket;

	    Graph g(factors);
		vector<const Variable*> ordering = g.ordering(variables);

		for (auto var: ordering) {

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
				Factor prod(1.0);
				for (auto const& f : bucket) {
					prod *= *f;
				}
				shared_ptr<Factor> p = make_shared<Factor>(prod.sum_out(var));

				new_flist.push_front(move(p));
	  		}

	  		flist = new_flist;
		}

		// generate result by multiplying all remaining factors in the pool
		Factor result(1.0);
		for (auto &f : flist) {
			result *= *f;
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
		vector<shared_ptr<Factor>> &factors,
		vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor,
		vector<unordered_map<unsigned,unsigned>> &observations) {

		// estimates
		vector<shared_ptr<Factor>> estimates;

		// prior model
		Factor prior_model(1.0);
		for (auto id : prior) {
			prior_model = prior_model * *(factors[id]);
		}

		// sensor model
		Factor sensor_model(1.0);
		for (auto id : sensor) {
			sensor_model = sensor_model * *(factors[id]);
		}

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

	    forward_list<shared_ptr<ADDFactor>> flist(factors.begin(), factors.end());
	    forward_list<shared_ptr<ADDFactor>> bucket;

		for (auto var: variables) {

			// select all factors with var in its scope
			bucket.clear();
			unsigned b = 0; // bucket size

			forward_list<shared_ptr<ADDFactor>>::const_iterator pf;

			forward_list<shared_ptr<ADDFactor>> new_flist;
			for (pf = flist.begin(); pf != flist.end(); ++pf) {

				if ((*pf)->in_scope(var)) {
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
				ADDFactor prod;
				for (auto const& f : bucket) {
					prod *= *f;
				}
				shared_ptr<ADDFactor> p = make_shared<ADDFactor>(prod.sum_out(var));

				new_flist.push_front(move(p));
			}

			flist = new_flist;
		}

		// generate result by multiplying all remaining factors in the pool
		ADDFactor result;
		for (auto &f : flist) {
			result *= *f;
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
		vector<shared_ptr<ADDFactor>> &factors,
		vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor,
		vector<unordered_map<unsigned,unsigned>> &observations)
	{
		// estimates
		vector<shared_ptr<ADDFactor>> estimates;

		// prior model
		ADDFactor prior_model;
		for (auto id : prior) {
			prior_model *= *factors[id];
		}

		// sensor model
		ADDFactor sensor_model;
		for (auto id : sensor) {
			sensor_model *= *factors[id];
		}

		// initialize forward message
		ADDFactor forward = prior_model;

		for (auto evidence : observations) {
			// project belief state
			ADDFactor projection = project(factors, transition, forward);

			// update belief state
			forward = update(projection, sensor_model, evidence);

			// add new estimate to filtering list
			estimates.push_back(make_shared<ADDFactor>(forward));
		}

		return estimates;
	}


	vector<shared_ptr<Factor>>
	unrolled_filtering(
		vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors,
		vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor,
		vector<unordered_map<unsigned,unsigned>> &observations,
		bool verbose)
	{

		vector<const Variable*> vars;
		vector<shared_ptr<Factor>> estimates;

		int N = variables.size();
		for (auto const &pv : variables) {
			vars.push_back(pv.get());
		}

		unordered_map<unsigned,const Variable *> renaming;
		for (auto it_transition : transition) {
			const Variable *next_var = vars[it_transition.first];
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
			ordering.push_back(vars[id_curr]);
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
			cout << endl;
		}

		Factor estimate = variable_elimination(ordering, unrolled_factors).normalize();
		unordered_map<unsigned,const Variable *> renaming_back;
		const Domain &estimate_domain = estimate.domain();
		for (unsigned i = 0; i < estimate_domain.width(); ++i) {
			unsigned var_id = estimate_domain[i]->id();
			for (auto it_renaming : renaming) {
				unsigned id_from = it_renaming.first;
				unsigned id_to = it_renaming.second->id();
				if (id_to == var_id) {
					renaming_back[var_id] = vars[id_from];
				}
			}
		}
		estimate = estimate.change_variables(renaming_back);
		estimates.push_back(make_shared<Factor>(estimate));

		unsigned id = factors.size();
		for (unsigned t = 1; t < observations.size(); ++t) {

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

				const Variable *new_var = new Variable(id, vars[id_next]->size());
				renaming[id_next] = new_var;
				vars.push_back(new_var);
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

			for (auto sensor_id : sensor) {
				const Variable *new_var = new Variable(id, vars[sensor_id]->size());
				renaming[sensor_id] = new_var;
				vars.push_back(new_var);
				id++;

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
				cout << endl;
			}

			Factor estimate = variable_elimination(ordering, unrolled_factors).normalize();
			unordered_map<unsigned,const Variable *> renaming_back;
			const Domain &estimate_domain = estimate.domain();
			for (unsigned i = 0; i < estimate_domain.width(); ++i) {
				unsigned var_id = estimate_domain[i]->id();
				for (auto it_renaming : renaming) {
					unsigned id_from = it_renaming.first;
					unsigned id_to = it_renaming.second->id();
					if (id_to == var_id) {
						renaming_back[var_id] = vars[id_from];
					}
				}
			}
			estimate = estimate.change_variables(renaming_back);
			estimates.push_back(make_shared<Factor>(estimate));
		}

		unsigned vars_sz = vars.size();
		for (unsigned i = N; i < vars_sz; ++i) {
			delete vars[i];
		}

		return estimates;
	}

}
