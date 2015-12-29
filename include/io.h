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

// INPUT-OUTPUT ROUTINES

#ifndef _MSP_IO_FILE_H
#define _MSP_IO_FILE_H

#include "variable.h"
#include "factor.h"

namespace msp {
  // read factor model in UAI format from stdin
  void load_uai_model( std::vector<Variable > &,  std::vector<Factor >&);
}

#endif
