CC=g++
CCFLAGS=-Wall -ansi -pedantic -std=c++11

OBJ=bin/variable.o bin/domain.o bin/factor.o bin/io.o bin/main.o

all: dbn

dbn: $(OBJ)
	$(CC) -o $@ $^

bin/%.o: src/%.cpp include/%.h
	$(CC) $(CCFLAGS) -I include/ -g -c -o $@ $<

bin/main.o: src/main.cpp
	$(CC) $(CCFLAGS) -I include/ -g -c -o $@ $<

.PHONY: clean

clean:
	rm -rfv dbn bin/*.o
