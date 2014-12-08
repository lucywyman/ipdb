#include "sockets.h"
#include "server.h"

int main(int argc, char* argv[]){
    int opt, port, ip;

    if(argc < 3)
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
                ip = atoi(optarg);
                break;
            default:
                print_help();
                break;
        }
    }   
    create_socket(port);
    return 0;
}

void error_exit(char* function){
    perror(function);
    exit(1);
}

void print_help(){

}

void create_socket(int port){
    int sock_fd, newsock_fd;
    struct sockaddr_in server, client;
    socklen_t client_len;
    char buffer[BUFFER];
    int i = 0;
    // Create thread pool
    pthread_t pool[LISTENQ];

    bzero(buffer, BUFFER);

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error_exit("socket");
    memset(&server, '\0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if(bind(sock_fd, (struct sockaddr *) &server, sizeof(server)) < 0)
        error_exit("bind");
    while(1){
        listen(sock_fd, 5);
        client_len = sizeof(client);
        if((newsock_fd=accept(sock_fd, (struct sockaddr *) &client, &client_len)) < 0)
            error_exit("accept");
        if(pthread_create(&pool[i++], NULL, &baby_server, (void *)newsock_fd) < 0)
            error_exit("pthread_create");
    }
}

void *baby_server(void* newsock_fd){
    char buffer[BUFFER];
    int sock_fd = (int) newsock_fd;
    char* help = "Sockets Client-Server\n\
    cd -- Change to your home directory\n\
    cd <directory> -- Change to a different directory\n\
    pwd -- Print your current directory\n\
    dir -- list all files in current directory (and other information about files\n\
    put -- Send a file to the server\n\
    get -- Get a file from the server\n\
    exit -- Terminate the client process\n";

    memset(&buffer, '\0', BUFFER);

    while((read(sock_fd, buffer, BUFFER)) > 0){
        if(strstr(buffer, "cd")){
            if(strlen(buffer) >=4){
                char cwd[BUFFER];
                buffer[strlen(buffer)-1] = '\0';
                //fprintf(stderr, "%s\n", &buffer[3]);
                if(chdir(&buffer[3]) !=0){
                    char* err = "That's not a valid directory!";
                    write(sock_fd, err, strlen(err));
                }
                sprintf(cwd, "Changed directory to %s", &buffer[3]);
                if(write(sock_fd, cwd, BUFFER) < 0)
                    error_exit("write");
            }
            else {
                char cwd[BUFFER];
                chdir(getenv("HOME"));
                sprintf(cwd, "Changed directory to %s", getenv("HOME"));
                if(write(sock_fd, cwd, BUFFER) < 0)
                    error_exit("write");
            }
        } 
        else if(strstr(buffer, "dir")){
            char* little;
            FILE* dir = popen(CMD_LS_POPEN, "r");
            size_t l_len = 0;
            ssize_t l_read;
            while((l_read = getline(&little, &l_len, dir)) != -1 && !feof(dir))
                write(sock_fd, little, strlen(little));
            fclose(dir);
        }
        else if(strstr(buffer, "pwd")){
            char cwd[BUFFER];
            memset(cwd, '\0', BUFFER);
            if(getcwd(cwd, sizeof(cwd)) != NULL){
                write(sock_fd, cwd, BUFFER);
            }
        }
        else if(strstr(buffer, "put")){
            printf("Put");
        }
        else if(strstr(buffer, "get")){
            printf("get");
        }
        else if(strstr(buffer, "exit")){
            char* term = "Terminating connection...\n";
            write(sock_fd, term, strlen(term));
            write(sock_fd, EOF_STR, strlen(EOF_STR));
            close(sock_fd);
            pthread_exit(NULL);
        }
        else{
            char* err = "Oops!  That wasn't a valid command.\n";
            write(sock_fd, err, strlen(err));
            write(sock_fd, help, strlen(help));
        }
        memset(buffer, '\0', BUFFER);
    }
    close(sock_fd);
    pthread_exit(NULL);
}

void close_socket(int sock_fd){
    if(close(sock_fd) == -1)
        error_exit("close");
}


