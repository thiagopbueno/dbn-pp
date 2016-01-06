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

#include <iomanip>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;
using namespace dbn;

void print_model(
    char *filename,
    vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors,
    vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor
);

void print_belief_states(vector<shared_ptr<Factor>> &states, vector<unordered_map<unsigned,unsigned>> &observations);

int main(int argc, char *argv[])
{
    unsigned order;
    vector<unique_ptr<Variable>> variables;
    vector<shared_ptr<Factor>> factors;

    vector<unsigned> prior;
    unordered_map<unsigned,const Variable*> transition;
    vector<unsigned> sensor;

    if (read_uai_model(argv[1], order, variables, factors, prior, transition, sensor)) return -1;
    cout << ">> MODEL: " << argv[1] << endl;
    print_model(argv[1], variables, factors, prior, transition, sensor);

    vector<unordered_map<unsigned,unsigned>> observations;
    if (read_observations(argv[2], observations)) return -2;

    cout << ">> FILTERING: " << argv[2] << endl;
    vector<shared_ptr<Factor>> states = filtering(factors, prior, transition, sensor, observations);
    print_belief_states(states, observations);

    return 0;
}

void print_model(
    char *filename,
    vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors,
    vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor) {

    cout << "=== Variables ===" << endl;
    for (auto const& pv : variables) {
        cout << *pv << endl;
    }
    cout << endl;

    cout << "=== Factors ===" << endl;
    for (auto const& pf : factors) {
        cout << *pf << endl;
    }
    cout << endl;

    cout << "=== Prior model ===" << endl;
    cout << "Variables {";
    for (auto id : prior) {
        cout << " " << id;
    }
    cout << " }" << endl << endl;

    cout << "=== Transition model ===" << endl;
    cout << "Variables {";
    for (auto it : transition) {
        unsigned id_next = it.first;
        unsigned id_curr = it.second->id();
        cout << " " << id_next << "->" << id_curr;
    }
    cout << " }" << endl << endl;

    cout << "=== Sensor model ===" << endl;
    cout << "Variables {";
    for (auto id : sensor) {
        cout << " " << id;
    }
    cout << " }" << endl << endl;
}

void print_belief_states(vector<shared_ptr<Factor>> &states, vector<unordered_map<unsigned,unsigned>> &observations) {
    cout << "=== Trajectory ===" << endl;

    const Domain &domain = states[0]->domain();
    for (unsigned i = 0; i < domain.width(); ++i) {
        cout << domain[i]->id() << " ";
    }
    cout << endl;

    std::vector<unsigned> instantiation(domain.width(), 0);
    for (unsigned i = 0; i < domain.size(); ++i) {

        unsigned pos = domain.position_instantiation(instantiation);

        // print instantiation
        for (auto d : instantiation) {
            cout << d << " ";
        }
        cout << ":";

        // print value trajectory
        for (auto const& pf : states) {
            cout << " " << setprecision(3) << (*pf)[pos];
        }
        cout << endl;

        // update instantiation
        domain.next_instantiation(instantiation);
    }
    cout << endl;

    cout << "=== Belief state factors ===" << endl;
    unsigned t = 1;
    for (auto const& pf : states) {
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
}
