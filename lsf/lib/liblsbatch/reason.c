/* $Id: lsb.reason.c 397 2007-11-26 19:04:00Z mblack $
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include <unistd.h>
#include <pwd.h>

#include "lsb/lsb.h"

#define   NL_SETN     13

static char msgbuf[MSGSIZE];
static char msgline[MAXLINELEN];
struct msgMap
{
    int number;
    char padding[4];
    char *message;
};

static void userIndexReasons (char *, int, int, struct loadIndexLog *);
static char *getMsg (struct msgMap *msgMap, int *msgId, int number);
static void getMsgByRes (int, int, char **, struct loadIndexLog *);

static char *
getMsg (struct msgMap *msgMap, int *msg_ID, int number)
{
    for ( int i = 0; msgMap[i].message != NULL; i++) {
        if (msgMap[i].number == number) {
            return (_i18n_msg_get (ls_catd, NL_SETN, msg_ID[i], msgMap[i].message));
        }
    }

  return ("");
}

char *
lsb_suspreason (int reasons, int subreasons, struct loadIndexLog *ld)
{
    static char __func__] = "lsb_suspreason";
    msgbuf[0] = '\0';

    if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC))  {
        ls_syslog (LOG_DEBUG1, "%s: reasons=%x, subreasons=%d", __func__, reasons, subreasons);
    }


    if (reasons & SUSP_USER_STOP)
/* catgets 500 */
        sprintf (msgbuf, "catgets 500:  The job was suspended by user;\n");   
    else if (reasons & SUSP_ADMIN_STOP)
/* catgets 501 */
        sprintf (msgbuf, "catgets 501:  The job was suspended by LSF admin or root;\n");  
    else if (reasons & SUSP_QUEUE_WINDOW)
/* catgets 502 */
        sprintf (msgbuf, "catgets 502:  The run windows of the queue are closed;\n"); 
    else if (reasons & SUSP_HOST_LOCK)
/* catgets 506 */
        sprintf (msgbuf, "catgets 506:  The execution host is locked by LSF administrator now;\n");   
    else if (reasons & SUSP_HOST_LOCK_MASTER) {
/* catgets 531 */
        sprintf (msgbuf, "catgets 531: The execution host is locked by master LIM now;\n");   
    }
    else if (reasons & SUSP_USER_RESUME)
/* catgets 507 */
        sprintf (msgbuf, "catgets 507:  Waiting for re-scheduling after being resumed by user;\n");   
  else if (reasons & SUSP_QUE_STOP_COND)
/* catgets 508 */
    sprintf (msgbuf, "catgets 508:  STOP_COND is true with current host load;\n");    
  else if (reasons & SUSP_QUE_RESUME_COND)
/* catgets 509 */
    sprintf (msgbuf, "catgets 509:  RESUME_COND is false with current host load;\n"); 
  else if (reasons & SUSP_RES_RESERVE)
/* catgets 510 */
    sprintf (msgbuf, "catgets 510:  Job's requirements for resource reservation not satisfied;\n");   
  else if (reasons & SUSP_PG_IT)
/* catgets 511 */
    sprintf (msgbuf, "catgets 511:  Job was suspended due to paging rate (pg) and the host is not idle yet\n");   

  else if (reasons & SUSP_LOAD_UNAVAIL)
/* catgets 512 */
    sprintf (msgbuf, "catgets 512:  Load information on execution host(s) is unavailable\n"); 
  else if (reasons & SUSP_LOAD_REASON)
    {
/* catgets 513 */
      strcpy (msgbuf, "catgets 513:  Host load exceeded threshold: ");    
      if (subreasons == R15S)
/* catgets 514 */
    sprintf (msgbuf, "catgets 514: %s 15-second CPU run queue length (r15s)\n", msgbuf);  
      else if (subreasons == R1M)
/* catgets 515 */
    sprintf (msgbuf, "catgets 515: %s 1-minute CPU run queue length (r1m)\n", msgbuf);    
      else if (subreasons == R15M)
/* catgets 516 */
    sprintf (msgbuf, "catgets 516: %s 15-minute CPU run queue length (r15m)\n", msgbuf);  
      else if (subreasons == UT)
/* catgets 517 */
    sprintf (msgbuf, "catgets 517: %s 1-minute CPU utilization (ut)\n", msgbuf);  
      else if (subreasons == IO)
/* catgets 518 */
    sprintf (msgbuf, "catgets 518: %s Disk IO rate (io)\n", msgbuf);  
      else if (subreasons == PG)
/* catgets 519 */
    sprintf (msgbuf, "catgets 519: %s Paging rate (pg)\n", msgbuf);   
      else if (subreasons == IT)
/* catgets 520 */
    sprintf (msgbuf, "catgets 520: %s Idle time (it)\n", msgbuf); 
      else if (subreasons == MEM)
/* catgets 521 */
    sprintf (msgbuf, "catgets 521: %s Available memory (mem)\n", msgbuf); 
      else if (subreasons == SWP)
/* catgets 522 */
    sprintf (msgbuf, "catgets 522: %s Available swap space (swp)\n", msgbuf); 
      else if (subreasons == TMP)
/* catgets 523 */
    sprintf (msgbuf, "catgets 523: %s Available /tmp space (tmp)\n", msgbuf); 
      else if (subreasons == LS)
/* catgets 524 */
    sprintf (msgbuf, "catgets 524: %s Number of login sessions (ls)\n", msgbuf);  
      else
    {
      userIndexReasons (msgline, 0, subreasons, ld);
      strcat (msgbuf, "  ");
      strcat (msgbuf, msgline);
      strcat (msgbuf, "\n");
    }
    }
  else if (reasons & SUSP_RES_LIMIT)
    {
      if (subreasons & SUB_REASON_RUNLIMIT)
/* catgets 525 */
    sprintf (msgbuf, "catgets 525:  RUNLIMIT was reached;\n");    
      else if (subreasons & SUB_REASON_DEADLINE)
/* catgets 526 */
    sprintf (msgbuf, "catgets 526:  DEADLINE was reached;\n");    
      else if (subreasons & SUB_REASON_PROCESSLIMIT)
/* catgets 527 */
    sprintf (msgbuf, "catgets 527:  PROCESSLIMIT was reached;\n");    
      else if (subreasons & SUB_REASON_CPULIMIT)
/* catgets 529 */
    sprintf (msgbuf, "catgets 529: CPULIMIT was reached;\n");   
      else if (subreasons & SUB_REASON_MEMLIMIT)
/* catgets 530 */
    sprintf (msgbuf, "catgets 530: MEMLIMIT was reached;\n");   
    }
  else
    sprintf (msgbuf, "catgets 528:  Unknown suspending reason code: %d\n", reasons);  /* catgets 528 */

  return msgbuf;

}

char *
lsb_pendreason (int numReasons, int *rsTb, struct jobInfoHead *jInfoH,
        struct loadIndexLog *ld)
{
  static char __func__] = "lsb_pendreason";
  int i, j, num = 0;
  int hostId, reason, hostIdJ, reasonJ;
  static int *reasonTb, memSize = 0;
  static char *hostList = NULL, *retMsg = NULL;
  char *sp;

  int pendMsg_ID[] = { 550, 551, 552, 553, 554, 555, 556, 557, 558, 559,
    560, 561, 562, 563, 564, 566, 567, 568,
    571, 583,
    655, 656, 586, 587,
    588, 589, 590, 591, 592, 596, 597, 598, 599, 600,
    662, 601, 602, 603, 604, 605, 606, 607,
    608, 609, 610, 611, 612,
    614, 615, 616, 617, 618, 619, 667, 620, 621, 622,
    623, 624, 625, 626, 627, 628, 629, 630, 631,
    633, 634, 635, 636, 638, 639, 640, 641, 642,
    664, 646, 647, 648, 649, 650, 651, 652, 653, 654,
    644, 645, 665, 666
  };

  struct msgMap pendMsg[] = {
    { PEND_JOB_NEW,               "   ", "catgets 550: New job is waiting for scheduling" },                                            /* catgets 550 */
    { PEND_JOB_START_TIME,        "   ", "catgets 551: The job has a specified start time" },                                           /* catgets 551 */
    { PEND_JOB_DEPEND,            "   ", "catgets 552: Job dependency condition not satisfied" },                                       /* catgets 552 */
    { PEND_JOB_DEP_INVALID,       "   ", "catgets 553: Dependency condition invalid or never satisfied" },                              /* catgets 553 */
    { PEND_JOB_MIG,               "   ", "catgets 554: Migrating job is waiting for rescheduling" },                                    /* catgets 554 */
    { PEND_JOB_PRE_EXEC,          "   ", "catgets 555: The job's pre-exec command exited with non-zero status" },                       /* catgets 555 */
    { PEND_JOB_NO_FILE,           "   ", "catgets 556: Unable to access job file" },                                                    /* catgets 556 */
    { PEND_JOB_ENV,               "   ", "catgets 557: Unable to set job's environment variables" },                                    /* catgets 557 */
    { PEND_JOB_PATHS,             "   ", "catgets 558: Unable to determine job's home/working directories" },                           /* catgets 558 */
    { PEND_JOB_OPEN_FILES,        "   ", "catgets 559: Unable to open job's I/O buffers" },                                             /* catgets 559 */
    { PEND_JOB_EXEC_INIT,         "   ", "catgets 560: Job execution initialization failed" },                                          /* catgets 560 */
    { PEND_JOB_RESTART_FILE,      "   ", "catgets 561: Unable to copy restarting job's checkpoint files" },                             /* catgets 561 */
    { PEND_JOB_DELAY_SCHED,       "   ", "catgets 562: The schedule of the job is postponed for a while" },                             /* catgets 562 */
    { PEND_JOB_SWITCH,            "   ", "catgets 563: Waiting for re-scheduling after switching queue" },                              /* catgets 563 */
    { PEND_JOB_DEP_REJECT,        "   ", "catgets 564: Event is rejected by eeventd due to syntax error" },                             /* catgets 564 */
    { PEND_JOB_NO_PASSWD,         "   ", "catgets 566: Failed to get user password" },                                                  /* catgets 566 */
    { PEND_JOB_MODIFY,            "   ", "catgets 568: Waiting for re-scheduling after parameters have been changed" },                 /* catgets 568 */
    { PEND_JOB_REQUEUED,          "   ", "catgets 571: Requeue the job for the next run" },                                             /* catgets 571 */
    { PEND_SYS_UNABLE,            "   ", "catgets 583: System is unable to schedule the job" },                                         /* catgets 583 */
    { PEND_JOB_ARRAY_JLIMIT,      "   ", "catgets 655: The job array has reached its running element limit" },                          /* catgets 655 */
    { PEND_CHKPNT_DIR,            "   ", "catgets 656: Checkpoint directory is invalid" },                                              /* catgets 656 */
    { PEND_QUE_INACT,             "   ", "catgets 586: The queue is inactivated by the administrator" },                                /* catgets 586 */
    { PEND_QUE_WINDOW,            "   ", "catgets 587: The queue is inactivated by its time windows" },                                 /* catgets 587 */
    { PEND_QUE_JOB_LIMIT,         "   ", "catgets 588: The queue has reached its job slot limit" },                                     /* catgets 588 */
    { PEND_QUE_PJOB_LIMIT,        "   ", "catgets 589: The queue has not enough job slots for the parallel job" },                      /* catgets 589 */
    { PEND_QUE_USR_JLIMIT,        "   ", "catgets 590: User has reached the per-user job slot limit of the queue" },                    /* catgets 590 */
    { PEND_QUE_USR_PJLIMIT,       "   ", "catgets 591: Not enough per-user job slots of the queue for the parallel job" },              /* catgets 591 */
    { PEND_QUE_PRE_FAIL,          "   ", "catgets 592: The queue's pre-exec command exited with non-zero status" },                     /* catgets 592 */
    { PEND_SYS_NOT_READY,         "   ", "catgets 596: System is not ready for scheduling after reconfiguration" },                     /* catgets 596 */
    { PEND_SBD_JOB_REQUEUE,       "   ", "catgets 597: Requeued job is waiting for rescheduling" },                                     /* catgets 597 */
    { PEND_JOB_SPREAD_TASK,       "   ", "catgets 598: Not enough hosts to meet the job's spanning requirement" },                      /* catgets 598 */
    { PEND_QUE_SPREAD_TASK,       "   ", "catgets 599: Not enough hosts to meet the queue's spanning requirement" },                    /* catgets 599 */
    { PEND_QUE_WINDOW_WILL_CLOSE, "   ", "catgets 600: Job will not finish before queue's run window is closed" },                      /* catgets 600 */
    { PEND_QUE_PROCLIMIT,         "   ", "catgets 662: Job no longer satisfies queue PROCLIMIT configuration" },                        /* catgets 662 */
    { PEND_USER_JOB_LIMIT,        "   ", "catgets 601: The user has reached his/her job slot limit" },                                  /* catgets 601 */
    { PEND_UGRP_JOB_LIMIT,        "   ", "catgets 602: One of the user's groups has reached its job slot limit" },                      /* catgets 602 */
    { PEND_USER_PJOB_LIMIT,       "   ", "catgets 603: The user has not enough job slots for the parallel job" },                       /* catgets 603 */
    { PEND_UGRP_PJOB_LIMIT,       "   ", "catgets 604: One of user's groups has not enough job slots for the parallel job" },           /* catgets 604 */
    { PEND_USER_RESUME,           "   ", "catgets 605: Waiting for scheduling after resumed by user" },                                 /* catgets 605 */
    { PEND_USER_STOP,             "   ", "catgets 606: The job was suspended by the user while pending" },                              /* catgets 606 */
    { PEND_ADMIN_STOP,            "   ", "catgets 607: The job was suspended by LSF admin or root while pending" },                     /* catgets 607 */
    { PEND_NO_MAPPING,            "   ", "catgets 608: Unable to determine user account for execution" },                               /* catgets 608 */
    { PEND_RMT_PERMISSION,        "   ", "catgets 609: The user has no permission to run the job on remote host/cluster" },             /* catgets 609 */
    { PEND_HOST_RES_REQ,          "   ", "catgets 610: Job's resource requirements not satisfied" },                                    /* catgets 610 */
    { PEND_HOST_NONEXCLUSIVE,     "   ", "catgets 611: Job's requirement for exclusive execution not satisfied" },                      /* catgets 611 */
    { PEND_HOST_JOB_SSUSP,        "   ", "catgets 612: Higher or equal priority jobs suspended by host load" },                         /* catgets 612 */
    { PEND_SBD_GETPID,            "   ", "catgets 614: Unable to get the PID of the restarting job" },                                  /* catgets 614 */
    { PEND_SBD_LOCK,              "   ", "catgets 615: Unable to lock host for exclusively executing the job" },                        /* catgets 615 */
    { PEND_SBD_ZOMBIE,            "   ", "catgets 616: Cleaning up zombie job" },                                                       /* catgets 616 */
    { PEND_SBD_ROOT,              "   ", "catgets 617: Can't run jobs submitted by root" },                                             /* catgets 617 */
    { PEND_HOST_WIN_WILL_CLOSE,   "   ", "catgets 618: Job will not finish on the host before queue's run window is closed" },          /* catgets 618 */
    { PEND_HOST_MISS_DEADLINE,    "   ", "catgets 619: Job will not finish on the host before job's termination deadline" },            /* catgets 619 */
    { PEND_FIRST_HOST_INELIGIBLE, "   ", "catgets 667: The specified first exection host is not eligible for this job at this time" },  /* catgets 667 */
    { PEND_HOST_DISABLED,         "   ", "catgets 620: Closed by LSF administrator" },                                                  /* catgets 620 */
    { PEND_HOST_LOCKED,           "   ", "catgets 621: Host is locked by LSF administrator" },                                          /* catgets 621 */
    { PEND_HOST_LESS_SLOTS,       "   ", "catgets 622: Not enough job slot(s)" },                                                       /* catgets 622 */
    { PEND_HOST_WINDOW,           "   ", "catgets 623: Dispatch windows closed" },                                                      /* catgets 623 */
    { PEND_HOST_JOB_LIMIT,        "   ", "catgets 624: Job slot limit reached" },                                                       /* catgets 624 */
    { PEND_QUE_PROC_JLIMIT,       "   ", "catgets 625: Queue's per-CPU job slot limit reached" },                                       /* catgets 625 */
    { PEND_QUE_HOST_JLIMIT,       "   ", "catgets 626: Queue's per-host job slot limit reached" },                                      /* catgets 626 */
    { PEND_USER_PROC_JLIMIT,      "   ", "catgets 627: User's per-CPU job slot limit reached" },                                        /* catgets 627 */
    { PEND_UGRP_PROC_JLIMIT,      "   ", "catgets 628: User group's per-CPU job slot limit reached" },                                  /* catgets 628 */
    { PEND_HOST_USR_JLIMIT,       "   ", "catgets 629: Host's per-user job slot limit reached" },                                       /* catgets 629 */
    { PEND_HOST_QUE_MEMB,         "   ", "catgets 630: Not usable to the queue" },                                                      /* catgets 630 */
    { PEND_HOST_USR_SPEC,         "   ", "catgets 631: Not specified in job submission" },                                              /* catgets 631 */
    { PEND_HOST_NO_USER,          "   ", "catgets 633: There is no such user account" },                                                /* catgets 633 */
    { PEND_HOST_ACCPT_ONE,        "   ", "catgets 634: Just started a job recently" },                                                  /* catgets 634 */
    { PEND_LOAD_UNAVAIL,          "   ", "catgets 635: Load information unavailable" },                                                 /* catgets 635 */
    { PEND_HOST_NO_LIM,           "   ", "catgets 636: LIM is unreachable now" },                                                       /* catgets 636 */
    { PEND_HOST_QUE_RESREQ,       "   ", "catgets 638: Queue's resource requirements not satisfied" },                                  /* catgets 638 */
    { PEND_HOST_SCHED_TYPE,       "   ", "catgets 639: Not the same type as the submission host" },                                     /* catgets 639 */
    { PEND_JOB_NO_SPAN,           "   ", "catgets 640: Not enough processors to meet the job's spanning requirement" },                 /* catgets 640 */
    { PEND_QUE_NO_SPAN,           "   ", "catgets 641: Not enough processors to meet the queue's spanning requirement" },               /* catgets 641 */
    { PEND_HOST_EXCLUSIVE,        "   ", "catgets 642: Running an exclusive job" },                                                     /* catgets 642  */
    { PEND_HOST_LOCKED_MASTER,    "   ", "catgets 664: Host is locked by master LIM" },                                                 /* catgets 664 */
    { PEND_SBD_UNREACH,           "   ", "catgets 646: Unable to reach slave batch server" },                                           /* catgets 646 */
    { PEND_SBD_JOB_QUOTA,         "   ", "catgets 647: Number of jobs exceeds quota" },                                                 /* catgets 647 */
    { PEND_JOB_START_FAIL,        "   ", "catgets 648: Failed in talking to server to start the job" },                                 /* catgets 648 */
    { PEND_JOB_START_UNKNWN,      "   ", "catgets 649: Failed in receiving the reply from server when starting the job" },              /* catgets 649 */
    { PEND_SBD_NO_MEM,            "   ", "catgets 650: Unable to allocate memory to run job" },                                         /* catgets 650 */
    { PEND_SBD_NO_PROCESS,        "   ", "catgets 651: Unable to fork process to run job" },                                            /* catgets 651 */
    { PEND_SBD_SOCKETPAIR,        "   ", "catgets 652: Unable to communicate with job process" },                                       /* catgets 652 */
    { PEND_SBD_JOB_ACCEPT,        "   ", "catgets 653: Slave batch server failed to accept job" },                                      /* catgets 653 */
    { PEND_HOST_LOAD,             "   ", "catgets 654: Load threshold reached" },                                                       /* catgets 654 */
    { PEND_HOST_QUE_RUSAGE,       "   ", "catgets 644: Queue's requirements for resource reservation not satisfied" },                  /* catgets 644 */
    { PEND_HOST_JOB_RUSAGE,       "   ", "catgets 645: Job's requirements for resource reservation not satisfied" },                    /* catgets 645 */
    { PEND_BAD_HOST,              "   ", "catgets 665: Bad host name, host group name or cluster name" },                               /* catgets 665 */
    { PEND_QUEUE_HOST,            "   ", "catgets 666: Host or host group is not used by the queue" },                                  /* catgets 666 */

    {0, "    ", NULL}
  };

  if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC))
    ls_syslog (LOG_DEBUG1, "%s: numReasons=%d", __func__, numReasons);

  if (!numReasons || !rsTb)
    {
      lsberrno = LSBE_BAD_ARG;
      return ("");
    }
  if (memSize < numReasons)
    {
      FREEUP (reasonTb);
      assert( numReasons >= 0);
      reasonTb = (int *) calloc ((unsigned long)numReasons, sizeof (int));
      if ( NULL == reasonTb && ENOMEM == errno )
    {
      memSize = 0;
      lsberrno = LSBE_NO_MEM;
      return ("");
    }
      memSize = numReasons;
    }
  for (i = 0; i < numReasons; i++)
    reasonTb[i] = rsTb[i];

  FREEUP (hostList);
  FREEUP (retMsg);
  if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL)
    {
      assert( jInfoH->numHosts >= 0 );
      hostList = malloc ((unsigned long)jInfoH->numHosts * MAXHOSTNAMELEN);
      retMsg = malloc ((unsigned long)jInfoH->numHosts * MAXHOSTNAMELEN + MSGSIZE);
      if ( ( NULL == hostList && ENOMEM == errno) || 
           ( NULL == retMsg   && ENOMEM == errno ) )
    {
      lsberrno = LSBE_NO_MEM;
      return ("");
    }
    }
  else
    {
      retMsg = malloc (MSGSIZE);
      if (retMsg == NULL)
    {
      lsberrno = LSBE_NO_MEM;
      return ("");
    }
    }

  retMsg[0] = '\0';
  for (i = 0; i < numReasons; i++)
    {
      if (!reasonTb[i])
    continue;
      GET_LOW (reason, reasonTb[i]);
      if (!reason)
    continue;
      GET_HIGH (hostId, reasonTb[i]);
      if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC))
    ls_syslog (LOG_DEBUG2, "%s: hostId=%d, reason=%d reasonTb[%d]=%d",
           __func__, hostId, reason, i, reasonTb[i]);
      if (!hostId)
    {
      sprintf (msgline, " %s;\n", getMsg (pendMsg, pendMsg_ID, reason));
      strcat (retMsg, msgline);
      continue;
    }
      if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL)
    strcpy (hostList, jInfoH->hostNames[hostId - 1]);
      else
    num = 1;

      for (j = i + 1; j < numReasons; j++)
    {
      if (reasonTb[j] == 0)
        continue;
      GET_LOW (reasonJ, reasonTb[j]);
      if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC))
        ls_syslog (LOG_DEBUG2, "%s: reasonJ=%d reasonTb[j]=%d",
               __func__, reasonJ, reasonTb[j]);
      if (reasonJ != reason)
        continue;
      GET_HIGH (hostIdJ, reasonTb[j]);
      if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC))
        ls_syslog (LOG_DEBUG2, "%s: j=%d, hostIdJ=%d", __func__, j, hostIdJ);
      reasonTb[j] = 0;
      if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL)
        {
          sprintf (hostList, "%s, %s", hostList,
               jInfoH->hostNames[hostIdJ - 1]);
        }
      else
        num++;
    }
      if (reason >= PEND_HOST_LOAD && reason < PEND_HOST_QUE_RUSAGE)
    {

      getMsgByRes (reason - PEND_HOST_LOAD, PEND_HOST_LOAD, &sp, ld);

    }
      else if (reason >= PEND_HOST_QUE_RUSAGE
           && reason < PEND_HOST_JOB_RUSAGE)
    {

      getMsgByRes (reason - PEND_HOST_QUE_RUSAGE,
               PEND_HOST_QUE_RUSAGE, &sp, ld);

    }
      else if (reason >= PEND_HOST_JOB_RUSAGE)
    {

      getMsgByRes (reason - PEND_HOST_JOB_RUSAGE,
               PEND_HOST_JOB_RUSAGE, &sp, ld);

    }
      else
    {
      sp = getMsg (pendMsg, pendMsg_ID, reason);
    }

      if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL)
    sprintf (retMsg, "%s %s: %s;\n", retMsg, sp, hostList);
      else if (num == 1)
    /* catgets 713 */
    sprintf (retMsg, "catgets 713: %s %s: 1 host;\n", retMsg, sp);
      else
    /* catgets 714 */
    sprintf (retMsg, "catgets 714: %s %s: %d hosts;\n", retMsg, sp, num);
    }

  return retMsg;
}


static void
userIndexReasons (char *msgline_, int resource, int reason, struct loadIndexLog *ld)
{

  if (ld == NULL || reason <= MEM || resource >= ld->nIdx)
    {
      /* catgets 711 */
      sprintf (msgline_, "catgets 711: External load index is beyond threshold");
      return;
    }

  if (reason == PEND_HOST_LOAD)
    {
      /* catgets 712 */
      sprintf (msgline_, "catgets 712: External load index (%s) is beyond threshold", ld->name[resource]);
    }
  else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 737 */
      sprintf (msgline_, "catgets 737: Queue requirements for reserving resource (%s) not satisfied", ld->name[resource]);
    }
  else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 738 */
      sprintf (msgline_, "catgets 738: Job's requirements for reserving resource (%s) not satisfied", ld->name[resource]);
    }

}

static void
getMsgByRes (int resource, int reason, char **sp, struct loadIndexLog *ld)
{
  switch (resource)
    {
    case R15S:

      if (reason == PEND_HOST_LOAD)
    {
    /* catgets 700 */
      sprintf (msgline, "catgets 700: The 15s effective CPU queue length (r15s) is beyond threshold");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 715 */
      sprintf (msgline, "catgets 715: Queue requirements for reserving resource (r15s) not satisfied ");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 716 */
      sprintf (msgline, "catgets 716: Job requirements for reserving resource (r15s) not satisfied");
    }
      break;

    case R1M:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 701 */
      sprintf (msgline, "catgets 701: The 1 min effective CPU queue length (r1m) is beyond threshold");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 717 */
      sprintf (msgline, "catgets 717: Queue requirements for reserving resource (r1m) not satisfied ");   
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 718 */
      sprintf (msgline, "catgets 718: Job requirements for reserving resource (r1m) not satisfied");
    }
      break;

    case R15M:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 702 */
      sprintf (msgline, "catgets 702: The 15 min effective CPU queue length (r15m) is beyond threshold");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 719 */
      sprintf (msgline, "catgets 719: Queue requirements for reserving resource (r15m) not satisfied ");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 720 */
      sprintf (msgline, "catgets 720: Job requirements for reserving resource (r15m) not satisfied");
    }
      break;

    case UT:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 703 */
      sprintf (msgline, "catgets 703: The CPU utilization (ut) is beyond threshold");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 721 */
      sprintf (msgline, "catgets 721: Queue requirements for reserving resource (ut) not satisfied ");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 722 */
      sprintf (msgline, "catgets 722: Job requirements for reserving resource (ut) not satisfied");
    }
      break;

    case PG:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 704 */
      sprintf (msgline, "catgets 704: The paging rate (pg) is beyond threshold");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 723 */
      sprintf (msgline, "catgets 723: Queue requirements for reserving resource (pg) not satisfied");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 724 */
      sprintf (msgline, "catgets 724: Job requirements for reserving resource (pg) not satisfied");
    }
      break;

    case IO:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 705 */
      sprintf (msgline, "catgets 705: The disk IO rate (io) is beyond threshold");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 725 */
      sprintf (msgline, "catgets 725: Queue requirements for reserving resource (io) not satisfied");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 726 */
      sprintf (msgline, "catgets 726: Job requirements for reserving resource (io) not satisfied");   
    }
      break;

    case LS:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 706 */
      sprintf (msgline,"catges 706: There are too many login users (ls)");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 727 */
      sprintf (msgline, "catgets 727: Queue requirements for reserving resource (ls) not satisfied");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 728 */
      sprintf (msgline, "catgets 728: Job requirements for reserving resource (ls) not satisfied");
    }
      break;

    case IT:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 707 */
      sprintf (msgline, "catges 707: The idle time (it) is not long enough");    
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 729 */
      sprintf (msgline, "catgets 729: Queue requirements for reserving resource (it) not satisfied"); 
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 730 */
      sprintf (msgline, "catgets 730: Job requirements for reserving resource (it) not satisfied");
    }
      break;

    case TMP:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 708 */
      sprintf (msgline, "catgets 708: The available /tmp space (tmp) is low");    
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 731 */
      sprintf (msgline, "catges 731: Queue requirements for reserving resource (tmp) not satisfied");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 732 */
      sprintf (msgline, "catgets 732: Job requirements for reserving resource (tmp) not satisfied");
    }
      break;

    case SWP:

      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 709 */
      sprintf (msgline, "catges 709: The available swap space (swp) is low");
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 733 */
      sprintf (msgline, "catges 733: Queue requirements for reserving resource (swp) not satisfied");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 734 */
      sprintf (msgline, "catgets 734: Job requirements for reserving resource (swp) not satisfied");
    }
      break;

    case MEM:
      if (reason == PEND_HOST_LOAD)
    {
      /* catgets 710 */
      sprintf (msgline, "catgets 710: The available memory (mem) is low");    
    }
      else if (reason == PEND_HOST_QUE_RUSAGE)
    {
      /* catgets 735 */
      sprintf (msgline, "catgets 735: Queue requirements for reserving resource (mem) not satisfied");
    }
      else if (reason == PEND_HOST_JOB_RUSAGE)
    {
      /* catgets 736 */
      sprintf (msgline, "catgets 736: Job requirements for reserving resource (mem) not satisfied"); 
    }
      break;

    default:
      userIndexReasons (msgline, resource, reason, ld);
      break;
    }

  *sp = msgline;
}
