

#include "server.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <dirent.h> // for directory reading

#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/time.h>
// #include <sys/select.h>

#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#define BYTE unsigned char
#define BUF_SIZE (2048)
#define CHANNEL_NAME ("gevent")

#define TYPE_IX 0
#define IDEN_IX 2
#define DOMAIN_IX (2 + 256)

#define TYPE_LEN 2
#define IDEN_LEN 256    // max = 255 char with 1 for null byte
#define DOMAIN_LEN 256  // max = 255 char with 1 for null byte

#define RECEIVE_MSG_INDEX(draft) (draft + 2 + 256)
#define RECEIVE_ID_INDEX(draft) (draft + 2)
#define SAY_MSG_INDEX(draft) (draft + 2)
#define SET_RECEIVE(draft) ( *(short*)draft = Receive)


int do_say(char * buffer, const char * domain, const char * to_daemon_fp) {


    DIR *dr = opendir(domain);
  
    if (dr == NULL) { // opendir returns NULL if couldn't open directory
        printf("Could not open current directory" );
        return -1;
    }
    return 0;
}

int main() {
    DIR *dr = opendir("");
  
    if (dr == NULL) { // opendir returns NULL if couldn't open directory
        printf("Could not open current directory" );
        return -1;
    }
    return 0;
}