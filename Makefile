all: doxer trial

doxer: main.c http_parser.c
	gcc -g -O0 -o $@ $^

trial: parsing_trial.c http_parser.c
	gcc -g -O0 -o $@ $^

.PHONY: clean

clean:
	rm -f *.o doxer trial
