#define _POSIX_SOURCE 
#define _XOPEN_SOURCE
#define _GNU_SOURCE
#define _SVID_SOURCE
#include "./Project4/ipdb.h"
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void init_shm(char*);
void err_exit(char*);
void add_row(char*, char*);
int check_hostname(char*, char*);
void write_to(char*, struct ip_row);
void read_from(char*);
void print_db(char*);
void clear_db(char*);
void save_db(char*, char*);
void lock_table(char*, int lock);
