#include <ctype.h>
#include "parser.h"
#include "wrappers.h"

//not needed outside of this file
static void clearJobList(jobList joblist[MAXJOBSPERCMDLN]);
static void clearCmdList(cmdList cmdlist[MAXCMDSPERJOB]);
static char * trimWhiteSpace(char * str);

/* getCndCount
 * Returns a count of the number of commands in the cmdlist.
 */
int getCmdCount(cmdList cmdlist[MAXCMDSPERJOB])
{
   int i;
   for (i = 0; cmdlist[i].filled == 1 && i < MAXCMDSPERJOB; i++);
   return i;
}

/* parseIntoCmds
 * Takes as input a job and parses the job into commands. Jobs
 * are separated by &. Commands are separated by |.
 * The cmdlist array is filled with the parsed job.
 * For example, if job is equal to
 *
 * "cmd1 23 45 | cmd2 a"
 *
 * then the cmdlist array is initialized as follows:
 *
 * cmdlist[0].cmd = "cmd1 23 45"
 * cmdlist[0].filled = 1
 * cmdlist[0].pipe = 1
 * cmdlist[0].args[0] = "cmd1"
 * cmdlist[0].args[1] = "23"
 * cmdlist[0].args[2] = "45"
 * cmdlist[0].args[3] = "NULL"
 * cmdlist[1].cmd = "cmd2 a"
 * cmdlist[1].filled = 1
 * cmdlist[1].pipe = 0
 * cmdlist[1].args[0] = "a"
 * cmdlist[1].args[1] = NULL"
 * cmdlist[2].cmd = NULL;
 * 
 * The NULL values are initialized by the clearCmdList function.
 */
void parseIntoCmds(char * job, cmdList cmdlist[MAXCMDSPERJOB])
{
   int i, j, pipe = 0;
   char input[MAXLINE];
   char * token;

   //look for pipe at very end
   if (job[strlen(job) - 1] == '|') pipe = 1;

   clearCmdList(cmdlist);

   memset(input, '\0', MAXLINE);
   strncpy(input, job, MAXLINE);
   token = strtok(input, "|");
   i = 0;
   //commands are separated by | characters
   while (token != NULL)
   {
      token = trimWhiteSpace(token);
      if (i == MAXCMDSPERJOB) unixError("number of commands exceeded");
      cmdlist[i].cmd = strdup(token);
      cmdlist[i].filled = 1;
      cmdlist[i].pipe = 1;
      token = strtok(NULL, "|");
      i++;
   }
   if (i > 1) cmdlist[i-1].pipe = pipe;

   //command arguments are separate by whitespace
   for (i = 0; cmdlist[i].filled == 1; i++)
   {
      memset(input, '\0', MAXLINE);
      strncpy(input, cmdlist[i].cmd, MAXLINE);
      token = strtok(input, " ");
      j = 0;
      while (token != NULL)
      {
         token = trimWhiteSpace(token);
         if (j == MAXARGS) unixError("number of arguments exceeded");
         cmdlist[i].args[j] = strdup(token);
         token = strtok(NULL, " ");
         j++;
      }
   }
}
         
/* initCmdList
 * Initializes the cmdlist array to NULLs and 0s
 */
void initCmdList(cmdList cmdlist[MAXJOBSPERCMDLN])
{
   int i, j;
   for (i = 0; i < MAXCMDSPERJOB; i++)
   {
      cmdlist[i].cmd = NULL;
      for (j = 0; j < MAXARGS; j++) 
         cmdlist[i].args[j] = NULL; 
      cmdlist[i].filled = 0;
      cmdlist[i].pipe = 0;
   }
}

/* clearCmdList
 * Clears a cmdList array, freeing the space allocated for 
 * strings.
 */
void clearCmdList(cmdList cmdlist[MAXJOBSPERCMDLN])
{
   int i, j;
   for (i = 0; i < MAXCMDSPERJOB; i++)
   {
      if (cmdlist[i].filled)
      {
         free(cmdlist[i].cmd);
         cmdlist[i].cmd = NULL;
         for (j = 0; j < MAXARGS; j++) 
            if (cmdlist[i].args[j] != NULL) 
            {
               free(cmdlist[i].args[j]);
               cmdlist[i].args[j] = NULL;
            }
         cmdlist[i].filled = 0;
         cmdlist[i].pipe = 0;
      }
   }
}

/* printCmdList
 * Outputs the contents of the cmdlist array.
 */
void printCmdList(cmdList cmdlist[MAXCMDSPERJOB])
{
   int i;
   int j;
   for (i = 0; i < MAXCMDSPERJOB; i++)
   {
      if (cmdlist[i].filled)
      {
         printf("command: %s ", cmdlist[i].cmd);
         for (j = 0; cmdlist[i].args[j] != NULL; j++)
         {
            printf("arg%d: %s ", j, cmdlist[i].args[j]);
         }
         printf("pipe: %d\n", cmdlist[i].pipe);
      }
   }
}

/* trimWhiteSpace
 * Removes the white space from the beginning and the
 * end of a string.
 */
char * trimWhiteSpace(char * str)
{
   char * end;
   char * begin = str;
   while (*begin == ' ') begin++;
   end = begin;
   while (*end != '\0') end++;
   end--;
   while (end > begin && isspace(*end)) 
   {
      *end = '\0';
      end--;
   }
   return begin;
}

/* parseIntoJobs
 * Parses a command line into jobs. Jobs are separated
 * by &. For example, if the cmdline contains:
 *
 * "cmd1 ab c | cmd2 12 & cmd3 & cmd4 123"
 *
 * then the joblist array is initialized as follows:
 *
 * joblist[0].job = "cmd1 ab c | cmd2 12"
 * joblist[0].filled = 1
 * joblist[0].bg = 1
 * joblist[1].job = "cmd3"
 * joblist[1].filled = 1
 * joblist[1].bg = 1
 * joblist[2].job = "cmd4 123"
 * joblist[1].filled = 1
 * joblist[1].bg = 0
 */
void parseIntoJobs(char * cmdline, jobList joblist[MAXJOBSPERCMDLN])
{
   int i = 0, bg = 0;
   clearJobList(joblist);
   cmdline = trimWhiteSpace(cmdline);
   if (cmdline[strlen(cmdline) - 1] == '&') bg = 1;
   char input[MAXLINE];
   strncpy(input, cmdline, MAXLINE);
   char * token = strtok(input, "&");
   while (token != NULL)
   {
      token = trimWhiteSpace(token);
      if (i == MAXJOBSPERCMDLN) unixError("too many jobs in commandline");
      joblist[i].job = strdup(token);
      joblist[i].filled = 1;
      joblist[i].bg = 1;
      token = strtok(NULL, "&");
      i++;
   }
   if (i > 0) joblist[i-1].bg = bg; 
}

/* initJobList
 * Initialize a job list to NULLs and 0s
 */
void initJobList(jobList joblist[MAXJOBSPERCMDLN])
{
   int i;
   for (i = 0; i < MAXJOBSPERCMDLN; i++)
   {
      joblist[i].filled = 0;
      joblist[i].bg = 0;
      joblist[i].job = NULL;
   }
}

/* clearJobList
 * Clears the job list, freeing the space allocated
 * for strings.
 */
void clearJobList(jobList joblist[MAXJOBSPERCMDLN])
{
   int i;
   for (i = 0; i < MAXJOBSPERCMDLN; i++)
   {
      if (joblist[i].filled == 1) 
      {
         free(joblist[i].job);
         joblist[i].job = NULL;
      }
      joblist[i].filled = 0;
      joblist[i].bg = 0;
   }
}

/* printJobList
 * Outputs the job list.
 */
void printJobList(jobList joblist[MAXJOBSPERCMDLN])
{
   int i;
   for (i = 0; i < MAXJOBSPERCMDLN; i++)
   {
      if (joblist[i].filled == 1)
      {
         printf("job: %s bg: %d\n", joblist[i].job, joblist[i].bg);
      }
   }
}

