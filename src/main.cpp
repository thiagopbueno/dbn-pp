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

#include "stdio.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>

#include "addfactor.h"

using namespace std;
using namespace dbn;

void print_model(
    vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors,
    vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor
);

void print_observations(vector<unordered_map<unsigned,unsigned>> &observations);

void print_trajectory(vector<shared_ptr<Factor>> &states, set<unsigned> &state_variables);

void print_test_add(vector<shared_ptr<Factor>> &factors, vector<unique_ptr<Variable>> &variables)
{
    ADDFactor distribution;

    for (auto &f : factors) {
        const Domain &domain = f->domain();
        unsigned id = domain[(unsigned)0]->id();

        string output = to_string(id);
        ADDFactor addf(output, *f);
        cout << addf << endl;

        // addf = addf.sum_out(variables[0].get());
        // cout << addf << endl;

        // addf = addf.normalize();
        // cout << addf << endl;

        // string filename = addf.output() + ".dot";
        // addf.dump_dot(filename);

        // distribution = distribution.product(addf);
        // distribution = distribution * addf;
        distribution *= addf;
    }

    string filename = "distribution.dot";
    distribution.dump_dot(filename);
    cout << distribution << endl;

    unordered_map<unsigned, unsigned> evidence;
    evidence[2] = 0;
    evidence[3] = 0;
    distribution = distribution.conditioning(evidence);
    distribution = distribution.sum_out(variables[0].get());
    distribution = distribution.normalize();
    cout << distribution << endl;
}

void usage(const char *filename)
{
    cout << "Usage: " << filename << " /path/to/model.duai /path/to/observations.duai.evid" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    unsigned order;
    vector<unique_ptr<Variable>> variables;
    vector<shared_ptr<Factor>> factors;
    vector<shared_ptr<ADDFactor>> addfactors;

    vector<unsigned> prior;
    unordered_map<unsigned,const Variable*> transition;
    vector<unsigned> sensor;

    if (read_uai_model(argv[1], order, variables, factors, addfactors, prior, transition, sensor)) return -1;
    cout << ">> MODEL: " << argv[1] << endl;
    print_model(variables, factors, prior, transition, sensor);

    // print_test_add(factors, variables);

    vector<unordered_map<unsigned,unsigned>> observations;
    set<unsigned> state_variables;
    if (read_observations(argv[2], observations, state_variables)) return -2;

    cout << ">> FILTERING: " << argv[2] << endl;
    print_observations(observations);

    cout << "Forward filtering:" << endl;
    vector<shared_ptr<Factor>> states = filtering(factors, prior, transition, sensor, observations);
    print_trajectory(states, state_variables);

    cout << "Forward ADD filtering:" << endl;
    vector<shared_ptr<ADDFactor>> states2 = filtering(addfactors, prior, transition, sensor, observations);
    int t = 1;
    for (auto pf : states2) {
        cout << "@ t = " << t++ << endl;
        cout << *pf << endl;
    }
    cout << endl;

    cout << "Unrolled filtering:" << endl;
    vector<shared_ptr<Factor>> states3 = unrolled_filtering(variables, factors, prior, transition, sensor, observations);
    print_trajectory(states3, state_variables);

    return 0;
}

void print_model(
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

    cout << "=== 2TBN ===" << endl;
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

void print_observations(vector<unordered_map<unsigned,unsigned>> &observations) {
    cout << "=== Observations ===" << endl;
    cout.precision(3);
    cout << fixed;
    unsigned T = observations.size();
    for (unsigned t = 1; t <= T; ++t) {
        cout << "@t = " << t << " {";
        for (auto it_evidence : observations[t-1]) {
            unsigned id = it_evidence.first;
            unsigned value = it_evidence.second;
            cout << " " << id << ":" << value;
        }
        cout << " }" << endl;
    }
    cout << endl;
}

void print_trajectory(vector<shared_ptr<Factor>> &states, set<unsigned> &state_variables) {

    // project factors over state variables
    const Domain &domain = states[0]->domain();
    vector<const Variable*> ordering;
    for (unsigned i = 0; i < domain.width(); ++i) {
        const Variable *v = domain[i];
        if (state_variables.find(v->id()) == state_variables.end()) {
            ordering.push_back(v);
        }
    }
    vector<shared_ptr<Factor>> factors;
    for (unsigned i = 0; i < states.size(); ++i) {
        factors.clear();
        factors.push_back(states[i]);
        states[i] = variable_elimination(ordering, factors);
    }

    cout << "=== Trajectory ===" << endl;
    const Domain &new_domain = states[0]->domain();
    for (unsigned i = 0; i < new_domain.width(); ++i) {
        cout << new_domain[i]->id() << " ";
    }
    cout << endl;

    cout.precision(3);
    cout << fixed;

    std::vector<unsigned> inst(new_domain.width(), 0);
    for (unsigned i = 0; i < new_domain.size(); ++i) {

        // print instantiation
        for (auto d : inst) {
            cout << d << " ";
        }
        cout << ":";

        // print value trajectory
        for (auto const& pf : states) {
            cout << " " << (*pf)[inst];
        }
        cout << endl;

        // update instantiation
        new_domain.next_instantiation(inst);
    }
    cout << endl;
}
