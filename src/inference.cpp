// Copyright (c) 2014 Denis Maua
// All Rights Reserved.
//
// This file is part of MSP library
//
// MSP is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MSP is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with MSP.  If not, see <http://www.gnu.org/licenses/>.

/** Implements direct inference algorithms */

/* whether to use Pareto pruninig in set-valued variable elimination */

#include <forward_list>
#include <utility>
#include <iostream>

#include "constants.h"
#include "inference.h"
#include "operations.h"

namespace msp {

  /** variable elimination algorithm.
   *
   * eliminates variables in the ordering given.
   *
   * @param variables a vector of variables
   * @param factors a forward list (singly linked list) of factors
   * @param verbosity an integer
   * @return a factor 
   */
  Factor variable_elimination(const std::vector<Variable >& variables, std::vector<Factor >& factors, int verbosity)
  {

    std::forward_list<Factor > flist(factors.begin(),factors.end()); // pool of factors
    std::forward_list<Factor > bucket;
    for (auto var: variables)
      {
	if (verbosity) std::cout << "-" << var << std::endl;
	// collect all factors with variable *v in their scope and remove them from factor list
	bucket.clear();
	unsigned b = 0; // bucket size
	for (std::forward_list<Factor >::const_iterator pf=flist.before_begin(), f=flist.begin(); f != flist.end(); pf=f, f++)
	  {
	    if ( f->in_scope( var ) ) { 
	      bucket.push_front( std::move( *f ) );
	      b++; f = pf;
	      flist.erase_after(pf);
	    }
	  }      
	// multiply all factors in bucket and eliminate variable *v
	if (b>0) {	    
	    Factor p(1.0);
	    std::forward_list<Factor >::const_iterator f = bucket.begin();
	    while (b>1)
	      {
		p = product(p,*f);
		f++; b--;
	      }
	    flist.push_front( sum_product(p,*f,var) );	    
	    if (verbosity > 1) { f = flist.begin(); std::cout << "+" << *f << std::endl; }	    
	  } 
      }    
    // generate result by multiplying all remaining factors in the pool
    Factor p(1.0); // product-identity factor
    for (std::forward_list<Factor >::const_iterator f = flist.begin(); f != flist.end(); f++)
    	p = product(p,*f);
    return p;
  }

}
