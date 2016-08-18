/* $Id: daemons.h 397 2007-11-26 19:04:00Z mblack $
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

#include "daemons/daemonout.h"
#include "libint/intlibout.h"
#include "lib/lproto.h"
#include "lib/queue.h"
#include "lib/table.h"
#include "lsb/lsbatch.h"

#ifndef MIN
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define MIN_CPU_TIME 0.0001

#define  FATAL_ERR          -1
#define  WARNING_ERR        -2

#define MBATCHD_SERV    "mbatchd"
#define MBATCHD_QUERY   "mbatchd_query"
#define SBATCHD_SERV    "sbatchd"

#define MASK_INT_JOB_STAT          0x000FFFFF
#define MASK_STATUS(s) ((s) & MASK_INT_JOB_STAT)

#define JOB_STAT_CHKPNTED_ONCE     0x10000000
#define JOB_STAT_RESERVE           0x20000000
#define JOB_STAT_MIG               0x40000000
#define JOB_STAT_MODIFY_ONCE       0x01000000
#define JOB_STAT_ZOMBIE            0x02000000
#define JOB_STAT_PRE_EXEC          0x04000000
#define JOB_STAT_SIGNAL            0x08000000
#define JOB_STAT_KILL              0x00800000
#define JOB_STAT_RSRC_PREEMPT_WAIT 0x00400000
#define JOB_STAT_VOID              0x00100000

#define SET_STATE(s ,n) ((s) = ((s) & ~(MASK_INT_JOB_STAT)) | (n))


#define SBD_SET_STATE(jp, n) { (jp->jobSpecs.jStatus) = ((jp->jobSpecs.jStatus) & ~(MASK_INT_JOB_STAT)) | (n); sbdlog_newstatus(jp);}


#define IS_RUN_JOB_CMD(s) (((s) & JOB_STAT_RUN) && !((s) & JOB_STAT_PRE_EXEC))

#define MAX_FAIL             5
#define MAX_SEQ_NUM          INFINIT_INT

// FIXME FIXME these have to be set in an enum

#define DEF_MSLEEPTIME            2
#define DEF_SSLEEPTIME            2
#define DEF_RETRY_INTVL           2
#define DEF_PREEM_PERIOD          INFINIT_INT
#define DEF_PG_SUSP_IT            180
#define WARN_TIME                 600
#define DEF_RUSAGE_UPDATE_RATE    1
#define DEF_RUSAGE_UPDATE_PERCENT 10
#define DEF_JTERMINATE_INTERVAL   10
#define SLAVE_FATAL               101
#define SLAVE_MEM                 102
#define SLAVE_RESTART             103
#define SLAVE_SHUTDOWN            104

#define NOT_LOG                    INFINIT_INT

#define JOB_SAVE_OUTPUT   0x10000000
#define JOB_FORCE_KILL    0x20000000

#define JOB_URGENT        0x40000000
#define JOB_URGENT_NOSTOP 0x80000000

char errbuf[MAXLINELEN]; // FIXME FIXME FIXME FIXME convert to dynamic allocation

#define lsb_merr1(fmt,a1)        sprintf(errbuf,fmt,a1),lsb_merr(errbuf)
#define lsb_merr2(fmt,a1,a2)     sprintf(errbuf,fmt,a1,a2),lsb_merr(errbuf)
#define lsb_merr3(fmt,a1,a2,a3)  sprintf(errbuf,fmt,a1,a2,a3),lsb_merr(errbuf)
#define lsb_mperr1(fmt,a1)       sprintf(errbuf,fmt,a1), lsb_mperr(errbuf)
#define lsb_mperr2(fmt,a1,a2)    sprintf(errbuf,fmt,a1,a2), lsb_mperr(errbuf)
#define lsb_mperr3(fmt,a1,a2,a3) sprintf(errbuf,fmt,a1,a2,a3), lsb_mperr(errbuf)

typedef enum
{
    ERR_NO_ERROR        = 1,
    ERR_BAD_REQ         = 2,
    ERR_NO_JOB          = 3,
    ERR_NO_FILE         = 4,
    ERR_FORK_FAIL       = 5,
    ERR_NO_USER         = 6,
    ERR_LOCK_FAIL       = 7,
    ERR_NO_LIM          = 8,
    ERR_MEM             = 9,
    ERR_NULL            = 10,
    ERR_FAIL            = 11,
    ERR_BAD_REPLY       = 12,
    ERR_JOB_QUOTA       = 13,
    ERR_JOB_FINISH      = 14,
    ERR_CHKPNTING       = 15,
    ERR_ROOT_JOB        = 16,
    ERR_SYSACT_FAIL     = 17,
    ERR_SIG_RETRY       = 18,
    ERR_HOST_BOOT       = 20,
    ERR_PID_FAIL        = 21,
    ERR_SOCKETPAIR      = 22,
    ERR_UNREACH_SBD     = 23,
    ERR_JOB_RETURN      = 24,
    ERR_RESTARTING_FILE = 25,
    ERR_HANDLE          = 26
} sbdReplyType;

#define LOAD_REASONS (SUSP_LOAD_REASON | SUSP_QUE_STOP_COND | SUSP_QUE_RESUME_COND | SUSP_PG_IT | SUSP_LOAD_UNAVAIL | SUSP_HOST_LOCK | SUSP_HOST_LOCK_MASTER)

struct thresholds
{
    uint nThresholds;
    uint nIdx;
    float **loadStop;
    float **loadSched;
};

struct jobSpecs
{
    short nice;
    ushort niosPort;
    int jStatus;
    int options;
    int priority;
    int jAttrib;
    int sigValue;
    int chkSig;
    int actValue;
    int options2;
    int userPriority;
    pid_t actPid;
    pid_t userId;
    pid_t jobPid;
    pid_t jobPGid;
    pid_t restartPid;
    pid_t execUid;
    uint numEnv;
    uint reasons;
    uint subreasons;
    uint numToHosts;
    uint umask;
    uint nxf;
    uint maxNumProcessors;
    int sigMap     [LSB_SIG_NUM];
    char jobName   [MAXLINELEN];
    char userName  [MAX_LSB_NAME_LEN];
    char queue     [MAX_LSB_NAME_LEN];
    char fromHost  [MAXHOSTNAMELEN];
    char resReq    [MAXLINELEN];
    char windows   [MAXLINELEN];
    char subHomeDir[MAXFILENAMELEN];
    char command   [MAXLINELEN];
    char jobFile   [MAXFILENAMELEN];
    char inFile    [MAXFILENAMELEN];
    char outFile   [MAXFILENAMELEN];
    char errFile   [MAXFILENAMELEN];
    char cwd       [MAXFILENAMELEN];
    char preExecCmd[MAXLINELEN];
    time_t runTime;
    time_t startTime;
    time_t termTime;
    time_t submitTime;
    time_t chkPeriod;
    time_t migThresh;
    time_t lastSSuspTime;
    char *loginShell;
    char *schedHostType;
    char *execHosts;
    char chkpntDir      [MAXFILENAMELEN];
    char mailUser       [MAXLINELEN];
    char clusterName    [MAX_LSB_NAME_LEN];
    char projectName    [MAX_LSB_NAME_LEN];
    char preCmd         [MAXLINELEN];
    char postCmd        [MAXLINELEN];
    char prepostUsername[MAX_LSB_NAME_LEN];
    char execCwd        [MAXFILENAMELEN];
    char execHome       [MAXFILENAMELEN];
    char requeueEValues [MAXLINELEN];
    char resumeCond     [MAXLINELEN];
    char stopCond       [MAXLINELEN];
    char suspendActCmd  [MAXLINELEN];
    char resumeActCmd   [MAXLINELEN];
    char terminateActCmd[MAXLINELEN];
    char jobSpoolDir    [MAXPATHLEN];
    char inFileSpool    [MAXFILENAMELEN];
    char commandSpool   [MAXFILENAMELEN];
    char execUsername   [MAX_LSB_NAME_LEN];
    float lastCpuTime;
    char padding1[4];
    LS_LONG_INT jobId;
    char **env;
    char **toHosts;
    struct thresholds thresholds;
    struct lsfLimit   lsfLimits[LSF_RLIM_NLIMITS];
    struct lenData    eexec;
    struct xFile      *xf;
};

struct statusReq
{
    int newStatus;
    int reason;
    int subreasons;
    int seq;
    int sigValue;
    int actStatus;
    int exitStatus;
    uint numExecHosts;
    uint msgId;
    pid_t jobPid;
    pid_t jobPGid;
    pid_t actPid;
    uid_t execUid;
    char padding1[4];
    char *execHome;
    char *execCwd;
    char *execUsername;
    char *queuePreCmd;
    char *queuePostCmd;
    char **execHosts;
    LS_LONG_INT jobId;
    struct lsfRusage lsfRusage;
    struct jRusage runRusage;
    sbdReplyType sbdReply;
    char padding2[4];
};

struct chunkStatusReq
{
    uint numStatusReqs;
    char padding1[4];
    struct statusReq **statusReqs;
};


struct sbdPackage
{
    int retryIntvl;
    int preemPeriod;
    int pgSuspIdleT;
    uint maxJobs;
    uint numJobs;
    uint uJobLimit;
    uint rusageUpdateRate;
    uint rusageUpdatePercent;
    uint jobTerminateInterval;
    uint nAdmins;
    pid_t managerId;
    pid_t mbdPid;
    time_t sbdSleepTime;
    char lsbManager[MAX_LSB_NAME_LEN];
    char padding1[4];
    char **admins;
    struct jobSpecs *jobs;
};

struct jobSig
{
  int sigValue;
  int actFlags;
  int reasons;
  int subReasons;
  char *actCmd;
  time_t chkPeriod;
  LS_LONG_INT jobId;
  LS_LONG_INT newJobId;
};

struct jobReply
{
    int jStatus;
    int reasons;
    pid_t jobPid;
    pid_t jobPGid;
    pid_t actPid;
    int actValue;
    int actStatus;
    char padding1[4];
    LS_LONG_INT jobId;
};


enum _bufstat
{
    MSG_STAT_QUEUED, 
    MSG_STAT_SENT, 
    MSG_STAT_RCVD
};

typedef struct proto proto_t;
struct proto
{
  int usrId;
  int msgId;
  int type;
  int instance;
  LS_LONG_INT jobId;
  int (*sndfnc) (int, char *, int);
  int (*rcvfnc) (int, char *, int);
};

struct bucket
{
  struct bucket *forw;
  struct bucket *back;
  struct Buffer *storage;
  enum _bufstat bufstat;
  proto_t proto;
  XDR xdrs;
};

#define NEW_BUCKET(BUCKET,chanBuf) \
{ \
    BUCKET = (struct bucket *)malloc(sizeof(struct bucket)); \
    if (BUCKET) { \
        BUCKET->proto.usrId = -1; \
        BUCKET->proto.jobId = -1; \
        BUCKET->proto.instance = -1; \
        BUCKET->proto.sndfnc = b_write_fix; \
        BUCKET->proto.rcvfnc = b_read_fix; \
        BUCKET->xdrs.x_ops = 0; \
        BUCKET->storage = chanBuf; \
    } else { \
        lsberrno = LSBE_NO_MEM; \
    } \
}

#define FREE_BUCKET(BUCKET) \
{ \
    if (BUCKET->xdrs.x_ops) \
        xdr_destroy(&BUCKET->xdrs); \
    free(BUCKET); \
}

#define QUEUE_INIT(pred) \
{ \
    struct bucket *_bucket_; \
    NEW_BUCKET(_bucket_, NULL); \
    pred = _bucket_; \
    pred->forw = pred->back = _bucket_; \
}

#define QUEUE_DESTROY(pred) \
{ \
    struct bucket *bp, *bpnxt; \
    for (bp = pred->forw; bp != pred; bp = bpnxt) { \
        bpnxt = bp->forw; \
        chanFreeStashedBuf_(bp->storage); \
        FREE_BUCKET(bp); \
    } \
    FREE_BUCKET(pred); \
}

#define QUEUE_APPEND(entry, pred) \
    entry->back = pred->back; \
    entry->forw = pred; \
    pred->back->forw = entry; \
    pred->back  = entry;

#define QUEUE_REMOVE(entry) \
   entry->back->forw = entry->forw; \
   entry->forw->back = entry->back;

#define LSBMSG_DECL(hdr, jm) \
    char _src_[LSB_MAX_SD_LENGTH]; \
    char _dest_[LSB_MAX_SD_LENGTH]; \
    char _strBuf_[MSGSIZE]; \
    struct lsbMsgHdr hdr; \
    struct lsbMsg    jm;

#define LSBMSG_INIT(hdr, jm) \
    hdr.src = _src_; \
    hdr.dest = _dest_; \
    jm.header = &hdr; \
    jm.msg = _strBuf_;

#define LSBMSG_FINALIZE(xdrs, jm) \
    if (xdrs->x_op == XDR_DECODE && jm.msg) free(jm.msg);

#define LSBMSG_CACHE_BUFFER(bucket, jm) \
    bucket->proto.usrId = jm.header->usrId; \
    bucket->proto.jobId = jm.header->jobId; \
    bucket->proto.msgId = jm.header->msgId; \
    bucket->proto.type  = jm.header->type;

int errno;
char **environ;

u_long nextJobId;
u_long numRemoveJobs;
u_long maxJobId;


char *lsbManager;
char *lsbSys;

// FIXME FIXME FIXME FIXME bunc of loose variables? 

uid_t batchId;
int debug;
int lsb_CheckMode;
int lsb_CheckError;
ushort mbd_port;
ushort sbd_port;
int batchSock;
char masterme;
char *masterHost;
char *clusterName;
time_t now;
int retryIntvl;
int sbdSleepTime;
int preemPeriod;
int pgSuspIdleT;
char *env_dir;
struct lsInfo *allLsInfo;
struct tclLsInfo *tclLsInfo;
int rusageUpdateRate;
int rusageUpdatePrecent;
int jobTerminateInterval;
int lsf_crossUnixNT;


#define DEFAULT_MAILTO     "^U"
#define DEFAULT_CRDIR     "/bin"
#define DEFAULT_MAILPROG  "/usr/lib/sendmail"  // FIXME FIXME FIXME FIXME FIXME replace with variable in config.ac

FILE *smail (char *to, char *tohost);
uid_t chuser (uid_t uid);
int get_ports (void);
void die (int sig);
void *my_malloc (size_t size, const char *);
void *my_calloc (size_t nelem, size_t esize, const char *caller);
void lsb_merr (char *s);
void merr_user (char *user, char *host, char *msg, char *type);
int portok (struct sockaddr_in *from);
char *safeSave (char *);
void lsb_mperr (char *msg);
void mclose (FILE * file);
void relife (void);
int getElock (void);
int touchElock (void);
void getElogLock (void);
void touchElogLock (void);
void releaseElogLock (void);
struct listEntry *tmpListHeader (struct listEntry *listHeader);
struct tclLsInfo *getTclLsInfo (void);
struct resVal *checkThresholdCond (char *);
uint *getResMaps (uint nRes, char **resource);
int checkResumeByLoad (LS_LONG_INT jobId, int num, struct thresholds thresholds, struct hostLoad *loads, uint *reason, uint *subreasons, int jAttrib, struct resVal *resumeCondVal, struct tclHostData *tclHostData);
void closeExceptFD (int);
void freeLsfHostInfo (struct hostInfo *, int);
void copyLsfHostInfo (struct hostInfo *, struct hostInfo *);
void freeTclHostData (struct tclHostData *);
void lsbFreeResVal (struct resVal **);


int initTcl (struct tclLsInfo *);


int fileExist (char *file, uid_t uid, struct hostent *);
void freeWeek (windows_t **);
void errorBack (uint chan, ushort replyCode, struct sockaddr_in *from);

int init_ServSock (u_short port);
int server_reply (int, char *, int);
int rcvJobFile (int, struct lenData *);
int do_readyOp (XDR * xdrs, int, struct sockaddr_in *,
               struct LSFHeader *);

#define FORK_REMOVE_SPOOL_FILE  (0x1)
#define CALL_RES_IF_NEEDED      (0x2)
void childRemoveSpoolFile (const char *, int, const struct passwd *);

int xdr_statusReq (XDR *, struct statusReq *, struct LSFHeader *);
int xdr_sbdPackage (XDR *, struct sbdPackage *, struct LSFHeader *);
int xdr_jobSpecs (XDR * xdrs, struct jobSpecs *jobSpecs, struct LSFHeader *);
int xdr_sbdPackage1 (XDR * xdrs, struct sbdPackage *, struct LSFHeader *);
int xdr_jobReply (XDR * xdrs, struct jobReply *jobReply, struct LSFHeader *);
int xdr_jobSig (XDR * xdrs, struct jobSig *jobSig, struct LSFHeader *);
int xdr_chunkStatusReq (XDR *, struct chunkStatusReq *, struct LSFHeader *);

float normalizeRq_ (float rawql, float cpuFactor, int nprocs);

void daemon_doinit (void);

void scaleByFactor (uint *h32, uint *l32, float cpuFactor);
int execNqsi (u_long, int, int, int *, char *, int, char *);
void doDaemonHang (char *);
