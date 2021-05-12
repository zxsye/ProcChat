#define _POSIX_SOURCE

#include "server.h"
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <sys/types.h> 
#include <sys/stat.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#define BYTE unsigned char
#define BUF_SIZE (2049)
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
    printf("%s\n", string);
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
    return &string[DOMAIN_IX];
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
 */
pid_t global_et_client(char * msg) {
    // Try connecting
    if (get_type(msg) != Connect) {
        return -1;
    }

    // Make domain
    char domain_str[BUF_SIZE];
    strncpy(domain_str, get_domain(msg), DOMAIN_LEN);  // domain is maximum 255
    // strncpy(domain_str, "\0", 0); 

    if ( -1 == mkdir(domain_str, 0777) ) {
        perror("Cannot make directory..."); // domain maps to something
    }; 

    // File path to FIFO
    char to_client_fp[BUF_SIZE];
    strncpy(to_client_fp, get_domain(msg), DOMAIN_LEN); // domain
    strcpy(to_client_fp, "/");                          // domain/
    strncpy(to_client_fp, get_identifier(msg), IDEN_LEN);     // domain/identifier

    char to_daemon_fp[BUF_SIZE];
    strcpy(to_daemon_fp, to_client_fp);                 // domain/identifier
    strcpy(to_daemon_fp, "_WR");                        // domain/identifier_RD
    strcpy(to_client_fp, "_RD");                        // domain/identifier_RD
    
    // strncpy(to_client_fp, "\0", 0); // add terminating character no matter what
    // strncpy(to_daemon_fp, "\0", 0); 

    // Starting FIFO
    if ( mkfifo(to_client_fp, S_IRWXU | S_IRWXG) == -1 ) {
        perror("Could not make pipe to client (RD)");
        return -1;
    }
    if ( mkfifo(to_daemon_fp, S_IRWXU | S_IRWXG) == -1 ) {
        perror("Could not make pipe to daemon (WR)");
        return -1;
    }

    // Begin forking...
    pid_t pid = fork();
    if (pid < 0) {
        perror("Could not fork.");
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

    } else {
    // Global processes: mother
        
        return -1;
    }

    return 0;
}


int main(int argc, char** argv) {

    // Open "gevent" FIFO pipe
    if((mkfifo(CHANNEL_NAME, S_IRWXU | S_IRWXG)) >= 0) { 

        // Read `gevent`
        int fd = open(CHANNEL_NAME, O_RDONLY);
        
        if(fd > 0) {
            FILE * read_channel = fdopen(fd, "r"); 
            char buf[BUF_SIZE]; 

            if (fgets(buf, BUF_SIZE, read_channel) != NULL) {
                global_et_client(buf);


            } else {
                perror("Message is wrong");
            }
                // split string
                // find command id
                // execute command 
                // make new process???
                // set up pipes...


            printf("closing pipe...\n");
            fclose(read_channel); 
        }
        close(fd);
    } else { 
        fprintf(stderr, "Unable to open pipe");
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
