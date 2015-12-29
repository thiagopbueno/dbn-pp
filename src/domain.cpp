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

/** Implements class Domain. */

#include "domain.h"
#include <cmath>

namespace msp {

  Domain::Domain(const std::vector<Variable* >& scope) {
    _scope.assign(scope.begin(),scope.end());
    // compute variable offsets and domain cardinality in time linear in the num. of variables
    _offset.resize(_scope.size());
    _var_to_index.reserve(_scope.size());
    _size = 1;
    for (unsigned i = 0; i < _scope.size(); ++i) {
      _offset[i] = _size;
      _size *= scope[i]->size();
      _var_to_index[_scope[i]->id()] = i;
    }
  }

  /** return the position of the configuration in the linearization of domain. */
  unsigned Domain::position(unsigned configuration[]) {
        unsigned pos = 0;
        for (unsigned i = 0; i < _scope.size(); ++i) pos += configuration[i] * _offset[i];
        return pos;
    }

  /** returns (pointer to) union of two domains. */
  Domain* union_of(const Domain* d1, const Domain* d2)
  {
    // get union of scopes
    // start with a copy of d1's scope
    std::vector<Variable* > scope(d1->scope_begin(), d1->scope_end());
    // then add variables of d2's scope not already in the scope
    for (auto it=d2->scope_begin(); it != d2->scope_end(); ++it)
      if (!d1->in_scope(**it)) scope.push_back(*it);
    return new Domain(scope);
  }

  /** returns (pointer to) union of two domains with given variable subctracted. */
  Domain* union_of(const Domain* d1, const Domain* d2, const Variable* v)
  {
    // get union of scopes
    // start with a copy of d1's scope
    std::vector<Variable* > scope;
    for (auto it = d1->scope_begin(); it != d1->scope_end(); ++it)
      if (**it != *v) scope.push_back(*it);
    // then add variables of d2's scope not already in the scope
    for (auto it=d2->scope_begin(); it != d2->scope_end(); ++it)
    	if (**it != *v && !d1->in_scope(**it)) scope.push_back(*it);    
    return new Domain(scope);
  }


  
  /** Default printing. */
  std::ostream& operator<<(std::ostream &o, const Domain &d) {
    o << "Domain(width:" << d.width() << ", lg_size:" << log2(d.size()) << ", scope:{";
    if (d.width() <= MSP_DIS_MAX_VARS)
      for (int i=0; i < d.width(); ++i) { o << d[i]->name(); if (i < d.width()-1) o << ", "; }
    else 
      {
	for (int i=0; i < MSP_DIS_MAX_VARS-1; ++i) o << d[i]->name() << ", "; 
	o << "..., " << *(d[d.width()-1]);
      }
    o << "})";
    return o;
  }

  /** Default printing. */
  std::ostream& operator<<(std::ostream &o, Domain &d) {
    o << "Domain(width:" << d.width() << ", lg_size:" << log2(d.size()) << ", scope:{";
    if (d.width() <= MSP_DIS_MAX_VARS)
      for (int i=0; i < d.width(); ++i) { o << d[i]->name(); if (i < d.width()-1) o << ", "; }
    else 
      {
	for (int i=0; i < MSP_DIS_MAX_VARS-1; ++i) o << d[i]->name() << ", "; 
	o << "..., " << *(d[d.width()-1]);
      }
    o << "})";
    return o;
  }

}
