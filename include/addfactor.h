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

namespace dbn {

	class ADDFactor {
	public:
		ADDFactor(Cudd &mgr, const std::string &output, double value);
		ADDFactor(Cudd &mgr, const std::string &output, const Factor &factor);
		ADDFactor(Cudd &mgr, const std::string &output, const ADD &dd, std::set<const Variable*> scope);

		std::string output() const;

		bool in_scope(const Variable *variable);

		ADDFactor sum_out(const Variable *variable);

		int dump_dot(std::string filename);
		friend std::ostream &operator<<(std::ostream& o, const ADDFactor &f);

	private:
		Cudd &_mgr;
		ADD _dd;
		std::string _output;
		std::set<const Variable*> _scope;
	};

}
	
#endif