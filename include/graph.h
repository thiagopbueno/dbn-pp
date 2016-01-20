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

#ifndef _DBN_GRAPH_H
#define _DBN_GRAPH_H

#include "variable.h"
#include "factor.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>

namespace dbn {

	class Graph {
	public:
		Graph(const std::vector<std::shared_ptr<Factor>> &factors);
		std::vector<const Variable*> ordering(const std::vector<const Variable*> &variables);

		friend std::ostream &operator<<(std::ostream &os, const Graph &g);

	private:
		unsigned min_fill(std::vector<const Variable*> &variables, std::unordered_set<const Variable*> &processed);

		std::unordered_map<const Variable*,std::unordered_set<const Variable*>> _adj;
	};

}

#endif