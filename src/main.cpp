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

    std::unique_ptr<Factor> f0(new Factor(1.0));

    std::unique_ptr<Factor> f = std::move(f0);
    for (unsigned i = 0; i < order; ++i) {
        f = product(*f, *(factors[i]));

        std::cout << *f << std::endl;
        double prob = 0.0;
        for (int i = 0; i < f->size(); ++i) { prob += (*f)[i]; }
        std::cout << "P(True) = " << prob << std::endl;
    }

    return 0;
}
