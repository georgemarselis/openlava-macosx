/*
 * Copyright (C) 2011 David Bigagli
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
 
#include "daemonout.h"
#include "daemons.h"
#include "intlib/bitset.h"
#include "jgrp.h"
#include "lsbatch.h"
#include "mbd/proxy.h"

#define DEF_CLEAN_PERIOD     3600
#define DEF_MAX_RETRY        5
#define DEF_MAXSBD_FAIL      3
#define DEF_ACCEPT_INTVL     1
#define DEF_PRIO             1
#define DEF_NICE             0
#define DEF_SCHED_DELAY      10
#define DEF_QUEUE_SCHED_DELAY    0
#define MAX_INTERNAL_JOBID   299999999999999LL // FIXME FIXME FIXME FIXME what's that LL at the end there?
#define DEF_MAX_JOB_NUM      1000
#define DEF_EXCLUSIVE        FALSE
#define DEF_EVENT_WATCH_TIME 60
#define DEF_COND_CHECK_TIME  600
#define DEF_MAX_SBD_CONNS     774
#define DEF_SCHED_STAY        3
#define DEF_FRESH_PERIOD     15
#define DEF_PEND_EXIT       512
#define DEF_JOB_ARRAY_SIZE  1000

#define DEF_LONG_JOB_TIME  1800

#define MAX_JOB_PRIORITY   INFINIT_INT

#define DEF_PRE_EXEC_DELAY    -1

/* Global MBD job lists
 */
typedef enum
{
  SJL,
  MJL,
  PJL,
  FJL,
  NJLIST,
  ZJL,
  ALLJLIST
} jlistno_t;

#define DEF_USR1_SC    0
#define DEF_USR1_ST    64000
#define DEF_USR2_SC    0
#define DEF_USR2_ST    64000


#define BAD_LOAD     1
#define TOO_LATE     2
#define FILE_MISSING 3
#define BAD_USER     4
#define JOB_MISSING  5
#define BAD_SUB      6
#define MISS_DEADLINE 8


#define JFLAG_LASTRUN_SUCC     0x002
#define JFLAG_DEPCOND_INVALID  0x004

#define JFLAG_READY            0x008
#define JFLAG_EXACT            0x200
#define JFLAG_UPTO             0x400
#define JFLAG_DEPCOND_REJECT   0x8000
#define JFLAG_SEND_SIG         0x10000
#define JFLAG_BTOP             0x20000
#define JFLAG_ADM_BTOP         0x40000
#define JFLAG_READY1           0x100000
#define JFLAG_READY2           0x200000
#define JFLAG_URGENT           0x400000
#define JFLAG_URGENT_NOSTOP    0x800000
#define JFLAG_REQUEUE          0x1000000
#define JFLAG_HAS_BEEN_REQUEUED 0x2000000

#define JFLAG_JOB_ANALYZER  0x20000000

#define JFLAG_WILL_BE_PREEMPTED  0x40000000

#define JFLAG_WAIT_SWITCH  0x80000000


#define M_STAGE_GOT_LOAD    0x0001
#define M_STAGE_LSB_CAND    0x0002
#define M_STAGE_QUEUE_CAND    0x0004
#define M_STAGE_REPLAY      0x0008
#define M_STAGE_INIT        0x0010
#define M_STAGE_RESUME_SUSP 0x0020
#ifdef MAINTAIN_FCFS_FOR_DEPENDENT_JOBS
#define M_STAGE_CHK_JGRPDEP 0x0040
#endif

#define JOB_STAGE_READY 0x0001
#define JOB_STAGE_CAND  0x0002
#define JOB_STAGE_DISP  0x0004
#define JOB_STAGE_DONE  0x0008
#define JOB_IS_PROCESSED(jp) ((jp)->processed & JOB_STAGE_DONE)

int mSchedStage;
int freshPeriod;
int maxSchedStay;

#define DEL_ACTION_KILL      0x01
#define DEL_ACTION_REQUEUE   0x02

#define ALL_USERS_ADMINS  INFINIT_INT

#define LOG_IT              0

#define CALCULATE_INTERVAL     900

struct qPRValues
{
  struct qData *qData;
  float usedByRunJob;
  float reservedByPreemptWait;
  float availableByPreempt;
  float preemptingRunJob;
};

struct preemptResourceInstance
{
  struct resourceInstance *instancePtr;
  int nQPRValues;
  struct qPRValues *qPRValues;
};

struct preemptResource
{
  int index;
  int numInstances;
  struct preemptResourceInstance *pRInstance;

};

struct objPRMO
{
  int numPreemptResources;
  struct preemptResource *pResources;

};
struct objPRMO *pRMOPtr;

#define PRMO_ALLVALUES                  0x0000
#define PRMO_USEDBYRUNJOB               0x0001
#define PRMO_RESERVEDBYPREEMPTWAIT      0x0002
#define PRMO_AVAILABLEBYPREEMPT         0x0004
#define PRMO_PREEMPTINGRUNJOB           0x0008

#define JOB_PREEMPT_WAIT(s)  ((s)->jStatus & JOB_STAT_RSRC_PREEMPT_WAIT)

#define MARKED_WILL_BE_PREEMPTED(s)  ((s)->jFlags & JFLAG_WILL_BE_PREEMPTED)

#define FORALL_PRMPT_RSRCS(resn) if (pRMOPtr != NULL) { \
    int _pRMOindex;                                     \
    for (_pRMOindex = 0;                                \
             _pRMOindex < pRMOPtr->numPreemptResources; \
             _pRMOindex++) {                            \
        resn = pRMOPtr->pResources[_pRMOindex].index;

#define ENDFORALL_PRMPT_RSRCS }         \
}

#define GET_RES_RSRC_USAGE(resn, val, jResValPtr, qResValPtr) { \
      int jobSet = FALSE, queueSet = FALSE;                     \
      if (jResValPtr)                                           \
          TEST_BIT(resn, jResValPtr->rusgBitMaps, jobSet);      \
      if (qResValPtr)                                           \
          TEST_BIT(resn, qResValPtr->rusgBitMaps, queueSet);    \
      if (jobSet == 0 && queueSet == 0) {                       \
          val = -1.0;                                           \
      } else if (jobSet == 0 && queueSet != 0) {                \
          val = qResValPtr->val[resn];                          \
      } else if (jobSet != 0 && queueSet == 0) {                \
          val = jResValPtr->val[resn];                          \
      } else if (jobSet != 0 && queueSet != 0) {                \
          val = jResValPtr->val[resn];                          \
      }                                                         \
  }

struct candHost
{
  struct hData *hData;
  int numSlots;
  int numAvailSlots;

  int numNonBackfillSlots;
  int numAvailNonBackfillSlots;
  LIST_T *backfilleeList;
};

struct askedHost
{
  struct hData *hData;
  int priority;
};

#define CLEAR_REASON(v, reason) if (v == reason) v = 0;
#define SET_REASON(condition, v, reason) \
        if (condition) v = reason; else CLEAR_REASON(v, reason)

#define NON_PRMPT_Q(qAttrib)    TRUE


struct rqHistory
{
  struct hData *host;
  int retry_num;
  time_t lasttime;
};

#define JOB_PEND(s)  (((s)->jStatus & JOB_STAT_PEND) && !((s)->jStatus & JOB_STAT_PSUSP))


struct groupCandHosts
{
  int numOfMembers;
  int tried;
  struct candHost *members;
};


struct jData // FIXME FIXME FIXME FIXME FIXME needs checking over
{
  struct jData *forw;
  struct jData *back;
  uid_t userId;
  char *userName;
  struct uData *uPtr;
  size_t jobId;
  float priority;
  int jStatus;
  time_t updStateTime;
  int jFlags;
  int oldReason;
  int newReason;
  int subreasons;
  int *reasonTb;
  int numReasons;
  struct qData *qPtr;
  struct hData **hPtr;
  int numHostPtr;
  struct askedHost *askedPtr;
  int numAskedPtr;
  int askedOthPrio;
  struct candHost *candPtr;
  int numCandPtr;
  struct candHost *execCandPtr;
  int numExecCandPtr;
  int numEligProc;
  int numAvailEligProc;
  int numSlots;         // size_t maybe?
  int numAvailSlots;    // size_t maybe?
  char usePeerCand;
  time_t reserveTime;
  int slotHoldTime;
  char processed;
  int dispCount;
  time_t dispTime;
  time_t startTime;
  time_t resumeTime;
  time_t predictedStartTime;
  int runCount;
  int retryHist;
  int nextSeq;
  int jobPid;
  int jobPGid;
  int runTime;
  float cpuTime;
  time_t endTime;
  time_t requeueTime;
  struct pendEvent
  {
    int notSwitched;
    int sig;
    int sig1;
    int sig1Flags;
    int sigDel;
    int notModified;
  } pendEvent;
  int numDependents;
  char *schedHost;
  int actPid;
  time_t ssuspTime;
  struct submitReq *newSub;
  struct lsfRusage *lsfRusage;
  int execUid;
  struct rqHistory *reqHistory;
  int lastDispHost;
  int requeMode;
  int reqHistoryAlloc;
  int exitStatus;
  char *execCwd;
  char *execHome;
  char *execUsername;
  char *queuePreCmd;
  char *queuePostCmd;
  int initFailCount;
  time_t jRusageUpdateTime;
  struct jRusage runRusage;
  int numUserGroup;
  char **userGroup;
  char *execHosts;
  int sigValue;
  struct jShared *shared;
  int numRef;
  struct jgTreeNode *jgrpNode;
  int nodeType;
  struct jData *nextJob;
  int restartPid;
  time_t chkpntPeriod;
  u_short port;


  int jSubPriority;
  int jobPriority;
  char *jobSpoolDir;
  struct hData **rsrcPreemptHPtr;
  int numRsrcPreemptHPtr;

  struct groupCandHosts *groupCands;
  int numOfGroups;
  int reservedGrp;
  int currentGrp;
  int *inEligibleGroups;
  int numSlotsReserve;
  int numAvailSlotsReserve;

};


#define JOB_HAS_CANDHOSTS(Job)  ((Job)->candPtr != NULL)

#define JOB_CANDHOST(Job,I) (Job)->candPtr[(I)].hData

#define JOB_CAND_HOSTNAME(Job,I) (Job)->candPtr[(I)].hData->host

#define JOB_EXECHOST(Job, I) (Job)->hPtr[(I)]

#define JOB_SUBMIT_TIME(Job)  (Job)->shared->jobBill.submitTime

#define JOB_PROJECT_NAME(Job) (Job)->shared->jobBill.projectName


#define FOR_EACH_JOB_LOCAL_EXECHOST(Host, Job) \
{ \
    int   __hidx__; \
    for (__hidx__ = 0; __hidx__ < (Job)->numHostPtr; __hidx__++) { \
        struct hData *Host = (Job)->hPtr[__hidx__]; \
        if (Host == NULL) break; \
        if (Host->hStatus & HOST_STAT_REMOTE) continue;

#define END_FOR_EACH_JOB_LOCAL_EXECHOST }}

#define JOB_RUNSLOT_NONPRMPT(Job) \
    (   ((Job)->jFlags & JFLAG_URGENT) \
     || ((Job)->qPtr->qAttrib & QUEUE_ATTRIB_BACKFILL) \
    )

#define JOB_RSVSLOT_NONPRMPT(Job) \
    (   (! (Job)->jStatus & JOB_STAT_PEND) \
    )


struct jShared
{
  int numRef;
  struct dptNode *dptRoot;
  struct submitReq jobBill;
  struct resVal *resValPtr;
};


#define HAS_LOCAL_RESERVEDHOSTS(JP) \
    ((JP)->jStatus & JOB_STAT_RESERVE && (JP)->numHostPtr > 0)

#define CONFIRM_LOCAL_RESERVEDHOSTS(JP) \
{ \
    updResCounters((JP), ~JOB_STAT_RESERVE); \
    (JP)->jStatus &= ~JOB_STAT_RESERVE; \
}

struct hostAcct
{
  struct hData *hPtr;
  int numRUN;
  int numSSUSP;
  int numUSUSP;
  int numRESERVE;
  int numNonPrmptRsv;
  int numAvailSUSP;
};

struct uData
{
  char *user;
  int uDataIndex;
  int flags;
  struct uData **gPtr;
  int numGrpPtr;
  struct gData *gData;
  int maxJobs;
  float pJobLimit;
  struct hTab *hAcct;
  int numPEND;
  int numRUN;
  int numSSUSP;
  int numUSUSP;
  int numJobs;
  int numRESERVE;
  int **reasonTb;
  int numSlots;
  LS_BITSET_T *children;
  LS_BITSET_T *descendants;
  LS_BITSET_T *parents;
  LS_BITSET_T *ancestors;
  LIST_T *pxySJL;
};

#define USER_GROUP_IS_ALL_USERS(UserGroup) \
     ((UserGroup)->numGroups == 0 && \
      (UserGroup)->memberTab.numEnts == 0) \

#define FOR_EACH_UGRP_CHILD_USER(User, Grp) \
    if ((User)->children != NULL) { \
        struct uData *Grp; \
        LS_BITSET_ITERATOR_T __iter__; \
        BITSET_ITERATOR_ZERO_OUT(&__iter__); \
        setIteratorAttach(&__iter__, (User)->children, "FOR_EACH_UGRP_CHILD_USER"); \
        for (Grp = (struct uData *)setIteratorBegin(&__iter__); \
             Grp != NULL; \
             Grp = (struct uData *)setIteratorGetNextElement(&__iter__)) \
        {

#define END_FOR_EACH_UGRP_CHILD_USER }}

#define FOR_EACH_UGRP_DESCENDANT_USER(Grp, User) \
    if ((User)->children != NULL) { \
        struct uData *User; \
        LS_BITSET_ITERATOR_T __iter__; \
        BITSET_ITERATOR_ZERO_OUT(&__iter__); \
        setIteratorAttach(&__iter__, (Grp)->descendants, "FOR_EACH_UGRP_CHILD_USER"); \
        for (User = (struct uData *)setIteratorBegin(&__iter__); \
             User != NULL; \
             User = (struct uData *)setIteratorGetNextElement(&__iter__)) \
        {

#define END_FOR_EACH_UGRP_DESCENDANT_USER }}

#define FOR_EACH_USER_ANCESTOR_UGRP(User, Grp) \
    if ((User)->ancestors != NULL) { \
        struct uData *Grp; \
        LS_BITSET_ITERATOR_T __iter__; \
        BITSET_ITERATOR_ZERO_OUT(&__iter__); \
        setIteratorAttach(&__iter__, (User)->ancestors, "FOR_EACH_USER_ANCESTOR_UGRP"); \
        for (Grp = (struct uData *)setIteratorBegin(&__iter__); \
             Grp != NULL; \
             Grp = (struct uData *)setIteratorGetNextElement(&__iter__)) \
        {

#define END_FOR_EACH_USER_ANCESTOR_UGRP }}

struct uDataTable
{
  struct uData **_base_;
  int _cur_;
  int _size_;
};

typedef struct uDataTable UDATA_TABLE_T;

#define UDATA_TABLE_NUM_ELEMENTS(Table) ( (Table)->_cur_ )

struct userAcct
{
  struct uData *uData;
  int userId;
  int numPEND;
  int numRUN;
  int numSSUSP;
  int numUSUSP;
  int numRESERVE;
  int numNonPrmptRsv;
  int numAvailSUSP;
  int reason;
  int numRunFromLastSession;
  int numVisitedInSession;
  int numPendJobsInSession;
  bool_t skipAccount;
};

struct qData
{
  struct qData *forw;
  struct qData *back;
  char *queue;
  int queueId;
  char *description;
  struct gData *hGPtr;
  int numProcessors;
  int nAdmins;
  int *adminIds;
  char *admins;
  char *resReq;
  struct resVal *resValPtr;
  float *loadSched;
  float *loadStop;
  int mig;
  int schedDelay;
  int acceptIntvl;
  int priority;
  int nice;
  char *preCmd;
  char *postCmd;
  char *prepostUsername;

  struct requeueEStruct
  {
    int type;
#define RQE_NORMAL   0
#define RQE_EXCLUDE  1
#define RQE_END     255
    int value;
    int interval;
  } *requeEStruct;

  char *requeueEValues;
  char *windowsD;
  windows_t *week[8];
  char *windows;
  windows_t *weekR[8];
  time_t windEdge;
  time_t runWinCloseTime;
  int rLimits[LSF_RLIM_NLIMITS];
  int defLimits[LSF_RLIM_NLIMITS];
  int procLimit;
  char *hostSpec;
  char *defaultHostSpec;
  int hJobLimit;
  float pJobLimit;
  struct hTab *hAcct;
  int uJobLimit;
  struct hTab *uAcct;
  int qAttrib;
  int qStatus;
  int maxJobs;
  int numJobs;
  int numPEND;
  int numRUN;
  int numSSUSP;
  int numUSUSP;
  int numRESERVE;
  int **reasonTb;
  int numReasons;
  int numSlots;
  int numUsable;
  int schedStage;
  int slotHoldTime;
  char *resumeCond;
  struct resVal *resumeCondVal;
  char *stopCond;
  char *jobStarter;
  int flags;
  char *suspendActCmd;
  char *resumeActCmd;
  char *terminateActCmd;
  int sigMap[LSB_SIG_NUM];
  struct gData *uGPtr;
  LS_BITSET_T *hostInQueue;
  char *hostList;
  int numHUnAvail;
  struct askedHost *askedPtr;
  int numAskedPtr;
  int askedOthPrio;
  struct jData *firstJob[PJL + 1];
  struct jData *lastJob[PJL + 1];
  time_t chkpntPeriod;
  char *chkpntDir;
  int minProcLimit;
  int defProcLimit;
};


#define HOST_STAT_REMOTE       0x80000000


struct hData
{
  struct hData *forw;
  struct hData *back;
  char *host;
  int hostId;
  char *hostType;
  char *hostModel;
  struct hostent hostEnt;
  float cpuFactor;
  int numCPUs;
  float *loadSched;
  float *loadStop;
  char *windows;
  windows_t *week[8];
  time_t windEdge;
  int acceptTime;
  int numDispJobs;
  time_t pollTime;
  int sbdFail;
  int hStatus;
  int uJobLimit;
  struct hTab *uAcct;
  int maxJobs;
  int numJobs;
  int numRUN;
  int numSSUSP;
  int numUSUSP;
  int numRESERVE;
  int mig;
  int chkSig;
  int maxMem;
  int maxSwap;
  int maxTmp;
  int nDisks;
  int *resBitMaps;
  int *limStatus;
  float *lsfLoad;
  float *lsbLoad;
  struct bucket *msgq[3];
  int *busyStop;
  int *busySched;
  int reason;
  int flags;
  int numInstances;
  struct resourceInstance **instances;
  LIST_T *pxySJL;
  LIST_T *pxyRsvJL;
  float leftRusageMem;
};


struct sbdNode
{
  struct sbdNode *forw;
  struct sbdNode *back;
  int chanfd;
  struct jData *jData;
  struct hData *hData;
  sbdReqType reqCode;
  time_t lastTime;
  int sigVal;
  int sigFlags;
};

struct sbdNode sbdNodeList;

struct gData
{
  char *group;
  hTab memberTab;
  int numGroups;
  struct gData *gPtr[MAX_GROUPS];
};

typedef enum
{
  DPT_AND = 0,
  DPT_OR = 1,
  DPT_NOT = 2,
  DPT_LEFT_ = 3,
  DPT_RIGHT_ = 4,
  DPT_DONE = 5,
  DPT_EXIT = 6,
  DPT_STARTED = 7,
  DPT_NAME = 8,
  DPT_ENDED = 9,
  DPT_NUMPEND = 10,
  DPT_NUMHOLD = 11,
  DPT_NUMRUN = 12,
  DPT_NUMEXIT = 13,
  DPT_NUMDONE = 14,
  DPT_NUMSTART = 15,
  DPT_NUMENDED = 16,
  DPT_COMMA = 17,
  DPT_GT = 18,
  DPT_GE = 19,
  DPT_LT = 20,
  DPT_LE = 21,
  DPT_EQ = 22,
  DPT_NE = 23,
  DPT_POST_DONE = 24,
  DPT_POST_ERR = 25,
  DPT_TRUE = 26,
  DPT_WINDOW = 27,

} dptType;

#define DP_FALSE     0
#define DP_TRUE      1
#define DP_INVALID  -1
#define DP_REJECT   -2

#define ARRAY_DEP_ONE_TO_ONE 1

struct jobIdx
{
  int numRef;
  struct idxList *idxList;
  struct listSet *depJobList;
};

struct dptNode
{
  dptType type;
  int value;
  int updFlag;
  union
  {
    struct
    {
      struct dptNode *left;
      struct dptNode *right;
    } opr;
    struct
    {
      int opType;
      int exitCode;
      int opFlag;
      struct jData *jobRec;
      struct jobIdx *jobIdx;
    } job;
    struct
    {
      struct timeWindow *timeWindow;
    } window;
    struct
    {
      int opType;
      int num;
      struct jgArrayBase *jgArrayBase;
    } jgrp;
  } dptUnion;
};

#define WINDOW_CLOSE       0
#define WINDOW_OPEN        1
struct timeWindow
{
  int status;
  char *windows;
  windows_t *week[8];
  time_t windEdge;
};

#define JOB_NEW    1
#define JOB_REQUE  2
#define JOB_REPLAY 3

struct clientNode
{
  struct clientNode *forw;
  struct clientNode *back;
  int chanfd;
  struct sockaddr_in from;
  char *fromHost;
  mbdReqType reqType;
  time_t lastTime;
};

struct condData
{
  char *name;
  int status;
  int lastStatus;
  time_t lastTime;
  int flags;
  struct dptNode *rootNode;
};

struct resourceInstance
{
  char *resName;
  int nHosts;
  struct hData **hosts;
  char *lsfValue;
  char *value;
};

struct profileCounters
{
  int cntVal;
  char *cntDescr;
};

#undef MBD_PROF_COUNTER
#define MBD_PROF_COUNTER(Func) PROF_CNT_ ## Func,

typedef enum profCounterType
{
#   include "mbd.profcnt.def"
  PROF_CNT_nullfunc
} PROF_COUNTER_TYPE_T;

#define INC_CNT(counterId) \
               { \
                  counters[counterId].cntVal++; \
               }

#define RESET_CNT() \
                 { \
                   int i; \
                   for(i = 0; counters[i].cntDescr != NULL; i++) \
                       counters[i].cntVal = 0; \
                 }
#define DUMP_CNT() \
                { \
                  int i; \
                  if (logclass & LC_PERFM )  \
                  for(i = 0; counters[i].cntDescr != NULL; i++) { \
                     if(counters[i].cntVal != 0) { \
                          ls_syslog(LOG_INFO,"dumpCounters: %s <%d>", \
                              counters[i].cntDescr,counters[i].cntVal); \
                     } \
                  } \
                }

#define CONF_COND 0x001

#define QUEUE_UPDATE      0x01
#define QUEUE_NEEDPOLL    0x02
#define QUEUE_REMOTEONLY  0x04
#define QUEUE_UPDATE_USABLE 0x08

/* Various host flags..
 */
#define HOST_UPDATE       (1 << 0)
#define HOST_NEEDPOLL     (1 << 1)
#define HOST_UPDATE_LOAD  (1 << 2)
#define HOST_JOB_RESUME   (1 << 3)
#define HOST_AUTOCONF_MXJ (1 << 5)
#define HOST_LOST_FOUND   (1 << 6)

#define USER_GROUP     0x001
#define USER_UPDATE    0x002
#define USER_INIT      0x004
#define USER_BROKER    0x008
#define USER_OTHERS    0x010
#define USER_ALL       0x020

#define FIRST_START   1
#define WINDOW_CONF   2
#define RECONFIG_CONF 3
#define NORMAL_RUN    4


#define dptJobRec  dptUnion.job.jobRec
#define dptJobIdx  dptUnion.job.jobIdx
#define dptJobParents  dptUnion.job.parentNodes
#define dptLeft  dptUnion.opr.left
#define dptRight dptUnion.opr.right
#define dptJgrp dptUnion.jgrp.jgArrayBase
#define dptWindow dptUnion.window.timeWindow


LIST_T *hostList;
struct hTab hostTab;
struct jData *jDataList[];
struct migJob *migJobList;
struct qData *qDataList;
UDATA_TABLE_T *uDataPtrTb;
struct hTab uDataList;
struct hTab calDataList;
struct jData *chkJList;
struct clientNode *clientList;
struct hTab jobIdHT;
struct hTab jgrpIdHT;
struct gData *usergroups[];
struct gData *hostgroups[];
struct profileCounters counters[];
char errstr[];
int debug;
int errno;
int nextId;
int numRemoteJobsInList;


char *defaultQueues;
char *defaultHostSpec;
int max_retry;
int max_sbdFail;
int accept_intvl;
int clean_period;
int delay_period;
char *dbSelectLoad;
char *pjobSpoolDir;
int preExecDelay;
int slotResourceReserve;
int maxAcctArchiveNum;
int acctArchiveInDays;
int acctArchiveInSize;
int lsbModifyAllJobs;


int numofqueues;
int numofprocs;
int numofusers;
int numofugroups;
int numofhgroups;
int maxjobnum;


int msleeptime;
int numRemoveJobs;
int eventPending;
int qAttributes;
int **hReasonTb;
uid_t *managerIds;
char **lsbManagers;
int nManagers;
char *lsfDefaultProject;
int dumpToDBPid;
int dumpToDBExit;
int maxJobArraySize;
int jobRunTimes;
int jobDepLastSub;
int maxUserPriority;
int jobPriorityValue;
int jobPriorityTime;
int scheRawLoad;

time_t last_hostInfoRefreshTime;
struct hTab condDataList;
int readNumber;
time_t condCheckTime;
struct userConf *userConf;
time_t last_hostInfoRefreshTime;
struct hTab condDataList;
int readNumber;
time_t condCheckTime;
struct userConf *userConf;

bool_t mcSpanClusters;
bool_t disableUAcctMap;

int numResources;
struct sharedResource **sharedResources;

int nSbdConnections;
int maxSbdConnections;
int maxJobPerSession;

struct hostInfo *LIMhosts;
int numLIMhosts;

float maxCpuFactor;
int freedSomeReserveSlot;

long schedSeqNo;


void pollSbatchds (int);
void hStatChange (struct hData *, int status);
int checkHosts (struct infoReq *, struct hostDataReply *);
struct hData *getHostData (char *host);
struct hData *getHostData2 (char *host);
float *getHostFactor (char *host);
float *getModelFactor (char *hostModel);
int getModelFactor_r (char *hostModel, float *cpuFactor);
void checkHWindow (void);
struct hEnt *findHost (char *hname);
void renewJob (struct jData *oldjob);
void getTclHostData (struct tclHostData *,
			    struct hData *, struct hData *);
int getLsbHostNames (char ***);
void getLsbHostInfo (void);
int getLsbHostLoad (void);
int getHostsByResReq (struct resVal *, int *,
			     struct hData **,
			     struct hData ***, struct hData *, int *);

struct resVal *checkResReq (char *, int);
void adjLsbLoad (struct jData *, int, bool_t);
int countHostJobs (struct hData *);
void getLsbResourceInfo (void);
struct resVal *getReserveValues (struct resVal *, struct resVal *);
void getLsfHostInfo (int);
struct hData *getHostByType (char *);

void checkQWindow (void);
int checkQueues (struct infoReq *, struct queueInfoReply *);
int ctrlQueue (struct controlReq *, struct lsfAuth *);
int ctrlHost (struct controlReq *, struct hData *, struct lsfAuth *);
void sumHosts (void);
void inQueueList (struct qData *);
struct qData *getQueueData (char *);
char hostQMember (char *, struct qData *);
char userQMember (char *, struct qData *);
int isQueAd (struct qData *, char *);
int isAuthQueAd (struct qData *, struct lsfAuth *);
int isInQueues (char *, char **, int);
void freeQueueInfoReply (struct queueInfoReply *, char *);
struct hostInfo *getLsfHostData (char *);
int createQueueHostSet (struct qData *);
int gethIndexByhData (void *);
void *gethDataByhIndex (int);
bool_t isHostQMember (struct hData *, struct qData *);
int getIndexByQueue (void *);
void *getQueueByIndex (int);
bool_t isQInQSet (struct qData *, LS_BITSET_T *);

struct listSet *voidJobList;
int newJob (struct submitReq *,
		   struct submitMbdReply *, int,
		   struct lsfAuth *, int *, int, struct jData **);
int chkAskedHosts (int, char **, int, int *,
			  struct askedHost **, int *, int *, int);
int selectJobs (struct jobInfoReq *, struct jData ***, int *);
int signalJob (struct signalReq *, struct lsfAuth *);
int statusJob (struct statusReq *, struct hostent *, int *);
int rusageJob (struct statusReq *, struct hostent *);
int statusMsgAck (struct statusReq *);
int switchJobArray (struct jobSwitchReq *, struct lsfAuth *);
int sbatchdJobs (struct sbdPackage *, struct hData *);
int countNumSpecs (struct hData *);
void packJobSpecs (struct jData *, struct jobSpecs *);
void freeJobSpecs (struct jobSpecs *);
int peekJob (struct jobPeekReq *,
		    struct jobPeekReply *, struct lsfAuth *);
int migJob (struct migReq *,
		   struct submitMbdReply *, struct lsfAuth *);
void clean (time_t);
int moveJobArray (struct jobMoveReq *, int, struct lsfAuth *);
void job_abort (struct jData *jData, char reason);
void marktime (struct jData *, int);
int rmjobfile (struct jData *jData);
void jStatusChange (struct jData *, int, time_t, const char *);
int findLastJob (int, struct jData *, struct jData **);
void initJobIdHT (void);
struct jData *getJobData (size_t jobId);
void inPendJobList (struct jData *, int list, time_t);
void inStartJobList (struct jData *);
void inFinishJobList (struct jData *);
void jobInQueueEnd (struct jData *, struct qData *);
struct jData *initJData (struct jShared *);
void assignLoad (float *, float *, struct qData *, struct hData *);
int resigJobs (int *resignal);
void removeJob (size_t);
bool_t runJob (struct runJobRequest *, struct lsfAuth *);
void addJobIdHT (struct jData *);
struct jData *createjDataRef (struct jData *);
void destroyjDataRef (struct jData *);
void setJobPendReason (struct jData *, int);
void destroySharedRef (struct jShared *);
struct jShared *createSharedRef (struct jShared *);
time_t runTimeSinceResume (const struct jData *);

void modifyJobPriority (struct jData *, int);
float queueGetUnscaledRunTimeLimit (struct qData *qp);
int arrayRequeue (struct jData *,
			 struct signalReq *, struct lsfAuth *);

int modifyJob (struct modifyReq *,
		      struct submitMbdReply *, struct lsfAuth *);
void freeJData (struct jData *);
void handleJParameters (struct jData *, struct jData *,
			       struct submitReq *, int, int, int);
void handleNewJob (struct jData *, int, int);
void copyJobBill (struct submitReq *, struct submitReq *, size_t);
void inZomJobList (struct jData *, int);
struct jData *getZombieJob (size_t);
int getNextJobId (void);
void accumRunTime (struct jData *, int, time_t);
void signalReplyCode (sbdReplyType reply,
			     struct jData *jData, int sigValue, int chkFlags);
void jobStatusSignal (sbdReplyType reply,
			     struct jData *jData,
			     int sigValue, int chkFlags,
			     struct jobReply *jobReply);
void tryResume (void);
void freeSubmitReq (struct submitReq *);
int shouldLockJob (struct jData *, int);
int sigPFjob (struct jData *, int, time_t, int);
void offJobList (struct jData *, int);
void handleRequeueJob (struct jData *, time_t);
int PJLorMJL (struct jData *);

void schedulerInit (void);
int scheduleAndDispatchJobs (void);
int scheduleJobs (int *schedule, int *dispatch, struct jData *);
int dispatchJobs (int *dispatch);
void updNumDependents (struct dptNode *, int);
int userJobLimitCk (struct jData *, int disp);
int pJobLimitOk (struct hData *, struct hostAcct *, float pJobLimit);
int uJobLimitOk (struct jData *, struct hTab *, int, int disp);
int hostSlots (int, struct jData *, struct hData *, int disp, int *);
void disp_clean_job (struct jData *);
bool_t dispatch_it (struct jData *);
int findBestHosts (struct jData *, struct resVal *, int, int,
			  struct candHost *, bool_t);
int hJobLimitOk (struct hData *, struct hostAcct *, int);
void freeReserveSlots (struct jData *);
int jobStartError (struct jData *jData, sbdReplyType reply);
int cntNumPrmptSlots (struct qData *, struct hData *,
			     struct uData *, int *numAvailSUSP);
int skipAQueue (struct qData *qp2, struct qData *qp1);
int userJobLimitOk (struct jData *, int, int *);
int getQUsable (struct qData *);
int handleForeignJob (struct jData *);
int reservePreemptResourcesForHosts (struct jData *jp);
int freeReservePreemptResources (struct jData *jp);
int deallocReservePreemptResources (struct jData *jp);
int orderByStatus (struct candHost *, int, bool_t);
void setLsbPtilePack (const bool_t);
int do_submitReq (XDR *, int, struct sockaddr_in *,
			 char *, struct LSFHeader *,
			 struct sockaddr_in *,
			 struct lsfAuth *, int *, int, struct jData **);
int do_signalReq (XDR *, int, struct sockaddr_in *,
			 char *, struct LSFHeader *, struct lsfAuth *);
int do_jobMsg (struct bucket *, XDR *, int,
		      struct sockaddr_in *,
		      char *, struct LSFHeader *, struct lsfAuth *);
int do_statusReq (XDR *, int, struct sockaddr_in *,
			 int *, struct LSFHeader *);
int do_errorReq (int, struct LSFHeader *);
int do_jobSwitchReq (XDR *, int, struct sockaddr_in *,
			    char *, struct LSFHeader *, struct lsfAuth *);
int do_hostInfoReq (XDR *, int, struct sockaddr_in *,
			   struct LSFHeader *);
int do_jobPeekReq (XDR *, int, struct sockaddr_in *,
			  char *, struct LSFHeader *, struct lsfAuth *);
int do_jobInfoReq (XDR *, int, struct sockaddr_in *,
			  struct LSFHeader *, int);
int do_queueInfoReq (XDR *, int, struct sockaddr_in *,
			    struct LSFHeader *);
int do_debugReq (XDR * xdrs, int chfd,
			struct sockaddr_in *from,
			char *hostName,
			struct LSFHeader *reqHdr, struct lsfAuth *auth);
int do_groupInfoReq (XDR *, int, struct sockaddr_in *,
			    struct LSFHeader *);
int do_queueControlReq (XDR *, int,
			       struct sockaddr_in *, char *,
			       struct LSFHeader *, struct lsfAuth *);
int do_reconfigReq (XDR *, int, struct sockaddr_in *,
			   char *, struct LSFHeader *);
int do_restartReq (XDR *, int, struct sockaddr_in *,
			  struct LSFHeader *);
int do_hostControlReq (XDR *, int,
			      struct sockaddr_in *, char *,
			      struct LSFHeader *, struct lsfAuth *);
int do_jobMoveReq (XDR *, int, struct sockaddr_in *,
			  char *, struct LSFHeader *, struct lsfAuth *);
int do_userInfoReq (XDR *, int, struct sockaddr_in *,
			   struct LSFHeader *);
int do_paramInfoReq (XDR *, int, struct sockaddr_in *,
			    struct LSFHeader *);
int do_hostPartInfoReq (XDR *, int,
			       struct sockaddr_in *, struct LSFHeader *);
int do_migReq (XDR *, int, struct sockaddr_in *, char *,
		      struct LSFHeader *, struct lsfAuth *);
int do_modifyReq (XDR *, int, struct sockaddr_in *,
			 char *, struct LSFHeader *, struct lsfAuth *);
void doNewJobReply (struct sbdNode *, int);
void doProbeReply (struct sbdNode *, int);
void doSignalJobReply (struct sbdNode *sbdPtr, int);
void doSwitchJobReply (struct sbdNode *sbdPtr, int);
int do_resourceInfoReq (XDR *, int,
			       struct sockaddr_in *, struct LSFHeader *);
int do_runJobReq (XDR *,
			 int,
			 struct sockaddr_in *,
			 struct lsfAuth *, struct LSFHeader *);
int getQUsable (struct qData *);
void allocateRemote (struct jData *, int);
void setExecHostsAcceptInterval (struct jData *);
#if defined(INTER_DAEMON_AUTH)
int authDaemonRequest (int chfd,
			      XDR * xdrs,
			      struct LSFHeader *reqHdr,
			      struct sockaddr_in *from_host,
			      char *client, char *server);
#endif


int requeueEParse (struct requeueEStruct **, char *, int *);
int fill_requeueHist (struct rqHistory **, int *, struct hData *);
int match_exitvalue (struct requeueEStruct *, int);
void clean_requeue (struct qData *);

LS_BITSET_T *allUsersSet;
LS_BITSET_T *uGrpAllSet;
LS_BITSET_T *uGrpAllAncestorSet;
int userSetOnNewUser (LS_BITSET_T *, void *, LS_BITSET_EVENT_T *);
int checkGroups (struct infoReq *, struct groupInfoReply *);
void fillMembers (struct gData *, char **, char);
char **expandGrp (struct gData *, char *, int *);
void fillMembers (struct gData *, char **, char);
char *getGroupMembers (struct gData *, char);
char *catGnames (struct gData *);
struct gData *getGroup (int, char *);
char gMember (char *word, struct gData *);
char gDirectMember (char *, struct gData *);
int countEntries (struct gData *, char);
struct gData *getUGrpData (char *);
struct gData *getHGrpData (char *);
struct gData *getGrpData (struct gData **, char *, int);
int sumMembers (struct gData *, char r, int first);
void createGroupuData ();
void createGroupTbPtr ();
void createGroupSet ();
int getIndexByuData (void *);
void *getuDataByIndex (int);
UDATA_TABLE_T *uDataTableCreate (void);
void uDataPtrTbInitialize (void);
void uDataTableAddEntry (UDATA_TABLE_T *, struct uData *);
int uDataTableGetNumEntries (UDATA_TABLE_T *);
struct uData *uDataTableGetNextEntry (UDATA_TABLE_T *);
void setuDataCreate (void);
void updHostList (void);
void uDataGroupCreate (void);
int sizeofGroupInfoReply (struct groupInfoReply *);
void child_handler (int);
void terminate_handler (int);
void announce_master (void);
void shutDownClient (struct clientNode *);
void setNextSchedTimeUponNewJob (struct jData *);
void setJobPriUpdIntvl (void);
int isAuthManagerExt (struct lsfAuth *);
void updCounters (struct jData *jData, int newStatus, time_t);
void updSwitchJob (struct jData *, struct qData *,
			  struct qData *, int);
void updUserData (struct jData *, int, int, int, int, int, int);
void updQaccount (struct jData *jData, int, int, int, int, int, int);
struct uData *getUserData (char *user);
struct userAcct *getUAcct (struct hTab *, struct uData *);
struct hostAcct *getHAcct (struct hTab *, struct hData *);
struct uData *addUserData (char *, int, float, char *, int, int);
int checkUsers (struct infoReq *, struct userInfoReply *);
void checkParams (struct infoReq *, struct parameterInfo *);
void mbdDie (int);
int isManager (char *);
int isAuthManager (struct lsfAuth *);
int isJobOwner (struct lsfAuth *, struct jData *);
char *getDefaultProject (void);
void updResCounters (struct jData *, int);
struct hostAcct *addHAcct (struct hTab **, struct hData *,
				  int, int, int, int);
void checkQusable (struct qData *, int, int);
void updHostLeftRusageMem (struct jData *, int);
int minit (int);
struct qData *lostFoundQueue (void);
void freeHData (struct hData *);
void deleteQData (struct qData *);

int my_atoi (char *, int, int);
void freeKeyVal (struct keymap *);
void queueHostsPF (struct qData *, int *);
struct hData *initHData (struct hData *);
int updAllConfCond (void);
void mbdReConf (int);

int log_newjob (struct jData *);
void log_switchjob (struct jobSwitchReq *, int, char *);
void log_movejob (struct jobMoveReq *, int, char *);
void log_startjob (struct jData *, int);
void log_startjobaccept (struct jData *);
void log_newstatus (struct jData *);
void log_chkpnt (struct jData *, int, int);
void log_mig (struct jData *, int, char *);
void log_route (struct jData *);
int log_modifyjob (struct modifyReq *, struct lsfAuth *);
void log_queuestatus (struct qData *, int, int, char *);
void log_hoststatus (struct hData *, int, int, char *);
void log_mbdStart (void);
void log_mbdDie (int);
void log_unfulfill (struct jData *);
void log_jobaccept (struct jData *);
void log_jobclean (struct jData *);
void log_jobforward (struct jData *);
void log_statusack (struct jData *);
void log_logSwitch (int);
void replay_requeuejob (struct jData *);
int init_log (void);
void switchELog (void);
int switch_log (void);
void checkAcctLog (void);
int switchAcctLog (void);
void logJobInfo (struct submitReq *, struct jData *, struct lenData *);
int rmLogJobInfo_ (struct jData *, int);
int readLogJobInfo (struct jobSpecs *, struct jData *,
			   struct lenData *, struct lenData *);
void log_signaljob (struct jData *, struct signalReq *, int, char *);
void log_jobmsg (struct jData *, struct lsbMsg *, int);
void log_jobmsgack (struct bucket *);
char *readJobInfoFile (struct jData *, int *);
void writeJobInfoFile (struct jData *, char *, int);
int replaceJobInfoFile (char *, char *, char *, int);
void log_executejob (struct jData *);
void log_jobsigact (struct jData *, struct statusReq *, int);
void log_jobrequeue (struct jData *jData);
void log_jobForce (struct jData *, int, char *);

time_t eventTime;
void log_jobattrset (struct jobAttrInfoEnt *, int);

#define REPLACE_1ST_CMD_ONLY  (0x1)
int stripJobStarter (char *, int *, char *);

sbdReplyType start_job (struct jData *, struct qData *,
			       struct jobReply *);
sbdReplyType signal_job (struct jData *jobPtr, struct jobSig *,
				struct jobReply *jobReply);
sbdReplyType switch_job (struct jData *, int options);
sbdReplyType msg_job (struct jData *, struct Buffer *,
			     struct jobReply *);
sbdReplyType probe_slave (struct hData *, char sendJobs);
sbdReplyType rebootSbd (char *host);
sbdReplyType shutdownSbd (char *host);
struct dptNode *parseDepCond (char *, struct lsfAuth *,
				     int *, char **, int *, int);
int evalDepCond (struct dptNode *, struct jData *);
void freeDepCond (struct dptNode *);
void resetDepCond (struct dptNode *);
bool_t autoAdjustIsEnabled (void);
int getAutoAdjustAtNumPend (void);
float getAutoAdjustAtPercent (void);
bool_t autoAdjustIsEnabled (void);
float getHRValue (char *, struct hData *, struct resourceInstance **);
int checkResources (struct resourceInfoReq *,
			   struct lsbShareResourceInfoReply *);
struct sharedResource *getResource (char *);
void resetSharedResource (void);
void updSharedResourceByRUNJob (const struct jData *);
int sharedResourceUpdFactor;
void freeSharedResource (void);
void newPRMO (char *);
void delPRMO ();
void resetPRMOValues (int);
void printPRMOValues ();
void mbdReconfPRMO ();
float getUsablePRHQValue (int, struct hData *,
				 struct qData *, struct resourceInstance **);
float getAvailableByPreemptPRHQValue (int,
					     struct hData *, struct qData *);
float getReservedByWaitPRHQValue (int, struct hData *, struct qData *);
float takeAvailableByPreemptPRHQValue (int, float,
					      struct hData *, struct qData *);
void addRunJobUsedPRHQValue (int, float,
				    struct hData *, struct qData *);
void removeRunJobUsedPRHQValue (int, float,
				       struct hData *, struct qData *);
void addReservedByWaitPRHQValue (int, float,
					struct hData *, struct qData *);
void removeReservedByWaitPRHQValue (int, float,
					   struct hData *, struct qData *);
void addAvailableByPreemptPRHQValue (int, float,
					    struct hData *, struct qData *);
void removeAvailableByPreemptPRHQValue (int, float,
					       struct hData *,
					       struct qData *);
int resName2resIndex (char *);
int isItPreemptResourceName (char *);
int isItPreemptResourceIndex (int);
int isReservePreemptResource (struct resVal *);
int isHostsInSameInstance (int, struct hData *, struct hData *);



void freeIdxList (struct idxList *);
struct idxList *parseJobArrayIndex (char *, int *, int *);
void handleNewJobArray (struct jData *, struct idxList *, int);
void offArray (struct jData *);
int getJobIdList (char *, int *, size_t **);
int getJobIdIndexList (char *, int *, struct idxList **);
struct jData *copyJData (struct jData *);
struct jShared *copyJShared (struct jData *);
struct idxList *getIdxListContext (void);
void setIdxListContext (const char *);
void freeIdxListContext (void);

#define FIRST_CHILD(x)   (x)->child
#define PARENT(x)        (x)->parent
#define LEFT_SIBLING(x)  (x)->left
#define RIGHT_SIBLING(x) (x)->right
#define IS_ANCESTOR(x,y) is_ancestor(x,y)
#define IS_PARENT(x,y)   (((x)==PARENT(y))? TRUE:FALSE)
#define IS_CHILD(x,y)    is_child(x,y)
#define IS_SIBLING(x,y)  ((PARENT(x) == PARENT(y))? TRUE:FALSE)

long schedSeqNo;

#define QUEUE_LIMIT(qPtr, i) \
     ((qPtr)->defLimits[i] > 0 ? \
     (qPtr)->defLimits[i] : (qPtr)->rLimits[i])
#define LIMIT_OF_JOB(jp, i) \
    ((jp)->shared->jobBill.rLimits[i] > 0 ? \
     (jp)->shared->jobBill.rLimits[i] : \
     ((jp)->qPtr->defLimits[i] > 0 ? \
      (jp)->qPtr->defLimits[i] : (jp)->qPtr->rLimits[i]) \
    )
#define RUN_LIMIT_OF_JOB(jp) LIMIT_OF_JOB(jp, LSF_RLIMIT_RUN)
#define CPU_LIMIT_OF_JOB(jp) LIMIT_OF_JOB(jp, LSF_RLIMIT_CPU)

#define IGNORE_DEADLINE(qp) ((qp)->qAttrib & QUEUE_ATTRIB_IGNORE_DEADLINE)
#define HAS_RUN_WINDOW(qp) (((qp)->windows != NULL) && \
                            (qp)->windows[0] != '\0')


#define NOT_NUMERIC(res) (((res).valueType != LS_NUMERIC))

#define DESTROY_BACKFILLEE_LIST(backfilleeList) \
    if ((backfilleeList) != NULL) { \
        listDestroy((backfilleeList), NULL); \
        (backfilleeList) = NULL; \
    }


#define FREE_IND_CANDPTR( candPtr, num ) { \
                               if ((candPtr)) { \
                                   int ii; \
                                   for (ii=0; ii< (num); ii++) \
                                       DESTROY_BACKFILLEE_LIST((candPtr)[ii].backfilleeList); \
                                   FREEUP((candPtr)); \
                               } \
                           }

#define FREE_CAND_PTR(jp) FREE_IND_CANDPTR((jp)->candPtr, (jp)->numCandPtr)

#define FREE_ALL_GRPS_CAND(job) { \
                     int jj;  \
                     if ((job)->groupCands != NULL) { \
                         for (jj = 0; jj < (job)->numOfGroups ; jj++) { \
                             FREE_IND_CANDPTR((job)->groupCands[jj].members, (job)->groupCands[jj].numOfMembers); \
                         } \
                     }\
                     FREEUP((job)->groupCands);\
                     (job)->numOfGroups = 0; \
                     (job)->reservedGrp = (job)->currentGrp = -1; \
                     FREEUP((job)->inEligibleGroups); \
                  }
#define USE_LOCAL        0x01
#define CHK_TCL_SYNTAX 0x02
#define PARSE_XOR        0x04

void copyJUsage (struct jRusage *, struct jRusage *);

struct timeWindow *newTimeWindow (void);
void freeTimeWindow (struct timeWindow *);
void updateTimeWindow (struct timeWindow *);
inline int numofhosts (void);
