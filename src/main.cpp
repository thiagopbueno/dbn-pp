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

#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <chrono>
#include <algorithm>

#include "addfactor.h"

using namespace std;
using namespace dbn;

void usage(const char *filename);
int read_options(int argc, char *argv[], bool &verbose, bool &m1, bool &m2, bool &m3);

void print_model(
    vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors,
    vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor
);

void print_observations(vector<unordered_map<unsigned,unsigned>> &observations);

template<class T>
void print_trajectory(vector<shared_ptr<T>> &states, set<unsigned> &state_variables);

int main(int argc, char *argv[])
{
    if (argc < 3) {
        usage(argv[0]);
        return -1;
    }

    char *model = argv[1];
    char *evidence = argv[2];

    bool verbose = false;
    bool m1 = false, m2 = false, m3 = false;
    if (read_options(argc, argv, verbose, m1, m2, m3)) return -1;

    unsigned order;
    vector<unique_ptr<Variable>> variables;
    vector<shared_ptr<Factor>> factors;
    vector<shared_ptr<ADDFactor>> addfactors;

    set<unsigned> interface;
    set<unsigned> sensor;
    set<unsigned> prior;
    set<unsigned> internals;

    unordered_map<unsigned,const Variable*> transition;

    // READ MODEL FROM FILE
    if (read_uai_model(model, order, variables, factors, addfactors, prior, interface, sensor, internals, transition)) return -2;

    unsigned nvariables = variables.size();
    unsigned interface_width = transition.size();
    unsigned observation_width = sensor.size();
    unsigned internals_width = internals.size();

    if (verbose) {
        cout << ">> NETWORK: " << model << endl;
        cout << "number of interface variables   = " << interface_width << endl;
        cout << "number of observation variables = " << observation_width << endl;
        cout << "number of internal variables    = " << internals_width << endl;
        cout << "total number of variables       = " << nvariables << endl;
        cout << endl;
        // print_model(variables, factors, prior, transition, sensor);
        // cout << endl;
    }

    // READ EVIDENCE FROM FILE
    vector<unordered_map<unsigned,unsigned>> observations;
    set<unsigned> state_variables;
    if (read_observations(evidence, observations, state_variables)) return -3;
    int T = observations.size();
    if (verbose) {
        cout << ">> OBSERVATIONS: " << evidence << endl;
        cout << "number of timeslices = " << T << endl << endl;
        // print_observations(observations);
        // cout << endl;
    }

    vector<const Variable*> vars;
    for (auto const &v : variables) {
        vars.push_back(v.get());
    }

    // COMPUTE FILTERING
    if (m1) {
        auto start = chrono::steady_clock::now();
        vector<shared_ptr<Factor>> states1 = unrolled_filtering(vars, factors, prior, sensor, internals, transition, observations);
        auto end = chrono::steady_clock::now();
        auto diff = end - start;

        if (verbose) {
            cout << ">> UNROLLED VARIABLE ELIMINATION:" << endl;
            cout << "total time = " << chrono::duration <double, milli> (diff).count() << " ms, ";
            cout << "time per slice = " << chrono::duration <double, milli> (diff).count() / T << " ms." << endl;
            print_trajectory<Factor>(states1, state_variables);
            cout << endl;
        }
        else {
            cout << model << ";";
            cout << 1 << ";";
            cout << T << ";";
            cout << nvariables << ";" << interface_width << ";" << observation_width << ";" << internals_width << ";";
            cout << chrono::duration <double, milli> (diff).count() << ";";
            cout << chrono::duration <double, milli> (diff).count() / T << endl;
        }
    }

    if (m2) {
        auto start = chrono::steady_clock::now();
        vector<shared_ptr<Factor>> states2 = filtering(vars, factors, prior, sensor, internals, transition, observations);
        auto end = chrono::steady_clock::now();
        auto diff = end - start;

        if (verbose) {
            cout << ">> INTERFACE:" << endl;
            cout << "total time = " << chrono::duration <double, milli> (diff).count() << " ms, ";
            cout << "time per slice = " << chrono::duration <double, milli> (diff).count() / T << " ms." << endl;
            print_trajectory<Factor>(states2, state_variables);
            cout << endl;
        }
        else {
            cout << model << ";";
            cout << 2 << ";";
            cout << T << ";";
            cout << nvariables << ";" << interface_width << ";" << observation_width << ";" << internals_width << ";";
            cout << chrono::duration <double, milli> (diff).count() << ";";
            cout << chrono::duration <double, milli> (diff).count() / T << endl;
        }
    }

    if (m3) {
        auto start = chrono::steady_clock::now();
        vector<shared_ptr<ADDFactor>> states3 = filtering(vars, addfactors, prior, sensor, internals, transition, observations);
        auto end = chrono::steady_clock::now();
        auto diff = end - start;

        if (verbose) {
            cout << ">> INTERFACE with ADDs:" << endl;
            cout << "total time = " << chrono::duration <double, milli> (diff).count() << " ms, ";
            cout << "time per slice = " << chrono::duration <double, milli> (diff).count() / T << " ms." << endl;
            print_trajectory<ADDFactor>(states3, state_variables);
            cout << endl;
        }
        else {
            cout << model << ";";
            cout << 3 << ";";
            cout << T << ";";
            cout << nvariables << ";" << interface_width << ";" << observation_width << ";" << internals_width << ";";
            cout << chrono::duration <double, milli> (diff).count() << ";";
            cout << chrono::duration <double, milli> (diff).count() / T << endl;
        }
    }

    return 0;
}

void
usage(const char *filename)
{
    string usage = "Usage: " + string(filename) + " /path/to/model.duai /path/to/observations.duai.evid [OPTIONS]";
    cout << usage << endl << endl;

    cout << "Filtering methods (-m option):" << endl;
    cout << "(1) variable elimination in unrolled network" << endl;
    cout << "(2) interface algorithm" << endl;
    cout << "(3) interface algorithm with ADDs" << endl;
    cout << endl;

    cout << "OPTIONS:" << endl;
    cout << "-m filtering method (1|2|3)" << endl;
    cout << "-v verbose" << endl;
}

int
read_options(int argc, char *argv[], bool &verbose, bool &m1, bool &m2, bool &m3)
{
    if (argc >= 4) {
        for (int i = 3; i < argc; ++i) {
            string option(argv[i]);
            if (option == "-v") verbose = true;
            else if (option == "-m") {
                char *m = argv[i+1];
                for (unsigned j = 0; j < strlen(m); ++j) {
                    switch (m[j]) {
                        case '1': m1 = true; break;
                        case '2': m2 = true; break;
                        case '3': m3 = true; break;
                        default:
                            cerr << "Error: wrong method option " << m << endl;
                            return -1;
                    }
                }
            }
        }
    }
    return 0;
}

void
print_model(
    vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors,
    vector<unsigned> &prior, unordered_map<unsigned,const Variable*> &transition, vector<unsigned> &sensor)
{
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

void
print_observations(vector<unordered_map<unsigned,unsigned>> &observations)
{
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
void
print_trajectory(vector<shared_ptr<T>> &states, set<unsigned> &state_variables)
{

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

    vector<const Variable*> scope = states[0]->domain().scope();
    sort(scope.begin(), scope.end(), [](const Variable *v1, const Variable *v2) { return v1->id() < v2->id(); });

    const Domain domain(scope);
    for (unsigned i = 0; i < domain.width(); ++i) {
        cout << domain[i]->id() << " ";
    }
    cout << endl;

    cout.precision(3);
    cout << fixed;

    std::vector<unsigned> inst(domain.width(), 0);
    for (unsigned i = 0; i < domain.size(); ++i) {

        // print instantiation
        for (auto d : inst) {
            cout << d << " ";
        }
        cout << ":";

        // print value trajectory
        for (auto const& pf : states) {
            const Domain &d = pf->domain();
            unsigned w = d.width();
            std::vector<unsigned> inst2(w, 0);

            for (unsigned j = 0; j < w; ++j) {
                const Variable *v = d[j];
                inst2[j] = inst[domain[v]];
            }

            cout << " " << (*pf)[inst2];
        }
        cout << endl;

        // update instantiation
        domain.next_instantiation(inst);
    }
    cout << endl;
}
