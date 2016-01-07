# DBN++

Data structures and algorithms in C++ for probabilistic modeling and inference in finite-state dynamic bayesian networks.

Current supported inference algorithms:

* Bucket elimination
* Filtering

Input formats accepted .duai (uai extended model for dynamic nets). See below for more details.

## Usage

```
$ make clean && make
$ ./dbn /path/to/model.duai /path/to/observations.evidence
```
## Input

### uai extended model for finete-state DBNs

The extended uai model for DBNs is inspired by the specification of the [UAI 2014 Inference Competition](http://www.hlt.utdallas.edu/~vgogate/uai14-competition/). The .duai model specification extends the [.uai model format](http://www.hlt.utdallas.edu/~vgogate/uai14-competition/modelformat.html) and the .duai.evid extends the [.uai.evid evidence format](http://www.hlt.utdallas.edu/~vgogate/uai14-competition/evidformat.html).

**Please note these are formats proposed by me for testing-only purposes and are still in development.**

The syntax for the .duai model is the following:

```
# model file with extension .duai

<N> [list of variable sizes]

<P> [list of prior variable ids]

<T> [list of pairs (id_var_slice1, id_var_slice2)]

<S> [list of sensor variable ids]

<FACTOR_0_WIDTH> <ID_PARENT_VAR_FACTOR_0> [list of children variable ids for factor 0]
<FACTOR_1_WIDTH> <ID_PARENT_VAR_FACTOR_1> [list of children variable ids for factor 1]
...
<FACTOR_N_WIDTH> <ID_PARENT_VAR_FACTOR_1> [list of children variable ids for factor N]

<FACTOR_0_SIZE> [list probability values accordingly to the domain linearization of factor 0]
<FACTOR_1_SIZE> [list probability values accordingly to the domain linearization of factor 1]
...
<FACTOR_N_SIZE> [list probability values accordingly to the domain linearization of factor N]

```

where the constants:

* N is the total number of variables
* P is the total number of variables in the prior model
* T is the total number of variables in the transition model
* S is the total numver of variables in the sensor model

Please note that N = T + S and P <= N.
The widht of a factor refers to the cardinality of its scope/domain and its size is the number of possible instantiations of its scope/domain variables.


```
# filtering file with extension .duai.evid
<E> <H>
<ID_VAR_1> [list of H evidence values for variable 1]
<ID_VAR_2> [list of H evidence values for variable 2]
...
<ID_VAR_E> [list of H evidence values for variable E]
<SV> [list of state variables to display filtering trajectory]
```

### Example

```
# data/models/enough-sleep.duai

# Variables
4
2 	#  id=0  name=S  domain={0,1}
2 	#  id=1  name=S' domain={0,1}
2 	#  id=2  name=R  domain={0,1}
2 	#  id=3  name=C  domain={0,1}

# Prior
1 0

# Transition
2 0 1

# Sensor
2 2 3

# Domains
1 0
2 1 0
2 2 0
2 3 0

# Factors
2 0.3 0.7           # Pr(S)
4 0.7 0.2 0.3 0.8   # Pr(S'|S)
4 0.3 0.8 0.7 0.2   # Pr(R|S)
4 0.7 0.9 0.3 0.1   # Pr(C|S)
```

```
# data/models/enough-sleep.duai.evid
# Pr( S(4) | R(1)=0, R(2)=1, R(3)=1, C(1)=0, C(2)=0, C(3)=1 )
2 3
2 0 1 1
3 0 0 1
1 0
```

```
$ ./dbn data/models/enough-sleep.duai data/models/enough-sleep.evidence

>> MODEL: data/models/enough-sleep.duai
=== Variables ===
Variable(id:0, size:2)
Variable(id:1, size:2)
Variable(id:2, size:2)
Variable(id:3, size:2)

=== Factors ===
Factor(Domain{0}, size:2, partition:1, values:[0.300, 0.700])
Factor(Domain{1, 0}, size:4, partition:2, values:[0.700, 0.200, 0.300, 0.800])
Factor(Domain{2, 0}, size:4, partition:2, values:[0.300, 0.800, 0.700, 0.200])
Factor(Domain{3, 0}, size:4, partition:2, values:[0.700, 0.900, 0.300, 0.100])

=== Prior model ===
Variables { 0 }

=== Transition model ===
Variables { 1->0 }

=== Sensor model ===
Variables { 2 3 }

>> FILTERING: data/models/enough-sleep.evidence
=== Belief state factors ===
@ t = 1
observations: { 3:0 2:0 }
Factor(Domain{0}, size:2, partition:1, values:[0.136, 0.864])

@ t = 2
observations: { 3:0 2:1 }
Factor(Domain{0}, size:2, partition:1, values:[0.499, 0.501])

@ t = 3
observations: { 3:1 2:1 }
Factor(Domain{0}, size:2, partition:1, values:[0.896, 0.104])

=== Trajectory ===
0
0 : 0.136 0.499 0.896
1 : 0.864 0.501 0.104
```
