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
#include "operations.h"
#include "inference.h"

#include <iostream>
#include <vector>
#include <memory>

using namespace dbn;

void print_factor(const Factor &f) {
    std::cout << f << std::endl;
    double prob = 0.0;
    for (int i = 0; i < f.size(); ++i) { prob += f[i]; }
    std::cout << "P(True) = " << prob << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
    unsigned order;
    std::vector<std::unique_ptr<Variable> > variables;
    std::vector<std::shared_ptr<Factor> > factors;

    read_uai_model(order, variables, factors);

    std::vector<const Variable* > ordering { (variables[1]).get(), (variables[2]).get(), (variables[0]).get() };
    std::unique_ptr<Factor> elim = variable_elimination(ordering, factors);
    print_factor(*elim);

    // for (auto const& pv : variables) {
    //     ordering.insert(ordering.begin(), pv.get());
    //     elim = variable_elimination(ordering, factors);
    //     print_factor(*elim);
    // }

    return 0;
}
