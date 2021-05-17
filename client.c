#define _POSIX_SOURCE
#include "server.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <dirent.h> // for directory reading

#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/select.h>

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

#define PPTIME 15

#define SAY_MSG_INDEX(draft) (draft + TYPE_LEN)
#define IDEN_INDEX(draft) (draft + TYPE_LEN)
#define DOMAIN_INDEX(draft) (draft + TYPE_LEN + IDEN_LEN)

#define RECEIVE_ID_INDEX(draft) (draft + TYPE_LEN)
#define RECEIVE_MSG_INDEX(draft) (draft + TYPE_LEN + IDEN_LEN)

#define GET_TYPE(string) (*(short*)string)
#define SET_TYPE(string, t) (*(short*)string = t)

#define IDEN(string) (string + IDEN_IX)
#define GET_DOMAIN(string) (string + DOMAIN_IX)

typedef struct pipeline {
    char domain[DOMAIN_LEN];
    char iden[IDEN_LEN];
    char to_client_fp[BUF_SIZE];
    char to_daemon_fp[BUF_SIZE];
} Pipeline;

void get_filepath(char * buffer, Pipeline * pline);

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



void clear_msg(char * draft) {
    for (int i = 0; i < BUF_SIZE; i++) {
        draft[i] = 0;
    }
}

int draft_say(char * draft, char * message, enum type t) {
    if (t != Say && t != Saycount) {
        return -1;
    }
    clear_msg(draft);
    SET_TYPE(draft, t);
    
    char * draft_msg = SAY_MSG_INDEX(draft);
    strcpy(draft_msg, message);

    return 0;
}

int send(Pipeline * sender, char * message) {
    char draft[BUF_SIZE] = {0};
    draft_say(draft, message, Say);

    errno = 0;
    int fd = open(sender->to_daemon_fp, O_WRONLY);
    if (fd <= 0) {
        printf("open failed for %s\n", sender->to_daemon_fp);
        perror("");
    }

    if (write(fd, draft, BUF_SIZE) <= 0) {
        printf("write failed for %s\n", sender->to_daemon_fp);
        perror("");
    }
    close(fd);

    printf("\n=== Sent message ===\n");
    printf("Sender: %s\nType: %d, Msg: %s\n", sender->to_daemon_fp, GET_TYPE(draft), SAY_MSG_INDEX(draft));

    return 0;
}

void print_msg(char * msg, Pipeline * receiver) {
    short type = *(short*)msg;
    char * from = IDEN_INDEX(msg);
    char * content = RECEIVE_MSG_INDEX(msg);

    printf("\n=== Received message ===\n");
    printf("Receiver: %s\n", receiver->to_daemon_fp);
    if (type == Receive) {
        printf("Type: %d\nFrom: %s\nContent: %s\n", type, from, content);

    } else if (type == Recvcont) {
        BYTE trm = *(BYTE*)(msg + BUF_SIZE - 1);
        printf("Type: %d\nFrom: %s\nContent: %s\nTerminate: %d\n", type, from, content, trm);
        
    } else {
        perror("Message received neither RECEIVE, RECVCONT, nor PING");
        return;
    }
}

void receive(Pipeline * receiver) {
    errno = 0;
    int fd = open(receiver->to_client_fp, O_RDONLY);
    if (fd <= 0) {
        printf("open failed for %s\n", receiver->to_client_fp);
        perror("");
        return;
    }

    char msg[BUF_SIZE];
    if (read(fd, msg, BUF_SIZE) <= 0) {
        printf("receive failed for %s\n", receiver->iden);
        return;
    }
    close(fd);

    print_msg(msg, receiver);
}


/*  Creates message for connection in the format:
    CONNECT <identifier> <domain>
 */
void connect_to_server(Pipeline * pline) {
    char draft[BUF_SIZE] = {0};
    SET_TYPE(draft, Connect);
    
    char * draft_iden = IDEN_INDEX(draft);
    strcpy(draft_iden, pline->iden);
    char * draft_domain = DOMAIN_INDEX(draft);
    strcpy(draft_domain, pline->domain);

    int fd = open(CHANNEL_NAME, O_WRONLY);
    write(fd, draft, BUF_SIZE);
    close(fd);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100 * 1000;
    select(0, NULL, NULL, NULL, &timeout);
    // Open reading channel
    // fd = open(pline->to_daemon_fp, O_WRONLY);
    // fd = open(pline->to_client_fp, O_RDONLY);    
}


void init_client_pipeline(Pipeline * pline, char * domain, char * iden) {
    strncpy(pline->domain, domain, DOMAIN_LEN); 
    strcpy(pline->iden, iden);

    strcpy(pline->to_client_fp, pline->domain);                       // domain
    strcat(pline->to_client_fp, "/");                          // domain/
    strcat(pline->to_client_fp, pline->iden);                         // domain/identifier

    strcpy(pline->to_daemon_fp, pline->to_client_fp);                 // domain/identifier

    strcat(pline->to_daemon_fp, "_WR");                        // domain/identifier_WR
    strcat(pline->to_client_fp, "_RD");                        // domain/identifier_RD
}

int main() {
    // Connect to server
    Pipeline colby;
    init_client_pipeline(&colby, "Rutherford", "Colby");
    // printf("Domain: %s\nIden: %s\nto_client: %s\nto_daemon: %s\n\n", colby.domain, colby.iden, colby.to_client_fp, colby.to_daemon_fp);
    connect_to_server(&colby);

    Pipeline zara;
    init_client_pipeline(&zara, "Rutherford", "Zara");
    connect_to_server(&zara);

    send(&colby, "Hi I'm Colby");
    receive(&zara);

    send(&zara, "Hi my name's Zara, nice to meet you");
    receive(&colby);

    Pipeline joshua;
    init_client_pipeline(&joshua, "Rutherford", "Joshua");
    connect_to_server(&joshua);

    send(&joshua, "Hey guys!");
    receive(&colby);
    receive(&zara);

    send(&colby, "Hey Joshua, welcome to the domain :)");
    receive(&joshua);
    receive(&zara);
    
    send(&zara, "Thanks for joining us");
    receive(&colby);
    receive(&joshua);
}