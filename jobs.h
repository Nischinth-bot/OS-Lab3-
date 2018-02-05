/* 
 *  Jobs states: FG (foreground), BG (background), ST (stopped)
 *  Job state transitions and enabling actions:
 *  FG -> ST  : ctrl-z
 *  ST -> FG  : fg command
 *  ST -> BG  : bg command
 *  BG -> FG  : fg command
 *  At most 1 job can be in the FG state.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */
#define MAXJOBS 16
#define MAXPIDS 16

typedef struct             /* The job struct */
{
   pid_t pid[MAXPIDS];     /* PIDs of processes that make up the job */
   pid_t pgrp;             /* process group id */
   int jid;                /* job ID [1, 2, ...] */
   int state;              /* UNDEF, BG, FG, or ST */
   char cmdline[MAXLINE];  /* command line */
} jobT;

void clearJob(jobT *job);
void initJobs(jobT jobs[MAXJOBS]);
int maxjid(jobT jobs[MAXJOBS]);
int addJob(pid_t pid[MAXPIDS], int pgrp, int state, 
           char *cmdline, jobT jobs[MAXJOBS]);
int deletePid(pid_t pid, jobT jobs[MAXJOBS]);
jobT *fgJob(jobT jobs[MAXJOBS]);
jobT *getJobPid(pid_t pid, jobT jobs[MAXJOBS]);
jobT *getJobJid(int jid, jobT jobs[MAXJOBS]);
int pid2jid(pid_t pid, jobT jobs[MAXJOBS]);
void listJobs(jobT jobs[MAXJOBS]);

