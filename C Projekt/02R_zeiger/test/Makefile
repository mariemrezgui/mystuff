CC=clang

.PHONY: lili_test clean

lili_test: clean
	cp ../src/lili.c .
	cp ../include/lili.h .
	$(CC) -c -o lili.o lili.c
	$(CC) -o $@ lili_test.c lili.o

clean:
	rm -f lili.c
	rm -f lili.h
	rm -f lili.o
	rm -f lili_test
