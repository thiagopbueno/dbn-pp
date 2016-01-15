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

#ifndef _DBN_ADDFACTOR_
#define _DBN_ADDFACTOR_

#include "factor.h"
#include "domain.h"
#include "variable.h"
#include "cuddObj.hh"

#include <iostream>
#include <set>
#include <unordered_map>

namespace dbn {

	class ADDFactor {
	public:
		ADDFactor(const std::string &output = "T", double value = 1.0);
		ADDFactor(const std::string &output, const Factor &factor);
		ADDFactor(const std::string &output, const ADD &dd, const Domain &domain);
		ADDFactor(const ADDFactor &f);
		ADDFactor(ADDFactor &&f);

		ADDFactor &operator=(ADDFactor &&f);
		ADDFactor operator*(const ADDFactor &f);
		void operator*=(const ADDFactor &f);

		std::string output() const;
		double partition() const;
		const Domain &domain() const { return *_domain; };

		ADDFactor change_variables(std::unordered_map<unsigned,const Variable*> renaming);
		bool in_scope(const Variable *variable) const;

		ADDFactor sum_out(const Variable *variable) const;
		ADDFactor product(const ADDFactor &f) const;
		ADDFactor normalize() const;
		ADDFactor conditioning(const std::unordered_map<unsigned,unsigned> &evidence) const;

		int dump_dot(std::string filename) const;
		friend std::ostream &operator<<(std::ostream& o, const ADDFactor &f);

	private:
		static Cudd mgr;

		ADD _dd;
		std::string _output;
		std::unique_ptr<Domain> _domain;
	};

}
	
#endif