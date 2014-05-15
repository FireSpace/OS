#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "syscall_wrappers.h"

#define BUFFER_SIZE 1024

ssize_t write_buff(int fildes, char* buff)
{
    return wrap_write(fildes, buff, strlen(buff));
}

typedef struct async_pipe_t
{
    int fildes1, fildes2;
    struct pollfd fds[2];
    int status; // 1 - time to read, 2 - time to write
    char* buffer;
    size_t data_len;
} async_pipe;

void async_pipe_c(async_pipe* ap, int fildes1, int fildes2)
{
    ap->status = 1;
    ap->buffer = wrap_malloc(BUFFER_SIZE);
    ap->data_len = 0;

    ap->fildes1 = fildes1;
    ap->fildes2 = fildes2;
    ap->fds[0].fd = fildes1;
    ap->fds[0].events = POLLIN;
    ap->fds[1].fd = fildes2;
    ap->fds[1].events = POLLOUT;
}

int poll_check(async_pipe* ap)
{
    int ret;
    if (ap->status == 1)
    {
        ret = poll(ap->fds, 1, -1);
        if (ret == -1) { perror("poll "); return 1; }

        if (ret)
        {
            ap->data_len = wrap_read(ap->fildes1, ap->buffer, BUFFER_SIZE);
            ap->status = 2;
        }
    }

    if (ap->status == 2)
    {
        ret = poll(ap->fds+1, 1, -1);
        if (ret == -1) { perror("poll "); return 1; }

        if (ret)
        {
            int tlen = wrap_write(ap->fildes2, ap->buffer, ap->data_len);
            if (tlen == (int)ap->data_len)
            {
                ap->data_len = 0;
                ap->status = 1;
            } else
            {
                memcpy(ap->buffer, ap->buffer+tlen, ap->data_len-tlen);
                ap->data_len -= tlen;
            }
        }
    }

    return 0;
}

void async_pipe_d(async_pipe* ap)
{
    free(ap->buffer);
}

int main(int argc, char** argv)
{
    if (argc < 3) { write_buff(STDERR_FILENO, "too few arguments\n"); return 1; }
    if (argc%2 == 0) { write_buff(STDERR_FILENO, "number of fds is odd\n"); }
    size_t numb_aps = (argc-1) / 2;

    async_pipe* aps = malloc(sizeof(async_pipe)*numb_aps);

    for (size_t i = 1, j = 0; i <= numb_aps; i+=2, ++j)
    {
        async_pipe_c(&(aps[j]), atoi(argv[i]), atoi(argv[i+1]));
    }

    while (1)
    {
        for (size_t i = 0; i < numb_aps; ++i)
        {
            if (poll_check(&(aps[i])))
            {
                for (size_t j = 0; j < numb_aps; ++j) async_pipe_d(&(aps[j]));
                //write_buff(STDERR_FILENO, "poll error\n");
                return 1;
            }
        }
    }
}
