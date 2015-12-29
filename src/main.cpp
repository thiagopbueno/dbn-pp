#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

#include "io.h"
#include "variable.h"
#include "factor.h"
#include "inference.h"

using namespace std;

void print_model(vector<msp::Variable> variables, vector<msp::Factor> factors)
{
    cout << ">> Model: " << variables.size() << " variables, " << factors.size() << " factors. " << endl;
    for (auto v: variables) { cout << v << endl; }
    for (auto f: factors)   { cout << f << endl; }
    cout << endl;
}

bool check_marginal_probability_distribution(msp::Factor factor)
{
    double total = 0.0;
    for (vector<double>::iterator it=factor.begin(); it!=factor.end(); ++it) {
        total += *it;
    }
    return (abs(total - 1.0) < 0.001);
}

void test_variable_elimination(vector<msp::Variable> ordering, vector<msp::Factor> factors)
{
	msp::Factor result = msp::variable_elimination(ordering, factors);
	cout << result << endl;
	assert(check_marginal_probability_distribution(result));
}

int main(int argc, char *argv[])
{
    vector<msp::Variable> variables;
    vector<msp::Factor> factors;

    msp::load_uai_model(variables, factors);
    print_model(variables, factors);

    cout << ">> Variable elimination:" << endl;
    vector<msp::Variable> ordering;
    test_variable_elimination(ordering, factors);
    for (auto v: variables) {
		ordering.push_back(v);
		test_variable_elimination(ordering, factors);
    }

    return 0;
}
