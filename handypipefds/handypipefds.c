#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "syscall_wrappers.h"

#define BUFFER_SIZE 1024

void write_buff(int fd, char *buff)
{
    write_all_data(fd, buff, strlen(buff));
}

typedef struct async_pipe_t
{
    int fdin, fdout;
    struct pollfd *fd_in_pos, *fd_out_pos;
    int iseof;
    char *buff;
    size_t data_len;
} async_pipe;

void async_pipe_c(async_pipe *ap, int fdin, int fdout, struct pollfd *fd_in_pos, struct pollfd* fd_out_pos)
{
    ap->fdin = fdin;
    ap->fdout = fdout;
    ap->fd_in_pos = fd_in_pos;
    ap->fd_out_pos = fd_out_pos;
    ap->iseof = 0;
    ap->buff = wrap_malloc(BUFFER_SIZE);
    ap->data_len = 0;
}

void async_pipe_d(async_pipe* ap)
{
    free(ap->buff);
}

int check_pipe_for_read(async_pipe *ap)
{
    if (ap->iseof){ printf("%i\n", ap->fdin); return 2; }

    if ((ap->fd_in_pos->revents & POLLIN) && ap->data_len < BUFFER_SIZE)
    {
        int res = wrap_read(ap->fdin, ap->buff + ap->data_len, BUFFER_SIZE - ap->data_len);
        //printf("%i : %i\n", ap->fdin, res);
        if (res == 0) { 
            printf("%i %i %i %i %i\n", ap->fdin, ap->fdout, ap->fd_in_pos->fd, ap->fd_out_pos->fd, ap->iseof);
            ap->iseof = 1;
        }
        else ap->data_len += res;
    }

    return ap->iseof;
}

int check_pipe_for_write(async_pipe *ap)
{
    if (ap->data_len == 0) return 0;
    else
    {
        int res = wrap_write(ap->fdout, ap->buff, ap->data_len);
        if (res == (int)ap->data_len) { ap->data_len = 0; return 0; }
        else 
        {
            memmove(ap->buff, ap->buff + res, ap->data_len - res);
            ap->data_len -= res;
            return 1;
        }
    }
}

void del_broken_pipe(async_pipe *ap, struct pollfd *fds, size_t pos, size_t fds_num)
{
    fds[pos] = fds[fds_num-2];
    fds[pos+1] = fds[fds_num-1];
    ap->fdin = fds[pos].fd;
    ap->fdout = fds[pos+1].fd;
    ap->fd_in_pos = fds+pos;
    ap->fd_out_pos = fds+pos+1;
    //printf("fds: %i\n", fds[pos].fd);
}

int main (int argc, char **argv)
{
    if (argc < 3) {write_buff(STDERR_FILENO, "too few arguments\n"); return 1;}
    if (argc%2 == 0) {write_buff(STDERR_FILENO, "count of fds is odd\n");}

    size_t pipes_num = (size_t)(argc-1) / 2;
    size_t fds_num = pipes_num*2;
    struct pollfd fds[fds_num];
    async_pipe ap[pipes_num];

    for (size_t i = 1, j = 0; i < (size_t)argc; i += 2, ++j)
    {
        fds[i-1].fd = atoi(argv[i]);
        fds[i-1].events = POLLIN;
        fds[i].fd = atoi(argv[i+1]);
        fds[i].events = POLLOUT;
        async_pipe_c(ap+j, atoi(argv[i]), atoi(argv[i+1]), fds + i-1, fds + i);
    }

    int is_not_eof_all = 1;
    int is_not_all_buffers_empty = 0;

    while (is_not_eof_all || is_not_all_buffers_empty)  
    {
        is_not_eof_all = 0;
        is_not_all_buffers_empty = 0;

        int res = 0;
        if (fds_num != 0) res = poll(fds, fds_num, -1);
        if (res == -1) { perror("poll"); return 1; }

        //check for read
        for (size_t i = 0; i < pipes_num; ++i)
        {
            int ret = check_pipe_for_read(ap+i);
            if (ret == 0) is_not_eof_all = 1;
            else if (ret == 1)
            {
                del_broken_pipe(ap + pipes_num -1, fds, i*2, fds_num);
                fds_num -= 2;
                //printf("fds_num: %i\n", (int)fds_num);
            }
        }

        //check for write
        for (size_t i = 0; i < pipes_num; ++i)
        {
            if (check_pipe_for_write(ap+i)) is_not_all_buffers_empty = 1;
        }

        for (size_t i = 0; i < pipes_num; ++i) 
        printf("%i %i %i %i %i\n", ap[i].fdin, ap[i].fdout, ap[i].fd_in_pos->fd, ap[i].fd_out_pos->fd, ap[i].iseof);
        printf("\n");

    }

    for (size_t i = 0; i < pipes_num; ++i) async_pipe_d(ap + i);
}
