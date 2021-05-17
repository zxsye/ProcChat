CC=gcc
CFLAGS=-Wall -Werror -Wvla -std=c99 -fsanitize=address
# CFLAGS=-Wall -Werror -Wvla -std=c11 -fsanitize=address
PFLAGS=-fprofile-arcs -ftest-coverage
DFLAGS=-g
HEADERS=server.h
SRC=server.c

procchat: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o $@

server: $(SRC) $(HEADERS)
	$(CC) $(SRC) -o procchat

test:
	make procchat
	make compile_tests
	sh test.sh

compile_tests:
	sh compile_tests.sh

clean:
	rm -f procchat
