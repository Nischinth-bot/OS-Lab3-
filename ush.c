
#include "wrappers.h"
#include "parser.h"
#include "jobs.h"

jobT jobs[MAXJOBS];     /* The job list */ 

void waitfg();
void sigchildHandler(int sig);
void sigintHandler(int sig);
void evalCmdLine(char *cmdline);
void evalJob(char * job, int bg);
int builtin(char * job); 

/**HELPER METHODS**/


/* The main drives the shell process.  Basically a shell reads
 * input, handles the input by executing a command in the foreground
 * or background, and repeats. 
 */
int main()
{
    char commandline[MAXLINE];
    int bytes;

    /* initialize the job list */
    initJobs(jobs);

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigintHandler);    /* ctrl-c entered at ush prompt*/
    Signal(SIGCHLD, sigchildHandler);  /* Terminated child */

    printf("ush> ");
    fflush(NULL);  //flush prompt

    bytes = read(0, commandline, MAXLINE - 1);
    if (bytes > 0) commandline[bytes - 1] = '\0'; 
    while (1) //quit in builtin
    {
        //if the number of bytes is 1 then the
        //user simply entered a newline
        if (bytes > 1) evalCmdLine(commandline);
        printf("ush> ");
        fflush(NULL);
        bytes = read(0, commandline, MAXLINE - 1);
        if (bytes > 0) commandline[bytes - 1] = '\0'; 
    }
    return 0;
}      

/* evalCmdLine
 * Takes as input a command line. Calls the parseIntoJobs
 * function to break the command line into jobs.
 * Calls evalJob on each job that is not "built into" the
 * shell.
 * 
 */
void evalCmdLine(char * cmdline)
{
    int i;
    jobList joblist[MAXJOBSPERCMDLN];

    //Parse the command line into jobs
    initJobList(joblist);
    parseIntoJobs(cmdline, joblist);

    i = 0;
    while (joblist[i].filled == 1)
    {
        //if the job starts with a built-in command like quit then
        //don't evaluate it (builtin will evaluate it)
        if (!builtin(joblist[i].job))
        {
            evalJob(joblist[i].job, joblist[i].bg);
        }
        i++;
    }
    return;
}

/******* You need to write these functions *********/
/* evalJob
 * This function takes a job, which may consist of a set of
 * commands separated by pipes. Each command is executed
 * by a new process.  A single job is created and added
 * to the joblist. The set of pids associated with the job
 * are stored in the job entry.
 */

void evalJob(char * job, int bg)
{
    int pids[MAXPIDS] = {0};
    int cmdCnt;
    //parse the job into commands
    //see parseIntoCmds documentation in parser.c
    cmdList cmdlist[MAXCMDSPERJOB];
    initCmdList(cmdlist);
    parseIntoCmds(job, cmdlist);
    //get the number of commands
    cmdCnt = getCmdCount(cmdlist);
    /* You'll need to execute a Fork and an Execvp for
     * each command.  Before creating any children, block the 
     * SIGINT and SIGCHLD signals.  This will allow you to 
     * add the job to the job list before those signals are handled.
     */
    int i;
    char* args = "";
    for (i = 0; i <  cmdCnt; i ++) {
        int pid = Fork();
        if (pid == 0) {
            char buffer[50];
            strcpy(buffer, "/bin/");
            strcat(buffer,cmdlist[i].args[0]);
            int result = Execvp(buffer, cmdlist[i].args); 
            exit(0);
        }
        pids[i] = pid;
    }
    for(i = 0; i < sizeof(pids); i ++)
    {
        int status;
        Waitpid(pids[i], &status, 0);
    }
}



/* builtin
 * Handles the builtin commands: jobs, quit, kill
 * Returns 1 if the job passed in is a built-in command
 * and 0 otherwise. Should handle:
 * quit - exits shell
 * jobs - lists the jobs (calls listJobs)
 * kill - handles SIGKILL (-9) and SIGINT (-2) only
 *      - can provide a job number preceded by a %,
 *        a group pid preceded by a - or a pid
 *        kill -9 12345
 *        kill -2 %1
 *        kill -2 -12345
 */
int builtin(char * job) 
{
    //Parse the job into commands
    cmdList cmdlist[MAXCMDSPERJOB];
    initCmdList(cmdlist);
    parseIntoCmds(job, cmdlist);
    int i;
    for(i = 0; i < MAXCMDSPERJOB; i ++) {
        if (strcmp(cmdlist[i].args[0],"quit") == 0 ) {
            exit(0);
            return 1;
        }
        if (strcmp(cmdlist[i].args[0],"jobs") == 0) {
            listJobs(jobs);
            return 1;
        }
        if (strcmp(cmdlist[i].args[0], "kill") == 0) {
            int result = Execvp(cmdlist[i].args[0], cmdlist[i].args);
            //kill(SIGKILL, pids[i]):
            return 1;
        }
        return 0;
    }
}

/* waitfg
 * Calls sleep(1) within a loop, while the fgJobs(jobs) is 
 * not NULL.  fgJobs(jobs) returns a pointer to the
 * foreground job.
 */
void waitfg()
{
    while ( fgJob(jobs) != NULL ){
        sleep(1);
    }
    return;
}

/*
 * sigchildHandler
 * This is called when the shell receives the SIGCHLD signal (one if
 * its children has terminated).  It should reap all terminated child
 * process. If the process is the foreground process, nothing is
 * printed in response.  However if the process is a background process,
 * it should print either:
 * jid killed
 * if the process terminated abnormally (for example, by a CTRL-C).
 * or
 * jid done
 * if the process terminated normally. It will only print if the
 * job is finished.  The job is finished when all processes within the
 * job terminate.
 */
void sigchildHandler(int sig)
{
}

/*
 * sigintHandler
 * This handler is executed if the shell is sent a SIGINT signal.
 * If there is a foreground process job, the handler should use
 * the kill command to send the signal to the foreground process
 * job using the foreground process group id.
 */
void sigintHandler(int sig)
{
    int i;
    for (i = 0; i < MAXJOBS; i ++) {
        if(jobs[i].state == FG){
            int j;
            for (j = 0; j < MAXPIDS; j ++) {
                kill(SIGINT, jobs[i].pid[j]);
            }
        }
    }
}

