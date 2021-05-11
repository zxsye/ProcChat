#include "server.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

// #include <stdio.h>
#include <unistd.h>

#define BUF_SZ (256)
#define CHANNEL_NAME ("gevent")

int main(int argc, char** argv) {
    if((mkfifo(CHANNEL_NAME, S_IRWXU | S_IRWXG)) >= 0) { 
        int fd = open(CHANNEL_NAME, O_RDONLY); 
        if(fd > 0) { 
            FILE * read_channel = fdopen(fd, "r"); 
            char buf[BUF_SZ]; 
            while(fgets(buf, BUF_SZ, read_channel) != NULL) { 
                puts(buf); 
            }
            printf("closing pipe...\n");
            fclose(read_channel); 
        }
        close(fd);
    } else { 
        fprintf(stderr, "Unable to open pipe");
    }

    return 0;

}