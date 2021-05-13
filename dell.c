#include "server.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>

#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#define BYTE unsigned char
#define BUF_SIZE (2049)
#define CHANNEL_NAME ("gevent")

#define TYPE_IX 0
#define IDEN_IX 2
#define DOMAIN_IX (2 + 256)

#define TYPE_LEN 2
#define IDEN_LEN 256    // max = 255 char with 1 for null byte
#define DOMAIN_LEN 256  // max = 255 char with 1 for null byte


#include "server.h"
#define RD (1)
#define WR (0)
#define MAX_SIZE (2048)
#define SAY (1)
#define SAYCONT (2)
#define RECEIVE (3)
#define RECVCONT (4)
#define PING (5)
#define PONG (6)
#define DISCONNECT (7)

int create_connection(int fd){
    char buffer[MAX_SIZE];
    uint16_t type;
    char identifier_buffer[256];
    char domain_buffer[1790];

    //reading the type
    if(read(fd, buffer, MAX_SIZE) == -1){
        printf("could not read\n");
        return 1;
    }
    type = buffer[1];
    printf("Type is %d\n", type);
    for(int i = 0; i < 256; i++){
        identifier_buffer[i] = buffer[2+i];
    }
    for(int i = 0; i < 1790; i++){
        domain_buffer[i] = buffer[258+i];
    }
    char* identifier = identifier_buffer;
    char* domain = domain_buffer;
    // printf("Identifier is %s\n", identifier);
    // printf("Domain is %s\n", domain);
    if(type == 0){
        //connect
        if(mkdir(domain, 0777) == -1){
            if(errno != EEXIST){
                printf("failed to create domain\n");
                return 1;
            }
        }
        char* dirname = strcat(domain, "/");
        dirname = strcat(dirname, identifier);
        char dirname_rd[2048];
        char dirname_wr[2048];
        strcpy(dirname_rd, dirname);
        strcpy(dirname_wr, dirname);
        strcat(dirname_rd, "_RD");
        strcat(dirname_wr, "_WR");
        // printf("thid is rd_path %s\n", dirname_rd);
        if(mkfifo(dirname_rd, 0777) == -1){
            if (errno != EEXIST) {
                printf("Could not create read\n");
                return 1;
            }
        }
        if(mkfifo(dirname_wr, 0777) == -1){
            if (errno != EEXIST) {
                printf("Could not create write\n");
                return 1;
            }
        }
    }
    else{
        printf("Incorrect Type\n");
        return 1;
    }

    return 0;
}

int main(int argc, char** argv) {
    // Your code here
    if (mkfifo("gevent", 0777) == -1) {
        if (errno != EEXIST) {
            printf("Could not create gevent\n");
            return 1;
        }
    }

    fd_set readfds;
    struct timeval timeout;
    int fd = open("gevent", O_RDONLY);
    int max_file = fd + 1;
    if(fd == -1){
        printf("could not open gevent\n");
        return 1;
    }
    while(1){
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        timeout.tv_sec = 2;
		timeout.tv_usec = 0;
        int ret = select(max_file, &readfds, NULL, NULL, &timeout);
        if (ret == -1 || ret == 0) {
			printf("Failed to select\n");

        }
        else{
			if(FD_ISSET(fd, &readfds) == 0){
                printf("Failed to set\n");
                return 1;
            }
            
            char buffer[3000];
            int nread = read(fd, buffer, sizeof(buffer));
            printf("received %s\n", buffer);
            
            // if(create_connection(fd) == 1){
            //     printf("failed to create connection\n");
            //     return 1;
            // }
        }

        
    }
    close(fd);
    return 0;
}