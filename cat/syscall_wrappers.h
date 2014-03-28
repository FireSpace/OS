#pragma once

#include <unistd.h>
#include <sys/types.h>

void error_and_exit(char* errmsg);

int wrap_open(const char* pathname, int flags);
int wrap_close(int fd);
ssize_t wrap_write(int fildes, const void* buf, size_t nbyte);
ssize_t wrap_read(int fildes, void* buf, size_t nbyte);
void* wrap_malloc(size_t size);
pid_t wrap_fork(void);
pid_t wrap_waitpid(pid_t pid, int* stat_loc, int options);
int wrap_execvp(const char* file, char* const argv[]);
void write_all_data(int fildes, const void* buf, ssize_t nbyte);
size_t read_all_data(int fildes, void* buf, size_t nbyte);
