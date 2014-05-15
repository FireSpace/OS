#include <unistd.h>
#include "syscall_wrappers.h"

int main()
{
    char buf[1];
    while(wrap_read(STDIN_FILENO, buf, 1))
    {
        write_all_data(STDOUT_FILENO, buf, 1);
    }
}
