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

#include "operations.h"

#include <vector>
#include <iostream>

namespace dbn {

	Domain *union_of(const Domain& d1, const Domain& d2) {
        std::vector<const Variable* > scope;
        unsigned total_width;

        unsigned width1 = d1.width();
        total_width = width1;

        for (unsigned i = 0; i < width1; ++i) {
            const Variable *v = d1[i];
            scope.push_back(v);
        }

        unsigned width2 = d2.width();
        for (unsigned i = 0; i < width2; ++i) {
            const Variable *v = d2[i];
            if (!d1.in_scope(v)) {
                scope.push_back(v);
                total_width++;
            }
        }

        return new Domain(scope, total_width);
    }

    Domain *union_of(const Domain& d1, const Domain& d2, const Variable* var) {
        std::vector<const Variable* > scope;
        unsigned total_width = 0;

        unsigned width1 = d1.width();

        for (unsigned i = 0; i < width1; ++i) {
            const Variable *v = d1[i];
            if (v->id() != var->id()) {
                scope.push_back(v);
                total_width++;
            }
        }

        unsigned width2 = d2.width();
        for (unsigned i = 0; i < width2; ++i) {
            const Variable *v = d2[i];
            if (var->id() != v->id() && !d1.in_scope(v)) {
                scope.push_back(v);
                total_width++;
            }
        }

        return new Domain(scope, total_width);
    }

    std::vector<unsigned> consistent_instantiation(std::vector<unsigned> inst1, const Domain& d1, const Domain& d2) {
        if (d2.width() == 0) { return std::vector<unsigned>(1, 0); }
        std::vector<unsigned> inst2(d2.width());
        for (unsigned i = 0; i < inst1.size(); ++i) {
            const Variable *v1 = d1[i];
            if (d2.in_scope(v1)) {
                inst2[d2[v1]] = inst1[i];
            }
        }
        return inst2;
    }

	Factor *product(const Factor& f1, const Factor& f2) {

        const Domain &d1 = f1.domain();
        const Domain &d2 = f2.domain();
        Domain *d = union_of(d1, d2);
		Factor *new_factor = new Factor(d, 0.0);

        std::vector<unsigned> counter(new_factor->width(), 0);
        for (unsigned i = 0; i < new_factor->size(); ++i) {

            // find consistent instantiation for both factors
            std::vector<unsigned> inst1 = consistent_instantiation(counter, *d, d1);
            std::vector<unsigned> inst2 = consistent_instantiation(counter, *d, d2);

            // find position in linearization
            unsigned pos1 = d1.position(inst1);
            unsigned pos2 = d2.position(inst2);

            // set product factor value
            (*new_factor)[i] = f1[pos1] * f2[pos2];

            // update counter
            unsigned j;
            for (j = 0; j < counter.size() && counter[j] == 1; ++j) {
                counter[j] = 0;
            }
            if (j < counter.size()) {
                counter[j] = 1;
            }
        }

        return new_factor;
    }

    Factor *sum_product(const Factor& f1, const Factor& f2, const Variable *v) {

        const Domain &d1 = f1.domain();
        const Domain &d2 = f2.domain();
        Domain *d = union_of(d1, d2, v);
        Factor *new_factor = new Factor(d, 0.0);

        std::vector<unsigned> counter(new_factor->width(), 0);
        for (unsigned i = 0; i < new_factor->size(); ++i) {

            // find consistent instantiation for both factors
            std::vector<unsigned> inst1 = consistent_instantiation(counter, *d, d1);
            std::vector<unsigned> inst2 = consistent_instantiation(counter, *d, d2);

            for (unsigned val = 0; val < v->size(); ++val) {

                // update instantiation with variable value
                if (d1.in_scope(v)) { inst1[d1[v]] = val; }
                if (d2.in_scope(v)) { inst2[d2[v]] = val; }

                // find position in linearization
                unsigned pos1 = d1.position(inst1);
                unsigned pos2 = d2.position(inst2);

                // update product factor value
                (*new_factor)[i] += f1[pos1] * f2[pos2];
            }

            // update counter
            unsigned j;
            for (j = 0; j < counter.size() && counter[j] == 1; ++j) {
                counter[j] = 0;
            }
            if (j < counter.size()) {
                counter[j] = 1;
            }
        }

        return new_factor;
    }

}
