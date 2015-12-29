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

// Defines class Variable

#ifndef MSP_VARIABLE_H
#define MSP_VARIABLE_H

#include <iostream>
#include <sstream>
#include <string>


namespace msp {

  /** Categorical random variable.
   *
   * A categorical random variable takes on finitely many states. 
   * The size of a variable is the number of states which it assumes.
   * Every variable object receives a unique id. Two varaibles are equal
   * if their ids match. Variables can also receive names (standard names
   * based on their ids are given if none is provided.
   */
  class Variable {

  private:
    unsigned _id; 
    int _size; 
    std::string _name; // variable description   
    static unsigned _num_instances; // total number of variables instantiated 
    
  public:
  Variable() : _id(_num_instances++), _size(0) { std::stringstream ssid; ssid << _id; _name="V"+ssid.str(); } // dimensionless variable
  Variable(int size) : _id(_num_instances++), _size(size) { std::stringstream ssid; ssid << _id; _name="V"+ssid.str(); }
  Variable(int size, std::string name) : _id(_num_instances++), _size(size), _name(name) { }
    unsigned id() const { return _id; } 
    unsigned size() const { return _size; } // number of states
    std::string name() const { return _name; } 
    static unsigned get_num_instances() { return _num_instances; }

    /* comparison operators */
    bool operator==(const Variable& other) { return (this->_id==other._id); } // two variables are the same iff they share the same id
    bool operator!=(const Variable& other) { return (this->_id!=other._id); } // two variables are the same iff they share the same id

    bool operator<(const Variable& other) const { return (this->_id<other._id); } // order variables by their ids 
    bool operator>(const Variable& other) const { return (this->_id>other._id); } // order variables by their ids 
    bool operator<=(const Variable& other) const { return (this->_id<=other._id); } // order variables by their ids 
    bool operator>=(const Variable& other) const { return (this->_id>=other._id); } // order variables by their ids 

    friend std::ostream &operator<<(std::ostream &o, Variable &v); // output variable info
    friend std::ostream &operator<<(std::ostream &o, const Variable &v);    

  protected:
    virtual const char* get_extra_printout_info() { return ""; }; // collect information specific to variable subtypes    
        
  };

  //bool compareVariables(Variable* v1, Variable* v2) { return (v1->id() < v2->id()); }

}

#endif
