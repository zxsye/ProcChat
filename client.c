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

typedef struct pipeline {
    char * domain;
    char * iden;
    char * to_client_fp;
    char * to_daemon_fp;
} Pipeline;

void clear_msg(char * draft) {
    for (int i = 0; i < BUF_SIZE; i++) {
        draft[i] = 0;
    }
}
/*  Creates message for connection in the format:
    CONNECT <identifier> <domain>
 */
void draft_connect_str(char * draft, char * identifier, char * domain) {
    clear_msg(draft);
    SET_TYPE(draft, Connect);
    
    char * draft_iden = IDEN_INDEX(draft);
    strcpy(draft_iden, identifier);

    char * draft_domain = DOMAIN_INDEX(draft);
    strcpy(draft_domain, domain);
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

void write_to(char * pipe_name, char * message) {

}

void read_from() {

}

int start_client(Pipeline * pline, char * domain, char * iden) {

    strncpy(domain, GET_DOMAIN(buffer), DOMAIN_LEN); 
    strcpy(iden, IDEN(buffer));

    strcpy(to_client_fp, domain);                       // domain
    strcat(to_client_fp, "/");                          // domain/
    strcat(to_client_fp, iden);                         // domain/identifier

    strcpy(to_daemon_fp, to_client_fp);                 // domain/identifier

    strcat(to_daemon_fp, "_WR");                        // domain/identifier_WR
    strcat(to_client_fp, "_RD");                        // domain/identifier_RD

    pline->domain = domain;
    pline->iden = iden;
    pline->to_client_fp = to_client_fp;
    pline->to_daemon_fp = to_daemon_fp;

}

int main() {
    // Connect to server
    int fp = open(CHANNEL_NAME, O_WRONLY);
    if (fp < 0) {
        perror("Failed to open gevent");
    }

    char draft[BUF_SIZE] = {0};
    draft_connect_str(draft, "Colby", "Rutherford");
    write(fp, draft, BUF_SIZE);
    close(fp);

    char * colby_RD = "Rutherford/Colby_RD";
    char * colby_WR = "Rutherford/Colby_WR";



    // Monitor server
    



}