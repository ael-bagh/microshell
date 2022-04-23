#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int ft_strlen(char *str)
{
    int i = 0;
    while(str[i])
        i++;
    return i;
}
void print_error(char *s1, char *s2, int exit_status)
{
    write(2,s1,ft_strlen(s1));
    if (s2)
        write(2,s2,ft_strlen(s2));
    write(2,"\n",1);
    if (exit_status)
        exit(1);
}

void exec_pipes(char **p, int in, int out, int to_close, char **env)
{
    if (to_close != -1)
        close(to_close);
    if ((dup2(in,0)) == -1)
        print_error("error: fatal", NULL, 1);
    if ((dup2(out,1)) == -1)
        print_error("error: fatal", NULL, 1);
    if (in != 0)
        close(in);
    if (out != 1)
        close(out);
    if (strcmp(p[0], "cd") == 0)
    {
        if (p[1] && !p[2])
        {
            if ((chdir(p[1])) == -1)
                print_error("error: cd: cannot change directory to ",p[1], 0);
        }
        else
            print_error("error: cd: bad arguments", NULL, 1);
    }
    else
    {
        execve(p[0],p,env);
        print_error("error: cannot execute ",p[0],1);
    }
}

void exec_cmds(char **cmds, char **env)
{
    int i = 0;
    int k = 0;
    int fd[2];
    int in = 0;
    int out = 1;
    int to_close;
    pid_t pid;
    pid_t pids[10000];

    while (cmds[i])
    {
        int j = 0;
        char *p[10000];
        while(cmds[i] && strcmp(cmds[i], "|"))
            p[j++] = cmds[i++];
        p[j] = NULL;
        to_close = -1;
        if (cmds[i])
        {
            i++;
            if ((pipe(fd)) == -1)
                print_error("error: fatal", NULL,1);
            out = fd[1];
            to_close = fd[0];
        }
        else
            out = 1;
        pid = fork();
        if (pid < 0)
            print_error("error: fatal", NULL, 1);
        pids[k++] = pid;
        if (pid == 0)
            exec_pipes(p,in,out,to_close,env);
        else
        {
            if (in != 0)
                close(in);
            if (out != 1)
                close(out);
            in = fd[0];
        }
    }
    for (int i = 0; i < k; i++)
        waitpid(pids[i], NULL, 0);
}

int main(int argc, char **argv, char **env)
{
    int i = 1;
    (void)argc;
    while (argv[i])
    {
        int j = 0;
        char *cmds[10000];
        while (argv[i] && strcmp(argv[i], ";") != 0)
            cmds[j++] = argv[i++];
        cmds[j] = NULL;
        if (argv[i])
            i++;
        exec_cmds(cmds, env);
    }
}