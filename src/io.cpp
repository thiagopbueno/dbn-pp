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
#include "domain.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <memory>

using namespace std;

namespace dbn {

    bool read_next_token(ifstream &input_file, string &token) {
        while (input_file) {
            input_file >> token;
            if (token[0] != '#') return true;
            getline(input_file, token);  // ignore rest of line
        }
        return false;
    }

    bool read_next_integer(ifstream &input_file, unsigned &i) {
        string token;
        if (!read_next_token(input_file, token)) return false;
        i = stoi(token);
        return true;
    }

    bool read_next_double(ifstream &input_file, double &d) {
        string token;
        if (!read_next_token(input_file, token)) return false;
        d = stod(token);
        return true;
    }

    string read_file_header(ifstream &input_file) {
        string token;
        read_next_token(input_file, token);
        if (token.compare("DBAYES") != 0)  {
            cerr << "ERROR! Expected 'DBAYES' file header, found: " << token << endl;
        }
        return token;
    }

    void read_variables(ifstream &input_file, unsigned &order, vector<unique_ptr<Variable>> &variables) {
        read_next_integer(input_file, order);

        unsigned sz = 0;
        for (unsigned id = 0; id < order; ++id) {
            read_next_integer(input_file, sz);
            variables.emplace_back(new Variable(id, sz));
        }
    }

    void read_prior_model(ifstream &input_file, unsigned &prior_order, vector<unsigned> &prior) {
        read_next_integer(input_file, prior_order);
        unsigned v;
        for (unsigned i = 0; i < prior_order; ++i) {
            read_next_integer(input_file, v);
            prior.push_back(v);
        }
    }

    void read_transition_model(ifstream &input_file, unsigned &transition_order, unordered_map<unsigned,const Variable*> &transition, const vector<unique_ptr<Variable>> &variables) {
        read_next_integer(input_file, transition_order);
        unsigned curr, next;
        for (unsigned i = 0; i < transition_order/2; ++i) {
            read_next_integer(input_file, curr);
            read_next_integer(input_file, next);
            transition[next] = variables[curr].get();
        }
    }

    void read_sensor_model(ifstream &input_file, unsigned &sensor_order, vector<unsigned> &sensor) {
        read_next_integer(input_file, sensor_order);
        unsigned v;
        for (unsigned i = 0; i < sensor_order; ++i) {
            read_next_integer(input_file, v);
            sensor.push_back(v);
        }
    }

    void read_factors(ifstream &input_file, unsigned order, vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors) {
        unsigned width, id;
        for (unsigned i = 0; i < order; ++i) {
            read_next_integer(input_file, width);

            vector<const Variable*> scope;
            for (unsigned j = 0; j < width; ++j) {
                read_next_integer(input_file, id);
                scope.push_back(variables[id].get());
            }

            factors.emplace_back(new Factor(new Domain(scope)));
        }

        for (unsigned i = 0; i < order; ++i) {
            unsigned factor_size;
            read_next_integer(input_file, factor_size);

            double partition = 0;
            for (unsigned j = 0; j < factor_size; ++j) {
                double value;
                read_next_double(input_file, value);
                (*(factors[i]))[j] = value;
                partition += value;
            }
            factors[i]->partition(partition);
        }
    }

    void read_addfactors(vector<shared_ptr<Factor>> &factors, vector<shared_ptr<ADDFactor>> &addfactors) {
        for (auto &f : factors) {
            const Domain &domain = f->domain();
            unsigned id = domain[(unsigned)0]->id();
            string output = to_string(id);
            ADDFactor *addf = new ADDFactor(output, *f);
            addfactors.emplace_back(addf);
        }
    }

    int read_uai_model(
        const char *filename,
        unsigned &order,
        vector<unique_ptr<Variable>> &variables,
        vector<shared_ptr<Factor>> &factors,
        vector<shared_ptr<ADDFactor>> &addfactors,
        vector<unsigned> &prior,
        unordered_map<unsigned,const Variable*> &transition,
        vector<unsigned> &sensor) {

        ifstream input_file(filename);
        if (input_file.is_open()) {
            read_file_header(input_file);
            read_variables(input_file, order, variables);
            unsigned prior_order, transition_order, sensor_order;
            read_prior_model(input_file, prior_order, prior);
            read_transition_model(input_file, transition_order, transition, variables);
            read_sensor_model(input_file, sensor_order, sensor);
            read_factors(input_file, order, variables, factors);
            read_addfactors(factors, addfactors);
            input_file.close();
            return 0;
        }
        else {
            cerr << "Error: couldn't read file " << filename << endl;
            return -1;
        }
    }


    int read_observations(
        const char *filename,
        vector<unordered_map<unsigned,unsigned>> &observations,
        set<unsigned> &state_variables) {

        ifstream input_file(filename);
        if (input_file.is_open()) {
            unsigned length, width;

            read_next_integer(input_file, width);
            read_next_integer(input_file, length);

            for (unsigned t = 0; t < length; ++t) {
                observations.emplace_back();
            }

            for (unsigned i = 0; i < width; ++i) {
                unsigned id;
                read_next_integer(input_file, id);
                for (unsigned t = 0; t < length; ++t) {
                    unsigned evidence;
                    read_next_integer(input_file, evidence);
                    observations[t][id] = evidence;
                }
            }

            unsigned state_width;
            read_next_integer(input_file, state_width);
            for (unsigned i = 0; i < state_width; ++i) {
                unsigned id;
                read_next_integer(input_file, id);
                state_variables.insert(id);
            }

            input_file.close();
            return 0;
        }
        else {
            cerr << "Error: couldn't read file " << filename << endl;
            return -1;
        }
    }
}
