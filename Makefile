LIBS  = -lpaho-mqtt3a
CFLAGS = -Wall -std=gnu99  -pthread
LIBNAME = liblynx.so

# Should be equivalent to your list of C files, if you don't build selectively

SRC=$(wildcard ./src/*.c)

all: example x64 

bin/x64/liblynx.so: $(SRC)
	gcc -shared -g -fPIC -Llib/x64 -Iinclude -o $@ $^ $(CFLAGS) $(LIBS)

x64: bin/x64/liblynx.so
	

example: x64
	gcc -Lbin/x64 -g -Llib/x64 -Iinclude -o bin/x64/example example.c $(CFLAGS) $(LIBS) -llynx
	cp lib/x64/libpaho-mqtt3a.so bin/x64/libpaho-mqtt3a.so

	
testmem: example
	cd bin/x64; valgrind --tool=massif --massif-out-file=massif.out --threshold=0.01 ./example
	cd bin/x64; ms_print --threshold=0.01 massif.out > massif.txt

testproc: example
	cd bin/x64; valgrind --tool=callgrind ./example

clean:
	rm -rf bin
	mkdir bin bin/x64

lines:
	find src -name '*.c' | xargs wc -l
	find include -name '*.h' | xargs wc -l