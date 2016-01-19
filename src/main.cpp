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
#include "inference.h"

#include "stdio.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <chrono>

#include "addfactor.h"

using namespace std;
using namespace dbn;

void print_model(
    vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors,
    vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor
);

void print_observations(vector<unordered_map<unsigned,unsigned>> &observations);

template<class T>
void print_trajectory(vector<shared_ptr<T>> &states, set<unsigned> &state_variables);

void usage(const char *filename)
{
    cout << "Usage: " << filename << " /path/to/model.duai /path/to/observations.duai.evid";
    cout << " [OPTIONS]" << endl << endl;
    cout << "OPTIONS:" << endl;
    cout << "-v verbose" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    bool verbose = false;
    if (argc == 4) {
        string options(argv[3]);
        if (options == "-v") verbose = true;
    }

    unsigned order;
    vector<unique_ptr<Variable>> variables;
    vector<shared_ptr<Factor>> factors;
    vector<shared_ptr<ADDFactor>> addfactors;

    vector<unsigned> prior;
    unordered_map<unsigned,const Variable*> transition;
    vector<unsigned> sensor;

    if (read_uai_model(argv[1], order, variables, factors, addfactors, prior, transition, sensor)) return -1;
    cout << ">> NETWORK: " << argv[1] << endl;
    if (verbose) {
        print_model(variables, factors, prior, transition, sensor);
    }

    vector<unordered_map<unsigned,unsigned>> observations;
    set<unsigned> state_variables;
    if (read_observations(argv[2], observations, state_variables)) return -2;
    int T = observations.size();

    cout << ">> OBSERVATIONS: " << argv[2] << endl;
    cout << "timeslices = " << T << endl;
    if (verbose) {
        print_observations(observations);
    }

    cout << ">> FILTERING: " << endl;
    cout << "@ Unrolled filtering:" << endl;
    auto start = chrono::steady_clock::now();
    vector<shared_ptr<Factor>> states1 = unrolled_filtering(variables, factors, prior, transition, sensor, observations);
    auto end = chrono::steady_clock::now();
    auto diff = end - start;
    cout << "total time = " << chrono::duration <double, milli> (diff).count() << " ms, ";
    cout << "time per slice = " << chrono::duration <double, milli> (diff).count() / T << " ms." << endl;
    if (verbose) {
        print_trajectory<Factor>(states1, state_variables);
    }

    cout << "@ Forward filtering:" << endl;
    start = chrono::steady_clock::now();
    vector<shared_ptr<Factor>> states2 = filtering(factors, prior, transition, sensor, observations);
    end = chrono::steady_clock::now();
    diff = end - start;
    cout << "total time = " << chrono::duration <double, milli> (diff).count() << " ms, ";
    cout << "time per slice = " << chrono::duration <double, milli> (diff).count() / T << " ms." << endl;
    if (verbose) {
        print_trajectory<Factor>(states2, state_variables);
    }

    cout << "@ Forward ADD filtering:" << endl;
    start = chrono::steady_clock::now();
    vector<shared_ptr<ADDFactor>> states3 = filtering(addfactors, prior, transition, sensor, observations);
    end = chrono::steady_clock::now();
    diff = end - start;
    cout << "total time = " << chrono::duration <double, milli> (diff).count() << " ms, ";
    cout << "time per slice = " << chrono::duration <double, milli> (diff).count() / T << " ms." << endl;
    if (verbose) {
        print_trajectory<ADDFactor>(states3, state_variables);
    }

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

template<class T>
void print_trajectory(vector<shared_ptr<T>> &states, set<unsigned> &state_variables) {

    unsigned timeslices = states.size();
    for (unsigned i = 0; i < timeslices; ++i) {
        T f = *(states[i]);

        const Domain &domain = states[i]->domain();
        unsigned width = domain.width();
        for (unsigned j = 0; j < width; ++j) {
            const Variable *v = domain[j];
            if (state_variables.find(v->id()) == state_variables.end()) {
                f = f.sum_out(v);
            }
        }
        states[i] = make_shared<T>(f);
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
