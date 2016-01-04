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
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;

namespace dbn {

    bool read_next_token(string &token) {
        while (cin) {
            cin >> token;
            if (token[0] != '#') return true;
            getline(cin, token);  // ignore rest of line
        }
        return false;
    }

    bool read_next_integer(unsigned &i) {
        string token;
        if (!read_next_token(token)) return false;
        i = stoi(token);
        return true;
    }

    bool read_next_double(double &d) {
        string token;
        if (!read_next_token(token)) return false;
        d = stod(token);
        return true;
    }

    string read_file_header() {
        string token;
        read_next_token(token);
        if (token.compare("DBAYES") != 0)  {
            cerr << "ERROR! Expected 'DBAYES' file header, found: " << token << endl;
        }
        return token;
    }

    void read_variables(unsigned &order, vector<unique_ptr<Variable>> &variables) {
        read_next_integer(order);

        unsigned sz;
        for (unsigned id = 0; id < order; ++id) {
            read_next_integer(sz);
            variables.emplace_back(new Variable(id, sz));
        }
    }

    void read_transition_model(unsigned &transition_order, unordered_map<unsigned, unsigned> &transition) {
        read_next_integer(transition_order);
        for (unsigned i = 0; i < transition_order/2; ++i) {
            unsigned curr, next;
            read_next_integer(curr);
            read_next_integer(next);
            transition[next] = curr;
        }
    }

    void read_sensor_model(unsigned &sensor_order, vector<unsigned> &sensor) {
        read_next_integer(sensor_order);
        for (unsigned i = 0; i < sensor_order; ++i) {
            unsigned v;
            read_next_integer(v);
            sensor.push_back(v);
        }
    }

    void read_factors(unsigned order, vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors) {
        unsigned width, id;
        for (unsigned i = 0; i < order; ++i) {
            read_next_integer(width);

            vector<const Variable*> scope;
            for (unsigned j = 0; j < width; ++j) {
                read_next_integer(id);
                scope.push_back(variables[id].get());
            }

            factors.emplace_back(new Factor(new Domain(scope, width)));
        }

        unsigned factor_size;
        double value;
        for (unsigned i = 0; i < order; ++i) {
            read_next_integer(factor_size);

            for (unsigned j = 0; j < factor_size; ++j) {
                read_next_double(value);
                (*(factors[i]))[j] = value;
            }
        }
    }

    int read_uai_model(unsigned &order, vector<unique_ptr<Variable>> &variables, vector<shared_ptr<Factor>> &factors, unordered_map<unsigned, unsigned> &transition, vector<unsigned> &sensor) {
        read_file_header();
        read_variables(order, variables);
        unsigned transition_order, sensor_order;
        read_transition_model(transition_order, transition);
        read_sensor_model(sensor_order, sensor);
        read_factors(order, variables, factors);
        return 0;
    }

}
