#include <stdlib.h>
#include <vector>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#define BUFFER_SIZE 3096
#define MAX_ARGS 30
int file_in;
int file_out;

std::ofstream debug("debug");

typedef struct parser_t 
{
    char *buff;
    int buflen;
    char *command;
    int comlen;
    int mode;
    std::vector<int> pos;
} parser;

parser* parser_c()
{
    parser *ret = (parser*)malloc(sizeof(parser));
    ret->buff = (char*)malloc(BUFFER_SIZE);
    ret->buflen = 0;
    ret->command = (char*)malloc(BUFFER_SIZE);
    ret->comlen = 0;
    ret->mode = 0;
    return ret;
}

void parser_d(parser *p)
{
    free(p->buff);
    free(p->command);
    free(p);
}

void parser_reset(parser *p)
{
    p->comlen = 0;
    p->mode = 0;
    p->pos.clear();
}

int read_parse(parser *p)
{
    if (p->comlen != 0) return p->comlen;
    if (p->buflen == BUFFER_SIZE) return -1;
    
    int rret = read(STDIN_FILENO, p->buff + p->buflen, BUFFER_SIZE - p->buflen);
    if (rret == 0) return 0;
    else p->buflen += rret;

    //buffer processing
    if (p->mode == 0) 
    {
        switch(p->buff[0])
        {
            case 'r':
                p->mode = 1;
                memmove(p->buff, p->buff+4, p->buflen-4);
                p->buflen -= 4;
                break;
            case 'b':
                p->mode = 2;
                memmove(p->buff, p->buff+5, p->buflen-5);
                p->buflen -= 5;
                break;
            case 'k':
                p->mode = 3;
                memmove(p->buff, p->buff+5, p->buflen-5);
                p->buflen -= 5;
                break;
            case 'w':
                p->mode = 4;
                memmove(p->buff, p->buff+5, p->buflen-5);
                p->buflen -= 5;
                break;
        }
    }
    for (int i = 0; i < p->buflen; ++i)
    {
        if (p->buff[i] == '|')
        {
            p->buff[i] = '\0';
            p->pos.push_back(i);
        }
        if (p->buff[i] == '\n') 
        {
            memmove(p->command, p->buff, i);
            p->buflen -= (i+1);
            p->comlen = i+1;
            p->command[i] = '\0';
        }
    }
    //end buffer processing
    return rret;
}

char ** get_args(parser *p, int begin, int end)
{
    if (begin != 0) begin = p->pos[begin-1]+1;
    end = (end > p->pos.size()) ? p->comlen : p->pos[end-1];

    char *temp = (char*)malloc(BUFFER_SIZE);
    strcpy(temp, p->command + begin);

    int numb = 0;
    for (int i = 0; i < end-begin; ++i)
    {
        if (temp[i] == ' ')
        {
            numb++;
            temp[i] = '\0';
        }
    }

    char **ret = (char**)malloc(sizeof(char*)*(numb+1));
    numb = 1;
    ret[0] = temp;
    for (int i = 0; i < end-begin-1; ++i)
    {
        if (temp[i] == '\0') ret[numb] = temp+i+1;
        numb++;
    }

    return ret;
}

void make_pipe(parser *p)
{
    setpgid(0, 0);
    int curout = file_in;
    for (size_t i = 0; i < p->pos.size(); ++i)
    {
        int pipefd[2]; if (pipe(pipefd) == -1) { perror("pipe"); return; }
        int id = fork();
        if (id == -1) { perror("second fork"); return; }
        if (id == 0)
        {
            dup2(pipefd[0], STDOUT_FILENO);
            dup2(curout, STDIN_FILENO);

            char ** args = get_args(p, i, i+1);
            execvp(args[0], args);
        } else
        {
            sleep(2);
            curout = pipefd[1];
        }
    }

    //for last element in pipe
    int id = fork();
    if (id == -1) { perror("second fork"); return; }
    if (id == 0)
    {
        dup2(file_out, STDOUT_FILENO);
        dup2(curout, STDIN_FILENO);

        char ** args = get_args(p, p->pos.size(), p->pos.size()+1);
        debug << args[0] << " " << getpid() << std::endl;
        execvp(args[0], args);
    }

    //wait all children
    int status;
    for (size_t i = 0; i < p->pos.size()+1; ++i)
        if (wait(&status) == -1) { perror("wait"); }
}

int main()
{
    file_in = open("input", O_CREAT | O_RDONLY, 0666);
    if (file_in == -1) { perror("open input file"); return 1; }
    file_out = open("output", O_CREAT | O_WRONLY, 0666);
    if (file_out == -1) { perror("open output file"); return 1; }

    std::vector<pid_t> pids;

    parser *p = parser_c();

    for(;;)
    {
        int ret = read_parse(p);
        if (ret == 0) return 0;
        if (ret == -1) { printf("buffer is full"); return 1; }
        
        switch(p->mode)
        {
            case 1:
            case 2:
            {
                pid_t pid = fork();
                if (pid == -1) { perror("first fork"); return 1; }
                if (pid == 0)
                {
                    make_pipe(p);
                } else
                {
                    setpgid(pid, pid);
                    pids.push_back(pid);
                    if (p->mode == 1)
                    {
                        int status;
                        if (waitpid(pid, &status, 1) == -1) { perror("waitpid"); }
                    }
                }
                break;
            }
            case 3: 
            {
                if (kill(-atoi(p->command), SIGTERM) == -1) { perror("kill"); }
                break;
            }
            case 4:
            {
                int status;
                if (waitpid(atoi(p->command), &status, 1) == -1) { perror("wait command"); }
                break;
            }
        }

        parser_reset(p);
    }

    parser_d(p);
}
