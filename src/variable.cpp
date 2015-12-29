// Copyright (c) 2013 Denis Maua
// All Rights Reserved.
//
// This file is part of MAP Local Search (MLS)
//
// MLS is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MLS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with MLS.  If not, see <http://www.gnu.org/licenses/>.

// Implements class Variable

#include "variable.h"

namespace msp {

  unsigned Variable::_num_instances = 0;

  // Default printing
  std::ostream& operator<<(std::ostream &o, const Variable &v) {
    o << "Variable(id:" << v.id() << ", name:\"" << v.name() << ", size:"<< v.size() << "\")";
    return o;
  }

  std::ostream& operator<<(std::ostream &o,  Variable &v) {
    o << "Variable(id:" << v.id() << ", name:\"" << v.name() << "\"" << ", size:"<< v.size() << v.get_extra_printout_info() << ")";
    return o;
  }

}


