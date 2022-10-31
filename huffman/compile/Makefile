# -*-MakeFile-*-

#constants:

CC = gcc
CFLAGS = -Wall -pedantic -std=c99
OBJ = huff
LIBS = -lm

all: ../bin/huff 
	

#...which .o files are needed to build huff executable?
#  >>make huff	
../bin/huff: ../obj/main.o ../obj/huffman.o ../obj/tree.o ../obj/pr_queue.o ../obj/bit.o ../obj/util.o
	$(CC) ../obj/main.o ../obj/huffman.o ../obj/tree.o ../obj/pr_queue.o ../obj/bit.o ../obj/util.o -o ../bin/huff $(CFLAGS) $(LIBS)

#...which files are needed to build .o files that are needed to build huff executable?
	
../obj/main.o: ../driver/main.c ../lib/api/huffman.h
	$(CC) -c ../driver/main.c -o ../obj/main.o $(CFLAGS)
	
../obj/huffman.o: ../lib/src/huffman.c ../lib/api/huffman.h ../lib/api/tree.h ../lib/api/util.h
	$(CC) -c ../lib/src/huffman.c -o ../obj/huffman.o $(CFLAGS)
	
../obj/tree.o: ../lib/api/tree.c ../lib/api/tree.h ../lib/api/bit.h ../lib/api/util.h
	$(CC) -c ../lib/api/tree.c -o ../obj/tree.o $(CFLAGS)
	
../obj/pr_queue.o: ../lib/api/pr_queue.c ../lib/api/pr_queue.h
	$(CC) -c ../lib/api/pr_queue.c -o ../obj/pr_queue.o $(CFLAGS)
	
../obj/bit.o: ../lib/api/bit.c ../lib/api/bit.h
	$(CC) -c ../lib/api/bit.c -o ../obj/bit.o $(CFLAGS)

../obj/util.o: ../lib/api/util.c ../lib/api/util.h
	$(CC) -c ../lib/api/util.c -o ../obj/util.o $(CFLAGS)


# .o files are built from .c files
# executable files are built from .o files
	


execc:
	../bin/huff -c ../bin/plrabn12.txt
	
execd:
	../bin/huff -d ../bin/plrabn12.txt.huff

#...remove .o files and executables, clean rebuild the whole project
clean: 
	rm -f ../obj/*.o ../driver/huff
#  >>make clean
#  >>make


	
	
