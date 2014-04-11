#include <stdlib.h>
#include "syscall_wrappers.h"
#include <stdio.h>
#include <string.h>

const size_t BUF_SIZE = 1024;
char* delimiter;

typedef struct buffer_t
{
    char* data;
    char delim;
    size_t size;
    size_t length;
} buffer;

void buffer_malloc(buffer* buff)
{
    buff->delim = '\n';
    buff->data = wrap_malloc(BUF_SIZE);
    buff->size = BUF_SIZE;
    buff->length = 0;
}

void buffer_free(buffer* buff)
{
    free(buff->data);
    buff->size = 0;
    buff->length = 0;
}

void buffer_realloc(buffer* buff)
{
    buff->size += BUF_SIZE;
    buff->data = wrap_realloc(buff->data, buff->size);
}

void reverse_and_print(char* buff, size_t len)
{
    char* tbuffer = wrap_malloc(BUF_SIZE);
    for (int i = len-1, j = 0; i >= 0; --i, ++j)
    {
        tbuffer[j] = buff[i];
    }
    tbuffer[len] = '\n';
    write_all_data(STDOUT_FILENO, tbuffer, (ssize_t)len+1);
}

void buffer_print_data(buffer* buff)
{
    char* tbuffer = wrap_malloc(BUF_SIZE);
    size_t len = 0;
    for (size_t i = 0; i < buff->length; ++i)
    {
        if (buff->data[i] == buff->delim) 
        {
            reverse_and_print(tbuffer, len);
            len = 0;
        } else 
        {
            tbuffer[len] = buff->data[i];
            len++;
        }
    }

    free(tbuffer);
}

void buffer_add(buffer* buff, char* newdata, size_t len)
{
    if (len + buff->length >= buff->size) buffer_realloc(buff);
    memmove(buff->data + buff->length, newdata, len);
    buff->length += len;
}

void read_and_go_out(int fildes)
{
    buffer buff;
    char* tbuffer = wrap_malloc(BUF_SIZE);
    size_t len = 0;
    buffer_malloc(&buff);

    int eof = 0;
    while (!eof)
    {
        ssize_t retval = wrap_read(fildes, tbuffer + len, BUF_SIZE - len);
        if (retval == 0) eof = 1;
        len += retval;
        if (len == BUF_SIZE || eof)
        {
            buffer_add(&buff, tbuffer, len);
            len = 0;
        }
    }

    buffer_print_data(&buff);
    buffer_free(&buff);
    free(tbuffer);
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
