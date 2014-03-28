#include "syscall_wrappers.h"
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

const int FILDES_LEN = 11;

int main(int argc, char **argv) 
{
    if (argc < 2) error_and_exit("not enough arguments");
    char* argv_exec[argc];
    argv_exec[0] = argv[1];

    for (int i = 1; i < argc-1; ++i)
    {
        argv_exec[i] = wrap_malloc(FILDES_LEN);
        memset(argv_exec[i], 0, FILDES_LEN);
        int fildes = wrap_open(argv[i+1], O_RDONLY);
        sprintf(argv_exec[i], "%d", fildes);
    }

    argv_exec[argc-1] = NULL;
    pid_t pid = wrap_fork();
    if (pid != 0)
    {
        int stat;
        wrap_waitpid(pid, &stat, 0);
        if (!(WIFEXITED(stat) && WEXITSTATUS(stat) == 0))
            error_and_exit("exit status of executed program");
    } else
    {
        wrap_execvp(argv_exec[0], argv_exec);
        _exit(EXIT_SUCCESS);
    }

    for (int i = 1; i < argc-1; ++i) free(argv_exec[i]);
    return EXIT_SUCCESS;
}
