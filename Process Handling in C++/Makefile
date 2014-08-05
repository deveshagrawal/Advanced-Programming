#define variables, CC for compiler
#objects is the output of compiler
CC=g++

# targets for make
all: holes.o
	$(CC) holes.o -lm -o holes
	
# holes.o	
holes.o: holes.cpp
	$(CC) -c -Wall holes.cpp -o holes.o

# rm	
clean:
	rm -rf *.o
	rm -rf holes
