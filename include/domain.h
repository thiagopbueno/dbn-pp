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

#ifndef _DBN_DOMAIN_H
#define _DBN_DOMAIN_H

#include "variable.h"

#include <vector>
#include <unordered_map>
#include <memory>

namespace dbn {

    class Domain {
    public:
        Domain(std::vector<const Variable*> scope, unsigned width);
        Domain() : _width(0), _size(1) { };

        unsigned width() const { return _width; };
        unsigned size()  const { return _size;  };

        const Variable *operator[](unsigned i) const {
            if (i < _width) return _scope[i];
            else throw "Domain::operator[unsigned i]: Index out of range!";
        }

        unsigned operator[](const Variable* v) const {
            if (in_scope(v)) {
                std::unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find(v->id());
                return it->second;
            }
            else throw "Domain::operator[const Variable*]: Invalid argument!";
        }

        std::vector<const Variable*>::iterator begin() { return _scope.begin(); };
        std::vector<const Variable*>::iterator end() { return _scope.end(); };
        std::vector<const Variable*>::const_iterator begin() const { return _scope.begin(); };
        std::vector<const Variable*>::const_iterator end() const { return _scope.end(); };

        bool in_scope(const Variable* v) const {
            std::unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find(v->id());
            return (it != _var_to_index.end());
        };

        bool in_scope(unsigned id) const {
            std::unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find(id);
            return (it != _var_to_index.end());
        };

        unsigned offset(const Variable *v) const {
            if (in_scope(v)) {
                std::unordered_map<unsigned,unsigned>::const_iterator it = _var_to_index.find(v->id());
                unsigned i = it->second;
                return _offset[i];
            }
            else throw "Domain::offset: Invalid argument!";
        };

        void consistent_instantiation(const std::vector<unsigned> &instantiation, const Domain& domain, std::vector<unsigned> &new_instantiation) const;
        void update_instantiation_with_evidence(std::vector<unsigned> &instantiation, const std::unordered_map<unsigned,unsigned> &evidence) const;

        void next_instantiation(std::vector<unsigned> &instantiation, const std::unordered_map<unsigned,unsigned> &evidence) const;


        unsigned position(std::vector<unsigned> instantiation) const;

        friend std::ostream &operator<<(std::ostream &o, const Domain &v); 

    private:
        std::vector<const Variable*> _scope;
        std::vector<unsigned> _offset;
        unsigned _width;
        unsigned _size;
        std::unordered_map<unsigned, unsigned> _var_to_index;
    };

}

#endif
