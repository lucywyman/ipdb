/********************
** Lucy Wyman
** wymanl@onid.oregonstate.edu
** CS344-001
** Assignment 4
********************/


#ifndef _SOCKETS_H_
#define _SOCKETS_H_

#define _GNU_SOURCE
#include "socket_hdr.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define BUFFER 1024

void error_exit(char*);
void print_help();
void close_socket(int);

#endif
