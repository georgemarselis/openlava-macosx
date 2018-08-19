// added by George Marselis <george@marsel.is>

#pragma once

// #define NL_SETN         10
//#define SORT_HOST_NUM   30
const unsigned short SORT_HOST_NUM = 30;

#define OUT_SCHED_RS(reason)                    \
    ((reason) == PEND_HOST_JOB_LIMIT            \
     || (reason) ==  PEND_QUEUE_JOB_LIMIT         \
     || (reason) ==  PEND_QUEUE_USR_JLIMIT        \
     || (reason) == PEND_QUEUE_PROC_JLIMIT        \
     || (reason) == PEND_QUEUE_HOST_JLIMIT        \
     || (reason) == PEND_USER_JOB_LIMIT         \
     || (reason) == PEND_UGRP_JOB_LIMIT         \
     || (reason) == PEND_USER_PROC_JLIMIT       \
     || (reason) == PEND_UGRP_PROC_JLIMIT       \
     || (reason) == PEND_HOST_USR_JLIMIT)

#define QUEUE_IS_BACKFILL(qPtr) ((qPtr)->qAttrib & QUEUE_ATTRIB_BACKFILL)

#define JOB_HAS_RUN_LIMIT(jp)                                   \
    ((jp)->shared->jobBill.rLimits[LSF_RLIMIT_RUN] > 0 ||       \
     (jp)->qPtr->rLimits[LSF_RLIMIT_RUN] > 0)

#define JOB_CAN_BACKFILL(jp) (QUEUE_IS_BACKFILL((jp)->qPtr) &&  \
                              JOB_HAS_RUN_LIMIT(jp))

#define HAS_BACKFILL_POLICY (qAttributes & QUEUE_ATTRIB_BACKFILL)

#define QUEUE_H_REASON_NOT_DUE_TO_LIMIT(qhreason)   \
    ((qhreason) != PEND_QUEUE_PROC_JLIMIT &&      \
     (qhreason) != PEND_QUEUE_HOST_JLIMIT &&      \
     (qhreason) != PEND_HOST_JOB_LIMIT)
#define HOST_UNUSABLE_TO_JOB_DUE_TO_QUEUE_H_REASON(qhreason, jp)    \
    (qhreason &&                                                \
     (QUEUE_H_REASON_NOT_DUE_TO_LIMIT(qhreason) ||                  \
      (!QUEUE_H_REASON_NOT_DUE_TO_LIMIT(qhreason) &&                \
       !QUEUE_IS_BACKFILL(jp->qPtr))))

#define HOST_UNUSABLE_DUE_TO_H_REASON(hreason)                          \
    (hreason && !(HAS_BACKFILL_POLICY && hreason == PEND_HOST_JOB_LIMIT))

#define HOST_UNUSABLE_TO_JOB_DUE_TO_H_REASON(hreason, jp)               \
    (hreason &&                                                         \
     !(QUEUE_IS_BACKFILL((jp)->qPtr) && hreason == PEND_HOST_JOB_LIMIT))


#define HOST_UNUSABLE_TO_JOB_DUE_TO_U_H_REASON(uhreason, jp)    \
    (uhreason &&                                                \
     (!QUEUE_IS_BACKFILL((jp)->qPtr) ||                         \
      (uhreason != PEND_USER_PROC_JLIMIT &&                     \
       uhreason != PEND_HOST_USR_JLIMIT)))


#define CANT_FINISH_BEFORE_DEADLINE(runLimit, deadline, cpuFactor)      \
    ((runLimit)/(cpuFactor) + now_disp > (deadline))

#define REASON_TABLE_COPY(src, dest, __func__)                             \
    {                                                                   \
        FREEUP (dest->reasonTb);                                        \
        dest->numReasons = src->numReasons;                             \
        if (dest->numReasons > 0) {                                     \
            dest->reasonTb = (int *) my_calloc (dest->numReasons,       \
                                                sizeof(int), __func__);    \
            memcpy((char *)dest->reasonTb, (char *)src->reasonTb,       \
                   sizeof(int) * dest->numReasons);                     \
        }                                                               \
    }

#define QUEUE_SCHED_DELAY(jpbw)                         \
    (((jpbw)->qPtr->schedDelay == INFINIT_INT) ?        \
     DEF_QUEUE_SCHED_DELAY : (jpbw)->qPtr->schedDelay)


#define END_OF_JOB_LIST(jp, listNum) ((jp) == jDataList[listNum])


#define HAS_JOB_LEVEL_SPAN_PTILE(jp)                    \
    ((jp)->shared->resValPtr != NULL &&                 \
     (jp)->shared->resValPtr->pTile != INFINIT_INT)

#define QUEUE_HAS_SPAN_PTILE(qPtr)              \
    ((qPtr)->resValPtr != NULL &&               \
     (qPtr)->resValPtr->pTile != INFINIT_INT)

#define HAS_QUEUE_LEVEL_SPAN_PTILE(jp) (QUEUE_HAS_SPAN_PTILE((jp)->qPtr))

#define JOB_LEVEL_SPAN_PTILE(jp) ((jp)->shared->resValPtr->pTile)
#define QUEUE_LEVEL_SPAN_PTILE(jp) ((jp)->qPtr->resValPtr->pTile)

#define HAS_JOB_LEVEL_SPAN_HOSTS(jp)            \
    ((jp)->shared->resValPtr != NULL &&         \
     (jp)->shared->resValPtr->maxNumHosts == 1)

#define QUEUE_HAS_SPAN_HOSTS(qPtr)              \
    ((qPtr)->resValPtr != NULL &&               \
     (qPtr)->resValPtr->maxNumHosts == 1)

#define HAS_QUEUE_LEVEL_SPAN_HOSTS(jp) (QUEUE_HAS_SPAN_HOSTS((jp)->qPtr))

#define HAS_JOB_LEVEL_SPAN(jp)                                          \
    (HAS_JOB_LEVEL_SPAN_PTILE(jp) || HAS_JOB_LEVEL_SPAN_HOSTS(jp))

#define HAS_QUEUE_LEVEL_SPAN(jp)                                        \
    (HAS_QUEUE_LEVEL_SPAN_PTILE(jp) || HAS_QUEUE_LEVEL_SPAN_HOSTS(jp))

#define HOST_HAS_ENOUGH_PROCS(jp, host, requestedProcs)         \
    (jobMaxUsableSlotsOnHost(jp, host) >= requestedProcs)

#define OTHERS_IS_IN_ASKED_HOST_LIST(jp) ((jp)->askedOthPrio >= 0)


#define FORALL_PRMPT_HOST_RSRCS(hostn, resn, val, jp)                   \
    if (jp->numRsrcPreemptHPtr && jp->rsrcPreemptHPtr != NULL) {        \
                                                                        \
    for (hostn = 0;                                                     \
         hostn == 0 || (slotResourceReserve &&                          \
                        hostn < jp->numRsrcPreemptHPtr);                \
         hostn++) {                                                     \
    if (jp->rsrcPreemptHPtr[hostn]->hStatus & HOST_STAT_UNAVAIL)        \
        continue;                                                       \
    FORALL_PRMPT_RSRCS(resn) {                                          \
    GET_RES_RSRC_USAGE(resn, val, jp->shared->resValPtr,                \
                       jp->qPtr->resValPtr);                            \
    if (val <= 0.0)                                                     \
        continue;

#define ENDFORALL_PRMPT_HOST_RSRCS } ENDFORALL_PRMPT_RSRCS;             \
    }                                                                   \
                                                                    }

#define CANNOT_BE_PREEMPTED_FOR_RSRC(s) ( (s->jFlags & JFLAG_URGENT) || \
                                          (s->jFlags & JFLAG_URGENT_NOSTOP) || \
                                          (s->jStatus & JOB_STAT_UNKWN))


#define DUMP_TIMERS(__func__)                                              \
    {                                                                   \
        if (logclass & LC_PERFM)                                        \
            ls_syslog(LOG_DEBUG,"\
%s timeGetQUsable %d ms timeGetCandHosts %d ms \
timeGetJUsable %d ms timeReadyToDisp %d ms timeCntUQSlots %d ms \
timeFSQelectPendJob %d ms timePickAJob %d ms timeScheduleAJob %d ms \
timeCollectPendReason %dms",                                            \
                      __func__,                                            \
                      timeGetQUsable,                                   \
                      timeGetCandHosts,                                 \
                      timeGetJUsable,                                   \
                      timeReadyToDisp,                                  \
                      timeCntUQSlots,                                   \
                      timeFSQelectPendJob,                              \
                      timePickAJob,                                     \
                      timeScheduleAJob,                                 \
                      timeCollectPendReason);                           \
    }

#define ZERO_OUT_TIMERS()                       \
    {                                           \
        timeGetJUsable      = 0;                \
        timeGetCandHosts    = 0;                \
        timeGetQUsable      = 0;                \
        timeReadyToDisp     = 0;                \
        timeCntUQSlots      = 0;                \
        timeFSQelectPendJob = 0;                \
        timePickAJob        = 0;                \
        timeScheduleAJob    = 0;                \
        timeFindBestHosts   = 0;                \
        timeHostPreference  = 0;                \
        timeHostJobLimitOk1 = 0;                \
        timeHJobLimitOk     = 0;                \
        timePJobLimitOk     = 0;                \
        timeCollectPendReason = 0;              \
    }

bool_t updateAccountsInQueue;

void resetSchedulerSession (void);

int reservePreemptResourcesForExecCands (struct jData *jp);
int reservePreemptResources (struct jData *jp, int numHosts, struct hData **hosts);

int readyToDisp (struct jData *jpbw, int *numAvailSlots);
enum candRetCode getCandHosts (struct jData *);
int getLsbUsable (void);
struct candHost *getJUsable (struct jData *, int *, int *);
void addReason (struct jData *jp, int hostId, int aReason);
int allInOne (struct jData *jp);
int ckResReserve (struct hData *hD, struct resVal *resValPtr, int *resource, struct jData *jp);

int getPeerCand (struct jData *jobp);
int getPeerCand1 (struct jData *jobp, struct jData *jpbw);
void copyPeerCand (struct jData *jobp, struct jData *jpbw);
void reserveSlots (struct jData *);

int cntUQSlots (struct jData *jpbw, int *numAvailSlots);
int ckPerHULimits (struct qData *, struct hData *, struct uData *, int *numAvailSlots, int *reason);
int getHostJobSlots (struct jData *, struct hData *, int *, int, LIST_T **);
int getHostJobSlots1 (int, struct jData *, struct hData *, int *, int);

int cntUserJobs (struct jData *, struct gData *, struct hData *, int *, int *, int *, int *);

int candHostOk (struct jData *jp, int indx, int *numAvailSlots, int *hReason);
int allocHosts (struct jData *jp);
int deallocHosts (struct jData *jp);
void jobStarted (struct jData *, struct jobReply *);
void disp_clean (void);
int overThreshold (float *load, float *thresh, int *reason);

void hostPreference (struct jData *, int);
void hostPreference1 (struct jData *, int, struct askedHost *, int, int, int *, int);
int sortHosts (int, int, int, struct candHost *, int, float, bool_t);
int notOrdered (int, int, float, float, float, float);
int cntUserSlots (struct hTab *, struct uData *, int *);
void checkSlotReserve (struct jData **, int *);
int cntHostSlots (struct hTab *, struct hData *);
void jobStartTime (struct jData *jp);
int isAskedHost (struct hData *hData, struct jData *jp);

void moveHostPos (struct candHost *, int, int);

// extern int scheduleAndDispatchJobs (void);

int checkIfJobIsReady (struct jData *);
int scheduleAJob (struct jData *, bool_t, bool_t);
enum dispatchAJobReturnCode dispatchAJob (struct jData *, int);
enum dispatchAJobReturnCode dispatchAJob0 (struct jData *, int);
enum candRetCode checkIfCandHostIsOk (struct jData *);
void getNumSlots (struct jData *);
enum dispatchAJobReturnCode dispatchToCandHost (struct jData *);
void getNumProcs (struct jData *);
void removeCandHost (struct jData *, int);
bool_t schedulerObserverSelect (void *, LIST_EVENT_T *);
int schedulerObserverEnter (LIST_T *, void *, LIST_EVENT_T *);
int schedulerObserverLeave (LIST_T *, void *, LIST_EVENT_T *);
int j1IsBeforeJ2 (struct jData *, struct jData *, struct jData *);
int queueObserverEnter (LIST_T *, void *, LIST_EVENT_T *);
int queueObserverLeave (LIST_T *, void *, LIST_EVENT_T *);
int listNumber (struct jData *);
int jobIsFirstOnSegment (struct jData *, struct jData *);
int jobIsLastOnSegment (struct jData *, struct jData *);
int jobsOnSameSegment (struct jData *, struct jData *, struct jData *);
struct jData *nextJobOnSegment (struct jData *, struct jData *);
struct jData *prevJobOnSegment (struct jData *, struct jData *);
void setQueueFirstAndLastJob (struct qData *, int);
int numOfOccuranceOfHost (struct jData *, struct hData *);
void removeNOccuranceOfHost (struct jData *, struct hData *, int, struct hData **);
struct backfillee *backfilleeCreate (void);
struct backfillee *backfilleeCreateByCopy (struct backfillee *);
void sortBackfillee (struct jData *, LIST_T *);
void insertIntoSortedBackfilleeList (struct jData *, LIST_T *, struct backfillee *);
int jobHasBackfillee (struct jData *);
int candHostInBackfillCandList (struct backfillCand *, int, int);
void freeBackfillSlotsFromBackfillee (struct jData *);
bool_t backfilleeDataCmp (void *, void *, int);
void removeBackfillSlotsFromBackfiller (struct jData *);
void getBackfillSlotsOnExecCandHost (struct jData *);
void doBackfill (struct jData *);
void deallocExecCandPtr (struct jData *);
int jobCantFinshBeforeDeadline (struct jData *, time_t);
void copyCandHosts (int, struct askedHost *, struct candHost *, int *, struct jData *, int, int, int *);
void copyCandHostData (struct candHost *, struct candHost *);
int noPreference (struct askedHost *, int, int);
int imposeDCSOnJob (struct jData *, time_t *, int *, int *);
void updateQueueJobPtr (int, struct qData *);
void copyReason (void);
void clearJobReason (void);
int isInCandList (struct candHost *, struct hData *, int);
bool_t enoughMaxUsableSlots (struct jData *);
int jobMaxUsableSlotsOnHost (struct jData *, struct hData *);
void hostHasEnoughSlots (struct jData *, struct hData *, int, int, int, int *);
void checkHostUsableToSpan (struct jData *, struct hData *, int, int *, int *);
void reshapeCandHost (struct jData *, struct candHost *, int *);
void getSlotsUsableToSpan (struct jData *, struct hData *, int, int *);
void exchangeHostPos (struct candHost *, int, int);
int notDefaultOrder (struct resVal *);
int isQAskedHost (struct hData *, struct jData *);
int totalBackfillSlots (LIST_T *);
float getNumericLoadValue (const struct hData *hp, int lidx);
void getRawLsbLoad (int, struct candHost *);
int handleFirstHost (struct jData *, int, struct candHost *);
int needHandleFirstHost (struct jData *);
bool_t jobIsReady (struct jData *);
bool_t isCandHost (char *, struct jData *);
void updPreemptResourceByRUNJob (struct jData *);
void checkAndReserveForPreemptWait (struct jData *);
int markPreemptForPRHQValues (struct resVal *, int, struct hData **, struct qData *);
int markPreemptForPRHQInstance (int needResN, float needVal, struct hData *needHost, struct qData *needQPtr);

int needHandleXor (struct jData *);
enum candRetCode handleXor (struct jData *);
enum candRetCode XORCheckIfCandHostIsOk (struct jData *);
enum dispatchAJobReturnCode XORDispatch (struct jData *, int, enum dispatchAJobReturnCode (*)(struct jData *, int));
void copyCandHostPtr (struct candHost **, struct candHost **, int *, int *);
void removeCandHostFromCandPtr (struct candHost **, int *, int i);
void groupCandsCopy (struct jData *dest, struct jData *src);
void groupCandHostsCopy (struct groupCandHosts *dest, struct groupCandHosts *src);
void groupCandHostsInit (struct groupCandHosts *gc);
void inEligibleGroupsInit (int **inEligibleGroups, int numGroups);
void groupCands2CandPtr (int numOfGroups, struct groupCandHosts *gc, int *numCandPtr, struct candHost **candPtr);


enum candRetCode
{
	CAND_NO_HOST,
	CAND_HOST_FOUND,
	CAND_FIRST_RES
};

enum dispatchAJobReturnCode
{
	DISP_OK,
	DISP_FAIL,
	DISP_RESERVE,
	DISP_NO_JOB,
	DISP_TIME_OUT
};

struct backfillee
{
	struct backfillee *forw;
	struct backfillee *back;
	struct jData *backfilleePtr;
	int indexInCandHostList;
	int backfillSlots;

	int numHostPtr;
	struct hData **hPtr;
};

struct backfillCand
{
	int numSlots;
	int numAvailSlots;
	int indexInCandHostList;
	LIST_T *backfilleeList;
};

struct backfilleeData
{
	struct backfilleeData *forw;
	struct backfilleeData *back;
	struct jData *backfilleePtr;
	LIST_T *slotsList;
};

struct leftTimeTable
{
	int leftTime;
	int slots;
};
/* openlava round robin
 */
struct jRef
{
	struct jRef *forw;
	struct jRef *back;
	struct jData *job;
};

bool_t lsbPtilePack = FALSE;

struct _list *jRefList = NULL;
struct jData *currentHPJob = NULL;
float bThresholds[] =  { 0.1, 0.1, 0.1, 0.1, 5.0, 5.0, 1.0, 5.0, 2.0, 2.0, 3.0 };


struct jData *currentJob[PJL + 1];


#undef MBD_PROF_COUNTER
#define MBD_PROF_COUNTER(Func) { 0, #Func },

struct profileCounters counters[] = {
#   include "mbd.profcnt.def"
	{-1, (char *) NULL}
};

time_t now_disp = 0;
int newSession[PJL + 1];
int numLsbUsable = 0;
int freedSomeReserveSlot = 0;
int timeGetJUsable = 0;
int timeGetQUsable = 0;
int timeGetCandHosts = 0;
int timeReadyToDisp = 0;
int timeCntUQSlots = 0;
int timeFSQelectPendJob = 0;
int timePickAJob = 0;
int timeScheduleAJob = 0;
int timeFindBestHosts = 0;
int timeHostPreference = 0;
int timeHostJobLimitOk1 = 0;
int timeHJobLimitOk = 0;
int timePJobLimitOk = 0;
int timeCollectPendReason = 0;
