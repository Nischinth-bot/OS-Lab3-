#include "parser.h"
#include "jobs.h"

#define verbose 0

static int nextjid = 1;

/* clearJob
 * Takes a pointer to a jobT in the jobs array and
 * clears it.
 */
void clearJob(jobT *job) 
{
   int i;
   for (i = 0; i < MAXPIDS; i++)
      job->pid[i] = 0;
   job->pgrp = 0;
   job->jid = 0;
   job->state = UNDEF;
   job->cmdline[0] = '\0';
}

/* initJobs
 * Initializes the jobs array.
 */  
void initJobs(jobT jobs[MAXJOBS]) {
   int i;
   for (i = 0; i < MAXJOBS; i++) clearJob(&jobs[i]);
}

/* maxjid
 * Returns the largest jid in the jobs array.
 */
int maxjid(jobT jobs[MAXJOBS])
{
   int i, max=0;

   for (i = 0; i < MAXJOBS; i++)
      if (jobs[i].jid > max) max = jobs[i].jid;
   return max;
}

/* addJob
 * Add a job to the jobs array given
 * the pids of the processes that make up the job, 
 * the process group id, the state of the job (background
 * or foreground) and the cmdline.
 */
int addJob(pid_t pid[MAXPIDS], int pgrp, int state, 
           char *cmdline, jobT jobs[MAXJOBS])
{
   int i, j;
   
   for (i = 0; i < MAXJOBS; i++) 
   {
      if (jobs[i].state == UNDEF) 
      {
         for (j = 0; j < MAXPIDS; j++) jobs[i].pid[j] = pid[j];
         jobs[i].pgrp = pgrp;
         jobs[i].state = state;
         jobs[i].jid = nextjid++;
         if (nextjid > MAXJOBS) nextjid = 1;
         strcpy(jobs[i].cmdline, cmdline);
         if(verbose)
         {
            printf("Added job [%d] %s\n", jobs[i].jid, jobs[i].cmdline);
         }
         return 1;
      }
   }
   printf("Tried to create too many jobs\n");
   return 0;
}

/* deletePid 
 * Delete a process whose PID=pid from the job list. 
 * Returns 1 if this causes the job to be deleted
 * because it is the last live process that is part of the job. 
 */
int deletePid(pid_t pid, jobT jobs[MAXJOBS])
{
   int i, j, found = 0, index;

   if (pid < 1) return 0;

   for (i = 0; i < MAXJOBS; i++) 
   {
      for (j = 0; j < MAXPIDS; j++)
      {
         if (jobs[i].pid[j] == pid) 
         { 
            found = 1;
            jobs[i].pid[j] = 0;
            index = i;
         }
      }
   }
   if (found)
   {
      //see if all process that are part of this job have terminated
      for (j = 0; j < MAXPIDS; j++) if (jobs[index].pid[j] != 0) return 0;
      clearJob(&jobs[index]);
      nextjid = maxjid(jobs)+1;
      return 1;
   }
   return 0;
}

/* fgJob 
 * Returns a pointer to foreground job.
 * Returns NULL if no such job 
 */
jobT * fgJob(jobT jobs[MAXJOBS]) 
{
   int i;

   for (i = 0; i < MAXJOBS; i++)
      if (jobs[i].state == FG) return &jobs[i];
   return NULL;
}

/* getJobPid  
 * Returns a pointer to the job containing the process
 * with PID = pid. Returns NULL if no such job.
 */ 
jobT *getJobPid(pid_t pid, jobT jobs[MAXJOBS]) {
   int i, j;

   if (pid < 1) return NULL;
   for (i = 0; i < MAXJOBS; i++)
      for (j = 0; j < MAXPIDS; j++) 
         if (jobs[i].pid[j] == pid)
            return &jobs[i];
   return NULL;
}

/* getJobJid 
 * Returns a pointer to the job with the job id equal to jid.
 * Returns NULL if no such job.
 */ 
jobT *getJobJid(int jid, jobT jobs[MAXJOBS])
{
   int i;

   if (jid < 1) return NULL;
   for (i = 0; i < MAXJOBS; i++)
      if (jobs[i].jid == jid)
         return &jobs[i];
   return NULL;
}

/* pid2jid 
 * Returns the jid of the job 
 * that contains the process with PID=pid.
 * Returns 0 if no such job.
 */ 
int pid2jid(pid_t pid, jobT jobs[MAXJOBS])
{
   int i, j;

   if (pid < 1) return 0;
   for (i = 0; i < MAXJOBS; i++)
      for (j = 0; j < MAXPIDS; j++)
         if (jobs[i].pid[j] == pid) 
         {
            return jobs[i].jid;
         }
   return 0;
}

/* listjobs
 * Prints the job list.
 */
void listJobs(jobT jobs[MAXJOBS])
{
   int i;

   for (i = 0; i < MAXJOBS; i++) 
   {
      if (jobs[i].jid != 0) 
      {
         printf("[%d] ", jobs[i].jid);
         switch (jobs[i].state) 
         {
            case BG:
               printf("Running ");
               break;
            case FG:
               printf("Foreground ");
               break;
            case ST:
               printf("Stopped ");
               break;
            default:
               printf("listjobs: Internal error: job[%d].state=%d ",
                      i, jobs[i].state);
         }
         printf("%s &\n", jobs[i].cmdline);
      }
   }
}

