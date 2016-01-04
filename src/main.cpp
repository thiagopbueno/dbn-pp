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

#include "io.h"
#include "operations.h"
#include "inference.h"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;
using namespace dbn;

void print_elimination_ordering(std::vector<const Variable*> ordering) {
    std::cout << "Elimination ordering: { ";
    for (auto pv : ordering) {
        std::cout << pv->id() << " ";
    }
    std::cout << "}" << std::endl;
}

void print_factor(const Factor &f) {
    std::cout << f << std::endl;
    double prob = 0.0;
    for (int i = 0; i < f.size(); ++i) { prob += f[i]; }
    std::cout << "Total Probability = " << prob << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
    unsigned order;
    std::vector<std::unique_ptr<Variable>> variables;
    std::vector<std::shared_ptr<Factor>> factors;

    std::unordered_map<unsigned, unsigned> transition;
    std::vector<unsigned> sensor;

    read_uai_model(order, variables, factors, transition, sensor);

    std::vector<const Variable*> ordering {};
    std::unique_ptr<Factor> factor;

    cout << ">> VARIABLE ELIMINATION" << endl;
    factor = variable_elimination(ordering, factors);
    print_elimination_ordering(ordering);
    print_factor(*factor);

    for (auto const& pv : variables) {
        ordering.push_back(pv.get());
        factor = variable_elimination(ordering, factors);
        print_elimination_ordering(ordering);
        print_factor(*factor);
    }

    cout << ">> CONDITIONING" << endl;
    std::unordered_map<unsigned,unsigned> evidence;
    evidence[0] = 1;

    for (auto const& pf : factors) {
        factor = unique_ptr<Factor>(conditioning(*pf, evidence));
        print_factor(*factor);
    }

    return 0;
}
