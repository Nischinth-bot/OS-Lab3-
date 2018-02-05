#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
   int fd[2];
   //parent creates the pipe
   pipe(fd);  
   if (fork() == 0)
   {
      char * args[]={"sort", "-r", NULL};
      //don't need write end
      //sort will read from the read end
      close(fd[1]);
      //make 0 an alias of fd[0] 
      dup2(fd[0], 0);
      //don't need fd[0] because 0
      //now is a duplicate of it
      close(fd[0]);
      execvp("sort", args);

   } 
   if (fork() == 0)
   {
      char * args[]={"ls", NULL};
      //don't need read end
      //ls will write to the write end
      close(fd[0]);
      //make 1 an alias of fd[1] 
      dup2(fd[1], 1);
      //now fd[1] isn't needed because 1
      //is now a duplicate up it
      close(fd[1]);
      execvp("ls", args);
   }
   //parent has its own file descriptors
   //need to close those up
   close(fd[0]);
   close(fd[1]);
}

