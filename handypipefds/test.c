#include <vector>
#include <iostream>

#include <fcntl.h>
#include <stdio.h>

#include "syscall_wrappers.h"

#define ARG_SIZE 10

int main(int argc, char ** argv) {
    std::vector<int> fds;
    for (int i = 1; i < argc; i++) {
        int modes, errfd;
        if (i % 2 == 1) {
            modes = O_RDONLY;
            errfd = 0;
        } else {
            modes = O_WRONLY;
            errfd = 1;
        }
        int fd = open(argv[i], O_CREAT | modes, S_IRWXU);
        if (fd == -1) {
            perror(argv[i]);
            fds.push_back(errfd);
        } else {
            fds.push_back(fd);
        }
    }
    char ** fdstrings = (char **) wrap_malloc((fds.size() + 2) * sizeof(char *));
    fdstrings[0] = "hui";
    for (size_t i = 1; i < fds.size() + 1; i++) {
        fdstrings[i] = (char *) wrap_malloc(ARG_SIZE);
        sprintf(fdstrings[i], "%d", fds[i - 1]);
    }
    fdstrings[fds.size() + 2] = 0;
    if (execvp("./handypipefds", fdstrings) == -1) {
        perror("пизда");
    }
}
