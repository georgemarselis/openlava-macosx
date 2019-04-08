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
  size_t jobId;
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



int readMbdStart (char *, struct mbdStartLog *);
int readMbdDie (char *, struct mbdDieLog *);
int readChkpnt (char *, struct chkpntLog *);
int readJobSigAct (char *, struct sigactLog *);
int readMig (char *, struct migLog *);
int readUnfulfill (char *, struct unfulfillLog *);
int readLoadIndex (char *, struct loadIndexLog *);

int readJobRequeue (char *, struct jobRequeueLog *);

int readJobSignal (char *, struct signalLog *);
int readJobMsg (char *, struct jobMsgLog *);
int readJobMsgAck (char *, struct jobMsgAckLog *);
int readJobClean (char *, struct jobCleanLog *);
int readLogSwitch (char *, struct logSwitchLog *);

int writeJobNew (FILE *, struct jobNewLog *);
int writeJobMod (FILE *, struct jobModLog *);
int writeJobStart (FILE *, struct jobStartLog *);
int writeJobStartAccept (FILE *, struct jobStartAcceptLog *);
int writeJobExecute (FILE *, struct jobExecuteLog *);
int writeJobStatus (FILE *, struct jobStatusLog *);
int writeSbdJobStatus (FILE *, struct sbdJobStatusLog *);
int writeJobSwitch (FILE *, struct jobSwitchLog *);
int writeJobMove (FILE *, struct jobMoveLog *);

int writeQueueCtrl (FILE *, struct queueCtrlLog *);
int writeHostCtrl (FILE *, struct hostCtrlLog *);
int writeMbdStart (FILE *, struct mbdStartLog *);
int writeMbdDie (FILE *, struct mbdDieLog *);
int writeUnfulfill (FILE *, struct unfulfillLog *);
int writeMig (FILE *, struct migLog *);
int writeChkpnt (FILE *, struct chkpntLog *);
int writeJobSigAct (FILE *, struct sigactLog *);
int writeJobFinish (FILE *, struct jobFinishLog *);
int writeLoadIndex (FILE *, struct loadIndexLog *);

int writeJobSignal (FILE * log_fp, struct signalLog *);
int writeJobMsg (FILE * log_fp, struct jobMsgLog *);
int writeJobMsgAck (FILE * log_fp, struct jobMsgLog *);
int writeJobRqueue (FILE * log_fp, struct jobRequeueLog *);
int writeJobClean (FILE * log_fp, struct jobCleanLog *);

int writeLogSwitch (FILE *, struct logSwitchLog *);
int writeJobForce (FILE *, struct jobForceRequestLog *);
int readJobForce (char *, struct jobForceRequestLog *);

int writeJobAttrSet (FILE *, struct jobAttrSetLog *);
int readJobAttrSet (char *, struct jobAttrSetLog *);

void freeLogRec (struct eventRec *);

struct eventRec *lsbGetNextJobEvent (struct eventLogHandle *ePtr, size_t *lineNum, size_t numJobIds, size_t *jobIds, struct jobIdIndexS *indexS);
struct eventRec *lsbGetNextJobRecFromFile( FILE *logFp, size_t *lineNum, size_t numJobIds, size_t *jobIds );
int checkJobEventAndJobId (char *line, int eventType, size_t numJobIds, size_t * jobIds);
int getEventTypeAndKind (char *, int *);
void readEventRecord (char *, struct eventRec *);
int lsb_readeventrecord (char *, struct eventRec *);
int getJobIdIndexFromEventFile (char *, struct sortIntList *, time_t *);
int getJobIdFromEvent (char *, int);
int writeJobIdIndexToIndexFile (FILE *, struct sortIntList *, time_t);
int updateJobIdIndexFile (char *, char *, int);
int getNextFileNumFromIndexS (struct jobIdIndexS *, int, size_t *);


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
