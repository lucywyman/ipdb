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
        raw[read-1] = '\0';
        if(strstr(raw, "fetch")){
            printf("Fetching %s\n", &raw[6]);
            if(check_hostname(shm_path, &raw[6]) == 0)
                add_row(shm_path, &raw[6]);
            else
                printf("That host is already in the database!\n");
        }
        else if(strstr(raw, "check")){
            check_hostname(shm_path, &raw[6]);
        }
        else if(strstr(raw, "show")){
            print_db(shm_path);
        }
        else if(strstr(raw, "save")){
            save_db(shm_path, &raw[5]);
        }
        else if(strstr(raw, "load")){
            load_db(shm_path, &raw[5]);
        }
        else if(strstr(raw, "clear")){
            if(shm_unlink(shm_path) == -1)
                perror("shm_unlink");
        }
        else if(strstr(raw, "table")){
            if(strstr(raw, "unlock")){
                lock_table(shm_path, 0);
            } else {
                lock_table(shm_path, 1);
            }
        }
        else if(strstr(raw, "row")){
            if(strstr(raw, "unlock")){
                lock_row(shm_path, &raw[11], 0);
            } else {
                lock_row(shm_path, &raw[9], 1);
            }
        }
        else if(strstr(raw, "exit")){
            printf("Exiting...\n");
            lock_table(shm_path, 2);
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

void err_exit(char* function){
    perror(function);
    exit(1);
}

//This function creates the shared memory object
void init_shm(char* shm_path){
    int fd, flags;
    mode_t perms;

    perms = S_IRUSR | S_IWUSR;
    flags = O_RDWR | O_CREAT ;

    if((fd = shm_open(shm_path, flags, perms)) == -1)
        err_exit("shm_open");

    if(close(fd) == -1)
        err_exit("close");
}

int check_hostname(char* shm_path, char* hostname){
    int fd;
    char* addr;
    struct stat sb;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1)
        err_exit("shm_open");

    if(fstat(fd, &sb) == -1)
        err_exit("fstat");

    if(sb.st_size == 0){
        close(fd);
        return 0;
    }

    addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED)
        err_exit("mmap");

    if(close(fd) == -1)
        err_exit("close");

    for(int i = 0; i<(sb.st_size/sizeof(ip_row_t)); i++){
        struct ip_row* row = (struct ip_row*)addr;
        if(strcmp(row->row_name, hostname) == 0){
            if(sem_wait(&row->row_lock) == -1)
                err_exit("sem_wait");
            printf("%s IPV4: %s   IPV6: %s\n", row->row_name, row->row_address4, row->row_address6);
            if(sem_post(&row->row_lock) == -1)
                err_exit("sem_post");
            munmap(addr, sb.st_size);
            return 1; 
        }
        else
            addr += sizeof(ip_row_t);
    }
    munmap(addr, sb.st_size);
    return 0;
}

void add_row(char* shm_path, char* hostname){
    struct addrinfo hints, *res, *p;
    int status;
    struct ip_row new_row;
    int added = 0;

    memset(&new_row, 0, sizeof(struct ip_row));
    memcpy(&new_row.row_name, hostname, NAME_SIZE);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    for(p = res; p != NULL; p = p->ai_next) {
        void* addr;
        char ipstr[INET6_ADDRSTRLEN];
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr); 
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            if(added == 0){
                memcpy(new_row.row_address4, ipstr, NAME_SIZE);
                added = 1;
            }
        } 
        else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr); 
            inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
            memcpy(new_row.row_address6, ipstr, NAME_SIZE);
        }
    }
    if(sem_init(&new_row.row_lock, 1, 1) == -1)
        err_exit("sem_init");
    printf("%s %s %s\n", new_row.row_name, new_row.row_address4, new_row.row_address6);
    write_to(shm_path, new_row);
    freeaddrinfo(res);
}

void write_to(char* shm_path, struct ip_row message){
    int fd = 0;
    char* addr;
    struct stat sb;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1)
        err_exit("shm_open");

    if(fstat(fd, &sb) == -1)
        err_exit("fstat");

    if((ftruncate(fd, (sizeof(message)+sb.st_size))) == -1)
        err_exit("ftruncate");

    addr = mmap(NULL, (size_t)(sizeof(message)+sb.st_size), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED)
        err_exit("mmap");

    if(close(fd) == -1)
        err_exit("close");
    //lock_table(shm_path, 1);
    memcpy(addr+sb.st_size, &message, sizeof(message));
    //lock_table(shm_path, 0);
    munmap(addr, (size_t)sizeof(message)+sb.st_size);
}

void print_db(char* shm_path){
    int fd;
    char* addr;
    struct stat sb;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1)
        err_exit("shm_open");

    if(fstat(fd, &sb) == -1)
        err_exit("fstat");

    addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED)
        err_exit("mmap");

    if(close(fd) == -1)
        err_exit("close");

    for(int i = 0; i<(sb.st_size/sizeof(ip_row_t)); i++){
        struct ip_row* row = (struct ip_row*)addr;
        if(sem_wait(&row->row_lock) == -1)
            err_exit("sem_wait");
        printf("%s IPV4: %s   IPV6: %s\n", row->row_name, row->row_address4, row->row_address6);
        if(sem_post(&row->row_lock) == -1)
            err_exit("sem_post");
        addr += sizeof(ip_row_t);
    }
    munmap(addr, (size_t)sb.st_size);
}

void save_db(char* shm_path, char* filename){
    int fd, file_fd, flags;
    char* addr;
    struct stat sb;

    flags = O_CREAT | O_RDWR;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1)
        err_exit("shm_open");

    if(fstat(fd, &sb) == -1)
        err_exit("fstat");

    addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED)
        err_exit("mmap");

    if(close(fd) == -1)
        err_exit("close");

    if((file_fd=open(filename, flags, S_IRUSR | S_IWUSR)) == -1)
        err_exit("open"); 

    for(int i = 0; i<(sb.st_size/sizeof(ip_row_t)); i++){
        struct ip_row* row = (struct ip_row*)addr;
        if(sem_wait(&row->row_lock) == -1)
            err_exit("sem_wait");
        write(file_fd, row->row_name, strlen(row->row_name));
        write(file_fd, "\n", 1);
        if(sem_post(&row->row_lock) == -1)
            err_exit("sem_post");
        addr += sizeof(ip_row_t);
    }

    if(close(file_fd) == -1)
        err_exit("close");
    munmap(addr, sb.st_size);
}

void load_db(char* shm_path, char* filename){
    int fd, count = 0;
    char* addr;
    char* line;
    struct stat sb;
    FILE* file;
    size_t len = 0;
    ssize_t read;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1)
        err_exit("shm_open");

    if((file=fopen(filename, "r+")) == (FILE*)NULL)
        perror("open");

    while((read = getline(&line, &len, file)) > 0){
        if(check_hostname(shm_path, line) == 0)
            count++;
    }

    if(fstat(fd, &sb) == -1)
        err_exit("fstat");

    addr = mmap(NULL, (sb.st_size+(count*sizeof(struct ip_row))), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED)
        err_exit("mmap");

    if(close(fd) == -1)
        err_exit("close");
    if(fseek(file, 0, SEEK_SET) == -1)
        err_exit("fseek");
    while((read = getline(&line, &len, file)) > 0){
        line[strlen(line)-1] = '\0';
        if(check_hostname(shm_path, line) == 0)
            add_row(shm_path, line);
    }
    fclose(file);
    munmap(addr, count*sizeof(struct ip_row));
}

void lock_table(char* shm_path, int lock){
    int fd;
    char* addr;
    struct stat sb;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1)
        err_exit("shm_open");

    if(fstat(fd, &sb) == -1)
        err_exit("fstat");

    addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED)
        err_exit("mmap");

    if(close(fd) == -1)
        err_exit("close");

    for(int i = 0; i<(sb.st_size/sizeof(ip_row_t)); i++){
        struct ip_row* row = (struct ip_row*)addr;
        if(lock == 1){
            if(sem_wait(&row->row_lock) == -1)
                err_exit("sem_wait");
        } else if(lock == 0){
            if(sem_post(&row->row_lock) == -1)
                err_exit("sem_post");
        } else if(lock == 2){
            if(sem_destroy(&row->row_lock) == -1)
                err_exit("sem_destroy");
        }
        addr += sizeof(ip_row_t);
    }
    munmap(addr, sb.st_size);
}

void lock_row(char* shm_path, char* hostname, int lock){
    int fd;
    char* addr;
    struct stat sb;

    if((fd = shm_open(shm_path, O_RDWR, 0)) == -1)
        err_exit("shm_open");

    if(fstat(fd, &sb) == -1)
        err_exit("fstat");

    addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED)
        err_exit("mmap");

    if(close(fd) == -1)
        err_exit("close");

    for(int i = 0; i<(sb.st_size/sizeof(ip_row_t)); i++){
        struct ip_row* row = (struct ip_row*)addr;
        if(strcmp(row->row_name, hostname) == 0){
            if(lock == 1){
                if(sem_wait(&row->row_lock) == -1)
                    perror("sem_wait");
            } else {
                if(sem_post(&row->row_lock) == -1)
                    perror("sem_wait");
            }
        }
        addr += sizeof(ip_row_t);
    }
    munmap(addr, sb.st_size);
}
