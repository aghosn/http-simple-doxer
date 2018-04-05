all: doxer

doxer: main.c http_parser.c
	gcc -g -O3 -o $@ $^

.PHONY: clean

clean:
	rm -f *.o doxer
