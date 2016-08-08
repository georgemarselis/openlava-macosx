/* $Id: lsb.xdr.c 397 2007-11-26 19:04:00Z mblack $
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

#ifdef __APPLE__
#undef __LP64__
#endif


#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include "lsb/lsb.h"
#include "lsb/xdr.h"

static int allocLoadIdx (float **sched, float **stop, uint *outSize, uint size);
static bool_t xdr_lsbSharedResourceInfo (XDR *, struct lsbSharedResourceInfo *, struct LSFHeader *);
static bool_t xdr_lsbShareResourceInstance (XDR *xdrs, struct lsbSharedResourceInstance  *, struct LSFHeader *);


bool_t
xdr_xFile (XDR *xdrs, struct xFile * xf, struct LSFHeader *hdr)
{
    char *sp = NULL;

    assert( hdr->length );

    if (xdrs->x_op == XDR_DECODE) {
        xf->subFn[0] = '\0';
        xf->execFn[0] = '\0';
    }
    sp = xf->subFn;

    if (!xdr_string (xdrs, &sp, MAXFILENAMELEN)) {
        return FALSE;
    }

    sp = xf->execFn;
    if (!xdr_string (xdrs, &sp, MAXFILENAMELEN)) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &xf->options)) {
        return FALSE;
    }

  return TRUE;
}


bool_t
xdr_submitReq (XDR *xdrs, struct submitReq *submitReq, struct LSFHeader *hdr)
{
    uint nLimits      = 0;
    int *uintNxf      = 0;
    uint *intNiosPort = 0;
    uint *intNumAskedHosts    = 0;
    uint *intMaxNumProcessors = 0;
    static uint numAskedHosts = 0;
    static char **askedHosts  = NULL;

    if (xdrs->x_op == XDR_DECODE)
    {
        submitReq->fromHost[0]     = '\0';
        submitReq->jobFile[0]      = '\0';
        submitReq->inFile[0]       = '\0';
        submitReq->outFile[0]      = '\0';
        submitReq->errFile[0]      = '\0';
        submitReq->inFileSpool[0]  = '\0';
        submitReq->commandSpool[0] = '\0';
        submitReq->chkpntDir[0]    = '\0';
        submitReq->hostSpec[0]     = '\0';
        submitReq->cwd[0]          = '\0';
        submitReq->subHomeDir[0]   = '\0';
        FREEUP( submitReq->queue);
        FREEUP( submitReq->command);
        FREEUP( submitReq->jobName);
        FREEUP( submitReq->preExecCmd);
        FREEUP( submitReq->dependCond);
        FREEUP( submitReq->resReq);
        FREEUP( submitReq->mailUser);
        FREEUP( submitReq->projectName);
        FREEUP( submitReq->loginShell);
        FREEUP( submitReq->schedHostType);
        if (numAskedHosts > 0) {
            for ( uint i = 0; i < numAskedHosts; i++) {
                FREEUP (askedHosts[i]);
            }
            FREEUP (askedHosts);
        }
        numAskedHosts = 0;
    }

    assert(  submitReq->numProcessors <= INT_MAX );
    if ( !( xdr_int      (xdrs, &submitReq->options) &&  
            xdr_var_string (xdrs, &submitReq->queue)  &&
            xdr_var_string (xdrs, &submitReq->resReq) && 
            xdr_int (xdrs, (int *)&submitReq->numProcessors))
        ) 
    {
      return FALSE;
    }

    if ( !(
        xdr_string     (xdrs, &submitReq->fromHost, MAXHOSTNAMELEN) &&
        xdr_var_string (xdrs, &submitReq->dependCond) &&
        xdr_var_string (xdrs, &submitReq->jobName) &&
        xdr_var_string (xdrs, &submitReq->command) &&
        xdr_string     (xdrs, &submitReq->jobFile, MAXFILENAMELEN) &&
        xdr_string     (xdrs, &submitReq->inFile, MAXFILENAMELEN)  &&
        xdr_string     (xdrs, &submitReq->outFile, MAXFILENAMELEN) &&
        xdr_string     (xdrs, &submitReq->errFile, MAXFILENAMELEN) &&
        xdr_var_string (xdrs, &submitReq->preExecCmd) &&
        xdr_string     (xdrs, &submitReq->hostSpec, MAXHOSTNAMELEN))
    )
    {
        return FALSE;
    }

    assert( submitReq->numAskedHosts <= UINT_MAX );
    *intNumAskedHosts = submitReq->numAskedHosts;
    if (!xdr_u_int (xdrs, intNumAskedHosts )) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE && submitReq->numAskedHosts > 0)
    {
        submitReq->askedHosts = calloc (submitReq->numAskedHosts, sizeof (char *));
        if ( NULL == submitReq->askedHosts && ENOMEM == errno ) {
            return FALSE;
        }
    }

    for ( uint i = 0; i < submitReq->numAskedHosts; i++) {
        if (!xdr_var_string (xdrs, &submitReq->askedHosts[i])) {
            submitReq->numAskedHosts = i;
            goto Error0;  // FIXME FIXME FIXME FIXME remove goto
        }
    }

    nLimits = LSF_RLIM_NLIMITS;
    if (!xdr_u_int (xdrs, &nLimits)) {
        goto Error0;  // FIXME FIXME FIXME FIXME remove goto
    }

    for (uint i = 0; i < nLimits; i++)
    {
        if (i < LSF_RLIM_NLIMITS)
        {
            if (!xdr_int (xdrs, &submitReq->rLimits[i])) {
                goto Error0;  // FIXME FIXME FIXME FIXME remove goto
            }
        }
        else
        {
            int j = 0;
            if (!xdr_int (xdrs, &j))
                goto Error0;  // FIXME FIXME FIXME FIXME remove goto
        }
    }

    if (! ( xdr_time_t (xdrs, &submitReq->submitTime)   &&
            xdr_time_t (xdrs, &submitReq->beginTime)    &&
            xdr_time_t (xdrs, &submitReq->termTime)     &&
            xdr_u_short(xdrs, &submitReq->umask)        &&
            xdr_int    (xdrs, &submitReq->sigValue)     &&
            xdr_int    (xdrs, &submitReq->restartPid)   &&
            xdr_time_t (xdrs, &submitReq->chkpntPeriod) &&
            xdr_string (xdrs, &submitReq->chkpntDir, MAXFILENAMELEN)  &&
            xdr_string (xdrs, &submitReq->subHomeDir, MAXFILENAMELEN) &&
            xdr_string (xdrs, &submitReq->cwd, MAXFILENAMELEN))
        )
    {
            goto Error0;   // FIXME FIXME FIXME FIXME remove goto
    }

    assert( submitReq->nxf >= 0 );
    *uintNxf = submitReq->nxf;
    if (!xdr_int (xdrs, uintNxf )) {
        goto Error0;   // FIXME FIXME FIXME FIXME remove goto
    }

    if (xdrs->x_op == XDR_DECODE && submitReq->nxf > 0)
    {
        submitReq->xf = calloc (submitReq->nxf, sizeof (struct xFile));
        if ( NULL == submitReq->xf && ENOMEM == errno ) {
            goto Error0;   // FIXME FIXME FIXME FIXME remove goto
        }
    }

    for ( uint i = 0; i < submitReq->nxf; i++)
    {
        if (!xdr_arrayElement (xdrs, &(submitReq->xf[i]), hdr, xdr_xFile)) {
            goto Error1;   // FIXME FIXME FIXME FIXME remove goto
        }
    }

    if (!xdr_var_string (xdrs, &submitReq->mailUser)) {
        goto Error1;   // FIXME FIXME FIXME FIXME remove goto
    }

    if (!xdr_var_string (xdrs, &submitReq->projectName)) {
        goto Error1;  // FIXME FIXME FIXME FIXME remove goto
    }

    assert( submitReq->niosPort >= 0 );
    *intNiosPort = submitReq->niosPort;
    if (!xdr_u_int (xdrs, intNiosPort)) {
        goto Error1;  // FIXME FIXME FIXME FIXME remove goto
    }

    assert( submitReq->maxNumProcessors >= 0 );
    *intMaxNumProcessors = submitReq->maxNumProcessors;
    if (!xdr_u_int (xdrs, intMaxNumProcessors )) {
        goto Error1;   // FIXME FIXME FIXME FIXME remove goto
    }

    if (!xdr_var_string (xdrs, &submitReq->loginShell)) {
        goto Error1;  // FIXME FIXME FIXME FIXME remove goto
    }

    if (!xdr_var_string (xdrs, &submitReq->schedHostType)) {
        goto Error1;  // FIXME FIXME FIXME FIXME remove goto
    }

    if (xdrs->x_op == XDR_DECODE) {
        numAskedHosts = submitReq->numAskedHosts;
        askedHosts = submitReq->askedHosts;
    }

    if (!xdr_int (xdrs, &submitReq->options2)) {
        goto Error1;  // FIXME FIXME FIXME FIXME remove goto
    }

    if (!(xdr_string (xdrs, &submitReq->inFileSpool, MAXFILENAMELEN) && xdr_string (xdrs, &submitReq->commandSpool, MAXFILENAMELEN))) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &submitReq->userPriority)) {
        goto Error1;  // FIXME FIXME FIXME FIXME remove goto
    }

    return TRUE;

Error1:
    if (xdrs->x_op == XDR_DECODE) {
        FREEUP (submitReq->xf);
        submitReq->xf = NULL;
    }

Error0:
    if (xdrs->x_op == XDR_DECODE)
    {
        for ( uint i = 0; i < submitReq->numAskedHosts; i++) {
            FREEUP (submitReq->askedHosts[i]);
        }
        FREEUP (submitReq->askedHosts);
        submitReq->askedHosts = NULL;
        submitReq->numAskedHosts = 0;
    }
  return FALSE;
}

bool_t
xdr_modifyReq (XDR *xdrs, struct modifyReq * modifyReq, struct LSFHeader *hdr)
{
    uint jobArrId     = 0;
    uint jobArrElemId = 0;

    if (xdrs->x_op == XDR_DECODE) {
        FREEUP (modifyReq->jobIdStr);
    }
    if (xdrs->x_op == XDR_ENCODE) {
        jobId64To32 (modifyReq->jobId, &jobArrId, &jobArrElemId);
    }

    if (!(xdr_u_int (xdrs, &jobArrId) && xdr_int (xdrs, &(modifyReq->delOptions)))) {
        return FALSE;
    }

    if (!xdr_arrayElement (xdrs, &modifyReq->submitReq, hdr, xdr_submitReq)) {
        return FALSE;
    }

    if (!xdr_var_string (xdrs, &(modifyReq->jobIdStr))) {
        return FALSE;
    }

    if (!(xdr_int (xdrs, &(modifyReq->delOptions2)))) {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &jobArrElemId)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
      jobId32To64 (&modifyReq->jobId, jobArrId, jobArrElemId);
    }

    return TRUE;
}

bool_t
xdr_jobInfoReq (XDR *xdrs, struct jobInfoReq *jobInfoReq, struct LSFHeader *hdr)
{
    uint jobArrId     = 0;
    uint jobArrElemId = 0;

    assert( hdr->length );

    if (!xdr_var_string (xdrs, &jobInfoReq->userName)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_ENCODE)
    {
        jobId64To32 (jobInfoReq->jobId, &jobArrId, &jobArrElemId);
    }

    if (!(xdr_u_int (xdrs, &jobArrId) && xdr_int (xdrs, &(jobInfoReq->options)) && xdr_var_string (xdrs, &jobInfoReq->queue))) {
        return FALSE;
    }

    if (!xdr_var_string (xdrs, &jobInfoReq->host)) {
        return FALSE;
    }

    if (!xdr_var_string (xdrs, &jobInfoReq->jobName)) {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &jobArrElemId))
    {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE)
    {
        jobId32To64 (&jobInfoReq->jobId, jobArrId, jobArrElemId);
    }

  return TRUE;
}

bool_t
xdr_signalReq (XDR *xdrs, struct signalReq *signalReq, struct LSFHeader *hdr)
{
    uint jobArrId     = 0;
    uint jobArrElemId = 0;

    assert( hdr->length ); 	// FIXME if you see an assert about struct LSFHeader *hdr
    						// 		 make sure you check the rest of the function 
    						//		 to see if the header is even present

    if (xdrs->x_op == XDR_ENCODE) {
        jobId64To32 (signalReq->jobId, &jobArrId, &jobArrElemId);
    }

    if (!(xdr_int (xdrs, &(signalReq->sigValue)) && xdr_u_int (xdrs, &jobArrId))) {
        return FALSE;
    }


    if (signalReq->sigValue == SIG_CHKPNT || signalReq->sigValue == SIG_DELETE_JOB || signalReq->sigValue == SIG_ARRAY_REQUEUE)
    {
        if (!(xdr_time_t (xdrs, &(signalReq->chkPeriod)) && xdr_int (xdrs, &(signalReq->actFlags))))
        {
            return FALSE;
        }
    }

    if (!xdr_u_int (xdrs, &jobArrElemId))
    {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE)
    {
        jobId32To64 (&signalReq->jobId, jobArrId, jobArrElemId);
    }

    return TRUE;
}


bool_t
xdr_lsbMsg (XDR *xdrs, struct lsbMsg *m, struct LSFHeader *hdr)
{
    int xdrrc = 0;
    uint jobArrId = 0; 
    uint jobArrElemId = 0;

    assert( hdr->length );

    if (xdrs->x_op == XDR_DECODE)
    {
        m->header->src  = NULL;
        m->header->dest = NULL;
    }

    if (xdrs->x_op == XDR_FREE)
    {
        FREEUP (m->header->src);
        FREEUP (m->header->dest);
        FREEUP (m->msg);
        return TRUE;
    }

    xdrrc = xdr_u_int (xdrs, &m->header->msgId); // msgId should never be negative
    if (!xdrrc) {
        return FALSE;
    }

    xdrrc = xdr_u_int (xdrs, &m->header->usrId);
    if (!xdrrc) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_ENCODE)
    {
        jobId64To32 (m->header->jobId, &jobArrId, &jobArrElemId);
    }
    xdrrc = xdr_u_int (xdrs, &jobArrId);
    if (!xdrrc) {
        return FALSE;
    }

    xdrrc = xdr_int (xdrs, &m->header->type);
    if (!xdrrc) {
        return FALSE;
    }

    xdrrc = xdr_string (xdrs, &m->header->src, LSB_MAX_SD_LENGTH);
    if (!xdrrc) {
        return FALSE;
    }

    xdrrc = xdr_string (xdrs, &m->header->dest, LSB_MAX_SD_LENGTH);
    if (!xdrrc) {
        return FALSE;
    }

    xdrrc = xdr_var_string (xdrs, &m->msg);
    if (!xdrrc) {
        return FALSE;
    }

    xdrrc = xdr_u_int (xdrs, &jobArrElemId);
    if (!xdrrc) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&m->header->jobId, jobArrId, jobArrElemId);
    }

    return TRUE;
}

bool_t
xdr_submitMbdReply (XDR *xdrs, struct submitMbdReply *reply, struct LSFHeader *hdr)
{
  // static char queueName[MAX_LSB_NAME_LEN]; // FIXME these oughta be malloc'ed
  //static char jobName[MAX_CMD_DESC_LEN];
	char *queueName = NULL;
	char *jobName = NULL;
	uint jobArrId = 0;
	uint jobArrElemId = 0;

	assert( hdr->length );

	if (xdrs->x_op == XDR_DECODE)
    {
		reply->queue = NULL;
		reply->badJobName = NULL;
    }

	if (xdrs->x_op == XDR_ENCODE)
    {
		jobId64To32 (reply->jobId, &jobArrId, &jobArrElemId);
    }
	if (!(xdr_u_int (xdrs, &(jobArrId)) &&
		xdr_int (xdrs, &(reply->badReqIndx)) &&
		xdr_string (xdrs, &reply->queue, MAX_LSB_NAME_LEN) &&
  		xdr_string (xdrs, &reply->badJobName, MAX_CMD_DESC_LEN)))
    	{
    		return FALSE;
    	};

  	if (!xdr_u_int (xdrs, &jobArrElemId))
    {
		return FALSE;
    }

	if (xdrs->x_op == XDR_DECODE)
    {
		jobId32To64 (&reply->jobId, jobArrId, jobArrElemId);
    }

	return TRUE;
}

bool_t
xdr_parameterInfo (XDR *xdrs, struct parameterInfo *paramInfo, struct LSFHeader *hdr)
{

    assert( hdr->length );

	if (xdrs->x_op == XDR_DECODE)
    {
		FREEUP (paramInfo->defaultQueues);
		FREEUP (paramInfo->defaultHostSpec);
		FREEUP (paramInfo->defaultProject);
		FREEUP (paramInfo->pjobSpoolDir);
    }

  if (!(xdr_var_string (xdrs, &paramInfo->defaultQueues) &&
  		xdr_var_string (xdrs, &paramInfo->defaultHostSpec)))
  		{
    		return FALSE;
  		}

  if (!(xdr_long (xdrs, &(paramInfo->mbatchdInterval)) &&
  			xdr_long (xdrs, &(paramInfo->sbatchdInterval)) &&
  			xdr_int (xdrs, &(paramInfo->jobAcceptInterval)) &&
  			xdr_int (xdrs, &(paramInfo->maxDispRetries)) &&
			xdr_int (xdrs, &(paramInfo->maxSbdRetries)) &&
  			xdr_int (xdrs, &(paramInfo->cleanPeriod)) &&
  			xdr_int (xdrs, &(paramInfo->maxNumJobs)) &&
  			xdr_int (xdrs, &(paramInfo->pgSuspendIt))))
  			{
    			return FALSE;
  			}

    if (!(xdr_var_string (xdrs, &paramInfo->defaultProject))) {
        return FALSE;
    }

    if (!(xdr_int (xdrs, &(paramInfo->maxJobArraySize))))
    {
      return FALSE;
    }

    if (!xdr_int (xdrs, &(paramInfo->jobTerminateInterval)))
        return FALSE;

    if (!xdr_bool (xdrs, (int *)&paramInfo->disableUAcctMap)) {
        return FALSE;
    }

    if (!(xdr_int (xdrs, &(paramInfo->maxJobArraySize)))) {
        return FALSE;
    }

    if (!(xdr_var_string (xdrs, &paramInfo->pjobSpoolDir)))
    {
      return FALSE;
    }

    if (! ( xdr_int (xdrs, &paramInfo->maxUserPriority)  &&
            xdr_int (xdrs, &paramInfo->jobPriorityValue) &&
            xdr_int (xdrs, &paramInfo->jobPriorityTime))
    )
    {
      return FALSE;
    }


	if (!xdr_u_int (xdrs, &(paramInfo->maxJobId)))
    {
      return FALSE;
    }

	if (!( xdr_int (xdrs, &(paramInfo->maxAcctArchiveNum)) &&
		 xdr_int (xdrs, &(paramInfo->acctArchiveInDays)) &&
  		 xdr_int (xdrs, &(paramInfo->acctArchiveInSize))))
    	{
			return FALSE;
		}

  	if (!(xdr_u_int (xdrs, &(paramInfo->jobDepLastSub)) && xdr_int (xdrs, &(paramInfo->sharedResourceUpdFactor))))
    {
      return FALSE;
    }

  return TRUE;
}

bool_t
xdr_jobInfoHead (XDR *xdrs, struct jobInfoHead *jobInfoHead, struct LSFHeader *hdr)
{
  static char **hostNames = NULL;
  static unsigned long numJobs = 0;
  static uint numHosts = 0;
  static LS_LONG_INT *jobIds = NULL;
  char *sp = NULL;
  uint *jobArrIds = NULL;
  uint *jobArrElemIds = NULL;

  assert( hdr->length );

  if (!(xdr_u_long (xdrs, &(jobInfoHead->numJobs)) && xdr_u_int (xdrs, &(jobInfoHead->numHosts))))
    {
      return FALSE;
    }
  if (jobInfoHead->numJobs > 0)
    {
        jobArrIds = calloc (jobInfoHead->numJobs, sizeof (int));
        if ( NULL == jobArrIds && ENOMEM == errno )
  {
    lsberrno = LSBE_NO_MEM;
    return FALSE;
  }
      jobArrElemIds = calloc (jobInfoHead->numJobs, sizeof (int));
      if ( NULL == jobArrElemIds && ENOMEM == errno )
  {
    lsberrno = LSBE_NO_MEM;
    FREEUP (jobArrIds);
    return FALSE;
  }
    }
  if (xdrs->x_op == XDR_DECODE)
    {
      if (jobInfoHead->numJobs > numJobs)
  {
    FREEUP (jobIds);
    numJobs = 0;
    jobIds = calloc (jobInfoHead->numJobs, sizeof (LS_LONG_INT));
    if (NULL == jobIds && ENOMEM == errno )
      {
        lsberrno = LSBE_NO_MEM;
        return FALSE;
      }
    numJobs = jobInfoHead->numJobs;
  }

      if (jobInfoHead->numHosts > numHosts)
  {
    for ( uint i = 0; i < numHosts; i++) {
      FREEUP (hostNames[i]);
    }
    numHosts = 0;
    FREEUP (hostNames);
    hostNames = calloc (jobInfoHead->numHosts, sizeof (char *));
    if (NULL == hostNames && ENOMEM == errno )
      {
        lsberrno = LSBE_NO_MEM;
        return FALSE;
      }
    for ( uint i = 0; i < jobInfoHead->numHosts; i++)
      {
        hostNames[i] = malloc (MAXHOSTNAMELEN);
        if (!hostNames[i])
    {
      numHosts = i;
      lsberrno = LSBE_NO_MEM;
      return FALSE;
    }
      }
    numHosts = jobInfoHead->numHosts;
  }
      jobInfoHead->jobIds = jobIds;
      jobInfoHead->hostNames = hostNames;
    }

  for (uint i = 0; i < jobInfoHead->numJobs; i++)
    {
      if (xdrs->x_op == XDR_ENCODE)
  {
    assert( jobArrIds[i] <= INT_MAX );
    assert( jobArrElemIds[i] <= INT_MAX );
    jobId64To32 (jobInfoHead->jobIds[i], &jobArrIds[i], &jobArrElemIds[i]);
  }
      if (!xdr_u_int (xdrs, &(jobArrIds[i])))
  return FALSE;
    }

  for (uint i = 0; i < jobInfoHead->numHosts; i++)
    {
      sp = jobInfoHead->hostNames[i];
      if (xdrs->x_op == XDR_DECODE) {
            sp[0] = '\0';
      }
      if (!(xdr_string (xdrs, &sp, MAXHOSTNAMELEN))) {
            return FALSE;
      }
    }

  for ( uint i = 0; i < jobInfoHead->numJobs; i++) {
      if (!xdr_u_int (xdrs, &jobArrElemIds[i]))
  {
    return FALSE;
  }
      if (xdrs->x_op == XDR_DECODE)
  {
    assert( jobArrIds[i] <= INT_MAX );
    assert( jobArrElemIds[i] <= INT_MAX );
    jobId32To64 (&jobInfoHead->jobIds[i], jobArrIds[i], jobArrElemIds[i]);
  }
    }

  FREEUP (jobArrIds);
  FREEUP (jobArrElemIds);
  return TRUE;
}

bool_t
xdr_jgrpInfoReply (XDR *xdrs, struct jobInfoReply * jobInfoReply, struct LSFHeader *hdr)
{
  char *sp = NULL;
  uint jobArrId     = 0;
  uint jobArrElemId = 0;

    assert( hdr->length );

  sp = jobInfoReply->userName;
  if (xdrs->x_op == XDR_DECODE)
    {
      sp = NULL;

      FREEUP (jobInfoReply->jobBill->dependCond);
      FREEUP (jobInfoReply->jobBill->jobName);
    }

  if (xdrs->x_op == XDR_ENCODE)
    {
      jobId64To32 (jobInfoReply->jobId, &jobArrId, &jobArrElemId);
    }
  if (!(xdr_int (xdrs, &jobArrId) &&
  xdr_int (xdrs, &(jobInfoReply->status)) &&
  xdr_int (xdrs, &(jobInfoReply->reasons)) &&
  xdr_int (xdrs, &(jobInfoReply->subreasons)) &&
  xdr_time_t (xdrs, &(jobInfoReply->startTime)) &&
  xdr_time_t (xdrs, &(jobInfoReply->predictedStartTime)) &&
  xdr_time_t (xdrs, &(jobInfoReply->endTime)) &&
  xdr_float (xdrs, &(jobInfoReply->cpuTime)) &&
  xdr_u_int (xdrs, &(jobInfoReply->numToHosts)) &&
  xdr_u_int (xdrs, &jobInfoReply->userId) &&
  xdr_string (xdrs, &sp, MAXLSFNAMELEN)))
    {
      return FALSE;
    }


  if (!(xdr_var_string (xdrs, &jobInfoReply->jobBill->dependCond) &&
  xdr_time_t (xdrs, &jobInfoReply->jobBill->submitTime) &&
  xdr_var_string (xdrs, &jobInfoReply->jobBill->jobName)))
    {
      FREEUP (jobInfoReply->jobBill->dependCond);
      FREEUP (jobInfoReply->jobBill->jobName);
      return FALSE;
    }

  if (!xdr_int (xdrs, &jobArrElemId))
    {
      return FALSE;
    }

  if (xdrs->x_op == XDR_DECODE)
    {
      jobId32To64 (&jobInfoReply->jobId, jobArrId, jobArrElemId);
    }
  return TRUE;

}

bool_t
xdr_jobInfoReply (XDR *xdrs, struct jobInfoReply *jobInfoReply, struct LSFHeader *hdr)
{
  char *sp = NULL;
  static float *loadSched = NULL, *loadStop = NULL;
  static uint nIdx = 0;
  static int *reasonTb = NULL;
  static uint nReasons = 0;
  int jobArrId = 0;
  int jobArrElemId = 0;


  if (!(xdr_int (xdrs, &jobInfoReply->jType) &&
  xdr_var_string (xdrs, &jobInfoReply->parentGroup) &&
  xdr_var_string (xdrs, &jobInfoReply->jName)))
    {
      FREEUP (jobInfoReply->parentGroup);
      FREEUP (jobInfoReply->jName);
      return FALSE;
    }
  for (uint i = 0; i < NUM_JGRP_COUNTERS; i++)
    {
      if (!xdr_int (xdrs, &jobInfoReply->counter[i]))
  {
    FREEUP (jobInfoReply->parentGroup);
    FREEUP (jobInfoReply->jName);
    return FALSE;
  }
    }
  if (jobInfoReply->jType == JGRP_NODE_GROUP)
    {
      return (xdr_jgrpInfoReply (xdrs, jobInfoReply, hdr));
    }


  sp = jobInfoReply->userName;
	if (xdrs->x_op == XDR_DECODE) {
		sp = NULL;
	}

  if (xdrs->x_op == XDR_ENCODE)
    {
      jobId64To32 (jobInfoReply->jobId, &jobArrId, &jobArrElemId);
    }
  if (!(xdr_int (xdrs, &jobArrId) &&
  xdr_int (xdrs, &(jobInfoReply->status)) &&
  xdr_int (xdrs, &(jobInfoReply->reasons)) &&
  xdr_int (xdrs, &(jobInfoReply->subreasons)) &&
  xdr_time_t (xdrs, &(jobInfoReply->startTime)) &&
  xdr_time_t (xdrs, &(jobInfoReply->endTime)) &&
  xdr_float (xdrs, &(jobInfoReply->cpuTime)) &&
  xdr_u_int (xdrs, &(jobInfoReply->numToHosts)) &&
  xdr_u_int (xdrs, &jobInfoReply->nIdx) &&
  xdr_u_int (xdrs, &jobInfoReply->numReasons) &&
  xdr_u_int (xdrs, &jobInfoReply->userId) &&
  xdr_string (xdrs, &sp, MAXLSFNAMELEN)))
    {
      return FALSE;
    }

  if (xdrs->x_op == XDR_DECODE)
    {
      if (jobInfoReply->nIdx > nIdx)
  {
    if (allocLoadIdx (&loadSched, &loadStop, &nIdx, jobInfoReply->nIdx) == -1) {
      return FALSE;
    }
  }
      jobInfoReply->loadSched = loadSched;
      jobInfoReply->loadStop = loadStop;

      if (jobInfoReply->numReasons > nReasons)
  {
    FREEUP (reasonTb);
    nReasons = 0;
    reasonTb = calloc (jobInfoReply->numReasons, sizeof (int));
    if ( NULL == reasonTb && ENOMEM == errno ) {
        return FALSE;
    }
    jobInfoReply->reasonTb = reasonTb;
    nReasons = jobInfoReply->numReasons;
  }
    }

    for ( uint i = 0; i < jobInfoReply->nIdx; i++)
    {
        if (!xdr_float (xdrs, &jobInfoReply->loadSched[i]) || !xdr_float (xdrs, &jobInfoReply->loadStop[i])) {
            return FALSE;
        }
    }

    for ( uint i = 0; i < jobInfoReply->numReasons; i++) {
        if (!xdr_int (xdrs, &jobInfoReply->reasonTb[i])) {
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_DECODE && jobInfoReply->numToHosts > 0)
    {
        jobInfoReply->toHosts = calloc (jobInfoReply->numToHosts, sizeof (char *));
        if ( NULL == jobInfoReply->toHosts && ENOMEM == errno ) {
            jobInfoReply->numToHosts = 0;
            return FALSE;
        }
    }
    for ( uint i = 0; i < jobInfoReply->numToHosts; i++)
    {
      if (xdrs->x_op == XDR_DECODE)
  {
    jobInfoReply->toHosts[i] = calloc (1, MAXHOSTNAMELEN);
    if (jobInfoReply->toHosts[i] == NULL)
      {
        for ( uint j = 0; j < i; j++) {
    		free (jobInfoReply->toHosts[j]);
        }
        free (jobInfoReply->toHosts);
        jobInfoReply->numToHosts = 0;
        jobInfoReply->toHosts = NULL;
        return FALSE;
      }
  }
      sp = jobInfoReply->toHosts[i];
      if (!xdr_string (xdrs, &sp, MAXHOSTNAMELEN))
  {
    if (xdrs->x_op == XDR_DECODE)
	{
		for ( uint j = 0; j < i; j++) {
    		free (jobInfoReply->toHosts[j]);
		}
        free (jobInfoReply->toHosts);
        jobInfoReply->numToHosts = 0;
        jobInfoReply->toHosts = NULL;
      }
    return FALSE;
  }
    }

  if (!xdr_arrayElement (xdrs, (jobInfoReply->jobBill), hdr, xdr_submitReq))
    {
      if (xdrs->x_op == XDR_DECODE)
  {
    for ( uint j = 0; j < jobInfoReply->numToHosts; j++) {
      free (jobInfoReply->toHosts[j]);
    }
    free (jobInfoReply->toHosts);
    jobInfoReply->numToHosts = 0;
    jobInfoReply->toHosts = NULL;
  }
      return FALSE;
    }

  if (!(xdr_u_int (xdrs,  &jobInfoReply->exitStatus) &&
  xdr_u_int (xdrs, &jobInfoReply->execUid) &&
  xdr_var_string (xdrs, &jobInfoReply->execHome) &&
  xdr_var_string (xdrs, &jobInfoReply->execCwd) &&
  xdr_var_string (xdrs, &jobInfoReply->execUsername)))
    {
      FREEUP (jobInfoReply->execHome);
      FREEUP (jobInfoReply->execCwd);
      FREEUP (jobInfoReply->execUsername);
      return FALSE;
    }

    if (!xdr_time_t (xdrs, &jobInfoReply->reserveTime) || !xdr_u_long (xdrs, &jobInfoReply->jobPid)) {
        return FALSE;
    }


    if (!(xdr_time_t (xdrs, &(jobInfoReply->jRusageUpdateTime)))) {
        return FALSE;
    }
    if (!(xdr_jRusage (xdrs, &(jobInfoReply->runRusage), hdr))) {
        return FALSE;
    }


  if (!xdr_time_t (xdrs, &(jobInfoReply->predictedStartTime)))
    {
      return FALSE;
    }


  if (!xdr_u_short (xdrs, &jobInfoReply->port))
    {
      return FALSE;
    }

  if (!xdr_int (xdrs, &jobInfoReply->jobPriority))
    {
      return FALSE;
    }

  if (!xdr_int (xdrs, &jobArrElemId))
    {
      return FALSE;
    }

  if (xdrs->x_op == XDR_DECODE)
    {
      jobId32To64 (&jobInfoReply->jobId, jobArrId, jobArrElemId);
    }

  return TRUE;

}

bool_t
xdr_queueInfoReply (XDR *xdrs, struct queueInfoReply * qInfoReply, struct LSFHeader *hdr)
{
    static uint memSize = 0;
    static uint nIdx = 0;
    static float *loadStop = NULL;
    static float *loadSched = NULL;
    static struct queueInfoEnt *qInfo = NULL;

    if (!(xdr_u_int (xdrs, &(qInfoReply->numQueues)) && xdr_u_int (xdrs, &(qInfoReply->badQueue)) && xdr_u_int (xdrs, &qInfoReply->nIdx))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        if (qInfoReply->numQueues > memSize) {
            for ( uint i = 0; i < memSize; i++) {
                FREEUP (qInfo[i].queue);
                FREEUP (qInfo[i].hostSpec);
            }
    
        FREEUP (qInfo);
        memSize = 0;
        if (!(qInfo = (struct queueInfoEnt *) calloc (qInfoReply->numQueues, sizeof (struct queueInfoEnt))))
            return FALSE;

        for ( uint i = 0; i < qInfoReply->numQueues; i++) {
                qInfo[i].queue = NULL;
                qInfo[i].hostSpec = NULL;
                memSize = i + 1;
            if (!(qInfo[i].queue = malloc (MAX_LSB_NAME_LEN)) || !(qInfo[i].hostSpec = malloc (MAX_LSB_NAME_LEN))) { 
                return FALSE;
            }
        }
    }


    for ( uint i = 0; i < qInfoReply->numQueues; i++) {
        FREEUP (qInfo[i].description);
        FREEUP (qInfo[i].userList);
        FREEUP (qInfo[i].hostList);
        FREEUP (qInfo[i].windows);
        FREEUP (qInfo[i].windowsD);
        FREEUP (qInfo[i].defaultHostSpec);
        FREEUP (qInfo[i].admins);
        FREEUP (qInfo[i].preCmd);
        FREEUP (qInfo[i].postCmd);
        FREEUP (qInfo[i].prepostUsername);
        FREEUP (qInfo[i].requeueEValues);
        FREEUP (qInfo[i].resReq);
        FREEUP (qInfo[i].resumeCond);
        FREEUP (qInfo[i].stopCond);
        FREEUP (qInfo[i].jobStarter);
        FREEUP (qInfo[i].chkpntDir);


        FREEUP (qInfo[i].suspendActCmd);
        FREEUP (qInfo[i].resumeActCmd);
        FREEUP (qInfo[i].terminateActCmd);
    }

      qInfoReply->queues = qInfo;


      if (qInfoReply->numQueues * qInfoReply->nIdx > nIdx && qInfoReply->numQueues > 0)
  {
    assert( nIdx <= INT_MAX );
    if (allocLoadIdx (&loadSched, &loadStop, &nIdx, qInfoReply->numQueues * qInfoReply->nIdx) == -1)
      return FALSE;
  }
    }

  for ( uint i = 0; i < qInfoReply->numQueues; i++)
    {
      if (xdrs->x_op == XDR_DECODE)
  {
    qInfoReply->queues[i].loadSched = loadSched + (i * qInfoReply->nIdx);
    qInfoReply->queues[i].loadStop = loadStop + (i * qInfoReply->nIdx);
  }

      if (!xdr_arrayElement (xdrs, &(qInfoReply->queues[i]), hdr, xdr_queueInfoEnt, &qInfoReply->nIdx)) {
            return FALSE;
      }
    }

  return TRUE;

}

bool_t
xdr_queueInfoEnt (XDR *xdrs, struct queueInfoEnt * qInfo, struct LSFHeader *hdr, uint *nIdx)
{
    char *sp = NULL;

    assert( hdr->length );

    if (xdrs->x_op == XDR_FREE)
    {
        if (qInfo->chkpntDir != 0)
        {
            FREEUP (qInfo->chkpntDir);
        }
        return TRUE;
    }

    sp = qInfo->queue;
    if (xdrs->x_op == XDR_DECODE)
    {
        sp = NULL;
        qInfo->suspendActCmd = NULL;
        qInfo->resumeActCmd = NULL;
        qInfo->terminateActCmd = NULL;
    }

    if (!(xdr_string (xdrs, &sp, MAX_LSB_NAME_LEN) && xdr_var_string (xdrs, &qInfo->description)))
        return FALSE;

    if (!(xdr_var_string (xdrs, &qInfo->userList))) {
        return FALSE;
    }

    if (!(xdr_var_string (xdrs, &qInfo->hostList))) {
        return FALSE;
    }

    if (!(xdr_var_string (xdrs, &qInfo->windows))) {
        return FALSE;
    }

    sp = qInfo->hostSpec;
    if (xdrs->x_op == XDR_DECODE) {
        sp = NULL;
    }
    if (!(xdr_string (xdrs, &sp, MAX_LSB_NAME_LEN))) {
        return FALSE;
    }

    for ( uint i = 0; i < LSF_RLIM_NLIMITS; i++)
    {
        if (!(xdr_int (xdrs, &qInfo->rLimits[i])))
            return FALSE;
    }

    qInfo->nIdx = *nIdx;
    for ( uint i = 0; i < *nIdx; i++)
    {
        if (!(xdr_float (xdrs, &(qInfo->loadSched[i]))) || !(xdr_float (xdrs, &(qInfo->loadStop[i]))))
            return FALSE;
    }

    if (! ( xdr_int (xdrs, &qInfo->priority)       &&
            xdr_int (xdrs, &qInfo->nice)         &&
            xdr_int (xdrs, &qInfo->userJobLimit)   &&
            xdr_float (xdrs, &qInfo->procJobLimit) &&
            xdr_int (xdrs, &qInfo->qAttrib)        &&
            xdr_int (xdrs, &qInfo->qStatus)        &&
            xdr_int (xdrs, &qInfo->maxJobs)        &&
            xdr_int (xdrs, &qInfo->numJobs)        &&
            xdr_int (xdrs, &qInfo->numPEND)        &&
            xdr_int (xdrs, &qInfo->numRUN)         &&
            xdr_int (xdrs, &qInfo->numSSUSP)       &&
            xdr_int (xdrs, &qInfo->numUSUSP)       &&
            xdr_int (xdrs, &qInfo->mig)            &&
            xdr_int (xdrs, &qInfo->acceptIntvl)    &&
            xdr_int (xdrs, &qInfo->schedDelay))
        )
    {
      return FALSE;
    }

    if (!(xdr_var_string (xdrs, &qInfo->windowsD) && xdr_var_string (xdrs, &qInfo->defaultHostSpec))) {
        return FALSE;
    }

    if( ! ( xdr_int (xdrs, &qInfo->procLimit) &&
            xdr_var_string (xdrs, &qInfo->admins) &&
            xdr_var_string (xdrs, &qInfo->preCmd) &&
            xdr_var_string (xdrs, &qInfo->postCmd) &&
            xdr_var_string (xdrs, &qInfo->prepostUsername) &&
            xdr_var_string (xdrs, &qInfo->requeueEValues) &&
            xdr_int (xdrs, &qInfo->hostJobLimit))
    ) 
    {
        return FALSE;
    }

    if( ! ( xdr_var_string (xdrs, &qInfo->resReq) &&
            xdr_int (xdrs, &qInfo->numRESERVE) &&
            xdr_int (xdrs, &qInfo->slotHoldTime) &&
            xdr_var_string (xdrs, &qInfo->resumeCond) &&
            xdr_var_string (xdrs, &qInfo->stopCond) &&
            xdr_var_string (xdrs, &qInfo->jobStarter) &&
            xdr_var_string (xdrs, &qInfo->suspendActCmd) &&
            xdr_var_string (xdrs, &qInfo->resumeActCmd) &&
            xdr_var_string (xdrs, &qInfo->terminateActCmd))
    )
    {
        return FALSE;
    }

    for ( uint j = 0; j < LSB_SIG_NUM; j++)
    {
        if (!xdr_int (xdrs, &qInfo->sigMap[j])) {
            return FALSE;
        }
    }

    if (!(xdr_var_string (xdrs, &qInfo->chkpntDir) && xdr_int (xdrs, &qInfo->chkpntPeriod))) {
        return FALSE;
    }

    for ( uint i = 0; i < LSF_RLIM_NLIMITS; i++)
    {
        if (!(xdr_int (xdrs, &qInfo->defLimits[i]))) {
            return FALSE;
        }
    }

    if (!(xdr_int (xdrs, &qInfo->minProcLimit) && xdr_int (xdrs, &qInfo->defProcLimit)))
    {
      return FALSE;
    }

    return TRUE;
}

bool_t
xdr_infoReq (XDR *xdrs, struct infoReq * infoReq, struct LSFHeader *hdr)
{
    static uint memSize = 0;
    static char **names = NULL;
    static char *resReq = NULL;
    uint counter = 0;

    assert( hdr->length );

    if (!(xdr_int (xdrs, &(infoReq->options)))) {
        return FALSE;
    }

    if (!(xdr_u_int (xdrs, &(infoReq->numNames)))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE)
    {
        if (names)
        {
            for ( uint i = 0; i < memSize; i++) {
                FREEUP (names[i]);
            }
        }

        if (infoReq->numNames + 2 > memSize) {
            FREEUP (names);

            memSize = infoReq->numNames + 2;
            if ((names = calloc (memSize, sizeof (char *))) == NULL)
            {
                memSize = 0;
                return FALSE;
            }
        }
        infoReq->names = names;
    }

    for (uint i = 0; i < infoReq->numNames; i++) {
        if (!(xdr_var_string (xdrs, &infoReq->names[i]))) {
            return FALSE;
        }
    }

    counter =  0;
    if (infoReq->options & CHECK_HOST)
    {
        if (!(xdr_var_string (xdrs, &infoReq->names[counter]))) {
            return FALSE;
        }
        counter++;
    }
    if (infoReq->options & CHECK_USER)
    {
        if (!(xdr_var_string (xdrs, &infoReq->names[counter]))) {
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_DECODE)
    {
      FREEUP (resReq);
    }

    if (!xdr_var_string (xdrs, &infoReq->resReq)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE)
    {
        resReq = infoReq->resReq;
    }

  return TRUE;
}

bool_t
xdr_hostDataReply (XDR *xdrs, struct hostDataReply *hostDataReply, struct LSFHeader *hdr)
{

  uint hostCount = 0;
  char *sp      = NULL;
  struct hostInfoEnt *hInfoTmp     = NULL;
  static struct hostInfoEnt *hInfo = NULL;
  static uint curNumHInfo          = 0;
  static char *mem                 = NULL;
  static float *loadSched = NULL; 
  static float *loadStop  = NULL;
  static float *load      = NULL;
  static float *realLoad  = NULL;
  static uint  *nIdx      = 0;
  static int *busySched   = NULL;
  static int *busyStop    = NULL;

    if (!xdr_u_int (xdrs, &hostDataReply->numHosts) || !xdr_u_int (xdrs, &hostDataReply->badHost) || !xdr_u_int (xdrs, &hostDataReply->nIdx)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        hostCount = hostDataReply->numHosts;

        if (curNumHInfo < hostCount) {

            hInfoTmp = calloc (hostCount, sizeof (struct hostInfoEnt));
            if ( NULL == hInfoTmp && ENOMEM == errno ) {
                lsberrno = LSBE_NO_MEM;
                return FALSE;
            }

            sp = malloc (hostCount * (MAXHOSTNAMELEN + MAXLINELEN));
            if ( NULL == sp && ENOMEM == errno ) {
                FREEUP (hInfoTmp);
                lsberrno = LSBE_NO_MEM;
                return FALSE;
            }
            
            if (hInfo != NULL) {
                FREEUP (mem);
                FREEUP (hInfo);
            }
            hInfo = hInfoTmp;
            curNumHInfo = hostCount;
            mem = sp;
            for ( uint i = 0; i < hostCount; i++ ) {
                hInfo[i].host = sp;
                sp += MAXHOSTNAMELEN;
                hInfo[i].windows = sp;
                sp += MAXLINELEN;
            }
        }

        hostDataReply->hosts = hInfo;
        if (hostDataReply->nIdx * hostDataReply->numHosts > *nIdx) {
            int returnStatus = allocLoadIdx (&loadSched, &loadStop, nIdx, hostDataReply->nIdx * hostDataReply->numHosts);
            if ( -1 == returnStatus ) {
                return FALSE;
            }
        }
        FREEUP (load);
        FREEUP (realLoad);
        FREEUP (busySched);
        FREEUP (busyStop);

        realLoad = malloc (hostDataReply->nIdx * hostDataReply->numHosts * sizeof (float));
        if (hostDataReply->numHosts > 0 && NULL == realLoad && ENOMEM == errno ) {
            return FALSE;
        }
        
        load = malloc (hostDataReply->nIdx * hostDataReply->numHosts * sizeof (float));
        if (hostDataReply->numHosts > 0 &&  NULL == load && ENOMEM == errno ) {
            return FALSE;
        }
        busySched = malloc (GET_INTNUM (hostDataReply->nIdx) * hostDataReply->numHosts * sizeof (int));
        if (hostDataReply->numHosts > 0 && NULL == busySched && ENOMEM == errno ) {
            return FALSE;
        }
        busyStop = malloc (GET_INTNUM (hostDataReply->nIdx) * hostDataReply->numHosts * sizeof (int));
        if (hostDataReply->numHosts > 0 && NULL == busyStop && ENOMEM == errno ) {
            return FALSE;
        }
    }

    for ( uint i = 0; i < hostDataReply->numHosts; i++)
    {
        if (xdrs->x_op == XDR_DECODE) {
            hostDataReply->hosts[i].loadSched = loadSched + (i * hostDataReply->nIdx);
            hostDataReply->hosts[i].loadStop  = loadStop + (i * hostDataReply->nIdx);
            hostDataReply->hosts[i].realLoad  = realLoad + (i * hostDataReply->nIdx);
            hostDataReply->hosts[i].load      = load + (i * hostDataReply->nIdx);
            hostDataReply->hosts[i].busySched = busySched + (i * GET_INTNUM (hostDataReply->nIdx));
            hostDataReply->hosts[i].busyStop  = busyStop + (i * GET_INTNUM (hostDataReply->nIdx));
        }

        if (!xdr_arrayElement (xdrs, &(hostDataReply->hosts[i]), hdr, xdr_hostInfoEnt, (char *) &hostDataReply->nIdx)) {
            return FALSE;
        }
    }

    if (!xdr_int (xdrs, &(hostDataReply->flag))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        if (hostDataReply->flag & LOAD_REPLY_SHARED_RESOURCE) {
            lsbSharedResConfigured_ = TRUE;
        }
    }

    return TRUE;
}

bool_t
xdr_hostInfoEnt (XDR *xdrs, struct hostInfoEnt *hostInfoEnt, struct LSFHeader *hdr, uint *nIdx)
{
    char *sp = hostInfoEnt->host;
    char *wp = hostInfoEnt->windows;

    assert( hdr->length );
    if (xdrs->x_op == XDR_DECODE)
    {
        sp = NULL;
        wp = NULL;
    }

    if ( ! (xdr_string (xdrs, &sp, MAXHOSTNAMELEN)        &&
            xdr_double (xdrs, &hostInfoEnt->cpuFactor)    &&
            xdr_string (xdrs, &wp, MAXLINELEN)            &&
            xdr_u_long (xdrs, &hostInfoEnt->userJobLimit) &&
            xdr_u_long (xdrs, &hostInfoEnt->maxJobs)      &&
            xdr_u_long (xdrs, &hostInfoEnt->numJobs)      &&
            xdr_u_long (xdrs, &hostInfoEnt->numRUN)       &&
            xdr_u_long (xdrs, &hostInfoEnt->numSSUSP)     &&
            xdr_u_long (xdrs, &hostInfoEnt->numUSUSP)     &&
            xdr_int    (xdrs, &hostInfoEnt->hStatus)      &&
            xdr_int    (xdrs, &hostInfoEnt->attr)         &&
            xdr_int    (xdrs, &hostInfoEnt->mig))
    )
    {
        return FALSE;
    }

    hostInfoEnt->nIdx = *nIdx;
    for ( uint i = 0; i < GET_INTNUM (*nIdx); i++)
    {
        if (!xdr_int (xdrs, &hostInfoEnt->busySched[i])|| !xdr_int (xdrs, &hostInfoEnt->busyStop[i])) {
            return FALSE;
        }
    }

    for ( uint i = 0; i < *nIdx; i++)
    {
        if (!xdr_float (xdrs, &hostInfoEnt->loadSched[i]) || !xdr_float (xdrs, &hostInfoEnt->loadStop[i]))
            return FALSE;
    }

    for ( uint i = 0; i < *nIdx; i++)
    {
        if (!xdr_float (xdrs, &hostInfoEnt->realLoad[i])) {
            return FALSE;
        }
        if (!xdr_float (xdrs, &hostInfoEnt->load[i])) {
            return FALSE;
        }
    }

    if (!xdr_int (xdrs, &hostInfoEnt->numRESERVE)) {
        return FALSE;
    }

  return TRUE;
}

bool_t
xdr_userInfoReply (XDR *xdrs, struct userInfoReply *userInfoReply, struct LSFHeader *hdr)
{
    struct userInfoEnt *uInfoTmp = NULL;
    char *sp = NULL;
    static struct userInfoEnt *uInfo;
    static uint curNumUInfo;
    static char *mem;
    uint *badUser = 0;
    uint *numUsers = 0;

    assert( userInfoReply->numUsers <= INT_MAX );
    *numUsers = userInfoReply->numUsers;
    assert( userInfoReply->badUser  <= INT_MAX );
    *badUser  = userInfoReply->badUser;
    if ( !xdr_u_int (xdrs, numUsers) && !xdr_uint (xdrs, badUser ) ) {
        return FALSE;
    }
    assert( *badUser >= 0 );
    userInfoReply->badUser = *badUser;
    assert( *numUsers >= 0 );
    userInfoReply->numUsers = *numUsers;


  if (xdrs->x_op == XDR_DECODE)
    {
      if (curNumUInfo < userInfoReply->numUsers)
  {
    uInfoTmp = calloc (userInfoReply->numUsers, sizeof (struct userInfoEnt));
    if (NULL == uInfoTmp && ENOMEM == errno )
      {
        lsberrno = LSBE_NO_MEM;
        return FALSE;
      }
    if (!(sp = malloc (userInfoReply->numUsers * MAX_LSB_NAME_LEN)))
      {
        free (uInfoTmp);
        lsberrno = LSBE_NO_MEM;
        return FALSE;
      }
    if (uInfo != NULL)
      {
        free (mem);
        free (uInfo);
      }
    uInfo = uInfoTmp;
    curNumUInfo = userInfoReply->numUsers;
    mem = sp;
    for (uint i = 0; i < userInfoReply->numUsers; i++)
      {
        uInfo[i].user = sp;
        sp += MAX_LSB_NAME_LEN;
      }
  }
      userInfoReply->users = uInfo;
    }

    for ( uint i = 0; i < userInfoReply->numUsers; i++) {
    	// FIXME throw the debugger to make sure that .user is what is needed here
        if (!xdr_arrayElement (xdrs, userInfoReply->users[i].user, hdr, xdr_userInfoEnt)) {
            return FALSE;
        }
    }

  return TRUE;

}

bool_t
xdr_userInfoEnt (XDR *xdrs, struct userInfoEnt *userInfoEnt, struct LSFHeader *hdr)
{
    char *sp = NULL;

    assert( hdr->length );

    sp = userInfoEnt->user;
    if (xdrs->x_op == XDR_DECODE) {
        sp = NULL;
    }

    if (!(xdr_string (xdrs, &sp, MAX_LSB_NAME_LEN) && xdr_float (xdrs, &userInfoEnt->procJobLimit) && xdr_int (xdrs, &userInfoEnt->maxJobs) && xdr_int (xdrs, &userInfoEnt->numStartJobs))) {
        return FALSE;
    }


    if (!(xdr_int (xdrs, &userInfoEnt->numJobs) && xdr_int (xdrs, &userInfoEnt->numPEND) && xdr_int (xdrs, &userInfoEnt->numRUN) && xdr_int (xdrs, &userInfoEnt->numSSUSP) && xdr_int (xdrs, &userInfoEnt->numUSUSP))) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &userInfoEnt->numRESERVE)) {
        return FALSE;
    }

  return TRUE;
}


bool_t
xdr_jobPeekReq (XDR *xdrs, struct jobPeekReq *jobPeekReq, struct LSFHeader *hdr)
{

  uint jobArrId = 0;
  uint jobArrElemId = 0;

  assert( hdr->length );

  if (xdrs->x_op == XDR_ENCODE)
    {
      jobId64To32 (jobPeekReq->jobId, &jobArrId, &jobArrElemId);
    }
    if (!xdr_u_int (xdrs, &jobArrId)) {
        return FALSE;
    }

  if (!xdr_u_int (xdrs, &jobArrElemId))
    {
      return FALSE;
    }

  if (xdrs->x_op == XDR_DECODE)
    {
      jobId32To64 (&jobPeekReq->jobId, jobArrId, jobArrElemId);
    }

  return TRUE;
}

bool_t
xdr_jobPeekReply (XDR *xdrs, struct jobPeekReply * jobPeekReply, struct LSFHeader *hdr)
{

    assert( hdr->length );
  static char outFile[MAXFILENAMELEN];

  if (xdrs->x_op == XDR_DECODE)
    {
      outFile[0] = '\0';
      jobPeekReply->outFile = outFile;
    }

    if (!xdr_string (xdrs, &(jobPeekReply->outFile), MAXFILENAMELEN)) {
        return FALSE;
    }


  if (!(xdr_var_string (xdrs, &jobPeekReply->pSpoolDir)))
    {
      return FALSE;
    }

  return TRUE;
}


bool_t
xdr_groupInfoReply (XDR *xdrs, struct groupInfoReply *groupInfoReply, struct LSFHeader *hdr)
{
    uint *numGroups = 0;

    if (xdrs->x_op == XDR_FREE) {
        for ( uint i = 0; i < groupInfoReply->numGroups; i++) {
            FREEUP (groupInfoReply->groups[i].group);
            FREEUP (groupInfoReply->groups[i].memberList);
        }

        FREEUP (groupInfoReply->groups);
        groupInfoReply->numGroups = 0;
        return TRUE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        groupInfoReply->numGroups = 0;
        groupInfoReply->groups = NULL;
    }

    assert( groupInfoReply->numGroups <= INT_MAX );
    *numGroups = groupInfoReply->numGroups;
    if ( !xdr_u_int (xdrs, numGroups) ) {
        return FALSE;
    }
    assert( *numGroups >= 0);
    groupInfoReply->numGroups = *numGroups;

    if (xdrs->x_op == XDR_DECODE && groupInfoReply->numGroups != 0)
    {

        groupInfoReply->groups = calloc (groupInfoReply->numGroups, sizeof (struct groupInfoEnt));
        if ( NULL == groupInfoReply->groups ) {
            return FALSE;
        }

    }

    for ( uint i = 0; i < groupInfoReply->numGroups; i++)
    {
        if (!xdr_arrayElement (xdrs, &(groupInfoReply->groups[i]), hdr, xdr_groupInfoEnt)) {
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_FREE) {
        FREEUP (groupInfoReply->groups);
    }

    return TRUE;
}


bool_t
xdr_groupInfoEnt (XDR *xdrs, struct groupInfoEnt *gInfo, struct LSFHeader *hdr)
{
    assert( hdr->length );

    if (xdrs->x_op == XDR_DECODE)
    {
      gInfo->group = NULL;
      gInfo->memberList = NULL;
    }

    if (!xdr_var_string (xdrs, &(gInfo->group)) || !xdr_var_string (xdrs, &(gInfo->memberList))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_FREE)
    {
        FREEUP (gInfo->group);
        FREEUP (gInfo->memberList);
    }

    return TRUE;
}

bool_t
xdr_controlReq (XDR *xdrs, struct controlReq *controlReq, struct LSFHeader *hdr)
{
  static char *sp = NULL;
  static int first = TRUE;

  assert( hdr->length );

  if (xdrs->x_op == XDR_DECODE)
    {
      if (first == TRUE)
  {
    sp = malloc (MAXHOSTNAMELEN);
    if( NULL == sp ) {
        return FALSE;
    }
    first = FALSE;
  }
      controlReq->name = sp;
      free(sp);
    }
  else
    {
      sp = controlReq->name;
    }
    if (!(xdr_int (xdrs, &controlReq->opCode) && xdr_string (xdrs, &sp, MAXHOSTNAMELEN)))  {
        return FALSE;
    }

  return TRUE;
}

bool_t
xdr_jobSwitchReq (XDR *xdrs, struct jobSwitchReq * jobSwitchReq, struct LSFHeader *hdr)
{
    char *sp = NULL;
    uint jobArrId = 0;
    uint jobArrElemId = 0;

    assert( hdr->length );

    sp = jobSwitchReq->queue;
    if (xdrs->x_op == XDR_DECODE) {
        sp = NULL;
    }

    if (xdrs->x_op == XDR_ENCODE)
    {
        jobId64To32 (jobSwitchReq->jobId, &jobArrId, &jobArrElemId);
    }
    if (!(xdr_u_int (xdrs, &jobArrId) && xdr_string (xdrs, &sp, MAX_LSB_NAME_LEN))) {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &jobArrElemId)) {
      return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&jobSwitchReq->jobId, jobArrId, jobArrElemId);
    }

    return TRUE;
}

bool_t
xdr_jobMoveReq (XDR *xdrs, struct jobMoveReq * jobMoveReq, struct LSFHeader *hdr)
{
    uint jobArrId     = 0;
    uint jobArrElemId = 0;

    assert( hdr->length );

    if (xdrs->x_op == XDR_ENCODE)
    {
      jobId64To32 (jobMoveReq->jobId, &jobArrId, &jobArrElemId);
    }
    if (!(xdr_u_int (xdrs, &(jobMoveReq->opCode)) && xdr_u_int (xdrs, &jobArrId) && xdr_int (xdrs, &(jobMoveReq->position)))) {
        return FALSE;
    }
    if (!xdr_u_int (xdrs, &jobArrElemId))
    {
      return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE)
    {
      jobId32To64 (&jobMoveReq->jobId, jobArrId, jobArrElemId);
    }

    return TRUE;
}

bool_t
xdr_migReq (XDR *xdrs, struct migReq *req, struct LSFHeader *hdr)
{
    char *sp = NULL;
    uint jobArrId     = 0;
    uint jobArrElemId = 0;
    uint *numAskedHosts = 0;

    assert( hdr->length );

    if( xdrs->x_op == XDR_ENCODE ) {
        jobId64To32 (req->jobId, &jobArrId, &jobArrElemId);
    }

    if( !(xdr_u_int (xdrs, &jobArrId) && xdr_int (xdrs, &req->options)))  {
        return FALSE;
    }

    assert( req->numAskedHosts <= INT_MAX );
    *numAskedHosts = req->numAskedHosts;
    if( !xdr_u_int (xdrs, numAskedHosts) ) {
        return FALSE;
    }
    assert( *numAskedHosts >= 0);
    req->numAskedHosts = *numAskedHosts;

    if (xdrs->x_op == XDR_DECODE && req->numAskedHosts > 0) {
        req->askedHosts = calloc (req->numAskedHosts, sizeof (char *));
        if( NULL == req->askedHosts )  {
            return FALSE;
        }

        for ( uint i = 0; i < req->numAskedHosts; i++) {

            req->askedHosts[i] = calloc (1, MAXHOSTNAMELEN);
            if (req->askedHosts[i] == NULL) {
                while (i--)  {
                    free (req->askedHosts[i]);
                }
                free (req->askedHosts);
                return FALSE;
            }
        }
    }

    for ( uint i = 0; i < req->numAskedHosts; i++)
    {
        sp = req->askedHosts[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp = NULL;
        }
        if (!xdr_string (xdrs, &sp, MAXHOSTNAMELEN)) {
            for ( uint j = 0; j < req->numAskedHosts; j++) {
                free (req->askedHosts[j]);
            }

            free (req->askedHosts);
            return FALSE;
        }
    }

    if (!xdr_u_int (xdrs, &jobArrElemId)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&req->jobId, jobArrId, jobArrElemId);
    }

  return TRUE;
}



static int
allocLoadIdx (float **loadSched, float **loadStop, uint *outSize, uint size)
{
    if (*loadSched) {
        free (*loadSched);
    }
    if (*loadStop) {
        free (*loadStop);
    }

    *loadSched = calloc (size, sizeof ( *loadSched ));
    if ( NULL == *loadSched && ENOMEM == errno )  {
        return (-1);
    }

    *loadStop = calloc (size, sizeof( *loadStop ) );
    if ( NULL == *loadStop && loadStop && ENOMEM == errno ) {
        return (-1);
    }

    *outSize = size;
    return (0);
}


bool_t
xdr_lsbShareResourceInfoReply (XDR *xdrs, struct lsbShareResourceInfoReply *lsbShareResourceInfoReply,  struct LSFHeader *hdr)
{

    int status = 0;
    uint *badResource = 0;
    uint *numResources = 0;

    if (xdrs->x_op == XDR_DECODE) {
        lsbShareResourceInfoReply->numResources = 0;
        lsbShareResourceInfoReply->resources = NULL;
    }

    assert(  lsbShareResourceInfoReply->numResources <= INT_MAX );
    *numResources = lsbShareResourceInfoReply->numResources;
    assert( lsbShareResourceInfoReply->badResource <= INT_MAX );
    *badResource  = lsbShareResourceInfoReply->badResource;
    if (!(xdr_u_int (xdrs, numResources) && xdr_u_int (xdrs, badResource))) {
        return FALSE;
    }
    assert( *badResource  >= 0 );
    lsbShareResourceInfoReply->badResource = *badResource;
    assert( *numResources >= 0 );
    lsbShareResourceInfoReply->numResources = *numResources;

    if (xdrs->x_op == XDR_DECODE && lsbShareResourceInfoReply->numResources > 0) {
        lsbShareResourceInfoReply->resources = malloc (lsbShareResourceInfoReply->numResources * sizeof (struct lsbSharedResourceInfo));
        if ( NULL == lsbShareResourceInfoReply->resources && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return FALSE;
        }
    }

    for ( uint i = 0; i < lsbShareResourceInfoReply->numResources; i++) {
        status = xdr_arrayElement (xdrs, &lsbShareResourceInfoReply->resources[i], hdr, xdr_lsbSharedResourceInfo);
        if (!status) {
            lsbShareResourceInfoReply->numResources = i;
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_FREE && lsbShareResourceInfoReply->numResources > 0) {
        FREEUP (lsbShareResourceInfoReply->resources);
        lsbShareResourceInfoReply->numResources = 0;
    }

    return TRUE;
}


static bool_t
xdr_lsbSharedResourceInfo (XDR *xdrs, struct lsbSharedResourceInfo *lsbSharedResourceInfo, struct LSFHeader *hdr)
{
    int status = 0;
    uint *nInstances = 0;

    if (xdrs->x_op == XDR_DECODE)
    {
      lsbSharedResourceInfo->resourceName = NULL;
      lsbSharedResourceInfo->instances = NULL;
      lsbSharedResourceInfo->nInstances = 0;
    }

    assert( lsbSharedResourceInfo->nInstances <= INT_MAX );
    *nInstances = lsbSharedResourceInfo->nInstances;
    if (!(xdr_var_string (xdrs, &lsbSharedResourceInfo->resourceName) && xdr_u_int (xdrs, nInstances))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE && lsbSharedResourceInfo->nInstances > 0)
    {
        lsbSharedResourceInfo->instances = malloc (lsbSharedResourceInfo->nInstances * sizeof (lsbSharedResourceInfo->instances));
        if ( NULL == lsbSharedResourceInfo->instances && ENOMEM == errno )
        {
            lserrno = LSE_MALLOC;
            return FALSE;
        }
    }

    for ( uint i = 0; i < lsbSharedResourceInfo->nInstances; i++)
    {
        status = xdr_arrayElement (xdrs, &lsbSharedResourceInfo->instances[i], hdr, xdr_lsbShareResourceInstance);
        if (!status)
        {
            lsbSharedResourceInfo->nInstances = i;
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_FREE && lsbSharedResourceInfo->nInstances > 0)
    {
        FREEUP (lsbSharedResourceInfo->instances);
        lsbSharedResourceInfo->nInstances = 0;
    }

    return TRUE;
}

static bool_t
xdr_lsbShareResourceInstance (XDR *xdrs, struct lsbSharedResourceInstance *instance, struct LSFHeader *hdr)
{
    uint *nHosts = 0;

    assert( hdr->length );
  	if (xdrs->x_op == XDR_DECODE)
    {
      instance->totalValue = NULL;
      instance->rsvValue = NULL;
      instance->hostList = NULL;
      instance->nHosts = 0;
    }

    assert( instance->nHosts <= INT_MAX);
    *nHosts = instance->nHosts;
  	if (!(xdr_var_string (xdrs, &instance->totalValue) && xdr_var_string (xdrs, &instance->rsvValue) && xdr_u_int (xdrs, nHosts))) 
  	{
    	return FALSE;
	}

	if (xdrs->x_op == XDR_DECODE && instance->nHosts > 0)
    {
      	if ((instance->hostList = malloc (instance->nHosts * sizeof (char *))) == NULL)
  		{
			lserrno = LSE_MALLOC;
			return FALSE;
		}
    }
	if (!xdr_array_string (xdrs, instance->hostList, MAXHOSTNAMELEN, instance->nHosts))
    {
		if (xdrs->x_op == XDR_DECODE)
  		{
    		FREEUP (instance->hostList);
    		instance->nHosts = 0;
  		}
		return FALSE;
    }
	if (xdrs->x_op == XDR_FREE && instance->nHosts > 0)
    {
      FREEUP (instance->hostList);
      instance->nHosts = 0;
    }
  return TRUE;
}

bool_t
xdr_runJobReq (XDR *xdrs, struct runJobRequest *runJobRequest,  struct LSFHeader *hdr)
{
    uint jobArrId     = 0;
    uint jobArrElemId = 0;
    uint *tempIntPtr  = 0;

    assert( hdr->length );

    if (xdrs->x_op == XDR_ENCODE) {
        jobId64To32 (runJobRequest->jobId, &jobArrId, &jobArrElemId);
    }

    assert( runJobRequest->numHosts <= INT_MAX );
    *tempIntPtr = runJobRequest->numHosts;
    if (!xdr_u_int (xdrs, tempIntPtr) || !xdr_u_int (xdrs, &jobArrId) || !xdr_int (xdrs, &(runJobRequest->options)))
    {
      return FALSE;
    }
    assert( *tempIntPtr >= 0 );
    runJobRequest->numHosts = *tempIntPtr;

    if (xdrs->x_op == XDR_DECODE)
    {
        runJobRequest->hostname = calloc (runJobRequest->numHosts, sizeof (char *));
        if( NULL == runJobRequest->hostname && ENOMEM == errno ) {
            return FALSE;
        }
    }

    for ( uint i = 0; i < runJobRequest->numHosts; i++) {
        if (!xdr_var_string (xdrs, &(runJobRequest->hostname[i])))
        {
            return FALSE;
        }
    }

    if (!xdr_u_int (xdrs, &jobArrElemId))
    {
      return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE)
    {
      jobId32To64 (&runJobRequest->jobId, jobArrId, jobArrElemId);
    }

    if (xdrs->x_op == XDR_FREE)
    {
      runJobRequest->numHosts = 0;
      FREEUP (runJobRequest->hostname);
    }

  return TRUE;
}

bool_t
xdr_jobAttrReq (XDR *xdrs, struct jobAttrInfoEnt * jobAttr, struct LSFHeader *hdr)
{
    char *sp = jobAttr->hostname;
    uint jobArrId     = 0;
    uint jobArrElemId = 0;

    assert( hdr->length );

    if (xdrs->x_op == XDR_ENCODE) {
        jobId64To32 (jobAttr->jobId, &jobArrId, &jobArrElemId);
    }

    if (!xdr_u_int (xdrs, &jobArrId)) {
      return FALSE;
    }

    if (!(xdr_u_short (xdrs, &(jobAttr->port)) && xdr_string (xdrs, &sp, MAXHOSTNAMELEN))) {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &jobArrElemId)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&jobAttr->jobId, jobArrId, jobArrElemId);
    }

  return TRUE;
}

