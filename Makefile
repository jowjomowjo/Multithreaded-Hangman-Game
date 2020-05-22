CC=g++
CFLAGS= -Wall -Werror -pedantic -std=c++11

all: game_server game_client

game_server: game_server.cpp
			$(CC) $(CFLAGS) -pthread game_server.cpp -o game_server

game_client: game_client.cpp
			$(CC) $(CFLAGS) game_client.cpp -o game_client

clean: rm -f game_server game_client
