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

namespace dbn {

    bool read_next_token(std::string &token) {
        while (std::cin) {
            std::cin >> token;
            if (token[0] != '#') return true;
            std::getline(std::cin, token);  // ignore rest of line
        }
        return false;
    }

    int read_uai_model(unsigned &order, Variable ***variables) {
        std::string token;

        // read file header
        read_next_token(token);
        if (token.compare("BAYES") != 0)  {
            std::cerr << "ERROR! Expected 'BAYES' file header, found: " << token << std::endl;
            return -1;
        }

        // read number of variables
        read_next_token(token);
        order = std::stoi(token);

        // read variables
        *variables = (Variable **) malloc (order * sizeof (Variable *));
        for (unsigned id = 0; id < order; ++id) {
            read_next_token(token);
            (*variables)[id] = new Variable(id, std::stoi(token));
        }

        // read domains
        read_next_token(token);
        unsigned ndomains = std::stoi(token);

        for (unsigned i = 0; i < ndomains; ++i) {

            read_next_token(token);
            unsigned width = std::stoi(token);

            Variable **scope = (Variable **) malloc(width * sizeof (Variable*));
            for (unsigned j = 0; j < width; ++j) {
                read_next_token(token);
                scope[j] = (*variables)[std::stoi(token)];
            }

            Domain *d = new Domain(scope, width);
            std::cout << *d << std::endl;
            delete d;
        }

        return 0;
    }

}
