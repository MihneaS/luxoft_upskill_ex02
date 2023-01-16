CC = gcc
CFLAGS = -Wextra -Werror -Wall -Wcast-align -g

build: server client

server: server.c utils.h constants.h common.h
	$(CC) $(CFLAGS) server.c -lpthread -o server

client: client.c utils.h constants.h common.h
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm server
