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

/* @file Interface for direct inference algorithms */

#ifndef MSP_INF_H
#define MSP_INF_H

#include <vector>

#include "variable.h"
#include "factor.h"

namespace msp {

  /** Variable Elimination algorithm.
   *
   * Eliminates variables in the ordering given.
   *
   * @param variables a vector of variables
   * @param factors a forward list (singly linked list) of factors
   * @return a factor 
   */
  Factor variable_elimination(const std::vector<Variable >& variables, std::vector<Factor >& factors, int verbosity=0);

}

#endif
