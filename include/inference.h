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

#ifndef _DBN_INFERENCE_H
#define _DBN_INFERENCE_H

#include "variable.h"
#include "factor.h"

#include <vector>
#include <memory>

namespace dbn {

	std::unique_ptr<Factor> variable_elimination(
		std::vector<const Variable*> &variables,
		std::vector<std::shared_ptr<Factor>> &factors
	);

	std::vector<std::shared_ptr<Factor>> filtering(
		std::vector<std::shared_ptr<Factor>> &factors,
		std::vector<unsigned> &prior,
		std::unordered_map<unsigned,const Variable*> &transition,
		std::vector<unsigned> &sensor,
		std::vector<std::unordered_map<unsigned,unsigned>> &observations
	);
}

#endif
