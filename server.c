

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
    if (string[0] == 0) {
        BYTE ret = string[1];
        return ret;
    } else if (string[1] == 0) {
        BYTE ret = string[0];
        return ret;
    }
    return -1;
}

void set_type(char * string, enum type t) {
    string[0] = t;
    string[1] = t;
}

typedef struct pipeline {
    int to_client_fp;
    int to_daemon_fp;
} Pipeline;

// Return pointer to first character of "identifier"
char * get_identifier(char * string) {
    // Only 256 characters !
    return &string[IDEN_IX];
}

// Return pointer to first character of the "domain"
char * get_domain(char * string) {
    return string + DOMAIN_IX;
}


/*  Relays <RECEIVE IDENTIFIER MSG> to client */
int do_receive(char * buffer, const char * to_client_fp) {
    perror("Doing receive");
    if (get_type(buffer) != Receive) {
        return -1;
    }

    int fd = open(to_client_fp, O_NONBLOCK, O_WRONLY);
    if (write(fd, buffer, 2048) < 0) {
        perror("Failed to write");
    } else {
        // DEBUG */ printf("wrote to client\n");
        // DEBUG */ printf("From: %s\nMsg: %s\n", buffer + 2, buffer + 2 + 256);
    }
    close(fd);
    return 1;
}


/*
Takes in buffer for maximum 2048 characters.
*/
int do_say(char * buffer, const char * domain, const char * to_daemon_fp, const char * to_client_fp) {
    // Find all other client handlers in current domain
    struct dirent *de;              // Pointer for directory entry
    DIR *dr = opendir(domain);      // opendir() returns a pointer of DIR type. 
  
    if (dr == NULL) { // opendir returns NULL if couldn't open directory
        perror("do_say: Could not open current directory" );
        return -1;
    }
  
    while ((de = readdir(dr)) != NULL) {
        //DEBUG*/printf("\n**** Directory: %s ****\n", de->d_name);
        
        char * filename = de->d_name;
        long filenm_len = strlen(filename);
        if (strlen(filename) <= 2) {
            //DEBUG*/perror("Filename too short");
            continue;
        }
        
        // Directly write to client RD
        if (filename[filenm_len - 2] == 'W' && filename[filenm_len - 1] == 'R') {

            char pipepath[BUF_SIZE];
            strcpy(pipepath, domain);
            strcat(pipepath, "/");
            strcat(pipepath, filename);

            // Skip write pipes to own client
            
            if (strcmp(pipepath, to_daemon_fp) == 0) {  
                //DEBUG*/printf("# Cannot write to self\n");
                continue;
            }

            //DEBUG*/printf("Writing from: %s :: %s\n", to_daemon_fp, pipepath);
            // Writing now
            int fd = open(pipepath, O_WRONLY);
            if (fd < 0) {
                perror("do_say: Error in piping message to other clients");
                return -1;
            }

            // Build RECEIVE message: RECEIVE <identifier> <message>
            char draft[BUF_SIZE] = {0};
            set_type(draft, Receive);

            const char * identity = FILEPATH_TO_IDEN(to_client_fp, domain);
            strncpy(draft + 2, identity, strlen(identity) - 3); // To remove _RD
            draft[2 + strlen(identity) - 3] = '\0';
            strcpy(draft + 2 + 256, SAY_MSG_INDEX(buffer));

            // Write to other clients
            if (write(fd, draft, 2048) < -1) {
                perror("Failed writing");
            }

            // CHECKING MESSAGE SENT PROPERLY
            // printf("Type = ");
            // if (get_type(draft) == Receive) {
            //     printf("Receive\n");
            // } else {
            //     printf("ERROR\n");
            // }
            // printf("iden: %s\n", draft + 2);
            // printf("msg: %s\n\n", draft + 2 + 256);
            ////////
            
            close(fd);
        }
    }
    //DEBUG*/printf("Finished reading directory\n");
    closedir(dr);
    return 0;
}

/*


*/
int handle_daemon_update(int fd_dae_RD, int fd_dae_WR, 
                          const char * to_client_fp, const char * to_daemon_fp,
                          const char * domain)
{
    // printf("@@@@@@@@@ %d @@@@@@@@@\n", getpid());
    char buffer[BUF_SIZE];
    int nread = read(fd_dae_RD, buffer, BUF_SIZE);
    if (nread == -1) {
        printf("Failed to read\n");
        return -1;
    }
    
    // Check message type
    if ( get_type(buffer) == Say) {
        // DEBUG*/printf("\n==== doing say ====\n");
        
        int st = do_say(buffer, domain, to_daemon_fp, to_client_fp); // write to other daemons

        if (st == -1) {
            perror("Failed do_say");
            return -1;
        }

    } else if ( get_type(buffer) == Saycount) {
    } else if ( get_type(buffer) == Receive) {
        // DEBUG */ printf("\n===== doing receive ====\n");
        do_receive(buffer, to_client_fp);
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
int start_daemon(int gevent_fd) {
    char buffer[BUF_SIZE];

    ssize_t nread = read(gevent_fd, buffer, sizeof(buffer));
    if (nread == -1) {
        printf("Failed to read\n");
        return -1;
    }

    // printf("%s\n", buffer);
    // return 1;

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
            // Domain exists
            errno = 0;
        } else {
            printf("Domain cannot be created\n");
            return -1;
        }
    }

    // File path to FIFO
    char to_client_fp[BUF_SIZE];
    char to_daemon_fp[BUF_SIZE];

    strncpy(to_client_fp, get_domain(buffer), 256);           // domain
    strcat(to_client_fp, "/");                          // domain/

    strcat(to_client_fp, get_identifier(buffer));          // domain/identifier
    strcpy(to_daemon_fp, to_client_fp);                 // domain/identifier

    strcat(to_daemon_fp, "_WR");                        // domain/identifier_WR
    strcat(to_client_fp, "_RD");                        // domain/identifier_RD
    
    // Starting FIFO
    // printf("%s\n", to_client_fp);
    // printf("%s\n", to_daemon_fp);
    // @TODO: overwrite existing pipe if needed
    if ( mkfifo(to_client_fp, 0777) == -1 ) {
        // perror("Cannot make pipe to client");
        errno = 0;
        return -1;
    }
    if ( mkfifo(to_daemon_fp, 0777) == -1 ) {
        // perror("Cannot make pipe to daemon");
        errno = 0;
        return -1;
    }

    // ========== FORKING ========== //
    pid_t pid = fork();
    if (pid < 0) {
        printf("Could not fork\n");
        return -1;
    }

    // Tell parent success
    if (pid != 0) {
        // printf("@@@@@@@@@ PARENT: %d @@@@@@@@@\n", getpid());
        return 0;
    }
    errno = 0;
    //DEBUG*/printf("Child process started...\n");

    // Child process: daemon
    // printf("@@@@@@@@@ CHILD: %d @@@@@@@@@\n", getpid());
    close(gevent_fd);

    // Open pipe as FD
    int fd_dae_WR = open(to_client_fp, O_NONBLOCK, O_WRONLY);
    int fd_dae_RD = open(to_daemon_fp, O_NONBLOCK, O_RDONLY);
    if (fd_dae_RD < 0 || fd_dae_WR < 0) {
		perror("Failed to open gevent FD");
		return 1;
	}
    
    // Reading from client
	fd_set allfds;
	int maxfd = fd_dae_RD + 1;
	struct timeval timeout;

    // ========= Monitoring client =========
    //DEBUG*/printf("Begin monitoring client...\n");
	while (1)
	{
		FD_ZERO(&allfds); //   000000
		FD_SET(fd_dae_RD, &allfds); // 100000
        
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
        timeout = timeout;
		
		int ret = select(maxfd, &allfds, NULL, NULL, NULL);

        // DEBUG*/printf("\n !!!!!!!! UPDATE !!!!!!!! \n");
		if (-1 == ret) {
			fprintf(stderr, "Error from select");	
            //@todo: return here
		} else if (0 == ret) {
			perror("Client said nothing...");

		} else if (FD_ISSET(fd_dae_RD, &allfds)) {
			// Start reading from clients
            //DEBUG*/printf("Handling message...\n");
            int succ = handle_daemon_update(fd_dae_RD, fd_dae_WR,
                                            to_client_fp, to_daemon_fp,
                                            domain_str);
            if (succ == -1) {
                return -1; //@TODO: change to something else
            }

		}
	}
    
    close(fd_dae_WR);
    close(fd_dae_RD);

    return 1;
}

int main() {
    // printf("hello");
	if ((mkfifo("gevent", 0777) < 0)) {
		perror("Cannot make fifo");
	}

	int gevent_fd = open("gevent", O_RDONLY);
	if (gevent_fd < 0) {
		perror("Failed to open gevent FD");
		return 1;
	}

	int maxfd = gevent_fd + 1;

	fd_set allfds;
	struct timeval timeout;


	while (1)
	{
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
            int dae = start_daemon(gevent_fd);
            if (dae == 0) // child successfully created
                continue;
            else if (dae == 1) {
                // child died
                return 0;
            }
		}
	}

    
    // run_daemon(int fd_RD, int fd_WR);
	return 0;
}