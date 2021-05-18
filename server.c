#include "server.h"
#include <string.h>

/*  Takes in "Connect" type message and converts to filepath string to open pipes
 */
int get_filepath(char * buffer, Pipeline * pline) {

    char * domain = GET_DOMAIN(buffer);
    if (strlen(domain) >= 256) {
        perror("Domain cannot exceed 255 characters");
        return -1;
    }
    strncpy(pline->domain, domain, DOMAIN_LEN); 
    char * iden = GET_DOMAIN(buffer);
    if (strlen(iden) >= 256) {
        perror("Identifier cannot exceed 255 characters");
        return -1;
    }
    strcpy(pline->iden, IDEN(buffer));

    strcpy(pline->to_client_fp, pline->domain);                       // domain
    strcat(pline->to_client_fp, "/");                          // domain/
    strcat(pline->to_client_fp, pline->iden);                         // domain/identifier

    strcpy(pline->to_daemon_fp, pline->to_client_fp);                 // domain/identifier

    strcat(pline->to_daemon_fp, "_WR");                        // domain/identifier_WR
    strcat(pline->to_client_fp, "_RD");                        // domain/identifier_RD
    return 0;
}

/*  Send a 2048 byte message to the client
 */
int send_to_client(char * msg, Pipeline * pline) {
    int ret = 0;
    int fd = open(pline->to_client_fp, O_WRONLY);

    if (fd < 0) {
        fprintf(stderr, "send_to_client: cannot open %s\n", pline->to_client_fp);
        ret = -1;
    }
    if (write(fd, msg, 2048) < 0) {
        ret = -1;
    }
    close(fd);
    return ret;
}

/*  Relays <RECEIVE IDENTIFIER MSG> to client */
int do_receive(char * buffer, Pipeline * pline) {
    if (GET_TYPE(buffer) != Receive) return -1;
    
    int ret = send_to_client(buffer, pline);
    if (ret == -1) return -1;

    return 0;
}

/*  Relays <RECEIVE IDENTIFIER MSG> to client */
int do_recvcont(char * buffer, Pipeline * pline) {
    if (GET_TYPE(buffer) != Recvcont) return -1;

    int ret = send_to_client(buffer, pline);
    if (ret == -1) return -1;

    return 0;
}

/*  Broadcast RECEIVE or RECVCONT to other clients in current domain
        - Find all other client handlers in current domain
 */
int domain_broadcast(char * msg, Pipeline * pline) {
    struct dirent *de;                     // Pointer for directory entry
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


            // Write to other clients
            int fd = open(pipepath, O_WRONLY);
            if (fd < 0) {
                perror("do_say: Error in piping message to other clients");
                return -1;
            }
            if (write(fd, msg, 2048) < 0) {
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
int do_say(char * in_mail, Pipeline * pline) {
    if (GET_TYPE(in_mail) != Say) {
            fprintf(stderr, "Failed do_saycount:\n");
            return -1;
        }

    char draft[BUF_SIZE] = {0};
    SET_TYPE(draft, Receive);

    strcpy(draft + 2, pline->iden); // To remove _RD
    char * msg = SAY_MSG_INDEX(in_mail);
    if (strlen(msg) >= 1790) {
        perror("Cannot SAY message more than 1789 characters");
        return -1;
    }
    strcpy(draft + 2 + 256, msg);

    domain_broadcast(draft, pline);
    return 0;
}

/*
Takes in buffer for maximum 2048 characters.
*/
int do_saycont(char * in_mail, Pipeline * pline) {
    if (GET_TYPE(in_mail) != Saycont) {
        fprintf(stderr, "Failed do_saycount:\n");
        return -1;
    }

    // Build RECEIVE message: RECEIVE <identifier> <message>
    char draft[BUF_SIZE] = {0};
    SET_TYPE(draft, Recvcont);

    strcpy(draft + 2, pline->iden); // To remove _RD
    char * msg = SAY_MSG_INDEX(in_mail);
    if (strlen(msg) >= 1789) {
        perror("Cannot SAYCONT message more than 1788 characters");
        return -1;
    }

    strcpy(draft + 2 + 256, SAY_MSG_INDEX(in_mail));

    draft[BUF_SIZE - 1] = in_mail[BUF_SIZE - 1]; // terminating character

    // Broadcast
    domain_broadcast(draft, pline);

    return 0;
}


/*  Send PING message to client */
int do_ping(char * ping, Pipeline * pline) {
    if (GET_TYPE(ping) != Ping) {
        return -1;
    }
    
    int ret = send_to_client(ping, pline);
    if (ret == -1)
        return -1;

    return 0;
}

/*
Handles all "normal operation" messages for daemon.
Otherwise return the message type.
*/
int daemon_protocol(char * buffer, Pipeline * pline) {
    // Check message type
    if (GET_TYPE(buffer) == Say) {
        if ( -1 == do_say(buffer, pline) ) {
            perror("Failed do_say");
            return -1;
        }
    } else if (GET_TYPE(buffer) == Saycont) {
        if ( -1 == do_saycont(buffer, pline) ) {
            perror("Failed do_say");
            return -1;
        }
    } else if ( GET_TYPE(buffer) == Receive) {
        if ( -1 == do_receive(buffer, pline) ) {
            perror("Failed to do_receive");
            return -1;
        }
    } else if ( GET_TYPE(buffer) == Recvcont) {
        if ( -1 == do_recvcont(buffer, pline) ) {
            perror("Failed do_recvcont");
            return -1;
        }
    } else if ( GET_TYPE(buffer) == Ping) {
        if ( -1 == do_ping(buffer, pline) ) {
            perror("Failed do_ping");
            return -1;
        }
    } else if ( GET_TYPE(buffer) == Pong) {
        return Pong;
    } else if ( GET_TYPE(buffer) == Disconnect) {
        return Disconnect;
    } else {
        fprintf(stdout, "Message is incorrect");
    }
    return 0;
}

/*  Takes in gevent buffer to construct daemon and FIFOs to handle new client.
    After successful construction begin running client handler.

        - Return -1: daemon has failed
        - Return DISCONNECT: daemon disconnected
*/
int run_daemon(char * buffer) {

    if (GET_TYPE(buffer) != Connect) {
        printf("Type is not connect\n");
        return -1;
    }

    // Make domain
    Pipeline pline;
    get_filepath(buffer, &pline);

    // Make domain directory
    if ( -1 == mkdir(pline.domain, 0777) ) {
        if (errno == EEXIST) 
            errno = 0;
        else {
            perror("Domain cannot be created");
            return -1;
        }
    }

    // Starting FIFO
    if ( mkfifo(pline.to_client_fp, 0777) == -1 ) {
        perror("Cannot make pipe to client");
        errno = 0;
        return -1;
    }
    if ( mkfifo(pline.to_daemon_fp, 0777) == -1 ) {
        perror("Cannot make pipe to daemon");
        errno = 0;
        return -1;
    }

    int client_alive = 1;
    struct timeval timeout;
    timeout.tv_sec = PPTIME;
    timeout.tv_usec = 0;

    // ========= Monitoring client =========
	while (1) {
        int fd_dae_RD = open(pline.to_daemon_fp, O_RDWR);
        if (fd_dae_RD < 0) {
            perror("Failed to open FIFO to/from client");
            break;
        }
        
        // Reading from client
        fd_set allfds;
        int maxfd;
        maxfd = fd_dae_RD + 1;

		FD_ZERO(&allfds); //   000000
		FD_SET(fd_dae_RD, &allfds); // 100000
		
		int select_ret = select(maxfd, &allfds, NULL, NULL, &timeout);

        // ======= NEW UPDATE =======
		if (select_ret < 0) {
            close(fd_dae_RD);
            perror("run_daemon: Failed select");
            continue;

		} else if (select_ret == 0 && client_alive) {
            close(fd_dae_RD);

            // Pong was received
            timeout.tv_sec = PPTIME;
            timeout.tv_usec = 0;
            client_alive = 0;

            // Send new ping
            char ping_draft[BUF_SIZE] ={0};
            *(short*)ping_draft = Ping;
            daemon_protocol(ping_draft, &pline);
            continue;

        } else if (select_ret == 0 && !client_alive) {
            close(fd_dae_RD);
            break;
        }

        // Read new client update
        char buffer[BUF_SIZE];
        int nread = read(fd_dae_RD, buffer, BUF_SIZE);
        close(fd_dae_RD);

        if (nread == -1) {
            perror("Failed to read");
            return -1;
        }

        // Handle update
        int dp = daemon_protocol(buffer, &pline);
        if (dp == -1) {
            return -1;
        } else if ( dp == Pong ) {
            client_alive = 1;
        } else if ( dp == Disconnect) {
            break;
        }

	}

    if (unlink(pline.to_daemon_fp) != 0)
        perror("Cannot close to_daemon_fp");
    if (unlink(pline.to_client_fp) != 0)
        perror("Cannot close to_client_fp");
    return Disconnect;
}

void handle_suicide(int signum) {
    wait(NULL);
}

/* Gevent monitor
*/
int main() {
	if ((mkfifo(CHANNEL_NAME, 0777) < 0)) {
		perror("Cannot make fifo");
        return 1;
	}
    signal(SIGUSR1, handle_suicide);

	while (1)
	{
        int gevent_fd = open(CHANNEL_NAME, O_RDWR);
        if (gevent_fd < 0) {
            perror("Failed to open gevent FD");
            return 1;
        }

        int maxfd = gevent_fd + 1;
        fd_set allfds;

		FD_ZERO(&allfds); //   000000
		FD_SET(gevent_fd, &allfds); // 100000

		int ret = select(maxfd, &allfds, NULL, NULL, NULL);
		if (ret <= 0 || !FD_ISSET(gevent_fd, &allfds)) {
            close(gevent_fd);
            continue;
		}

        // Start reading
        char buffer[BUF_SIZE];
        ssize_t nread = read(gevent_fd, buffer, sizeof(buffer));
        
        if (nread == -1) {
            printf("Failed to read\n");
            return -1;
        }
        close(gevent_fd);

        // ========== FORKING =========== //
        pid_t pid = fork();

        if (pid < 0) {
            printf("Failed fork\n");
            return -1;
        } else if (pid != 0) {
            continue;
        }

        // ========== CHILD RUN DAEMON ========== //
        int dae = run_daemon(buffer);

        if (dae == Disconnect) {
            break;
        } else if (dae == -1) {
            perror("run_daemon crashed");
            break;
        }

	}

    pid_t ppid = getppid();
    kill(ppid, SIGUSR1);

	return 0;
}