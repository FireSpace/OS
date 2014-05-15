#include <unistd.h>
#include "syscall_wrappers.h"

int main()
{
    while(1)
    {
        char* buf = wrap_malloc(1);
        if (wrap_read(STDIN_FILENO, buf, 1))
            while (!wrap_write(STDOUT_FILENO, buf, 1)) {  }

    }

}
