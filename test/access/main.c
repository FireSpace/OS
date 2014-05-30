#include <unistd.h>
#include <stdio.h>

int main(int arhc, char ** argv)
{
    printf("%i\n", access(argv[1], R_OK));
}
