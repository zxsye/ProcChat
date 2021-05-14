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

enum type {
    Connect = 0,
    Say = 1,
    Saycount = 2,
    Receive = 3,
    Recvcont = 4,
    Disconnect = 7,
    Ping = 5,
    Pong = 6
};

int main() {
    int gevent_fd = open("gevent", O_WRONLY);
    if (gevent_fd < 0) {
        return 1;
    }

    char msg[BUF_SIZE];
    msg[0] = 0;
    msg[1] = 0;
    strcpy(msg + 2, "friday");
    strcpy(msg + 2 + 256, "april");

    write(gevent_fd, msg, 2048);

    // Connect june
    sleep(1);
    msg[0] = 0;
    msg[1] = 0;
    strcpy(msg + 2, "sunday");
    strcpy(msg + 2 + 256, "april");

    write(gevent_fd, msg, 2048);

    // SAY
    sleep(1);
    int wr = open("april/friday_WR", O_WRONLY);
    msg[0] = 0;
    msg[1] = Say;
    strcpy(msg + 2, "what's up doc?");

    write(wr, msg, 2048);

    // RECEIVE
    sleep(2);
    for (int i = 0; i < sizeof(msg); i++) {
        msg[i] = 0;
    }

    printf("\nClient trying to receive...\n");

    int rd = open("april/sunday_RD", O_NONBLOCK, O_RDONLY);
    if (rd < 0) {
        perror("cannot open sunday_rd");
    }
    printf("opened sunday_RD\n");

    
    if (read(rd, msg, sizeof(msg)) < 0) {
        perror("cannot read");
    }
    printf("[ april/sunday receieved ]\n");
    
    printf("Type: %d %d\n", msg[0], msg[1]);
    printf("From: %s\n", msg + 2);
    printf("Message: %s\n", msg + 2 + 256);

    for (int i = 0; i < sizeof(msg); i++) {
        printf("%c ", msg[i]);
    }

    // CLOSING EVERYTHING
    close(gevent_fd);
    close(wr);
    close(rd);

    return 0;
}