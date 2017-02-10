.PHONY = all clean arm

SRC = $(wildcard src/*.c)
OBJ_ARM = $(patsubst src/%.c, obj/armeabi-v7a/%.o, $(SRC))
OBJ_X64 = $(patsubst src/%.c, obj/x64/%.o, $(SRC))

NDK_CC = $(MY_TOOLCHAIN)/bin/arm-linux-androideabi-gcc
NDK_AR = $(MY_TOOLCHAIN)/bin/arm-linux-androideabi-ar
NDK_FLAGS = -Wall -std=gnu99 -g --sysroot=${MY_SYSROOT}

CFLAGS = -Wall -std=gnu99 -g -pthread

$(OBJ_X64): $(SRC)
	gcc -Iinclude -c $^ $(CFLAGS)
	mv *.o obj/x64

$(OBJ_ARM): $(SRC)
	$(NDK_CC) -Iinclude -c $^ $(NDK_FLAGS)
	mv *.o obj/armeabi-v7a

bin/lib/x64/liblynx.a: $(OBJ_X64)
	ar -x lib/x64/libpaho-mqtt3a.a
	mv *.o obj/x64
	ar rcs $@ obj/x64/*.o
	cp -r include/* bin/include

bin/lib/armeabi-v7a/liblynx.a: $(OBJ_ARM)
	$(NDK_AR) -x lib/armeabi-v7a/libpaho-mqtt3a.a
	mv *.o obj/armeabi-v7a
	$(NDK_AR) rcs $@ obj/armeabi-v7a/*.o
	cp -r include/* bin/include

arm: bin/lib/armeabi-v7a/liblynx.a

x64: bin/lib/x64/liblynx.a

example: x64
	gcc -L/usr/lib/x86_64-linux-gnu -Lbin/x64 -g -Llib/x64  -Iinclude -o bin/lib/x64/example example.c $(CFLAGS) bin/lib/x64/liblynx.a -lssl -lcrypto

run: example
	cp crt/ca.crt bin/lib/x64; cd bin/lib/x64; ./example ${ARGS}

test: x64
	gcc -L/usr/lib/x86_64-linux-gnu -Lbin/x64 -g -Llib/x64  -Iinclude -o bin/lib/x64/test test/test.c $(CFLAGS) bin/lib/x64/liblynx.a -lssl -lcrypto

runtest: test
	cp crt/ca.crt bin/lib/x64; cd bin/lib/x64; ./test ${ARGS}

testmem: example
	cd bin/lib/x64; valgrind --tool=massif --massif-out-file=massif.out --threshold=0.01 ./example
	cd bin/lib/x64; ms_print --threshold=0.01 massif.out > massif.txt

testproc: example
	cd bin/lib/x64; valgrind --tool=callgrind ./example

lines:
	find src -name '*.c' | xargs wc -l
	find include -name '*.h' | xargs wc -l

all: clean arm x64 example


#x64: bin/x64/liblynx.so
#	cp lib/x64/libpaho-mqtt3a.so bin/x64/libpaho-mqtt3a.so
	


#example: x64
#	gcc -Lbin/x64 -g -Llib/x64 -Iinclude -o bin/x64/example example.c $(CFLAGS) $(LIBS) -llynx
#
#	
#testmem: example
#	cd bin/x64; valgrind --tool=massif --massif-out-file=massif.out --threshold=0.01 ./example
#	cd bin/x64; ms_print --threshold=0.01 massif.out > massif.txt
#
#testproc: example
#	cd bin/x64; valgrind --tool=callgrind ./example
#
clean:
	rm -rf bin obj 
	mkdir obj obj/armeabi-v7a obj/x64 bin bin/include bin/lib bin/lib/armeabi-v7a bin/lib/x64

#
#lines:
#	find src -name '*.c' | xargs wc -l
#	find include -name '*.h' | xargs wc -l
