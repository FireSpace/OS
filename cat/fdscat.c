#include <stdlib.h>
#include "syscall_wrappers.h"
#include <stdio.h>
#include <string.h>

const size_t BUF_SIZE = 1024;
char* delimiter;

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
    delimiter = argv[1];
    for (int i = 2; i < argc; ++i) 
    {
        read_and_go_out(atoi(argv[i]));
        if (i != argc-1) write_all_data(STDOUT_FILENO, delimiter, strlen(delimiter)); 
    }
    return EXIT_SUCCESS;
}
