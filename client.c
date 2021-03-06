#define _POSIX_SOURCE
#include "server.h"

void clear_msg(char * draft) {
    for (int i = 0; i < BUF_SIZE; i++) {
        draft[i] = 0;
    }
}

int draft_say(char * draft, char * message, int msg_type, char trm) {

    clear_msg(draft);
    SET_TYPE(draft, msg_type);
    
    char * draft_msg = SAY_MSG_INDEX(draft);
    if (msg_type == Saycont) {
        SET_TRM(draft, trm);
    }
    strcpy(draft_msg, message);

    return 0;
}

int send(Pipeline * sender, char * message, int msg_type, char trm) {
    char draft[BUF_SIZE] = {0};
    draft_say(draft, message, msg_type, trm);

    errno = 0;
    int fd = open(sender->to_daemon_fp, O_WRONLY);
    if (fd <= 0) {
        perror("Failed in send()");
        fprintf(stderr, "open failed for: %s\n", sender->to_daemon_fp);
        return -1;
    }

    if (write(fd, draft, BUF_SIZE) <= 0) {
        perror("Failed in send()");
        fprintf(stderr, "write failed for: %s\n", sender->to_daemon_fp);
    }
    close(fd);

    printf("\n=== Sent message ===\n");
    printf("Sender: %s\nType: %d, Msg: %s\n", sender->to_daemon_fp, GET_TYPE(draft), SAY_MSG_INDEX(draft));
    if (msg_type == Saycont) {
        printf("Terminate: %d\n", ((BYTE*)draft)[2047]);
    }

    // struct timeval timeout;
    // timeout.tv_sec = 0;
    // timeout.tv_usec = 50000;
    // select(0, NULL, NULL, NULL, &timeout);  

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
        
    } else if (type == Ping) {
        printf("Type: %d\nFrom: %s\nContent: %s\n", type, from, content);

    } else {
        perror("Message received neither RECEIVE, RECVCONT, nor PING");
        return;
    }
}

void receive(Pipeline * receiver) {
    errno = 0;
    int fd = open(receiver->to_client_fp, O_RDONLY);
    if (fd <= 0) {
        perror("receive(): fail open");
        fprintf(stderr, "open failed for: %s\n", receiver->to_client_fp);
        return;
    }

    char msg[BUF_SIZE];
    if (read(fd, msg, BUF_SIZE) <= 0) {
        perror("receive(): failed read");
        fprintf(stderr, "receive failed for: %s\n", receiver->to_client_fp);
        return;
    }
    close(fd);

    print_msg(msg, receiver);
}


/*  Creates message for connection in the format:
    CONNECT <identifier> <domain>
 */
void connect_to_server(Pipeline * pline) {
    sleep(1);

    char draft[BUF_SIZE] = {0};
    SET_TYPE(draft, Connect);
    
    char * draft_iden = IDEN_INDEX(draft);
    strcpy(draft_iden, pline->iden);
    char * draft_domain = DOMAIN_INDEX(draft);
    strcpy(draft_domain, pline->domain);

    int fd = open(CHANNEL_NAME, O_WRONLY);
    write(fd, draft, BUF_SIZE);
    close(fd);

    sleep(1);
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
