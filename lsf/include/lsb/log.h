/* $Id: lsb.log.h 397 2007-11-26 19:04:00Z mblack $
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
 
#include <string.h>
#include <stdlib.h>

#include "lsb/lsb.h"
#include "lsb/lsbatch.h"

// #define MAXEVENTNAMELEN   128
// const size_t MAXEVENTNAMELEN = 128


/*struct chkpntLog
{
  LS_LONG_INT jobId;
  int chkperiod;
};
*/

/*union eventLog
{
  struct newJobLog newJobLog;
  struct startJobLog startJobLog;
  struct newStatusLog newStatusLog;
  struct qControlLog qControlLog;
  struct switchJobLog switchJobLog;
  struct moveJobLog moveJobLog;
  struct paramsLog paramsLog;
  struct chkpntLog chkpntLog;
  struct finishJobLog finishJobLog;
};*/



static int readMbdStart (char *, struct mbdStartLog *);
static int readMbdDie (char *, struct mbdDieLog *);
static int readChkpnt (char *, struct chkpntLog *);
static int readJobSigAct (char *, struct sigactLog *);
static int readMig (char *, struct migLog *);
static int readUnfulfill (char *, struct unfulfillLog *);
static int readLoadIndex (char *, struct loadIndexLog *);

static int readJobRequeue (char *, struct jobRequeueLog *);

static int readJobSignal (char *, struct signalLog *);
static int readJobMsg (char *, struct jobMsgLog *);
static int readJobMsgAck (char *, struct jobMsgAckLog *);
static int readJobClean (char *, struct jobCleanLog *);
static int readLogSwitch (char *, struct logSwitchLog *);

static int writeJobNew (FILE *, struct jobNewLog *);
static int writeJobMod (FILE *, struct jobModLog *);
static int writeJobStart (FILE *, struct jobStartLog *);
static int writeJobStartAccept (FILE *, struct jobStartAcceptLog *);
static int writeJobExecute (FILE *, struct jobExecuteLog *);
static int writeJobStatus (FILE *, struct jobStatusLog *);
static int writeSbdJobStatus (FILE *, struct sbdJobStatusLog *);
static int writeJobSwitch (FILE *, struct jobSwitchLog *);
static int writeJobMove (FILE *, struct jobMoveLog *);

static int writeQueueCtrl (FILE *, struct queueCtrlLog *);
static int writeHostCtrl (FILE *, struct hostCtrlLog *);
static int writeMbdStart (FILE *, struct mbdStartLog *);
static int writeMbdDie (FILE *, struct mbdDieLog *);
static int writeUnfulfill (FILE *, struct unfulfillLog *);
static int writeMig (FILE *, struct migLog *);
static int writeChkpnt (FILE *, struct chkpntLog *);
static int writeJobSigAct (FILE *, struct sigactLog *);
static int writeJobFinish (FILE *, struct jobFinishLog *);
static int writeLoadIndex (FILE *, struct loadIndexLog *);

static int writeJobSignal (FILE * log_fp, struct signalLog *);
static int writeJobMsg (FILE * log_fp, struct jobMsgLog *);
static int writeJobMsgAck (FILE * log_fp, struct jobMsgLog *);
static int writeJobRqueue (FILE * log_fp, struct jobRequeueLog *);
static int writeJobClean (FILE * log_fp, struct jobCleanLog *);

static int writeLogSwitch (FILE *, struct logSwitchLog *);
static int writeJobForce (FILE *, struct jobForceRequestLog *);
static int readJobForce (char *, struct jobForceRequestLog *);

static int writeJobAttrSet (FILE *, struct jobAttrSetLog *);
static int readJobAttrSet (char *, struct jobAttrSetLog *);

static void freeLogRec (struct eventRec *);

struct eventRec *lsbGetNextJobEvent (struct eventLogHandle *ePtr, size_t *lineNum, size_t numJobIds, LS_LONG_INT *jobIds, struct jobIdIndexS *indexS);
static struct eventRec *lsbGetNextJobRecFromFile( FILE *logFp, size_t *lineNum, size_t numJobIds, LS_LONG_INT *jobIds );
int checkJobEventAndJobId (char *line, int eventType, size_t numJobIds, LS_LONG_INT * jobIds);
static int getEventTypeAndKind (char *, int *);
static void readEventRecord (char *, struct eventRec *);
int lsb_readeventrecord (char *, struct eventRec *);
int getJobIdIndexFromEventFile (char *, struct sortIntList *, time_t *);
int getJobIdFromEvent (char *, int);
int writeJobIdIndexToIndexFile (FILE *, struct sortIntList *, time_t);
int updateJobIdIndexFile (char *, char *, int);
int getNextFileNumFromIndexS (struct jobIdIndexS *, int, LS_LONG_INT *);


struct eventLogHandle *lsb_openelog (struct eventLogFile *, size_t *lineNum);
struct eventRec *lsb_getelogrec (struct eventLogHandle *, size_t *lineNum);
void lsb_closeelog (struct eventLogHandle *);
void countLineNum (FILE *, long, size_t *lineNum);

struct eventRec *lsb_geteventrec (FILE *log_fp, size_t *lineNum);
struct eventRec *lsb_geteventrec_ex (FILE * log_fp, size_t *lineNum, char *usedLine);
time_t lsb_getAcctFileTime (char *fileName);

#define   EVENT_JOB_RELATED     1
#define   EVENT_NON_JOB_RELATED 0


size_t copyQStr(char *line, size_t maxLen, int nonNil, char *destStr);
size_t saveQStr( char *line, char *destStr);

/*#define copyQStr(line, maxLen, nonNil, destStr)    {            \
        char *tmpLine=0;                                        \
        int ccounter=0;                                           \
        if ((tmpLine = (char *) malloc (strlen(line))) == NULL) \
            return (LSBE_NO_MEM);                               \
        if ((ccounter = stripQStr(line, tmpLine)) < 0) {          \
            FREEUP (tmpLine);                                   \
            return (LSBE_EVENT_FORMAT);                         \
        }                                                       \
        line += ccounter + 1;                                     \
        if (strlen(tmpLine) >= maxLen                           \
            || (nonNil && strlen(tmpLine)==0)) {                \
            FREEUP (tmpLine);                                   \
            return (LSBE_EVENT_FORMAT);                         \
        }                                                       \
        strcpy(destStr, tmpLine);                               \
        FREEUP (tmpLine);                                       \
    }

#define saveQStr(line, destStr)  {                              \
        char *tmpLine;                                          \
        int ccounter;                                             \
        if ((tmpLine = (char *) malloc (strlen(line))) == NULL) \
            return (LSBE_NO_MEM);                               \
        if ((ccounter = stripQStr(line, tmpLine)) < 0)  {         \
            FREEUP (tmpLine);                                   \
            return (LSBE_EVENT_FORMAT);                         \
        }                                                       \
        line += ccounter + 1;                                     \
        if ((destStr = putstr_(tmpLine)) == NULL)     {         \
            FREEUP (tmpLine);                                   \
            return (LSBE_NO_MEM);                               \
        }                                                       \
        FREEUP (tmpLine);                                       \
    }
*/

int putEventRec (FILE *, struct eventRec *);
struct eventRec *getEventRec (char *);
char *getNextValue0 (char **line, char, char);

long lsb_array_jobid( long jobId );
