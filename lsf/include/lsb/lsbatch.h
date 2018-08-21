/* $Id: lsbatch.h 397 2007-11-26 19:04:00Z mblack $
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

#pragma once

#include "lsf.h"
#include "lsb/misc.h"

#define _PATH_NULL      "/dev/null"

#define MAX_VERSION_LEN     12
#define MAX_HPART_USERS     100
#define MAX_GROUPS          150
#define MAX_CHARLEN         20
#define MAX_LSB_NAME_LEN    60
#define MAX_CMD_DESC_LEN    256
#define MAX_USER_EQUIVALENT 128

#define DEFAULT_MSG_DESC    "no description"

#define MAX_NRLIMITS      512
#define MAXQUEUENAMELEN   512
#define MAXJOBDESPLEN     1024
#define MAXVERSIONLEN     128 


#define HOST_STAT_OK         0x0
#define HOST_STAT_BUSY       0x01
#define HOST_STAT_WIND       0x02
#define HOST_STAT_DISABLED   0x04
#define HOST_STAT_LOCKED     0x08
#define HOST_STAT_FULL       0x10
#define HOST_STAT_UNREACH    0x20
#define HOST_STAT_UNAVAIL    0x40
#define HOST_STAT_NO_LIM     0x80
#define HOST_STAT_EXCLUSIVE  0x100
#define HOST_STAT_LOCKED_MASTER 0x200

#define LSB_HOST_OK(status)      (status == HOST_STAT_OK)

#define LSB_HOST_BUSY(status)    ((status & HOST_STAT_BUSY) != 0)

#define LSB_HOST_CLOSED(status)  ((status & (HOST_STAT_WIND | HOST_STAT_DISABLED | HOST_STAT_LOCKED | HOST_STAT_LOCKED_MASTER | HOST_STAT_FULL | HOST_STAT_NO_LIM)) != 0)

#define LSB_HOST_FULL(status)    ((status & HOST_STAT_FULL) != 0)

#define LSB_HOST_UNREACH(status) ((status & HOST_STAT_UNREACH) != 0)

#define LSB_HOST_UNAVAIL(status) ((status & HOST_STAT_UNAVAIL) != 0)

#define HOST_BUSY_NOT          0x000
#define HOST_BUSY_R15S         0x001
#define HOST_BUSY_R1M          0x002
#define HOST_BUSY_R15M         0x004
#define HOST_BUSY_UT           0x008
#define HOST_BUSY_PG           0x010
#define HOST_BUSY_IO           0x020
#define HOST_BUSY_LS           0x040
#define HOST_BUSY_IT           0x080
#define HOST_BUSY_TMP          0x100
#define HOST_BUSY_SWP          0x200
#define HOST_BUSY_MEM          0x400

#define LSB_ISBUSYON(status, index)  \
(((status[(index)/INTEGER_BITS]) & (1 << (index)%INTEGER_BITS)) != 0)

#define QUEUE_STAT_OPEN         0x01
#define QUEUE_STAT_ACTIVE       0x02
#define QUEUE_STAT_RUN          0x04
#define QUEUE_STAT_NOPERM       0x08
#define QUEUE_STAT_DISC         0x10
#define QUEUE_STAT_RUNWIN_CLOSE 0x20

#define QUEUE_ATTRIB_EXCLUSIVE        0x01
#define QUEUE_ATTRIB_DEFAULT          0x02
#define QUEUE_ATTRIB_ROUND_ROBIN      0x04
#define QUEUE_ATTRIB_BACKFILL         0x80
#define QUEUE_ATTRIB_HOST_PREFER      0x100
#define QUEUE_ATTRIB_NO_INTERACTIVE   0x800
#define QUEUE_ATTRIB_ONLY_INTERACTIVE 0x1000
#define QUEUE_ATTRIB_NO_HOST_TYPE     0x2000
#define QUEUE_ATTRIB_IGNORE_DEADLINE  0x4000
#define QUEUE_ATTRIB_CHKPNT           0x8000
#define QUEUE_ATTRIB_RERUNNABLE       0x10000
#define QUEUE_ATTRIB_ENQUE_INTERACTIVE_AHEAD 0x80000

#define MASTER_NULL           200
#define MASTER_RESIGN         201
#define MASTER_RECONFIG       202
#define MASTER_FATAL          203
#define MASTER_MEM            204
#define MASTER_CONF           205

#define JOB_STAT_NULL         0x00
#define JOB_STAT_PEND         0x01
#define JOB_STAT_PSUSP        0x02
#define JOB_STAT_RUN          0x04
#define JOB_STAT_SSUSP        0x08
#define JOB_STAT_USUSP        0x10
#define JOB_STAT_EXIT         0x20
#define JOB_STAT_DONE         0x40
#define JOB_STAT_PDONE        0x80
#define JOB_STAT_PERR         0x100
#define JOB_STAT_WAIT         0x200
#define JOB_STAT_UNKWN        0x10000


// EVENT_JOB_*
#define EVENT_JOB_NEW          1
#define EVENT_JOB_START        2
#define EVENT_JOB_STATUS       3
#define EVENT_JOB_SWITCH       4
#define EVENT_JOB_MOVE         5
#define EVENT_JOB_FINISH       10
#define EVENT_JOB_MODIFY       16
#define EVENT_JOB_SIGNAL       17
#define EVENT_JOB_EXECUTE      18
#define EVENT_JOB_MSG          19
#define EVENT_JOB_MSG_ACK      20
#define EVENT_JOB_REQUEUE      21
#define EVENT_JOB_SIGACT       22
#define EVENT_JOB_START_ACCEPT 24
#define EVENT_JOB_CLEAN        25
#define EVENT_JOB_FORCE        26
#define EVENT_JOB_MODIFY2      28
#define EVENT_JOB_ATTR_SET     29

#define EVENT_QUEUE_CTRL       6

#define EVENT_MBD_START        15
#define EVENT_MBD_DIE          8
#define EVENT_MBD_UNFULFILL    9

#define EVENT_HOST_CTRL        7

#define EVENT_LOAD_INDEX       11

#define EVENT_CHKPNT           12

#define EVENT_MIG              13

#define EVENT_PRE_EXEC_START   14

#define EVENT_SBD_JOB_STATUS   23

#define EVENT_LOG_SWITCH       27

#define EVENT_UNUSED_30        30
#define EVENT_UNUSED_31        31
#define EVENT_UNUSED_32        32


enum pendingCatgets {
	PEND_JOB_REASON = 0, 	//
	PEND_JOB_NEW,   		// from lsbatch.h
	PEND_JOB_START_TIME,
	PEND_JOB_DEPEND,
	PEND_JOB_DEP_INVALID,
	PEND_JOB_MIG,
	PEND_JOB_PRE_EXEC,
	PEND_JOB_NO_FILE,
	PEND_JOB_ENV,
	PEND_JOB_PATHS,
	PEND_JOB_OPEN_FILES,
	PEND_JOB_EXEC_INIT,
	PEND_JOB_RESTART_FILE,
	PEND_JOB_DELAY_SCHED,
	PEND_JOB_SWITCH,
	PEND_JOB_DEP_REJECT,
	PEND_JOB_NO_PASSWD,
	PEND_JOB_UKNOWN_567,
	PEND_JOB_MODIFY,
	PEND_JOB_REQUEUED,
	PEND_JOB_ARRAY_JLIMIT,
	PEND_CHKPNT_DIR,
	PEND_SYS_UNABLE,
	PEND_QUEUE_INACT,
	PEND_QUEUE_WINDOW,
	PEND_QUEUE_JOB_LIMIT,
	PEND_QUEUE_PJOB_LIMIT,
	PEND_QUEUE_USR_JLIMIT,
	PEND_QUEUE_USR_PJLIMIT,
	PEND_QUEUE_PRE_FAIL,
	PEND_SYS_NOT_READY,
	PEND_SBD_JOB_REQUEUE,
	PEND_JOB_SPREAD_TASK,
	PEND_QUEUE_SPREAD_TASK,
	PEND_QUEUE_WINDOW_WILL_CLOSE,
	PEND_USER_JOB_LIMIT,
	PEND_UGRP_JOB_LIMIT,
	PEND_USER_PJOB_LIMIT,
	PEND_UGRP_PJOB_LIMIT,
	PEND_USER_RESUME,
	PEND_USER_STOP,
	PEND_ADMIN_STOP,
	PEND_NO_MAPPING,
	PEND_RMT_PERMISSION,
	PEND_HOST_RES_REQ,
	PEND_HOST_NONEXCLUSIVE,
	PEND_HOST_JOB_SSUSP,
	PEND_SBD_GETPID,
	PEND_SBD_LOCK,
	PEND_SBD_ZOMBIE,
	PEND_SBD_ROOT,
	PEND_HOST_WIN_WILL_CLOSE,
	PEND_HOST_MISS_DEADLINE,
	PEND_HOST_DISABLED,
	PEND_HOST_LOCKED,
	PEND_HOST_LESS_SLOTS,
	PEND_HOST_WINDOW,
	PEND_HOST_JOB_LIMIT,
	PEND_QUEUE_PROC_JLIMIT,
	PEND_QUEUE_HOST_JLIMIT,
	PEND_USER_PROC_JLIMIT,
	PEND_UGRP_PROC_JLIMIT,
	PEND_HOST_USR_JLIMIT,
	PEND_HOST_QUEUE_MEMB,
	PEND_HOST_USR_SPEC,
	PEND_HOST_NO_USER,
	PEND_HOST_ACCPT_ONE,
	PEND_LOAD_UNAVAIL,
	PEND_HOST_NO_LIM,
	PEND_HOST_QUEUE_RESREQ,
	PEND_HOST_SCHED_TYPE,
	PEND_JOB_NO_SPAN,
	PEND_QUEUE_NO_SPAN,
	PEND_HOST_EXCLUSIVE,
	PEND_HOST_QUEUE_RUSAGE,
	PEND_HOST_JOB_RUSAGE,
	PEND_SBD_UNREACH,
	PEND_SBD_JOB_QUOTA,
	PEND_JOB_START_FAIL, 
	PEND_JOB_START_UNKNWN,
	PEND_SBD_NO_MEM,
	PEND_SBD_NO_PROCESS,
	PEND_SBD_SOCKETPAIR,
	PEND_SBD_JOB_ACCEPT,
	PEND_HOST_LOAD,
	PEND_UKNOWN_656,
	PEND_QUEUE_PROCLIMIT,
	PEND_HOST_LOCKED_MASTER,
	PEND_BAD_HOST,
	PEND_QUEUE_HOST,
	PEND_FIRST_HOST_INELIGIBLE
};

int pendMsg_ID[] = { 
	550, 551, 552, 553, 554, 555, 556, 557, 558, 559,
	560, 561, 562, 563, 564,      566, 567, 568,
	571, 
	583,           586, 587, 588, 589,
	590, 591, 592,                596, 597, 598, 599,
	600, 601, 602, 603, 604, 605, 606, 607, 608, 609,
	610, 611, 612,      614, 615, 616, 617, 618, 619,
	620, 621, 622, 623, 624, 625, 626, 627, 628, 629,
	630, 631,      633, 634, 635, 636,      638, 639,
	640, 641, 642,      644, 645, 646, 647, 648, 649,
	650, 651, 652, 653, 654, 655, 656,
	662,      664, 665, 666, 667
};

struct msgMap
{
	int number;
	const char padding[4];
	const char *message;
};

struct msgMap pendMsg[] = {
// sorted by catgets number
// 		empty lines are meant to draw attention to gaps in numbering
// 	if discrepancies found during The Dig, they are pointed out.
//

// the reason there is a 4-letter space between the label and the description string is because
// 		PEND_JOB_* labels can go up to 2900
//
// FIXME FIXME FIXME add the 3/4 digit word

// 550
	{ PEND_JOB_NEW,                 "   ", "catgets 550: New job is waiting for scheduling" },                                            /* catgets 550 */
	{ PEND_JOB_START_TIME,          "   ", "catgets 551: The job has a specified start time" },                                           /* catgets 551 */
	{ PEND_JOB_DEPEND,              "   ", "catgets 552: Job dependency condition not satisfied" },                                       /* catgets 552 */
	{ PEND_JOB_DEP_INVALID,         "   ", "catgets 553: Dependency condition invalid or never satisfied" },                              /* catgets 553 */
	{ PEND_JOB_MIG,                 "   ", "catgets 554: Migrating job is waiting for rescheduling" },                                    /* catgets 554 */
	{ PEND_JOB_PRE_EXEC,            "   ", "catgets 555: The job's pre-exec command exited with non-zero status" },                       /* catgets 555 */
	{ PEND_JOB_NO_FILE,             "   ", "catgets 556: Unable to access job file" },                                                    /* catgets 556 */
	{ PEND_JOB_ENV,                 "   ", "catgets 557: Unable to set job's environment variables" },                                    /* catgets 557 */
	{ PEND_JOB_PATHS,               "   ", "catgets 558: Unable to determine job's home/working directories" },                           /* catgets 558 */
	{ PEND_JOB_OPEN_FILES,          "   ", "catgets 559: Unable to open job's I/O buffers" },                                             /* catgets 559 */

// 560
	{ PEND_JOB_EXEC_INIT,           "   ", "catgets 560: Job execution initialization failed" },                                          /* catgets 560 */
	{ PEND_JOB_RESTART_FILE,        "   ", "catgets 561: Unable to copy restarting job's checkpoint files" },                             /* catgets 561 */
	{ PEND_JOB_DELAY_SCHED,         "   ", "catgets 562: The schedule of the job is postponed for a while" },                             /* catgets 562 */
	{ PEND_JOB_SWITCH,              "   ", "catgets 563: Waiting for re-scheduling after switching queue" },                              /* catgets 563 */
	{ PEND_JOB_DEP_REJECT,          "   ", "catgets 564: Event is rejected by eeventd due to syntax error" },                             /* catgets 564 */

	{ PEND_JOB_NO_PASSWD,           "   ", "catgets 566: Failed to get user password" },                                                  /* catgets 566 */
	// FIXME FIXME FIXME FIXME catgets 567 did not originally exist!
	{ PEND_JOB_UKNOWN_567,          "   ", "catgets 567: apparently I am not supposed to exist" },                                        /* catgets 567 */
	{ PEND_JOB_MODIFY,              "   ", "catgets 568: Waiting for re-scheduling after parameters have been changed" },                 /* catgets 568 */

// 570
	{ PEND_JOB_REQUEUED,            "   ", "catgets 571: Requeue the job for the next run" },                                             /* catgets 571 */

// 650
	{ PEND_JOB_ARRAY_JLIMIT,        "   ", "catgets 655: The job array has reached its running element limit" },                          /* catgets 655 */
	{ PEND_CHKPNT_DIR,              "   ", "catgets 656: Checkpoint directory is invalid" },                                              /* catgets 656 */

// 580
	{ PEND_SYS_UNABLE,              "   ", "catgets 583: System is unable to schedule the job" },                                         /* catgets 583 */

	{ PEND_QUEUE_INACT,             "   ", "catgets 586: The queue is inactivated by the administrator" },                                /* catgets 586 */
	{ PEND_QUEUE_WINDOW,            "   ", "catgets 587: The queue is inactivated by its time windows" },                                 /* catgets 587 */
	{ PEND_QUEUE_JOB_LIMIT,         "   ", "catgets 588: The queue has reached its job slot limit" },                                     /* catgets 588 */
	{ PEND_QUEUE_PJOB_LIMIT,        "   ", "catgets 589: The queue has not enough job slots for the parallel job" },                      /* catgets 589 */

// 590
	{ PEND_QUEUE_USR_JLIMIT,        "   ", "catgets 590: User has reached the per-user job slot limit of the queue" },                    /* catgets 590 */
	{ PEND_QUEUE_USR_PJLIMIT,       "   ", "catgets 591: Not enough per-user job slots of the queue for the parallel job" },              /* catgets 591 */
	{ PEND_QUEUE_PRE_FAIL,          "   ", "catgets 592: The queue's pre-exec command exited with non-zero status" },                     /* catgets 592 */

	{ PEND_SYS_NOT_READY,           "   ", "catgets 596: System is not ready for scheduling after reconfiguration" },                     /* catgets 596 */
	{ PEND_SBD_JOB_REQUEUE,         "   ", "catgets 597: Requeued job is waiting for rescheduling" },                                     /* catgets 597 */
	{ PEND_JOB_SPREAD_TASK,         "   ", "catgets 598: Not enough hosts to meet the job's spanning requirement" },                      /* catgets 598 */
	{ PEND_QUEUE_SPREAD_TASK,       "   ", "catgets 599: Not enough hosts to meet the queue's spanning requirement" },                    /* catgets 599 */

// 600
	{ PEND_QUEUE_WINDOW_WILL_CLOSE, "   ", "catgets 600: Job will not finish before queue's run window is closed" },                      /* catgets 600 */
	{ PEND_USER_JOB_LIMIT,          "   ", "catgets 601: The user has reached his/her job slot limit" },                                  /* catgets 601 */
	{ PEND_UGRP_JOB_LIMIT,          "   ", "catgets 602: One of the user's groups has reached its job slot limit" },                      /* catgets 602 */
	{ PEND_USER_PJOB_LIMIT,         "   ", "catgets 603: The user has not enough job slots for the parallel job" },                       /* catgets 603 */
	{ PEND_UGRP_PJOB_LIMIT,         "   ", "catgets 604: One of user's groups has not enough job slots for the parallel job" },           /* catgets 604 */
	{ PEND_USER_RESUME,             "   ", "catgets 605: Waiting for scheduling after resumed by user" },                                 /* catgets 605 */
	{ PEND_USER_STOP,               "   ", "catgets 606: The job was suspended by the user while pending" },                              /* catgets 606 */
	{ PEND_ADMIN_STOP,              "   ", "catgets 607: The job was suspended by LSF admin or root while pending" },                     /* catgets 607 */
	{ PEND_NO_MAPPING,              "   ", "catgets 608: Unable to determine user account for execution" },                               /* catgets 608 */
	{ PEND_RMT_PERMISSION,          "   ", "catgets 609: The user has no permission to run the job on remote host/cluster" },             /* catgets 609 */

// 610
	{ PEND_HOST_RES_REQ,            "   ", "catgets 610: Job's resource requirements not satisfied" },                                    /* catgets 610 */
	{ PEND_HOST_NONEXCLUSIVE,       "   ", "catgets 611: Job's requirement for exclusive execution not satisfied" },                      /* catgets 611 */
	{ PEND_HOST_JOB_SSUSP,          "   ", "catgets 612: Higher or equal priority jobs suspended by host load" },                         /* catgets 612 */

	{ PEND_SBD_GETPID,              "   ", "catgets 614: Unable to get the PID of the restarting job" },                                  /* catgets 614 */
	{ PEND_SBD_LOCK,                "   ", "catgets 615: Unable to lock host for exclusively executing the job" },                        /* catgets 615 */
	{ PEND_SBD_ZOMBIE,              "   ", "catgets 616: Cleaning up zombie job" },                                                       /* catgets 616 */
	{ PEND_SBD_ROOT,                "   ", "catgets 617: Can't run jobs submitted by root" },                                             /* catgets 617 */
	{ PEND_HOST_WIN_WILL_CLOSE,     "   ", "catgets 618: Job will not finish on the host before queue's run window is closed" },          /* catgets 618 */
	{ PEND_HOST_MISS_DEADLINE,      "   ", "catgets 619: Job will not finish on the host before job's termination deadline" },            /* catgets 619 */

// 620
	{ PEND_HOST_DISABLED,           "   ", "catgets 620: Closed by LSF administrator" },                                                  /* catgets 620 */
	{ PEND_HOST_LOCKED,             "   ", "catgets 621: Host is locked by LSF administrator" },                                          /* catgets 621 */
	{ PEND_HOST_LESS_SLOTS,         "   ", "catgets 622: Not enough job slot(s)" },                                                       /* catgets 622 */
	{ PEND_HOST_WINDOW,             "   ", "catgets 623: Dispatch windows closed" },                                                      /* catgets 623 */
	{ PEND_HOST_JOB_LIMIT,          "   ", "catgets 624: Job slot limit reached" },                                                       /* catgets 624 */
	{ PEND_QUEUE_PROC_JLIMIT,       "   ", "catgets 625: Queue's per-CPU job slot limit reached" },                                       /* catgets 625 */
	{ PEND_QUEUE_HOST_JLIMIT,       "   ", "catgets 626: Queue's per-host job slot limit reached" },                                      /* catgets 626 */
	{ PEND_USER_PROC_JLIMIT,        "   ", "catgets 627: User's per-CPU job slot limit reached" },                                        /* catgets 627 */
	{ PEND_UGRP_PROC_JLIMIT,        "   ", "catgets 628: User group's per-CPU job slot limit reached" },                                  /* catgets 628 */
	{ PEND_HOST_USR_JLIMIT,         "   ", "catgets 629: Host's per-user job slot limit reached" },                                       /* catgets 629 */

// 630
	{ PEND_HOST_QUEUE_MEMB,         "   ", "catgets 630: Not usable to the queue" },                                                      /* catgets 630 */
	{ PEND_HOST_USR_SPEC,           "   ", "catgets 631: Not specified in job submission" },                                              /* catgets 631 */

	{ PEND_HOST_NO_USER,            "   ", "catgets 633: There is no such user account" },                                                /* catgets 633 */
	{ PEND_HOST_ACCPT_ONE,          "   ", "catgets 634: Just started a job recently" },                                                  /* catgets 634 */
	{ PEND_LOAD_UNAVAIL,            "   ", "catgets 635: Load information unavailable" },                                                 /* catgets 635 */
	{ PEND_HOST_NO_LIM,             "   ", "catgets 636: LIM is unreachable now" },                                                       /* catgets 636 */

	{ PEND_HOST_QUEUE_RESREQ,       "   ", "catgets 638: Queue's resource requirements not satisfied" },                                  /* catgets 638 */
	{ PEND_HOST_SCHED_TYPE,         "   ", "catgets 639: Not the same type as the submission host" },                                     /* catgets 639 */

// 640
	{ PEND_JOB_NO_SPAN,             "   ", "catgets 640: Not enough processors to meet the job's spanning requirement" },                 /* catgets 640 */
	{ PEND_QUEUE_NO_SPAN,           "   ", "catgets 641: Not enough processors to meet the queue's spanning requirement" },               /* catgets 641 */
	{ PEND_HOST_EXCLUSIVE,          "   ", "catgets 642: Running an exclusive job" },                                                     /* catgets 642 */

	{ PEND_HOST_QUEUE_RUSAGE,       "   ", "catgets 644: Queue's requirements for resource reservation not satisfied" },                  /* catgets 644 */
	{ PEND_HOST_JOB_RUSAGE,         "   ", "catgets 645: Job's requirements for resource reservation not satisfied" },                    /* catgets 645 */
	{ PEND_SBD_UNREACH,             "   ", "catgets 646: Unable to reach slave batch server" },                                           /* catgets 646 */
	{ PEND_SBD_JOB_QUOTA,           "   ", "catgets 647: Number of jobs exceeds quota" },                                                 /* catgets 647 */
	{ PEND_JOB_START_FAIL,          "   ", "catgets 648: Failed in talking to server to start the job" },                                 /* catgets 648 */
	{ PEND_JOB_START_UNKNWN,        "   ", "catgets 649: Failed in receiving the reply from server when starting the job" },              /* catgets 649 */

// 650
	{ PEND_SBD_NO_MEM,              "   ", "catgets 650: Unable to allocate memory to run job" },                                         /* catgets 650 */
	{ PEND_SBD_NO_PROCESS,          "   ", "catgets 651: Unable to fork process to run job" },                                            /* catgets 651 */
	{ PEND_SBD_SOCKETPAIR,          "   ", "catgets 652: Unable to communicate with job process" },                                       /* catgets 652 */
	{ PEND_SBD_JOB_ACCEPT,          "   ", "catgets 653: Slave batch server failed to accept job" },                                      /* catgets 653 */
	{ PEND_HOST_LOAD,               "   ", "catgets 654: Load threshold reached" },                                                       /* catgets 654 */

	// FIXME FIXME FIXME FIXME catgets 656 did not originally exist!
	{ PEND_UKNOWN_656,              "   ", "catgets 656: apparently I am not supposed to exist" },                                        /* catgets 656 */

// 660
	{ PEND_QUEUE_PROCLIMIT,         "   ", "catgets 662: Job no longer satisfies queue PROCLIMIT configuration" },                        /* catgets 662 */

	{ PEND_HOST_LOCKED_MASTER,      "   ", "catgets 664: Host is locked by master LIM" },                                                 /* catgets 664 */
	{ PEND_BAD_HOST,                "   ", "catgets 665: Bad host name, host group name or cluster name" },                               /* catgets 665 */
	{ PEND_QUEUE_HOST,              "   ", "catgets 666: Host or host group is not used by the queue" },                                  /* catgets 666 */
	{ PEND_FIRST_HOST_INELIGIBLE,   "   ", "catgets 667: The specified first exection host is not eligible for this job at this time" },  /* catgets 667 */

	{0, "    ", NULL}
};


//
// PEND_*
// 

// PEND_JOB_*

// #define PEND_JOB_REASON        0
// #define PEND_JOB_NEW           1
// #define PEND_JOB_START_TIME    2
// #define PEND_JOB_DEPEND        3
// #define PEND_JOB_DEP_INVALID   4
// #define PEND_JOB_MIG           5
// #define PEND_JOB_PRE_EXEC      6
// #define PEND_JOB_NO_FILE       7
// #define PEND_JOB_ENV           8
// #define PEND_JOB_PATHS         9
// #define PEND_JOB_OPEN_FILES    10
// #define PEND_JOB_EXEC_INIT     11
// #define PEND_JOB_RESTART_FILE  12
// #define PEND_JOB_DELAY_SCHED   13
// #define PEND_JOB_SWITCH        14
// #define PEND_JOB_DEP_REJECT    15
// #define PEND_JOB_NO_PASSWD     17
// #define PEND_JOB_UKNOWN_567    18 // NOTE: originally undefined
// #define PEND_JOB_MODIFY        19
// #define PEND_JOB_REQUEUED      23
// #define PEND_JOB_ARRAY_JLIMIT  38
// #define PEND_JOB_SPREAD_TASK   312
// #define PEND_JOB_NO_SPAN       1320
// #define PEND_JOB_START_FAIL    1603
// #define PEND_JOB_START_UNKNWN  1604



// PEND_CHKPNT_*
// #define PEND_CHKPNT_DIR        39


// PEND_SYS_*
// #define PEND_SYS_UNABLE        35
// #define PEND_SYS_NOT_READY     310


// PEND_QUEUE
// #define PEND_QUEUE_INACT             301
// #define PEND_QUEUE_WINDOW            302
// #define PEND_QUEUE_JOB_LIMIT         303
// #define PEND_QUEUE_USR_JLIMIT        304
// #define PEND_QUEUE_USR_PJLIMIT       305
// #define PEND_QUEUE_PRE_FAIL          306
// #define PEND_QUEUE_SPREAD_TASK       313
// #define PEND_QUEUE_PJOB_LIMIT        314
// #define PEND_QUEUE_WINDOW_WILL_CLOSE 315
// #define PEND_QUEUE_PROCLIMIT         316
// #define PEND_QUEUE_PROC_JLIMIT       1306
// #define PEND_QUEUE_HOST_JLIMIT       1307
// #define PEND_QUEUE_NO_SPAN           1321
// #define PEND_QUEUE_HOST              1326


// PEND_WORD_*
#define PEND_SBD_JOB_REQUEUE       311


#define PEND_USER_JOB_LIMIT    601
#define PEND_USER_PJOB_LIMIT   603
#define PEND_USER_RESUME       605
#define PEND_USER_STOP         607

#define PEND_UGRP_JOB_LIMIT    602

#define PEND_UGRP_PJOB_LIMIT   604


#define PEND_NO_MAPPING        608


#define PEND_RMT_PERMISSION    609


#define PEND_ADMIN_STOP        610


// PEND_HOST_*
#define PEND_HOST_RES_REQ        1001
#define PEND_HOST_NONEXCLUSIVE   1002
#define PEND_HOST_JOB_SSUSP      1003
#define PEND_HOST_WIN_WILL_CLOSE 1009
#define PEND_HOST_MISS_DEADLINE  1010
#define PEND_HOST_DISABLED       1301
#define PEND_HOST_LOCKED         1302
#define PEND_HOST_LESS_SLOTS     1303
#define PEND_HOST_WINDOW         1304
#define PEND_HOST_JOB_LIMIT      1305
#define PEND_HOST_USR_JLIMIT     1309
#define PEND_HOST_QUEUE_MEMB     1310
#define PEND_HOST_USR_SPEC       1311
#define PEND_HOST_PART_USER      1312
#define PEND_HOST_NO_USER        1313
#define PEND_HOST_ACCPT_ONE      1314
#define PEND_HOST_NO_LIM         1316
#define PEND_HOST_QUEUE_RESREQ   1318
#define PEND_HOST_SCHED_TYPE     1319
#define PEND_HOST_LOAD           2001
#define PEND_HOST_QUEUE_RUSAGE   2301
#define PEND_HOST_JOB_RUSAGE     2601
#define PEND_HOST_EXCLUSIVE      1322
#define PEND_HOST_LOCKED_MASTER  1327


#define PEND_SBD_GETPID          1005
#define PEND_SBD_LOCK            1006
#define PEND_SBD_ZOMBIE          1007
#define PEND_SBD_ROOT            1008

#define PEND_SBD_UNREACH         1601
#define PEND_SBD_JOB_QUOTA       1602
#define PEND_SBD_NO_MEM          1605
#define PEND_SBD_NO_PROCESS      1606
#define PEND_SBD_SOCKETPAIR      1607
#define PEND_SBD_JOB_ACCEPT      1608


#define PEND_FIRST_HOST_INELIGIBLE 1011
#define PEND_USER_PROC_JLIMIT      1308
#define PEND_LOAD_UNAVAIL          1315
#define PEND_UGRP_PROC_JLIMIT      1324

#define PEND_BAD_HOST              1325


#define PEND_MAX_REASONS 2900


//
// SUSP_*
//

#define SUSP_USER_REASON        0x00000000
#define SUSP_USER_RESUME        0x00000001
#define SUSP_USER_STOP          0x00000002

#define SUSP_QUEUE_REASON       0x00000004
#define SUSP_QUEUE_WINDOW       0x00000008

#define SUSP_HOST_LOCK          0x00000020
#define SUSP_LOAD_REASON        0x00000040
#define SUSP_QUEUE_STOP_COND      0x00000200
#define SUSP_QUEUE_RESUME_COND    0x00000400
#define SUSP_PG_IT              0x00000800
#define SUSP_REASON_RESET       0x00001000
#define SUSP_LOAD_UNAVAIL       0x00002000
#define SUSP_ADMIN_STOP         0x00004000
#define SUSP_RES_RESERVE        0x00008000
#define SUSP_MBD_LOCK           0x00010000
#define SUSP_RES_LIMIT          0x00020000
#define SUB_REASON_RUNLIMIT     0x00000001
#define SUB_REASON_DEADLINE     0x00000002
#define SUB_REASON_PROCESSLIMIT 0x00000004
#define SUB_REASON_CPULIMIT     0x00000008
#define SUB_REASON_MEMLIMIT     0x00000010
#define SUSP_SBD_STARTUP        0x00040000
#define SUSP_HOST_LOCK_MASTER   0x00080000
#define EXIT_NORMAL             0x00000000
#define EXIT_RESTART            0x00000001
#define EXIT_ZOMBIE             0x00000002
#define FINISH_PEND             0x00000004
#define EXIT_KILL_ZOMBIE        0x00000008
#define EXIT_ZOMBIE_JOB         0x00000010
#define EXIT_RERUN              0x00000020
#define EXIT_NO_MAPPING         0x00000040
#define EXIT_INIT_ENVIRON       0x00000100
#define EXIT_PRE_EXEC           0x00000200
#define EXIT_REQUEUE            0x00000400
#define EXIT_REMOVE             0x00000800
#define EXIT_VALUE_REQUEUE      0x00001000

#define LSB_MODE_BATCH    0x1

#define LSBE_NO_ERROR      00
#define LSBE_NO_JOB        01
#define LSBE_NOT_STARTED   02
#define LSBE_JOB_STARTED   03
#define LSBE_JOB_FINISH    04
#define LSBE_STOP_JOB      05
#define LSBE_DEPEND_SYNTAX  6
#define LSBE_EXCLUSIVE      7
#define LSBE_ROOT           8
#define LSBE_MIGRATION      9
#define LSBE_J_UNCHKPNTABLE 10
#define LSBE_NO_OUTPUT      11
#define LSBE_NO_JOBID       12
#define LSBE_ONLY_INTERACTIVE 13
#define LSBE_NO_INTERACTIVE   14

#define LSBE_NO_USER       15
#define LSBE_BAD_USER      16
#define LSBE_PERMISSION    17
#define LSBE_BAD_QUEUE     18
#define LSBE_QUEUE_NAME    19
#define LSBE_QUEUE_CLOSED  20
#define LSBE_QUEUE_WINDOW  21
#define LSBE_QUEUE_USE     22
#define LSBE_BAD_HOST      23
#define LSBE_PROC_NUM      24
#define LSBE_RESERVE1      25
#define LSBE_RESERVE2      26
#define LSBE_NO_GROUP      27
#define LSBE_BAD_GROUP     28
#define LSBE_QUEUE_HOST    29
#define LSBE_UJOB_LIMIT    30
#define LSBE_NO_HOST       31

#define LSBE_BAD_CHKLOG    32
#define LSBE_PJOB_LIMIT    33
#define LSBE_NOLSF_HOST    34

#define LSBE_BAD_ARG       35
#define LSBE_BAD_TIME      36
#define LSBE_START_TIME    37
#define LSBE_BAD_LIMIT     38
#define LSBE_OVER_LIMIT    39
#define LSBE_BAD_CMD       40
#define LSBE_BAD_SIGNAL    41
#define LSBE_BAD_JOB       42
#define LSBE_QJOB_LIMIT    43

#define LSBE_UNKNOWN_EVENT 44
#define LSBE_EVENT_FORMAT  45
#define LSBE_EOF           46

#define LSBE_MBATCHD       47
#define LSBE_SBATCHD       48
#define LSBE_LSBLIB        49
#define LSBE_LSLIB         50
#define LSBE_SYS_CALL      51
#define LSBE_NO_MEM        52
#define LSBE_SERVICE       53
#define LSBE_NO_ENV        54
#define LSBE_CHKPNT_CALL   55
#define LSBE_NO_FORK       56

#define LSBE_PROTOCOL      57
#define LSBE_XDR           58
#define LSBE_PORT          59
#define LSBE_TIME_OUT      60
#define LSBE_CONN_TIMEOUT  61
#define LSBE_CONN_REFUSED  62
#define LSBE_CONN_EXIST    63
#define LSBE_CONN_NONEXIST 64
#define LSBE_SBD_UNREACH   65
#define LSBE_OP_RETRY      66
#define LSBE_USER_JLIMIT   67

#define LSBE_JOB_MODIFY       68
#define LSBE_JOB_MODIFY_ONCE  69

#define LSBE_J_UNREPETITIVE   70
#define LSBE_BAD_CLUSTER      71

#define LSBE_JOB_MODIFY_USED  72

#define LSBE_HJOB_LIMIT       73

#define LSBE_NO_JOBMSG        74

#define LSBE_BAD_RESREQ       75

#define LSBE_NO_ENOUGH_HOST   76

#define LSBE_CONF_FATAL       77
#define LSBE_CONF_WARNING     78


#define LSBE_NO_RESOURCE        79
#define LSBE_BAD_RESOURCE       80
#define LSBE_INTERACTIVE_RERUN  81
#define LSBE_PTY_INFILE         82
#define LSBE_BAD_SUBMISSION_HOST  83
#define LSBE_LOCK_JOB           84
#define LSBE_UGROUP_MEMBER      85
#define LSBE_OVER_RUSAGE        86
#define LSBE_BAD_HOST_SPEC      87
#define LSBE_BAD_UGROUP         88
#define LSBE_ESUB_ABORT         89
#define LSBE_EXCEPT_ACTION      90
#define LSBE_JOB_DEP            91
#define LSBE_JGRP_NULL           92
#define LSBE_JGRP_BAD            93
#define LSBE_JOB_ARRAY           94
#define LSBE_JOB_SUSP            95
#define LSBE_JOB_FORW            96
#define LSBE_BAD_IDX             97
#define LSBE_BIG_IDX             98
#define LSBE_ARRAY_NULL          99
#define LSBE_JOB_EXIST           100
#define LSBE_JOB_ELEMENT         101
#define LSBE_BAD_JOBID           102
#define LSBE_MOD_JOB_NAME        103

#define LSBE_PREMATURE           104

#define LSBE_BAD_PROJECT_GROUP   105

#define LSBE_NO_HOST_GROUP       106
#define LSBE_NO_USER_GROUP       107
#define LSBE_INDEX_FORMAT        108

#define LSBE_SP_SRC_NOT_SEEN     109
#define LSBE_SP_FAILED_HOSTS_LIM 110
#define LSBE_SP_COPY_FAILED      111
#define LSBE_SP_FORK_FAILED      112
#define LSBE_SP_CHILD_DIES       113
#define LSBE_SP_CHILD_FAILED     114
#define LSBE_SP_FIND_HOST_FAILED 115
#define LSBE_SP_SPOOLDIR_FAILED  116
#define LSBE_SP_DELETE_FAILED    117

#define LSBE_BAD_USER_PRIORITY   118
#define LSBE_NO_JOB_PRIORITY     119
#define LSBE_JOB_REQUEUED        120

#define LSBE_MULTI_FIRST_HOST    121
#define LSBE_HG_FIRST_HOST       122
#define LSBE_HP_FIRST_HOST       123
#define LSBE_OTHERS_FIRST_HOST   124

#define LSBE_PROC_LESS           125
#define LSBE_MOD_MIX_OPTS        126
#define LSBE_MOD_CPULIMIT        127
#define LSBE_MOD_MEMLIMIT        128
#define LSBE_MOD_ERRFILE         129
#define LSBE_LOCKED_MASTER       130
#define LSBE_DEP_ARRAY_SIZE      131
#define LSBE_NUM_ERR             131


// Duplicates from <daemons/daemonout.h>
// #define PREPARE_FOR_OP          1024
// #define READY_FOR_OP            1023


#define SUB_JOB_NAME       0x01
#define SUB_QUEUE          0x02
#define SUB_HOST           0x04
#define SUB_IN_FILE        0x08
#define SUB_OUT_FILE       0x10
#define SUB_ERR_FILE       0x20
#define SUB_EXCLUSIVE      0x40
#define SUB_NOTIFY_END     0x80
#define SUB_NOTIFY_BEGIN   0x100
#define SUB_USER_GROUP     0x200
#define SUB_CHKPNT_PERIOD  0x400
#define SUB_CHKPNT_DIR     0x800
#define SUB_CHKPNTABLE     SUB_CHKPNT_DIR
#define SUB_RESTART_FORCE  0x1000
#define SUB_RESTART        0x2000
#define SUB_RERUNNABLE     0x4000
#define SUB_WINDOW_SIG     0x8000
#define SUB_HOST_SPEC      0x10000
#define SUB_DEPEND_COND    0x20000
#define SUB_RES_REQ        0x40000
#define SUB_OTHER_FILES    0x80000
#define SUB_PRE_EXEC       0x100000
#define SUB_LOGIN_SHELL    0x200000
#define SUB_MAIL_USER      0x400000
#define SUB_MODIFY         0x800000
#define SUB_MODIFY_ONCE    0x1000000
#define SUB_PROJECT_NAME   0x2000000
#define SUB_INTERACTIVE    0x4000000
#define SUB_PTY            0x8000000
#define SUB_PTY_SHELL      0x10000000

#define SUB2_HOLD          0x01
#define SUB2_MODIFY_CMD    0x02
#define SUB2_BSUB_BLOCK    0x04
#define SUB2_HOST_NT       0x08
#define SUB2_HOST_UX       0x10
#define SUB2_QUEUE_CHKPNT  0x20
#define SUB2_QUEUE_RERUNNABLE  0x40
#define SUB2_IN_FILE_SPOOL 0x80
#define SUB2_JOB_CMD_SPOOL 0x100
#define SUB2_JOB_PRIORITY  0x200
#define SUB2_USE_DEF_PROCLIMIT  0x400
#define SUB2_MODIFY_RUN_JOB 0x800
#define SUB2_MODIFY_PEND_JOB 0x1000

#define LOST_AND_FOUND  "lost_and_found"

#define DELETE_NUMBER     -2
#define DEL_NUMPRO        INFINIT_INT
#define DEFAULT_NUMPRO    INFINIT_INT -1

#define LSB_CHKPERIOD_NOCHNG -1

#define LSB_CHKPNT_KILL  0x1
#define LSB_CHKPNT_FORCE 0x2
#define LSB_CHKPNT_COPY  0x3
#define LSB_CHKPNT_MIG   0x4
#define LSB_CHKPNT_STOP  0x8
#define LSB_KILL_REQUEUE 0x10

#define ALL_USERS       "all"
#define ALL_JOB         0x0001
#define DONE_JOB        0x0002
#define PEND_JOB        0x0004
#define SUSP_JOB        0x0008
#define CUR_JOB         0x0010
#define LAST_JOB        0x0020
#define RUN_JOB         0x0040
#define JOBID_ONLY      0x0080
#define HOST_NAME       0x0100
#define NO_PEND_REASONS 0x0200
#define JGRP_ARRAY_INFO 0x1000
#define JOBID_ONLY_ALL  0x02000
#define ZOMBIE_JOB      0x04000

#define JGRP_NODE_JOB    1
#define JGRP_NODE_GROUP  2
#define JGRP_NODE_ARRAY  3

#define LSB_MAX_ARRAY_JOBID 0x0FFFFFFFF // FIXME FIXME FIXME FIXME FIXME this needs to be converted to appropriate CPU type to roll with
#define LSB_MAX_ARRAY_IDX   0x0FFFF  // FIXME FIXME FIXME FIXME FIXME this needs to be converted to appropriate CPU type to roll with
#define LSB_MAX_SEDJOB_RUNID    (0x0F)
#define LSB_JOBID(array_jobId, array_idx) (((unsigned long)array_idx << 32UL) | array_jobId) // FIXME FIXME FIXME FIXME FIXME this needs to be converted to appropriate CPU type to roll with
#define LSB_ARRAY_IDX(jobId) (((jobId) == -1UL) ? (0) : (unsigned long)(((unsigned long)jobId >> 32UL) & LSB_MAX_ARRAY_IDX)) // FIXME FIXME FIXME FIXME FIXME this needs to be converted to appropriate CPU type to roll with
#define LSB_ARRAY_JOBID(jobId) (((jobId) == -1UL) ? (-1) : (unsigned long)(jobId & LSB_MAX_ARRAY_JOBID)) // FIXME FIXME FIXME FIXME FIXME this needs to be unrolled.


#define JGRP_ACTIVE        1
#define JGRP_UNDEFINED     -1

#define JGRP_COUNT_NJOBS   0
#define JGRP_COUNT_PEND    1
#define JGRP_COUNT_NPSUSP  2
#define JGRP_COUNT_NRUN    3
#define JGRP_COUNT_NSSUSP  4
#define JGRP_COUNT_NUSUSP  5
#define JGRP_COUNT_NEXIT   6
#define JGRP_COUNT_NDONE   7

#define NUM_JGRP_COUNTERS 8

#define ALL_QUEUE       0x01
#define DFT_QUEUE       0x02
#define CHECK_HOST      0x80
#define CHECK_USER      0x100
#define SORT_HOST       0x200

#define LSB_SIG_NUM               23


#define ACT_NO              0
#define ACT_START           1
#define ACT_DONE            3
#define ACT_FAIL            4

#define H_ATTR_CHKPNTABLE  0x1


#define DEF_MAX_JOBID   999999
#define MAX_JOBID_LOW   999999
#define MAX_JOBID_HIGH 9999999

#define USER_GRP          0x1
#define HOST_GRP          0x2
#define GRP_RECURSIVE     0x8
#define GRP_ALL           0x10
#define GRP_SHARES        0x40

#define RUNJOB_OPT_NORMAL     0x01
#define RUNJOB_OPT_NOSTOP     0x02
#define RUNJOB_OPT_PENDONLY   0x04
#define RUNJOB_OPT_FROM_BEGIN 0x08

#define REQUEUE_DONE   0x1
#define REQUEUE_EXIT   0x2
#define REQUEUE_RUN    0x4

#define TO_TOP            1
#define TO_BOTTOM         2

#define QUEUE_OPEN        1
#define QUEUE_CLOSED      2
#define QUEUE_ACTIVATE    3
#define QUEUE_INACTIVATE  4

#define HOST_OPEN         1
#define HOST_CLOSE        2
#define HOST_REBOOT       3
#define HOST_SHUTDOWN     4

#define MBD_RESTART       0
#define MBD_RECONFIG      1
#define MBD_CKCONFIG      2


#define MAXFILENAMELEN          4096 // FIXME FIXME FIXME re-introduced from lsf.h, cuz gcc was complaining


#define LSF_JOBIDINDEX_FILENAME "lsb.events.index"       // FIXME FIXME FIXME is this approprate ? should it be named like that?
#define LSF_JOBIDINDEX_FILETAG "#LSF_JOBID_INDEX_FILE" 


#define LSB_MAX_SD_LENGTH 128


#define CONF_NO_CHECK       0x00
#define CONF_CHECK          0x01
#define CONF_EXPAND         0X02
#define CONF_RETURN_HOSTSPEC    0X04
#define CONF_NO_EXPAND      0X08



#define IS_PEND(s)  (((s) & JOB_STAT_PEND) || ((s) & JOB_STAT_PSUSP))

#define IS_START(s)  (((s) & JOB_STAT_RUN) || ((s) & JOB_STAT_SSUSP) \
|| ((s) & JOB_STAT_USUSP))

#define IS_FINISH(s) (((s) & JOB_STAT_DONE) || ((s) & JOB_STAT_EXIT))

#define IS_SUSP(s) (((s) & JOB_STAT_PSUSP) || ((s) & JOB_STAT_SSUSP) \
||  ((s) & JOB_STAT_USUSP))

#define IS_POST_DONE(s) ( ( (s) & JOB_STAT_PDONE) == JOB_STAT_PDONE )
#define IS_POST_ERR(s) ( ( (s) & JOB_STAT_PERR) == JOB_STAT_PERR )
#define IS_POST_FINISH(s) ( IS_POST_DONE(s) || IS_POST_ERR(s) )

#define PRINT_SHORT_NAMELIST  0x01
#define PRINT_LONG_NAMELIST   0x02
#define PRINT_MCPU_HOSTS      0x04


// #define XF_OP_SUB2EXEC         0x1
// #define XF_OP_EXEC2SUB         0x2
// #define XF_OP_SUB2EXEC_APPEND  0x4
// #define XF_OP_EXEC2SUB_APPEND  0x8
// #define XF_OP_URL_SOURCE       0x10
enum XF_OP_OPTIONS {
	XF_OP_SUB2EXEC = 0x1,
	XF_OP_EXEC2SUB = 0x2,
	XF_OP_SUB2EXEC_APPEND = 0x4,
	XF_OP_EXEC2SUB_APPEND = 0x8,
	XF_OP_URL_SOURCE = 0x10 
};

struct xFile {
	char *subFn;
	char *execFn;
	int options;
	const char padding[4];
};

struct submit {
	int options;
	int options2;
	unsigned int numAskedHosts;
	unsigned int numProcessors;
	int sigValue;
	unsigned int nxf;
	int delOptions;
	int delOptions2;
	unsigned int maxNumProcessors;
	int userPriority;
	int rLimits[LSF_RLIM_NLIMITS];
	const char padding[4];
	char *jobName;
	char *queue;
	char *resReq;
	char *hostSpec;
	char *dependCond;
	char *inFile;
	char *outFile;
	char *errFile;
	char *command;
	char *newCommand;
	char **askedHosts;
	char *chkpntDir;
	char *preExecCmd;
	char *mailUser;
	char *projectName;
	char *loginShell;
	time_t beginTime;
	time_t termTime;
	time_t chkpntPeriod;
	struct xFile *xf;
};

struct submitReply
{
	char *queue;
	char *badJobName;
	int badReqIndx;
	const char padding[4];
	unsigned long badJobId;
};

struct submig
{
	unsigned long jobId;
	int options;
	unsigned int numAskedHosts;
	char **askedHosts;
};


struct jobAttrInfoEnt
{
	unsigned short port;
	char hostname[MAXHOSTNAMELEN];
	const char padding[6];
	unsigned long jobId;
};

struct jobAttrSetLog
{
	unsigned long jobId;
	int idx;
	int uid;
	int port;
	const char padding[4];
	char *hostname;
};

struct jobInfoHead
{
	unsigned int numHosts;
	const char padding[4];
	unsigned long *jobIds;
	unsigned long numJobs;
	char **hostNames;
};

struct jobInfoEnt
{
	unsigned short port;
	const char padding[2];
	int status;
	int reasons;
	int subreasons;
	int umask;
	int exitStatus;
	int execUid;
	int jType;
	int jobPriority;
	unsigned int numReasons;
	unsigned int nIdx;
	unsigned int numExHosts;
	int *reasonTb;
	char *user;
	char *cwd;
	char *subHomeDir;
	char *fromHost;
	char **exHosts;
	char *execHome;
	char *execCwd;
	char *execUsername;
	char *parentGroup;
	char *jName;
	float *loadSched;
	float *loadStop;
	unsigned long jobPid;
	float cpuTime;
	float cpuFactor;
	time_t submitTime;
	time_t reserveTime;
	time_t startTime;
	time_t predictedStartTime;
	time_t endTime;
	time_t jRusageUpdateTime;
	int counter[NUM_JGRP_COUNTERS];
	struct submit submit;
	struct jRusage runRusage;
	unsigned long jobId;
};

struct userInfoEnt
{
    char *user;					// FIXME FIXME FIXME this oughta get larger
    float procJobLimit;
    unsigned int maxJobs;
    unsigned int numStartJobs;
    unsigned int numJobs;
    unsigned int numPEND;
    unsigned int numRUN;
    unsigned int numSSUSP;
    unsigned int numUSUSP;
    unsigned int numRESERVE;
    const char padding[4];
};


struct queueInfoEnt
{
	int nice;
	int priority;
	int userJobLimit;
	int qAttrib;
	int qStatus;
	int maxJobs;
	int numJobs;
	int numPEND;
	int numRUN;
	int numSSUSP;
	int numUSUSP;
	int mig;
	int schedDelay;
	int acceptIntvl;
	int procLimit;
	int hostJobLimit;
	int numRESERVE;
	int slotHoldTime;
	int chkpntPeriod;
	int minProcLimit;
	int defProcLimit;
	unsigned int nIdx;
	int rLimits[LSF_RLIM_NLIMITS];
	int sigMap[LSB_SIG_NUM];
	int defLimits[LSF_RLIM_NLIMITS];
	float procJobLimit;
	char *queue;
	char *description;
	char *userList;
	char *hostList;
	char *windows;
	char *hostSpec;
	char *windowsD;
	char *defaultHostSpec;
	char *admins;
	char *preCmd;
	char *postCmd;
	char *resumeCond;
	char *stopCond;
	char *jobStarter;
	char *suspendActCmd;
	char *resumeActCmd;
	char *terminateActCmd;
	char *prepostUsername;
	char *requeueEValues;
	char *resReq;
	float *loadSched;
	float *loadStop;
	char *chkpntDir;
};

struct hostInfoEnt
{
	int hStatus;
	int mig;
	int attr;
	int numRESERVE;
	int chkSig;
	unsigned int nIdx;
	int *busySched;
	int *busyStop;
	unsigned long userJobLimit;
	unsigned long maxJobs;
	unsigned long numJobs;
	unsigned long numRUN;
	unsigned long numSSUSP;
	unsigned long numUSUSP;
	float *load;
	float *loadSched;
	float *loadStop;
	char *host;
	char *windows;
	float *realLoad;
	double cpuFactor;
};

struct parameterInfo
{
	int jobAcceptInterval;
	int maxDispRetries;
	int maxSbdRetries;
	int cleanPeriod;
	int maxNumJobs;
	int pgSuspendIt;
	int retryIntvl;
	int rusageUpdateRate;
	int rusageUpdatePercent;
	int condCheckTime;
	int maxSbdConnections;
	int maxSchedStay;
	int freshPeriod;
	int maxJobArraySize;
	int jobTerminateInterval;
	int maxUserPriority;
	int jobPriorityValue;
	int jobPriorityTime;
	int sharedResourceUpdFactor;
	int scheRawLoad;
	int slotResourceReserve;
	int maxAcctArchiveNum;
	int acctArchiveInSize;
	int acctArchiveInDays;
	unsigned int disableUAcctMap;
	unsigned int jobRunTimes;
	unsigned int jobDepLastSub;
	unsigned int maxJobId;
	char *defaultQueues;
	char *defaultHostSpec;
	char *defaultProject;
	char *pjobSpoolDir;
	time_t mbatchdInterval;
	time_t sbatchdInterval;
	time_t preExecDelay;
};


struct loadInfoEnt
{
	const char *hostName;
	int status;
	const char padding[4];
	float *load;
};


struct groupInfoEnt
{
	const char *group;
	const char *memberList;
};

struct runJobRequest
{
	int options;
	unsigned int numHosts;
	unsigned long jobId;
	const char **hostname;
};

struct jobrequeue
{
	unsigned long jobId;
	int status;
	int options;
};

struct logSwitchLog
{
	int lastJobId;
};


struct jobNewLog
{
	int options;
	int options2;
	int sigValue;
	int restartPid;
	int idx;
	int userPriority;
	int umask;
	int niosPort;
	int rLimits[LSF_RLIM_NLIMITS];
	unsigned int nxf;
	unsigned int numProcessors;
	unsigned int numAskedHosts;
	unsigned int maxNumProcessors;
	const char padding1[4];
	time_t chkpntPeriod;
	time_t submitTime;
	time_t beginTime;
	time_t termTime;
	uid_t userId;
	const char padding2[4];
	unsigned long jobId;
	float hostFactor;
	const char padding3[4];
	char *queue;
	char *fromHost;
	char *cwd;
	char *chkpntDir;
	char *inFile;
	char *outFile;
	char *errFile;
	char *inFileSpool;
	char *commandSpool;
	char *jobSpoolDir;
	char *subHomeDir;
	char *jobFile;
	char *userName;
	char *hostSpec;
	char *jobName;
	char *command;
	char *preExecCmd;
	char *mailUser;
	char *projectName;
	char *resReq;
	char **askedHosts;
	char *dependCond;
	char *schedHostType;
	char *loginShell;
	struct xFile *xf;
};

struct jobModLog
{

	int options;
	int options2;
	int delOptions;
	int delOptions2;
	uid_t userId;
	int submitTime;
	int umask;
	int numProcessors;
	int beginTime;
	int termTime;
	int sigValue;
	int restartPid;
	int numAskedHosts;
	int chkpntPeriod;
	int nxf;
	int niosPort;
	int maxNumProcessors;
	int userPriority;

	char *jobIdStr;
	char *userName;
	char *jobName;
	char *queue;
	char *resReq;
	char *hostSpec;
	char *dependCond;
	char *subHomeDir;
	char *inFile;
	char *outFile;
	char *errFile;
	char *command;
	char *inFileSpool;
	char *commandSpool;
	char *chkpntDir;
	char *jobFile;
	char *fromHost;
	char *cwd;
	char *preExecCmd;
	char *mailUser;
	char *projectName;
	char *loginShell;
	char *schedHostType;

	char **askedHosts;

	int rLimits[LSF_RLIM_NLIMITS];
	const char padding[4];
	struct xFile *xf;
};


struct jobStartLog
{
	int jFlags;
	int idx;
	unsigned int numExHosts;
	int jStatus;
	pid_t jobPid;
	pid_t jobPGid;
	float hostFactor;
	const char padding1[4];
	unsigned long jobId;
	char *queuePreCmd;
	char *queuePostCmd;
	char **execHosts;
};

struct jobStartAcceptLog
{
	int idx;
	pid_t jobPid;
	pid_t jobPGid;
	const char padding1[4];
	unsigned long jobId;
};


struct jobExecuteLog
{
	int idx;
	uid_t execUid;
	pid_t jobPGid;
	pid_t jobPid;
	char *execHome;
	char *execCwd;
	char *execUsername;
	unsigned long jobId;
};


struct jobStatusLog
{
	int jStatus;
	int reason;
	int subreasons;
	int ru;
	int jFlags;
	int exitStatus;
	int idx;
	float cpuTime;
	const char padding[8];
	time_t endTime;
	unsigned long jobId;
	struct lsfRusage lsfRusage;
};


struct sbdJobStatusLog
{
	int jStatus;
	int reasons;
	int subreasons;
	int actPid;
	int actValue;
	int actFlags;
	int actStatus;
	int actReasons;
	int actSubReasons;
	int idx;
	time_t actPeriod;
	unsigned long jobId;
};

struct jobSwitchLog
{
	int idx;
	uid_t userId;
	char queue[MAX_LSB_NAME_LEN];
	char userName[MAX_LSB_NAME_LEN];
	unsigned long jobId;
};

struct jobMoveLog
{
	int position;
	int base;
	int idx;
	uid_t userId;
	char *userName; // [MAX_LSB_NAME_LEN];
	unsigned long jobId;
};

struct chkpntLog
{
	int chkperiod;
	int ok;
	int flags;
	int idx;
	pid_t pid;
	const char padding[4];
	time_t period;
	unsigned long jobId;
};

struct jobRequeueLog
{
	int idx;
	const char padding[4];
	unsigned long jobId;
};

struct jobCleanLog
{
	int idx;
	const char padding[4];
	unsigned long jobId;
};

struct sigactLog
{
	int jStatus;
	int reasons;
	int flags;
	int actStatus;
	int idx;
	pid_t pid;
	time_t period;
	// const char padding[6] ;
	char *signalSymbol;
	unsigned long jobId;
};

struct migLog
{
	int idx;
	int numAskedHosts;
	uid_t userId;
    const char padding[4];
    char *userName; // [MAX_LSB_NAME_LEN];
	unsigned long jobId;
    char **askedHosts;
};

struct signalLog
{
	int idx;
	int runCount;
	uid_t userId;
	char userName[MAX_LSB_NAME_LEN];
	char *signalSymbol;
	unsigned long jobId;
};
struct queueCtrlLog
{
	int opCode;
	uid_t userId;
	char *queue; // [MAX_LSB_NAME_LEN];
	char *userName; // [MAX_LSB_NAME_LEN];
};

struct newDebugLog
{
	int opCode;
	int level;
	int logclass;
	int turnOff;
	uid_t userId;
	const char padding[4];
	char *logFileName; //[MAXLSFNAMELEN];
};

struct hostCtrlLog
{
	int opCode;
	char host[MAXHOSTNAMELEN];
	uid_t userId;
	char userName[MAX_LSB_NAME_LEN];
};

struct mbdStartLog
{
	char master[MAXHOSTNAMELEN];
	char cluster[MAXLSFNAMELEN];
	int numHosts;
	int numQueues;
};

struct mbdDieLog
{
	char master[MAXHOSTNAMELEN];
	int numRemoveJobs;
	int exitCode;
};

struct unfulfillLog
{
	int notSwitched;
	int sig;
	int sig1;
	int sig1Flags;
	int notModified;
	int idx;
	unsigned long jobId;
	time_t chkPeriod;
};

struct jobFinishLog
{
	int options;
	int jStatus;
	int exitStatus;
	int idx;
	int maxRMem;
	int maxRSwap;
	uid_t userId;
	char queue       [MAX_LSB_NAME_LEN];
	char userName    [MAX_LSB_NAME_LEN];
	char fromHost    [MAXHOSTNAMELEN];
	char cwd         [MAXPATHLEN];
	char inFile      [MAXFILENAMELEN];
	char outFile     [MAXFILENAMELEN];
	char errFile     [MAXFILENAMELEN];
	char inFileSpool [MAXFILENAMELEN];
	char commandSpool[MAXFILENAMELEN];
	char jobFile     [MAXFILENAMELEN];
	char jobName     [MAXLINELEN];
	char command     [MAXLINELEN];
	const char padding[4];
	char *resReq;
	char *dependCond;
	char *preExecCmd;
	char *mailUser;
	char *projectName;
	char *loginShell;
	char **askedHosts;
	char **execHosts;
	unsigned long jobId;
	size_t numProcessors;
	size_t numAskedHosts;
	size_t numExHosts;
	size_t maxNumProcessors;
	float hostFactor;
	float cpuTime;
	time_t submitTime;
	time_t beginTime;
	time_t termTime;
	time_t startTime;
	time_t endTime;
	struct lsfRusage lsfRusage;
};

struct loadIndexLog
{
	int nIdx;
	const char padding[4];
	char **name;
};

struct jobMsgLog
{
	unsigned long jobId;
	int msgId;
	int type;
	int idx;
	uid_t usrId;
	char *src;
	char *dest;
	char *msg;
};

struct jobMsgAckLog
{
	int msgId;
	int type;
	int idx;
	uid_t usrId;
	char *src;
	char *dest;
	char *msg;
	unsigned long jobId;
};

struct jobForceRequestLog
{
	int idx;
	int options;
	int numExecHosts;
	uid_t userId;
	unsigned long jobId;
	char **execHosts;
	char userName[MAX_LSB_NAME_LEN];
	const char padding[4];
};

// from union eventLog @ include/lsb/log.h
struct newJobLog
{
	int flags;
	int nproc;
	int sigval;
    int chkperiod; // not sure if chkperiod should be an int
    int restartpid;
    int mailUserId;
    int umask;
    int numUsHosts;
    char *queue; // [MAXQUEUENAMELEN];
    char *resReq;
    char *fromHost;
    char *cwd;
    char *chkdir;
    char *inFile;
    char *outFile;
    char *errFile;
    char *jobFile;
    char *jobName;
    char *command;
    int limits[MAX_NRLIMITS]; // these subscribts got to go
	uid_t userId;
	const char padding[4];
    time_t submitTime;
    time_t startTime;
    time_t termTime;
	unsigned long jobId;
	unsigned long rjobId;
    char **usHosts;
};

struct startJobLog
{
	unsigned long jobId;
	int status;
	pid_t jobPid;
	int jobPGid;
	int numExHosts;
	char **execHosts;
};

struct newStatusLog
{
	int status;
	int reasons;
	unsigned long jobId;
	float cpuTime;
	const char padding[4];
	time_t doneTime;
	time_t delayTime;
};

struct qControlLog
{
	int opCode;
	char queue[MAXQUEUENAMELEN];
};

struct switchJobLog
{
	uid_t userId;
	const char padding[4];
	unsigned long jobId;
	char queue[MAXQUEUENAMELEN];
};

struct moveJobLog
{
	uid_t userId;
	int pos;
	int top;
	const char padding[4];
	unsigned long jobId;
};

struct paramsLog
{
	int nextId;
	int job_count;
};

struct finishJobLog
{
	unsigned long jobId;
	uid_t userId;
	int flags;
	int nproc;
	int status;
	int numUsHosts;
	char queue[MAXQUEUENAMELEN];
	char resReq[MAXLINELEN];
	char fromHost[MAXHOSTNAMELEN];
	char cwd[MAXPATHLEN];
	char inFile[MAXFILENAMELEN];
	char outFile[MAXFILENAMELEN];
	char errFile[MAXFILENAMELEN];
	char jobFile[MAXFILENAMELEN];
	char jobName[MAXJOBDESPLEN];
	char command[MAXJOBDESPLEN];
	float cpuTime;
	time_t submitTime;
	time_t startTime;
	time_t termTime;
	time_t dispatchTime;
	time_t doneTime;
	char **usHosts;
};

// end 


union eventLog
{
	struct jobNewLog jobNewLog;
	struct jobStartLog jobStartLog;
	struct jobStatusLog jobStatusLog;
	struct sbdJobStatusLog sbdJobStatusLog;
	struct jobSwitchLog jobSwitchLog;
	struct jobMoveLog jobMoveLog;
	struct queueCtrlLog queueCtrlLog;
	struct newDebugLog newDebugLog;
	struct hostCtrlLog hostCtrlLog;
	struct mbdStartLog mbdStartLog;
	struct mbdDieLog mbdDieLog;
	struct unfulfillLog unfulfillLog;
	struct jobFinishLog jobFinishLog;
	struct loadIndexLog loadIndexLog;
	struct migLog migLog;
	struct signalLog signalLog;
	struct jobExecuteLog jobExecuteLog;
	struct jobMsgLog jobMsgLog;
	struct jobMsgAckLog jobMsgAckLog;
	struct jobRequeueLog jobRequeueLog;
	struct chkpntLog chkpntLog;
	struct sigactLog sigactLog;
	struct jobStartAcceptLog jobStartAcceptLog;
	struct jobCleanLog jobCleanLog;
	struct jobForceRequestLog jobForceRequestLog;
	struct logSwitchLog logSwitchLog;
	struct jobModLog jobModLog;
	struct jobAttrSetLog jobAttrSetLog;
// from union eventLog @ include/lsb/log.h
	struct newJobLog newJobLog;
	struct startJobLog startJobLog;
	struct newStatusLog newStatusLog;
	struct qControlLog qControlLog;
	struct switchJobLog switchJobLog;
	struct moveJobLog moveJobLog;
	struct paramsLog paramsLog;
	struct finishJobLog finishJobLog;
// end
};

struct eventRec
{
	char version[MAX_VERSION_LEN];
	int type;
	time_t eventTime;
	union eventLog eventLog;
};

struct eventLogFile
{
	char eventDir[MAXFILENAMELEN];
	time_t beginTime;
	time_t endTime;
};

struct eventLogHandle
{
	FILE *fp;
	char openEventFile[MAXFILENAMELEN];
	int curOpenFile;
	int lastOpenFile;
};


struct jobIdIndexS
{
	int totalRows;
	int curRow;
	int totalJobIds;
	float version;
	time_t lastUpdate;
	time_t timeStamp;
	unsigned long minJobId;
	unsigned long maxJobId;
	FILE *fp;
	unsigned long *jobIds;
    char *fileName; //[MAXFILENAMELEN];
};

struct sortIntList
{
	int value;
	const char padding[4];
	struct sortIntList *forw;
	struct sortIntList *back;
};

struct lsbMsgHdr
{
	uid_t usrId;
	unsigned int   msgId;
	int   type;
	const char padding[4];
	char  *src;
	char  *dest;
	unsigned long jobId;
};

struct lsbMsg
{
	struct lsbMsgHdr *header;
	char *msg;
};
struct paramConf
{
	struct parameterInfo *param;
};

struct userConf
{
	unsigned int numUgroups;
	unsigned int numUsers;
	struct groupInfoEnt *ugroups;
	struct userInfoEnt *users;
};

struct hostConf
{
	unsigned int numHosts;
	unsigned int numHgroups;
	struct hostInfoEnt *hosts;
	struct groupInfoEnt *hgroups;
};

typedef struct lsbSharedResourceInstance // FIXME FIXME typedef has to go
{
	unsigned int nHosts;
	const char padding[4];
	char *totalValue;
	char *rsvValue;
	char **hostList;

} LSB_SHARED_RESOURCE_INST_T;

typedef struct lsbSharedResourceInfo // FIXME FIXME typedef has to go
{
	char *resourceName;
	unsigned int nInstances;
	const char padding[4];
	LSB_SHARED_RESOURCE_INST_T *instances;
} LSB_SHARED_RESOURCE_INFO_T;

struct queueConf
{
	unsigned int numQueues;
	const char padding[4];
	struct queueInfoEnt *queues;
};


int lsberrno;

int lsb_mbd_version;


// Ice Age Relic
// #if defined(__STDC__)
// #define P_(s) s
// #else
// #define P_(s) ()
// #endif


struct paramConf *lsb_readparam (struct lsConf *);
struct userConf *lsb_readuser (struct lsConf *, int, struct clusterConf *);
struct userConf *lsb_readuser_ex (struct lsConf *, int, struct clusterConf *, struct sharedConf *);
struct hostConf *lsb_readhost (struct lsConf *, struct lsInfo *, int, struct clusterConf *);
struct queueConf *lsb_readqueue (struct lsConf *, struct lsInfo *, int, struct sharedConf *);
void updateClusterConf (struct clusterConf *);


int lsb_init (char *appName);
int lsb_openjobinfo (unsigned long jobId, char *, char *, char *, char *, int);
struct jobInfoHead *lsb_openjobinfo_a (unsigned long, char *, char *, char *, char *, int);
struct jobInfoEnt *lsb_readjobinfo (int *);
unsigned long lsb_submit (struct submit *, struct submitReply *);


void lsb_closejobinfo (void);

int lsb_hostcontrol (char *, int);
struct queueInfoEnt *lsb_queueinfo (char **queues, unsigned int *numQueues, char *host, char *userName, int options);
int lsb_reconfig (int);
int lsb_signaljob (unsigned long jobId, int signal);
int lsb_msgjob (unsigned long jobId, char *message);
int lsb_chkpntjob (unsigned long jobId, time_t, int);
int lsb_deletejob (unsigned long jobId, int, int);
int lsb_forcekilljob (unsigned long jobId);
int lsb_requeuejob (struct jobrequeue *newQueue);
char *lsb_sysmsg (void);
void lsb_perror (char *);
char *lsb_sperror (char *);
char *lsb_peekjob (unsigned long jobId);

int lsb_mig (struct submig *, int *badHostIdx);

struct hostInfoEnt *lsb_hostinfo (char **, unsigned int *);
struct hostInfoEnt *lsb_hostinfo_ex (char **, unsigned int *, char *, int);
int lsb_movejob (unsigned long jobId, int *, int);
int lsb_switchjob (unsigned long jobId, char *queue);
int lsb_queuecontrol (char *, int);
struct userInfoEnt *lsb_userinfo (char **, unsigned int *);
struct groupInfoEnt *lsb_hostgrpinfo (char **groups, unsigned int *numGroups, int options);
struct groupInfoEnt *lsb_usergrpinfo (char **groups, unsigned int *numGroups, int options);
struct parameterInfo *lsb_parameterinfo (char **, unsigned int *, int);
unsigned long lsb_modify (struct submit *, struct submitReply *, unsigned long);
float *getCpuFactor (char *, int);
// char *lsb_suspreason (int, int, struct loadIndexLog *);
// char *lsb_pendreason (int, int *, struct jobInfoHead *, struct loadIndexLog *);

int lsb_puteventrec (FILE *, struct eventRec *);
// struct eventRec *lsb_geteventrec (FILE *log_fp, size_t *lineNum);
struct lsbSharedResourceInfo *lsb_sharedresourceinfo (char **, unsigned int *, char *, int);

int lsb_runjob (struct runJobRequest *);

char *lsb_jobid2str  ( unsigned long ) ;
char *lsb_jobidinstr  ( unsigned long ) ;
void jobId32To64 (unsigned long *, unsigned long jobId, unsigned int jobArrElemId ) ; // FIXME FIXME FIXME FIXME unsigned int to unsigned long or u_int64_t
void jobId64To32 (unsigned long, unsigned int *jobId, unsigned int *jobArrElemId ) ; // FIXME FIXME FIXME FIXME unsigned int to unsigned long or u_int64_t
int lsb_setjobattr (int, struct jobAttrInfoEnt *);

unsigned long lsb_rexecv (int, char **, char **, int *, int);
int lsb_catch (const char *, int (*)(void *));
void lsb_throw (const char *, void *);

struct sortIntList *initSortIntList (int);
int insertSortIntList (struct sortIntList *, int);
struct sortIntList *getNextSortIntList (struct sortIntList *, struct sortIntList *, int *);
void freeSortIntList (struct sortIntList *);
int getMinSortIntList (struct sortIntList *, int *);
int getMaxSortIntList (struct sortIntList *, int *);
int getTotalSortIntList (struct sortIntList *);

int updateJobIdIndexFile (char *, char *, int);

/*static int readJobNew (char *, struct jobNewLog *);
static int readJobMod (char *, struct jobModLog *);
static int readJobStart (char *, struct jobStartLog *);
static int readJobStartAccept (char *, struct jobStartAcceptLog *);
static int readJobExecute (char *, struct jobExecuteLog *);
static int readJobStatus (char *, struct jobStatusLog *);
static int readSbdJobStatus (char *, struct sbdJobStatusLog *);
static int readJobSwitch (char *, struct jobSwitchLog *);
static int readJobMove (char *, struct jobMoveLog *);
static int readJobFinish (char *, struct jobFinishLog *, time_t);
static int readQueueCtrl (char *, struct queueCtrlLog *);
static int readHostCtrl (char *, struct hostCtrlLog *);*/

// #undef P_

