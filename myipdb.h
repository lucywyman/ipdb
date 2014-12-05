#define _POSIX_SOURCE 
#define _XOPEN_SOURCE
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "./Project4/ipdb.h"
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void init_shm(char*);
void add_row(char*, char*);
int check_hostname(char*, char*);
void write_to(char*, struct ip_row);
void read_from(char*);
void print_db(char*);
void clear_db(char*);
