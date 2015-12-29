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

// This file contains interfaces to operations on factors and variables

#ifndef MSP_OPERATIONS_H
#define MSP_OPERATIONS_H

#include "variable.h"
#include "factor.h"

namespace msp {

  /** Multiplies two factors 
   * @param f1 the first factor
   * @param f2 the second factor
   * @result a factor containing the product of f1 and f2
   */
  Factor product(const Factor& f1, const Factor& f2);

  /** Combined factor multiplication and variable elimination 
   *  @param f1 the first factor
   *  @param f2 the second factor
   *  @param var a pointer to a variable
   *  @return the elimination of var from the product of f1 and f2
   */
  Factor sum_product(const Factor& f1, const Factor& f2, const Variable& var);

}

#endif
