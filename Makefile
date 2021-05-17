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
	$(CC) $(CFLAGS) $(PFLAGS) $(SRC) -o $@

clean:
	rm -f procchat

# My testing
sani:
	rm client
	rm server
	rm gevent
	rm -r Rutherford

server:
	gcc -o server server.c

client: 
	gcc -o client client.c

all:
	make server
	make client

me:
	make sani
	make all
