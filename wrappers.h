#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

void unixError(char * msg);
int Fork();
int Execvp(char * path, char * argv[]);
void *Signal(int signum, void (*handler)(int));
int Sigemptyset(sigset_t *set);
int Sigaddset(sigset_t *set, int signo);
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void *Malloc(size_t size);
int Dup(int oldfd);
int Dup2(int oldfd, int newfd);
int Close(int fildes);
int Setpgid(pid_t pid, pid_t pgid);
int Pipe(int pipefd[2]);
pid_t Waitpid(pid_t pid, int *status, int options); 
