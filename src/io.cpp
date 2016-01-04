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
#include <memory>

namespace dbn {

    bool read_next_token(std::string &token) {
        while (std::cin) {
            std::cin >> token;
            if (token[0] != '#') return true;
            std::getline(std::cin, token);  // ignore rest of line
        }
        return false;
    }

    bool read_next_integer(unsigned &i) {
        std::string token;
        if (!read_next_token(token)) return false;
        i = std::stoi(token);
        return true;
    }

    bool read_next_double(double &d) {
        std::string token;
        if (!read_next_token(token)) return false;
        d = std::stod(token);
        return true;
    }

    std::string read_file_header() {
        std::string token;
        read_next_token(token);
        if (token.compare("BAYES") != 0)  {
            std::cerr << "ERROR! Expected 'BAYES' file header, found: " << token << std::endl;
        }
        return token;
    }

    void read_variables(unsigned &order, std::vector<std::unique_ptr<Variable>> &variables) {
        read_next_integer(order);

        unsigned sz;
        for (unsigned id = 0; id < order; ++id) {
            read_next_integer(sz);
            variables.emplace_back(new Variable(id, sz));
        }
    }

    void read_factors(unsigned order, std::vector<std::unique_ptr<Variable>> &variables, std::vector<std::shared_ptr<Factor>> &factors) {
        unsigned width, id;
        for (unsigned i = 0; i < order; ++i) {
            read_next_integer(width);

            std::vector<const Variable*> scope;
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

    int read_uai_model(unsigned &order, std::vector<std::unique_ptr<Variable>> &variables, std::vector<std::shared_ptr<Factor>> &factors) {
        read_file_header();
        read_variables(order, variables);
        read_factors(order, variables, factors);
        return 0;
    }

}
