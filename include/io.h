// Copyright (c) 2015 Thiago Pereira Bueno
// All Rights Reserved.
//
// This file is part of DBN library.
//
// DBN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DBN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DBN.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _DBN_IO_FILE_H
#define _DBN_IO_FILE_H

#include "variable.h"
#include "factor.h"

#include <vector>
#include <memory>

namespace dbn {

	int read_uai_model(unsigned &order, std::vector<std::unique_ptr<Variable>> &variables, std::vector<std::shared_ptr<Factor>> &factors, std::unordered_map<unsigned,const Variable*> &transition, std::vector<unsigned> &sensor);

}

#endif
