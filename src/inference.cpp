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
					unique_ptr<Factor> p(product(*prod, **pf));
					prod = move(p);
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

	vector<shared_ptr<Factor>> filtering(
		vector<shared_ptr<Factor>> &factors,
		std::vector<unsigned> &prior,
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

		// variable elimination
		vector<const Variable*> ordering;
		vector<shared_ptr<Factor>> sum_prod_factors;
		for (auto it_transition : transition) {
			unsigned id_prime = it_transition.first;
			const Variable *variable = it_transition.second;
			ordering.push_back(variable);
			sum_prod_factors.push_back(factors[id_prime]);
		}

		for (auto evidence : observations) {

			sum_prod_factors.push_back(forward);

			// compute sum_product factors
			unique_ptr<Factor> sum_prod = variable_elimination(ordering, sum_prod_factors);
			sum_prod->change_variables(transition);

			// add observation from time t
			unique_ptr<Factor> evidence_t = unique_ptr<Factor>(conditioning(*sensor_model, evidence));

			// unique_ptr<Factor> evidence_t = unique_ptr<Factor>(new Factor(1.0));
			// for (auto sensor_id : sensor) {
			// 	unique_ptr<Factor> sensor_factor = unique_ptr<Factor>(conditioning(*factors[sensor_id], evidence));
			// 	evidence_t = unique_ptr<Factor>(product(*evidence_t, *sensor_factor));
			// }

			forward = unique_ptr<Factor>(product(*evidence_t, *sum_prod));
			forward = unique_ptr<Factor>(normalization(*forward));

			// add new estimate to filtering list
			estimates.push_back(forward);

			sum_prod_factors.pop_back();
		}

		return estimates;
	}

}
