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

int main(int argc, char *argv[])
{
    unsigned order;
    vector<unique_ptr<Variable>> variables;
    vector<shared_ptr<Factor>> factors;

    vector<unsigned> prior;
    unordered_map<unsigned,const Variable*> transition;
    vector<unsigned> sensor;

    vector<unordered_map<unsigned,unsigned>> observations;

    if (read_uai_model(argv[1], order, variables, factors, prior, transition, sensor)) return -1;
    if (read_observations(argv[2], observations)) return -2;

    cout << ">> FILTERING" << endl;
    vector<shared_ptr<Factor>> estimates = filtering(factors, prior, transition, sensor, observations);

    unsigned t = 1;
    for (auto const& pf : estimates) {
        cout << "@ t = " << t << endl;
        cout << "observations: {";
        for (auto it_evidence : observations[t-1]) {
            unsigned id = it_evidence.first;
            unsigned value = it_evidence.second;
            cout << " " << id << ":" << value;
        }
        cout << " }" << endl;
        cout << *pf << endl << endl;
        ++t;
    }

    return 0;
}
