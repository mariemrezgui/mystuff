CFLAGS = -std=c11 -pedantic -Wall -Wextra 
CC = clang
.PHONY: all clean
all: helloworld

shellutilstest: shellutils.o test.o
	$(CC) -o $@ $+ 
	
helloworld: hello.o
	$(CC) -o helloworld hello.o

hello.o: hello.c 
	$(CC) $(CFLAGS) -c hello.c 
%.o:%.c
	$(CC) $(CFLAGS) -c $< -g -O0
clean:
	rm -f *.o helloworld shellutilstest
