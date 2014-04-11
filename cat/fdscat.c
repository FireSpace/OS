#include <stdlib.h>
#include "syscall_wrappers.h"
#include <stdio.h>
#include <string.h>

const size_t BUF_SIZE = 1024;
char* delimiter;

struct buffer_t
{
    char* data;
    char delim;
    size_t size;
    size_t length;
};

void buffer_malloc(struct buffer_t* buff)
{
    buff->delim = '\n';
    buff->data = wrap_malloc(BUF_SIZE);
    buff->size = BUF_SIZE;
    buff->length = 0;
}

void buffer_free(struct buffer_t* buff)
{
    free(buff->data);
    buff->size = 0;
    buff->length = 0;
}

void buffer_realloc(struct buffer_t* buff)
{
    buff->size += BUF_SIZE;
    buff->data = wrap_realloc(buff->data, buff->size);
}

void reverse_and_print(char* buff, size_t len)
{
    char* buffer = wrap_malloc(BUF_SIZE);
    for (int i = len-1, j = 0; i >= 0; --i, ++j)
    {
        buffer[j] = buff[i];
    }
    buffer[len] = '\n';
    write_all_data(STDOUT_FILENO, buffer, (ssize_t)len+1);
}

void buffer_print_data(struct buffer_t* buff)
{
    char* buffer = wrap_malloc(BUF_SIZE);
    size_t len = 0;
    for (size_t i = 0; i < buff->length; ++i)
    {
        if (buff->data[i] == buff->delim) 
        {
            reverse_and_print(buffer, len);
            len = 0;
        } else 
        {
            buffer[len] = buff->data[i];
            len++;
        }
    }

    free(buffer);
}

void buffer_add(struct buffer_t* buff, char* newdata, size_t len)
{
    if (len + buff->length >= buff->size) buffer_realloc(buff);
    memmove(buff->data + buff->length, newdata, len);
    buff->length += len;
}

void read_and_go_out(int fildes)
{
    struct buffer_t buffer;
    char* tbuffer = wrap_malloc(BUF_SIZE);
    size_t len = 0;
    buffer_malloc(&buffer);

    int eof = 0;
    while (!eof)
    {
        ssize_t retval = wrap_read(fildes, tbuffer + len, BUF_SIZE - len);
        if (retval == 0) eof = 1;
        len += retval;
        if (len == BUF_SIZE || eof)
        {
            buffer_add(&buffer, tbuffer, len);
            len = 0;
        }
    }

    buffer_print_data(&buffer);
    buffer_free(&buffer);
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
