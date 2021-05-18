#ifndef SERVER_H
#define SERVER_H

#define _POSIX_SOURCE

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
#define TRM_IX (2048 - 1)

#define TYPE_LEN 2
#define IDEN_LEN 256    // max = 255 char with 1 for null byte
#define DOMAIN_LEN 256  // max = 255 char with 1 for null byte

#define PPTIME 15
#define TRM -1
#define NON_TRM 0

#define SAY_MSG_INDEX(draft) (draft + TYPE_LEN)
#define IDEN_INDEX(draft) (draft + TYPE_LEN)
#define DOMAIN_INDEX(draft) (draft + TYPE_LEN + IDEN_LEN)

#define RECEIVE_ID_INDEX(draft) (draft + TYPE_LEN)
#define RECEIVE_MSG_INDEX(draft) (draft + TYPE_LEN + IDEN_LEN)

#define GET_TYPE(string) (*(short*)string)
#define SET_TYPE(string, t) (*(short*)string = t)
#define SET_TRM(string, trm) (*(string + 2048 - 1) = trm )

#define IDEN(string) (string + IDEN_IX)
#define GET_DOMAIN(string) (string + DOMAIN_IX)

enum type {
    Connect = 0,
    Say = 1,
    Saycont = 2,
    Receive = 3,
    Recvcont = 4,
    Disconnect = 7,
    Ping = 5,
    Pong = 6
};

typedef struct pipeline {
    char domain[DOMAIN_LEN];
    char iden[IDEN_LEN];
    char to_client_fp[BUF_SIZE];
    char to_daemon_fp[BUF_SIZE];
} Pipeline;

#endif
