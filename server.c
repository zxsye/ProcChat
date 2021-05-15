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
#define FILEPATH_TO_IDEN(filepath, domain) (filepath + strlen(domain) + 1)

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

enum type get_type(char * string) {
    return *(short*)string;
}

void set_type(char * string, enum type t) {
    string[0] = t;
    string[1] = 0;
}

typedef struct pipeline {
    char * domain;
    char * iden;
    char * to_client_fp;
    char * to_daemon_fp;
} Pipeline;

// Return pointer to first character of "identifier"
char * get_iden(char * string) {
    // Only 256 characters !
    return string + IDEN_IX;
}

// Return pointer to first character of the "domain"
char * get_domain(char * string) {
    return string + DOMAIN_IX;
}


/*  Relays <RECEIVE IDENTIFIER MSG> to client */
int do_receive(char * buffer, Pipeline * pline) {
    //DEBUG*/perror("Doing receive");
    //DEBUG*/errno = 0;

    if (get_type(buffer) != Receive) {
        return -1;
    }

    int fd = open(pline->to_client_fp, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "do_receive: cannot open %s\n", pline->to_client_fp);
        return -1;
    }
    if (write(fd, buffer, 2048) < 0) {
        fprintf(stderr, "do_receive: cannot write()\n");
        fprintf(stderr, "Target: %s\n", pline->to_client_fp);
        fprintf(stderr, "Identifer: %s\n", buffer + 2);
        fprintf(stderr, "Msg: %s\n\n", buffer + 2 + 256);
    }

    close(fd);
    return 1;
}

/*  Relays <RECEIVE IDENTIFIER MSG> to client */
int do_recvcont(char * buffer, Pipeline * pline) {
    if (get_type(buffer) != Recvcont) { 
        return -1;
    }

    int fd = open(pline->to_client_fp, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "do_receive: cannot open %s\n", pline->to_client_fp);
        return -1;
    }

    if (write(fd, buffer, 2048) < 0) {
        fprintf(stderr, "do_receive: cannot write()\n");
    }
    close(fd);

    return 0;
}


/*
Takes in buffer for maximum 2048 characters.
*/
int do_say(char * buffer, Pipeline * pline) {
    // Find all other client handlers in current domain
    struct dirent *de;              // Pointer for directory entry
    DIR *dr = opendir(pline->domain);      // opendir() returns a pointer of DIR type. 
  
    if (dr == NULL) { // opendir returns NULL if couldn't open directory
        perror("do_say: Could not open current directory" );
        return -1;
    }
  
    while ((de = readdir(dr)) != NULL) {
        
        char * filename = de->d_name;
        long filenm_len = strlen(filename);
        if (strlen(filename) <= 2) {
            continue;
        }
        
        // Directly write to other client handlers (WR)
        if (filename[filenm_len - 2] == 'W' && filename[filenm_len - 1] == 'R') {

            char pipepath[BUF_SIZE];
            strcpy(pipepath, pline->domain);
            strcat(pipepath, "/");
            strcat(pipepath, filename);

            // Skip write pipes to own client
            if ( strcmp(pipepath, pline->to_daemon_fp) == 0 ) continue;

            // Build RECEIVE message: RECEIVE <identifier> <message>
            char draft[BUF_SIZE] = {0};
            set_type(draft, Receive);

            const char * identity = pline->iden;
            strncpy(draft + 2, identity, strlen(identity) - 3); // To remove _RD
            draft[2 + strlen(identity) - 3] = '\0';
            strcpy(draft + 2 + 256, SAY_MSG_INDEX(buffer));

            // Write to other clients
            int fd = open(pipepath, O_WRONLY);
            if (fd < 0) {
                perror("do_say: Error in piping message to other clients");
                return -1;
            }
            if (write(fd, draft, 2048) < -1) {
                perror("Failed writing");
            }
            close(fd);
            
        }
    }

    closedir(dr);
    return 0;
}

/*
Takes in buffer for maximum 2048 characters.
*/
int do_saycount(char * msg, Pipeline * pline) {


    if (get_type(msg) != Saycount) {
        fprintf(stderr, "Failed do_saycount:\n");
        return -1;
    }

    // Find all other client handlers in current domain
    struct dirent *de;              // Pointer for directory entry
    DIR *dr = opendir(pline->domain);      // opendir() returns a pointer of DIR type. 
  
    if (dr == NULL) { // opendir returns NULL if couldn't open directory
        perror("do_say: Could not open current directory" );
        return -1;
    }
  
    while ((de = readdir(dr)) != NULL) {
        char * filename = de->d_name;
        long filenm_len = strlen(filename);
        if (strlen(filename) <= 2) {
            continue;
        }
        
        // Directly write to client RD
        if (filename[filenm_len - 2] == 'W' && filename[filenm_len - 1] == 'R') {

            char pipepath[BUF_SIZE];
            strcpy(pipepath, pline->domain);
            strcat(pipepath, "/");
            strcat(pipepath, filename);

            // Skip write pipes to own client
            
            if (strcmp(pipepath, pline->to_daemon_fp) == 0) {  
                continue;
            }

            // Build RECEIVE message: RECEIVE <identifier> <message>
            char draft[BUF_SIZE] = {0};
            set_type(draft, Recvcont);

            const char * identity = pline->iden;
            strncpy(draft + 2, identity, strlen(identity) - 3); // To remove _RD
            draft[2 + strlen(identity) - 3] = '\0';
            strcpy(draft + 2 + 256, SAY_MSG_INDEX(msg));

            draft[BUF_SIZE - 1] = msg[BUF_SIZE - 1]; // terminating character

            // Write to other clients
            int fd = open(pipepath, O_WRONLY);
            if (fd < 0) {
                perror("do_say: Error in piping message to other clients");
                return -1;
            }
            if (write(fd, draft, 2048) < 0) {
                perror("Failed writing");
            }
            close(fd);

        }
    }
    //DEBUG*/printf("Finished reading directory\n");
    closedir(dr);
    return 0;
}

/*


*/
int handle_daemon_update(char * buffer, Pipeline * pline) {
    
    
    // Check message type
    if ( get_type(buffer) == Say) {
        int st = do_say(buffer, pline); // write to other daemons

        if (st == -1) {
            perror("Failed do_say");
            return -1;
        }

    } else if ( get_type(buffer) == Saycount) {
        int st = do_saycount(buffer, pline); // write to other daemons

        if (st == -1) {
            perror("Failed do_say");
            return -1;
        }
    } else if ( get_type(buffer) == Receive) {
        // DEBUG */ printf("\n===== doing receive ====\n");
        do_receive(buffer, pline);
        
    } else if ( get_type(buffer) == Recvcont) {
        do_recvcont(buffer, pline);

    } else {
        fprintf(stderr, "Not implemented type");
    }
    return 0;
}

/*
Takes in fd for gevent, reads latest message from pipe to construct new pipes for client.

- Return -1: tell parent failure
- Return 0: tell parent of birth success 
- Return 1: tell child's main of suicide
- Return 2: 

*/
int start_daemon(char * buffer) {

    if (get_type(buffer) != Connect) {
        printf("Type is not connect\n");
        return -1;
    }

    // Make domain
    char domain_str[BUF_SIZE];
    strncpy(domain_str, get_domain(buffer), DOMAIN_LEN);  // domain is maximum 255

    // Make domain directory
    if ( -1 == mkdir(domain_str, 0777) ) {
        if (errno == EEXIST) {
            errno = 0;
        } else {
            printf("Domain cannot be created\n");
            return -1;
        }
    }

    // File path to FIFO
    char domain[256];
    char iden[256];
    strncpy(domain, get_domain(buffer), DOMAIN_LEN);  // domain is maximum 255
    strcpy(iden, get_iden(buffer));
    fprintf(stderr, "%s, %s\n", domain, iden);

    char to_client_fp[BUF_SIZE];
    char to_daemon_fp[BUF_SIZE];
    strcpy(to_client_fp, domain);           // domain
    strcat(to_client_fp, "/");                          // domain/
    strcat(to_client_fp, iden);          // domain/identifier

    strcpy(to_daemon_fp, to_client_fp);                 // domain/identifier

    strcat(to_daemon_fp, "_WR");                        // domain/identifier_WR
    strcat(to_client_fp, "_RD");                        // domain/identifier_RD
    
    Pipeline pline;
    pline.domain = domain;
    pline.iden = iden;
    pline.to_client_fp = to_client_fp;
    pline.to_daemon_fp = to_daemon_fp;
    pline = pline;

    // Starting FIFO
    if ( mkfifo(to_client_fp, 0777) == -1 ) {
        perror("Cannot make pipe to client");
        errno = 0;
        return -1;
    }
    if ( mkfifo(to_daemon_fp, 0777) == -1 ) {
        perror("Cannot make pipe to daemon");
        errno = 0;
        return -1;
    }


    // Open pipe as FD

    // ========= Monitoring client =========
	while (1)
	{
        int fd_dae_RD = open(to_daemon_fp, O_RDWR);
        if (fd_dae_RD < 0) {
            perror("Failed to open FIFO to/from client");
            return 1;
        }
        
        // Reading from client
        fd_set allfds;
        int maxfd = fd_dae_RD + 1;
        struct timeval timeout;

		FD_ZERO(&allfds); //   000000
		FD_SET(fd_dae_RD, &allfds); // 100000
        
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
        timeout = timeout;
		
		int ret = select(maxfd, &allfds, NULL, NULL, NULL);

        // ======= NEW UPDATE =======
		if (-1 == ret) {
			fprintf(stderr, "Error from select");	
		} else if (0 == ret) {
			perror("Client said nothing...");

		} else if (FD_ISSET(fd_dae_RD, &allfds)) {

            char buffer[BUF_SIZE];
            int nread = read(fd_dae_RD, buffer, BUF_SIZE);
            if (nread == -1) {
                printf("Failed to read\n");
                return -1;
            }

            int succ = handle_daemon_update(buffer, &pline);
            if (succ == -1) {
                return -1; //@TODO: change to something else
            }

		}
        close(fd_dae_RD);
	}
    
    // close(fd_dae_WR);

    return 1;
}

/* Gevent monitor
*/
int main() {
    // printf("hello");
	if ((mkfifo("gevent", 0777) < 0)) {
		perror("Cannot make fifo");
	}

	while (1)
	{
        int gevent_fd = open("gevent", O_RDWR);
        if (gevent_fd < 0) {
            perror("Failed to open gevent FD");
            return 1;
        }

        int maxfd = gevent_fd + 1;

        fd_set allfds;
        struct timeval timeout;

		FD_ZERO(&allfds); //   000000
		FD_SET(gevent_fd, &allfds); // 100000
        
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		timeout = timeout;

		int ret = select(maxfd, &allfds, NULL, NULL, NULL);

		if (-1 == ret) {
			fprintf(stderr, "Error from select\n");	
		} else if (0 == ret) {
			perror("Nothing to report");

		} else if (FD_ISSET(gevent_fd, &allfds)) {
			// Start reading
            char buffer[BUF_SIZE];
            ssize_t nread = read(gevent_fd, buffer, sizeof(buffer));
            if (nread == -1) {
                printf("Failed to read\n");
                return -1;
            }
            pid_t pid = fork();
            if (pid < 0) {
                printf("Could not fork\n");
                return -1;
            }

            // Parent continues
            if (pid != 0) {
                close(gevent_fd);
                continue;
            } else {
            // Child starts daemon
                int dae = start_daemon(buffer);
                if (dae == 1) {
                    break;
                }
            }
		}

	}


    // run_daemon(int fd_RD, int fd_WR);
	return 0;
}