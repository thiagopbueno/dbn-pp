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

/** Describes Domain class interface. */

#ifndef MSP_DOMAIN_H
#define MSP_DOMAIN_H

#include <vector>
#include <iostream>
#include <unordered_map>
#include <utility>
#include "constants.h"
#include "variable.h"

namespace msp {

  /** Class for domain objects.
   *
   * A domain is a set of multidimensional tuples where each coordinate
   * is a state of a variable.
   * The width of a domain is the number of variables involved, while its
   * size is the number of configurations. We consider only domains obtained
   * as the direct product of variable domains.
   */

  class Domain {

  protected:
    std::vector<Variable* > _scope;
    unsigned _size; /*< size of domain. */
    std::vector<unsigned> _offset; /*< position offset used when extending ordering. */
    std::unordered_map<unsigned, unsigned> _var_to_index; /*< variable's order in scope. */
    //    unsigned* _projpos; // cache for projected position
    //    unsigned* _var_to_incr; // cache for next variable to increase in ordering
  public:
    Domain() : _size(1) {} /*< Constructor for the empty domain. */
    /** Constructor for a given tuple of variables representing the scope. */
    Domain(const std::vector<Variable* >& scope);
    Domain(const std::vector<Variable* >&& scope);
    /** Constructor for a given array of pointer to variables representing the scope. */
    //Domain(Variable** scope, unsigned N);
    /** Constructor for rvalue */
    Domain(Domain&& d) : _scope( std::move(d._scope) ), _size( std::move(d._size) ), _offset( std::move(d._offset) ), _var_to_index( std::move (d._var_to_index) ) {} 
    
    /** returns domain cardinality. */
    unsigned size() const { return _size; } 

    /** returns scope cardinality */
    unsigned width() const { return _scope.size(); }

    /** size of i-th variable in scope 
     * @param i an integer
     * @result the size of the i-th variable in the scope
     */
    unsigned size_of_var(unsigned i) const
    {
      if (i < _scope.size()) return _scope[i]->size(); 
      else throw "Domain::size_of_var: Index out of range!";
    }

    /** returns the position of the configuration in the array of values. */
    unsigned position(unsigned configuration[]);

    /** returns iterator to the first variable in the scope */
    std::vector<Variable *>::const_iterator scope_begin() const { return _scope.begin(); }


    /** returns iterator to the last variable in the scope */
    std::vector<Variable *>::const_iterator scope_end() const { return _scope.end(); }

    /** returns the position of the configuration in the array of values. */
    unsigned operator[](unsigned configuration[]) { return position(configuration); }
    // TO-DO: Implement overloaded version in which configuration is given as a dictionary on a possibly larger scope

    /** determines whether variable is in the scope.
     * @param v a variable
     * @return true if it is, false otherwise
     */
    bool in_scope(const Variable& v) const {
      std::unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find (v.id());
      if (it == _var_to_index.end())
	return false;
      else
	return true;
    }

    /** returns the offset of the i-th variable in the scope. 
     * @param i an unsigned integer
     * @return an integer
     */
    unsigned offset(unsigned i) const { 
      if (i < _scope.size()) return _offset[i];
      else throw "Domain::offset: Index out of range!";
    }

    /** returns the offset of a variable in the scope. 
     * @param v a variable.
     * @return an integer (zero if the variable is not in the scope.
     */
    unsigned offset(const Variable& v) const { 
      std::unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find (v.id());
      if (it == _var_to_index.end())
	return 0;
      else
	return _offset[it->second];
    }

    /** returns i-th variable in the scope. */
    Variable* var_at(unsigned i) const {
      return _scope[i];
    }

    /** returns i-th variable in the scope. */
    Variable* operator[](unsigned i) const { 
      if (i < _scope.size()) return _scope[i];
      else throw "Domain::operator[]: Index out of range!";
    }
    
    /** output domain content info. */
    friend std::ostream &operator<<(std::ostream &o, Domain &v); 
    friend std::ostream &operator<<(std::ostream &o, const Domain &v);    

  };

  /** Returns (pointer to) the union of two domains. */
  Domain* union_of(const Domain* d1, const Domain* d2);

  /** Returns (pointer to) the union of two domains with given variable subctracted. */
  Domain* union_of(const Domain* d1, const Domain* d2, const Variable* v);

}


#endif
