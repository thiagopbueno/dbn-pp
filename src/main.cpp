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

void print_elimination_ordering(vector<const Variable*> ordering) {
    cout << "Elimination ordering: { ";
    for (auto pv : ordering) { cout << pv->id() << " "; }
    cout << "}" << endl;
}

int main(int argc, char *argv[])
{
    unsigned order;
    vector<unique_ptr<Variable>> variables;
    vector<shared_ptr<Factor>> factors;

    unordered_map<unsigned,const Variable*> transition;
    vector<unsigned> sensor;

    read_uai_model(order, variables, factors, transition, sensor);

    vector<const Variable*> ordering {};
    unique_ptr<Factor> factor;

    cout << ">> VARIABLE ELIMINATION" << endl;
    factor = variable_elimination(ordering, factors);
    print_elimination_ordering(ordering);
    cout << *factor << endl;

    for (auto const& pv : variables) {
        ordering.push_back(pv.get());
        factor = variable_elimination(ordering, factors);
        print_elimination_ordering(ordering);
        cout << *factor << endl << endl;
    }

    cout << ">> CONDITIONING" << endl;
    unordered_map<unsigned,unsigned> evidence;
    evidence[0] = 1;
    evidence[1] = 0;

    factor = unique_ptr<Factor>(product(*factors[1], *factors[2]));
    cout << *factor << endl;
    factor = unique_ptr<Factor>(conditioning(*factor, evidence));
    cout << *factor << endl;
    factor = unique_ptr<Factor>(normalization(*factor));
    cout << *factor << endl << endl;

    for (auto const& pf : factors) {
        factor = unique_ptr<Factor>(conditioning(*pf, evidence));
        cout << *factor << endl;
        factor = unique_ptr<Factor>(normalization(*factor));
        cout << *factor << endl << endl;
    }

    cout << ">> CHANGE SCOPE" << endl;
    cout << *factors[1] << endl;
    factor = unique_ptr<Factor>(sum_product(*factors[1], Factor(1.0), variables[0].get()));
    cout << *factor << endl;
    factor->change_variables(transition);
    cout << *factor << endl << endl;
    factor = unique_ptr<Factor>(product(*factors[1], *factors[2]));
    cout << *factor << endl;
    transition[0] = variables[1].get();
    transition[1] = variables[2].get();
    transition[2] = variables[3].get();
    factor->change_variables(transition);
    cout << *factor << endl << endl;
    factor = unique_ptr<Factor>(new Factor(1.0));
    cout << *factor << endl;
    factor->change_variables(transition);
    cout << *factor << endl << endl;

    return 0;
}
