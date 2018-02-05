#define MAXLINE                500  /* max line size */
#define MAXARGS                10   /* max number of command line args */
#define MAXLEN                 50   /* max length of an individual argument */
#define MAXJOBSPERCMDLN        10   /* max number of jobs in a command line */
#define MAXCMDSPERJOB          10   /* max number of commands in a job */

typedef struct
{
   char * job;      /* the commands that make up a job: cmd1 23 | cmd2 */
   int filled;      /* 1 ifthis job entry is filled */
   int bg;          /* 1 if the job runs in the background */
} jobList;

typedef struct
{
   char * cmd;            /* a single command and its arguments */
   char * args[MAXARGS];  /* the parsed command and its args */
   int filled;            /* 1 if this cmdlist entry is filled */
   int pipe;              /* 1 if the output of this command is piped */
} cmdList;

//jobs are separated by &
void initJobList(jobList joblist[MAXJOBSPERCMDLN]);
void parseIntoJobs(char * cmdline, jobList joblist[MAXJOBSPERCMDLN]); 
void printJobList(jobList joblist[MAXJOBSPERCMDLN]); 

//commands are jobs separated by |
void initCmdList(cmdList cmdlist[MAXCMDSPERJOB]);
void parseIntoCmds(char * job, cmdList cmdlist[MAXCMDSPERJOB]); 
void printCmdList(cmdList cmdlist[MAXCMDSPERJOB]); 
int getCmdCount(cmdList cmdlist[MAXCMDSPERJOB]);

