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

/** Interface for the Factor class */

#ifndef MSP_FACTOR_H
#define MSP_FACTOR_H

#include "constants.h"
#include "variable.h"
#include "domain.h"
#include <string>
#include <vector>
#include <iostream>

namespace msp {

  /**  Class of real-valued factors.
   *
   * A factor is a real-valued multidimensional mapping
   * which maps every configuration of a set of variables
   * into a real number. Algebraic operations such as product,
   * sum, and variable elimination are defined on factors.
  */
  class Factor {

  private:
    unsigned _id; /**< factor id. */
    Domain* _domain; /**< factor domain. */
    std::vector<double> _values; /**< factor image. */
    std::string _name; /**< factor name (optional). */
    static unsigned _num_instances; /**< total number of factor objects instantiated. */
    
  public:
    Factor() : _id( _num_instances++ ), _domain( new Domain() ), _values(1,MSP_DEFAULT_VALUE) {} /*< unidimensional factor constructor */
    Factor(double value) : _id( _num_instances++ ), _domain( new Domain() ), _values(1,value) {} /*< constant factor constructor. */
    /** Constructor for factor given the domain scope, that is, a vector of (distinct) variables */
    Factor(std::vector<Variable* >& scope) :  _id( _num_instances++ ), _domain ( new Domain(scope) ), _values(_domain->size(),MSP_DEFAULT_VALUE) {}
    Factor(std::vector<Variable* >& scope, double value) :  _id( _num_instances++ ), _domain ( new Domain(scope) ), _values(_domain->size(),value) {}

    /** Constructor for rvalue assignment */
  /* Factor(Factor&& f) : _id( std::move(f._id) ), _domain( std::move(f._domain) ), _values( std::move(f._values) ), _name( std::move(f._name) )  {}  */

    /** Constructs a factor whose domain is the union of two given factors */
    Factor(const Factor& f1, const Factor& f2);

    /** Constructs a factor whose domain is the union of two given factors with a given variable removed */
    Factor(const Factor& f1, const Factor& f2, const Variable& v);

    unsigned id() const { return _id; } 
    unsigned size() const { return _values.size(); }
    unsigned width() const { return _domain->width(); }

    /** size of i-th variable in scope 
     * @param i an integer
     * @result the size of the i-th variable in the scope
     */
    unsigned size_of_var(unsigned i) { return _domain->size_of_var(i); }

    /** get the domain offset for the i-th variable in the scope.
     * @param i an integer specifiy a variable index.
     * @return the offset by which values of the i-th variable are shifted when linearizing the array.
     */
    unsigned offset(unsigned i) const { return _domain->offset(i); }

    /** get the domain offset for a variable.
     * @param v a variable
     * @return the offset by which values of the variable are shifted when linearizing the array or zero if the variable is not in the scope.
     */
    unsigned offset(const Variable& v) const { return _domain->offset(v); }

    /** determines whether variable is in the scope.
     * @param v a variable
     * @return true if it is, false otherwise
     */
    bool in_scope(const Variable& v) const { return _domain->in_scope(v); }

    /** get variable at localtion i in the scope.
     * @param i an integer
     * @return a pointer to the variable object
     */
    Variable* var_at(unsigned i) const { return _domain->var_at(i); }

    /** sets the i-th value.
     * @param i an integer specifying a position in the linearized array.
     * @param value a double specifying the new value.
     */
    void set(unsigned i, double value) { 
      if (i < _values.size()) _values[i] = value;
      else throw "Index out of range.";
    }
    
    /** Returns factor label. */
    std::string name() const { return _name; }

    /** Returns total number of instances created. */
    static unsigned get_num_instances() { return _num_instances; }
    
    /** Returns the i-th value.
     * @param i an integer specifying a variable index.
     * @return the value at the i-th position of the linearized array.
     */
    double operator[](unsigned i) const { 
      if (i < _values.size()) return _values[i];
      else throw "Index out of range.";
    }

    /** Returns iterator to first value. */
    std::vector<double>::iterator begin() { return _values.begin(); }

    /** Returns iterator to past the last value. */
    std::vector<double>::iterator end() { return _values.end(); }

    /* /\** Factor assignment *\/ */
    /* friend void swap(Factor& f, Factor& g)  */
    /*   { */
    /* 	std::swap(f._id, g._id); */
    /* 	Domain* t = f._domain; */
    /* 	f._domain = g._domain; */
    /* 	g._domain = t; */
    /* 	t = 0; */
    /* 	f._values.swap(g._values); */
    /* 	f._name.swap(g._name); */
    /*   } */


    /* /\** Factor assignment *\/ */
    /* Factor& operator=(Factor f) */
    /*   { */
    /* 	swap(*this,f); */
    /* 	return *this; */
    /*   } */

    /* /\** factor multiplication. *\/ */
    /* Factor operator*(const Factor& f) const; // { Factor f; return f; } */

    /** factor comparison. */
    bool operator<=(const Factor& f) const {
      if (size() != f.size()) return false;
      for (unsigned i=0; i<size(); i++) 
	if (_values[i] > f[i]) return false;
      return true;
    }

    /** factor comparison. */
    bool operator>=(const Factor& f) const {
      if (size() != f.size()) return false;
      for (unsigned i=0; i<size(); i++) 
	if (_values[i] < f[i]) return false;
      return true;
    }

    /** output factor content info. */
    friend std::ostream &operator<<(std::ostream &o, Factor &v); 
    friend std::ostream &operator<<(std::ostream &o, const Factor &f);    

  };

}

#endif
