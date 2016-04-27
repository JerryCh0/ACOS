#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int **fd;
int pipenum;

void createIOSon(const char *progname, int index){
    int chpid = 0;
    chpid = fork();
    if (chpid < 0){
        write(2, "fork error\n", 11 * sizeof(char));
        return;
    }
    if (chpid != 0){
        return;
    }
    if (index != 0){
        //printf("%s change discR\n", progname);
        dup2(fd[index - 1][0], 0);
    }
    if (index != pipenum){
        //printf("%s change discW\n", progname);
        dup2(fd[index][1], 1);
    }
    //write(2, progname, sizeof(char) * strlen(progname));
    //write(2, "\n", 1);
    int i = 0;
    for (i = 0; i < pipenum; i++){
        close(fd[i][0]);
        close(fd[i][1]);
    }
    execlp(progname, progname, NULL);
    exit(1);
}

int main(int argc, char **argv){
    //1 - stdout
    //0 - stdin
    //pipe[0] - for read from pipe
    //pipe[1] - for write to pipe
    pipenum = argc - 2;
    fd = (int**)malloc(sizeof(int*) * pipenum);
    int i = 0;
    for (i = 0; i < pipenum; i++){
        fd[i] = (int*)malloc(sizeof(int) * 2);
        pipe(fd[i]);
    }
    for (i = 0; i < pipenum + 1; i++){
        createIOSon(argv[i + 1], i);
    }
    for (i = 0; i < pipenum; i++){
        close(fd[i][0]);
        close(fd[i][1]);
    }
    int n;
    for (i = 1; i < argc; i++){
        wait(&n);
    }
    for (i = 1; i < pipenum; i++){
        free(fd[i]);
    }
    free(fd);
    return 0;
}
