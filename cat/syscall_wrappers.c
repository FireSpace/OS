#include "syscall_wrappers.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

const int ERROR = -1;

void error_and_exit(char* errmsg)
{
    perror(errmsg);
    exit(EXIT_FAILURE);
}

void* wrap_malloc(size_t size) 
{
    void* result = malloc(size);
    if (result == NULL) error_and_exit("malloc : allocate memory error");

    return result;
}

void* wrap_realloc(void* ptr, size_t size)
{
    void* result = realloc(ptr, size);
    if (result == NULL) error_and_exit("realloc : reallocate memory error");

    return result;

int wrap_open(const char* pathname, int flags)
{
    int result = open(pathname, flags);
    if (result == ERROR) error_and_exit("open");
    return result;
}

int wrap_close(int fd)
{
    int result = close(fd);
    if (result == ERROR) error_and_exit("close");
    return result;
}

ssize_t wrap_write(int fildes, const void* buf, size_t nbyte)
{
    ssize_t result = write(fildes, buf, nbyte);
    if (result == ERROR) error_and_exit("write");
    return result;
}

ssize_t wrap_read(int fildes, void* buf, size_t nbyte)
{
    ssize_t result = read(fildes, buf, nbyte);
    if (result == ERROR) error_and_exit("read");
    return result;
}

pid_t wrap_fork(void) 
{
    pid_t result = fork();
    if (result == ERROR) error_and_exit("fork");
    return result;
}

pid_t wrap_waitpid(pid_t pid, int* stat_loc, int options)
{
    pid_t result = waitpid(pid, stat_loc, options);
    if (result == ERROR) error_and_exit("waitpid");
    return result;
}

int wrap_execvp(const char* file, char* const argv[])
{
    int result = execvp(file, argv);
    if (result == ERROR) error_and_exit("execvp");
    return result;
}

void write_all_data(int fildes, const void* buf, ssize_t nbyte)
{
    ssize_t nowwrite = 0;
    while (nowwrite < nbyte)
        nowwrite += wrap_write(fildes, buf + nowwrite, nbyte - nowwrite);
}
