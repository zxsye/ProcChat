#include "server.h"

// #include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {

    int fd[2] = { -1, -1};
    pipe(fd);

    close(fd[0]);
    close(fd[1]);

}
