all: base14.o bitio.o dir
	gcc -o build/base16384 build/base14.o build/bitio.o -O3
bitio.o: bitio.c dir
	gcc -o build/bitio.o -c bitio.c -O3
base14.o: base16384.c dir
	gcc -o build/base14.o -c base16384.c -O3
dir:
	if [ ! -d "build" ]; then mkdir build; fi
clean:
	if [ -d "build" ]; then rm -rf build; fi
install:
	cp build/base16384 /usr/local/bin/
	chmod +x /usr/local/bin/base16384