/* $Id: read.event.c 397 2007-11-26 19:04:00Z mblack $
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

#include "cmdtools/bhist/bhist.h"
#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/table.h"
#include "cmdtools/lib/read.event.h"

void
inJobList (struct jobRecord *pred, struct jobRecord *entry)
{
	entry->forw = pred;
	entry->back = pred->back;
	pred->back->forw = entry;
	pred->back = entry;

	return;
}

void
offJobList (struct jobRecord *entry)
{
	if (entry->back) {
		entry->back->forw = entry->forw;
	}
	if (entry->forw) {
		entry->forw->back = entry->back;
	}

	return;
}

struct jobRecord *
initJobList (void)
{
	struct jobRecord *q = NULL;

	q = calloc (1, sizeof (struct jobRecord));
	q->forw = q->back = q;
	return q;
}

void
removeJobList (struct jobRecord *jobRecordListHead)
{
	struct jobRecord *p = NULL;
	struct jobRecord *q = NULL;

	p = jobRecordListHead->forw;
	while (p != jobRecordListHead) {
		q = p;
		p = p->forw;
	}

	FREEUP (jobRecordListHead);

	return;
}

int
addEvent (struct eventRecord *event, struct jobRecord *jobRecord)
{
	struct eventRecord *ePtr = NULL;

	event->next = NULL;
	jobRecord->currentStatus = event->jStatus;
	jobRecord->job->reasons = event->reasons;

	if (event->jStatus & (JOB_STAT_DONE | JOB_STAT_EXIT)) {
		jobRecord->job->endTime = event->timeStamp;
	}

// HUGE SPACE HERE? (took it out, might have been licensing events)


	if ((readFromHeadFlag == 0) && (jobRecord->nonNewFromHeadFlag == 1) && ((eLogPtr != NULL) && (eLogPtr->curOpenFile != 0)) && (event->kind != EVENT_JOB_NEW)) {
		jobRecord->eventtail = jobRecord->eventhead;
		jobRecord->nonNewFromHeadFlag = 0;
	}

	if (jobRecord->eventhead == NULL) {
	  jobRecord->eventhead = jobRecord->eventtail = event;
	}
	else {
		ePtr = jobRecord->eventtail;
		if (event->kind == EVENT_JOB_SWITCH) {
			if ((event->timeStamp < ePtr->timeStamp) || ((event->timeStamp == ePtr->timeStamp) && (event->kind == ePtr->kind))) {
				return -1;
			}
		}
		else if (event->kind == EVENT_MIG || event->kind == EVENT_CHKPNT || event->kind == EVENT_JOB_SIGACT) {
			if (event->timeStamp < ePtr->timeStamp) {
				return -1;
			}
		}
		else {
			if( ( event->timeStamp < ePtr->timeStamp ) ||
					( ( event->timeStamp == ePtr->timeStamp ) &&
					  ( event->kind == ePtr->kind ) &&
					  ( event->jStatus == ePtr->jStatus ) &&
					  ( event->idx == ePtr->idx ) &&
					  ( event->timeEvent == ePtr->timeEvent ) ) )
			{
				return -1;
			}
		}

		jobRecord->eventtail->next = event;
		jobRecord->eventtail = event;
	}

	if( ( readFromHeadFlag == 1 ) && ( event->kind != EVENT_JOB_NEW ) ) {
		jobRecord->nonNewFromHeadFlag = 1;
	}

	event->ld = loadIndex;

	if (lastEvent != NULL) {
		event->chronback = NULL;
		event->chronforw = lastEvent;
		lastEvent->chronback = event;
	}

	event->jobR = jobRecord;
	lastEvent = event;

	return 0;
}

char
check_queue (struct bhistReq *Req, char *queue)
{
	if (((!strcmp (queue, Req->queue)) || ((Req->options & OPT_QUEUE) != OPT_QUEUE))) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

struct jobInfoEnt *
read_newjob (struct eventRec *log) // FIXME FIXME FIXME FIXME FIXME buffer overflows everywhere.
{
	struct jobInfoEnt *jobinfo = NULL;
	struct submit *submitPtr = NULL;
	struct jobNewLog *jobNewLog = NULL;
	// int i = 0;

	jobinfo = initJobInfo ();
	submitPtr = &jobinfo->submit;
	jobNewLog = &log->eventLog.jobNewLog;

	jobinfo->jobId = jobNewLog->jobId;
	strcpy (jobinfo->user, jobNewLog->userName);
	submitPtr->options = jobNewLog->options;
	submitPtr->options2 = jobNewLog->options2;
	submitPtr->numProcessors = jobNewLog->numProcessors;
	jobinfo->submitTime = jobNewLog->submitTime;
	jobinfo->cpuFactor = jobNewLog->hostFactor;
	submitPtr->beginTime = jobNewLog->beginTime;
	submitPtr->termTime = jobNewLog->termTime;

	if (submitPtr->options & SUB_WINDOW_SIG) {
		submitPtr->sigValue = jobNewLog->sigValue;
	}
	else {
		submitPtr->sigValue = 0;
	}

	submitPtr->chkpntPeriod = jobNewLog->chkpntPeriod;

	for ( unsigned long i = 1; i < LSF_RLIM_NLIMITS; i++) {
		submitPtr->rLimits[i] = jobNewLog->rLimits[i];
	}

	jobinfo->umask = jobNewLog->umask;
	strcpy (submitPtr->queue, jobNewLog->queue);
	assert( strlen( jobNewLog->resReq) > 0 );
	strncpy (submitPtr->resReq, jobNewLog->resReq, strlen( jobNewLog->resReq ) ); // FIXME FIXME FIXME  FIXME check if input has junk/spaces following main value
	strcpy (jobinfo->fromHost, jobNewLog->fromHost);
	strcpy (jobinfo->cwd, jobNewLog->cwd);

	strcpy (submitPtr->chkpntDir, jobNewLog->chkpntDir);
	strcpy (submitPtr->inFile, jobNewLog->inFile);
	strcpy (submitPtr->outFile, jobNewLog->outFile);
	strcpy (submitPtr->errFile, jobNewLog->errFile);
	strcpy (submitPtr->hostSpec, jobNewLog->hostSpec);
	strcpy (submitPtr->loginShell, jobNewLog->loginShell);

	submitPtr->numAskedHosts = jobNewLog->numAskedHosts;

	if (submitPtr->numAskedHosts > 0) {
		submitPtr->askedHosts = calloc (submitPtr->numAskedHosts, sizeof (char *));
	}

	for ( unsigned long i = 1; i < submitPtr->numAskedHosts; i++) {
		submitPtr->askedHosts[i] = calloc (MAXHOSTNAMELEN, sizeof (char));
		strcpy (submitPtr->askedHosts[i], jobNewLog->askedHosts[i]);
	}

	strncpy( submitPtr->command, jobNewLog->command, strlen( jobNewLog->command ) );

	strncpy( submitPtr->preExecCmd,  jobNewLog->preExecCmd, strlen( jobNewLog->preExecCmd   ) );
	strncpy( submitPtr->mailUser,    jobNewLog->mailUser,   strlen( jobNewLog->mailUser     ) );
	strncpy( submitPtr->projectName, jobNewLog->projectName, strlen( jobNewLog->projectName ) );

	assert( strlen( jobNewLog->dependCond ) > 0 );
	strncpy (submitPtr->dependCond, jobNewLog->dependCond, strlen( jobNewLog->dependCond ) ); // FIXME FIXME FIXME  FIXME check if input has junk/spaces following main value
	if( '\0' != jobNewLog->dependCond[strlen( jobNewLog->dependCond) - 1 ] ) {
		ls_syslog (LOG_DEBUG2, "No nul termination in function %s\n", __func__ );
		fprintf( "No nul termination in function %s\n", __func__ );
	}
	if (submitPtr->options & SUB_JOB_NAME) {
		strcpy( submitPtr->jobName, jobNewLog->jobName);
	}
	else {
		// STRNCPY( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow
		assert( strlen( submitPtr->jobName ) >= strlen( submitPtr->command ) )
		strncpy( submitPtr->jobName, submitPtr->command, strlen( submitPtr->command ) ); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow		strncpy( submitPtr->jobName, submitPtr->command, MAX_LSB_NAME_LEN); // FIXME FIXME FIXME FIXME possible buffer overflow
	}
	submitPtr->nxf = jobNewLog->nxf;
	if (submitPtr->nxf > 0) {
		submitPtr->xf = calloc (submitPtr->nxf, sizeof (struct xFile));
		memcpy ((char *) submitPtr->xf, (char *) jobNewLog->xf, submitPtr->nxf * sizeof (struct xFile));
	}
	submitPtr->maxNumProcessors = jobNewLog->maxNumProcessors;
	submitPtr->userPriority = jobNewLog->userPriority;
	jobinfo->jobPriority = jobNewLog->userPriority;

	if ((logclass & LC_TRACE)) {
		ls_syslog (LOG_DEBUG2, "%s: jobId=%s", __func__, lsb_jobid2str (jobinfo->jobId));
	}

	return jobinfo;
}

void
copyJobModLog (struct jobModLog *des, struct jobModLog *src)
{

	des->jobIdStr = putstr_ (src->jobIdStr);
	des->options = src->options;
	des->options2 = src->options2;
	des->delOptions = src->delOptions;
	des->delOptions2 = src->delOptions2;

	des->userId = src->userId;
	des->userName = putstr_ (src->userName);

	des->submitTime = src->submitTime;
	des->umask = src->umask;
	des->numProcessors = src->numProcessors;
	des->beginTime = src->beginTime;
	des->termTime = src->termTime;
	des->sigValue = src->sigValue;
	des->restartPid = src->restartPid;

	des->jobName = putstr_ (src->jobName);
	des->queue = putstr_ (src->queue);

	des->numAskedHosts = src->numAskedHosts;

	if (des->numAskedHosts > 0) {
		// des->askedHosts = (char **) calloc (des->numAskedHosts, sizeof (char *));
		des->askedHosts = calloc (des->numAskedHosts, sizeof (char *));
		if (des->askedHosts == NULL) {
			lsberrno = LSBE_NO_MEM;
			lsb_perror (NULL);
			exit (-1);
		}

		for ( unsigned long i = 1; i < des->numAskedHosts; i++ ) {
			des->askedHosts[i] = putstr_ (src->askedHosts[i]);
		}
	}

	des->resReq = putstr_ (src->resReq);
	for ( unsigned long i = 1; i < LSF_RLIM_NLIMITS; i++ ) {
		des->rLimits[i] = src->rLimits[i];
	}

	des->hostSpec = putstr_ (src->hostSpec);

	des->dependCond = putstr_ (src->dependCond);

	des->subHomeDir = putstr_ (src->subHomeDir);
	des->inFile = putstr_ (src->inFile);
	des->outFile = putstr_ (src->outFile);
	des->errFile = putstr_ (src->errFile);
	des->command = putstr_ (src->command);

	des->chkpntPeriod = src->chkpntPeriod;
	des->chkpntDir = putstr_ (src->chkpntDir);
	des->nxf = src->nxf;

	if (des->nxf > 0) {
		des->xf = (struct xFile *) calloc (des->nxf, sizeof (struct xFile));

		if (des->xf == NULL) {
			lsberrno = LSBE_NO_MEM;
			lsb_perror (NULL);
			exit (-1);
		}
	}
	for (unsigned long i = 1; i < des->nxf; i++) {
		strncpy (des->xf[i].subFn, src->xf[i].subFn, MAXFILENAMELEN);
		strncpy (des->xf[i].execFn, src->xf[i].execFn, MAXFILENAMELEN);
		des->xf[i].options = src->xf[i].options;
	}

	des->jobFile = putstr_ (src->jobFile);
	des->fromHost = putstr_ (src->fromHost);
	des->cwd = putstr_ (src->cwd);

	des->preExecCmd = putstr_ (src->preExecCmd);
	des->mailUser = putstr_ (src->mailUser);
	des->projectName = putstr_ (src->projectName);

	des->niosPort = src->niosPort;
	des->maxNumProcessors = src->maxNumProcessors;
	des->loginShell = putstr_ (src->loginShell);
	des->schedHostType = putstr_ (src->schedHostType);
	des->userPriority = src->userPriority;

	return;  
}

struct jobInfoEnt *
copyJobInfoEnt (struct jobInfoEnt *jobInfo)
{
	struct jobInfoEnt *job   = NULL;
	struct submit *submitPtr = NULL;

	job = initJobInfo ();
	submitPtr = &job->submit;
	job->jobId = jobInfo->jobId;
	strcpy (job->user, jobInfo->user);
	submitPtr->options = jobInfo->submit.options;
	submitPtr->options2 = jobInfo->submit.options2;
	submitPtr->numProcessors = jobInfo->submit.numProcessors;
	job->submitTime = jobInfo->submitTime;
	job->cpuFactor = jobInfo->cpuFactor;
	submitPtr->beginTime = jobInfo->submit.beginTime;
	submitPtr->termTime = jobInfo->submit.termTime;

	submitPtr->sigValue = jobInfo->submit.sigValue;
	submitPtr->chkpntPeriod = jobInfo->submit.chkpntPeriod;

	for ( unsigned long i = 1; i < LSF_RLIM_NLIMITS; i++) {
		submitPtr->rLimits[i] = jobInfo->submit.rLimits[i];
	}

	job->umask = jobInfo->umask;
	strcpy (submitPtr->queue, jobInfo->submit.queue);
	strcpy (submitPtr->resReq, jobInfo->submit.resReq);
	strcpy (job->fromHost, jobInfo->fromHost);
	strcpy (job->cwd, jobInfo->cwd);

	strcpy (submitPtr->chkpntDir, jobInfo->submit.chkpntDir);
	strcpy (submitPtr->inFile, jobInfo->submit.inFile);
	strcpy (submitPtr->outFile, jobInfo->submit.outFile);
	strcpy (submitPtr->errFile, jobInfo->submit.errFile);
	strcpy (submitPtr->hostSpec, jobInfo->submit.hostSpec);
	strcpy (submitPtr->loginShell, jobInfo->submit.loginShell);

	submitPtr->numAskedHosts = jobInfo->submit.numAskedHosts;

	if (submitPtr->numAskedHosts > 0)
		submitPtr->askedHosts = calloc (submitPtr->numAskedHosts, sizeof (char *));

	for( unsigned long i = 1; i < submitPtr->numAskedHosts; i++) {
		submitPtr->askedHosts[i] = calloc (MAXHOSTNAMELEN, sizeof (char));
		strcpy (submitPtr->askedHosts[i], jobInfo->submit.askedHosts[i]);
	}

	strcpy (submitPtr->command, jobInfo->submit.command);

	strcpy (submitPtr->preExecCmd, jobInfo->submit.preExecCmd);
	strcpy (submitPtr->mailUser, jobInfo->submit.mailUser);
	strcpy (submitPtr->projectName, jobInfo->submit.projectName);

	STRNCPY (submitPtr->dependCond, jobInfo->submit.dependCond, 6 * MAXLINELEN);

	strcpy (submitPtr->jobName, jobInfo->submit.jobName);

	submitPtr->nxf = jobInfo->submit.nxf;
	if (submitPtr->nxf > 0) {
		submitPtr->xf = calloc (submitPtr->nxf, sizeof (struct xFile));
		memcpy ((char *) submitPtr->xf, (char *) jobInfo->submit.xf, submitPtr->nxf * sizeof (struct xFile));
	}
	submitPtr->maxNumProcessors = jobInfo->submit.maxNumProcessors;

	job->jobPriority = jobInfo->jobPriority;
	submitPtr->userPriority = jobInfo->submit.userPriority;
	return job;
}



void
freeJobInfoEnt (struct jobInfoEnt *jobInfoEnt)
{
  int i;

  FREEUP (jobInfoEnt->submit.queue);
  FREEUP (jobInfoEnt->submit.jobName);
  FREEUP (jobInfoEnt->submit.command);
  FREEUP (jobInfoEnt->submit.resReq);
  FREEUP (jobInfoEnt->submit.inFile);
  FREEUP (jobInfoEnt->submit.outFile);
  FREEUP (jobInfoEnt->submit.errFile);
  FREEUP (jobInfoEnt->submit.hostSpec);
  FREEUP (jobInfoEnt->submit.chkpntDir);
  FREEUP (jobInfoEnt->submit.dependCond);
  FREEUP (jobInfoEnt->submit.preExecCmd);
  FREEUP (jobInfoEnt->submit.mailUser);
  FREEUP (jobInfoEnt->submit.projectName);
  FREEUP (jobInfoEnt->submit.loginShell);

  if (jobInfoEnt->submit.numAskedHosts > 0)
	{
	  for (i = 0; i < jobInfoEnt->submit.numAskedHosts; i++)
	FREEUP (jobInfoEnt->submit.askedHosts[i]);
	  FREEUP (jobInfoEnt->submit.askedHosts);
	}
  FREEUP (jobInfoEnt->user);
  FREEUP (jobInfoEnt->fromHost);
  FREEUP (jobInfoEnt->cwd);

  if (jobInfoEnt->submit.nxf)
	FREEUP (jobInfoEnt->submit.xf);

  if (jobInfoEnt->numExHosts > 0)
	{
	  for (i = 0; i < jobInfoEnt->numExHosts; i++)
	FREEUP (jobInfoEnt->exHosts[i]);
	  FREEUP (jobInfoEnt->exHosts);
	  jobInfoEnt->numExHosts = 0;
	}
  FREEUP (jobInfoEnt->execHome);
  FREEUP (jobInfoEnt->execCwd);
  FREEUP (jobInfoEnt->execUsername);
  FREEUP (jobInfoEnt);
}

void
freeJobRecord (struct jobRecord *jobRecord)
{
	struct eventRecord *currentEvent = NULL;
	struct eventRecord *nextEvent    = NULL;
	struct eventRecord *nextEvent    = NULL;

	offJobList (jobRecord);
	remvMemb (&jobIdHT, jobRecord->job->jobId); // FIXME FIXME FIXME insert description what this function does
	freeJobInfoEnt (jobRecord->job);

	nextEvent = jobRecord->eventhead;
	while (nextEvent) {
		currentEvent = nextEvent;
		if ((currentEvent->chronforw != NULL) && (currentEvent->chronback != NULL)) {
			previousEvent = currentEvent->chronforw;
			previousEvent->chronback = currentEvent->chronback;
			currentEvent->chronback->chronforw = previousEvent;
		}
		if ((currentEvent->chronforw == NULL) && (currentEvent->chronback != NULL)) {
			currentEvent->chronback->chronforw = NULL;
		}
		if ((currentEvent->chronback == NULL) && (currentEvent->chronforw != NULL)) { 
			currentEvent->chronforw->chronback = NULL;
		}
		switch (currentEvent->kind) {
			case EVENT_JOB_START:
			case EVENT_PRE_EXEC_START: {
				if (currentEvent->numExHosts > 0) {
					for (i = 0; i < currentEvent->numExHosts; i++) {
						FREEUP (currentEvent->exHosts[i]);
					}
					FREEUP (currentEvent->exHosts);
					currentEvent->numExHosts = 0;
				}
				break;
			}
			case EVENT_JOB_EXECUTE: {
				FREEUP (currentEvent->execHome);
				FREEUP (currentEvent->execCwd);
				FREEUP (currentEvent->execUsername);
				break;
			}
			case EVENT_MIG: {
				if (currentEvent->migAskedHosts) {
					for ( unsigned long i = 1; i < currentEvent->migNumAskedHosts; i++)
						FREEUP (currentEvent->migAskedHosts[i]);
			  		FREEUP (currentEvent->migAskedHosts);
				}
				FREEUP (currentEvent->userName);
				break;
			}
			case EVENT_JOB_SIGNAL: {
				FREEUP (currentEvent->userName);
				FREEUP (currentEvent->sigSymbol);
				break;
			}
			case EVENT_JOB_SIGACT: {
				FREEUP (currentEvent->sigSymbol);
				break;
			}
			case EVENT_JOB_MODIFY: {
				freeJobInfoEnt (currentEvent->newParams);
				break;
			}
			case EVENT_JOB_SWITCH:
			case EVENT_JOB_MOVE:
			case EVENT_JOB_FORCE: {
				FREEUP (currentEvent->userName);
				break;
			}
		} // end switch

		nextEvent = nextEvent->next;

		if (lastEvent == currentEvent) {
			lastEvent = currentEvent->chronback;
		}
		FREEUP (currentEvent);
	}
 	FREEUP (jobRecord);

 	return;
}

char
check_host (struct bhistReq *Req, struct jobRecord *jobRecord)
{
	if ((Req->options & OPT_HOST) != OPT_HOST) {
		return TRUE;
	}
	else {
		for ( unsigned long i = 1; i < jobRecord->job->numExHosts; i++) {
			if (!strcmp (Req->checkHost, jobRecord->job->exHosts[i])) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

struct jobRecord *
read_startjob (struct eventRec *log)
{
	struct jobRecord *jobRecord = NULL;
	struct jobStartLog *jobStartLog = NULL;
	unsigned long jobId = 0;
	struct hEnt *ent = 0;

	if (log->type == EVENT_JOB_EXECUTE) {
		jobId = (Req.options & OPT_ARRAY_INFO) ? log->eventLog.jobExecuteLog.jobId : LSB_JOBID (log->eventLog.jobExecuteLog.jobId, log->eventLog.jobExecuteLog.idx);
	}
	else {
		jobId = (Req.options & OPT_ARRAY_INFO) ? log->eventLog.jobStartLog.jobId : LSB_JOBID (log->eventLog.jobStartLog.jobId, log->eventLog.jobStartLog.idx);
	}

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
	  return NULL;
	}
	jobRecord = (struct jobRecord *) ent->hData;
	if (log->type == EVENT_JOB_EXECUTE) {
	  jobRecord->jobGid = log->eventLog.jobExecuteLog.jobPGid;
	  jobRecord->jobPid = log->eventLog.jobExecuteLog.jobPid;
	  jobRecord->job->execUid = log->eventLog.jobExecuteLog.execUid;
	  strcpy (jobRecord->job->execHome, log->eventLog.jobExecuteLog.execHome);
	  strcpy (jobRecord->job->execCwd, log->eventLog.jobExecuteLog.execCwd);
	  strcpy (jobRecord->job->execUsername, log->eventLog.jobExecuteLog.execUsername);
	  return jobRecord;
	}


	jobStartLog = &log->eventLog.jobStartLog;

	jobRecord->jobPid = jobStartLog->jobPid;
	jobRecord->jobGid = jobStartLog->jobPGid;
	jobRecord->job->startTime = log->eventTime;

	if (log->type == EVENT_JOB_START && (jobRecord->job->submit.options & SUB_PRE_EXEC)) {
		return jobRecord;
	}

	if (jobRecord->job->numExHosts > 0)
	{
		for (i = 0; i < jobRecord->job->numExHosts; i++) {
			FREEUP (jobRecord->job->exHosts[i]);
		}
		FREEUP (jobRecord->job->exHosts);
	}
	jobRecord->job->numExHosts = jobStartLog->numExHosts;

	if (jobRecord->job->numExHosts > 0) {
		if (jobRecord->job->exHosts) {
			for( unsigned long i = 1; i < jobRecord->job->numExHosts; i++)
				FREEUP (jobRecord->job->exHosts[i]);
			FREEUP (jobRecord->job->exHosts);
		}
		jobRecord->job->exHosts = calloc (jobRecord->job->numExHosts, sizeof (char *));
		for ( unsigned long i = 1; i < jobRecord->job->numExHosts; i++) {
			jobRecord->job->exHosts[i] = calloc (MAXHOSTNAMELEN, sizeof (char));
			sprintf (jobRecord->job->exHosts[i], "%s", jobStartLog->execHosts[i]);
		}
	}

	if (strcmp (jobRecord->job->submit.hostSpec, "") == 0) {
		jobRecord->hostFactor = log->eventLog.jobStartLog.hostFactor;
		jobRecord->hostPtr = jobRecord->job->exHosts[0];
	}


	jobRecord->job->cpuFactor = log->eventLog.jobStartLog.hostFactor;

	if ((logclass & LC_TRACE)) {
		ls_syslog (LOG_DEBUG2, "%s: jobId=%s", __func__, lsb_jobid2str (jobRecord->job->jobId));
	}

	return jobRecord;
}

char
read_newstat (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct jobStatusLog *jobStatusLog = NULL;
	struct hEnt *ent = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.jobStatusLog.jobId, log->eventLog.jobStatusLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		if ((Req.options & OPT_CHRONICLE) && matchJobId (&Req, log->eventLog.jobStatusLog.jobId)) {
			if ((jobRecord = createJobRec (log->eventLog.jobStatusLog.jobId)) == NULL) {
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		jobRecord = (struct jobRecord *) ent->hData;
	}
	jobStatusLog = &log->eventLog.jobStatusLog;
	jobRecord->job->cpuTime = jobStatusLog->cpuTime;
	jobRecord->job->endTime = jobStatusLog->endTime;
	jobRecord->job->exitStatus = jobStatusLog->exitStatus;
	event = calloc (1, sizeof (struct eventRecord));

	event->jStatus = jobStatusLog->jStatus;
	event->cpuTime = jobStatusLog->cpuTime;
	event->reasons = jobStatusLog->reason;
	event->subreasons = jobStatusLog->subreasons;
	event->exitStatus = jobStatusLog->exitStatus;
	event->timeStamp = log->eventTime;

	event->kind = EVENT_JOB_STATUS;
	event->idx = log->eventLog.jobStatusLog.idx;

	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}


char
read_sigact (struct eventRec *log)
{
	struct eventRecord *event   = NULL;
	struct jobRecord *jobRecord = NULL;
	struct sigactLog *sigactLog = NULL;
	struct hEnt *ent = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.sigactLog.jobId, log->eventLog.sigactLog.idx);


	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	sigactLog = &log->eventLog.sigactLog;

	event->chkPeriod = sigactLog->period;
	event->actPid = sigactLog->pid;
	event->jStatus = sigactLog->jStatus;
	event->reasons = sigactLog->reasons;
	event->actStatus = sigactLog->actStatus;
	event->actFlags = sigactLog->flags;
	event->sigSymbol = malloc (strlen (sigactLog->signalSymbol) + 1);
	strcpy (event->sigSymbol, log->eventLog.sigactLog.signalSymbol);

	event->timeStamp = log->eventTime;
	event->jStatus = jobRecord->currentStatus;
	event->reasons = jobRecord->job->reasons;

	event->kind = EVENT_JOB_SIGACT;
	event->idx = log->eventLog.sigactLog.idx;
	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}

char
read_jobrequeue (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct hEnt *ent = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.jobRequeueLog.jobId, log->eventLog.jobRequeueLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	event->kind = EVENT_JOB_REQUEUE;
	event->timeStamp = log->eventTime;
	event->idx = log->eventLog.jobRequeueLog.idx;
	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}


char
read_chkpnt (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct chkpntLog *chkLog = NULL;
	struct hEnt *ent = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.chkpntLog.jobId, log->eventLog.chkpntLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));
	chkLog = &log->eventLog.chkpntLog;

	event->chkPeriod = chkLog->period;
	event->actPid = chkLog->pid;
	event->chkOk = chkLog->ok;
	event->actFlags = chkLog->flags;
	event->timeStamp = log->eventTime;
	event->jStatus = jobRecord->currentStatus;
	event->reasons = jobRecord->job->reasons;

	event->kind = EVENT_CHKPNT;
	event->idx = log->eventLog.chkpntLog.idx;
	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}


char
read_mig (struct eventRec *log)
{
	struct eventRecord *event   = NULL;
	struct jobRecord *jobRecord = NULL;
	struct migLog *migLog       = NULL;
	struct hEnt *ent            = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.sigactLog.jobId, log->eventLog.sigactLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));
	migLog = &log->eventLog.migLog;
	event->migNumAskedHosts = migLog->numAskedHosts;

	if (event->migNumAskedHosts > 0) {
		event->migAskedHosts = calloc (event->migNumAskedHosts, sizeof (char *));
		for( unsigned long i = 1; i < event->migNumAskedHosts; i++) {
			event->migAskedHosts[i] = malloc (strlen (migLog->askedHosts[i]) + 1);
			strcpy (event->migAskedHosts[i], migLog->askedHosts[i]);
		}
	}
	else {
		event->migAskedHosts = NULL;
	}

	event->kind = EVENT_MIG;
	event->jStatus = jobRecord->currentStatus;
	event->reasons = jobRecord->job->reasons;
	event->timeStamp = log->eventTime;
	event->userId = migLog->userId;
	event->userName = putstr_ (migLog->userName);
	event->idx = log->eventLog.migLog.idx;
	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}

char
read_signal (struct eventRec *log)
{
  struct eventRecord *event = NULLs;
  struct jobRecord *jobRecord = NULL;
  struct hEnt *ent = NULL;
  unsigned long jobId = 0;

  jobId = GET_JOBID (log->eventLog.signalLog.jobId, log->eventLog.signalLog.idx);


	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	event->kind = EVENT_JOB_SIGNAL;
	event->jStatus = jobRecord->currentStatus;
	event->reasons = jobRecord->job->reasons;
	event->timeStamp = log->eventTime;
	event->runCount = log->eventLog.signalLog.runCount;
	event->userId = log->eventLog.signalLog.userId;
	event->userName = putstr_ (log->eventLog.signalLog.userName);
	event->sigSymbol = malloc (strlen (log->eventLog.signalLog.signalSymbol) + 1);
	strcpy (event->sigSymbol, log->eventLog.signalLog.signalSymbol);
	event->idx = log->eventLog.signalLog.idx;

	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}

char
read_jobstartaccept (struct eventRec *log)
{
	struct eventRecord *event = NULLs;
	struct jobRecord *jobRecord = NULLs;
	struct hEnt *ent = NULL;
	unsigned long jobId = NULL;

	jobId = GET_JOBID (log->eventLog.jobStartAcceptLog.jobId, log->eventLog.jobStartAcceptLog.idx);

	if( (sent = chekMemb (&jobIdHT, jobId)) == NULL ) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	event->kind = EVENT_JOB_START_ACCEPT;
	event->jobPid = log->eventLog.jobStartAcceptLog.jobPid;
	event->timeStamp = log->eventTime;
	event->jStatus = JOB_STAT_RUN;
	event->idx = log->eventLog.jobStartAcceptLog.idx;
	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}

char
read_jobmsg (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct hEnt *ent = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.jobMsgLog.jobId, log->eventLog.jobMsgLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	event->kind = EVENT_JOB_MSG;
	event->jStatus = jobRecord->currentStatus;
	event->reasons = jobRecord->job->reasons;
	event->timeStamp = log->eventTime;

	event->usrId = log->eventLog.jobMsgLog.usrId;
	event->idx = log->eventLog.jobMsgLog.idx;
	event->jmMsgId = log->eventLog.jobMsgLog.msgId;
	event->jmMsgType = log->eventLog.jobMsgLog.type;
	event->jmSrc = malloc (strlen (log->eventLog.jobMsgLog.src) + 1);
	strcpy (event->jmSrc, log->eventLog.jobMsgLog.src);

	event->jmDest = malloc (strlen (log->eventLog.jobMsgLog.dest) + 1);
	strcpy (event->jmDest, log->eventLog.jobMsgLog.dest);
	event->jmMsg = malloc (strlen (log->eventLog.jobMsgLog.msg) + 1);
	strcpy (event->jmMsg, log->eventLog.jobMsgLog.msg);
	event->idx = log->eventLog.jobMsgLog.idx;
	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}

char
read_jobmsgack (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct hEnt *ent = NULLs;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.jobMsgAckLog.jobId, log->eventLog.jobMsgAckLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	event->kind = EVENT_JOB_MSG_ACK;
	event->jStatus = jobRecord->currentStatus;
	event->reasons = jobRecord->job->reasons;
	event->timeStamp = log->eventTime;

	event->usrId = log->eventLog.jobMsgAckLog.usrId;
	event->idx = log->eventLog.jobMsgAckLog.idx;
	event->jmMsgId = log->eventLog.jobMsgAckLog.msgId;
	event->jmMsgType = log->eventLog.jobMsgAckLog.type;
	event->jmSrc = malloc (strlen (log->eventLog.jobMsgAckLog.src) + 1);
	strcpy (event->jmSrc, log->eventLog.jobMsgAckLog.src);
	event->jmDest = malloc (strlen (log->eventLog.jobMsgAckLog.dest) + 1);
	strcpy (event->jmDest, log->eventLog.jobMsgAckLog.dest);
	event->jmMsg = malloc (strlen (log->eventLog.jobMsgAckLog.msg) + 1);
	strcpy (event->jmMsg, log->eventLog.jobMsgAckLog.msg);
	event->idx = log->eventLog.jobMsgAckLog.idx;

	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}




char
read_switch (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct hEnt *ent = NULL;
	unsigned jobId;

	jobId = GET_JOBID (log->eventLog.jobSwitchLog.jobId, log->eventLog.jobSwitchLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) { 
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	strcpy (event->queue, log->eventLog.jobSwitchLog.queue);

	event->jStatus = jobRecord->currentStatus;
	event->reasons = jobRecord->job->reasons;
	event->userId = log->eventLog.jobSwitchLog.userId;
	event->userName = putstr_ (log->eventLog.jobSwitchLog.userName);
	event->timeStamp = log->eventTime;

	event->kind = EVENT_JOB_SWITCH;
	event->idx = log->eventLog.jobSwitchLog.idx;

	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}


char
read_jobmove (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct hEnt *ent = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.jobMoveLog.jobId, log->eventLog.jobMoveLog.idx);


	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	event->base = log->eventLog.jobMoveLog.base;
	event->position = log->eventLog.jobMoveLog.position;
	event->userId = log->eventLog.jobMoveLog.userId;
	event->userName = putstr_ (log->eventLog.jobMoveLog.userName);
	event->timeStamp = log->eventTime;

	event->kind = EVENT_JOB_MOVE;
	event->idx = log->eventLog.jobMoveLog.idx;

	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}


char
read_loadIndex (struct eventRec *log)
{
	if (!loadIndex) { // FIXME FIXME FIXME FIXME the fakity fuck is loadIndex and where is it?
		return FALSE;
	}

	if (loadIndex->name) {
		for (i = 0; i < loadIndex->nIdx; i++) {
			FREEUP (loadIndex->name[i]);
		}
		FREEUP (loadIndex->name);
	}

	loadIndex->nIdx = log->eventLog.loadIndexLog.nIdx;
	loadIndex->name = calloc (loadIndex->nIdx, sizeof (char *));

	for( unsigned integer i = 0; i < loadIndex->nIdx; i++) {
		loadIndex->name[i] = malloc (MAXLSFNAMELEN);
		strcpy (loadIndex->name[i], log->eventLog.loadIndexLog.name[i]);
	}

	return TRUE;
}

char
read_jobforce (struct eventRec *log)
{
	struct eventRecord *event = NULL;
	struct jobRecord *jobRecord = NULL;
	struct hEnt *ent = NULL;
	unsigned long jobId = 0;

	jobId = GET_JOBID (log->eventLog.jobForceRequestLog.jobId, log->eventLog.jobForceRequestLog.idx);

	if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
		return FALSE;
	}

	jobRecord = (struct jobRecord *) ent->hData;
	event = calloc (1, sizeof (struct eventRecord));

	event->kind = EVENT_JOB_FORCE;
	event->timeStamp = log->eventTime;
	event->userId = log->eventLog.jobForceRequestLog.userId;
	event->userName = strdup (log->eventLog.jobForceRequestLog.userName);
	event->idx = log->eventLog.jobForceRequestLog.idx;

	if (addEvent (event, jobRecord) == -1) {
		FREEUP (event);
		return FALSE;
	}

	return TRUE;
}


int
addJob (struct jobRecord *newjobRecord)
{
	struct hEnt *ent = NULL;
	struct jobRecord *jobRecord = NULL;

	newjobRecord->job->startTime = 0;
	newjobRecord->job->endTime = 0;
	newjobRecord->currentStatus = JOB_STAT_PEND;
	newjobRecord->eventhead = NULL;
	newjobRecord->eventtail = NULL;
	newjobRecord->forw = NULL;
	newjobRecord->back = NULL;
	newjobRecord->nonNewFromHeadFlag = 0;

	if ((ent = chekMemb (&jobIdHT, newjobRecord->job->jobId)) != NULL) {
		jobRecord = (struct jobRecord *) ent->hData;

		if (newjobRecord->job->submitTime == jobRecord->job->submitTime) {
			freeJobRecord (jobRecord);
			if ((ent = addMemb (&jobIdHT, newjobRecord->job->jobId)) != NULL) {
				ent->hData = newjobRecord;
			}
			else {
				return -1;
			}
		}
		else
			ent->hData = (int *) newjobRecord;
	}
	else {
		if ((ent = addMemb (&jobIdHT, newjobRecord->job->jobId)) != NULL) {
			ent->hData = newjobRecord;
		}
	}

	inJobList (jobRecordList, newjobRecord);

	return 0;
}

struct jobInfoEnt *
initJobInfo (void)
{
	struct jobInfoEnt *job = NULL;
	struct submit *submitPtr = NULL;

	job = calloc (1, sizeof (struct jobInfoEnt));
	job->cpuTime = 0;
	job->cpuFactor = 0;
	job->status = JOB_STAT_PEND;
	job->reasons = PEND_JOB_NEW;
	job->subreasons = 0;
	job->numExHosts = 0;
	job->exHosts = NULL;
	job->startTime = 0;
	job->endTime = 0;
	job->nIdx = 0;
	job->loadSched = NULL;
	job->loadStop = NULL;
	job->jobPriority = -1;

	submitPtr = &job->submit;

	submitPtr->queue = malloc (MAX_LSB_NAME_LEN);
	submitPtr->jobName = malloc (MAX_CMD_DESC_LEN);
	submitPtr->command = malloc (MAX_CMD_DESC_LEN);
	submitPtr->resReq = malloc (MAXLINELEN);
	submitPtr->inFile = malloc (MAXFILENAMELEN);
	submitPtr->outFile = malloc (MAXFILENAMELEN);
	submitPtr->errFile = malloc (MAXFILENAMELEN);
	submitPtr->hostSpec = malloc (MAXFILENAMELEN);
	submitPtr->chkpntDir = malloc (MAXFILENAMELEN);
	submitPtr->dependCond = malloc (6 * MAXLINELEN);
	submitPtr->preExecCmd = malloc (MAXLINELEN);
	submitPtr->mailUser = malloc (MAXHOSTNAMELEN);
	submitPtr->projectName = malloc (MAX_LSB_NAME_LEN);
	submitPtr->loginShell = malloc (MAX_LSB_NAME_LEN);
	job->user = malloc (MAX_LSB_NAME_LEN);
	job->fromHost = malloc (MAXHOSTNAMELEN);
	job->cwd = malloc (MAXFILENAMELEN);
	job->execCwd = malloc (MAXFILENAMELEN);
	job->execHome = malloc (MAXFILENAMELEN);
	job->execUsername = malloc (MAX_LSB_NAME_LEN);

	submitPtr->queue[0] = '\0';
	submitPtr->jobName[0] = '\0';
	submitPtr->command[0] = '\0';
	submitPtr->resReq[0] = '\0';
	submitPtr->inFile[0] = '\0';
	submitPtr->outFile[0] = '\0';
	submitPtr->errFile[0] = '\0';
	submitPtr->hostSpec[0] = '\0';
	submitPtr->chkpntDir[0] = '\0';
	submitPtr->dependCond[0] = '\0';
	submitPtr->preExecCmd[0] = '\0';
	submitPtr->mailUser[0] = '\0';
	submitPtr->projectName[0] = '\0';
	submitPtr->loginShell[0] = '\0';
	job->user[0] = '\0';
	job->fromHost[0] = '\0';
	job->cwd[0] = '\0';
	job->execCwd[0] = '\0';
	job->execHome[0] = '\0';
	job->execUsername[0] = '\0';

	submitPtr->options = 0;
	submitPtr->options2 = 0;
	submitPtr->numAskedHosts = 0;
	submitPtr->numProcessors = 0;
	submitPtr->beginTime = 0;
	submitPtr->termTime = 0;
	submitPtr->sigValue = 0;
	submitPtr->chkpntPeriod = 0;
	submitPtr->nxf = 0;
	submitPtr->delOptions = 0;
	submitPtr->delOptions2 = 0;
	submitPtr->userPriority = -1;

	for ( unsigned long i = 1; i < LSF_RLIM_NLIMITS; i++) {
		submitPtr->rLimits[i] = -1;
	}

	return job;
}

void
parse_event (struct eventRec *log, struct bhistReq *Req)
{
	struct jobInfoEnt *job         = NULL;
	struct submit *submitPtr       = NULL;
	struct jobRecord *newjobRecord = NULL;
	struct jobRecord *jobRecord    = NULL;
	struct eventRecord *event      = NULL;
	struct hEnt *ent               = NULL;
	unsigned long oldjobnum        = 0;
	unsigned int found             = 0;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG2, "%s: log.type=%x", __func__, log->type);
	}

	switch (log->type) { 
		case EVENT_JOB_NEW: {
			job = read_newjob (log);
			if (job != NULL) {
				submitPtr = &job->submit;
				if (((Req->options & OPT_ALLUSERS) != OPT_ALLUSERS) && strcmp (job->user, Req->userName)) {
					freeJobInfoEnt (job);
					return;
				}
				if (Req->options & OPT_JOBID) {
					found = FALSE;
					for( unsigned long i = 1; i < Req->numJobs; i++) { // jobs are always ulong and always start from 1
						if (((job->jobId == LSB_ARRAY_JOBID (Req->jobIds[i]))) || (job->jobId == Req->jobIds[i])) {
							found = TRUE;
							break;
						}
					}
					if (found == FALSE) {
						freeJobInfoEnt (job);
						return;
					}
				}
				if (Req->options & OPT_JOBNAME) {
					unsigned int *idxList = 1;
					if (!matchName (Req->jobName, job->submit.jobName)) {
						freeJobInfoEnt (job);
						return;
					}
					oldjobnum = Req->numJobs;
					Req->numJobs += getSpecIdxs (Req->jobName, &idxList);
					if (Req->numJobs - oldjobnum > 0) {
						Req->jobIds =  realloc (Req->jobIds, Req->numJobs * sizeof (unsigned long ));
						if (Req->jobIds == NULL) {
							unsigned int index = 96;
							char buff[ index ];
							memset( buff, '\0', index );
							sprintf ( buff, "realloc in %s", __func__ )
							perror ( buff );
							exit (-1);
						}
						for ( unsigned long i = oldjobnum; i < Req->numJobs; i++) {
							Req->jobIds[i] =	LSB_JOBID (LSB_ARRAY_JOBID (job->jobId), idxList[i - oldjobnum]);
						}
					}
					FREEUP (idxList);
				}

				if (check_queue (Req, submitPtr->queue)) {
					unsigned int numEles = 0, 
					unsigned int *idxList = NULL;

					if (job->submit.options & SUB_JOB_NAME) {
						numEles = getSpecIdxs (job->submit.jobName, &idxList);
					}
					else {
						idxList = NULL;
					}

					if ((Req->options & OPT_ARRAY_INFO) || numEles <= 0) {
						if (!idxList) {
							if ((idxList = calloc (1, sizeof( unsigned int ) ) ) == NULL) {
								perror ("malloc");
								exit (-1);
							}
						}
						numEles = 1;
						idxList[0] = 0;
					}


					for (unsigned int i = 0; i < numEles; i++) {
						job->jobId = LSB_JOBID (LSB_ARRAY_JOBID (job->jobId), idxList[i]);
						if (!(newjobRecord = malloc (sizeof (struct jobRecord)))) {
							char i18nBuf[100];
							sprintf (i18nBuf, I18N_FUNC_FAIL_M, __func__, "malloc");
							perror (i18nBuf);
							exit (-1);
						}
						newjobRecord->job = job;
						newjobRecord->hostFactor = log->eventLog.jobNewLog.hostFactor;
						newjobRecord->currentStatus = 0;

						if (addJob (newjobRecord) == -1) {
							freeJobRecord (newjobRecord);
							return;
						}
						if (!(event = malloc (sizeof (struct eventRecord)))) {
							char i18nBuf[100];
							sprintf (i18nBuf, I18N_FUNC_FAIL_M, __func__, "malloc");
							perror (i18nBuf);
							exit (-1);
						}
						memset ((struct eventRecord *) event, 0, sizeof (struct eventRecord));

						if (submitPtr->options2 & SUB2_HOLD) {
							event->jStatus = JOB_STAT_PSUSP;
						}
						else {
							event->jStatus = JOB_STAT_PEND;
						}
						event->reasons = PEND_JOB_NEW;

						if (log->eventTime > job->submitTime) {
							event->timeStamp = job->submitTime;
						}
						else {
							event->timeStamp = log->eventTime;
						}
						event->kind = EVENT_JOB_NEW;
						event->idx = log->eventLog.jobNewLog.idx;

						if (addEvent (event, newjobRecord) == -1) {
							FREEUP (event);
							return;
						}
						if ((i + 1) < numEles) {
							job = copyJobInfoEnt (job);
						}
					}
					if (idxList)
						FREEUP (idxList);
				}
				else {
					freeJobInfoEnt (job);
					return;
				}
			} // end if( new_job not null )
			break;
		} // end case EVENT_JOB_NEW 

		case EVENT_JOB_MODIFY: {
			job = read_newjob (log);
			if ((ent = chekMemb (&jobIdHT, job->jobId)) == NULL) {
				freeJobInfoEnt (job);
				break;
			}
			// jobRecord = (struct jobRecord *) ent->hData;
			jobRecord = ent->hData;

			if (!(event = malloc (sizeof (struct eventRecord)))) {
				perror ("malloc");
				freeJobInfoEnt (job);
				exit (-1);
			}
			memset ((struct eventRecord *) event, 0, sizeof (struct eventRecord));
			event->kind = log->type;
			event->newParams = job;
			event->jStatus = jobRecord->currentStatus;
			event->reasons = jobRecord->job->reasons;
			event->timeStamp = log->eventTime;
			event->idx = log->eventLog.jobNewLog.idx;
			if (addEvent (event, jobRecord) == -1) {
				freeJobInfoEnt (job);
			}
			break;
		}
		case EVENT_JOB_MODIFY2: {

			unsigned int array_jobId = 0
			unsigned int array_ele = 0;
			unsigned int numEles = 0
			unsigned int *idxList = NULL;
			unsigned long jobId;

			numEles = getSpecIdxs (log->eventLog.jobModLog.jobIdStr, &idxList);

			if ((Req->options & OPT_ARRAY_INFO) || numEles <= 0) {

				if( !idxList ) {
					if ((idxList = calloc (1, sizeof (int))) == NULL) {
						perror ("calloc");
						exit (-1);
					}
				}
				numEles = 1;
				idxList[0] = 0;
			}

			sscanf (log->eventLog.jobModLog.jobIdStr, "%d[%d]", &array_jobId, &array_ele);
			jobId = LSB_JOBID (array_jobId, array_ele);

			ent = chekMemb (&jobIdHT, jobId);
			if (ent == NULL && numEles == 1) {

				struct sTab hashSearchPtr = NULL;
				struct hEnt *hEntPtr = h_firstEnt_ (&jobIdHT, &hashSearchPtr);

				while (hEntPtr) {
					unsigned long tmpId = 0;
					int tmpArrId = 0;
					tmpId = atoi64_ (hEntPtr->keyname);
					tmpArrId = LSB_ARRAY_JOBID (tmpId);
					if (tmpArrId == array_jobId) {
						if ((ent = chekMemb (&jobIdHT, tmpId)) == NULL) {
							break;
						}
						insertModEvent (log, ent);
					}
					hEntPtr = h_nextEnt_ (&hashSearchPtr);
				}
			}
		else
			for (unsigned long i = 0; i < numEles; i++) {
				jobId = LSB_JOBID (array_jobId, idxList[i]);
				if ((ent = chekMemb (&jobIdHT, jobId)) == NULL) {
					break;
				}
				insertModEvent (log, ent);
			}
			if (idxList) {
				free (idxList);
			}
		}
		break;
	}
	case EVENT_PRE_EXEC_START:
	case EVENT_JOB_START:
	case EVENT_JOB_EXECUTE: {

		jobRecord = read_startjob (log);

		if (jobRecord != NULL) {
			if (check_host (Req, jobRecord)) {
				event = calloc (1, sizeof (struct eventRecord));

				if (event == NULL) {
					char i18nBuf[100];
					sprintf (i18nBuf, I18N_FUNC_FAIL, __func__, "malloc");
					perror (i18nBuf);
					exit (-1);
				}

				memset( event, 0, sizeof (struct eventRecord));

				event->jStatus = JOB_STAT_RUN;
				event->timeStamp = log->eventTime;

				event->kind = log->type;
				event->jobPid = jobRecord->jobPid;
				if (log->type == EVENT_JOB_EXECUTE) {
					event->numExHosts = 0;
					event->execUid = jobRecord->job->execUid;
					event->execHome = malloc (MAXFILENAMELEN);
					strcpy (event->execHome, jobRecord->job->execHome);
					event->execCwd = malloc (MAXFILENAMELEN);
					strcpy (event->execCwd, jobRecord->job->execCwd);
					event->execUsername = malloc (MAX_LSB_NAME_LEN);
					strcpy (event->execUsername, jobRecord->job->execUsername);
				}
				else {
					event->numExHosts = jobRecord->job->numExHosts;
					if (event->numExHosts > 0) {
						event->exHosts = calloc (event->numExHosts, sizeof (char *));
						for ( unsigned long i = 0; i < event->numExHosts; i++) {
							event->exHosts[i] = malloc (MAXHOSTNAMELEN);
							strcpy (event->exHosts[i], jobRecord->job->exHosts[i]);
						}
					}
				}
				if (log->type == EVENT_JOB_EXECUTE) {
					event->idx = log->eventLog.jobExecuteLog.idx;
				}
				else {
					event->idx = log->eventLog.jobStartLog.idx;
				}

				if (addEvent (event, jobRecord) == -1) {
					if (event->numExHosts > 0) {
						for ( unsigned long i = 0; i < event->numExHosts; i++) {
							FREEUP (event->exHosts[i]);
						}
						FREEUP (event->exHosts);
					}
					if (log->type == EVENT_JOB_EXECUTE) {
						FREEUP (event->execHome);
						FREEUP (event->execCwd);
						FREEUP (event->execUsername);
					}

					FREEUP (event);

					return;
				}
			}
			else {
				if (jobRecord->job->numExHosts) {
					for ( unsigned long i = 0; i < jobRecord->job->numExHosts; i++) {
						FREEUP (jobRecord->job->exHosts[i]);
					}
					FREEUP (jobRecord->job->exHosts);
				}
				jobRecord->job->numExHosts = 0;
				freeJobRecord (jobRecord);
			}
		}

		break;
	}
	case EVENT_JOB_STATUS: {
	  read_newstat (log);
	  break;
	}
	case EVENT_JOB_FORCE: {
	  read_jobforce (log);
	  break;
	}
	case EVENT_JOB_SWITCH: {
	  read_switch (log);
	  break;
	}
	case EVENT_CHKPNT: {
	  read_chkpnt (log);
	  break;
	}
	case EVENT_MIG: {
	  read_mig (log);
	  break;
	}
	case EVENT_LOAD_INDEX: {
	  read_loadIndex (log);
	  break;
	}
	case EVENT_JOB_SIGNAL: {
	  read_signal (log);
	  break;
	}
	case EVENT_JOB_START_ACCEPT: {
	  read_jobstartaccept (log);
	  break;
	}
	case EVENT_JOB_MSG: {
	  read_jobmsg (log);
	  break;
	}
	case EVENT_JOB_MSG_ACK: {
	  read_jobmsgack (log);
	  break;
	}
	case EVENT_JOB_SIGACT: {
	  read_sigact (log);
	  break;
	}
	case EVENT_JOB_REQUEUE: {
	  read_jobrequeue (log);
	  break;
	}
	case EVENT_JOB_MOVE: {
	  read_jobmove (log);
	  break;
	}
	case EVENT_QUEUE_CTRL: {
	  break;
	}
}

struct jobRecord *
createJobRec (int jobId)
{
	struct jobRecord *newjobRecord = NULL;
	struct jobInfoEnt *job = NULL;

	job = initJobInfo ();
	job->jobId = jobId;
	strcpy (job->user, Req.userName);
	strcpy (job->submit.projectName, Req.projectName);
	strcpy (job->submit.queue, Req.queue);
	if (!(newjobRecord = malloc (sizeof (struct jobRecord)))) {
		perror ("malloc");
		exit (-1);
	}

	newjobRecord->job = job;
	newjobRecord->currentStatus = 0;

	if (addJob (newjobRecord) == -1) {
		freeJobRecord (newjobRecord);
		return NULL;
	}

	return newjobRecord;
}

int
matchJobId (struct bhistReq *Req, unsigned long jobId)
{
	for (unsigned int i = 0; i < Req->numJobs; i++) {
		if (Req->options & OPT_JOBNAME) {
			if (LSB_ARRAY_IDX (Req->jobIds[i]) > 0) {
				if ((jobId == Req->jobIds[i]) || (
						(LSB_ARRAY_JOBID (jobId) == LSB_ARRAY_JOBID (Req->jobIds[i])) && 
						(LSB_ARRAY_IDX (jobId) == 0))
					) {
						return TRUE;
					}
				}
	  			else {
					if (LSB_ARRAY_JOBID (jobId) == Req->jobIds[i]) {
						return TRUE;
					}
				}
		}
		else if ( ( jobId == Req->jobIds[i] ) || (
						LSB_ARRAY_JOBID (Req->jobIds[i]) == LSB_ARRAY_JOBID (jobId) && (
							LSB_ARRAY_IDX (Req->jobIds[i]) == 0 || 
								LSB_ARRAY_IDX (jobId) == 0 
							) 
						)
				)
			{
			return TRUE;
		}
	}

	if (Req->numJobs) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}


void
insertModEvent (struct eventRec *log, hEnt * ent)
{
	struct jobRecord *jobRecord = NULL;
	struct eventRecord *event = NULL;

//	jobRecord = (struct jobRecord *) ent->hData;
	jobRecord = ent->hData;

	if (!(event = malloc (sizeof (struct eventRecord)))) {
		perror ("malloc");
		exit (-1);
	}

	memset ( event, 0, sizeof (struct eventRecord));
	event->kind = log->type;
	event->timeStamp = log->eventTime;


	if (log->type == EVENT_JOB_MODIFY2) {
		copyJobModLog (&(event->eventRecUnion.jobModLog), &(log->eventLog.jobModLog));
	}

	if (addEvent (event, jobRecord) == -1) {
		// FIXME FXME FIXME FIXME FIXME what the fuckety fuck was here?
	}

	return;
}

int
bhistReqInit (struct bhistReq *bhistReq)
{
	bhistReq->options = 0;
	bhistReq->options |= OPT_DFTSTATUS;
	bhistReq->options |= OPT_DFTFORMAT;
	bhistReq->options |= OPT_ALLPROJ;
	bhistReq->numJobs = 0;
	bhistReq->queue[0] = '\0';
	bhistReq->checkHost[0] = '\0';
	bhistReq->jobName = NULL;
	bhistReq->numLogFile = 1;
	bhistReq->projectName[0] = '\0';
	bhistReq->searchTime[0] = -1;
	bhistReq->searchTime[1] = -1;

	bhistReq->userId = getuid ();
	if (getLSFUser_ (bhistReq->userName, MAX_LSB_NAME_LEN) != 0)
	{
		fprintf (stderr, I18N_FUNC_FAIL, "bhistReqInit", "getLSFUser_");
		return -1;
	}
	return 0;
}
