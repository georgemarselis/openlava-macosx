// added by George Marselis <george@marsel.is

#pragma once

// #define NL_SETN  6
#define GET_JOBID(jobId, idx) ((Req.options & OPT_ARRAY_INFO)) ? (jobId): LSB_JOBID((jobId), (idx))

// extern void initTab (struct hTab *tabPtr);
// extern hEnt *addMemb (struct hTab *tabPtr, LS_LONG_INT member);
// extern char remvMemb (struct hTab *tabPtr, LS_LONG_INT member);
// extern int matchName (char *, char *);

struct eventRecord *lastEvent = NULL;

void inJobList (struct jobRecord *pred, struct jobRecord *entry);
void offJobList (struct jobRecord *entry);
void insertModEvent (struct eventRec *log, hEnt * ent);
struct jobRecord *createJobRec (int);

// extern struct jobRecord *jobRecordList;
// extern struct eventLogHandle *eLogPtr;
// extern float version;
