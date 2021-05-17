CC=gcc
CFLAGS=-Wall -Werror -Wvla -std=c99 -fsanitize=address
# CFLAGS=-Wall -Werror -Wvla -std=c11 -fsanitize=address
PFLAGS=-fprofile-arcs -ftest-coverage
DFLAGS=-g
HEADERS=server.h
SRC=server.c

procchat: $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(DFLAGS) $(SRC) -o $@

test:
	make procchat
# sh compile_tests.sh
	sh test.sh

clean:
	rm -f procchat

# My testing
sani:
	rm client
	rm server
	rm gevent
