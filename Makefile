CC=g++
CCFLAGS=-Wall -Wextra -ansi -pedantic -std=c++11

OBJ=bin/variable.o bin/domain.o bin/factor.o bin/addfactor.o bin/io.o bin/graph.o bin/inference.o bin/main.o
OBJDEBUG=debug/variable.o debug/domain.o debug/factor.o debug/addfactor.o debug/io.o debug/graph.o debug/inference.o debug/main.o

CUDD=/usr/local/CUDD/cudd-3.0.0
# CUDD=/home/posmac/tbueno/lib/CUDD/cudd-3.0.0

INCLUDE=-Iinclude -I$(CUDD)/cudd -I$(CUDD)/cplusplus

LIBS=$(CUDD)/cudd/.libs/libcudd.a

all: dbn

install: clean
	mkdir bin/ debug/

dbn: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

bin/%.o: src/%.cpp include/%.h
	$(CC) $(CCFLAGS) $(INCLUDE) -O2 -c -o $@ $<

bin/main.o: src/main.cpp
	$(CC) $(CCFLAGS) $(INCLUDE) -O2 -c -o $@ $<

debug: dbn-debug
	valgrind --leak-check=full ./dbn-debug data/models/enough-sleep.duai data/evidence/enough-sleep.duai.evid -v -m 123
	# valgrind --leak-check=full --suppressions=dbn.supp ./dbn-debug data/models/enough-sleep.duai data/evidence/enough-sleep.duai.evid -v -m 123

dbn-debug: $(OBJDEBUG)
	$(CC) -o $@ $^ $(LIBS)

debug/%.o: src/%.cpp include/%.h
	$(CC) $(CCFLAGS) $(INCLUDE) -g -c -o $@ $<

debug/main.o: src/main.cpp
	$(CC) $(CCFLAGS) $(INCLUDE) -g -c -o $@ $<

.PHONY: clean
clean:
	rm -rfv dbn bin/*.o dbn-debug dbn-debug.dSYM/ dbn.dSYM/ debug/*.o
