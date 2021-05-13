#define _POSIX_SOURCE

#include "server.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

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


enum type {
    Connect,
    Say,
    Recvcont,
    Disconnect,
    Ping,
    Pong
};

enum type get_type(char * string) {
    if (string[0] == 0) {
        BYTE ret = string[1];
        return ret;
    }
    return -1;
}

// Return pointer to first character of "identifier"
char * get_identifier(char * string) {
    // Only 256 characters !
    return &string[IDEN_IX];
}

// Return pointer to first character of the "domain"
char * get_domain(char * string) {
    return string + DOMAIN_IX;
}

/*  Handles async monitoring of client, reporting back to global process
 */
int daemon(int fd_dae_WR, int fd_dae_RD) {
    /*
    
    Daemon stores shared memory of "fd_set allfds" for its own domain
    with read and write set of fds.

    When a message is received, it can pipe directly to all other daemons of the
    same domain. Using to_client pipe, the current daemon sends <RECEIVE> 
    message to other daemons.

    They will handle independently.

    */


    return 0;
}

/*  Takes in "2046 + 7 = 2047" characters to setup DAEMON.
 *      - Creates domain (if non-existant)
 *      - Creates two FIFOs (read and write), connecting DAEMON <-> client
 *      - Starts DAEMON function: monitors read pipe
 *
 *  Return: 0 = child terminated
 *          1 = success
 *          -1 = fail
 */
pid_t global_et_client(char * buffer) {
    // Try connecting
    if (get_type(buffer) != Connect) {
        printf("\nType is not connect");
        return -1;
    }
    //DEBUG*/ printf("building client....\n");
    //DEBUG*/ printf("msg length: %ld\n", strlen(msg + 2));
    // for (int i = 0; i < 2048; i++) {
    //     printf("%d ", msg[i]);
    // }

    // for (int i = 0; i < 2048; i++) {
    //     printf("%c ", msg[i]);
    // }

    // Make domain
    char domain_str[BUF_SIZE];
    strncpy(domain_str, get_domain(buffer), DOMAIN_LEN);  // domain is maximum 255

    //DEBUG*/ printf("domain_str: %s\ndomain len: %ld\n", domain_str, strlen(domain_str));
    if ( -1 == mkdir(domain_str, 0777) ) {
        //@todo: check exists using errno
        //DEBUG*/ printf("Domain exists or Cannot be made\n"); // domain maps to something
        // return -1;
    }

    // File path to FIFO
    char to_client_fp[BUF_SIZE];
    strcpy(to_client_fp, "");
    //DEBUG*/ printf("Starting string: %s\n", to_client_fp);
    char to_daemon_fp[BUF_SIZE];

    strcat(to_client_fp, get_domain(buffer));              // domain
    strcat(to_client_fp, "/");                          // domain/
    strcat(to_client_fp, get_identifier(buffer));          // domain/identifier
    strcat(to_daemon_fp, to_client_fp);                 // domain/identifier

    //DEBUG*/ printf("%s\n", to_client_fp);
    //DEBUG*/ printf("%s\n", to_daemon_fp);

    strcat(to_daemon_fp, "_WR");                        // domain/identifier_RD
    strcat(to_client_fp, "_RD");                        // domain/identifier_RD
    
    // strncpy(to_client_fp, "\0", 0); // add terminating character no matter what
    // strncpy(to_daemon_fp, "\0", 0); 

    // Starting FIFO
    //DEBUG*/ printf("%s\n", to_client_fp);
    //DEBUG*/ printf("%s\n", to_daemon_fp);
    if ( mkfifo(to_client_fp, S_IRWXU | S_IRWXG) == -1 ) {
        //DEBUG*/ printf("Could not make pipe to client (RD)\n");
        return -1;
    }
    //DEBUG*/ printf("made PIPE 1\n");
    if ( mkfifo(to_daemon_fp, S_IRWXU | S_IRWXG) == -1 ) {
        //DEBUG*/ printf("Could not make pipe to daemon (WR)\n");
        return -1;
    }
    //DEBUG*/ printf("made PIPE 2\n");

    // Begin forking...
    pid_t pid = fork();
    if (pid < 0) {
        printf("\nCould not fork.");
        return -1;
    }

    // Child process: daemon
    if (pid == 0) {
        // Open pipe as FD
        int fd_dae_WR = open(to_client_fp, O_NONBLOCK | O_WRONLY);
        int fd_dae_RD = open(to_daemon_fp, O_NONBLOCK | O_WRONLY);
        
        // Reading from client
        if (fd_dae_RD > 0) {
            FILE * read_channel = fdopen(fd_dae_RD, "r");
            char buf[BUF_SIZE];
            while( fgets(buf, BUF_SIZE, read_channel) != NULL ) {

            }
            /* After all write ports have been closed (from client side),
             * we exit the loop and will close the read port. */
            fclose( read_channel );
        }
        
        close(fd_dae_WR);
        close(fd_dae_RD);
        return 0;

    } else {
    // Global processes: mother
        return 1;
    }

    return 1;
}

int start_daemon(int gevent_fd) {
    char buffer[BUF_SIZE];

    ssize_t nread = read(gevent_fd, buffer, sizeof(buffer));
    if (nread == -1) {
        printf("Failed to read\n");
        return -1;
    }

    printf("%s\n", buffer);
    return 1;

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
        } else {
            printf("Domain cannot be created\n");
            return -1;
        }
    }

    // File path to FIFO
    char to_client_fp[BUF_SIZE];
    char to_daemon_fp[BUF_SIZE];
    // strcpy(to_client_fp, "");

    strcat(to_client_fp, get_domain(buffer));              // domain
    strcat(to_client_fp, "/");                          // domain/
    strcat(to_client_fp, get_identifier(buffer));          // domain/identifier
    strcat(to_daemon_fp, to_client_fp);                 // domain/identifier

    strcat(to_daemon_fp, "_WR");                        // domain/identifier_RD
    strcat(to_client_fp, "_RD");                        // domain/identifier_RD
    
    // Starting FIFO
    if ( mkfifo(to_client_fp, 0777) == -1 ) {
        return -1;
    }
    //DEBUG*/ printf("made PIPE 1\n");
    if ( mkfifo(to_daemon_fp, 0777) == -1 ) {
        return -1;
    }

    return 0;

    // Begin forking...
    // pid_t pid = fork();
    // if (pid < 0) {
    //     printf("\nCould not fork.");
    //     return -1;
    // }

    // Child process: daemon
    // if (pid == 0) {
    //     // Open pipe as FD
    //     int fd_dae_WR = open(to_client_fp, O_NONBLOCK | O_WRONLY);
    //     int fd_dae_RD = open(to_daemon_fp, O_NONBLOCK | O_WRONLY);
        
    //     // Reading from client
    //     if (fd_dae_RD > 0) {
    //         FILE * read_channel = fdopen(fd_dae_RD, "r");
    //         char buf[BUF_SIZE];
    //         while( fgets(buf, BUF_SIZE, read_channel) != NULL ) {

    //         }
    //         /* After all write ports have been closed (from client side),
    //          * we exit the loop and will close the read port. */
    //         fclose( read_channel );
    //     }
        
    //     close(fd_dae_WR);
    //     close(fd_dae_RD);
    //     return 0;

    // } else {
    // // Global processes: mother
    //     return 1;
    // }

    return 1;

}

int main() {
	printf("welcome\n");

	if ((mkfifo("gevent", 0777) < 0)) {
		perror("Cannot make fifo");
	}
	printf("Made pipe !\n");


	int gevent_fd = open("gevent", O_RDONLY);
	if (gevent_fd < 0) {
		perror("Failed to open gevent FD");
		return 1;
	}
	printf("Opened pipe FD !\n");

	int maxfd = gevent_fd + 1;

	fd_set allfds;
	struct timeval timeout;

	int i = 0;
	printf("Starting while loop");

	while (1)
	{
		printf("%d\n", i++);

		FD_ZERO(&allfds); //   000000
		FD_SET(gevent_fd, &allfds); // 100000
        
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		
		int ret = select(maxfd, &allfds, NULL, NULL, &timeout);

		if (-1 == ret) {
			fprintf(stderr, "Error from select\n");	

		} else if (0 == ret) {
			printf("Nothing to report\n");

		} else if (FD_ISSET(gevent_fd, &allfds)) {

			// Start reading
            start_daemon(gevent_fd);

			// char buffer[BUF_SIZE];
			// int nread = read(gevent_fd, buffer, BUF_SIZE);

			// if (-1 == nread) {
			// 	perror("failed to read");
			// } else {
			// 	// buffer[nread] = '\0';
			// 	printf("received %s\n", buffer);
			// 	// global_et_client(buffer);
				
			// }
		}
	}

	return 0;
}







    // char arg0[ARG_SIZE];
	// char arg1[ARG_SIZE];
	// char buf[BUF_SIZE];
	// char* n_check = NULL;
	// pid_t pids[PID_LIST_SIZE];
	// size_t n_procs = 0;
	
	// while(1) {
	// 	printf("> ");
	// 	n_check = fgets(buf, BUF_SIZE, stdin);
	// 	// sscanf(buf, "%s %s", arg0, arg1);
		
	// 	for(size_t i = 0; i < PID_LIST_SIZE; i++) {

	// 		// Check on all children
	// 		int status = 0;
	// 		if(waitpid(pids[i], &status, WNOHANG) == -1) {
	// 			shift_down(pids, PID_LIST_SIZE, i);
	// 		}
	// 		// Have to check for exit status
	// 		if(WIFEXITED(status)) {
	// 			shift_down(pids, PID_LIST_SIZE, i);
	// 		}
	// 	}
		
	// 	// if(strcmp(arg0, "exit") == 0) {
	// 	// 	break;
	// 	// }
		
	// 	if(n_check != NULL) {
	// 		pid_t pid = fork();
	// 		if(pid == 0) {
	// 			snprintf(buf, BUF_SIZE, BIN_PATH"%s", arg0);
	// 			execl(buf, arg0, arg1, NULL);
	// 			return 0;
	// 		}
	// 	}
