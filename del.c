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



int start_daemon(int gevent_fd) {
    ssize_t nread;
    char buffer[BUF_SIZE];

    nread = read(gevent_fd, buffer, sizeof(buffer));
    if (nread == -1) {
        printf("Failed to read\n");
        return -1;
    }

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

	fd_set allfds;
	FD_ZERO(&allfds);
	FD_SET(gevent_fd, &allfds);
	int maxfd = 0 + 1;

	struct timeval timeout;

	int i = 0;
	printf("Starting while loop");
	while (1)
	{
		printf("%d\n", i++);
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		
		FD_ZERO(&allfds); //   000000
		FD_SET(gevent_fd, &allfds); // 100000
		int ret = select(maxfd, &allfds, NULL, NULL, &timeout);

		if (-1 == ret) {
			fprintf(stderr, "Error from select\n");	

		} else if (0 == ret) {
			printf("Nothing to report\n");

		} else {
			// Read from gevent
			printf("%d file descriptors ready\n", ret);

			int modified = FD_ISSET(gevent_fd, &allfds);
			printf("modified: %d\n", modified);

			// Start reading
			char buffer[BUF_SIZE];
			int nread = read(gevent_fd, buffer, BUF_SIZE);

			if (-1 == nread) {
				perror("failed to read");
			} else {
				// buffer[nread] = '\0';
				printf("received %s\n", buffer);
				if (strncmp(buffer, "quit", 4) == 0)
					break;
			}
		}
	}

	return 0;
}