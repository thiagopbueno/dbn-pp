// Copyright (c) 2014 Denis Maua
// All Rights Reserved.
//
// This file is part of MSP library
//
// MSP is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MSP is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with MSP.  If not, see <http://www.gnu.org/licenses/>.


// INPUT-OUTPUT ROUTINES

#include <iostream>
#include <string>
#include <fstream>
#include <istream>
#include <sstream>
#include <vector>
#include "io.h"
#include "variable.h"
#include "domain.h"
#include "factor.h"

namespace msp {

  /** Load model from file in UAI format. Read input from stdin --
   * redirect cin to read from file.
   * @param variables vector of pointer to variables
   * @param factors vector of pointer to factors
   */
  void load_uai_model(  std::vector<Variable >& variables, std::vector<Factor > & factors )
  {

    std::string str;
    double num;
    int i,j,k,l;
    int nvars, nfactors;                // number of variables, of factors

    // ensure vectors are initially empty
    variables.clear(); factors.clear();

    // cin is set accordingly
    std::cin >> str; // header
    while (str[0] == '#') {        // skip comments 
      // ignore rest of this line
      std::getline(std::cin, str);
      // get first word in next line
      std::cin >> str; 
    }
    if (str.compare("BAYES") != 0 && str.compare("MARKOV") != 0) 
      {
	std::cerr << "ERROR! Expected 'MARKOV' or 'BAYES' file header, found: " << str << std::endl;
	throw;
      }
    // number of variables
    std::cin >> nvars;
    variables.reserve(nvars); // allocate memory
    for (i = 0; i < nvars; ++i) 
      {
	std::cin >> j;
	variables.emplace_back(j);
      }
    // number of factors
    std::cin >> nfactors;
    factors.reserve(nfactors); // allocate memory
    // domains
    for (i=0; i < nfactors; ++i)
      {
	std::cin >> j; // size of factor scope
	std::vector<Variable* > scope;
	scope.resize(j);
	for (k=0; k < j; ++k)
	  {
	    std::cin >> l;
	    if (l < 0 || l >= nvars)
	      {
		std::cerr << "ERROR! Variable index is outside allowed range: " << l << " <> [0," << nvars << "]" << std::endl;
		throw;
	      }
	    // reverse variable ordering to conform with UAI format (in which rightmost variable in domains is the least significant)
	    scope[j-k-1] = &(variables[l]);
	  }
	factors.emplace_back(scope); // instantiate new factor and append it to factor vector
      }
    // factors
    for (i=0; i < nfactors; ++i)
      {
	std::cin >> j; // size of factor scope
	if (j != factors[i].size())
	  {
	    std::cerr << "ERROR! Factor size does not match previously defined domain size: " << j << " <> " << factors[i].size() << std::endl;
	    throw;
	  }
	for (k=0; k < j; ++k)
	  {
	    std::cin >> num;
	    factors[i].set(k,num);
	  }
      }
    variables.shrink_to_fit(); factors.shrink_to_fit();
  }

}
