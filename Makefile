all: doxer

doxer: main.c
	gcc -g -O3 -o $@ $^

.PHONY: clean

clean:
	rm -f *.o doxer