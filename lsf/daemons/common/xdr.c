/* $Id: daemons.xdr.c 397 2007-11-26 19:04:00Z mblack $
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

#include <string.h>
#include <stdlib.h>

#include "daemons/daemonout.h"
#include "daemons/daemons.h"
#include "lib/xdr.h"

// #define NL_SETN           10
// #define MAX_USER_NAME_LEN 64

extern int xdr_time_t (XDR *xdrs, time_t * t);
extern int xdr_lsfRusage (XDR *xdrs, struct lsfRusage *);
extern void jobId64To32 (size_t, uint *, uint *);
extern void jobId32To64 (size_t *, uint, uint);
static int xdr_thresholds (XDR *xdrs, struct jobSpecs *jp);

bool_t
xdr_jobSpecs (XDR *xdrs, struct jobSpecs *jobSpecs, struct LSFHeader *hdr)
{
    static char __func__] = "xdr_jobSpecs";
    char *sp[15]         = { "a" }; // FIXME wtf
    char *pTemp          = 0;
    uint nLimits         = 0;
    uint jobArrId        = 0;
    uint jobArrElemId    = 0;
    size_t tmpJobId = 0;

    if (xdrs->x_op == XDR_DECODE) {
        jobSpecs->numToHosts    = 0;
        jobSpecs->toHosts       = NULL;
        jobSpecs->nxf           = 0;
        jobSpecs->xf            = NULL;
        jobSpecs->numEnv        = 0;
        jobSpecs->env           = NULL;
        jobSpecs->eexec.len     = 0;
        jobSpecs->eexec.data    = NULL;
        jobSpecs->loginShell    = NULL;
        jobSpecs->schedHostType = NULL;
        jobSpecs->execHosts     = NULL;
    }

    if (xdrs->x_op == XDR_FREE)
        {

        for (uint i = 0; i < jobSpecs->numToHosts; i++)
            {
            FREEUP (jobSpecs->toHosts[i]);
            }
        FREEUP (jobSpecs->toHosts);

        for ( uint i = 0; i < jobSpecs->numEnv; i++) {
            FREEUP (jobSpecs->env[i]);
        }
        FREEUP (jobSpecs->env);

        FREEUP (jobSpecs->xf);
        FREEUP (jobSpecs->loginShell);
        FREEUP (jobSpecs->schedHostType);
        FREEUP (jobSpecs->execHosts);
        if (!xdr_thresholds (xdrs, jobSpecs) || !xdr_lenData (xdrs, &jobSpecs->eexec)) {
            return FALSE;
        }
        return TRUE;
        }

    if (xdrs->x_op == XDR_ENCODE)
        {
        jobId64To32 (jobSpecs->jobId, &jobArrId, &jobArrElemId);
        }
    if (!xdr_u_int (xdrs, &jobArrId))
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_int", "jobId");
        return FALSE;
        }

    if (!(xdr_int    (xdrs, &jobSpecs->userId)        &&
          xdr_int    (xdrs, &jobSpecs->options)       &&
          xdr_short  (xdrs, &jobSpecs->nice)          &&
          xdr_int    (xdrs, &jobSpecs->priority)      &&
          xdr_int    (xdrs, &jobSpecs->chkSig)        &&
          xdr_int    (xdrs, &jobSpecs->actPid)        &&
          xdr_time_t (xdrs, &jobSpecs->chkPeriod)     &&
          xdr_time_t (xdrs, &jobSpecs->migThresh)     &&
          xdr_time_t (xdrs, &jobSpecs->lastSSuspTime) &&
          xdr_float  (xdrs, &jobSpecs->lastCpuTime)))
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_int", "userId");
        return FALSE;
        }

    nLimits = LSF_RLIM_NLIMITS;
    tmpJobId = jobArrId;
    if (!xdr_u_int (xdrs, &nLimits))
        {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_int", "nLimits");
        return FALSE;
        }


    for ( uint i = 0; i < nLimits && i < LSF_RLIM_NLIMITS; i++)
        {
        if (!xdr_lsfLimit (xdrs, &jobSpecs->lsfLimits[i], hdr))
            {
            ls_syslog (LOG_ERR, I18N_JOB_FAIL_S, __func__, lsb_jobid2str (tmpJobId), "xdr_lsfLimit");
            return FALSE;
            }
        }

    if (nLimits > LSF_RLIM_NLIMITS)
        {

        for ( uint i = LSF_RLIM_NLIMITS; i < nLimits; i++)
            {
            struct lsfLimit lsfLimit;
            if (!xdr_lsfLimit (xdrs, &lsfLimit, hdr))
                {
                ls_syslog (LOG_ERR, I18N_JOB_FAIL_S, __func__, lsb_jobid2str (tmpJobId), "xdr_lsfLimit");
                return FALSE;
                }
            }
        }

    if (!(xdr_int    (xdrs, &jobSpecs->jStatus)    &&
          xdr_u_int  (xdrs, &jobSpecs->reasons)    &&
          xdr_u_int  (xdrs, &jobSpecs->subreasons) &&
          xdr_time_t (xdrs, &jobSpecs->termTime)   &&
          xdr_time_t (xdrs, &jobSpecs->startTime)  &&
          xdr_time_t (xdrs, &jobSpecs->runTime)    &&
          xdr_time_t (xdrs, &jobSpecs->submitTime) &&
          xdr_int    (xdrs, &jobSpecs->jobPid)     &&
          xdr_int    (xdrs, &jobSpecs->jobPGid)    &&
          xdr_int    (xdrs, &jobSpecs->restartPid) &&
          xdr_int    (xdrs, &jobSpecs->sigValue)   &&
          xdr_u_int  (xdrs, &jobSpecs->umask)      &&
          xdr_int    (xdrs, &jobSpecs->jAttrib)))
        {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_int", "jStatus");
        return FALSE;
        }

    sp[0] = jobSpecs->jobFile;
    sp[1] = jobSpecs->inFile;
    sp[2] = jobSpecs->outFile;
    sp[3] = jobSpecs->errFile;
    sp[4] = jobSpecs->chkpntDir;
    sp[5] = jobSpecs->cwd;
    sp[6] = jobSpecs->subHomeDir;
    sp[7] = jobSpecs->command;
    sp[8] = jobSpecs->jobName;
    sp[9] = jobSpecs->preExecCmd;
    sp[10] = jobSpecs->fromHost;
    sp[11] = jobSpecs->resReq;

    if (xdrs->x_op == XDR_DECODE) {
        for ( uint i = 0; i < 11; i++) {
            sp[i][0] = '\0';
        }
    }
    if (!(xdr_string (xdrs, &sp[0], MAX_FILENAME_LEN) &&
          xdr_string (xdrs, &sp[1], MAX_FILENAME_LEN) &&
          xdr_string (xdrs, &sp[2], MAX_FILENAME_LEN) &&
          xdr_string (xdrs, &sp[3], MAX_FILENAME_LEN) &&
          xdr_string (xdrs, &sp[4], MAX_FILENAME_LEN) &&
          xdr_string (xdrs, &sp[5], MAX_FILENAME_LEN) &&
          xdr_string (xdrs, &sp[6], MAX_FILENAME_LEN) &&
          xdr_string (xdrs, &sp[7], MAX_LINE_LEN) &&
          xdr_string (xdrs, &sp[8], MAX_LINE_LEN) &&
          xdr_string (xdrs, &sp[9], MAX_LINE_LEN) &&
          xdr_string (xdrs, &sp[10], MAXHOSTNAMELEN)))
        {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_int", "jobFile");
        return FALSE;
        }
    if (xdrs->x_op == XDR_DECODE) {
        sp[11][0] = '\0';
    }
    if (!xdr_string (xdrs, &sp[11], MAX_LINE_LEN)) {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_int", "jobFile");
        return FALSE;
    }

    sp[12] = jobSpecs->queue;
    sp[13] = jobSpecs->windows;
    sp[14] = jobSpecs->userName;

    if (xdrs->x_op == XDR_DECODE) {
        for ( uint i = 12; i < 15; i++) {
            sp[i][0] = '\0';
        }
    }

    if (!(xdr_string (xdrs, &sp[12], MAX_FILENAME_LEN) &&  xdr_string (xdrs, &sp[13], MAX_LINE_LEN) && xdr_string (xdrs, &sp[14], MAX_LSB_NAME_LEN)))
        {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_int", "jobFile");
        return FALSE;
        }



    if (!xdr_u_int (xdrs, &jobSpecs->numToHosts))
        {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_int", "numToHosts");
        return FALSE;
        }

    if (xdrs->x_op == XDR_DECODE && jobSpecs->numToHosts)
        {
        jobSpecs->toHosts = (char **) my_calloc (jobSpecs->numToHosts, sizeof (char *), __func__);
        }

    for ( uint i = 0; i < jobSpecs->numToHosts; i++)
        {
        if (!xdr_var_string (xdrs, &jobSpecs->toHosts[i]))
            {
            ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_var_string", "toHosts");
            return FALSE;
            }
        }

    if (!xdr_thresholds (xdrs, jobSpecs))
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S, __func__, lsb_jobid2str (tmpJobId), "xdr_thresholds");

    if (!xdr_u_int (xdrs, &jobSpecs->nxf))
        {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_int", "nxf");
        return FALSE;
        }

    if (xdrs->x_op == XDR_DECODE && jobSpecs->nxf > 0)
        {
        jobSpecs->xf = (struct xFile *) my_calloc (jobSpecs->nxf, sizeof (struct xFile), __func__);
        }

    for ( uint i = 0; i < jobSpecs->nxf; i++)
        {
        if (!xdr_arrayElement (xdrs, (char *) &(jobSpecs->xf[i]), hdr, xdr_xFile))
            {
            ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (tmpJobId), "xdr_arrayElement", "xf");
            return FALSE;
            }
        }

    sp[0] = jobSpecs->mailUser;
    sp[1] = jobSpecs->clusterName;
    sp[2] = jobSpecs->projectName;
    sp[3] = jobSpecs->preCmd;
    sp[4] = jobSpecs->postCmd;
    sp[5] = jobSpecs->execCwd;
    sp[6] = jobSpecs->execHome;
    sp[7] = jobSpecs->requeueEValues;

    if (xdrs->x_op == XDR_DECODE) {
        for ( uint i = 0; i < 8; i++) {
            sp[i][0] = '\0';
        }
    }

    if (!(xdr_string (xdrs, &sp[0], MAX_LINE_LEN)          &&
          xdr_string (xdrs, &sp[1], MAX_LSB_NAME_LEN)    &&
          xdr_string (xdrs, &sp[2], MAX_LSB_NAME_LEN)    &&
          xdr_string (xdrs, &sp[3], MAX_LINE_LEN)          &&
          xdr_string (xdrs, &sp[4], MAX_LINE_LEN)          &&
          xdr_string (xdrs, &sp[5], MAX_FILENAME_LEN)      &&
          xdr_string (xdrs, &sp[6], MAX_FILENAME_LEN)      &&
          xdr_string (xdrs, &sp[7], MAX_LINE_LEN)          &&
          xdr_int    (xdrs, &jobSpecs->execUid)          &&
          xdr_u_int  (xdrs, &jobSpecs->maxNumProcessors) &&
          xdr_u_int  (xdrs, &jobSpecs->numEnv))
        )
        {
            return FALSE;
        }

    if (xdrs->x_op == XDR_DECODE && jobSpecs->numEnv) {
        jobSpecs->env = (char **) my_calloc (jobSpecs->numEnv, sizeof (char *), __func__);
        }

    for ( uint i = 0; i < jobSpecs->numEnv; i++) {
        if (!xdr_var_string (xdrs, &jobSpecs->env[i])) {
            return FALSE;
        }
    }


    if (!xdr_lenData (xdrs, &jobSpecs->eexec)) {
        return FALSE;
    }

    if (!xdr_u_short (xdrs, &jobSpecs->niosPort)) {
        return FALSE;
    }
    sp[0] = jobSpecs->resumeCond;
    sp[1] = jobSpecs->stopCond;
    sp[2] = jobSpecs->suspendActCmd;
    sp[3] = jobSpecs->resumeActCmd;
    sp[4] = jobSpecs->terminateActCmd;

    if (xdrs->x_op == XDR_DECODE)
        {
        sp[0][0] = '\0';
        sp[1][0] = '\0';
        }
    if (!(xdr_string (xdrs, &sp[0], MAX_LINE_LEN)) ||
        !(xdr_string (xdrs, &sp[1], MAX_LINE_LEN)))
        return FALSE;


    if (xdrs->x_op == XDR_DECODE)
        {
        for ( uint i = 2; i < 5; i++) {
            sp[i][0] = '\0';
        }
    }

    for ( uint i = 2; i < 5; i++) {
        if (!(xdr_string (xdrs, &sp[i], MAX_LINE_LEN)))
            return FALSE;
    }


    for ( uint i = 0; i < LSB_SIG_NUM; i++) {
        if (!(xdr_int (xdrs, &jobSpecs->sigMap[i]))) {
            return FALSE;
        }
    }

    if (!(xdr_int (xdrs, &jobSpecs->actValue))) {
        return FALSE;
    }

    if (!xdr_var_string (xdrs, &jobSpecs->loginShell)) {
        return FALSE;
    }

    if (!xdr_var_string (xdrs, &jobSpecs->schedHostType)) {
        return FALSE;
    }

    if (!xdr_var_string (xdrs, &jobSpecs->execHosts)) {
        return FALSE;
    }


    if (!xdr_int (xdrs, &jobSpecs->options2))
        {
        return FALSE;
        }


    pTemp = jobSpecs->jobSpoolDir;
    if (!(xdr_string (xdrs, &pTemp, MAX_PATH_LEN)))
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_string", "jobSpoolDir");
        return FALSE;
        }

    if (!(xdr_u_int (xdrs, &jobArrElemId)))
        {
        return FALSE;
        }

    if (xdrs->x_op == XDR_DECODE)
        {
        jobId32To64 (&jobSpecs->jobId, jobArrId, jobArrElemId);
        }

    sp[0] = jobSpecs->inFileSpool;
    sp[1] = jobSpecs->commandSpool;

    if (!(xdr_string (xdrs, &sp[0], MAX_FILENAME_LEN)))
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_string", "inFileSpool");
        return FALSE;
        }
    if (!(xdr_string (xdrs, &sp[1], MAX_FILENAME_LEN)))
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_string", "commandSpool");
        return FALSE;
        }

    if (!(xdr_int (xdrs, &jobSpecs->userPriority)))
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_int", "userPriority");
        return FALSE;
        }

    sp[0] = jobSpecs->execUsername;
    if( !( xdr_string ( xdrs, &sp[0], MAX_LSB_NAME_LEN ) ) ) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_string", "execUsername");
        return FALSE;
        }

    sp[0] = jobSpecs->prepostUsername;
    if( !( xdr_string ( xdrs, &sp[0], MAX_LSB_NAME_LEN ) ) ) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_string", "prepostUsername");
        return FALSE;
    }

    return TRUE;
}

bool_t
xdr_jobSig (XDR *xdrs, struct jobSig * jobSig, struct LSFHeader *hdr)
{
    static char *actCmd = NULL;
    uint jobArrId        = 0;
    uint jobArrElemId    = 0;
    uint newJobArrId     = 0;
    uint newJobArrElemId = 0;

    assert( hdr->length );

    jobArrId = jobArrElemId = newJobArrId = newJobArrElemId = 0;

    if (xdrs->x_op == XDR_DECODE) {
        FREEUP (actCmd);
    }
    if (xdrs->x_op == XDR_ENCODE) {
        jobId64To32 (jobSig->jobId, &jobArrId, &jobArrElemId);
    }

    if ( ! (xdr_u_int  (xdrs, &jobArrId)            &&
            xdr_int    (xdrs, &(jobSig->sigValue))  &&
            xdr_time_t (xdrs, &(jobSig->chkPeriod)) &&
            xdr_int    (xdrs, &(jobSig->actFlags))  &&
            xdr_int    (xdrs, &(jobSig->reasons))   &&
            xdr_int    (xdrs, &(jobSig->subReasons)))
        )
    {
        return FALSE;
    }
    if (!xdr_var_string (xdrs, &jobSig->actCmd)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        actCmd = jobSig->actCmd;
    }

    if (!xdr_u_int (xdrs, &jobArrElemId)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&jobSig->jobId, jobArrId, jobArrElemId);
    }

    if (xdrs->x_op == XDR_ENCODE) {
        jobId64To32 (jobSig->newJobId, &newJobArrId, &newJobArrElemId);
    }

    if (!xdr_u_int (xdrs, &newJobArrId)) {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &newJobArrElemId)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&jobSig->newJobId, newJobArrId, newJobArrElemId);
    }

    return TRUE;
}

bool_t
xdr_jobReply (XDR *xdrs, struct jobReply * jobReply, struct LSFHeader *hdr)
{
    uint jobArrId = 0;
    uint jobArrElemId = 0;

    assert( hdr->length );

    if (xdrs->x_op == XDR_ENCODE)
        {
        jobId64To32 (jobReply->jobId, &jobArrId, &jobArrElemId);
        }
    if (!xdr_u_int (xdrs, &jobArrId))
        return FALSE;

    if (!xdr_int (xdrs, &jobReply->jobPid)) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &jobReply->jobPGid)) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &jobReply->actPid)) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &jobReply->jStatus)) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &jobReply->reasons)) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &jobReply->actValue)) {
        return FALSE;
    }

    if (!xdr_int (xdrs, &jobReply->actStatus)) {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &jobArrElemId)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&jobReply->jobId, jobArrId, jobArrElemId);
    }

    return TRUE;
}

bool_t
xdr_statusReq (XDR *xdrs, struct statusReq *statusReq, struct LSFHeader *hdr)
{
    uint jobArrId     = 0;
    uint jobArrElemId = 0;

    if (xdrs->x_op == XDR_FREE)
        {
        for ( uint i = 0; i < statusReq->numExecHosts; i++) {
            FREEUP (statusReq->execHosts[i]);
        }
        if (statusReq->numExecHosts > 0) {
            FREEUP (statusReq->execHosts);
        }
        statusReq->numExecHosts = 0;
        FREEUP (statusReq->execHome);
        FREEUP (statusReq->execCwd);
        FREEUP (statusReq->queuePreCmd);
        FREEUP (statusReq->queuePostCmd);
        FREEUP (statusReq->execUsername);
        if (statusReq->runRusage.npids > 0) {
            FREEUP (statusReq->runRusage.pidInfo);
        }
        if (statusReq->runRusage.npgids > 0) {
            FREEUP (statusReq->runRusage.pgid);
        }

        return TRUE;
    }

    if (xdrs->x_op == XDR_ENCODE)  {
        jobId64To32 (statusReq->jobId, &jobArrId, &jobArrElemId);
    }
    if (!(xdr_u_int (xdrs, &jobArrId) &&
          xdr_int (xdrs, &statusReq->jobPid) &&
          xdr_int (xdrs, &statusReq->jobPGid) &&
          xdr_int (xdrs, &statusReq->actPid) &&
          xdr_int (xdrs, &statusReq->seq) &&
          xdr_int (xdrs, &statusReq->newStatus) &&
          xdr_int (xdrs, &statusReq->reason) &&
          xdr_int (xdrs, &statusReq->subreasons) &&
          xdr_int (xdrs, (int *) &statusReq->sbdReply))
        ) 
    {
        return FALSE;
    }

    if (!xdr_arrayElement (xdrs, (char *) &statusReq->lsfRusage, hdr, xdr_lsfRusage)) {
        return FALSE;
    }

    if (!(xdr_u_int (xdrs, &statusReq->execUid) && xdr_u_int (xdrs, &statusReq->numExecHosts))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        if (statusReq->numExecHosts > 0) {
            statusReq->execHosts = (char **) calloc (statusReq->numExecHosts, sizeof (char *));
            if (!statusReq->execHosts) {
                return FALSE;
            }
        }
    }

    if (!xdr_array_string (xdrs, statusReq->execHosts, MAXHOSTNAMELEN, statusReq->numExecHosts)) {
        return FALSE;
    }

    if (!(xdr_int (xdrs, &statusReq->exitStatus)          &&
          xdr_var_string (xdrs, &statusReq->execHome)     &&
          xdr_var_string (xdrs, &statusReq->execUsername) &&
          xdr_var_string (xdrs, &statusReq->execCwd)      &&
          xdr_var_string (xdrs, &statusReq->queuePreCmd)  &&
          xdr_var_string (xdrs, &statusReq->queuePostCmd))
        )
    {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &statusReq->msgId)) {
        return FALSE;
    }

    if (!(xdr_jRusage (xdrs, &(statusReq->runRusage), hdr))) {
        return FALSE;
    }

    if (!(xdr_int (xdrs, &(statusReq->sigValue)) && xdr_int (xdrs, &(statusReq->actStatus)))) {
        return FALSE;
    }

    if (!xdr_u_int (xdrs, &jobArrElemId)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        jobId32To64 (&statusReq->jobId, jobArrId, jobArrElemId);
    }

    return TRUE;
}

bool_t
xdr_chunkStatusReq (XDR *xdrs, struct chunkStatusReq *chunkStatusReq, struct LSFHeader *hdr)
{
    static char __func__] = "xdr_chunkStatusReq";

    if (xdrs->x_op == XDR_DECODE)
        {
        chunkStatusReq->numStatusReqs = 0;
        chunkStatusReq->statusReqs = NULL;
        }

    if (xdrs->x_op == XDR_FREE)
        {
        for (uint i = 0; i < chunkStatusReq->numStatusReqs; i++)
            {
            xdr_lsffree (xdr_statusReq, (char *) chunkStatusReq->statusReqs[i], hdr);
            FREEUP (chunkStatusReq->statusReqs[i]);
            }
        FREEUP (chunkStatusReq->statusReqs);
        return TRUE;
        }
    if (!xdr_u_int (xdrs, &chunkStatusReq->numStatusReqs))
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_int", "numStatusReqs");
        return FALSE;
        }
    if (xdrs->x_op == XDR_DECODE && chunkStatusReq->numStatusReqs)
        {
        chunkStatusReq->statusReqs = (struct statusReq **) my_calloc (chunkStatusReq->numStatusReqs, sizeof (struct statusReq *), __func__);
        for ( uint i = 0; i < chunkStatusReq->numStatusReqs; i++)
            {
            chunkStatusReq->statusReqs[i] = (struct statusReq *) my_calloc (1, sizeof (struct statusReq), __func__);
            }
        }
    for ( uint i = 0; i < chunkStatusReq->numStatusReqs; i++)
        {
        xdr_statusReq (xdrs, chunkStatusReq->statusReqs[i], hdr);
        }
    return TRUE;
}

bool_t
xdr_sbdPackage (XDR *xdrs, struct sbdPackage * sbdPackage, struct LSFHeader *hdr)
{
    static char __func__] = "xdr_sbdPackage";
    char *sp;

    sp = sbdPackage->lsbManager;
    if (xdrs->x_op == XDR_DECODE) {
        sp[0] = '\0';
    }

    if (!(xdr_int   (xdrs, &sbdPackage->mbdPid)       &&
          xdr_int   (xdrs, &sbdPackage->retryIntvl)   &&
          xdr_int   (xdrs, &sbdPackage->preemPeriod)  &&
          xdr_int   (xdrs, &sbdPackage->pgSuspIdleT)  &&
          xdr_u_int (xdrs, &sbdPackage->maxJobs)      &&
          xdr_long  (xdrs, &sbdPackage->sbdSleepTime) &&
          xdr_int   (xdrs, &sbdPackage->managerId)    &&
          xdr_u_int (xdrs, &sbdPackage->numJobs)      &&
          xdr_string (xdrs, &sp, MAX_LSB_NAME_LEN))
        )
    {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_int", "lsbManager");
        return FALSE;
    }

    if (xdrs->x_op == XDR_ENCODE) {
        for (uint i = 0; i < sbdPackage->numJobs; i++) {
            if (!xdr_arrayElement (xdrs, (char *) &(sbdPackage->jobs[i]), hdr, xdr_jobSpecs)) {
                return FALSE;
            }
        }
    }

    if (!xdr_sbdPackage1 (xdrs, sbdPackage, hdr)) {
        return FALSE;
    }

    return TRUE;
}

bool_t
xdr_sbdPackage1 (XDR *xdrs, struct sbdPackage *sbdPackage, struct LSFHeader *hdr)
{
    static char __func__] = "xdr_sbdPackage1";

    assert( hdr->length );

    if (!(xdr_u_int (xdrs, &sbdPackage->uJobLimit))) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_int", "uJobLimit");
        return FALSE;
    }
    
    if (!(xdr_u_int (xdrs, &sbdPackage->rusageUpdateRate) && xdr_u_int (xdrs, &sbdPackage->rusageUpdatePercent))) {
        return FALSE;
    }
    
    
    if (!xdr_u_int (xdrs, &sbdPackage->jobTerminateInterval)) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_int", "jobTerminateInterval");
        return FALSE;
    }

    if (xdrs->x_op == XDR_ENCODE || (xdrs->x_op != XDR_FREE)) {
        
        if (!xdr_u_int (xdrs, &sbdPackage->nAdmins)) {
            return FALSE;
        }
        
        if (xdrs->x_op == XDR_DECODE && sbdPackage->nAdmins > 0) {
            sbdPackage->admins = (char **) calloc (sbdPackage->nAdmins, sizeof (char *));
            if (sbdPackage->admins == NULL) {
                sbdPackage->nAdmins = 0;
                return FALSE;
            }
        }
        for ( uint i = 0; i < sbdPackage->nAdmins; i++) {
            if (!xdr_var_string (xdrs, &sbdPackage->admins[i]))
                return FALSE;
            }
        }
    
    if (xdrs->x_op == XDR_FREE) {
        for ( uint i = 0; i < sbdPackage->nAdmins; i++) {
            FREEUP (sbdPackage->admins[i]);
        }
        FREEUP (sbdPackage->admins);
    }
    
    return TRUE;
}

static int
xdr_thresholds (XDR *xdrs, struct jobSpecs *jobSpecs)
{
    static char __func__] = "xdr_thresholds";
    
    if (xdrs->x_op == XDR_DECODE) {
        jobSpecs->thresholds.loadSched = NULL;
        jobSpecs->thresholds.loadStop = NULL;
    }
    
    if (xdrs->x_op == XDR_FREE)  {

        for ( uint i = 0; i < jobSpecs->thresholds.nThresholds; i++) {
            FREEUP (jobSpecs->thresholds.loadSched[i]);
            FREEUP (jobSpecs->thresholds.loadStop[i]);
        }

        FREEUP (jobSpecs->thresholds.loadSched);
        FREEUP (jobSpecs->thresholds.loadStop);
        return TRUE;
    }
    
    if (!(xdr_u_int (xdrs, &jobSpecs->thresholds.nIdx) && xdr_u_int (xdrs, &jobSpecs->thresholds.nThresholds))) {
        ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (jobSpecs->jobId), "xdr_int", "nIdx/nThresholds");
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        jobSpecs->thresholds.loadSched = (float **) my_calloc (jobSpecs->thresholds.nThresholds, sizeof (float *), __func__); // FIXME FIXME FIXME FIXME FIXME to the debugger, you go!
        jobSpecs->thresholds.loadStop = (float **)  my_calloc (jobSpecs->thresholds.nThresholds, sizeof (float *), __func__); // FIXME FIXME FIXME FIXME FIXME to the debugger, you go!
        for ( uint i = 0; i < jobSpecs->thresholds.nThresholds; i++) {
            jobSpecs->thresholds.loadSched[i] = (float *) my_calloc (jobSpecs->thresholds.nIdx, sizeof (float), __func__);
            jobSpecs->thresholds.loadStop[i] = (float *)  my_calloc (jobSpecs->thresholds.nIdx, sizeof (float), __func__);
        }
    }

    for ( uint j = 0; j < jobSpecs->thresholds.nThresholds; j++) {
        for ( uint i = 0; i < jobSpecs->thresholds.nIdx; i++) {
            if (!(xdr_float (xdrs, &jobSpecs->thresholds.loadStop[j][i]) && xdr_float (xdrs, &jobSpecs->thresholds.loadSched[j][i]))) {
                ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S, __func__, lsb_jobid2str (jobSpecs->jobId), "xdr_float", "loadStop/loadSched");
                return FALSE;
            }
        }
    }
    return TRUE;
    
}
