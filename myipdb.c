#include "myipdb.h"
#include "./Project4/ipdb.h"

int main(){
    char* shm_path = malloc(NAME_SIZE*sizeof(char));
    SHARED_MEM_NAME(shm_path);
    init_shm(shm_path);
    size_t len = 0;
    ssize_t read;
    char* raw = NULL;

    printf("%s", PROMPT);
    while((read = getline(&raw, &len, stdin)) != -1){
        if(strstr(raw, "fetch")){
            raw[read-1] = '\0';
            printf("Fetching %s", &raw[6]);
            if(check_hostname(shm_path, &raw[6]) == 0)
                add_row(shm_path, &raw[6]);
        }
        else if(strstr(raw, "check")){
        }
        else if(strstr(raw, "show")){
            print_db(shm_path);
        }
        else if(strstr(raw, "save")){
        }
        else if(strstr(raw, "load")){
        }
        else if(strstr(raw, "clear")){
            if(shm_unlink(shm_path) == -1)
                perror("shm_unlink");
        }
        else if(strstr(raw, "table")){
            if(strstr(raw, "unlock")){
            } else {
            }
        }
        else if(strstr(raw, "row")){
            if(strstr(raw, "unlock")){
            } else {
            }
        }
        else if(strstr(raw, "exit")){
            printf("Exiting...\n");
            if(shm_unlink(shm_path) == -1){
                printf("Could not unlink memory\n");
                exit(1);
            }
            free(shm_path);
            free(raw);
            exit(0);
        }
        else{
            printf("Sorry! That wasn't a valid command.\n");
        }
        printf("%s", PROMPT);
    }
    if(shm_unlink(shm_path) == -1){
        printf("Could not unlink memory\n");
        exit(1);
    }
    free(shm_path);
    free(raw);
    return 0;
}

//This function creates the shared memory object
void init_shm(char* shm_path){
    int fd, flags;
    mode_t perms;

    perms = S_IRUSR | S_IWUSR;
    flags = O_RDWR | O_CREAT ;

    if((fd = shm_open(shm_path, flags, perms)) == -1){
        printf("Could not open shared memory!\n");
        exit(1);
    }
}

int check_hostname(char* shm_path, char* hostname){
    return 0;
}

void add_row(char* shm_path, char* hostname){
    struct addrinfo hints, *res, *p;
    int status;
    struct ip_row new_row;
    int added = 0;

    memset(&new_row.row_name, 0, NAME_SIZE);
    memcpy(&new_row.row_name, hostname, NAME_SIZE);

    memset(&hints, 0, sizeof hints);
    memset(&new_row.row_address4, 0, NAME_SIZE);
    memset(&new_row.row_address6, 0, NAME_SIZE);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    printf("IP addresses for %s:\n\n", hostname);

    for(p = res; p != NULL; p = p->ai_next) {
        void* addr;
        char ipstr[INET6_ADDRSTRLEN];
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr); 
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            if(added == 0){
                printf("%s\n", ipstr);
                memcpy(new_row.row_address4, ipstr, NAME_SIZE);
                added = 1;
            }
        } 
        else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr); 
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            printf("%s\n", ipstr);
            memcpy(new_row.row_address4, ipstr, NAME_SIZE);
        }
    }
    new_row.row_lock = *(sem_open(shm_path, O_CREAT, S_IRUSR | S_IWUSR));

    write_to(shm_path, new_row);
    freeaddrinfo(res);
}

void write_to(char* shm_path, struct ip_row message){
    int fd = 0;
    char* addr;
    struct stat sb;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1){
        printf("Could not open shared memory!\n");
        exit(1);
    }

    if(fstat(fd, &sb) == -1){
        printf("Could not stat file!\n");
        exit(1);
    }

    if((ftruncate(fd, (sizeof(message)+sb.st_size))) == -1){
        printf("Could not resize shared memory!\n");
        exit(1);
    }

    addr = mmap(NULL, (size_t)sizeof(message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, sb.st_size);
    if(addr == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    if(close(fd) == -1){
        printf("Could not close file!\n");
        exit(1);
    }
    printf("Copying %ld bytes to shared memory\n", sizeof(message));
    memcpy(addr, &message, sizeof(message));
}

void read_from(char* shm_path){
    int fd;
    char* addr;
    struct stat sb;

    if((fd = shm_open(shm_path, O_RDONLY, 0)) == -1){
        printf("Could not open shared memory!\n");
        exit(1);
    }

    if(fstat(fd, &sb) == -1){
        printf("Could not stat file!\n");
        exit(1);
    }

    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED){
        printf("Could not map memory!\n");
        exit(1);
    }

    if(close(fd) == -1){
        printf("Could not close file!\n");
        exit(1);
    }

    write(STDOUT_FILENO, addr, sb.st_size);
    printf("\n");
}

void print_db(char* shm_path){
   read_from(shm_path);     
}
