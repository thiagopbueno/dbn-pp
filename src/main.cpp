#include <iostream>
#include <vector>

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

int main(int argc, char *argv[])
{
    vector<msp::Variable> variables;
    vector<msp::Factor> factors;

    msp::load_uai_model(variables, factors);
    print_model(variables, factors);

    cout << ">> Variable elimination:" << endl;
    cout << msp::variable_elimination(variables, factors) << endl;

	return 0;
}
