#define variables, CC for compiler
#objects is the output of compiler
CC=gcc

# targets for make
all: dine.o
	$(CC) dine.o -lpthread -o dine
	
# dine.o	
dine.o: dine.c
	$(CC) -c -Wall -std=c99 dine.c -o dine.o

# rm	
clean:
	rm -rf *.o
	rm -rf dine
