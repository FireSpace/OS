#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "syscall_wrappers.h"

int main(int argc, char **argv)
{
    //if (argc < 3 || argc%2 == 0) { printf("bad arguments"); return 1; }

    int steps = (argc-1) / 2;
    int step = 0;
    while (1)
    {
        int data_len = (step < steps) ? atoi(argv[step*2]) : 1;
        int sleep_time = (step < steps) ? atoi(argv[step*2+1]) : 0;
        step += (step < steps) ? 1 : 0;

        char buff[data_len];
        int read_len = 0;
        while ((data_len - read_len) > 0)
        {
            int ret = wrap_read(STDIN_FILENO, buff + read_len, data_len - read_len);
            if (ret == 0) { close(STDOUT_FILENO); return 0; }
            read_len += ret;
        }

        write_all_data(STDOUT_FILENO, buff, data_len);
        sleep(sleep_time);
    }
}
