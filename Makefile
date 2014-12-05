CC=clang
CFLAGS= -Wall -std=c99 -pthread -g -lrt

all: myipdb

myipdb: 
	$(CC) $(CFLAGS) myipdb.c -o myipdb 


clean:
	rm -rf ./myipdb 
