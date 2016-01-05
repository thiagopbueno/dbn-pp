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

using namespace std;

namespace dbn {

	Domain *union_of(const Domain& d1, const Domain& d2) {
        vector<const Variable*> scope;

        unsigned width1 = d1.width();

        for (unsigned i = 0; i < width1; ++i) {
            const Variable *v = d1[i];
            scope.push_back(v);
        }

        unsigned width2 = d2.width();
        for (unsigned i = 0; i < width2; ++i) {
            const Variable *v = d2[i];
            if (!d1.in_scope(v)) {
                scope.push_back(v);
            }
        }

        return new Domain(scope);
    }

    Domain *union_of(const Domain& d1, const Domain& d2, const Variable* var) {
        vector<const Variable*> scope;

        unsigned width1 = d1.width();
        for (unsigned i = 0; i < width1; ++i) {
            const Variable *v = d1[i];
            if (v->id() != var->id()) {
                scope.push_back(v);
            }
        }

        unsigned width2 = d2.width();
        for (unsigned i = 0; i < width2; ++i) {
            const Variable *v = d2[i];
            if (var->id() != v->id() && !d1.in_scope(v)) {
                scope.push_back(v);
            }
        }

        return new Domain(scope);
    }

	Factor *product(const Factor& f1, const Factor& f2) {

        const Domain &d1 = f1.domain();
        const Domain &d2 = f2.domain();
        Domain *d = union_of(d1, d2);
		Factor *new_factor = new Factor(d, 0.0);

        vector<unsigned> instantiation(new_factor->width(), 0);

        double partition = 0;
        for (unsigned i = 0; i < new_factor->size(); ++i) {

            // find position in linearization of consistent instantiation
            unsigned pos1 = d1.position_consistent_instantiation(instantiation, *d);
            unsigned pos2 = d2.position_consistent_instantiation(instantiation, *d);

            // set product factor value
            double value = f1[pos1] * f2[pos2];
            (*new_factor)[i] = value;
            partition += value;

            // find next instantiation
            d->next_instantiation(instantiation);
        }
        new_factor->partition() = partition;

        return new_factor;
    }

    Factor *sum_product(const Factor& f1, const Factor& f2, const Variable *v) {

        const Domain &d1 = f1.domain();
        const Domain &d2 = f2.domain();
        Domain *d = union_of(d1, d2, v);
        Factor *new_factor = new Factor(d, 0.0);

        vector<unsigned> instantiation(new_factor->width(), 0);

        double partition = 0;
        for (unsigned i = 0; i < new_factor->size(); ++i) {

            for (unsigned val = 0; val < v->size(); ++val) {

                // find position in linearization
                unsigned pos1 = d1.position_consistent_instantiation(instantiation, *d, v, val);
                unsigned pos2 = d2.position_consistent_instantiation(instantiation, *d, v, val);

                // update product factor value
                double value = f1[pos1] * f2[pos2];
                (*new_factor)[i] += value;
                partition += value;
            }

            // find next instantiation
            d->next_instantiation(instantiation);
        }
        new_factor->partition() = partition;

        return new_factor;
    }

    Factor *conditioning(const Factor &f, const unordered_map<unsigned,unsigned> &evidence) {
        Factor *new_factor = new Factor(new Domain(f.domain(), evidence));

        const Domain &d = f.domain();

        // incorporate evidence in instantiation
        vector<unsigned> instantiation(f.width(), 0);
        d.update_instantiation_with_evidence(instantiation, evidence);

        double partition = 0;
        for (unsigned i = 0; i < new_factor->size(); ++i) {

            // update new factor
            unsigned pos = d.position_instantiation(instantiation);
            double value = f[pos];
            (*new_factor)[i] = value;
            partition += value;

            // find next instantiation
            d.next_instantiation(instantiation, evidence);
        }
        new_factor->partition() = partition;

        return new_factor;
    }

    Factor *normalization(const Factor &f) {
        return new Factor(f, true);
    }

}
