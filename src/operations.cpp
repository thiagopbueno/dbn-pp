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

// This file implements operations on factors and variables

#include <vector>
#include "variable.h"
#include "factor.h"
#include "operations.h"
#include "constants.h"

namespace msp {

  /** Factor multiplication 
   *  @param f1 the first factor
   *  @param f2 the second factor
   *  @return the product of f1 and f2
   */
  Factor product(const Factor& f1, const Factor& f2) {

    Factor res(f1,f2); // creates factor with union domain

    unsigned i, j=0, k=0, l;
    std::vector<unsigned> c(res.width(),0);
    Variable* v;

    for (i=0; i<res.size(); i++)
      {
	res.set(i, f1[j] * f2[k]);
	for (l=0; l<res.width(); l++)
	  {
	    c[l]++;
	    v = res.var_at(l);
	    if (c[l] == v->size())
	      {
		c[l] = 0;
		j -= (v->size()-1)*f1.offset( *v );
		k -= (v->size()-1)*f2.offset( *v );
	      }
	    else
	      {
		j += f1.offset( *v ); k += f2.offset( *v );
		break;
	      }
	  }
      }

    return res;

  }


  /** Combined factor multiplication and variable elimination 
   *  @param f1 the first factor
   *  @param f2 the second factor
   *  @param var a pointer to a variable
   *  @return the elimination of var from the product of f1 and f2
   */
  Factor sum_product(const Factor& f1, const Factor& f2, const Variable& var) {

    Factor res(f1,f2,var); // creates factor with union domain setminus var

    unsigned i, j=0, k=0, l, m;
    std::vector<unsigned> c(res.width()+1,0);
    Variable* v;

    for (i=0; i<res.size(); i++)
      {
	for (m=0; m<var.size(); m++) 
	  {
	    res.set(i, res[i] + f1[j] * f2[k]);
	    c[0]++;
	    if (c[0] == var.size())
	      {
		c[0] = 0;
		j -= (var.size()-1)*f1.offset( var );
		k -= (var.size()-1)*f2.offset( var );

		for (l=1; l<res.width()+1; l++)
		  {
		    c[l]++;
		    v = res.var_at(l-1);
		    if (c[l] == v->size())
		      {
			c[l] = 0;
			j -= (v->size()-1)*f1.offset( *v );
			k -= (v->size()-1)*f2.offset( *v );
		      }
		    else
		      {
			j += f1.offset( *v ); k += f2.offset( *v );
			break;
		      }
		  }

	      }
	    else
	      {
		j += f1.offset( var ); k += f2.offset( var );
	      }
	  }
      }

    return res;

  }

}
