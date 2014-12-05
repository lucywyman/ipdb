/*
 * R Jesse Chaney
 * chaneyr@eecs.orst.edu
 * CS344-001
 * Fall 2014
 * Oregon State University
 * School of EECS
 */

/*
 * $Author: chaneyr $
 * $Date: 2014/11/24 11:55:53 $
 * $RCSfile: socket_hdr.h,v $
 * $Revision: 1.4 $
 */


#ifndef _SOCKET_HDR_H_
# define _SOCKET_HDR_H_

// This will be the backlog argument.  The maximum length
// to which the queue of pending connections for sockfd
// may grow.
# define LISTENQ 100

# define MAXLINE 1000  // max text line length

// This is a list of the available commands for the client.
// You can get more about them by typing 'help' into the client.
# define CMD_EXIT         "exit"
# define CMD_CHDIR        "cd"
# define CMD_LOCAL_CHDIR  "lcd"
# define CMD_DIR          "dir"
# define CMD_LOCAL_DIR    "ldir"
# define CMD_PWD          "pwd"
# define CMD_LOCAL_PWD    "lpwd"
# define CMD_PUT          "put"
# define CMD_GET          "get"
# define CMD_HELP         "help"

// This is the command to use with popen() to get a
// directory listing.
# define CMD_LS_POPEN    "ls -lFa"

# define PROMPT ">>> "

# define RETURN_ERROR "**ERROR**"
# define DIR_SEP "/"
# define EOF_STR "\04"

#endif // _SOCKET_HDR_H_
