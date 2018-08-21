/* $Id: sbd.h 397 2007-11-26 19:04:00Z mblack $
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


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "daemons/daemonout.h"
#include "daemons/daemons.h"
#include "lib/osal.h"
#include "lib/rf.h"
#include "lsb/lsb.h"
#include "lib/channel.h"

#define NULLFILE "/dev/null"
#define JOBFILEEXT ""

#define JOBFILE_CREATED -1

enum
{
  JSUPER_STAT_SUSP
};

struct jobCard
{
  struct jobCard *forw;
  struct jobCard *back;
  gid_t execGid;
  char execUsername[MAX_LSB_NAME_LEN];
  int notReported;
  time_t windEdge;
  windows_t *week[8];

  char active;
  char timeExpire;
  char missing;
  char mbdRestarted;
  time_t windWarnTime;
  int runTime;
  int w_status;
  /* pre-exec report flag */
  float cpuTime;
  time_t lastChkpntTime;
  int migCnt;
  struct jobSpecs jobSpecs;
  struct lsfRusage lsfRusage;
  int needReportRU;
  int cleanupPid;
  int collectedChild;
  int execJobFlag;
#define JOB_EXEC_QPRE_OK          0x1
#define JOB_EXEC_QPRE_KNOWN       0x2
#define JOB_EXEC_STARTED          0x4

  char *stdinFile;

  time_t lastStatusMbdTime;

  struct jRusage runRusage;
  struct jRusage mbdRusage;

  struct jRusage maxRusage;

  int delieveredMsgId;
  struct clientNode *client;
  int regOpFlag;
  bool_t newPam;
  struct jRusage wrkRusage;
  int jSupStatus;
#define REG_CHKPNT 0x1
#define REG_SIGNAL 0x2
#define REG_RUSAGE 0x4
#define REG_NICE   0x8
  struct resVal *resumeCondVal;
  struct resVal *stopCondVal;
  int actFlags;
  int actStatus;
  int actReasons;
  int actSubReasons;
  int exitPid;
  int jobDone;
  time_t lastCheck;
  char *actCmd;
  char *exitFile;
  char *clusterName;
  int servSocket;
  int crossPlatforms;
  char *spooledExec;
  char postJobStarted;
  char userJobSucc;
};

typedef enum
{
  NO_SIGLOG,
  SIGLOG
} logType;


#define JOB_RUNNING(jp) (((jp)->jobSpecs.jStatus & JOB_STAT_RUN) && \
                          (JOB_STARTED(jp)))

#define FILE_ERRNO(errno) \
	(errno == ENOENT || errno == EPERM || errno == EACCES || \
	 errno == ELOOP || errno == ENAMETOOLONG || errno == ENOTDIR || \
	 errno == EBADF || errno == EFAULT || \
	 errno == EEXIST || errno == ENFILE || errno == EINVAL || \
	 errno == EISDIR || errno == ENOSPC || errno == ENXIO || \
	 errno == EROFS || errno == ETXTBSY)


struct clientNode
{
  struct clientNode *forw;
  struct clientNode *back;
  int chanfd;
  struct sockaddr_in from;
  int jobType;
  unsigned long jobId;
  struct jobCard *jp;
};

struct jobSetup
{
  unsigned long jobId;
  int jStatus;
  float cpuTime;
  int w_status;
  struct lsfRusage lsfRusage;
  int reason;
  int jobPid;
  int jobPGid;
  int execGid;
  int execUid;
  char execUsername[MAX_LSB_NAME_LEN];
  char execHome[MAXFILENAMELEN];
  char execCwd[MAXFILENAMELEN];
  int execJobFlag;

#define LSB_PRE_ABORT 99
};

struct jobSyslog
{
  int logLevel;
  char msg[MAXLINELEN];
};

#define UID_MAPPED(jp) (strcmp((jp)->jobSpecs.userName, (jp)->execUsername))
#define PURE_INTERACTIVE(jobSpecsPtr) \
     (((jobSpecsPtr)->options & SUB_INTERACTIVE) && \
       !((jobSpecsPtr)->options & (SUB_IN_FILE | SUB_OUT_FILE | SUB_ERR_FILE)) \
       && !( (jobSpecsPtr)->preCmd && (jobSpecsPtr)->preCmd[0] != '\0' \
	     && (jobSpecsPtr)->postCmd && (jobSpecsPtr)->postCmd[0] != '\0') )



#define OTHER_REASONS  (SUSP_ADMIN_STOP | SUSP_RES_RESERVE)


#define SUSP_USER(jp)    ((jp)->jobSpecs.reasons & SUSP_USER_STOP)

#define SUSP_WINDOW(jp)  ((jp)->jobSpecs.reasons & SUSP_QUEUE_WINDOW)

#define SUSP_LOAD(jp)    ((jp)->jobSpecs.reasons & (LOAD_REASONS))

#define SUSP_OTHER(jp)   ((jp)->jobSpecs.reasons & (OTHER_REASONS))

#define JOB_STARTED(jp)  (((jp)->execJobFlag & JOB_EXEC_STARTED) || \
    (!daemonParams[LSB_BSUBI_OLD].paramValue && \
     PURE_INTERACTIVE(&(jp)->jobSpecs)))


int mbdPid;
int jobcnt;
int maxJobs;
int uJobLimit;
int pgSuspIdleT;
int listenNqs;
windows_t *host_week[8];
time_t host_windEdge;
char host_active;
char master_unknown;
char myStatus;

#define NO_LIM		0x0001

char need_checkfinish;
int failcnt;
float myFactor;
int pgSuspIdleT;
char *env_dir;
time_t bootTime;

struct listEntry *jobQue;
struct jobCard *jobQueHead;
struct jobTable *joblist[];
struct clientNode *clientList;
struct bucket *jmQueue;

int statusChan;

time_t bootTime = 0;

char errbuf[MAXLINELEN];

char *lsbManager = NULL;
int debug = 0;
int lsb_CheckMode = 0;
int lsb_CheckError = 0;
uid_t batchId = 0;
int managerId = 0;
char masterme = FALSE;
char master_unknown = TRUE;
char myStatus = 0;
char need_checkfinish = FALSE;
int failcnt = 0;
unsigned short sbd_port;
unsigned short mbd_port;
int sbdSleepTime = DEF_SSLEEPTIME;
int msleeptime = DEF_MSLEEPTIME;
int retryIntvl = DEF_RETRY_INTVL;
int preemPeriod = DEF_PREEM_PERIOD;
int pgSuspIdleT = DEF_PG_SUSP_IT;
int rusageUpdateRate = DEF_RUSAGE_UPDATE_RATE;
int rusageUpdatePercent = DEF_RUSAGE_UPDATE_PERCENT;

int jobTerminateInterval = DEF_JTERMINATE_INTERVAL;
char psbdJobSpoolDir[MAXPATHLEN];

time_t now;
int connTimeout;
int readTimeout;

int batchSock;

int mbdPid = 0;
short mbdExitVal = MASTER_NULL;
int mbdExitCnt = 0;
int jobcnt = 0;
int maxJobs = 0;
int urgentJob = 0;
int uJobLimit = 0;
float myFactor = 0.0;

int statusChan = -1;

windows_t *host_week[8];
time_t host_windEdge = 0;
char host_active = TRUE;

char delay_check = FALSE;
char *env_dir = NULL;

char *masterHost;
char *clusterName;
struct jobCard *jobQueHead;
struct lsInfo *allLsInfo;
struct clientNode *clientList;
struct bucket *jmQueue;
struct tclLsInfo *tclLsInfo;

#define CLEAN_TIME (12*60*60)

#define CHECK_MBD_TIME 30
char mbdStartedBySbd = FALSE;

int getpwnamRetry = 1;
int lsbMemEnforce = FALSE;
int lsbJobCpuLimit = -1;
int lsbJobMemLimit = -1;
int lsbStdoutDirect = FALSE;

int sbdLogMask;

void start_master (void);
void shutDownClient (struct clientNode *);

void do_newjob (XDR * xdrs, int s, struct LSFHeader *);
void do_switchjob (XDR * xdrs, int s, struct LSFHeader *);
void do_sigjob (XDR * xdrs, int s, struct LSFHeader *);
void do_probe (XDR * xdrs, int s, struct LSFHeader *);
void do_reboot (XDR * xdrs, int s, struct LSFHeader *);
void do_shutdown (XDR * xdrs, int s, struct LSFHeader *);
void do_jobSetup (XDR * xdrs, int s, struct LSFHeader *);
void do_jobSyslog (XDR * xdrs, int s, struct LSFHeader *);
void do_jobMsg (struct bucket *, XDR *, int s, struct LSFHeader *);
void do_rmConn (XDR *, int, struct LSFHeader *, struct clientNode *);
void do_lsbMsg (XDR *, int s, struct LSFHeader *);
void deliverMsg (struct bucket *);

void getJobsState (struct sbdPackage *sbdPackage);
int status_job (mbdReqType, struct jobCard *, int, sbdReplyType);
void sbdSyslog (int, char *);
void jobSetupStatus (int, int, struct jobCard *);
int msgSupervisor (struct lsbMsg *, struct clientNode *);
#ifdef INTER_DAEMON_AUTH
int getSbdAuth (struct lsfAuth *);
#endif
int sendUnreportedStatus (struct chunkStatusReq *chunkStatusReq);

struct jobCard *addJob (struct jobSpecs *, int);
void refreshJob (struct jobSpecs *);
sbdReplyType job_exec (struct jobCard *jobCardPtr, int);
void status_report (void);
int job_finish (struct jobCard *, int);
void setRunLimit (struct jobCard *, int);
void inJobLink (struct jobCard *);
void deallocJobCard (struct jobCard *);
void freeToHostsEtc (struct jobSpecs *);
void saveSpecs (struct jobSpecs *, struct jobSpecs *);
void renewJobStat (struct jobCard *jp);
void jobGone (struct jobCard *jp);
void preJobStatus (struct jobCard *jp, int sfd);
int setIds (struct jobCard *jobCardPtr);
void preExecFinish (struct jobCard *);
void jobGone (struct jobCard *jp);
int setJobEnv (struct jobCard *);
int runQPost (struct jobCard *);
int acctMapOk (struct jobCard *);
int acctMapTo (struct jobCard *);
int postJobSetup (struct jobCard *);
void runUPre (struct jobCard *);
int reniceJob (struct jobCard *);
int updateRUsageFromSuper (struct jobCard *jp, char *mbuf);
void sbdChild (char *, char *);
int initJobCard (struct jobCard *jp, struct jobSpecs *jobSpecs, int *);
void freeThresholds (struct thresholds *);
void saveThresholds (struct jobSpecs *, struct thresholds *);

void job_checking (void);
int job_resume (struct jobCard *);
void checkFinish (void);
void setInclRs (struct jobSpecs *jobSpecs, int reason);
void resetInclRs (struct jobSpecs *jobSpecs, int reason);
int testInclRs (struct jobSpecs *jobSpecs, int reason);


int chkpntJob (struct jobCard *, int);
void execRestart (struct jobCard *jobCardPtr, struct hostent *hp);


int rmJobBufFiles (struct jobCard *);
void writePreJobFail (struct jobCard *jp);

int appendJobFile (struct jobCard *jobCard, char *header, struct hostent *hp, char *errMsg);
int initPaths (struct jobCard *jp, struct hostent *fromHp, struct lenData *jf);
int rcpFile (struct jobSpecs *, struct xFile *, char *, int, char *);
void delCredFiles (void);
void jobFileExitStatus (struct jobCard *jobCard);
int isAbsolutePathSub (struct jobCard *, const char *);
int isAbsolutePathExec (const char *);

void milliSleep (int msec);
char window_ok (struct jobCard *jobPtr);
void child_handler_sbatchd (int);
void shout_err (struct jobCard *jobPtr, char *);
int fcp (char *, char *, struct hostent *);
int rmDir (char *);
void closeBatchSocket (void);
void getManagerId (struct sbdPackage *);

bool_t xdr_jobSetup (XDR *, struct jobSetup *, struct LSFHeader *);
bool_t xdr_jobSyslog (XDR *, struct jobSyslog *, struct LSFHeader *);
bool_t xdr_jobCard (XDR *, struct jobCard *, struct LSFHeader *);
int sizeofJobCard (struct jobCard *);

int jobSigStart (struct jobCard *jp, int sigValue, int actFlags, int actPeriod, logType logFlag);
int jobact (struct jobCard *, int, char *, int, int);
int jobsig (struct jobCard *jobTable, int sig, int forkKill);
int sbdread_jobstatus (struct jobCard *jp);
int sbdCheckUnreportedStatus ();
void exeActCmd (struct jobCard *jp, char *actCmd, char *exitFile);
void exeChkpnt (struct jobCard *jp, int chkFlags, char *exitFile);

// #define NL_SETN    11
void sinit (void);
void init_sstate (void);
void processMsg (struct clientNode *);
void clientIO (struct Masks *);
void houseKeeping (void);
int authCmdRequest (struct clientNode *client, XDR * xdrs, struct LSFHeader *reqHdr);
int isLSFAdmin_sbatchd (struct lsfAuth *auth); // FIXME FIXME FIXME FIXME there is a second isLSFAdmin in daemons/libresd/resd.h. Consolidate, if possible

#ifdef INTER_DAEMON_AUTH
int authMbdRequest (struct clientNode *, XDR *, struct LSFHeader *);
#endif
int get_new_master (struct sockaddr_in *from);

