CFLAGS=-Wall
SRC=$(src/*.c)
HEADER=$(src/*.h)

all:
	gcc -std=c99 $(wildcard src/*.c) -I ./include -o lib.o