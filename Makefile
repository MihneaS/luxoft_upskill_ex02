CC = gcc
CFLAGS = -Wextra -Werror -Wall -Wcast-align -g

build: server client

common: common.h common.c
	$(CC) $(CFLAGS) -c common.c -o common.o

server: server.c utils.h constants.h common
	$(CC) $(CFLAGS) server.c common.o -lpthread -o server

client: client.c utils.h constants.h common
	$(CC) $(CFLAGS) client.c common.o -o client

clean:
	rm server
