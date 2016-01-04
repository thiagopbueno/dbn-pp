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

#include <forward_list>
#include <utility>
#include <iostream>

#include "inference.h"
#include "operations.h"

namespace dbn {

	std::unique_ptr<Factor> variable_elimination(std::vector<const Variable*> &variables, std::vector<std::shared_ptr<Factor>> &factors) {

	    std::forward_list<std::shared_ptr<Factor>> flist(factors.begin(), factors.end());
	    std::forward_list<std::shared_ptr<Factor>> bucket;

		for (auto var: variables) {

			// std::cout << ">> Eliminating variable: " << *var << std::endl;

			// std::cout << ">> All factors:" << std::endl;
			// for (auto f : flist) {
			// 	std::cout << *f << std::endl;
			// }

			// select all factors with var in its scope
			bucket.clear();
			unsigned b = 0; // bucket size

			std::forward_list<std::shared_ptr<Factor>>::const_iterator pf;

			std::forward_list<std::shared_ptr<Factor>> new_flist;
			for (pf = flist.begin(); pf != flist.end(); ++pf) {

				if ((*pf)->domain().in_scope(var)) {
					bucket.push_front(*pf);
					b++;
				}
				else {
					new_flist.push_front(*pf);
				}
			}

			// std::cout << ">> Bucket:" << std::endl;
			// for (auto f : bucket) {
			// 	std::cout << *f << std::endl;
			// }

			// std::cout << ">> Remaining factors:" << std::endl;
			// for (auto f : new_flist) {
			// 	std::cout << *f << std::endl;
			// }

			flist = new_flist;

			// multiply all factors in bucket and eliminate variable
			if (b > 0) {
				std::unique_ptr<Factor> prod(new Factor(1.0));
				std::forward_list<std::shared_ptr<Factor>>::const_iterator pf = bucket.begin();
				while (b > 1) {
					// std::cout << ">> prod X prod" << std::endl;
					// std::cout << *prod << std::endl;
					// std::cout << **pf << std::endl;

					std::unique_ptr<Factor> p(product(*prod, **pf));

					// std::cout << "ANS: ";
					// std::cout << *p << std::endl;

					prod = std::move(p);
					pf++;
					b--;
				}

				std::unique_ptr<Factor> p(sum_product(*prod, **pf, var));

				// std::cout << ">> sum_prod" << std::endl;
				// std::cout << *prod << std::endl;
				// std::cout << **pf << std::endl;
				// std::cout << *p << std::endl;

				new_flist.push_front(std::move(p));
	  		}

			// std::cout << ">> New list:" << std::endl;
			// for (auto f : new_flist) {
			// 	std::cout << *f << std::endl;
			// }
			// std::cout << std::endl;

	  		flist = new_flist;
		}

		// generate result by multiplying all remaining factors in the pool
		std::unique_ptr<Factor> prod(new Factor(1.0));
		std::forward_list<std::shared_ptr<Factor>>::const_iterator pf;
		for (pf = flist.begin(); pf != flist.end(); ++pf) {
			prod = std::unique_ptr<Factor>(product(*prod, **pf));
		}

		return prod;
	}

}
