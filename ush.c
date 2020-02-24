
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
void closeAllOthers(int i, int cmdCnt, int fds[cmdCnt - 1][2]);

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
    int i,j;
    int fd[cmdCnt - 1][2];
    if(cmdCnt > 1){
        for(j = 0; j < cmdCnt ; j ++){
            pipe(fd[j]); 
        }
    }
    for (i = 0; i <  cmdCnt; i ++) {
        int pid = Fork();
        if (pid == 0) {
            if(i == 0) setpgid(0,0);
            else setpgid(0,pids[0]);
            char buffer[50];
            if(!cmdlist[i].args[0][0] == '.' 
                    && !cmdlist[i].args[0][1] == '/'){ 
                strcpy(buffer, "/bin/");}
            strcat(buffer,cmdlist[i].args[0]);
            if(cmdCnt > 1){     
                if(i == 0){
                    closeAllOthers(i,cmdCnt,fd);
                    close(fd[i][0]); //close read end
                    dup2(fd[i][1],1); //fd 1 now points to file of fd[i][1], ie, fd 3
                    close(fd[i][1]); //close fd 3 since 1 points to the same location anyway
                    Execvp(buffer, cmdlist[i].args);
                    exit(0);
                }
                else if (i == cmdCnt - 1){
                    closeAllOthers(i - 1, cmdCnt, fd); 
                    dup2(fd[i-1][0],0);
                    close(fd[i-1][1]);
                    close(fd[i-1][0]); 
                    Execvp(buffer, cmdlist[i].args);
                    exit(0);
                }
                else 
                {
                    int j;

                    for(j = 0; j < cmdCnt; j ++){
                        if(j != i && j != i - 1){
                            close(fd[j][0]);
                            close(fd[j][1]);
                        }
                    }
                    dup2(fd[i-1][0],0);
                    dup2(fd[i][1],1);
                    close(fd[i-1][0]);
                    close(fd[i-1][1]);
                    close(fd[i][1]);
                    close(fd[i][0]);
                    Execvp(buffer, cmdlist[i].args);
                    exit(0);
                }

            }

            Execvp(buffer, cmdlist[i].args); 
            exit(0);
        }

        pids[i] = pid;
    }
    if(cmdCnt > 1){
        for(j = 0; j < cmdCnt ; j ++){
            close(fd[j][0]); 
            close(fd[j][1]);
        }
    }
    int state;
    state = bg == 0 ? FG : BG;
    int  lastProcess  =  pids[cmdCnt - 1];
    addJob(pids, pids[0], state, job, jobs);
    int jid = pid2jid(pids[0],jobs);
    if(bg == 1){
        printf("[%d] %d\n", jid, lastProcess);
        return;    
    } waitfg(); 
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
            int signal,pid;
            if(strcmp(cmdlist[i].args[0], "-9") == 0){
                signal = SIGKILL;
            }
            else signal = SIGINT;

            if(cmdlist[i].args[2][0] == '%'){
                int j;
                int jid = atoi(&cmdlist[i].args[2][1]);
                jobT* job = getJobJid(jid, jobs); 
                for(j = 0; j < MAXPIDS; j ++){
                    if(job->pid[j] != 0) kill(job->pid[j],signal);
                }
                return 1;
            }
            if(cmdlist[i].args[2][0] == '-') pid = atoi(&cmdlist[i].args[2][1]);
            else pid = atoi(&cmdlist[i].args[2][0]);
            kill(pid,signal);
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
    int status;
    int pid = 0;
    while(pid == 0){pid = waitpid(-1, &status, WNOHANG);}
    if(pid != -1){
        int jid = pid2jid(pid,jobs);
        jobT* job = getJobJid(jid, jobs);
        int state = job->state;
        char buffer[MAXLINE];
        strncpy(buffer,jobs->cmdline,MAXLINE - 1);
        int result = deletePid(pid,jobs);
        if(result == 1 && state == BG){
            if(!WIFEXITED(status)){
                printf("[%d] killed  \t%s\n", jid, buffer);
            }else
            {
                printf("[%d] done \t %s\n", jid, buffer);
            }
        }
    }
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
                kill(jobs[i].pid[j], SIGINT);
            }
        }
    }
    fflush(NULL);
}


void closeAllOthers(int i, int cmdCnt, int fds[cmdCnt - 1][2])
{
    int j;
    for(j = 0; j < cmdCnt; j ++){
        if(i != j){
            close(fds[j][0]);
            close(fds[j][1]);
        }
    }
}

