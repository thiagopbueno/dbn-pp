# DBN++

Data structures and algorithms in C++ for **probabilistic modeling and inference** in (discrete-time) finite-state Dynamic Bayesian Networks (DBNs).

Currently supported inference tasks: filtering.

Algorithms implemented in this version:

* Variable elimination in unrolled network
* Forward algorithm
* Forward algorithm with ADD (Algebraic Decision Diagrams)

The overall structure used for variable, factor and domain representation is highly inspired by the [kpu-pp project](https://github.com/denismaua/kpu-pp).

Inputs are accepted in the format .duai (uai extended model for dynamic nets). See below for more details.

## Installation

Follow the instructions to [install CUDD package](http://vlsi.colorado.edu/~fabio/CUDD/cudd.pdf) in your platform and set the CUDD variable in Makefile to point to your library's installation location. Then enter the following comands in your prompt.

```
$ git clone https://github.com/thiagopbueno/dbn.git
$ make install
$ make
$ ./dbn
```

## Usage

```
$ ./dbn /path/to/model.duai /path/to/observations.duai.evid [OPTIONS]

OPTIONS:
-v verbose
```

## Input

### uai extended specification for finite-state DBNs

The extended uai model for DBNs is inspired by the specification of the [UAI 2014 Inference Competition](http://www.hlt.utdallas.edu/~vgogate/uai14-competition/). The .duai model specification extends the [.uai model format](http://www.hlt.utdallas.edu/~vgogate/uai14-competition/modelformat.html) and the .duai.evid extends the [.uai.evid evidence format](http://www.hlt.utdallas.edu/~vgogate/uai14-competition/evidformat.html).

*Please note these formats are proposed by me for testing-only purposes and are still in development.*

#### File .duai model specification

The syntax for the .duai model is the following:

```
# model file with extension .duai

DBAYES

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

where the constants are such that:

* N is the total number of variables
* P is the total number of variables in the prior model
* T is the total number of variables in the transition model
* S is the total numver of variables in the sensor model

Please note that N = T + S and P <= N.
The width of a factor refers to the cardinality of its scope/domain and its size is the number of possible instantiations of its scope/domain variables.

Comments are allowed anywhere and start with '#' sign and go until the end of the line. Whitespaces are ignored.

#### File .duai.evid evidence specification

The syntax for the .duai.evid evidence format is the following:

```
# evidence file with extension .duai.evid
<E> <H>
<ID_VAR_1> [list of H evidence values for variable 1]
<ID_VAR_2> [list of H evidence values for variable 2]
...
<ID_VAR_E> [list of H evidence values for variable E]
<SV> [list of state variables to display filtering trajectory]
```

where the constants are such that:

* E is the total number of evidence variables
* H is the horizon size (number of timesteps) for filtering
* SV is the total number of state variables of interest (if this is less than T/2, variable elimination is performed)

Comments are allowed anywhere and start with '#' sign and go until the end of the line. Whitespaces are ignored.

### Example

The following example Enough Sleep Student Problem is extracted from exercices 15.13 and 15.14 of the textbook Artificial Intelligence: A Modern Approach - 3rd Edition, by Stuart Russel, Peter Norvig.

```
# data/models/enough-sleep.duai

DBAYES

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
$ ./dbn data/models/enough-sleep.duai data/models/enough-sleep.duai.evid -v

>> NETWORK: data/models/enough-sleep.duai
=== Variables ===
Variable(id:0, size:2)
Variable(id:1, size:2)
Variable(id:2, size:2)
Variable(id:3, size:2)

=== Factors ===
Factor(width = 1, size = 2, partition = 1)
0
0 : 0.3
1 : 0.7

Factor(width = 2, size = 4, partition = 2)
1 0
0 0 : 0.7
0 1 : 0.2
1 0 : 0.3
1 1 : 0.8

Factor(width = 2, size = 4, partition = 2)
2 0
0 0 : 0.3
0 1 : 0.8
1 0 : 0.7
1 1 : 0.2

Factor(width = 2, size = 4, partition = 2)
3 0
0 0 : 0.7
0 1 : 0.9
1 0 : 0.3
1 1 : 0.1

=== Prior model ===
Variables { 0 }

=== 2TBN ===
Variables { 1->0 }

=== Sensor model ===
Variables { 2 3 }

>> OBSERVATIONS: data/models/enough-sleep.duai.evid
timeslices = 7
=== Observations ===
@t = 1 { 3:0 2:0 }
@t = 2 { 3:0 2:1 }
@t = 3 { 3:1 2:1 }
@t = 4 { 3:0 2:0 }
@t = 5 { 3:0 2:0 }
@t = 6 { 3:0 2:1 }
@t = 7 { 3:1 2:0 }

>> FILTERING:
@ Unrolled filtering:
total time = 0.850 ms, time per slice = 0.121 ms.
=== Trajectory ===
0
0 : 0.136 0.499 0.896 0.349 0.149 0.507 0.483
1 : 0.864 0.501 0.104 0.651 0.851 0.493 0.517

@ Forward filtering:
total time = 0.246 ms, time per slice = 0.035 ms.
=== Trajectory ===
0
0 : 0.136 0.499 0.896 0.349 0.149 0.507 0.483
1 : 0.864 0.501 0.104 0.651 0.851 0.493 0.517

@ Forward ADD filtering:
total time = 0.441 ms, time per slice = 0.063 ms.
=== Trajectory ===
0
0 : 0.136 0.499 0.896 0.349 0.149 0.507 0.483
1 : 0.864 0.501 0.104 0.651 0.851 0.493 0.517
```

## LICENSE

Copyright (c) 2015-2016 Thiago Pereira Bueno
All Rights Reserved.

DBN is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DBN is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with DBN.  If not, see <http://www.gnu.org/licenses/>.
