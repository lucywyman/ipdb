// TODO fix exit function
// TODO puts and gets
// TODO Test multiple clients
// TODO Extra credit?
#include "sockets.h"
#include "client.h"

int main(int argc, char* argv[]){
    int opt, port;
    char* ip;

    if(argc < 5)
        print_help();
    while((opt = getopt(argc, argv, "p:i:h")) != -1){
        switch(opt){
            case 'h': 
                print_help();
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'i':
                ip = optarg;
                break;
            default:
                print_help();
                break;
        }   

    }
    create_client(port, ip);
    return 0;
}

void error_exit(char* function){
    perror(function);
    exit(1);
}

void print_help(){
    printf("Sockets Client-Server\n\
            cd -- Change to your home directory\n\
            cd <directory> -- Change to a different directory\n\
            pwd -- Print your current directory\n\
            dir -- list all files in current directory (and other information about files\n\
            put -- Send a file to the server\n\
            get -- Get a file from the server\n\
            exit -- Terminate the client process\n");
}

void create_client(int port, char* ip){
    signal(SIGINT, destroy_client);
    int sock_fd;
    struct sockaddr_in server_address;
    //struct hostent* server;
    char buffer[BUFFER];

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error_exit("socket");

    bzero((char *) &server_address, sizeof(server_address));

    inet_pton(AF_INET, ip, &(server_address.sin_addr));    
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (connect(sock_fd,(struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
        error_exit("connect");

    printf("%s", PROMPT);
    bzero(buffer, BUFFER);
    while(fgets(buffer, BUFFER, stdin) > 0){
        if(buffer[0] == 'l' || strstr(buffer, "help")){
            parse(buffer);
        } else {
            if(write(sock_fd, buffer, BUFFER) == -1)
                error_exit("write");
            memset(buffer, '\0', BUFFER);
            if(read(sock_fd, buffer, BUFFER) == -1)
                error_exit("read");
            write(STDOUT_FILENO, buffer, BUFFER);

        }
        memset(buffer, '\0', BUFFER);
        // Wait for process to finish to print next prompt.  Hacky and gross, but it works
        sleep(1);
        printf("\n%s", PROMPT);
    }
    close(sock_fd);
}

void parse(char* buffer){
    if(strstr(buffer, "lcd")){
        if(strlen(buffer) >= 4){
            chdir(&buffer[4]);
            printf("Changed directory to %s", &buffer[4]);
        }
        else{
            chdir(getenv("HOME"));
            printf("Changed to home directory");
        }
    }
    else if(strstr(buffer, "lpwd")){
        char cwd[BUFFER];
        if(getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s", cwd);
    }
    else if(strstr(buffer, "ldir")){
        if(popen(CMD_LS_POPEN, "w") == (FILE *)NULL)
            error_exit("popen");
    }
    else if(strstr(buffer, "help"))
        print_help();
    else{
        printf("Oops!  That wasn't a valid command");
        print_help();
    }
}

void close_socket(int sock_fd){
    if(close(sock_fd) == -1)
        error_exit("close");
}

void destroy_client(int signo){
    if(signo == SIGINT){
        printf("Terminating connection...\n");
        exit(0);
    }
}

