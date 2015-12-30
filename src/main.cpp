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

#include "variable.h"
#include "io.h"
#include "model.h"
#include "operations.h"

#include <iostream>
#include <vector>
#include <memory>

using namespace dbn;

int main(int argc, char *argv[])
{
    unsigned order;
    std::vector<std::unique_ptr<Variable> > variables;
    std::vector<std::unique_ptr<Factor> > factors;

    read_uai_model(order, variables, factors);
    
    for (auto const &pv : variables) {
    	std::cout << *pv << std::endl;
    }

    for (auto const &pf : factors) {
    	std::cout << *pf << std::endl;
    }

    // std::unique_ptr<Model> m{new Model(order, variables, factors)};
    // std::cout << *m << std::endl;

    return 0;
}
