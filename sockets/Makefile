CC=clang
CFLAGS= -Wall -std=c99 -pthread -g -lrt

all: myipdb

myipdb: 
	$(CC) $(CFLAGS) myipdb.c -o myipdb 

sockets: server client

server:
	$(CC) $(CFLAGS) socket_server.c -o server

client:
	$(CC) $(CFLAGS) socket_client.c -o client

clean:
	rm -rf ./myipdb ./server ./client
