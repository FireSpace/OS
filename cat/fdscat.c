#include <stdlib.h>
#include "syscall_wrappers.h"
#include <stdio.h>

const size_t BUF_SIZE = 1024;

void read_and_go_out(int fildes)
{
    char* buffer = wrap_malloc(BUF_SIZE);
    size_t len = 0;
    int eof = 0;
    while (!eof)
    {
        ssize_t retval = wrap_read(fildes, buffer + len, BUF_SIZE - len);
        if (retval == 0) eof = 1;
        len += retval;
        if (len == BUF_SIZE || eof)
        {
            write_all_data(STDOUT_FILENO, buffer, len);
            len = 0;
        }
    }
    free(buffer);
}
int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
        read_and_go_out(atoi(argv[i]));
    return EXIT_SUCCESS;
}
