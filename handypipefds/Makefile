gcc: syscall_wrappers.c handypipefds.c
	gcc -std=c99 -Wall -Wextra syscall_wrappers.c handypipefds.c -o handypipefds

debug: syscall_wrappers.c handypipefds.c
	gcc -std=c99 -Wall -Wextra -g syscall_wrappers.c handypipefds.c -o handypipefds

test: test.c syscall_wrappers.c
	g++ -Wall -Wextra test.c -o test -g syscall_wrappers.c

clang: syscall_wrappers.c handypipefds.c
	clang syscall_wrappers.c handypipefds.c -o handypipefds
