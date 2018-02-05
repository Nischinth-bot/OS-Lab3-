
#include "wrappers.h"

/* Waitpid
 * Wrapper for waitpid
 * All of these system calls are used to wait for state changes
 * in a child of the calling process, and obtain information 
 * about the child whose state has changed. A state change is
 * considered to be: the child terminated; the child was stopped
 * by a signal; or the child was resumed by a signal. In the case
 * of a terminated child, performing a wait allows the system to
 * release the resources associated with the child; if a wait is
 * not performed, then the terminated child remains in a "zombie" state. 
 *
 * on success, returns the process ID of the child whose state 
 * has changed; if WNOHANG was specified and one or more 
 * child(ren) specified by pid exist, but have not yet changed
 * state, then 0 is returned. If not children exist, -1 is returned. 
 */
pid_t Waitpid(pid_t pid, int *status, int options)
{
   //check return value in shell
   return waitpid(pid, status, options);
}

/* Setpgid
 * Wrapper for setpgid
 * setpgid() sets the PGID of the process specified by pid to pgid.  If
 * pid is zero, then the process ID of the calling process is used.  If
 * pgid is zero, then the PGID of the process specified by pid is made
 * the same as its process ID.
 */
int Setpgid(pid_t pid, pid_t pgid)
{
   if (setpgid(pid, pgid) == -1) unixError("setpgid error");
   return 0;
}

/* Close
 * Wrapper for close.
 * The close() function shall deallocate the file descriptor indicated 
 * by fildes. To deallocate means to make the file descriptor available
 * for return by subsequent calls to open() or other functions that 
 * allocate file descriptors.
 *
 */
int Close(int fildes)
{
   if (close(fildes) == -1) unixError("close error");
   return 0;
}
/* Dup
 * Wrapper for dup.
 * The dup() system call creates a copy of the file descriptor oldfd,
 * using the lowest-numbered unused file descriptor for the new
 * descriptor.
 */
int Dup(int oldfd)
{
   int fd;
   fd = dup(oldfd);
   if (fd == -1) unixError("dup error");
   return fd;
}

/* Dup2
 * Wrapper for dup2.
 * The dup2() system call performs the same task as dup(), but instead
 * of using the lowest-numbered unused file descriptor, it uses the file
 * descriptor number specified in newfd.  If the file descriptor newfd
 * was previously open, it is silently closed before being reused.
 */
int Dup2(int oldfd, int newfd)
{
   int fd;
   fd = dup2(oldfd, newfd);
   if (fd == -1) unixError("dup2 error");
   return fd;
}

/* Pipe
 * Wrapper for pipe.
 * pipe() creates a pipe, a unidirectional data channel that can be used
 * for interprocess communication. The array pipefd is used to return 
 * two file descriptors referring to the ends of the pipe. pipefd[0] 
 * refers to the read end of the pipe. pipefd[1] refers to the write 
 * end of the pipe. Data written to the write end of the pipe is buffered
 * by the kernel until it is read from the read end of the pipe. 
 */
int Pipe(int pipefd[2])
{
   if (pipe(pipefd) == -1) unixError("pipe error");
   return 0;
}


/* Malloc
 * Wrapper for malloc.
 * The malloc() function shall allocate unused 
 * space for an object whose size in bytes is specified by size 
 * and whose value is unspecified.
 */
void *Malloc(size_t size)
{
   void * ptr = malloc(size);
   if (ptr == NULL) unixError("malloc error");
   return ptr;
}

/* Sigprocmask
 * Wrapper for sigprocmask.
 * sigprocmask() is used to fetch and/or change the signal mask of the
 * calling thread.  The signal mask is the set of signals whose delivery
 * is currently blocked for the caller 
 * The behavior of the call is dependent on the value of how, as                           follows.
 * SIG_BLOCK: The set of blocked signals is the union of the current set and
 *         the set argument.
 * SIG_UNBLOCK: The signals in set are removed from the current set of blocked
 *           signals.  It is permissible to attempt to unblock a signal
 *           which is not blocked.
 * SIG_SETMASK: The set of blocked signals is set to the argument set.
 *
 * If oldset is non-NULL, the previous value of the signal mask is
 * stored in oldset.
 */
int Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
   if (sigprocmask(how, set, oldset) == -1) unixError("sigprocmask error");
   return 0;
}

/* Sigemptyset
 * Wrapper for sigemptyset.
 * The sigemptyset() function initializes the signal 
 * set pointed to by set, such that all signals are excluded.
*/
int Sigemptyset(sigset_t *set)
{
   if (sigemptyset(set) == -1) unixError("sigemptyset error");
   return 0;
}

/* Sigaddset
 * Wrapper for sigaddet.
 * The sigaddset() function adds the individual signal specified
 * by the signo to the signal set pointed to by set.
*/
int Sigaddset(sigset_t *set, int signo)
{
   if (sigaddset(set, signo) == -1) unixError("sigaddset error");
   return 0;
}

/* unixError
 * Called when an error occurs to print an error message and exit.
 */
void unixError(char * msg)
{
   fprintf(stderr, "%s: %s\n", msg, strerror(errno));
   exit(1);
}

/* 
 * Fork                          
 * Wrapper function for fork.         
 * Creates a process and returns the pid. 
 * The child process is returned the value of 0.
 * The parent process is returned the pid of the child.
 * If a process cannot be created, the function
 * calls the unixError function to print a message
 * and exit.
 */
int Fork()
{
   int pid = fork();
   if (pid == -1) unixError("fork failed.");
   return pid;
}

/* 
 * Execvp
 * Wrapper function for execvp function.
 * The execve takes as input the name of a command
 * (program) and the arguments to the command 
 * (including the name of the command) and
 * executes the command in the context of the calling
 * process.  The last element in the array of arguments
 * (immediately after the actual arguments) must be NULL.
 * If the execvp fails (for example, the command
 * can not be found), execvp returns negative one.
 * For example, given this array:
 * char * args[3] = {"ls", "-l", NULL};
 * execvp could be called like this:
 * execvp(args[0], args);
*/
int Execvp(char * file, char * argv[])
{
   //check return value in eval function
   return execvp(file, argv);
}

/*
 *  Signal 
 *  Wrapper for the sigaction function.
 *  This function installs a signal handler.
*/
void *Signal(int signum, void (*handler)(int))
{
   struct sigaction action, old_action;

   action.sa_handler = handler;
   sigemptyset(&action.sa_mask); /* block sigs of type being handled */
   action.sa_flags = SA_RESTART; /* restart syscalls if possible */

   if (sigaction(signum, &action, &old_action) < 0)
      unixError("Signal error");
   return (old_action.sa_handler);
}

