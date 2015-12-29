CC=g++
CCFLAGS=-Wall -ansi -pedantic -std=c++11

OBJ=bin/variable.o bin/factor.o bin/domain.o bin/io.o bin/operations.o bin/inference.o bin/main.o

all: dbn

dbn: $(OBJ)
	$(CC) -o $@ $^

bin/%.o: src/%.cpp
	$(CC) $(CCFLAGS) -I include/ -O2 -c -o $@ $<

.PHONY: clean

clean:
	rm -rfv dbn bin/*.o
