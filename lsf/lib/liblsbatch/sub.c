/* $Id: lsb.sub.c 397 2007-11-26 19:04:00Z mblack $
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


#include <ctype.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef _XOPEN_SOURCE
#include <time.h>
#else
#include <sys/time.h>
#endif

// #define exit(a)         _exit(a)

#include "lib/lib.h"
#include "lib/mls.h"
#include "lib/table.h"
#include "lib/xdr.h"
#include "libint/intlibout.h"
#include "lsb/log.h"
#include "lsb/lsb.h"
#include "lsb/misc.h"
#include "lsb/sub.h"

// #define SKIPSPACE(sp)      while (isspace(*(sp))) (sp)++;

// #define EMBED_INTERACT     0x01
// #define EMBED_OPTION_ONLY  0x02
// #define EMBED_BSUB         0x04
// #define EMBED_RESTART      0x10
// #define EMBED_QSUB         0x20
// #define    NL_SETN           13

// #define ESUBNAME "esub"

/////////////////////////////////////////////////////
// global variables
static int lsbMode_ = LSB_MODE_BATCH;



void PRINT_ERRMSG0( char **errMsg, char *fmt)
{
	if (errMsg == NULL) {
		// fprintf(stderr, fmt);
		fprintf( stderr, "%s %s\n", *errMsg, fmt );
	}
	else {
		// sprintf(*errMsg, fmt);
		sprintf( *errMsg, "%s %s\n", *errMsg, fmt );
	}
}

void PRINT_ERRMSG1( char  **errMsg, char *fmt, char *msg1 )
{
	if (errMsg == NULL) {
		// fprintf(stderr, fmt, msg1);
		fprintf( stderr, "%s %s %s\n", *errMsg, fmt, msg1);
	}
	else {
		// sprintf(*errMsg, fmt, msg1);
		sprintf( *errMsg, "%s %s %s\n", *errMsg, fmt, msg1);
	}
}

void PRINT_ERRMSG2( char **errMsg, char *fmt, char *msg1, char *msg2 )
{
	if (errMsg == NULL) {
		// fprintf(stderr, fmt, msg1, msg2);
		fprintf( stderr, "%s %s %s %s\n", *errMsg, fmt, msg1, msg2);
	}
	else {
		// sprintf(*errMsg, fmt, msg1, msg2);
		sprintf( *errMsg, "%s %s %s %s\n", *errMsg, fmt, msg1, msg2);
	}
}

void PRINT_ERRMSG3( char **errMsg, char *fmt, char *msg1, char *msg2, char *msg3 )
{
	if (errMsg == NULL) {
		// fprintf(stderr, fmt, msg1, msg2, msg3);
		fprintf( stderr, "%s %s %s %s %s\n", *errMsg, fmt, msg1, msg2, msg3);
	}
	else {
		// sprintf(*errMsg, fmt, msg1, msg2, msg3);
		sprintf( *errMsg, "%s %s %s %s %s\n", *errMsg, fmt, msg1, msg2, msg3);
	}
}


LS_LONG_INT
lsb_submit (struct submit *jobSubReq, struct submitReply *submitRep)
{
	LS_LONG_INT jobId = -1;
	struct lsfAuth auth;
	char cwd[MAX_FILENAME_LEN]; // FIXME FIXME figure out if MAX_FILENAME_LEN is a constant offered by the filesystem/OS or by LSF; set to filesystem/OS appropriately
	struct group *grpEntry;
	char *queue = NULL;
	struct submitReq submitReq = {
		"  ", // padding1 
		0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, // 11 ints of varying signage
		"    ", // padding3 
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 23 NULL
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		0, 0, 0, 0, // 4 time stamps
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 11 zeros, defined from LSF_RLIM_NLIMITS from lsf.h
		"    ", // padding2
		NULL
	};

	if (logclass & (LC_TRACE | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	lsberrno = LSBE_BAD_ARG;


	subNewLine_ (jobSubReq->resReq);
	subNewLine_ (jobSubReq->dependCond);
	subNewLine_ (jobSubReq->preExecCmd);
	subNewLine_ (jobSubReq->mailUser);
	subNewLine_ (jobSubReq->jobName);
	subNewLine_ (jobSubReq->queue);
	subNewLine_ (jobSubReq->inFile);
	subNewLine_ (jobSubReq->outFile);
	subNewLine_ (jobSubReq->errFile);
	subNewLine_ (jobSubReq->chkpntDir);
	subNewLine_ (jobSubReq->projectName);
	for ( unsigned int i = 0; i < jobSubReq->numAskedHosts; i++)
		{
		subNewLine_ (jobSubReq->askedHosts[i]);
		}

	if (getCommonParams (jobSubReq, &submitReq, submitRep) < 0) {
		return -1;
	}

	if (!(jobSubReq->options & SUB_QUEUE))
		{

		if ((queue = getenv ("LSB_DEFAULTQUEUE")) != NULL && queue[0] != '\0')
			{
			submitReq.queue = queue;
			submitReq.options |= SUB_QUEUE;
			}
		}

	submitReq.cwd = cwd;


	if ((grpEntry = getgrgid (getgid ())) == NULL)
		{
		if (logclass & (LC_TRACE | LC_EXEC))
			ls_syslog (LOG_DEBUG, "%s: group id %d, does not have an name in the unix group file", __func__, (int) getgid ());
		}
	else
		{


		if (putEnv ("LSB_UNIXGROUP", grpEntry->gr_name) < 0)
			{
			if (logclass & (LC_TRACE | LC_EXEC))
				ls_syslog (LOG_DEBUG, "%s: group <%s>, cannot be set in the environment.", __func__, grpEntry->gr_name);
			}
		}

	makeCleanToRunEsub ();


	if (getUserInfo (&submitReq, jobSubReq) < 0) {
		return -1;
	}


	if (!(jobSubReq->options & SUB_QUEUE))
		{
		if (queue != NULL && queue[0] != '\0')
			{
			jobSubReq->queue = queue;
			jobSubReq->options |= SUB_QUEUE;
			}
		}

	modifyJobInformation (jobSubReq);
	if (getCommonParams (jobSubReq, &submitReq, submitRep) < 0)
		return -1;

#ifdef INTER_DAEMON_AUTH
	putEnv ("LSF_EAUTH_AUX_PASS", "yes");
#endif


	if ((lsbParams[LSB_INTERACTIVE_STDERR].paramValue != NULL) && (strcasecmp (lsbParams[LSB_INTERACTIVE_STDERR].paramValue,
						"y") == 0))
		{
		if (putEnv ("LSF_INTERACTIVE_STDERR", "y") < 0)
			{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_S, __func__, "putenv");
			}
		}

	if (authTicketTokens_ (&auth, NULL) == -1)
		{
		return -1;
		}

	if (submitReq.options & SUB_RESTART) {
		jobId = subRestart (jobSubReq, &submitReq, submitRep, &auth);
	}
	else {
		jobId = subJob (jobSubReq, &submitReq, submitRep, &auth);
	}

	return jobId;

}


int
getCommonParams (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep)
{
	int useKb = 0;

	if (logclass & (LC_TRACE | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	if (jobSubReq == NULL || submitRep == NULL)
		return -1;

	submitReq->options = jobSubReq->options;
	submitReq->options2 = jobSubReq->options2;


	if (jobSubReq->options & SUB_DEPEND_COND)
		{
		if (dependCondSyntax (jobSubReq->dependCond) < 0) {
			return -1;
		}
		else
			{
			submitReq->dependCond = jobSubReq->dependCond;
			}
		} 
	else {
		submitReq->dependCond = "";
	}

	if (jobSubReq->options & SUB_PRE_EXEC)
		{
		if (!jobSubReq->preExecCmd) {
			return -1;
		}
		if (strlen (jobSubReq->preExecCmd) >= MAX_LINE_LEN - 1) {
			return -1;
		}
		else {
			submitReq->preExecCmd = jobSubReq->preExecCmd;
		}
		}
	else {
		submitReq->preExecCmd = "";
	}

	if (jobSubReq->options & SUB_QUEUE)
		{
		if (!jobSubReq->queue)
			{
			lsberrno = LSBE_BAD_QUEUE;
			return -1;
			}
		submitReq->queue = jobSubReq->queue;
		}
	else
		{
		submitReq->queue = "";
		}

	if (jobSubReq->options & SUB_HOST)
		{
		submitReq->numAskedHosts = jobSubReq->numAskedHosts;
		submitReq->askedHosts = jobSubReq->askedHosts;
		}
	else {
		submitReq->numAskedHosts = 0;
	}

// FIXME  file a ticket to check for negative host numbers from command-line tools or conf
/*    if (submitReq->numAskedHosts < 0) {
		return -1;
	}*/

	for ( unsigned int i = 0; i < submitReq->numAskedHosts; i++)
		{
		if ((submitReq->askedHosts[i])  && (strlen (submitReq->askedHosts[i]) + 1) < MAXHOSTNAMELEN) {
			continue;
		}
		lsberrno = LSBE_BAD_HOST;
		assert( i <= INT_MAX );
		submitRep->badReqIndx = (int) i;
		return -1;
		}

	if (jobSubReq->options & SUB_HOST_SPEC)
		{
		if (!jobSubReq->hostSpec) {
			return -1;
		}
		if (strlen (jobSubReq->hostSpec) >= MAXHOSTNAMELEN - 1) {
			return -1;
		}
		else {
			submitReq->hostSpec = jobSubReq->hostSpec;
		}
		}
	else {
		submitReq->hostSpec = "";
	}

	submitReq->beginTime = jobSubReq->beginTime;
	submitReq->termTime = jobSubReq->termTime;


	if (!limitIsOk_ (jobSubReq->rLimits))
		{
		useKb = 1;
		submitReq->options |= SUB_RLIMIT_UNIT_IS_KB;
		}

	for ( unsigned int i = 0; i < LSF_RLIM_NLIMITS; i++) {
		submitReq->rLimits[i] = -1;
	}

	if (jobSubReq->rLimits[LSF_RLIMIT_CPU] >= 0) {
		submitReq->rLimits[LSF_RLIMIT_CPU] = jobSubReq->rLimits[LSF_RLIMIT_CPU];
	}
	if (jobSubReq->rLimits[LSF_RLIMIT_RUN] >= 0) {
		submitReq->rLimits[LSF_RLIMIT_RUN] = jobSubReq->rLimits[LSF_RLIMIT_RUN];
	}

	if (jobSubReq->rLimits[LSF_RLIMIT_FSIZE] > 0)
		{
		if (useKb)
			{
			submitReq->rLimits[LSF_RLIMIT_FSIZE] = jobSubReq->rLimits[LSF_RLIMIT_FSIZE];
			}
		else
			{
			submitReq->rLimits[LSF_RLIMIT_FSIZE] = jobSubReq->rLimits[LSF_RLIMIT_FSIZE] * 1024;
			}
		}
	if (jobSubReq->rLimits[LSF_RLIMIT_DATA] > 0)
		{
		if (useKb)
			{
			submitReq->rLimits[LSF_RLIMIT_DATA] = jobSubReq->rLimits[LSF_RLIMIT_DATA];
			}
		else
			{
			submitReq->rLimits[LSF_RLIMIT_DATA] = jobSubReq->rLimits[LSF_RLIMIT_DATA] * 1024;
			}
		}
	if (jobSubReq->rLimits[LSF_RLIMIT_STACK] > 0)
		{
		if (useKb)
			{
			submitReq->rLimits[LSF_RLIMIT_STACK] = jobSubReq->rLimits[LSF_RLIMIT_STACK];
			}
		else
			{
			submitReq->rLimits[LSF_RLIMIT_STACK] = jobSubReq->rLimits[LSF_RLIMIT_STACK] * 1024;
			}
		}
	if (jobSubReq->rLimits[LSF_RLIMIT_CORE] >= 0)
		{
		if (useKb)
			{
			submitReq->rLimits[LSF_RLIMIT_CORE] = jobSubReq->rLimits[LSF_RLIMIT_CORE];
			}
		else
			{
			submitReq->rLimits[LSF_RLIMIT_CORE] = jobSubReq->rLimits[LSF_RLIMIT_CORE] * 1024;
			}
		}
	if (jobSubReq->rLimits[LSF_RLIMIT_RSS] > 0)
		{
		if (useKb)
			{
			submitReq->rLimits[LSF_RLIMIT_RSS] = jobSubReq->rLimits[LSF_RLIMIT_RSS];
			}
		else
			{
			submitReq->rLimits[LSF_RLIMIT_RSS] = jobSubReq->rLimits[LSF_RLIMIT_RSS] * 1024;
			}
		}
	if (jobSubReq->rLimits[LSF_RLIMIT_SWAP] > 0)
		{
		if (useKb)
			{
			submitReq->rLimits[LSF_RLIMIT_SWAP] = jobSubReq->rLimits[LSF_RLIMIT_SWAP];
			}
		else
			{
			submitReq->rLimits[LSF_RLIMIT_SWAP] = jobSubReq->rLimits[LSF_RLIMIT_SWAP] * 1024;
			}
		}

	if (jobSubReq->rLimits[LSF_RLIMIT_PROCESS] > 0) {
		submitReq->rLimits[LSF_RLIMIT_PROCESS] = jobSubReq->rLimits[LSF_RLIMIT_PROCESS];
	}

	if ((jobSubReq->beginTime > 0 && jobSubReq->termTime > 0) && (submitReq->beginTime > submitReq->termTime))
		{
		lsberrno = LSBE_START_TIME;
		return -1;
		}


	submitReq->submitTime = time (0);

	if (jobSubReq->options2 & SUB2_JOB_PRIORITY)
		{
		submitReq->userPriority = jobSubReq->userPriority;
		}
	else
		{
		submitReq->userPriority = -1;
		}

	if (logclass & (LC_TRACE | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: Okay", __func__);
	}

	return 0;
}

int
createJobInfoFile (struct submit *jobSubReq, struct lenData *jf)
{
	char **ep     = NULL;
	char *sp      = NULL;
	char *num     = malloc( sizeof( char ) * MAX_LSB_NAME_LEN + 1 );
	char *p       = NULL; 
	char *oldp    = NULL;
	unsigned int length   = 0;
	unsigned int len      = 0;
	unsigned int len1     = 0;
	unsigned int numEnv   = 0;
	unsigned int noEqual  = 0;;
	unsigned int size     = MSGSIZE;
	unsigned int tsoptlen = 0;

	if (logclass & (LC_TRACE | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	length += sizeof (CMDSTART);
	length += sizeof (TRAPSIGCMD);
	length += sizeof (WAITCLEANCMD);
	length += sizeof (EXITCMD);
	length += strlen (jobSubReq->command);
	length += tsoptlen;
	length += sizeof (LSBNUMENV);
	length += sizeof (ENVSSTART);
	length += sizeof (EDATASTART);
	length += sizeof (SHELLLINE) + 1;
	length += MAX_LSB_NAME_LEN * 2 + ed.len;

	jf->len = 0;
	size = MAX (length, MSGSIZE);
	if ((jf->data = (char *) malloc (size)) == NULL)
		{
		lsberrno = LSBE_NO_MEM;
		return -1;
		}
	jf->data[0] = '\0';
	strcat (jf->data, SHELLLINE);

	if (useracctmap)
		{
		strcat (jf->data, "LSB_ACCT_MAP='");
		strcat (jf->data, useracctmap);
		strcat (jf->data, "'; export LSB_ACCT_MAP\n");
		length += 14 + strlen (useracctmap) + 24;
		free (useracctmap);
		useracctmap = NULL;
		}

	strcat (jf->data, "OPENLAVA_VERSION='");
	sprintf (num, "%d", OPENLAVA_VERSION);
	strcat (jf->data, num);
	strcat (jf->data, "'; export OPENLAVA_VERSION\n");
	length += 13 + strlen (num) + 23;

	for (ep = environ; *ep; ep++)
		{
		noEqual = FALSE;
		if (logclass & (LC_TRACE | LC_EXEC))
			{
			ls_syslog (LOG_DEBUG, "%s: environment variable <%s>", __func__, *ep);
			}

		if (!strncmp (*ep, "LSB_JOBID=", 10) ||
			!strncmp (*ep, "LSB_HOSTS=", 10) ||
			!strncmp (*ep, "LSB_QUEUE=", 10) ||
			!strncmp (*ep, "LSB_JOBNAME=", 12) ||
			!strncmp (*ep, "LSB_TRAPSIGS=", 13) ||
			!strncmp (*ep, "LSB_JOBFILENAME=", 16) ||
			!strncmp (*ep, "LSB_RESTART=", 12) ||
			!strncmp (*ep, "LSB_EXIT_PRE_ABORT=", 19) ||
			!strncmp (*ep, "LSB_EXIT_REQUEUE=", 17) ||
			!strncmp (*ep, "LS_JOBPID=", 10) ||
			!strncmp (*ep, "LSB_INTERACTIVE=", 16) ||
			!strncmp (*ep, "LSB_ACCT_MAP=", 13) ||
			!strncmp (*ep, "LSB_JOB_STARTER=", 16) ||
			!strncmp (*ep, "LSB_EVENT_ATTRIB=", 17) ||
			!strncmp (*ep, "OPENLAVA_VERSION=", 12) ||
			!strncmp (*ep, "LSB_SUB_", 8) ||
			!strncmp (*ep, "HOME=", 5) ||
			!strncmp (*ep, "PWD=", 4) || !strncmp (*ep, "USER=", 5))
			{
			continue;
			}

		if (!(jobSubReq->options & SUB_INTERACTIVE))
			{

			if (!strncmp (*ep, "TERMCAP=", 8) || !strncmp (*ep, "TERM=", 5))
				continue;
			}

		sp = putstr_ (*ep);
		oldp = sp;
		if (!sp)
			{
			FREEUP (sp);
			lsberrno = LSBE_NO_MEM;
			return -1;
			}

		if (strncmp (sp, "DISPLAY=", 8) == 0)
			{

			sp = chDisplay_ (sp);
			}

		for (p = sp; *p != '\0' && *p != '='; p++) {
			;
		}

		if (*p == '\0')
			{

			noEqual = TRUE;
			if (logclass & (LC_TRACE | LC_EXEC))
				{
				ls_syslog (LOG_DEBUG,
						   "%s: environment variable <%s> doesn't have '='",
						   __func__, sp);
				}
			}
		else
			{
			*p = '\0';
			}

		if (noEqual == TRUE)
			{
			len1 = 2;
			}
		else
			{
				assert( strlen(p + 1) + 1 <= UINT_MAX );
			len1 = (unsigned int)strlen (p + 1) + 1;
			}

		assert( strlen (sp) + len1 + sizeof (TAILCMD) + strlen (sp) + 1 <= UINT_MAX );
		len = (unsigned int) (strlen (sp) + len1 + sizeof (TAILCMD) + strlen (sp) + 1);
		length += len;
		if ( length > size)
			{
			char *newp = (char *) realloc (jf->data, (size += (len > MSGSIZE ? len : MSGSIZE)));
			if (newp == NULL)
				{
				if (noEqual != TRUE)
					*p = '=';
				lsberrno = LSBE_NO_MEM;
				FREEUP (oldp);
				free (jf->data);
				return -1;
				}
			jf->data = newp;
			}
		strcat (jf->data, sp);
		strcat (jf->data, "='");
		if (noEqual == TRUE)
			{
			strcat (jf->data, "\0");
			}
		else
			{
			strcat (jf->data, p + 1);
			}
		strcat (jf->data, TAILCMD);
		strcat (jf->data, sp);
		strcat (jf->data, "\n");
		if (noEqual != TRUE)
			{
			*p = '=';
			}
		if (logclass & (LC_TRACE | LC_EXEC))
			{
			ls_syslog (LOG_DEBUG,
					   "%s:length=%d, size=%d, jf->len=%d, numEnv=%d", __func__,
					   length, size, strlen (jf->data), ++numEnv);
			}
		FREEUP (oldp);
		}

	len =  sizeof (TRAPSIGCMD); 
	len += sizeof (CMDSTART);
	len += strlen (jobSubReq->command);
	len += tsoptlen;
	len += sizeof (WAITCLEANCMD);
	len += sizeof (EXITCMD);
	len += sizeof (EDATASTART);
	len += ed.len;
	len += MAX_LSB_NAME_LEN;
	length += len;
	if ( length > size)
		{
		char *newp = (char *) realloc (jf->data, (size +=(len > MSGSIZE ? len : MSGSIZE)));
		if ( NULL  == newp && ENOMEM == errno )
			{
			lsberrno = LSBE_NO_MEM;
			FREEUP (jf->data);
			return -1;
			}
		jf->data = newp;
		}
	if (logclass & (LC_TRACE | LC_EXEC))
		{
		ls_syslog (LOG_DEBUG, "%s:length=%d, size=%d, jf->len=%d, numEnv=%d",
				   __func__, length, size, strlen (jf->data), numEnv);
		}

	strcat (jf->data, TRAPSIGCMD);
	strcat (jf->data, CMDSTART);

	strcat (jf->data, jobSubReq->command);

	strcat (jf->data, WAITCLEANCMD);
	strcat (jf->data, EXITCMD);

	appendEData (jf, &ed);

	return 0;
}

void
appendEData (struct lenData *jf, struct lenData *ed_)
{
	char *sp, num[MAX_LSB_NAME_LEN];

	strcat (jf->data, EDATASTART);
	sprintf (num, "%zu\n", ed_->len);
	strcat (jf->data, num);

	sp = jf->data + strlen (jf->data) + 1;
	memcpy (sp, ed_->data, ed_->len);
	jf->len = strlen (jf->data) + 1 + ed_->len;
}


LS_LONG_INT
send_batch (struct submitReq *submitReqPtr, struct lenData *jf, struct submitReply *submitReply, struct lsfAuth *auth)
{
	mbdReqType mbdReqtype;
	XDR xdrs;
	int cc = 0;
	unsigned long reqBufSize = 0;
	char *request_buf = NULL;
	char *reply_buf   = NULL;
	struct LSFHeader hdr;
	struct submitMbdReply *reply = NULL;
	LS_LONG_INT jobId;

	if (logclass & (LC_TRACE | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	reqBufSize = (unsigned long)xdrSubReqSize (submitReqPtr);
	reqBufSize += (unsigned long)xdr_lsfAuthSize (auth);
	request_buf = (char *) malloc (reqBufSize);
	if ( NULL == request_buf && ENOMEM == errno )
		{
		if (logclass & LC_EXEC) {
			ls_syslog (LOG_DEBUG, "%s: request_buf malloc (%d) failed: %m", __func__, reqBufSize);
		}
		lsberrno = LSBE_NO_MEM;
		return -1;
	}



	mbdReqtype = BATCH_JOB_SUB;
	assert( reqBufSize <= UINT_MAX );
	xdrmem_create (&xdrs, request_buf, (unsigned int)reqBufSize, XDR_ENCODE);
	initLSFHeader_ (&hdr);
	hdr.opCode = mbdReqtype;
	if (!xdr_encodeMsg (&xdrs, (char *) submitReqPtr, &hdr, xdr_submitReq, 0, auth))
		{
		xdr_destroy (&xdrs);
		lsberrno = LSBE_XDR;
		free (request_buf);
		return -1;
	}

	assert( XDR_GETPOS (&xdrs) <= INT_MAX );
	cc = callmbd (NULL, request_buf, (int)XDR_GETPOS (&xdrs), &reply_buf, &hdr, NULL, sndJobFile_, (int *) jf);
	if ( cc  < 0)
		{
		xdr_destroy (&xdrs);
		if (logclass & (LC_TRACE | LC_EXEC)) {
			ls_syslog (LOG_DEBUG, "%s: callmbd() failed; cc=%d", __func__, cc);
		}
		free (request_buf);
		return -1;
		}
	xdr_destroy (&xdrs);
	free (request_buf);

	lsberrno = hdr.opCode;
	if (cc == 0)
		{
		submitReply->badJobId = 0;
		submitReply->badReqIndx = 0;
		submitReply->queue = "";
		submitReply->badJobName = "";
		return -1;
		}

	reply = (struct submitMbdReply *) malloc (sizeof (struct submitMbdReply));
	if ( NULL == reply && ENOMEM == errno )
		{
		lsberrno = LSBE_NO_MEM;
		free (reply);
		return -1;
		}

	assert( XDR_DECODE_SIZE_ (cc) >= 0 );
	xdrmem_create (&xdrs, reply_buf, (unsigned int)XDR_DECODE_SIZE_ (cc), XDR_DECODE);

	if (!xdr_submitMbdReply (&xdrs, reply, &hdr))
		{
		lsberrno = LSBE_XDR;
		free (reply_buf);
		free (reply);
		xdr_destroy (&xdrs);
		return -1;
		}

	free (reply_buf);

	xdr_destroy (&xdrs);


	submitReply->badJobId = reply->jobId;
	submitReply->badReqIndx = reply->badReqIndx;
	submitReply->queue = reply->queue;
	submitReply->badJobName = reply->badJobName;

	if (lsberrno == LSBE_NO_ERROR)
		{
		if (reply->jobId == 0) {
			lsberrno = LSBE_PROTOCOL;
		}
		jobId = reply->jobId;
		free (reply);
		if (logclass & (LC_TRACE | LC_EXEC)) {
			ls_syslog (LOG_DEBUG1, "%s: mbd says job <%s> has been restarted", __func__, lsb_jobid2str (jobId));
		}
		return jobId;
		}

	free (reply);
	return -1;
}


// FIXME figure out if this function is needed
int
dependCondSyntax (char *dependCond)
{
	assert( *dependCond );
	return 0;
}

LS_LONG_INT
subRestart (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth)
{	
	char *resReq        = NULL;
	char *fromHost      = NULL;
	char *chkPath       = NULL;
	char *mailUser      = NULL;
	char *projectName   = NULL;
	char *loginShell    = NULL;
	char *schedHostType = NULL;

	struct lenData jf = { 0, NULL }; 
  
	struct jobNewLog *jobLog = NULL;
	struct xFile *xFiles     = NULL;
	LS_LONG_INT jobId        = -1;
	LS_WAIT_T status         = 0;
	char *ptr                = NULL ;
	unsigned int length      = 0;
	size_t lineNum           = 0 ;
	int childIoFd[2]         = { 0, 0 };
	pid_t pid                = 0;
	struct
	{
		int error;
		int eno;
		int lserrno;
		int lsberrno;
	} err = { 0, 0, 0, 0 } ;
	struct linger linstr = { 1, 5 };

	chkPath = malloc( sizeof( char ) * MAX_FILENAME_LEN + 1);
	
	if (logclass & (LC_TRACE | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}


	if (socketpair (AF_UNIX, SOCK_STREAM, 0, childIoFd) < 0) {
		lsberrno = LSBE_SYS_CALL;
		return -1;
	}

	makeCleanToRunEsub ();

	pid = fork ();
	if (pid < 0) {
		lsberrno = LSBE_SYS_CALL;
		return -1;
	}
	else if (pid == 0)
	{
		FILE *fp = 0 ;
		uid_t uid = 0;
		struct eventRec *logPtr = NULL;
		char *chklog = malloc( sizeof( char ) * MAX_FILENAME_LEN + 1 );
		int exitVal = -1;

		close (childIoFd[0]);
		setsockopt (childIoFd[1], SOL_SOCKET, SO_LINGER, &linstr, sizeof (linstr));

		uid = getuid ();
		if( setuid( uid ) < 0) {
			lsberrno = LSBE_BAD_USER;
			goto sendErr;    // FIXME FIXME FIXME FIXME remove goto
		}

		if (getChkDir (jobSubReq->command, chkPath) == -1) {
			lsberrno = LSBE_BAD_CHKLOG;
		}
		else {
			sprintf (chklog, "%s/chklog", chkPath);
		}

		if (write (childIoFd[1], chkPath, sizeof (chkPath)) != sizeof (chkPath)) {
			goto childExit;    // FIXME FIXME FIXME FIXME remove goto
		}

		if (lsberrno == LSBE_BAD_CHKLOG) {
			goto sendErr;     // FIXME FIXME FIXME FIXME remove goto
		}

		if (logclass & (LC_TRACE | LC_EXEC)) {
			ls_syslog (LOG_DEBUG1, "%s: Child tries to open chklog file <%s>", __func__, chklog);
		}
		if( ( fp = fopen( chklog, "r" ) ) == NULL )	{
			lsberrno = LSBE_BAD_CHKLOG;
			goto sendErr;
		}
		if ((logPtr = lsb_geteventrec (fp, &lineNum)) == NULL || logPtr->type != EVENT_JOB_NEW) {
			lsberrno = LSBE_BAD_CHKLOG;
			fclose (fp);
			goto sendErr;
		}
		jobLog = &logPtr->eventLog.jobNewLog;

		if (logclass & (LC_TRACE | LC_EXEC)) {
			ls_syslog (LOG_DEBUG1, "%s: Child got job log from chklog file", __func__);
		}

		err.error = FALSE;
		if (write (childIoFd[1], &err, sizeof (err)) != sizeof (err)) {
			goto childExit;
		}

		if (write (childIoFd[1], &ed.len, sizeof (ed.len)) != sizeof (ed.len)) {
			goto childExit;
		}

		if (ed.len > 0) {
			ssize_t temp = write (childIoFd[1], ed.data, ed.len);
			if( temp == -1 ) {
				ls_syslog (LOG_DEBUG1, "%s: Write could not complete", __func__);
			}

			if ( (size_t)temp != ed.len) {
				goto childExit;   // FIXME FIXME FIXME FIXME remove goto
			}
		}


		// FIXME FIXME FIXME FIXME FIXME 
		// jobLog->errFile is DEFINATELLY WRONG
		// THIS MUST BE ADDRESSED BY ATTACHING THE DEBUGGER
		// ... or figuring out wtf does this code is supposed to do.
		if( b_write_fix( childIoFd[1], jobLog->errFile, sizeof( struct jobNewLog ) ) != sizeof( struct jobNewLog ) )
		{
			goto childExit;
		}

		assert( strlen(jobLog->resReq) + 1 <= UINT_MAX );
		if ((length = strlen (jobLog->resReq) + 1) >= MAX_LINE_LEN) {
			goto childExit;
		}

		if (write (childIoFd[1], &length, sizeof (length)) != sizeof (length)) {
			goto childExit;   // FIXME FIXME FIXME FIXME remove goto
		}

		if (write (childIoFd[1], jobLog->resReq, length) != length) {
			goto childExit;   // FIXME FIXME FIXME FIXME remove goto
		}

		if ((jobLog->options & SUB_OTHER_FILES) && jobLog->nxf > 0) {
			assert(jobLog->nxf >= 0);
			length = jobLog->nxf * sizeof (struct xFile);
			if (write (childIoFd[1], jobLog->xf, length) != length) {
				goto childExit;    // FIXME FIXME FIXME FIXME remove goto
			}
		}
		if (jobLog->options & SUB_MAIL_USER) {
			assert( strlen(jobLog->mailUser) + 1 <= UINT_MAX );
			if ((length = strlen (jobLog->mailUser) + 1) >= MAX_LSB_NAME_LEN) {
				goto childExit;    // FIXME FIXME FIXME FIXME remove goto
			}
			if (write (childIoFd[1], &length, sizeof (length))  != sizeof (length)) {
				goto childExit;    // FIXME FIXME FIXME FIXME remove goto
			}
			if (write (childIoFd[1], jobLog->mailUser, length) != length) {
				goto childExit;    // FIXME FIXME FIXME FIXME remove goto
			}
		}

		if (jobLog->options & SUB_PROJECT_NAME ) {
			assert( strlen(jobLog->projectName) + 1 <= UINT_MAX );
			if ((length = strlen (jobLog->projectName) + 1) >= MAX_LSB_NAME_LEN) {
				goto childExit;     // FIXME FIXME FIXME FIXME remove goto
			}
			if (write (childIoFd[1], &length, sizeof (length))!= sizeof (length)) {
				goto childExit;   // FIXME FIXME FIXME FIXME remove goto
			}
			if (write (childIoFd[1], jobLog->projectName, length) != length) {
				goto childExit;   // FIXME FIXME FIXME FIXME remove goto
			}
		}

		if (jobLog->options & SUB_LOGIN_SHELL) {
			assert( strlen(jobLog->loginShell) + 1 <= UINT_MAX );
			if ((length = strlen (jobLog->loginShell) + 1) >= MAX_LSB_NAME_LEN) {
				goto childExit;   // FIXME FIXME FIXME FIXME remove goto
			}
			if (write (childIoFd[1], &length, sizeof (length)) != sizeof (length)) {
				goto childExit;   // FIXME FIXME FIXME FIXME remove goto
			}
			if (write (childIoFd[1], jobLog->loginShell, length) != length) {
				goto childExit;   // FIXME FIXME FIXME FIXME remove goto
			}
		}
		assert( strlen(jobLog->schedHostType) + 1 <= UINT_MAX );
		if ((length = strlen (jobLog->schedHostType) + 1) >= MAX_LSB_NAME_LEN) {
			goto childExit;   // FIXME FIXME FIXME FIXME remove goto
		}
		if (write (childIoFd[1], &length, sizeof (length)) != sizeof (length)) {
			goto childExit;   // FIXME FIXME FIXME FIXME remove goto
		}
		if (write (childIoFd[1], jobLog->schedHostType, length) != length) {
			goto childExit;
		}


		if ((logPtr = lsb_geteventrec (fp, &lineNum)) == NULL || logPtr->type != EVENT_JOB_START) {
			goto childExit;   // FIXME FIXME FIXME FIXME remove goto
		}
		length = sizeof (logPtr->eventLog.jobStartLog.jobPGid);
		if (write (childIoFd[1], &logPtr->eventLog.jobStartLog.jobPGid, length) != length) {
			goto childExit;
		}
		if (logPtr->eventLog.jobStartLog.numExHosts <= 0) {
			goto childExit;
		}
		assert( strlen (logPtr->eventLog.jobStartLog.execHosts[0]) + 1 <= UINT_MAX);
		length = strlen (logPtr->eventLog.jobStartLog.execHosts[0]) + 1;
		if (length >= MAXHOSTNAMELEN) {
			goto childExit;
		}
		if (write (childIoFd[1], &length, sizeof (length)) != sizeof (length)) {
			goto childExit;
		}
		if (write (childIoFd[1], logPtr->eventLog.jobStartLog.execHosts[0], length) != length) {
			goto childExit;
		}
		exitVal = 0;

childExit:
		{
			if (logclass & (LC_TRACE | LC_EXEC)) {
				if (exitVal == 0) {
					ls_syslog (LOG_DEBUG1, "%s: Child succeeded in sending messages to parent", __func__);
				}
				else{
					ls_syslog (LOG_DEBUG, "%s: Child failed in sending messages to parent", __func__);
				}
			}
			fclose (fp);
			close (childIoFd[1]);
			exit (exitVal);

			sendErr:
			err.error = TRUE;
			err.eno = errno;
			err.lserrno = lserrno;
			err.lsberrno = lsberrno;

			if (write (childIoFd[1], &err, sizeof (err)) != sizeof (err)) {
				close (childIoFd[1]);
				exit (-1);
			}
			close (childIoFd[1]);
			exit (0);
		}

		close (childIoFd[1]);

		if (read (childIoFd[0], chkPath, sizeof (chkPath)) != sizeof (chkPath)) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}

		err.error = FALSE;
		if (read (childIoFd[0], &err, sizeof (err)) != sizeof (err)) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}
		if (err.error)
		{
			errno = err.eno;
			lserrno = err.lserrno;
			lsberrno = err.lsberrno;
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}


		if (read (childIoFd[0], &ed.len, sizeof (ed.len)) != sizeof (ed.len)) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}

		if (ed.len > 0) {

			long temp = 0;
			ed.data = malloc (ed.len);
			if (NULL == ed.data && ENOMEM == errno ) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}

			temp = b_read_fix (childIoFd[0], ed.data, ed.len);
			assert( temp <= LONG_MAX && temp >= 0);
			assert( ed.len <= LONG_MAX );
			if ( temp != (long) ed.len) {
				FREEUP (ed.data);
				ed.len = 0;
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
		}
		else {
			ed.data = NULL;
		}

		jobLog = malloc (sizeof ( struct jobNewLog ) );
		if ( NULL == jobLog && ENOMEM == errno ) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}
		if (b_read_fix (childIoFd[0], (char *) jobLog, sizeof (struct jobNewLog)) != sizeof (struct jobNewLog)) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}
		if (read (childIoFd[0], (char *) &length, sizeof (length)) != sizeof (length)) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}
		jobLog->resReq = resReq;
		if (read (childIoFd[0], jobLog->resReq, length) != length) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}
		if ((jobLog->options & SUB_OTHER_FILES) && jobLog->nxf > 0)
		{
			assert( jobLog->nxf >= 0);
			length = jobLog->nxf * sizeof (struct xFile);
			xFiles = malloc (length); // FIXME FIXME FIXME FIXME FIXME wat
			if ( NULL == xFiles && ENOMEM == errno ) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
			jobLog->xf = xFiles;
			if (read (childIoFd[0], jobLog->xf, length) != length) {
				free (jobLog->xf);
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
		}

		if (jobLog->options & SUB_MAIL_USER) {
			if (read (childIoFd[0], &length, sizeof (length)) != sizeof (length)) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
			jobLog->mailUser = mailUser;
			if (read (childIoFd[0], jobLog->mailUser, length) != length) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
		}

		if (jobLog->options & SUB_PROJECT_NAME) {
			if (read (childIoFd[0], &length, sizeof (length)) != sizeof (length)) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
			jobLog->projectName = projectName;
			if (read (childIoFd[0], jobLog->projectName, length) != length) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
		}


		if (jobLog->options & SUB_LOGIN_SHELL) {
			if (read (childIoFd[0], &length, sizeof (length)) != sizeof (length)) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
			jobLog->loginShell = loginShell;
			if (read (childIoFd[0], jobLog->loginShell, length) != length) {
				goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
			}
		}
		if (read (childIoFd[0], &length, sizeof (length)) != sizeof (length)) {
			goto parentErr;    // FIXME FIXME FIXME FIXME remove goto
		}
		jobLog->schedHostType = schedHostType;
		if (read (childIoFd[0], jobLog->schedHostType, length) != length) {
			goto parentErr;
		}


		length = sizeof (submitReq->restartPid);
		if (read (childIoFd[0], (char *) &submitReq->restartPid, length) != length) {
			goto parentErr;
		}
		if (read (childIoFd[0], (char *) &length, sizeof (length)) != sizeof (length)) {
			goto parentErr;
		}
		submitReq->fromHost = fromHost;
		if (read (childIoFd[0], submitReq->fromHost, length) != length) {
			goto parentErr;
		}

		if (logclass & (LC_TRACE | LC_EXEC)) {
			ls_syslog (LOG_DEBUG1, "%s: Parent got the job log from child", __func__);
		}


		lsberrno = LSBE_BAD_CHKLOG;


		if (strlen (jobLog->jobName) >= MAX_CMD_DESC_LEN) {
			goto parentErr;		// FIXME FIXME FIXME FIXME replace label with code;
		}

		if ((ptr = strchr (jobLog->jobName, '[')) != NULL && strrchr (jobSubReq->command, '/')) {

			char *element = malloc( sizeof(char ) * sizeof( u_long  ) * 8 + 1); // FIXME FIXME FIXME these numbers look awfully suspect? peculiar? 
			ptr = NULL;
			ptr = strrchr (jobSubReq->command, '/');
			ptr++;
			if (islongint_ (ptr)) {

				long temp = 0;
				temp = b_read_fix (childIoFd[0], ed.data, ed.len); // FIXME FIXME FIXME FIXME FIXME FIXME this might not acutally do what the code intents to
				assert( temp <= LONG_MAX && temp >= 0);
				assert( ed.len <= LONG_MAX );
				if ( temp != (long) ed.len) {
					sprintf (element, "[%d]", lsb_array_idx(atoi64_ (ptr)));  // this may be the wrong size to return;
				}
				strcat (jobLog->jobName, element);
			}

			if ((ptr = strchr (jobLog->jobName, '/')) != NULL) {
				if (ptr == strrchr (jobLog->jobName, '/') ) {
					sprintf (jobLog->jobName, "%s", ++ptr);
				}
			}

			submitReq->jobName = jobLog->jobName;
			submitReq->options |= SUB_JOB_NAME;

			if (!(jobSubReq->options & SUB_QUEUE)) {
				if (strlen (jobLog->queue) >= MAX_LSB_NAME_LEN - 1) {
					goto parentErr;
				}
				submitReq->queue = jobLog->queue;
				submitReq->options |= SUB_QUEUE;
			}

			if (strlen (jobLog->resReq) >= MAX_LINE_LEN - 1) {
				goto parentErr;
			}
			submitReq->resReq = jobLog->resReq;
			if (jobLog->options & SUB_RES_REQ) {
				submitReq->options |= SUB_RES_REQ;
			}
// FIXME FIXME FIXME file a ticket to check for negative host numbers from command-line tools or conf
/*    		if (jobLog->numProcessors < 0) {
				goto parentErr;
			}
*/    		submitReq->numProcessors = jobLog->numProcessors;

/*    	if (jobLog->maxNumProcessors < 0) {
			goto parentErr;
	}*/
			submitReq->maxNumProcessors = jobLog->maxNumProcessors;

			if (strlen (jobLog->inFile) >= MAX_FILENAME_LEN - 1) {
				goto parentErr;
			}
			submitReq->inFile = jobLog->inFile;
			if (jobLog->options & SUB_IN_FILE) {
				submitReq->options |= SUB_IN_FILE;
			}

			if (strlen (jobLog->outFile) >= MAX_FILENAME_LEN - 1) {
				goto parentErr;
			}
			submitReq->outFile = jobLog->outFile;
			if (jobLog->options & SUB_OUT_FILE) {
				submitReq->options |= SUB_OUT_FILE;
			}

			if (strlen (jobLog->errFile) >= MAX_FILENAME_LEN - 1) {
				goto parentErr;
			}
			submitReq->errFile = jobLog->errFile;
			if (jobLog->options & SUB_ERR_FILE) {
				submitReq->options |= SUB_ERR_FILE;
			}

			if (strlen (jobLog->inFileSpool) >= MAX_FILENAME_LEN) {
				goto parentErr;
			}
			submitReq->inFileSpool = jobLog->inFileSpool;
			if (jobLog->options2 & SUB2_IN_FILE_SPOOL) {
				submitReq->options2 |= SUB2_IN_FILE_SPOOL;
			}

			if (strlen (jobLog->commandSpool) >= MAX_FILENAME_LEN) {
				goto parentErr;
			}
			submitReq->commandSpool = jobLog->commandSpool;
			if (jobLog->options2 & SUB2_JOB_CMD_SPOOL) {
				submitReq->options2 |= SUB2_JOB_CMD_SPOOL;
			}

			if (strlen (jobLog->command) >= MAX_CMD_DESC_LEN) {
				goto parentErr;
			}
			submitReq->command = jobLog->command;

			if (jobLog->userPriority > 0) {
			submitReq->userPriority = jobLog->userPriority;
			}

			submitReq->chkpntPeriod = jobLog->chkpntPeriod;
			if (jobLog->options & SUB_CHKPNT_PERIOD) {
				submitReq->options |= SUB_CHKPNT_PERIOD;
			}

			submitReq->options |= SUB_CHKPNT_DIR;
			if (strlen (jobLog->chkpntDir) >= MAX_FILENAME_LEN - 1) {
				goto parentErr;
			}
			submitReq->chkpntDir = chkPath;

			if (strlen (jobLog->jobFile) >= MAX_FILENAME_LEN - 1) {
				goto parentErr;
			}
			submitReq->jobFile = jobLog->jobFile;

			submitReq->umask = jobLog->umask;

			if (strlen (jobLog->cwd) >= MAX_FILENAME_LEN - 1) {
				goto parentErr;
			}
			submitReq->cwd = jobLog->cwd;

			if (strlen (jobLog->subHomeDir) >= MAX_FILENAME_LEN - 1) {
				goto parentErr;
			}
			submitReq->subHomeDir = jobLog->subHomeDir;

			submitReq->sigValue = (jobLog->options & SUB_WINDOW_SIG) ? sig_encode (jobLog->sigValue) : 0;
			if (submitReq->sigValue > 31 || submitReq->sigValue < 0) {
				goto parentErr;
			}

			if (jobLog->options & SUB_MAIL_USER) {
				submitReq->mailUser = jobLog->mailUser;
				submitReq->options |= SUB_MAIL_USER;
			}
			else {
				submitReq->mailUser = "";
			}

			if (jobLog->options & SUB_PROJECT_NAME) {
				submitReq->projectName = jobLog->projectName;
				submitReq->options |= SUB_PROJECT_NAME;
			}
			else {
				submitReq->projectName = "";
			}

			if (jobLog->options & SUB_LOGIN_SHELL) {
				submitReq->loginShell = jobLog->loginShell;
				submitReq->options |= SUB_LOGIN_SHELL;
			}
			else {
				submitReq->loginShell = "";
			}


			if (jobLog->options & SUB_RERUNNABLE) {
				submitReq->options |= SUB_RERUNNABLE;
			}

			if (jobLog->options2 & SUB2_HOLD) {
				submitReq->options2 |= SUB2_HOLD;
			}

			if ((jobLog->options & SUB_OTHER_FILES) && jobLog->nxf > 0) {
				submitReq->options |= SUB_OTHER_FILES;
				submitReq->nxf = jobLog->nxf;
				if (jobLog->nxf <= 0) {
					goto parentErr;
				}
				submitReq->xf = jobLog->xf;
			}
			else {
				submitReq->nxf = 0;
			}
			if (jobLog->schedHostType)
				submitReq->schedHostType = jobLog->schedHostType;
			else {
				submitReq->schedHostType = "";
			}

			close (childIoFd[0]);
			if (waitpid (pid, &status, 0) < 0) {
				lsberrno = LSBE_SYS_CALL;
				goto leave;
			}
			if (createJobInfoFile (jobSubReq, &jf) == -1) {
				goto leave;
			}

			jobId = send_batch (submitReq, &jf, submitRep, auth);

			if (jobId > 0) {
				if (!getenv ("BSUB_QUIET")) {
					postSubMsg (jobSubReq, jobId, submitRep);
				}
			}

leave:

			if (jobLog) {
				free (jobLog);
			}
			if (xFiles) {
				free (xFiles);
			}
			return jobId;

parentErr:
			if (logclass & (LC_TRACE | LC_EXEC)) {
				ls_syslog (LOG_DEBUG, "%s: Parent failed in receiving messages from child", __func__);
			}
			if (!err.error) {
				lsberrno = LSBE_SYS_CALL;
				err.eno = errno;
			}

			close (childIoFd[0]);
			if (waitpid (pid, &status, 0) < 0) {
				lsberrno = LSBE_SYS_CALL;
			}
			else {
				errno = err.eno;
			}
			jobId = -1;
			goto leave;
		}
	} // FIXME FIXME FIXME FIXME FIXME there is a 99% chance this bracket is in the wrong place
	return 0;
}


int
getChkDir (char *givenDir, char *chkPath)
{
	char *strPtr = NULL;

	if (((strPtr = strrchr (givenDir, '/')) != NULL) && (islongint_ (strPtr + 1))) {
		sprintf (chkPath, "%s", givenDir);
		return 0;
	}
	else {
		DIR *dirp = NULL;
		struct dirent *dp = NULL;
		char jobIdDir[MAX_FILENAME_LEN];
		int i = 0;

		i = 0;
		if ((dirp = opendir (givenDir)) == NULL) {
			return -1;
		}
		while ((dp = readdir (dirp)) != NULL) {

			if (strcpy (jobIdDir, dp->d_name) == NULL) {
				i++;
			}
			if (i > 1) {
				closedir (dirp);
				return -1;
			}
		}
		if (islongint_ (jobIdDir)) {
			sprintf (chkPath, "%s/%s", givenDir, jobIdDir);
			closedir (dirp);
			return 0;
		}
		else {
			closedir (dirp);
			return -1;
		}
	}

	fprintf( stderr, "%s: i am not sure you are supposed to be here.\n", __func__ );
	return -666;
}


LS_LONG_INT
subJob (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth)
{

	char homeDir[MAX_FILENAME_LEN]; // FIXME FIXME FIXME convert to dynamic string
	char resReq[MAX_LINE_LEN]; // FIXME FIXME FIXME convert to dynamic string
	char cmd[MAX_LINE_LEN]; // FIXME FIXME FIXME convert to dynamic string
	struct lenData jf;
	int niosSock = 0;
	LSB_SUB_SPOOL_FILE_T subSpoolFiles;
	LS_LONG_INT jobId = -1;

	subSpoolFiles.inFileSpool[0] = 0;
	subSpoolFiles.commandSpool[0] = 0;

	submitReq->subHomeDir = homeDir; // FIXME FIXME FIXME FIXME FIXME the fuck is this and why?
	submitReq->resReq = resReq; // FIXME FIXME FIXME FIXME FIXME the fuck is this and why?
	submitReq->command = cmd; // FIXME FIXME FIXME FIXME FIXME the fuck is this and why?

	if (getOtherParams (jobSubReq, submitReq, submitRep, auth, &subSpoolFiles) < 0)
		{
		goto cleanup;
		}

	if (createJobInfoFile (jobSubReq, &jf) == -1)
		{
		goto cleanup;
		}

	if (submitReq->options & SUB_INTERACTIVE)
		{
		if (submitReq->options & SUB_PTY)
			{
			if (!isatty (0) && !isatty (1))
				submitReq->options &= ~SUB_PTY;
			}
		if ((niosSock = createNiosSock (submitReq)) < 0)
			{
			goto cleanup;
			}
		}

	if (submitReq->options2 & SUB2_BSUB_BLOCK)
		{
		if ((niosSock = createNiosSock (submitReq)) < 0)
			{
			goto cleanup;
			}
		}

	jobId = send_batch (submitReq, &jf, submitRep, auth);
	free (jf.data);

	if (jobId > 0)
		{

		if (submitReq->options & SUB_INTERACTIVE)
			{
			sigset_t sigMask;
			sigemptyset (&sigMask);
			sigaddset (&sigMask, SIGINT);
			sigprocmask (SIG_BLOCK, &sigMask, NULL);

			}

		if (!getenv ("BSUB_QUIET")) {
			postSubMsg (jobSubReq, jobId, submitRep);
		}

		if (submitReq->options & SUB_INTERACTIVE)
			{
			int jobIdFd, pid;
			char *envBuf;
			if ((envBuf = getenv ("JOBID_FD")) != NULL)
				{
				jobIdFd = atoi (envBuf);
				if (b_write_fix (jobIdFd, (char *) &jobId, sizeof (jobId))
					!= sizeof (jobId))
					{
					goto cleanup;
					}
				if (b_read_fix (jobIdFd, (char *) &pid, sizeof (pid))
					!= sizeof (pid))
					{
					goto cleanup;
					}
				close (jobIdFd);
				}

			startNios (submitReq, niosSock, jobId);
					// startNios does not return
					// exit (-1);
			}

		if (submitReq->options2 & SUB2_BSUB_BLOCK)
			{
			startNios (submitReq, niosSock, jobId);
				// startNios does not return
				// exit (-1);
			}
		}

cleanup:

	if (jobId < 0)
		{

		const char *spoolHost;
		int err;


		if (subSpoolFiles.inFileSpool[0])
			{
			spoolHost = getSpoolHostBySpoolFile (subSpoolFiles.inFileSpool);
			err = chUserRemoveSpoolFile (spoolHost, subSpoolFiles.inFileSpool);
			if (err)
				{
				/* catgets 442 */
				fprintf (stderr, "catgets 442: Submission failed, and the spooled file <%s> can not be removed on host <%s>, please manually remove it", subSpoolFiles.inFileSpool, spoolHost);
				}
			}


		if (subSpoolFiles.commandSpool[0])
			{
			spoolHost = getSpoolHostBySpoolFile (subSpoolFiles.commandSpool);
			err = chUserRemoveSpoolFile (spoolHost, subSpoolFiles.commandSpool);
			if (err)
				{
				/* catgets 442 */
				fprintf (stderr, "catgets 442: Submission failed, and the spooled file <%s> can not be removed on host <%s>, please manually remove it", subSpoolFiles.commandSpool, spoolHost);
				}
			}
		}

	return jobId;
}



static const char *
getDefaultSpoolDir ()
{
	char *spoolDir   = NULL;
	char *clusterName = NULL;

	clusterName = ls_getclustername ();
	if (clusterName == NULL)
		{
		return NULL;
		}

	sprintf (spoolDir, "%s/%s", lsbParams[LSB_SHAREDIR].paramValue, clusterName);

	return spoolDir;
}


const LSB_SPOOL_INFO_T *
chUserCopySpoolFile (const char *srcFile, spoolOptions_t fileType)
{
	static LSB_SPOOL_INFO_T parentSpoolInfo;
	pid_t pid = 0;
	ssize_t cc  = 0;
	int exitVal = 0;
	int rcpp[2] = { 0 , 0 };
	LS_WAIT_T status;
	

	if (defaultSpoolDir == NULL)
		{
		defaultSpoolDir = getDefaultSpoolDir ();
		}

	memset (&parentSpoolInfo, 0, sizeof (parentSpoolInfo));

	if (pipe (rcpp) < 0)
		{
		perror ("chUserCopySpoolFile:pipe");
		lsberrno = LSBE_SYS_CALL;
		return NULL;
		}

	switch (pid = fork ())
	{
		case 0:

		close (rcpp[0]);

		exitVal = -1;

		{
		const LSB_SPOOL_INFO_T *childSpoolInfo;

		setuid (getuid ());
		childSpoolInfo = copySpoolFile (srcFile, fileType);
		if (childSpoolInfo)
			{
			memcpy (&parentSpoolInfo, childSpoolInfo,
					sizeof (parentSpoolInfo));
			exitVal = 0;
			}
		}

		cc = write (rcpp[1], &parentSpoolInfo, sizeof (parentSpoolInfo));
		assert( cc >= 0 );
		if ( (size_t) cc < sizeof (parentSpoolInfo ))
			{
			exitVal = -1;
			}
		close (rcpp[1]);

		exit (exitVal);

		case -1:

		close (rcpp[0]);
		close (rcpp[1]);
		perror ("chUserCopySpoolFile:fork");
		lsberrno = LSBE_SYS_CALL;
		return NULL;

		default:

		close (rcpp[1]);

		cc = read (rcpp[0], &parentSpoolInfo, sizeof (parentSpoolInfo));
		assert (cc >= 0 );
		if ( (size_t)cc < sizeof (parentSpoolInfo))
			{
			perror ("chUserCopySpoolFile:read");
			lsberrno = LSBE_SYS_CALL;
			return NULL;
			}
		close (rcpp[0]);

		if (waitpid (pid, &status, 0) < 0)
			{
			perror ("chUserCopySpoolFile:waitpid");
			lsberrno = LSBE_SYS_CALL;
			return NULL;
			}


		if (WIFEXITED (status) == 0)
			{
			/* catgets 443 */
			fprintf (stderr, "catgets 443: Child process killed by signal.\n");  
			lsberrno = LSBE_SYS_CALL;
			return NULL;
			}
		else
			{
			if (WEXITSTATUS (status) == 0xff)
				{
				lsberrno = LSBE_SYS_CALL;
				return NULL;
				}
			}

		break;
	}

	return &parentSpoolInfo;
}


int
chUserRemoveSpoolFile (const char *hostName, const char *spoolFile)
{
	pid_t pid;
	LS_WAIT_T status;
	const char *sp;
	char dirName[MAX_FILENAME_LEN];

	sp = getLowestDir_ (spoolFile);
	if (sp)
		{
		strcpy (dirName, sp);
		}
	else
		{
		strcpy (dirName, spoolFile);
		}

	switch (pid = fork ())
	{
		case 0:


		setuid (getuid ());
		if (removeSpoolFile (hostName, dirName) == 0)
			{
			exit (0);
			}
		else
			{
			exit (-1);
			}
		// break; will never be executed

		case -1:

		perror ("chUserRemoveSpoolFile:fork");
		lsberrno = LSBE_SYS_CALL;
		return -1;

		default:


		if (waitpid (pid, &status, 0) < 0)
			{
			perror ("chUserRemoveSpoolFile:waitpid");
			lsberrno = LSBE_SYS_CALL;
			return -1;
			}


		if (WIFEXITED (status) == 0)
			{
			/* catgets 443 */
			fprintf (stderr, "catgets 443:Child process killed by signal.\n");
			lsberrno = LSBE_SYS_CALL;
			return -1;
			}
		else
			{
			if (WEXITSTATUS (status) == 0xff)
				{
				lsberrno = LSBE_SYS_CALL;
				return -1;
				}
			}

	}

	return 0;
}


int
getOtherParams (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth, LSB_SUB_SPOOL_FILE_T * subSpoolFiles)
{
	char jobdespBuf[MAX_CMD_DESC_LEN];
	char lineStrBuf[MAX_LINE_LEN];
	char *sp = NULL;
	char *jobdesp = NULL;
	char *myHostName = NULL;
	struct passwd *pw = NULL;
	long jobSubReqCmd1Offset = 0;
	int counter = 0;

	char lastNonSpaceChar = 'a';
	unsigned int lastNonSpaceIdx  = 0;

	assert ( submitRep->queue );
	
	if (jobSubReq->options & SUB_JOB_NAME)
		{
		if (!jobSubReq->jobName)
			{
			lsberrno = LSBE_BAD_JOB;
			return -1;
			}


		if (strlen (jobSubReq->jobName) >= MAX_CMD_DESC_LEN - 1)
			{
			lsberrno = LSBE_BAD_JOB;
			return -1;
			}
		else {
			submitReq->jobName = jobSubReq->jobName;
		}
		}
	else {
		submitReq->jobName = "";
	}

	if (jobSubReq->options & SUB_IN_FILE)
		{
		if (!jobSubReq->inFile)
			{
			lsberrno = LSBE_BAD_ARG;
			return -1;
			}

		if (strlen (jobSubReq->inFile) >= MAX_FILENAME_LEN - 1)
			{
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
			}
		submitReq->inFile = jobSubReq->inFile;
		submitReq->inFileSpool = "";
		}
	else if (jobSubReq->options2 & SUB2_IN_FILE_SPOOL)
		{

		const char *pSpoolFile = NULL;
		const LSB_SPOOL_INFO_T *spoolInfo = NULL;
		unsigned long spoolFileLen = 0;


		spoolInfo = chUserCopySpoolFile (jobSubReq->inFile, SPOOL_INPUT_FILE);
		if (spoolInfo == NULL)
			{
			return -1;
			}


		pSpoolFile = spoolInfo->spoolFile;
		spoolFileLen = strlen (pSpoolFile);
		if (spoolFileLen >= MAX_FILENAME_LEN)
			{
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
			}
		memcpy (subSpoolFiles->inFileSpool, pSpoolFile, spoolFileLen + 1);
		submitReq->inFileSpool = subSpoolFiles->inFileSpool;
		submitReq->inFile = jobSubReq->inFile;
		}
	else
		{
		submitReq->inFileSpool = "";
		submitReq->inFile = "";
		}


	if (jobSubReq->options & SUB_MAIL_USER)
		{
		if (!jobSubReq->mailUser)
			{
			lsberrno = LSBE_BAD_ARG;
			return -1;
			}
		if (strlen (jobSubReq->mailUser) >= MAXHOSTNAMELEN - 1)
		{
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
		}
		submitReq->mailUser = jobSubReq->mailUser;
		}
	else {
		submitReq->mailUser = "";
	}

	if (jobSubReq->options & SUB_PROJECT_NAME)
		{
		if (!jobSubReq->projectName)
			{
			lsberrno = LSBE_BAD_ARG;
			return -1;
			}
		if (strlen (jobSubReq->projectName) >= MAX_LSB_NAME_LEN - 1)
			{
			lsberrno = LSBE_BAD_ARG;
			errno = ENAMETOOLONG;
			return -1;
			}
		submitReq->projectName = jobSubReq->projectName;
		}
	else {
		submitReq->projectName = "";
	}

	if (jobSubReq->options & SUB_OUT_FILE)
		{
		if (!jobSubReq->outFile)
			{
			lsberrno = LSBE_BAD_ARG;
			return -1;
			}

		if (strlen (jobSubReq->outFile) >= MAX_FILENAME_LEN - 1)
			{
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
			}
		submitReq->outFile = jobSubReq->outFile;
		}
	else {
		submitReq->outFile = "";
	}

	if (jobSubReq->options & SUB_ERR_FILE)
		{
		if (!jobSubReq->errFile)
			{
			lsberrno = LSBE_BAD_ARG;
			return -1;
			}

		if (strlen (jobSubReq->errFile) >= MAX_FILENAME_LEN - 1)
			{
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
			}
		submitReq->errFile = jobSubReq->errFile;
		}
	else {
		submitReq->errFile = "";
	}

	if (jobSubReq->options & SUB_CHKPNT_PERIOD)
		{
		if (!(jobSubReq->options & SUB_CHKPNTABLE)) {
			return -1;
		}

		if (jobSubReq->chkpntPeriod < 0) {
			return -1;
		}
		else {
			submitReq->chkpntPeriod = jobSubReq->chkpntPeriod;
		}
		}
	else {
		submitReq->chkpntPeriod = 0;
	}

	if (jobSubReq->options & SUB_CHKPNT_DIR)
		{
		if (!jobSubReq->chkpntDir)
			{
			lsberrno = LSBE_BAD_ARG;
			return -1;
			}

		if (strlen (jobSubReq->chkpntDir) >= MAX_FILENAME_LEN - 1)
			{
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
			}

		submitReq->chkpntDir = jobSubReq->chkpntDir;
		submitReq->options |= SUB_CHKPNTABLE;
		}
	else {
		submitReq->chkpntDir = "";
	}


// FIXME FIXME FIXME check this out:  if (jobSubReq->numProcessors < 0 || jobSubReq->maxNumProcessors < jobSubReq->numProcessors)
	if ( jobSubReq->maxNumProcessors < jobSubReq->numProcessors)
		{
		lsberrno = LSBE_BAD_ARG;
		return -1;
		}

	if (jobSubReq->numProcessors == 0 && jobSubReq->maxNumProcessors == 0)
		{

		jobSubReq->options2 |= SUB2_USE_DEF_PROCLIMIT;
		submitReq->options2 |= SUB2_USE_DEF_PROCLIMIT;
		}

	if (jobSubReq->numProcessors != DEFAULT_NUMPRO && jobSubReq->maxNumProcessors != DEFAULT_NUMPRO)
		{
		submitReq->numProcessors = (jobSubReq->numProcessors) ? jobSubReq->numProcessors : 1;
		submitReq->maxNumProcessors = (jobSubReq->maxNumProcessors) ? jobSubReq->maxNumProcessors : 1;
		}
	else  {
		submitReq->numProcessors = DEFAULT_NUMPRO;
		submitReq->maxNumProcessors = DEFAULT_NUMPRO;
	}

	if (jobSubReq->options & SUB_LOGIN_SHELL) {
		if (!jobSubReq->loginShell) {
			lsberrno = LSBE_BAD_ARG;
			return -1;
		}
		if (strlen (jobSubReq->loginShell) >= MAX_LSB_NAME_LEN - 1) {
			lsberrno = LSBE_BAD_ARG;
			errno = ENAMETOOLONG;
			return -1;
		}
		submitReq->loginShell = jobSubReq->loginShell;
	}
	else {
		submitReq->loginShell = "";
	}

	submitReq->schedHostType = "";
	submitReq->restartPid = 0;
	submitReq->jobFile = "";

	if (jobSubReq->command == (char *) NULL)  {
		lsberrno = LSBE_BAD_CMD;
		return -1;
	}

	lineStrBuf[0] = '\0';
	jobdesp = lineStrBuf;
	strncat (lineStrBuf, jobSubReq->command, MAX_CMD_DESC_LEN - 1);

	if ((sp = strstr (jobdesp, "SCRIPT_\n")) != NULL) {

		jobSubReqCmd1Offset = sp + strlen ("SCRIPT_\n") - jobdesp;
		jobdesp += jobSubReqCmd1Offset;
		if ((sp = strstr (jobdesp, "SCRIPT_\n")) != NULL) {
			while (*sp != '\n') {
				--sp;
			}
			*sp = '\0';
		}
	}
	else {
		jobSubReqCmd1Offset = 0;
	}
	jobdespBuf[0] = '\0';
	strncat (jobdespBuf, jobdesp, MAX_CMD_DESC_LEN - 1);
	jobdesp = jobdespBuf;


	lastNonSpaceIdx = 0; lastNonSpaceChar = ';';
	for (unsigned int i = 0; jobdesp[i] != '\0'; i++)
	{
		if (jobdesp[i] == '\n')
		{
			if (lastNonSpaceChar != ';' && lastNonSpaceChar != '&') {
				jobdesp[i] = ';';
			}
			else {
				jobdesp[i] = ' ';
			}
		}
		if (jobdesp[i] != ' ' && jobdesp[i] != '\t' && isascii(jobdesp[i])) {
			lastNonSpaceChar = jobdesp[i];
			lastNonSpaceIdx = i;
		}
	}

	if (jobdesp[lastNonSpaceIdx] == ';') {
		jobdesp[lastNonSpaceIdx] = '\0';
	}

	for (unsigned int i = 0; jobSubReq->command[i] != '\0'; i++) {

		if (jobSubReq->command[i] == ';' || jobSubReq->command[i] == ' ' ||
			jobSubReq->command[i] == '&' || jobSubReq->command[i] == '>' ||
			jobSubReq->command[i] == '<' || jobSubReq->command[i] == '|' ||
			jobSubReq->command[i] == '\t' || jobSubReq->command[i] == '\n'
		   )
		{
			lsberrno = LSBE_BAD_CMD;
			break;
		}
	}

	if ((LSBE_BAD_CMD == lsberrno ) && (jobSubReq->command[0] != ' ')) {
		lsberrno = LSBE_BAD_CMD; // yes, reset.
		return -1;
	}

	strcpy (submitReq->command, jobdesp);

	if (jobSubReq->options2 & SUB2_MODIFY_CMD) {
		for (unsigned int i = 0; jobSubReq->newCommand[i] != '\0'; i++) {

			if (jobSubReq->newCommand[i] == ';' ||
				jobSubReq->newCommand[i] == ' ' ||
				jobSubReq->newCommand[i] == '&' ||
				jobSubReq->newCommand[i] == '>' ||
				jobSubReq->newCommand[i] == '<' ||
				jobSubReq->newCommand[i] == '|' ||
				jobSubReq->newCommand[i] == '\t' ||
				jobSubReq->newCommand[i] == '\n'
			   ) 
			{
				lsberrno = LSBE_BAD_CMD;
				break;
			}
		}
		if ((LSBE_BAD_CMD == lsberrno) && (jobSubReq->command[0] != ' '))
			{
			lsberrno = LSBE_BAD_CMD;
			return -1;
			}
		}


	if (jobSubReq->options2 & SUB2_JOB_CMD_SPOOL) {

		const LSB_SPOOL_INFO_T *spoolInfo;
		const char *pSpoolCmd;
		const char *pSrcCmd;
		unsigned long spoolCmdLen = 0;
		u_long *srcCmdLen   = 0;


		if (jobSubReq->options & SUB_MODIFY) {
			strcpy (submitReq->command, jobSubReq->newCommand);
			pSrcCmd = getCmdPathName_ (jobSubReq->newCommand, srcCmdLen);
		}
		else {
			pSrcCmd = getCmdPathName_ (&jobSubReq->command[jobSubReqCmd1Offset], srcCmdLen);
		}

		assert( sizeof (lineStrBuf) <= LONG_MAX );
		if ( *srcCmdLen >= (long)sizeof (lineStrBuf)) {
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
		}

		memcpy (lineStrBuf, pSrcCmd, *srcCmdLen);
		lineStrBuf[*srcCmdLen] = 0;

		spoolInfo = chUserCopySpoolFile (lineStrBuf, SPOOL_COMMAND);
		if (spoolInfo == NULL) {
			return -1;
		}

		pSpoolCmd = spoolInfo->spoolFile;
		spoolCmdLen = strlen (pSpoolCmd);
		if (spoolCmdLen >= MAX_FILENAME_LEN) {
			lsberrno = LSBE_SYS_CALL;
			errno = ENAMETOOLONG;
			return -1;
		}
		memcpy (subSpoolFiles->commandSpool, pSpoolCmd, spoolCmdLen + 1);
		submitReq->commandSpool = subSpoolFiles->commandSpool;

		if (!(jobSubReq->options & SUB_MODIFY)) {

			unsigned long counter_ = 0;
			char *pChangeSrcCmd = (char *) pSrcCmd;

			assert( sizeof (*srcCmdLen) >= 0 );
			if (spoolCmdLen <= (unsigned long)*srcCmdLen) {

				memcpy (pChangeSrcCmd, pSpoolCmd, spoolCmdLen);
				assert( *srcCmdLen >= 0 );
				for ( unsigned long ii = spoolCmdLen, jj = (unsigned long)*srcCmdLen; pChangeSrcCmd[jj]; ii++, jj++) {
					pChangeSrcCmd[ii] = pChangeSrcCmd[jj];
					counter_ = ii;
				}
				pChangeSrcCmd[counter_] = 0;
			}
			else {

				unsigned long oldLen = 0;
				unsigned long newLen = 0;
				char *sp_  = NULL;

				sp_ = jobSubReq->command;
				oldLen = strlen (sp_);
				assert( *srcCmdLen >= 0 );
				newLen = oldLen + spoolCmdLen - (unsigned long)*srcCmdLen;
				sp_ = realloc (sp_, newLen);
				if ( NULL == sp_ && ENOMEM == errno ) {
					lsberrno = LSBE_NO_MEM;
					return -1;
				}
				jobSubReq->command = sp_;

				pChangeSrcCmd = (char *)
				getCmdPathName_ (&jobSubReq->command[jobSubReqCmd1Offset], srcCmdLen);

				assert( *srcCmdLen >= 0);
				for ( unsigned long ii = newLen, jj = oldLen; jj >= (unsigned long)*srcCmdLen; ii--, jj--) {
					pChangeSrcCmd[ii] = pChangeSrcCmd[jj];
				}
				memcpy (pChangeSrcCmd, pSpoolCmd, spoolCmdLen);
			}
		}
	}
	else if (jobSubReq->options2 & SUB2_MODIFY_CMD) {
		strcpy (submitReq->command, jobSubReq->newCommand);
		submitReq->commandSpool = "";
	}
	else {
		submitReq->commandSpool = "";
	}

	lineStrBuf[0] = '\0';
	strncat (lineStrBuf, jobSubReq->command, MIN (counter, MAX_LINE_LEN));

	if ((myHostName = ls_getmyhostname ()) == NULL) {
		lsberrno = LSBE_LSLIB;
		return -1;
	}
	if (jobSubReq->resReq != NULL && jobSubReq->options & SUB_RES_REQ) {

		if (strlen (jobSubReq->resReq) > MAX_LINE_LEN - 1) {
			lsberrno = LSBE_BAD_RESREQ;
			return -1;
		}
		strcpy (submitReq->resReq, jobSubReq->resReq);
	}
	else {
		ls_eligible (lineStrBuf, submitReq->resReq, LSF_REMOTE_MODE);
	}

	submitReq->fromHost = myHostName;
	umask (submitReq->umask = umask (0077));

	if ((jobSubReq->options & SUB_OTHER_FILES) && jobSubReq->nxf > 0) {
		submitReq->nxf = jobSubReq->nxf;
		submitReq->xf = jobSubReq->xf;
	}
	else {
		submitReq->nxf = 0;
	}

	if ((pw = getpwdirlsfuser_ (auth->lsfUserName)) == NULL) {
		lsberrno = LSBE_SYS_CALL;
		return -1;
	}

	if (strlen (pw->pw_dir) >= MAX_FILENAME_LEN - 1) {
		lsberrno = LSBE_SYS_CALL;
		errno = ENAMETOOLONG;
		return -1;
	}

	for( int i = 0; pw->pw_dir[i] != '\0' && submitReq->cwd[i] == pw->pw_dir[i]; i++) {
		counter = i;
	}

	if ((counter != 0) && (pw->pw_dir[counter] == '\0')) {
		if (submitReq->cwd[counter] == '\0') {
			submitReq->cwd[0] = '\0';
		}
		else if (submitReq->cwd[counter] == '/') {
			strcpy (submitReq->cwd, submitReq->cwd + counter + 1);
		}
	}

	strcpy (submitReq->subHomeDir, pw->pw_dir);

	submitReq->sigValue = (jobSubReq->options & SUB_WINDOW_SIG) ? sig_encode (jobSubReq->sigValue) : 0;
	if (submitReq->sigValue > 31 || submitReq->sigValue < 0) {
		lsberrno = LSBE_BAD_SIGNAL;
		return -1;
	}

	return 0;

}


char *
acctMapGet (int *fail, char *lsfUserName)
{

	char hostfn[MAX_FILENAME_LEN] = "a";
	char clusorhost[MAX_LSB_NAME_LEN] = "a";
	char user[MAX_LSB_NAME_LEN] = "a";
	char dir[40] = "a";
	char *line = NULL;
	char *map  = NULL; 
	unsigned long maplen = 0;
	unsigned long len    = 0;
	int num    = 0;
	struct passwd *pw;
	struct stat statbuf;
	FILE *fp;

	if ((pw = getpwdirlsfuser_ (lsfUserName)) == NULL) {
		return NULL;
	}

	strcpy (hostfn, pw->pw_dir);
	strcat (hostfn, "/.lsfhosts");

	if ((fp = fopen (hostfn, "r")) == NULL) {
		return NULL;
	}

	if ((fstat (fileno (fp), &statbuf) < 0) || (statbuf.st_uid != 0 && statbuf.st_uid != pw->pw_uid) || (statbuf.st_mode & 066)) {
		fclose (fp);
		return NULL;
	}

	maplen = 256;
	map = malloc (maplen);
	if ( NULL == map && ENOMEM == errno ) {
		lsberrno = LSBE_NO_MEM;
		*fail = 1;
		return NULL;
	}
	map[0] = '\0';
	len = 0;
	while ((line = getNextLine_ (fp, TRUE)) != NULL) {
		num = sscanf (line, "%s %s %s", clusorhost, user, dir);
		if (num < 2 || (num == 3 && (strcmp (dir, "recv") == 0 && strcmp (dir, "send") != 0))) {
			continue;
		}

		if (strcmp (user, "+") == 0) {
			continue;
		}

		len += strlen (clusorhost) + 1 + strlen (user) + 1;
		if (len > maplen)
			{
			char *newmap;

			maplen += 256;
			if ((newmap = realloc (map, maplen)) == NULL)
				{
				lsberrno = LSBE_NO_MEM;
				free (map);
				*fail = 1;
				return NULL;
				}
			map = newmap;
			}
		strcat (map, clusorhost);
		strcat (map, " ");
		strcat (map, user);
		strcat (map, " ");
		}
	return map;

}

int
getUserInfo (struct submitReq *submitReq, struct submit *jobSubReq)
{
	char lsfUserName[MAX_LINE_LEN];		// FIXME FIXME malloc
	int childIoFd[2];
	uid_t uid  = 0;
	pid_t pid  = 0;
	int fail   = 0;
	char *usermap        = NULL;
	unsigned long len    = 0;
	unsigned long cwdLen = 0;
	
	struct {
		int error;
		int eno;
		int lserrno;
		int lsberrno;
	} err;

	if (getLSFUser_ (lsfUserName, MAX_LINE_LEN) != 0) {
		return -1;
	}

	uid = getuid ();
	if (socketpair (AF_UNIX, SOCK_STREAM, 0, childIoFd) < 0) {
		lsberrno = LSBE_SYS_CALL;
		return -1;
	}

	if ((pid = fork ()) < 0) {
		lsberrno = LSBE_SYS_CALL;
		return -1;
	}
	else if (pid > 0) {
		LS_WAIT_T status;
		close (childIoFd[1]);

		err.eno = -1;
		if (read (childIoFd[0], (char *) &err, sizeof (err)) != sizeof (err)) {
			lsberrno = LSBE_SYS_CALL;
			err.error = TRUE;
			goto waitforchild;
		}

		if (err.error) {
			errno = err.eno;
			lserrno = err.lserrno;
			lsberrno = err.lsberrno;
			goto waitforchild;
		}
		err.eno = -1;

		if (read (childIoFd[0], (char *) &cwdLen, sizeof (cwdLen)) != sizeof (cwdLen)) {
			err.error = TRUE;
			lsberrno = LSBE_SYS_CALL;
			goto waitforchild;
		}
		assert( cwdLen <= LONG_MAX );
		if (read (childIoFd[0], (char *) submitReq->cwd, cwdLen) != (long)cwdLen)
			{
			err.error = TRUE;
			lsberrno = LSBE_SYS_CALL;
			goto waitforchild;
			}


		err.eno = -1;
		if (read (childIoFd[0], (char *) &err, sizeof (err)) != sizeof (err))
			{

			lsberrno = LSBE_SYS_CALL;
			err.error = TRUE;
			goto waitforchild;
			}

		if (err.error)
			{
			errno = err.eno;
			lserrno = err.lserrno;
			lsberrno = err.lsberrno;
			goto waitforchild;
			}
		err.eno = -1;


		if (read (childIoFd[0], (char *) &ed.len, sizeof (ed.len)) !=
			sizeof (ed.len))
			{
			err.error = TRUE;
			lsberrno = LSBE_SYS_CALL;
			goto waitforchild;
			}

		FREEUP (ed.data);
		if (ed.len > 0) {
			ed.data = (char *) malloc (ed.len);
			if (NULL == ed.data && ENOMEM == errno ) {
				err.error = TRUE;
				lsberrno = LSBE_NO_MEM;
				goto waitforchild;
			}
			assert( ed.len <= LONG_MAX);
			if (b_read_fix (childIoFd[0], ed.data, ed.len) != (long)ed.len) {
				FREEUP (ed.data);
				err.error = TRUE;
				lsberrno = LSBE_SYS_CALL;
				goto waitforchild;
			}
		}
		else {
			ed.data = NULL;
		}

		err.eno = -1;
		if (read (childIoFd[0], (char *) &err, sizeof (err)) != sizeof (err)) {

			lsberrno = LSBE_SYS_CALL;
			err.error = TRUE;
			goto waitforchild;
		}

		if (err.error) {
			errno = err.eno;
			lserrno = err.lserrno;
			lsberrno = err.lsberrno;
			goto waitforchild;
		}
		err.eno = -1;

		if (read (childIoFd[0], (char *) &len, sizeof (len)) != sizeof (len))
			{
			err.error = TRUE;
			lsberrno = LSBE_SYS_CALL;
			goto waitforchild;
			}
		if (len)
			{
			FREEUP (useracctmap);
			if ((useracctmap = malloc (len)) == NULL)
				{
				lsberrno = LSBE_NO_MEM;
				err.error = TRUE;
				goto waitforchild;
				}
			assert( len <= LONG_MAX);
			if (read (childIoFd[0], useracctmap, len) != (long)len)
				{
				err.error = TRUE;
				lsberrno = LSBE_SYS_CALL;
				goto waitforchild;
				}
			}

		waitforchild:
		close (childIoFd[0]);

		if (err.eno < 0)
			err.eno = errno;
		if (waitpid (pid, &status, 0) < 0)
			{
			lsberrno = LSBE_SYS_CALL;
			return -1;
			}

		if (err.error)
			{
			errno = err.eno;
			return -1;
			}

		return 0;
		}



	close (childIoFd[0]);

	if (setuid (uid) < 0)
		{
		lsberrno = LSBE_BAD_USER;
		goto errorParent;
		}


	if (mygetwd_ (submitReq->cwd) == NULL)
		{
		lsberrno = LSBE_SYS_CALL;
		goto errorParent;
		}
	cwdLen = strlen (submitReq->cwd) + 1;

	err.error = FALSE;
	if (write (childIoFd[1], (char *) &err, sizeof (err)) != sizeof (err))
		{
		close (childIoFd[1]);
		exit (-1);
		}


	if (write (childIoFd[1], (char *) &cwdLen, sizeof (cwdLen))
		!= sizeof (cwdLen))
		{
		close (childIoFd[1]);
		exit (-1);
		}
	assert( cwdLen <= LONG_MAX );
	if (write (childIoFd[1], (char *) submitReq->cwd, cwdLen) != (long)cwdLen)
		{
		close (childIoFd[1]);
		exit (-1);
		}

	if (runBatchEsub (&ed, jobSubReq) < 0)
		{
		goto errorParent;
		}
	else
		{
		err.error = FALSE;
		if (write (childIoFd[1], (char *) &err, sizeof (err)) != sizeof (err))
			{
			close (childIoFd[1]);
			exit (-1);
			}
		}


	if (write (childIoFd[1], (char *) &ed.len, sizeof (ed.len))
		!= sizeof (ed.len))
		{
		close (childIoFd[1]);
		exit (-1);
		}

	if (ed.len > 0)
		{
		assert( ed.len <= LONG_MAX );
		if (write (childIoFd[1], (char *) ed.data, ed.len) != (long)ed.len)
			{
			close (childIoFd[1]);
			exit (-1);
			}
		}


	fail = 0;

	if ((usermap = acctMapGet (&fail, lsfUserName)) == NULL) {
		len = 0;
	}
	else {
		len = strlen (usermap) + 1;
	}

	if (fail)
		goto errorParent;
	else
		{
		err.error = FALSE;
		if (write (childIoFd[1], (char *) &err, sizeof (err)) != sizeof (err))
			{
			close (childIoFd[1]);
			exit (-1);
			}
		}

	if (write (childIoFd[1], (char *) &len, sizeof (len)) != sizeof (len))
		{
		close (childIoFd[1]);
		exit (-1);
		}
	assert( len <= LONG_MAX );
	if (write (childIoFd[1], (char *) usermap, len) != (long)len)
		{
		close (childIoFd[1]);
		exit (-1);
		}
	exit (0);

errorParent:

	err.error = TRUE;
	err.eno = errno;
	err.lserrno = lserrno;
	err.lsberrno = lsberrno;


	if (write (childIoFd[1], (char *) &err, sizeof (err)) != sizeof (err)) {
		exit (-1);
	}

	exit (0);

}


int
xdrSubReqSize (struct submitReq *req)
{
	int sz;

	sz = 1024 + ALIGNWORD_ (sizeof (struct submitReq)); // FIXME FIXME FIXME FIXME that 1024 is .. suspect

	//
	sz += ALIGNWORD_ (strlen (req->queue) + 1) + 4 +
	ALIGNWORD_ (strlen (req->resReq)) + 4 +
	ALIGNWORD_ (strlen (req->fromHost) + 1) + 4 +
	ALIGNWORD_ (strlen (req->dependCond) + 1) + 4 +
	ALIGNWORD_ (strlen (req->jobName) + 1) + 4 +
	ALIGNWORD_ (strlen (req->command) + 1) + 4 +
	ALIGNWORD_ (strlen (req->jobFile) + 1) + 4 +
	ALIGNWORD_ (strlen (req->inFile) + 1) + 4 +
	ALIGNWORD_ (strlen (req->outFile) + 1) + 4 +
	ALIGNWORD_ (strlen (req->errFile) + 1) + 4 +
	ALIGNWORD_ (strlen (req->inFileSpool) + 1) + 4 +
	ALIGNWORD_ (strlen (req->commandSpool) + 1) + 4 +
	ALIGNWORD_ (strlen (req->preExecCmd) + 1) + 4 +
	ALIGNWORD_ (strlen (req->hostSpec) + 1) + 4 +
	ALIGNWORD_ (strlen (req->chkpntDir) + 1) + 4 +
	ALIGNWORD_ (strlen (req->subHomeDir) + 1) + 4 +
	ALIGNWORD_ (strlen (req->cwd) + 1) + 4 +
	ALIGNWORD_ (strlen (req->mailUser) + 1) + 4 +
	ALIGNWORD_ (strlen (req->projectName) + 1) + 4;

	for (unsigned int i = 0; i < req->numAskedHosts; i++) {
		sz += ALIGNWORD_ (strlen (req->askedHosts[i]) + 1 + 4);
	}

	for ( unsigned int i = 0; i < req->nxf; i++) {
		sz += ALIGNWORD_ (sizeof (struct xFile) + 4 * 4);
	}

	return sz;
}


int
createNiosSock (struct submitReq *submitReq)
{
	int asock = 0;
	socklen_t len;
	struct sockaddr_in sin;

	if ((asock = TcpCreate_ (TRUE, 0)) < 0)
		{
		lsberrno = LSBE_LSLIB;
		return -1;
		}

	len = sizeof (sin);
	if (getsockname (asock, (struct sockaddr *) &sin, &len) < 0) {
		close (asock);
		lsberrno = LSBE_LSLIB;
		lserrno = LSE_SOCK_SYS;
		return -1;
	}

	submitReq->niosPort = ntohs (sin.sin_port);
	return asock;
}


void
startNios (struct submitReq *submitReq, int asock, LS_LONG_INT jobId)
{
	char sockStr[10];
	char envStr[64];
	struct config_param niosParams[] = {
		{"LSF_SERVERDIR", NULL},
		{NULL, NULL}
	};

	setuid (getuid ());
	if (initenv_ (niosParams, NULL) < 0)
		{
		ls_perror ("initenv nios");
		exit (-1);
		}

	if (submitReq->options2 & SUB2_BSUB_BLOCK)
		{
		sprintf (envStr, "%s", lsb_jobidinstr (jobId));
		putEnv ("BSUB_BLOCK", envStr);
		}

	sprintf (envStr, "%s", lsb_jobidinstr (jobId));
	putEnv ("LSB_JOBID", envStr);

	sprintf (envStr, "bkill %s", lsb_jobidinstr (jobId));
	putEnv ("LSF_NIOS_DIE_CMD", envStr);

	sprintf (sockStr, "%d", asock);

	sprintf (niosPath, "%s/nios", niosParams[LSF_NIOSDIR].paramValue);
	niosArgv[0] = niosPath;
	niosArgv[1] = "-N";
	niosArgv[2] = sockStr;
	if (submitReq->options & SUB_PTY)
		{
		if (submitReq->options & SUB_PTY_SHELL) {
			niosArgv[3] = "2";
		}        
		else {
			niosArgv[3] = "1";
		}
		}
	else
		{
		niosArgv[3] = "0";
		}

	niosArgv[4] = NULL;

	execv (niosArgv[0], niosArgv);
	perror (niosArgv[0]);
	exit (-1);
}


void
postSubMsg (struct submit *req, LS_LONG_INT jobId, struct submitReply *reply)
{


	if ((req->options & SUB_QUEUE) || (req->options & SUB_RESTART))
		{
		if (getenv ("BSUB_STDERR")) {
			/* catgets 400 */
			fprintf (stderr, "catgets 400: Job <%s> is submitted to queue <%s>.\n", lsb_jobid2str (jobId), reply->queue);
		}
		else {
			/* catgets 400 */
			fprintf (stdout, "catgets 400: Job <%s> is submitted to queue <%s>.\n", lsb_jobid2str (jobId), reply->queue);
		}
	}
	else
		{
		if (getenv ("BSUB_STDERR")) {
			/* catgets 401 */
			fprintf (stderr, "catgets 401: Job <%s> is submitted to default queue <%s>.\n", lsb_jobid2str (jobId), reply->queue);
		}
		else {
			/* catgets 401 */
			fprintf (stdout, "catgets 401: Job <%s> is submitted to default queue <%s>.\n", lsb_jobid2str (jobId), reply->queue);
		}
	}

	fflush (stdout);

	prtBETime_ (req);

	if ( ( req->options2 & SUB2_BSUB_BLOCK) || (req->options & SUB_INTERACTIVE) ) {
		/* catgets 402 */
		fprintf (stderr, "catgets 402: <<Waiting for dispatch ...>>\n");
	}
}


void
prtBETime_ (struct submit *req)
{
	char *sp = NULL;

	if (logclass & (LC_TRACE | LC_EXEC | LC_SCHED)) {
		ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);
	}

	if (req->beginTime > 0)
		{
		sp = _i18n_ctime (ls_catd, CTIME_FORMAT_a_b_d_T_Y, &req->beginTime);
		/* catgets 404 */
		fprintf (stderr, "catgets 404: Job will be scheduled after %s\n", sp);
		}
	if (req->termTime > 0)
		{
		sp = _i18n_ctime (ls_catd, CTIME_FORMAT_a_b_d_T_Y, &req->termTime);
		/* catgets 405 */
		fprintf (stderr, "catgets 405: Job will be terminated by %s\n", sp);
	}

	return;
}



int
gettimefor (char *toptarg, time_t * tTime)
{
	struct tm *tmPtr;
	char *cp;
	int tindex, ttime[5];
	int currhour, currmin, currday;

	// TIMEIT (1, 
	*tTime = time (0);
	//, "time");
	// TIMEIT (1, 
	tmPtr = localtime (tTime);
	//, "localtime");
	tmPtr->tm_sec = 0;
	currhour = tmPtr->tm_hour;
	currmin = tmPtr->tm_min;
	currday = tmPtr->tm_mday;

	for (tindex = 0; toptarg; tindex++) {
		ttime[tindex] = 0;
		cp = strrchr (toptarg, ':');
		if (cp != NULL)
			{
			if (!isint_ (cp + 1)) {
				return -1;
			}
			ttime[tindex] = atoi (cp + 1);
			*cp = '\000';
			}
		else
			{
			if (!isint_ (toptarg)) {
				return -1;
			}
			ttime[tindex] = atoi (toptarg);
			tindex++;
			break;
			}
		}
	if (tindex < 2 || tindex > 4)
		{
		return -1;
		}
	if (ttime[0] < 0 || ttime[0] > 59)
		{
		return -1;
		}
	tmPtr->tm_min = ttime[0];

	if (ttime[1] < 0 || ttime[1] > 23)
		{
		return -1;
		}
	tmPtr->tm_hour = ttime[1];

	tindex -= 2;
	if (tindex > 0)
		{
		if (ttime[2] < 1 || ttime[2] > 31)
			{
			return -1;
			}
		if (((ttime[2] < tmPtr->tm_mday) ||
			 ((ttime[2] == tmPtr->tm_mday) && (ttime[1] < currhour)) ||
			 ((ttime[2] == tmPtr->tm_mday) && (ttime[1] == currhour) &&
			  (ttime[0] < currmin))) && tindex == 1)
			tmPtr->tm_mon++;
		tmPtr->tm_mday = ttime[2];

		tindex--;
		switch (tindex)
			{
				case 1:
				if (ttime[3] < 0 || ttime[3] > 12)
					{
					return -1;
					}
				if ((((ttime[3] - 1) < tmPtr->tm_mon) ||
					 (((ttime[3] - 1) == tmPtr->tm_mon) && (ttime[2] < currday)) ||
					 (((ttime[3] - 1) == tmPtr->tm_mon) && (ttime[2] == currday) &&
					  (ttime[1] < currhour)) ||
					 (((ttime[3] - 1) == tmPtr->tm_mon) && (ttime[2] == currday) &&
					  (ttime[1] == currhour) && (ttime[0] < currmin))))
					tmPtr->tm_year++;
				tmPtr->tm_mon = ttime[3] - 1;
				break;
				default:
				break;
			}
		}


	tmPtr->tm_isdst = -1;


	*tTime = *tTime < mktime (tmPtr) ? mktime (tmPtr) : mktime (tmPtr) + 86400;

	return 0;
}

int checkSubDelOption( struct submit *req, int option ) {
	if (req->options & SUB_MODIFY ) {
		if (req->options & option) { 
			 /* catgets 406 */
			fprintf(stderr, "catgets 406: You cannot modify and set default at the same time");  
			return -1;
		}
		req->delOptions |= option;
		// break;
	}
	return 0;
}

int checkSubDelOption2( struct submit *req, int option2 ) { 
	if (req->options & SUB_MODIFY ) {
		if (req->options2 & option2) { 
			/* catgets 406 */ 
			fprintf(stderr, "catgets 406: You cannot modify and set default at the same time"); 
			return -1;
		}
		req->delOptions2 |= option2;
		// break;
	}
	return 0;
}

int checkRLDelOption( struct submit *req, int rLimit ) 
{ 
	if (req->options & SUB_MODIFY ) { 
		if (req->rLimits[rLimit] != DEFAULT_RLIMIT && req->rLimits[rLimit] != DELETE_NUMBER) { 
			/* catgets 406 */
			fprintf(stderr, "catgets 406: You cannot modify and set default at the same  time"); 
			return -1;
		}
		req->rLimits[rLimit] = DELETE_NUMBER;
		// break;
	}
	return 0;
}


int
setOption_ (int argc, char **argv, char *template, struct submit *req, int mask, int mask2, char **errMsg)
{
	int eflag = 0;
	int oflag = 0;
	int badIdx = 0;
	int flagI = 0;
	int flagK = 0;
	unsigned int v1  = 0;
	unsigned int v2  = 0;;
	char *sp = NULL;
	char *cp = NULL;
	char *optName;
	char *pExclStr;
	char *(*getoptfunc) ();
	char savearg[MAX_LINE_LEN];

	struct args {
		int argc;
		char padding[4];
		char **argv;
	} myArgs;

	myArgs.argc = req->options;
	myArgs.argv = errMsg;

	getoptfunc = my_getopt;

	while ((optName = getoptfunc (argc, argv, template, errMsg)) != NULL)
		{
		switch (optName[0])
			{
				case 'E':
				{
					req->options2 |= SUB2_MODIFY_PEND_JOB;
					if( !checkSubDelOption( req, SUB_PRE_EXEC ) ){
						break;
					}
					if (mask & SUB_PRE_EXEC) {
						req->options |= SUB_PRE_EXEC;
						req->preExecCmd = optarg;
					}
				}
				break;

				case 'w':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				if( checkSubDelOption ( req, SUB_DEPEND_COND ) ){
					;
				}
				if (mask & SUB_DEPEND_COND)
					{
					req->options |= SUB_DEPEND_COND;
					req->dependCond = optarg;
					}
				break;

				case 'L':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption ( req, SUB_LOGIN_SHELL );
				if (mask & SUB_LOGIN_SHELL)
					{
					req->options |= SUB_LOGIN_SHELL;
					req->loginShell = optarg;
					}
				break;

				case 'B':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption ( req, SUB_NOTIFY_BEGIN );
				req->options |= SUB_NOTIFY_BEGIN;
				break;

				case 'f':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption ( req, SUB_OTHER_FILES );
				if (req->options & SUB_RESTART)
					{
					req->options |= SUB_RESTART_FORCE;
					break;
					}


				if (mask & SUB_OTHER_FILES)
					{
					if (parseXF (req, optarg, errMsg) < 0) {
						return -1;
					}
					req->options |= SUB_OTHER_FILES;
					}
				break;

				case 'k':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption ( req, (SUB_CHKPNT_PERIOD | SUB_CHKPNT_DIR));
				if (!(mask & SUB_CHKPNT_DIR)) {
					break;
				}

				cp = optarg;
				while (*(cp++) == ' ');

				if ((sp = strchr (cp, ' ')) != NULL)
					{


					int bPeriodExist = 0;
					int bMethodExist = 0;
					char *pCurWord = NULL;


					*sp = '\0';


					while (*(++sp) == ' ') {
						;
					}


					while ((sp != NULL) && (*sp != '\0'))
						{
						pCurWord = sp;

						if (isdigit ((int) (*pCurWord)))
							{

							if (bPeriodExist)
								{
								/* catgets 408 */
								PRINT_ERRMSG1 (errMsg, "catgets 408: %s: Bad checkpoint period value", pCurWord);
								return -1;
								}


							sp = strchr (pCurWord, ' ');
							if (sp != NULL) {
								*sp = '\0';
							}

							if (!isint_ (pCurWord) || (req->chkpntPeriod = atoi (pCurWord) * 60) <= 0)
								{
								/* catgets 408 */
								PRINT_ERRMSG1 (errMsg, "catgets 408: %s: Bad checkpoint period value", pCurWord);
								return -1;
								}
							bPeriodExist = 1;
							req->options |= SUB_CHKPNT_PERIOD;

							}
						else if (strstr (pCurWord, "method=") == pCurWord)
							{


							if (bMethodExist)
								{
								/* catgets 445 */
								PRINT_ERRMSG1 (errMsg, "catgets 445: %s: Syntax error. Correct syntax is method=name of your checkpoint method", pCurWord);
								return -1;
								}

							sp = strchr (pCurWord, ' ');
							if (sp != NULL)
								{
								*sp = '\0';
								}

							pCurWord = strchr (pCurWord, '=');
							if (*(++pCurWord) != '\0')
								{
								putEnv ("LSB_ECHKPNT_METHOD", pCurWord);
								}
							else
								{
								/* catgets 445 */
								PRINT_ERRMSG1 (errMsg, "catgets 445: %s: Syntax error. Correct syntax is method=name of your checkpoint method", pCurWord);
								return -1;
								}
							bMethodExist = 1;
							}
						else
							{
							/* catgets 446 */
							PRINT_ERRMSG1 (errMsg, "catgets 446: %s: Syntax error. Correct syntax is bsub -k \"chkpnt_dir [period] [method=checkpoint method]\"", pCurWord);
							return -1;
							}

						if (sp != NULL)
							{
							while (*(++sp) == ' ');
							}
						}
					}
				req->chkpntDir = optarg;
				req->options |= SUB_CHKPNT_DIR;
				break;

				case 'R':

				checkSubDelOption( req, SUB_RES_REQ );
				if (mask & SUB_RES_REQ)
					{
					if (req->resReq != NULL)
						{
						/* catgets 442 */
						PRINT_ERRMSG0 (errMsg, "catgets 442: Invalid syntax; the -R option was used more than once.\n");
						return -1;
						}
					req->resReq = optarg;
					req->options |= SUB_RES_REQ;
					while (*(req->resReq) == ' ')
						req->resReq++;
					}
				break;

				case 'x':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption( req, SUB_EXCLUSIVE );
				req->options |= SUB_EXCLUSIVE;
				break;

				case 'I':
				if (flagI || flagK)
					{
					myArgs.argc = req->options;
					lsb_throw ("LSB_BAD_BSUBARGS", &myArgs);

					return -1;
					}

				if (!strcmp (optName, "I"))
					{
					req->options |= SUB_INTERACTIVE;
					flagI++;
					}
				else if (!strcmp (optName, "Ip"))
					{
					req->options |= SUB_INTERACTIVE | SUB_PTY;
					flagI++;
					}
				else if (!strcmp (optName, "Is"))
					{
					req->options |= SUB_INTERACTIVE | SUB_PTY | SUB_PTY_SHELL;
					flagI++;
					}
				else
					{
					myArgs.argc = req->options;
					lsb_throw ("LSB_BAD_BSUBARGS", &myArgs);

					return -1;
					}
				break;

				case 'H':
				checkSubDelOption( req, SUB2_HOLD );
				req->options2 |= SUB2_HOLD;
				break;
				case 'K':
				if (flagI || flagK)
					{
					myArgs.argc = req->options;
					lsb_throw ("LSB_BAD_BSUBARGS", &myArgs);

					return -1;
					}
				flagK++;
				req->options2 |= SUB2_BSUB_BLOCK;
				break;
				case 'r':
				req->options2 |= SUB2_MODIFY_RUN_JOB;
				checkSubDelOption( req, SUB_RERUNNABLE );

				req->options |= SUB_RERUNNABLE;
				break;

				case 'N':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption( req, SUB_NOTIFY_END );
				req->options |= SUB_NOTIFY_END;
				break;

				case 'h':
				myArgs.argc = req->options;
				lsb_throw ("LSB_BAD_BSUBARGS", &myArgs);

				return -1;

				case 'm':
				{
				unsigned int numAskedHosts = 0;
				unsigned long *foo = 0;
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption( req, SUB_HOST );
				if (!(mask & SUB_HOST)) {
					break;
				}

				req->options |= SUB_HOST;
				numAskedHosts = (unsigned int)req->numAskedHosts;
				assert( badIdx >= 0 );
				*foo = (unsigned long) badIdx;
				if (getAskedHosts_ (optarg, &req->askedHosts, &numAskedHosts, foo, FALSE) < 0 && lserrno != LSE_BAD_HOST)
					{
					lsberrno = LSBE_LSLIB;
					PRINT_ERRMSG0 (errMsg, ls_sysmsg ());
					return -1;
					}
				if (req->numAskedHosts == 0)
					{
					myArgs.argc = req->options;
					lsb_throw ("LSB_BAD_BSUBARGS", &myArgs);

					return -1;
					}
				}
				break;

				case 'J':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption( req, SUB_JOB_NAME );
				if (mask & SUB_JOB_NAME)
					{
					req->jobName = optarg;
					req->options |= SUB_JOB_NAME;
					req->options2 |= SUB2_MODIFY_PEND_JOB;
					}
				break;

				case 'i':

				pExclStr = "isn|is|in|i";
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				if (strncmp (optName, "is", 2) == 0)
					{



					if ((req->options & SUB_IN_FILE)
						|| (req->delOptions & SUB_IN_FILE))
						{
						/* catgets  444 */
						PRINT_ERRMSG1 (errMsg, "catgets 444: %s options are exclusive", pExclStr);
						return -1;
						}
					checkSubDelOption2( req, SUB2_IN_FILE_SPOOL );
					if (!(mask2 & SUB2_IN_FILE_SPOOL)) {
						break;
					}

					if (strlen (optarg) > MAX_FILENAME_LEN - 1)
						{
						/* catgets 409 */
						PRINT_ERRMSG1 (errMsg, "catgets 409: %s: File name too long", optarg);
						return -1;
						}
					req->inFile = optarg;
					req->options2 |= SUB2_IN_FILE_SPOOL;
					}
				else
					{



					if ((req->options2 & SUB2_IN_FILE_SPOOL)
						|| (req->delOptions2 & SUB2_IN_FILE_SPOOL))
						{
						/* catgets  444 */
						PRINT_ERRMSG1 (errMsg, "catgets 444: %s options are exclusive", pExclStr);
						return -1;
						}

					checkSubDelOption( req, SUB_IN_FILE );
					if (!(mask & SUB_IN_FILE)) {
						break;
					}

					if (strlen (optarg) > MAX_FILENAME_LEN - 1)
						{
						/* catgets 409 */
						PRINT_ERRMSG1 (errMsg, "catgets 409: %s: File name too long", optarg);
						return -1;
						}
					req->inFile = optarg;
					req->options |= SUB_IN_FILE;
					}
				break;
				case 'o':
				req->options2 |= SUB2_MODIFY_RUN_JOB;
				checkSubDelOption( req, SUB_OUT_FILE );

				if (!(mask & SUB_OUT_FILE)) {
					break;
				}

				if (strlen (optarg) > MAX_FILENAME_LEN - 1)
					{
					/* catgets 410 */
					PRINT_ERRMSG1 (errMsg, "catgets 410: %s: File name too long", optarg);
					return -1;
					}
				req->outFile = optarg;
				req->options |= SUB_OUT_FILE;
				oflag = 1;
				if (eflag)
					{
					if (strcmp (req->outFile, req->errFile) == 0)
						{
						req->options &= ~SUB_ERR_FILE;
						req->errFile = "";
						}
					}
				break;

				case 'u':

				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption( req, SUB_MAIL_USER );
				if ((mask & SUB_MAIL_USER))
					{
					if (strlen (optarg) > MAXHOSTNAMELEN - 1)
						{
						/* catgets 411 */
						PRINT_ERRMSG1 (errMsg, "catgets 411: %s: Mail destination name too long", optarg);
						return -1;
						}

					req->mailUser = optarg;
					req->options |= SUB_MAIL_USER;
					}
				break;

				case 'e':
				req->options2 |= SUB2_MODIFY_RUN_JOB;
				checkSubDelOption( req, SUB_ERR_FILE);

				if (!(mask & SUB_ERR_FILE)) {
					break;
				}

				if (strlen (optarg) > MAX_FILENAME_LEN - 1)
					{
					/* catgets 412 */
					PRINT_ERRMSG1 (errMsg, "catgets 412: %s: File name too long", optarg);
					return -1;
					}
				req->errFile = optarg;
				req->options |= SUB_ERR_FILE;
				eflag = 1;
				if (oflag)
					{
					if (strcmp (req->outFile, req->errFile) == 0)
						{
						req->options &= ~SUB_ERR_FILE;
						req->errFile = "";
						}
					}
				break;

				case 'n':
				{
				int vtemp1 = 0;
				int vtemp2 = 0;
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				if (req->options & SUB_MODIFY && strcmp (optName, "nn") == 0)
					{
					if (req->numProcessors != 0)
						{
						/* catgets 413 */
						PRINT_ERRMSG0 (errMsg, "catgets 413: You cannot modify and set default at the same time");
						return -1;
						}
					req->numProcessors = DEL_NUMPRO;
					req->maxNumProcessors = DEL_NUMPRO;
					break;
					}
				if (req->numProcessors != 0) {
					break;
				}

				assert( v1 <= INT_MAX );
				assert( v2 <= INT_MAX );
				vtemp1 = (int) v1;
				vtemp2 = (int) v2; 
				if (getValPair (&optarg, &vtemp1, &vtemp2) < 0)
					{
					PRINT_ERRMSG0 (errMsg, "catgets 414: Bad argument for option -n");  /* catgets 414 */
					return -1;
					}
				if (v1 <= 0 || v2 <= 0)
					{
					/* catgets 415 */
					PRINT_ERRMSG0 (errMsg, "catgets 415: The number of processors must be a positive integer"); 
					return -1;
					}
				if (v1 != INFINIT_INT)
					{
					req->numProcessors = v1;
					req->maxNumProcessors = v1;
					}
				if (v2 != INFINIT_INT)
					{
					req->maxNumProcessors = v2;
					if (v1 == INFINIT_INT)
						req->numProcessors = 1;
					}
				if (req->numProcessors > req->maxNumProcessors)
					{
					/* catgets 416 */
					PRINT_ERRMSG0 (errMsg, "catgets 416: Bad argument for option -n");  
					return -1;
					}
				}
				break;

				case 'q':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption( req, SUB_QUEUE );
				if (mask & SUB_QUEUE)
					{
					req->options |= SUB_QUEUE;
					req->queue = optarg;
					}
				break;

				case 'b':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				if (req->options & SUB_MODIFY && strcmp (optName, "bn") == 0)
					{
					if (req->beginTime != 0 && req->beginTime != DELETE_NUMBER)
						{
						/* catgets 417 */
						PRINT_ERRMSG0 (errMsg, "catgets 417: You cannot modify and set default at the same time"); 
						return -1;
						}
					req->beginTime = DELETE_NUMBER;
					break;
					}
				if (req->beginTime != 0) {
					break;
				}

				strcpy (savearg, optarg);
				if (gettimefor (optarg, &req->beginTime) < 0)
					{
					lsberrno = LSBE_BAD_TIME;
					if (errMsg != NULL) {
						sprintf (*errMsg, "%s:%s", savearg, lsb_sysmsg ());
					}
					else {
						sub_perror (savearg);
					}
					return -1;
				}
				break;

				case 't':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				if (req->options & SUB_MODIFY && strcmp (optName, "tn") == 0)
					{
					if (req->termTime != 0 && req->termTime != DELETE_NUMBER)
						{
						/* catgets 417 */
						PRINT_ERRMSG0 (errMsg, "catgets 417: You cannot modify and set default at the same time");
						return -1;
						}
					req->termTime = DELETE_NUMBER;
					break;
					}

				if (req->termTime != 0) {
					break;
				}

				strcpy (savearg, optarg);
				if (gettimefor (optarg, &req->termTime) < 0)
					{
					lsberrno = LSBE_BAD_TIME;
					if (errMsg != NULL) {
						sprintf (*errMsg, "%s:%s", savearg, lsb_sysmsg ());
					}
					else{
						sub_perror (savearg);
					}
					return -1;
					}
				break;

				case 's':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				if (strncmp (optName, "sp", 2) == 0)
					{

					checkSubDelOption2( req, SUB2_JOB_PRIORITY );
					if (!(mask2 & SUB2_JOB_PRIORITY)) {
						break;
					}

					if (!isint_ (optarg) || (req->userPriority = atoi (optarg)) <= 0)
						{
						/* catgets 494 */
						PRINT_ERRMSG1 (errMsg, "catgets 494: %s: Illegal job priority", optarg);
						return -1;
						}
					req->options2 |= SUB2_JOB_PRIORITY;
					}
				else
					{
					printf ("-s option is set\n");
					checkSubDelOption( req, SUB_WINDOW_SIG );

					if (!(mask & SUB_WINDOW_SIG)) {
						break;
					}
					if ((req->sigValue = getSigVal (optarg)) < 0) {
						{
						/* catgets 419 */
						PRINT_ERRMSG1 (errMsg, "catgets 419: %s: Illegal signal value", optarg);
						return -1;
					}
				}
					req->options |= SUB_WINDOW_SIG;
			}
				break;

				case 'c':
				req->options2 |= SUB2_MODIFY_RUN_JOB;
				checkRLDelOption( req, LSF_RLIMIT_CPU );
				if (req->rLimits[LSF_RLIMIT_CPU] != DEFAULT_RLIMIT) {
					break;
				}
				strcpy (savearg, optarg);

				if ((sp = strchr (optarg, '/')) != NULL && strlen (sp + 1) > 0)
					{
					req->options |= SUB_HOST_SPEC;
					if (req->hostSpec && strcmp (req->hostSpec, sp + 1) != 0)
						{   
						 /* catgets 420 */
						PRINT_ERRMSG2 (errMsg, "catgets 420: More than one host_spec is specified: <%s> and <%s>", req->hostSpec, sp + 1);
						return -1;
						}
					req->hostSpec = sp + 1;
					}
				if (sp) {
					*sp = '\0';
				}

				if ((cp = strchr (optarg, ':')) != NULL) {
					*cp = '\0';
				}
				if ((!isint_ (optarg)) || (atoi (optarg) < 0))
					{
					/* catgets 421 */
					PRINT_ERRMSG1 (errMsg, "catgets 421: %s: Bad CPULIMIT specification", savearg);
					return -1;
					}
				else {
					req->rLimits[LSF_RLIMIT_CPU] = atoi (optarg);
				}
				if (cp != NULL)
					{
					optarg = cp + 1;
					if ((!isint_ (optarg)) || (atoi (optarg) < 0))
						{
						/* catgets 421 */
						PRINT_ERRMSG1 (errMsg, "catgets 421: %s: Bad CPULIMIT specification", savearg);
						return -1;
						}
					else
						{
						req->rLimits[LSF_RLIMIT_CPU] *= 60;
						req->rLimits[LSF_RLIMIT_CPU] += atoi (optarg);
						}
					}
				if (req->rLimits[LSF_RLIMIT_CPU] < 0)
					{
					/* catgets 423 */
					PRINT_ERRMSG1 (errMsg, "catgets 423: %s: CPULIMIT value should be a positive integer", savearg);
					return -1;
					}

				if (!checkLimit (req->rLimits[LSF_RLIMIT_CPU], 60))
					{
					/* catgets 424 */
					PRINT_ERRMSG1 (errMsg, "catgets 424: %s: CPULIMIT value is too big", optarg);
					return -1;
					}

				req->rLimits[LSF_RLIMIT_CPU] *= 60;
				break;

				case 'P':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkSubDelOption( req, SUB_PROJECT_NAME );
				if ((mask & SUB_PROJECT_NAME))
					{
					if (strlen (optarg) > MAX_LSB_NAME_LEN - 1)
						{
						/* catgets 425 */
						PRINT_ERRMSG1 (errMsg, "catgets 425: %s:  project name too long", optarg);
						return -1;
						}

					req->projectName = optarg;
					req->options |= SUB_PROJECT_NAME;
					}
				break;


				case 'W':
				req->options2 |= SUB2_MODIFY_RUN_JOB;
				checkRLDelOption( req, LSF_RLIMIT_RUN );

				if (req->rLimits[LSF_RLIMIT_RUN] != DEFAULT_RLIMIT) {
					break;
				}

				strcpy (savearg, optarg);
				if ((sp = strchr (optarg, '/')) != NULL)
					{
					req->options |= SUB_HOST_SPEC;
					if (req->hostSpec && strcmp (req->hostSpec, sp + 1) != 0)
						{
						/* catgets 420 */
						PRINT_ERRMSG2 (errMsg, "catgets 420: More than one host_spec is specified: <%s> and <%s>", req->hostSpec, sp + 1);
						return -1;
						}
					req->hostSpec = sp + 1;
					*sp = '\0';
					}
				if ((cp = strchr (optarg, ':')) != NULL) {
					*cp = '\0';
				}
				if ((!isint_ (optarg)) || (atoi (optarg) < 0))
					{
					/* catgets 428 */
					PRINT_ERRMSG1 (errMsg, "catgets 428: %s: Bad RUNLIMIT specification", savearg);
					return -1;
					}
				else {
					req->rLimits[LSF_RLIMIT_RUN] = atoi (optarg);
				}
				if (cp != NULL)
					{
					optarg = cp + 1;
					if ((!isint_ (optarg)) || (atoi (optarg) < 0))
						{
						PRINT_ERRMSG1 (errMsg, "catgets 428: %s: Bad RUNLIMIT specification", savearg);
						return -1;
						}
					else
						{
						req->rLimits[LSF_RLIMIT_RUN] *= 60;
						req->rLimits[LSF_RLIMIT_RUN] += atoi (optarg);
						}
					}
				if (req->rLimits[LSF_RLIMIT_RUN] < 0)
					{
					/* catgets 430 */
					PRINT_ERRMSG1 (errMsg, "catgets 430: %s: RUNLIMIT value should be a positive integer", savearg);
					return -1;
					}
				if (!checkLimit (req->rLimits[LSF_RLIMIT_RUN], 60))
					{
					/* catgets 431 */
					PRINT_ERRMSG1 (errMsg, "catgets 431: %s: RUNLIMIT value is too big", optarg);
					return -1;
					}

				req->rLimits[LSF_RLIMIT_RUN] *= 60;
				break;

				case 'F':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkRLDelOption( req, LSF_RLIMIT_FSIZE);
				if (req->rLimits[LSF_RLIMIT_FSIZE] != DEFAULT_RLIMIT) {
					break;
				}

				if (isint_ (optarg) && ((req->rLimits[LSF_RLIMIT_FSIZE] = atoi (optarg)) > 0))
					{
					break;
					}
				/* catgets 432 */
				PRINT_ERRMSG1 (errMsg, "catgets 432: %s: FILELIMIT value should be a positive integer", optarg);
				return -1;

				case 'D':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkRLDelOption( req, LSF_RLIMIT_DATA);
				if (req->rLimits[LSF_RLIMIT_DATA] != DEFAULT_RLIMIT) {
					break;
				}

				if (isint_ (optarg) && ((req->rLimits[LSF_RLIMIT_DATA] = atoi (optarg)) > 0))
					{
					break;
					}
				/* catgets 433 */
				PRINT_ERRMSG1 (errMsg, "catgets 433: %s: DATALIMIT value should be a positive integer", optarg);
				return -1;

				case 'S':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkRLDelOption( req, LSF_RLIMIT_STACK);

				if (req->rLimits[LSF_RLIMIT_STACK] != DEFAULT_RLIMIT) {
					break;
				}

				if (isint_ (optarg)  && ((req->rLimits[LSF_RLIMIT_STACK] = atoi (optarg)) > 0))
					{
					break;
					}
				/* catgets 434 */
				PRINT_ERRMSG1 (errMsg, "catgets 434: %s: STACKLIMIT value should be a positive integer", optarg);
				return -1;

				case 'C':
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				checkRLDelOption( req, LSF_RLIMIT_CORE);
				if (req->rLimits[LSF_RLIMIT_CORE] != DEFAULT_RLIMIT) {
					break;
				}

				if (isint_ (optarg) && ((req->rLimits[LSF_RLIMIT_CORE] = atoi (optarg)) >= 0))
					{
					break;
					}
				/* catgets 435 */
				PRINT_ERRMSG1 (errMsg, "catgets 435: %s: CORELIMIT value should be a positive integer", optarg);
				return -1;

				case 'M':
				req->options2 |= SUB2_MODIFY_RUN_JOB;
				checkRLDelOption ( req, LSF_RLIMIT_RSS);
				if (req->rLimits[LSF_RLIMIT_RSS] != DEFAULT_RLIMIT) {
					break;
				}

				if (isint_ (optarg) && ((req->rLimits[LSF_RLIMIT_RSS] = atoi (optarg)) > 0))
					{
					break;
					}
				/* catgets 436 */
				PRINT_ERRMSG1 (errMsg, "catgets 436: %s: MEMLIMT value should be a positive integer", optarg);
				return -1;


				case 'p':
				checkRLDelOption( req, LSF_RLIMIT_PROCESS);
				if (req->rLimits[LSF_RLIMIT_PROCESS] != DEFAULT_RLIMIT)

					break;

				if (isint_ (optarg) && ((req->rLimits[LSF_RLIMIT_PROCESS] = atoi (optarg)) > 0))
					{
					if (!checkLimit (req->rLimits[LSF_RLIMIT_PROCESS], 1))
						{
						/* catgets 437 */
						PRINT_ERRMSG1 (errMsg, "catgets 437: %s: PROCESSLIMIT value is too big\n", optarg);
						return -1;
						}
					break;
					}
				/* catgets 438 */
				PRINT_ERRMSG1 (errMsg, "catgets 438: %s: PROCESSLIMIT value should be a positive integer\n", optarg);
				return -1;

				case 'v':
				checkRLDelOption( req, LSF_RLIMIT_SWAP );
				if (req->rLimits[LSF_RLIMIT_SWAP] != DEFAULT_RLIMIT) {
					break;
				}

				if (isint_ (optarg) && ((req->rLimits[LSF_RLIMIT_SWAP] = atoi (optarg)) > 0))
					{
					break;
					}
				/* catgets 439 */
				PRINT_ERRMSG1 (errMsg, "catgets 439: %s: SWAPLIMIT value should be a positive integer\n", optarg);
				return -1;

				case 'O':
				if (req->options == 0)
					{
					optionFlag = TRUE;
					strcpy (optionFileName, optarg);
					}
				else
					req->options |= SUB_MODIFY_ONCE;
				break;

				case 'Z':
				pExclStr = "Zsn|Zs|Z";
				req->options2 |= SUB2_MODIFY_PEND_JOB;
				if (strncmp (optName, "Zs", 2) == 0)
					{

					if (req->options2 & SUB2_MODIFY_CMD)
						{
						/* catgets  444 */
						PRINT_ERRMSG1 (errMsg, "catgets 444: %s options are exclusive", pExclStr);
						return -1;
						}

					checkSubDelOption2( req, SUB2_JOB_CMD_SPOOL);
					if (!(mask2 & SUB2_JOB_CMD_SPOOL)) {
						break;
					}

					if (req->options & SUB_MODIFY)
						{
						if (strlen (optarg) >= MAX_LINE_LEN)
							{
							/* catgets 409 */
							PRINT_ERRMSG1 (errMsg, "catgets 409: %s: File name too long", optarg);
							return -1;
							}

						req->newCommand = optarg;
						req->options2 |= SUB2_MODIFY_CMD;
						}

					req->options2 |= SUB2_JOB_CMD_SPOOL;
					}
				else
					{

					if ((req->options2 & SUB2_JOB_CMD_SPOOL)
						|| (req->delOptions2 & SUB2_JOB_CMD_SPOOL))
						{
						/* catgets  444 */
						PRINT_ERRMSG1 (errMsg, "catgets 444: %s options are exclusive", pExclStr);
						return -1;
						}

					if (strlen (optarg) >= MAX_LINE_LEN)
						{
						/* catgets 409 */
						PRINT_ERRMSG1 (errMsg, "catgets 409: %s: File name too long", optarg);
						return -1;
						}

					req->newCommand = optarg;
					req->options2 |= SUB2_MODIFY_CMD;
					}
				break;
				case 'a':
				additionEsubInfo = putstr_ (optarg);
				break;
				case 'V':
				fputs (_LS_VERSION_, stderr);
				exit (0);

				default:
				myArgs.argc = req->options;
				lsb_throw ("LSB_BAD_BSUBARGS", &myArgs);

				return -1;
			}

		}

	if ((req->options & SUB_INTERACTIVE) && (req->options & SUB_JOB_NAME) && strchr (req->jobName, '['))
		{
		/* catgets 440 */
		PRINT_ERRMSG0 (errMsg, "catgets 440: Interactive job not supported for job arrays");
		return -1;
		}

	if ((req->options2 & SUB2_BSUB_BLOCK) && (req->options & SUB_JOB_NAME) && strchr (req->jobName, '['))
		{
		/* catgets 441 */
		PRINT_ERRMSG0 (errMsg, "catgets 441: Job array doesn't support -K option");
		return -1;
		}
	return 0;
}


int
parseLine_ (char *line, unsigned int *embedArgc, char ***embedArgv, char **errMsg)
{
#define INCREASE 40
	int parsing = TRUE, i;
	static char **argBuf = NULL;
	char *key         = NULL;
	static int argNum = 0;
	static int first  = TRUE;
	unsigned int bufSize = INCREASE;
	char *sp       = NULL;
	char *sQuote   = NULL;
	char *dQuote   = NULL;
	char quoteMark;
	char **tmp     = NULL;

	assert( errMsg ); // FIXME FIXME FIXME FIXME useless call

	if (first == TRUE)
		{
		argBuf = malloc (INCREASE * sizeof( *argBuf ) );
		if (NULL == argBuf && ENOMEM == errno)
			{
			fprintf( stderr,  "%s: failure to allocate more memory for argBuf.\n", __func__ ); // FIXME FIXME FIXME FIXME this should really be a call to syslog
			return -1;
			}
		first = FALSE;
		}
	else
		{
		for (i = 0; i < argNum; i++) {
			FREEUP (argBuf[i + 1]);
		}
		argNum = 0;
	}
	*embedArgc = 1;
	argBuf[0] = "bsub";
	argNum = 1;
	key = "BSUB";
	SKIPSPACE (line);


	if (*line == '\0' || *line != '#') {
		return -1;
	}

	++line;
	SKIPSPACE (line);
	if (strncmp (line, key, strlen (key)) == 0)
		{
		line += strlen (key);
		SKIPSPACE (line);
		if (*line != '-')
			{
			parsing = FALSE;
			return -1;
			}
		while (TRUE)
			{
			quoteMark = '"';
			if ((sQuote = strchr (line, '\'')) != NULL)
				if ((dQuote = strchr (line, '"')) == NULL || sQuote < dQuote)

					quoteMark = '\'';

			if ((sp = getNextValueQ_ (&line, quoteMark, quoteMark)) == NULL) {
				goto FINISH;
			}

			if (*sp == '#') {
				goto FINISH;
			}

			assert( *embedArgc + 2 >= 0);
			if ( (*embedArgc + 2) > bufSize)
				{
				tmp = realloc (argBuf, (bufSize + INCREASE) * sizeof (char *));
				if (NULL == tmp )
					{
					// PRINT_ERRMSG2 (errMsg, I18N_FUNC_FAIL_M, __func__, "realloc");
					fprintf( stderr, "%s: failed to realloc memory for argBuf\n", __func__ );
					argNum = *embedArgc - 1;
					*embedArgv = argBuf;
					return -1;
					}
				argBuf = tmp;
				bufSize += INCREASE;
				}
			argBuf[*embedArgc] = putstr_ (sp);
			(*embedArgc)++;
			argBuf[*embedArgc] = NULL;
			}
		}
	else {
		return -1;
	}

FINISH:
	argNum = *embedArgc - 1;
	argBuf[*embedArgc] = NULL;
	*embedArgv = argBuf;
	return 0;
}


struct submit *
parseOptFile_ (char *filename, struct submit *req, char **errMsg)
{
	ssize_t length  = 0;
	size_t lineLen  = 0;
	unsigned int optArgc     = 0;
	char **optArgv  = NULL;
	char *lineBuf   = NULL;
	char template[] = "E:T:w:f:k:R:m:J:L:u:i:o:e:n:q:b:t:sp:s:c:W:F:D:S:C:M:P:p:v:Ip|Is|I|r|H|x|N|B|h|V|G:X:";
	pid_t pid = 0;
	uid_t uid = 0;
	int childIoFd[2] = { 0, 0 };
	LS_WAIT_T status = 0;

	if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: Entering function", __func__);
	}
	if (access (filename, F_OK) != 0)
		{
		// PRINT_ERRMSG3 (errMsg, I18N_FUNC_S_FAIL, __func__, "access", filename);
		fprintf( stderr, "%s: file access failure at filename %s\n", __func__, filename);
		return NULL;
		}

	if (socketpair (AF_UNIX, SOCK_STREAM, 0, childIoFd) < 0)
		{
		// PRINT_ERRMSG2 (errMsg, I18N_FUNC_FAIL, __func__, "socketpair");
		fprintf( stderr, "%s: failure at openting socket pair\n", __func__ );
		lsberrno = LSBE_SYS_CALL;
		return NULL;
		}
	optArgc = 0;

	pid = fork ();
	if (pid < 0)
		{
		lsberrno = LSBE_SYS_CALL;
//		PRINT_ERRMSG2 (errMsg, I18N_FUNC_FAIL_M, __func__, "fork");
		fprintf( stderr, "%s: could not fork \n", __func__ );
		return NULL;
		}
	else if (pid == 0)
		{

		char childLine[MAX_LINE_LEN * 4];
		int exitVal = -1;

		close (childIoFd[0]);
		uid = getuid ();
		if (setuid (uid) < 0)
			{
			lsberrno = LSBE_BAD_USER;
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "setuid");
			goto childExit;
			}
		if (logclass & (LC_TRACE | LC_EXEC)) {
			ls_syslog (LOG_DEBUG1, "%s: Child tries to open a option file <%s>", __func__, filename);
		}
		lineLen = 0;
		childLine[0] = '\0';

		if (readOptFile (filename, childLine) < 0) {
			exit (exitVal);
		}

		lineLen = strlen (childLine);

		if (write (childIoFd[1], (char *) &lineLen, sizeof (lineLen))
			!= sizeof (lineLen))
			{
			goto childExit;
			}
		assert( lineLen <= LONG_MAX );
		if (write (childIoFd[1], (char *) childLine, lineLen) != (long) lineLen)
			{
			goto childExit;
			}
		exitVal = 0;
		childExit:
		if (logclass & (LC_TRACE | LC_EXEC))
			{
			if (exitVal == 0) {
				ls_syslog (LOG_DEBUG, "%s: Child succeeded in sending messages to parent", __func__);
			}
			else {
				ls_syslog (LOG_DEBUG, "%s: Child failed in sending messages to parent", __func__);
			}
			}
		close (childIoFd[1]);
		exit (exitVal);
		}


	if (waitpid (pid, &status, 0) < 0)
		{
		lsberrno = LSBE_SYS_CALL;
		goto parentErr;
		}

	close (childIoFd[1]);
	if (WEXITSTATUS (status))
		{
		ls_syslog (LOG_DEBUG, "%s: child failed!", __func__);
		goto parentErr;
		}

	if (read (childIoFd[0], (char *) &length, sizeof (length)) != sizeof (length)) {
		goto parentErr;
	}

	assert( length >= 0 );
	lineBuf = (char *) malloc ((size_t)length + 1);
	if (NULL == lineBuf && ENOMEM == errno  )
		{
		if (logclass & (LC_TRACE | LC_EXEC)) {
			ls_syslog (LOG_DEBUG, "%s: parent malloc faild!", __func__);
		}
		goto parentErr;
		}
	assert( length >= 0 );
	if (read (childIoFd[0], (char *) lineBuf, (unsigned long)length) != length)
		{
		goto parentErr;
		}

	if (length)
		{
		char *p;
		p = lineBuf + length;
		*p = '\0';
		if (parseLine_ (lineBuf, &optArgc, &optArgv, errMsg) == -1) {
			goto parentErr;
		}
		}
	if (optArgc > 1)
		{

		optind = 1;
		if (setOption_ (optArgc, optArgv, template, req, ~req->options, ~req->options2, errMsg) == -1) {
			goto parentErr;
		}
		}
	close (childIoFd[0]);
	FREEUP (lineBuf);
	return req;

parentErr:
	if (logclass & (LC_TRACE | LC_EXEC)) {
		ls_syslog (LOG_DEBUG, "%s: parent malloc faild!", __func__);
	}
	FREEUP (lineBuf);
	close (childIoFd[0]);
	return NULL;
}


void
subUsage_ (int option, char **errMsg)
{
	if (errMsg == NULL)
		{
		if (option & SUB_RESTART) {
			/* catgets 5000 */
			fprintf (stderr, "catgets 5000: Usage");
			fprintf (stderr, ": brestart [-h] [-V] [-x] [-f] [-N] [-B] [-q \"queue_name...\"] \n");
			fprintf (stderr, "\t\t[-m \"host_name[+[pref_level]] | host_group[+[pref_level]]...]\"\n");

			fprintf (stderr, "\t\t[-w \'dependency_expression\'] [-b begin_time] [-t term_time]\n");
			fprintf (stderr, "\t\t[-c [hour:]minute[/host_name|/host_model]]\n");
			fprintf (stderr, "\t\t[-F file_limit] [-D data_limit]\n");
			fprintf (stderr, "\t\t[-C core_limit] [-M mem_limit] \n");
			fprintf (stderr, "\t\t[-W run_limit[/host_name|/host_model]] \n");
			fprintf (stderr, "\t\t[-S stack_limit] [-E \"pre_exec_command [argument ...]\"]\n");

			fprintf (stderr, "\t\tcheckpoint_dir[job_ID | \"job_ID[index]\"]\n");
			/* catgets 447 */
			fprintf (stderr, "catgets 447:      [-a additional_esub_information]");

			}
		else if (option & SUB_MODIFY)
			{
			/* catgets 5000 */
			fprintf (stderr, "catgets 5000: Usage");
			fprintf (stderr, ": bmod [-h] [-V] [-x | -xn]");

			fprintf (stderr, "\n");

			if (lsbMode_ & LSB_MODE_BATCH)
				{
				fprintf (stderr, "\t\t[-r | -rn] [-N | -Nn] [-B | -Bn]\n");
				fprintf (stderr, "\t\t[-c cpu_limit[/host_spec] | -cn] [-F file_limit | -Fn]\n");
				fprintf (stderr, "\t\t[-M mem_limit | -Mn] [-D data_limit | -Dn] [-S stack_limit | -Sn]\n");
				fprintf (stderr, "\t\t[-C core_limit | -Cn] [-W run_limit[/host_spec] | -Wn ]\n");
				fprintf (stderr, "\t\t[-k chkpnt_dir [chkpnt_period] | -kn] [-P project_name | -Pn]\n");
				fprintf (stderr, "\t\t[-L login_shell | -Ln] \n");
				}


			fprintf (stderr, "\t\t[-w depend_cond | -wn] [-R res_req| -Rn] [-J job_name | -Jn]\n");
			fprintf (stderr, "\t\t[-q queue_name ... | -qn] \n");
			fprintf (stderr, "\t\t[-m host_name[+[pref_level]] | host_group[+[pref_level]]...| -mn]\"\n");
			fprintf (stderr, "\t\t[-n min_processors[,max_processors] | -nn]\n");
			fprintf (stderr, "\t\t[-b begin_time | -bn] [-t term_time | -tn]\n");
			fprintf (stderr, "\t\t[-i in_file | -is in_file | -in | -isn]\n");
			fprintf (stderr, "\t\t[-o out_file | -on] [-e err_file | -en]\n");
			fprintf (stderr, "\t\t[-u mail_user | -un] [[-f \"lfile op [rfile]\"] ... | -fn] \n");
			fprintf (stderr, "\t\t[-E \"pre_exec_command [argument ...]\" | -En]\n");
			fprintf (stderr, "\t\t[-sp job_priority | -spn]\n");
			fprintf (stderr, "\t\t[-Z \"new_command\" | -Zs \"new_command\" | -Zsn] \n");
			fprintf (stderr, "\t\t[ jobId | \"jobId[index_list]\" ] \n");
			/* catets 447 */
			fprintf (stderr, "catgets 447:      [-a additional_esub_information]");
			}
		else
			{

			/* catgets 5000 */
			fprintf (stderr, "catgets 5000: Usage");
			fprintf (stderr, ": bsub [-h] [-V] [-x] [-H]");

			if (lsbMode_ & LSB_MODE_BATCH)
				{
				fprintf (stderr, " [-r] [-N] [-B] [-I | -K | -Ip | -Is]\n");
				fprintf (stderr, "\t\t[-L login_shell] [-c cpu_limit[/host_spec]] [-F file_limit]\n");
				fprintf (stderr, "\t\t[-W run_limit[/host_spec]] [-k chkpnt_dir [chkpnt_period] [method=chkpnt_dir]]\n");
				fprintf (stderr, "\t\t[-P project_name] ");
				}
			fprintf (stderr, "\n");

			fprintf (stderr, "\t\t[-q queue_name ...]  [-R res_req]\n");
			fprintf (stderr, "\t\t[-m \"host_name[+[pref_level]] | host_group[+[pref_level]]...]\"\n");
			fprintf (stderr, "\t\t[-n min_processors[,max_processors]] [-J job_name]\n");
			fprintf (stderr, "\t\t[-b begin_time] [-t term_time] [-u mail_user]\n");
			fprintf (stderr, "\t\t[-i in_file | -is in_file] [-o out_file] [-e err_file]\n");
			fprintf (stderr, "\t\t[-M mem_limit]  [-D data_limit]  [-S stack_limit]\n");

			fprintf (stderr, "\t\t[[-f \"lfile op [rfile]\"] ...] [-w depend_cond]\n");

			fprintf (stderr, "\t\t[-E \"pre_exec_command [argument ...]\"] [-Zs]\n");
			fprintf (stderr, "\t\t[-sp job_priority]\n");
			fprintf (stderr, "\t\t[command [argument ...]]\n");
			fprintf (stderr, "catgets 447:        [-a additional_esub_information]");
			}

		exit (-1);
		}

}


int
parseXF (struct submit *req, char *arg, char **errMsg)
{
	static unsigned int maxNxf = 0;
	static struct xFile *xp = NULL;
	struct xFile *tmpXp = NULL;
	int options = 0;
	char op[MAX_LINE_LEN], lf[MAX_FILENAME_LEN], rf[MAX_FILENAME_LEN];
	char *p;
	char saveArg[MAX_LINE_LEN];
	const int NUMXF = 10;

	if (maxNxf == 0) {

		xp = (struct xFile *) malloc (NUMXF * sizeof (struct xFile));
		if ( NULL == xp && ENOMEM == errno ) {
			if (errMsg != NULL) {
				 /* catgets 4 */
				sprintf (*errMsg, "catgets 4: %s: %s() failed, %s.", __func__, "malloc", lsb_sysmsg ());
			}
			else {
				/* catgets 484 */
				sub_perror( "catgets 484: Unable to allocate memory for -f option" );  
			}
			return -1;
		}
		maxNxf = NUMXF;
	}

	req->xf = xp;

	strcpy (saveArg, arg);
	if ((p = strstr (saveArg, "<<")))
		{
		strcpy (op, "<<");
		options = XF_OP_EXEC2SUB | XF_OP_EXEC2SUB_APPEND;
		}
	else if ((p = strstr (saveArg, "><")))
		{
		strcpy (op, "><");
		options = XF_OP_EXEC2SUB | XF_OP_SUB2EXEC;
		}
	else if ((p = strstr (saveArg, "<>")))
		{
		strcpy (op, "<>");
		options = XF_OP_EXEC2SUB | XF_OP_SUB2EXEC;
		}
	else if ((p = strstr (saveArg, ">>")))
		{
		strcpy (op, ">>");
		/* catgets 487 */
		PRINT_ERRMSG2 (errMsg, "catgets 487: Invalid file operation \"%s\" specification in -f \"%s\"", op, saveArg);
		return -1;
		}
	else if ((p = strstr (saveArg, "<")))
		{
		strcpy (op, "<");
		options = XF_OP_EXEC2SUB;
		}
	else if ((p = strstr (saveArg, ">")))
		{
		strcpy (op, ">");
		options = XF_OP_SUB2EXEC;
		}
	else
		{
		/* catgets 487 */
		PRINT_ERRMSG2 (errMsg, "catgets 487: Invalid file operation \"%s\" specification in -f \"%s\"", op, saveArg);
		return -1;
		}

	memset (lf, 0, MAX_FILENAME_LEN);
	memset (rf, 0, MAX_FILENAME_LEN);
	memcpy (lf, saveArg, p - saveArg);
	memcpy (rf, p + strlen (op), strlen (saveArg) - strlen (lf) - strlen (op));

	if (strstr (lf, ">") || strstr (rf, ">") || strstr (rf, "<"))
		{
		/* catgets 487 */
		PRINT_ERRMSG2 (errMsg, "catgets 487: Invalid file operation \"%s\" specification in -f \"%s\"", op, saveArg);
		return -1;
		}

	if (strlen (lf) != 0)
		{
		if ((lf[strlen (lf) - 1]) != ' ')
			{
			/* catgets 487 */
			PRINT_ERRMSG2 (errMsg, "catgets 487: Invalid file operation \"%s\" specification in -f \"%s\"", op, saveArg);
			return -1;
			}

		trimSpaces (lf);

		if (strlen (lf) == 0)
			{
			/* catgets 485 */
			PRINT_ERRMSG1 (errMsg, "catgets 485: Invalid local file specification in -f \"%s\"", saveArg);
			return -1;
			}
		}
	else
		{
		/* catgets 485 */
		PRINT_ERRMSG1 (errMsg, "catgets 485: Invalid local file specification in -f \"%s\"", saveArg);
		return -1;
		}

	if (strlen (rf) != 0)
		{
		if ((rf[0]) != ' ')
			{
			/* catgets 487 */
			PRINT_ERRMSG2 (errMsg, "catgets 487: Invalid file operation \"%s\" specification in -f \"%s\"", op, saveArg);
			return -1;
			}

		trimSpaces (rf);

		if (strlen (rf) == 0)
			{
			strcpy (rf, lf);
			}
		}
	else
		{
		strcpy (rf, lf);
		}

	if (req->nxf + 1 > maxNxf)
		{
		tmpXp = xp;
		xp = (struct xFile *) myrealloc (req->xf, (maxNxf + NUMXF) * sizeof (struct xFile));
		if ( NULL == xp && ENOMEM == errno )
			{
			if (errMsg != NULL)
				{
				/* catgets 4 */
				sprintf (*errMsg, "catgets 4: %s: %s() failed, %s.", __func__, "myrealloc", lsb_sysmsg ());
				}
			else {
				/* catgets 484 */
				sub_perror( "catgets 484: Unable to allocate memory for -f option" );
			}
			xp = tmpXp;
			return -1;
		}
		maxNxf += NUMXF;
		req->xf = xp;
	}

	strcpy (req->xf[req->nxf].subFn, lf);
	strcpy (req->xf[req->nxf].execFn, rf);
	req->xf[req->nxf].options = options;

	req->nxf++;
	return 0;
}


int
checkLimit (int limit, int factor)
{
	if ((float) limit * (float) factor >= (float) INFINIT_INT) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}


int
runBatchEsub (struct lenData *ed_, struct submit *jobSubReq)
{

	char *subRLimitName[LSF_RLIM_NLIMITS] = 
	{   "LSB_SUB_RLIMIT_CPU",
		"LSB_SUB_RLIMIT_FSIZE",
		"LSB_SUB_RLIMIT_DATA",
		"LSB_SUB_RLIMIT_STACK",
		"LSB_SUB_RLIMIT_CORE",
		"LSB_SUB_RLIMIT_RSS",
		"LSB_SUB_RLIMIT_NOFILE",
		"LSB_SUB_RLIMIT_OPEN_MAX",
		"LSB_SUB_RLIMIT_SWAP",
		"LSB_SUB_RLIMIT_RUN",
		"LSB_SUB_RLIMIT_PROCESS"
	};
	int cc = 0;
	// int i  = 0;
	char parmFile[MAX_FILENAME_LEN];
	char esub[MAX_FILENAME_LEN];
	FILE *parmfp;
	struct stat sbuf;
	const unsigned int LSB_SUB_COMMANDNAME  = 0;
	struct config_param myParams[] = { 
		{ "LSB_SUB_COMMANDNAME", NULL },
		{ NULL,                  NULL }
	};

// FIXME FIXME FIXME FIXME FIXME these defines have to become regular functions

#define QUOTE_STR(_str1,_str)  { \
int ii, j; unsigned int cnt=0; \
char ch, next, *tmp_str=NULL; \
for (ii=0; _str1[ii]; ii++){ \
if (_str1[ii] == '"') \
cnt++ ; \
} \
tmp_str = malloc((strlen(_str1)+1+cnt+2)*sizeof(char)) ; \
if (tmp_str != NULL) { \
tmp_str[0] = '"' ; \
_str = tmp_str+1 ; \
strcpy (_str, _str1) ; \
for (ii=0; _str[ii]; ){ \
if (_str[ii] == '"') { \
ch = _str[ii] ; \
_str[ii] = '\\' ; \
next = ch ; \
for ( j=ii+1; _str[j]; j++){ \
ch = _str[j] ;  \
_str[j] = next ;  \
next = ch ; \
} \
_str[j] = next ;  \
_str[j+1] = '\0' ; \
ii += 2;  \
} \
ii++ ; \
}  \
for (ii=0; _str[ii]; ii++ ){} \
_str[ii] = '"' ; \
_str[ii+1] = '\0' ; \
} \
_str = tmp_str ; \
}

#define SET_PARM_STR(flag, name, sub, field) { \
char *quote_field;  \
if (((sub)->options & flag) && (sub)->field) { \
QUOTE_STR((sub)->field,quote_field) ; \
if (quote_field != NULL ){ \
fprintf(parmfp, "%s=%s\n", name, quote_field); \
free(quote_field) ;  \
} \
} else if ((sub)->delOptions & flag) { \
fprintf(parmfp, "%s=SUB_RESET\n", name); \
} \
}

#define SET_PARM_BOOL(flag, name, sub) { \
if ((sub)->options & flag) { \
fprintf(parmfp, "%s=Y\n", name); \
} else if ((sub)->delOptions & flag) { \
fprintf(parmfp, "%s=SUB_RESET\n", name); \
} \
}

#define SET_PARM_INT(flag, name, sub, field) { \
if ((sub)->options & flag) { \
fprintf(parmfp, "%s=%d\n", name, (int) (sub)->field); \
} else if ((sub)->delOptions & flag) { \
fprintf(parmfp, "%s=SUB_RESET\n", name); \
} \
}

#define SET_PARM_STR_2(flag, name, sub, field) { \
char *quote_field;  \
if (((sub)->options2 & flag) && (sub)->field) { \
QUOTE_STR((sub)->field,quote_field) ; \
if (quote_field != NULL ){ \
fprintf(parmfp, "%s=%s\n", name, quote_field); \
free(quote_field) ;  \
} \
} else if ((sub)->delOptions2 & flag) { \
fprintf(parmfp, "%s=SUB_RESET\n", name); \
} \
}

#define SET_PARM_BOOL_2(flag, name, sub) { \
if ((sub)->options2 & flag) { \
fprintf(parmfp, "%s=Y\n", name); \
} else if ((sub)->delOptions2 & flag) { \
fprintf(parmfp, "%s=SUB_RESET\n", name); \
} \
}

#define SET_PARM_INT_2(flag, name, sub, field) { \
if ((sub)->options2 & flag) { \
fprintf(parmfp, "%s=%d\n", name, (int) (sub)->field); \
} else if ((sub)->delOptions2 & flag) { \
fprintf(parmfp, "%s=SUB_RESET\n", name); \
} \
}

#define SET_PARM_NUMBER(name, field, delnum, defnum) { \
if (field == delnum) { \
fprintf(parmfp, "%s=SUB_RESET\n", name); \
} else if (field != defnum) { \
fprintf(parmfp, "%s=%d\n", name, (int) field); \
} \
}



	sprintf (esub, "%s/%s", lsbParams[LSB_SERVERDIR].paramValue, __func__);
	if (stat (esub, &sbuf) < 0) {
		return 0;
	}


	sprintf (parmFile, "%s/.lsbsubparm.%d", LSTMPDIR, (int) getpid ());

	if ((parmfp = fopen (parmFile, "w")) == NULL)
		{
		lsberrno = LSBE_SYS_CALL;
		return -1;
		}

	chmod (parmFile, 0666);

	SET_PARM_STR (SUB_JOB_NAME, "LSB_SUB_JOB_NAME", jobSubReq, jobName);
	SET_PARM_STR (SUB_QUEUE, "LSB_SUB_QUEUE", jobSubReq, queue);
	SET_PARM_STR (SUB_IN_FILE, "LSB_SUB_IN_FILE", jobSubReq, inFile);
	SET_PARM_STR (SUB_OUT_FILE, "LSB_SUB_OUT_FILE", jobSubReq, outFile);
	SET_PARM_STR (SUB_ERR_FILE, "LSB_SUB_ERR_FILE", jobSubReq, errFile);
	SET_PARM_BOOL (SUB_EXCLUSIVE, "LSB_SUB_EXCLUSIVE", jobSubReq);
	SET_PARM_BOOL (SUB_NOTIFY_END, "LSB_SUB_NOTIFY_END", jobSubReq);
	SET_PARM_BOOL (SUB_NOTIFY_BEGIN, "LSB_SUB_NOTIFY_BEGIN", jobSubReq);
	SET_PARM_INT (SUB_CHKPNT_PERIOD, "LSB_SUB_CHKPNT_PERIOD", jobSubReq,
				  chkpntPeriod);
	SET_PARM_STR (SUB_CHKPNT_DIR, "LSB_SUB_CHKPNT_DIR", jobSubReq, chkpntDir);
	SET_PARM_BOOL (SUB_RESTART_FORCE, "LSB_SUB_RESTART_FORCE", jobSubReq);
	SET_PARM_BOOL (SUB_RESTART, "LSB_SUB_RESTART", jobSubReq);
	SET_PARM_BOOL (SUB_RERUNNABLE, "LSB_SUB_RERUNNABLE", jobSubReq);
	SET_PARM_BOOL (SUB_WINDOW_SIG, "LSB_SUB_WINDOW_SIG", jobSubReq);
	SET_PARM_STR (SUB_HOST_SPEC, "LSB_SUB_HOST_SPEC", jobSubReq, hostSpec);
	SET_PARM_STR (SUB_DEPEND_COND, "LSB_SUB_DEPEND_COND", jobSubReq,
				  dependCond);
	SET_PARM_STR (SUB_RES_REQ, "LSB_SUB_RES_REQ", jobSubReq, resReq);
	SET_PARM_STR (SUB_PRE_EXEC, "LSB_SUB_PRE_EXEC", jobSubReq, preExecCmd);
	SET_PARM_STR (SUB_LOGIN_SHELL, "LSB_SUB_LOGIN_SHELL", jobSubReq,
				  loginShell);
	SET_PARM_STR (SUB_MAIL_USER, "LSB_SUB_MAIL_USER", jobSubReq, mailUser);
	SET_PARM_BOOL (SUB_MODIFY, "LSB_SUB_MODIFY", jobSubReq);
	SET_PARM_BOOL (SUB_MODIFY_ONCE, "LSB_SUB_MODIFY_ONCE", jobSubReq);
	SET_PARM_STR (SUB_PROJECT_NAME, "LSB_SUB_PROJECT_NAME", jobSubReq,
				  projectName);
	SET_PARM_BOOL (SUB_INTERACTIVE, "LSB_SUB_INTERACTIVE", jobSubReq);
	SET_PARM_BOOL (SUB_PTY, "LSB_SUB_PTY", jobSubReq);
	SET_PARM_BOOL (SUB_PTY_SHELL, "LSB_SUB_PTY_SHELL", jobSubReq);

	SET_PARM_BOOL_2 (SUB2_HOLD, "LSB_SUB_HOLD", jobSubReq);
	SET_PARM_INT_2 (SUB2_JOB_PRIORITY, "LSB_SUB2_JOB_PRIORITY", jobSubReq,
					userPriority);
	SET_PARM_STR_2 (SUB2_IN_FILE_SPOOL, "LSB_SUB2_IN_FILE_SPOOL", jobSubReq,
					inFile);
	SET_PARM_BOOL_2 (SUB2_JOB_CMD_SPOOL, "LSB_SUB2_JOB_CMD_SPOOL", jobSubReq);

	ls_readconfenv (myParams, NULL);


	if (myParams[LSB_SUB_COMMANDNAME].paramValue)
		{
		unsigned long tmpCnt = 0;
		unsigned long start = 0;
		unsigned long cmdNameSize = 0;
		unsigned long cmdSize = 0;

		cmdSize = strlen (jobSubReq->command);
		cmdNameSize = 0;

		if (strstr (jobSubReq->command, SCRIPT_WORD) != NULL)
			{
			char *p = NULL;
			char *q = NULL;
			int found = 0;
			unsigned long ii = 0;


			while (jobSubReq->command[ii] != '\n') {
				ii++;
			}

			start = ii + 1;;
			cmdNameSize = ii + 1;

			do
				{

				found = 0;


				while ((cmdNameSize < cmdSize) && (isspace (jobSubReq->command[cmdNameSize]) || (jobSubReq->command[cmdNameSize] == '\n')))
					{
					start++;
					cmdNameSize++;
					found = 1;
					}


				while ((cmdNameSize < cmdSize) && (jobSubReq->command[start] == '#'))
					{
					while ((cmdNameSize < cmdSize) &&  (jobSubReq->command[start] != '\n'))
						{
						start++;
						cmdNameSize++;
						}
					found = 1;
					}
				}
			while (found);


			p = strstr (&jobSubReq->command[start], SCRIPT_WORD_END);
			q = &jobSubReq->command[start];
			while (q != p)
				{
				q++;

				if (*q == '\n')
					break;
				}

			if (q == p)
				start = cmdNameSize = cmdSize;
			}


		while (cmdNameSize < cmdSize)
			{
			if (isspace (jobSubReq->command[cmdNameSize]))
				break;
			cmdNameSize++;
			}


		fprintf (parmfp, "%s=\"", "LSB_SUB_COMMANDNAME");
		for (tmpCnt = start; tmpCnt < cmdNameSize; tmpCnt++)
			{
			if (jobSubReq->command[tmpCnt] == '"' ||  jobSubReq->command[tmpCnt] == '\\')
				fprintf (parmfp, "\\");

			fprintf (parmfp, "%c", jobSubReq->command[tmpCnt]);
			}
		fprintf (parmfp, "\"\n");


		do
			{
			char *x = unwrapCommandLine (jobSubReq->command);
			fprintf (parmfp, "LSB_SUB_COMMAND_LINE=\"%s\"\n", x);
			}
		while (0);
		}

	if (jobSubReq->options & SUB_HOST)
		{
		char askedHosts[MAX_LINE_LEN];

		askedHosts[0] = '\0';
		for (unsigned int i = 0; i < jobSubReq->numAskedHosts; i++)
			{
			strcat (askedHosts, jobSubReq->askedHosts[i]);
			strcat (askedHosts, " ");
			}

		if (askedHosts[0] != '\0')
			{
			fprintf (parmfp, "LSB_SUB_HOSTS=\"%s\"\n", askedHosts);
			}
		}
	else if (jobSubReq->delOptions & SUB_HOST)
		{
		fprintf (parmfp, "LSB_SUB_HOSTS=SUB_RESET\n");
		}

	for (unsigned int i = 0; i < LSF_RLIM_NLIMITS; i++)
		{
		SET_PARM_NUMBER (subRLimitName[i], jobSubReq->rLimits[i],
						 DELETE_NUMBER, DEFAULT_RLIMIT);

		}

	SET_PARM_NUMBER ("LSB_SUB_NUM_PROCESSORS", (jobSubReq->numProcessors ? jobSubReq->numProcessors : DEFAULT_NUMPRO), DEL_NUMPRO, DEFAULT_NUMPRO);
	SET_PARM_NUMBER ("LSB_SUB_MAX_NUM_PROCESSORS", (jobSubReq->maxNumProcessors ? jobSubReq->maxNumProcessors : DEFAULT_NUMPRO), DEL_NUMPRO, DEFAULT_NUMPRO);
	SET_PARM_NUMBER ("LSB_SUB_BEGIN_TIME", jobSubReq->beginTime, DELETE_NUMBER, 0);
	SET_PARM_NUMBER ("LSB_SUB_TERM_TIME", jobSubReq->termTime, DELETE_NUMBER, 0);

	if (jobSubReq->delOptions & SUB_OTHER_FILES)
		{
		fprintf (parmfp, "LSB_SUB_OTHER_FILES=SUB_RESET\n");
		}
	else if (jobSubReq->options & SUB_OTHER_FILES)
		{
		char str[MAX_LINE_LEN];

		fprintf (parmfp, "LSB_SUB_OTHER_FILES=%d\n", jobSubReq->nxf);

		for (unsigned int i = 0; i < jobSubReq->nxf; i++)
			{
			sprintf (str, "%s ", jobSubReq->xf[i].subFn);

			if (jobSubReq->xf[i].options & XF_OP_SUB2EXEC) {
				strcat (str, ">");
			}
			else if (jobSubReq->xf[i].options & XF_OP_SUB2EXEC_APPEND) {
				strcat (str, ">>");
			}
			if (jobSubReq->xf[i].options & XF_OP_EXEC2SUB_APPEND) {
				strcat (str, "<<");
			}
			else if (jobSubReq->xf[i].options & XF_OP_EXEC2SUB) {
				strcat (str, "<");
			}

			sprintf (str, "%s %s", str, jobSubReq->xf[i].execFn);
			fprintf (parmfp, "LSB_SUB_OTHER_FILES_%d=\"%s\"\n", i, str);
			}
		}

	if (additionEsubInfo != NULL)
		{
		fprintf (parmfp, "LSB_SUB_ADDITIONAL=\"%s\"\n", additionEsubInfo);
		}

	fclose (parmfp);

	putEnv ("LSB_SUB_ABORT_VALUE", "97");
	putEnv ("LSB_SUB_PARM_FILE", parmFile);

	if ((cc = runEsub_ (ed_, NULL)) < 0)
		{
		if (logclass & LC_TRACE) {
			ls_syslog (LOG_DEBUG, "%s: runEsub_() failed %d: %M", __func__, cc);
		}
		if (cc == -2)
			{
			char *deltaFileName = NULL;
			struct stat stbuf;

			lsberrno = LSBE_ESUB_ABORT;
			unlink (parmFile);


			if ((deltaFileName = getenv ("LSB_SUB_MODIFY_FILE")) != NULL)
				{
				if (stat (deltaFileName, &stbuf) != ENOENT) {
					unlink (deltaFileName);
				}
				}

			deltaFileName = NULL;
			if ((deltaFileName = getenv ("LSB_SUB_MODIFY_ENVFILE")) != NULL)
				{
				if (stat (deltaFileName, &stbuf) != ENOENT) {
					unlink (deltaFileName);
				}
				}
			return -1;
			}
		}

	unlink (parmFile);

	return 0;

}


int
readOptFile (char *filename, char *childLine)
{
	char *p     = NULL;
	char *sp    = NULL;
	char *sline = NULL;
	char *start = NULL;
	unsigned long lineLen = 0;
	FILE *fp;

	if ((fp = fopen (filename, "r")) == NULL)
		{
		ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, "readOptFile", "fopen", filename);
		return -1;
		}
	lineLen = 0;

	while ((sline = getNextLine_ (fp, FALSE)) != NULL)
		{
		sp = sline;
		SKIPSPACE (sp);

		if (*sp != '#') {
			continue;
		}
		if ((p = strstr (sp, "\\n")) != NULL) {
			*p = '\0';
		}
		++sp;
		SKIPSPACE (sp);

		if (strncmp (sp, "BSUB", 4) == 0)
			{
			sp += 4;
			start = sp;
			SKIPSPACE (sp);
			if (*sp != '-') {
				continue;
			}
			else
				{
				if ((sp = strstr (sp, "#")) != NULL) {
					*sp = '\0';
				}
				sp = sline;
				lineLen = strlen (sp);
				if (childLine[0] == '\0') {
					strcpy (childLine, sline);
				}
				else {
					strncat (childLine, start, strlen (start));
				}
				continue;
				}
			}
		else {
			continue;
		}
		}

	fclose (fp);
	return 0;
}

int
bExceptionTabInit (void)
{
	bExceptionTab = calloc (1, sizeof (hTab));
	h_initTab_ (bExceptionTab, 3);

	return 0;
}

static bException_t *
bExceptionCreate (void)
{
	bException_t *exception;

	exception = calloc (1, sizeof (bException_t));

	return exception;
}

int
mySubUsage_ (void *args)
{
	struct args
	{
		int argc;
		char padding[4];
		char **argv;
	} *myArgs;
	myArgs = (struct args *) args;
	subUsage_ (myArgs->argc, myArgs->argv);
	return 0;
}

void
lsb_throw (const char *exceptionName, void *extra)
{
	bException_t *exception;
	hEnt *hEnt;

	if (!exceptionName || *exceptionName == '\0') {
		return;
	}

	if (!bExceptionTab) {
		return;
	}

	hEnt = h_getEnt_ (bExceptionTab, (char *) exceptionName);

	if (!hEnt) {
		return;
	}

	exception = (bException_t *) hEnt->hData;

	if (!exception || !exception->handler) {
		return;
	}
	else
		(*exception->handler) (extra);

}

int
lsb_catch (const char *exceptionName, int (*exceptionHandler) (void *))
{
	hEnt *hEnt;
	bException_t *exception;

	if (!exceptionName || *exceptionName == '\0')
		{
		lsberrno = LSBE_BAD_ARG;
		return -1;
		}

	if (!bExceptionTab)
		{
		lsberrno = LSBE_LSLIB;
		return -1;
		}

	hEnt = h_getEnt_ (bExceptionTab, (char *) exceptionName);
	if (!hEnt)
		{
		hEnt = h_addEnt_ (bExceptionTab, (char *) exceptionName, NULL);
		hEnt->hData = bExceptionCreate ();
		}

	exception = hEnt->hData;
	if (!exception->name)
		exception->name = strdup (exceptionName);
	exception->handler = exceptionHandler;

	return 0;
}


// #define I18N_MSG(msgid,msg) _i18n_msg_get(ls_catd,NL_SETN,msgid,msg)

void makeCleanToRunEsub ( void )
{
	char parmDeltaFile[MAX_PATH_LEN];
	char envDeltaFile[MAX_PATH_LEN];
	struct stat stbuf;

	sprintf (parmDeltaFile, "%s/.lsbsubdeltaparm.%d.%d", LSTMPDIR, (int) getpid (), (int) getuid ());
	sprintf (envDeltaFile, "%s/.lsbsubdeltaenv.%d.%d",   LSTMPDIR, (int) getpid (), (int) getuid ());

	if (stat (parmDeltaFile, &stbuf) != ENOENT)
		{
		unlink (parmDeltaFile);
		}

	if (stat (envDeltaFile, &stbuf) != ENOENT)
		{
		unlink (envDeltaFile);
		}

	putEnv ("LSB_SUB_MODIFY_FILE", parmDeltaFile);
	putEnv ("LSB_SUB_MODIFY_ENVFILE", envDeltaFile);
}

void
modifyJobInformation (struct submit *jobSubReq)
{
	char parmDeltaFile[MAX_PATH_LEN];
	char envDeltaFile[MAX_PATH_LEN];
	int validKey = 0;
	int v = 0;
	char *sValue = NULL;

#define INTPARM   1
#define STRPARM   2
#define NUMPARM   3
#define BOOLPARM  4
#define INT2PARM  5
#define STR2PARM  6
#define BOOL2PARM 7
#define RLIMPARM  8
#define STRSPARM  9

#define FIELD_OFFSET(type,field) (long)(&(((struct type *)0)->field))
#define FIELD_PTR_PTR(base,offset) (((char *)base)+offset)

	FILE *fp;
	size_t lineNum = 0;
	char *line = NULL;
	char *key  = NULL;
	static struct
	{
		char *parmName;
		int parmType;
		char padding[4];
		long fieldOffset;
		long subOption;
	} jobSubReqParams[] =
		{
		{ "LSB_SUB_JOB_NAME",           STRPARM,   "    ", FIELD_OFFSET (submit, jobName), SUB_JOB_NAME },
		{ "LSB_SUB_QUEUE",              STRPARM,   "    ", FIELD_OFFSET (submit, queue),   SUB_QUEUE },
		{ "LSB_SUB_IN_FILE",            STRPARM,   "    ", FIELD_OFFSET (submit, inFile),  SUB_IN_FILE },
		{ "LSB_SUB_OUT_FILE",           STRPARM,   "    ", FIELD_OFFSET (submit, outFile), SUB_OUT_FILE },
		{ "LSB_SUB_ERR_FILE",           STRPARM,   "    ", FIELD_OFFSET (submit, errFile), SUB_ERR_FILE },
		{ "LSB_SUB_EXCLUSIVE",          BOOLPARM,  "    ", -1, SUB_EXCLUSIVE },
		{ "LSB_SUB_NOTIFY_END",         BOOLPARM,  "    ", -1, SUB_NOTIFY_END },
		{ "LSB_SUB_OUT_NOTIFY_BEGIN",   BOOLPARM,  "    ", -1, SUB_NOTIFY_BEGIN },
		{ "LSB_SUB_CHKPNT_PERIOD",      INTPARM,   "    ", FIELD_OFFSET (submit, chkpntPeriod), SUB_CHKPNT_PERIOD },
		{ "LSB_SUB_CHKPNT_DIR",         STRPARM,   "    ", FIELD_OFFSET (submit, chkpntDir),    SUB_CHKPNT_DIR },
		{ "LSB_SUB_RESTART_FORCE",      BOOLPARM,  "    ", -1, SUB_RESTART_FORCE },
		{ "LSB_SUB_RESTART",            BOOLPARM,  "    ", -1, SUB_RESTART },
		{ "LSB_SUB_RERUNNABLE",         BOOLPARM,  "    ", -1, SUB_RERUNNABLE },
		{ "LSB_SUB_WINDOW_SIG",         BOOLPARM,  "    ", -1, SUB_WINDOW_SIG },
		{ "LSB_SUB_HOST_SPEC",          STRPARM,   "    ", FIELD_OFFSET (submit, hostSpec),   SUB_HOST_SPEC },
		{ "LSB_SUB_DEPEND_COND",        STRPARM,   "    ", FIELD_OFFSET (submit, dependCond), SUB_DEPEND_COND },
		{ "LSB_SUB_RES_REQ",            STRPARM,   "    ", FIELD_OFFSET (submit, resReq),     SUB_RES_REQ },
		{ "LSB_SUB_PRE_EXEC",           STRPARM,   "    ", FIELD_OFFSET (submit, preExecCmd), SUB_PRE_EXEC },
		{ "LSB_SUB_LOGIN_SHELL",        STRPARM,   "    ", FIELD_OFFSET (submit, loginShell), SUB_LOGIN_SHELL },
		{ "LSB_SUB_MAIL_USER",          STRPARM,   "    ", FIELD_OFFSET (submit, mailUser),   SUB_MAIL_USER },
		{ "LSB_SUB_MODIFY",             BOOLPARM,  "    ", -1, SUB_MODIFY },
		{ "LSB_SUB_MODIFY_ONCE",        BOOLPARM,  "    ", -1, SUB_MODIFY_ONCE },
		{ "LSB_SUB_PROJECT_NAME",       STRPARM,   "    ", FIELD_OFFSET (submit, projectName), SUB_PROJECT_NAME },
		{ "LSB_SUB_INTERACTIVE",        BOOLPARM,  "    ", -1, SUB_INTERACTIVE },
		{ "LSB_SUB_PTY",                BOOLPARM,  "    ", -1, SUB_PTY },
		{ "LSB_SUB_PTY_SHELL",          BOOLPARM,  "    ", -1, SUB_PTY_SHELL },
		{ "LSB_SUB_HOSTS",              STRSPARM,  "    ", FIELD_OFFSET (submit, askedHosts), 0 },
		{ "LSB_SUB_HOLD",               BOOL2PARM, "    ", -1, SUB2_HOLD },
		{ "LSB_SUB2_JOB_PRIORITY",      INT2PARM,  "    ", FIELD_OFFSET (submit, userPriority), SUB2_JOB_PRIORITY },
		{ "LSB_SUB_RLIMIT_CPU",         RLIMPARM,  "    ", 0, -1 },
		{ "LSB_SUB_RLIMIT_FSIZE",       RLIMPARM,  "    ", 1, -1 },
		{ "LSB_SUB_RLIMIT_DATA",        RLIMPARM,  "    ", 2, -1 },
		{ "LSB_SUB_RLIMIT_STACK",       RLIMPARM,  "    ", 3, -1 },
		{ "LSB_SUB_RLIMIT_CORE",        RLIMPARM,  "    ", 4, -1 },
		{ "LSB_SUB_RLIMIT_RSS",         RLIMPARM,  "    ", 5, -1 },
		{ "LSB_SUB_RLIMIT_NOFILE",      RLIMPARM,  "    ", 6, -1 },
		{ "LSB_SUB_RLIMIT_OPEN_MAX",    RLIMPARM,  "    ", 7, -1 },
		{ "LSB_SUB_RLIMIT_SWAP",        RLIMPARM,  "    ", 8, -1 },
		{ "LSB_SUB_RLIMIT_RUN",         RLIMPARM,  "    ", 9, -1 },
		{ "LSB_SUB_RLIMIT_PROCESS",     RLIMPARM,  "    ", 10, -1 },
		{ "LSB_SUB_NUM_PROCESSORS",     NUMPARM,   "    ", FIELD_OFFSET (submit, numProcessors), DEFAULT_NUMPRO },
		{ "LSB_SUB_MAX_NUM_PROCESSORS", NUMPARM,   "    ", FIELD_OFFSET (submit, maxNumProcessors), DEFAULT_NUMPRO },
		{ "LSB_SUB_BEGIN_TIME",         NUMPARM,   "    ", FIELD_OFFSET (submit, beginTime), 0 },
		{ "LSB_SUB_TERM_TIME",          NUMPARM,   "    ", FIELD_OFFSET (submit, termTime), 0 },
		{ "LSB_SUB_COMMAND_LINE",       STRPARM,   "    ", FIELD_OFFSET (submit, command), 0 },
		{ NULL, 0, "    ", 0, 0}
	};

	sprintf (parmDeltaFile, "%s/.lsbsubdeltaparm.%d.%d", LSTMPDIR, getpid (), getuid ());
	sprintf (envDeltaFile, "%s/.lsbsubdeltaenv.%d.%d",   LSTMPDIR, getpid (), getuid ());

	if (access (parmDeltaFile, R_OK) == F_OK)
		{
		fp = fopen (parmDeltaFile, "r");
		lineNum = 0;

		while ((line = getNextLineC_ (fp, &lineNum, TRUE)) != NULL)
			{
			int i = 0;
			long j = 0;

			key = getNextWordSet (&line, " \t=!@#$%^&*()");

			while (*line != '=') {
				line++;
			}
			line++;
			while (isspace ((int) *line)) {
				line++;
			}

			validKey = 0;


			if (strncmp (key, "LSB_SUB_OTHER_FILES", strlen ("LSB_SUB_OTHER_FILES")) == 0)
				{
				processXFReq (key, line, jobSubReq);
				continue;
				}

			for (i = 0; jobSubReqParams[i].parmName; i++)
				{
				if (strcmp (key, jobSubReqParams[i].parmName) == 0)
					{
					validKey = 1;

					switch (jobSubReqParams[i].parmType)
						{
							case STRPARM:
							if (checkEmptyString (line))
								{
								/* catgets 5557 */
								ls_syslog (LOG_WARNING, "catgets 5557: %s: The value of parameter %s is empty, the setting will be ignored.", __func__, key);
								break;
								}

							if ((strcmp (key, "LSB_SUB_COMMAND_LINE") == 0) &&
								(jobSubReq->options & SUB_RESTART))
								{

								break;
								}

							if (stringIsToken (line, "SUB_RESET"))
								{
								jobSubReq->options &= ~jobSubReqParams[i].subOption;
								jobSubReq->delOptions |=
								jobSubReqParams[i].subOption;
								}
							else
								{

								sValue = extractStringValue (line);
								if (sValue == NULL)
									{
									/* catgets 5556 */
									ls_syslog (LOG_WARNING, "catgets 5556: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored.", __func__, line, key);
									break;
									}

								if (strcmp (key, "LSB_SUB_COMMAND_LINE") != 0)
									{
									*(char **) (FIELD_PTR_PTR (jobSubReq, jobSubReqParams[i].fieldOffset)) = putstr_ (sValue); // FIXME FIXME FIXME
									}
								else
									{
									*(char **) (FIELD_PTR_PTR (jobSubReq, jobSubReqParams[i].fieldOffset)) = wrapCommandLine (sValue); // FIXME FIXME FIXME
									if (jobSubReq->options & SUB_MODIFY)
										{
										jobSubReq->newCommand = jobSubReq->command;
										jobSubReq->options2 |= SUB2_MODIFY_CMD;
										jobSubReq->delOptions2 &= ~SUB2_MODIFY_CMD;
										}
									}

								jobSubReq->options |= jobSubReqParams[i].subOption;
								jobSubReq->delOptions &=
								~jobSubReqParams[i].subOption;
								}
							break;
							case INTPARM:
							if (stringIsToken (line, "SUB_RESET"))
								{
								jobSubReq->options &= ~jobSubReqParams[i].subOption;
								jobSubReq->delOptions |=
								jobSubReqParams[i].subOption;
								}
							else
								{
								if (!stringIsDigitNumber (line))
									{
									/* catgets 5551 */
									ls_syslog (LOG_WARNING, "catgets 5551: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored. The value of this parameter can  only be SUB_RESET or an integer.", __func__, line, key);
									break;
									}

								v = atoi (line);
								*(FIELD_PTR_PTR (jobSubReq, jobSubReqParams[i].fieldOffset)) = (char) v; // FIXME FIXME FIXME
								jobSubReq->options |= jobSubReqParams[i].subOption;
								jobSubReq->delOptions &= ~jobSubReqParams[i].subOption;
								}
							break;
							case BOOLPARM:
							if (stringIsToken (line, "Y"))
								{
								jobSubReq->options |= jobSubReqParams[i].subOption;
								jobSubReq->delOptions &= ~jobSubReqParams[i].subOption;
								}
							else if (stringIsToken (line, "SUB_RESET"))
								{
								jobSubReq->options &= ~jobSubReqParams[i].subOption;
								jobSubReq->delOptions |= jobSubReqParams[i].subOption;
								}
							else
								{
								/* catgets 5554 */
								ls_syslog (LOG_WARNING, "catgets 5554: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored. The value of this parameter can only be SUB_RESET or 'Y'.", __func__, line, key);
								}
							break;
							case STR2PARM:
							if (checkEmptyString (line))
								{
								/* catgets 5557 */
								ls_syslog (LOG_WARNING, "catgets 5557: %s: The value of parameter %s is empty, the setting will be ignored.", __func__, key);
								break;
								}

							if (stringIsToken (line, "SUB_RESET"))
								{
								jobSubReq->options2 &= ~jobSubReqParams[i].subOption;
								jobSubReq->delOptions2 |= jobSubReqParams[i].subOption;
								}
							else
								{

								sValue = extractStringValue (line);
								if (sValue == NULL)
									{
									/* catgets 5556 */
									ls_syslog (LOG_WARNING, "catgets 5556: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored.", __func__, line, key);
									break;
									}

								*(char **) (FIELD_PTR_PTR (jobSubReq, jobSubReqParams[i].fieldOffset)) = putstr_ (sValue); // FIXME FIXME FIXME

								jobSubReq->options2 |= jobSubReqParams[i].subOption;
								jobSubReq->delOptions2 &= ~jobSubReqParams[i].subOption;
								}
							break;
							case INT2PARM:
							if (stringIsToken (line, "SUB_RESET"))
								{
								jobSubReq->options2 &= ~jobSubReqParams[i].subOption;
								jobSubReq->delOptions2 |= jobSubReqParams[i].subOption;
								}
							else
								{
								if (!stringIsDigitNumber (line))
									{
									/* catgets 5557 ? */
									// ls_syslog (LOG_WARNING, _i18n_msg_get(ls_catd,NL_SETN,msgid,msg), __func__, line, key);
									break;
									}

								v = atoi (line);
								*(FIELD_PTR_PTR (jobSubReq, jobSubReqParams[i].fieldOffset)) = (char)v; // FIXME FIXME FIXME 
								jobSubReq->options2 |= jobSubReqParams[i].subOption;
								jobSubReq->delOptions2 &= ~jobSubReqParams[i].subOption;
								}
							break;
							case BOOL2PARM:
							if (stringIsToken (line, "Y"))
								{
								jobSubReq->options2 |= jobSubReqParams[i].subOption;
								jobSubReq->delOptions2 &= ~jobSubReqParams[i].subOption;
								}
							else if (stringIsToken (line, "SUB_RESET"))
								{
								jobSubReq->options2 &= ~jobSubReqParams[i].subOption;
								jobSubReq->delOptions2 |= jobSubReqParams[i].subOption;
								}
							else
								{
								/* catgets 5554 */
								ls_syslog (LOG_WARNING, "catgets 5554: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored. The value of this parameter can only be SUB_RESET or 'Y'.", __func__, line, key);
								}
							break;
							case NUMPARM:
							if (stringIsToken (line, "SUB_RESET"))
								{
								assert (jobSubReqParams[i].subOption <= INT_MAX);
								*(FIELD_PTR_PTR (jobSubReq, jobSubReqParams[i].fieldOffset)) = (char) jobSubReqParams[i].subOption; // FIXME FIXME FIXME
								}
							else
								{
								if (!stringIsDigitNumber (line))
									{
									/* catgets 5557 ? */
									// ls_syslog (LOG_WARNING, _i18n_msg_get(ls_catd,NL_SETN,msgid,msg), __func__, line, key);
									break;
									}

								v = atoi (line);
								*(FIELD_PTR_PTR (jobSubReq, jobSubReqParams[i].fieldOffset)) = (char) v; // FIXME FIXME FIXME
								}

							if (jobSubReq->maxNumProcessors < jobSubReq->numProcessors)
								{
								jobSubReq->maxNumProcessors = jobSubReq->numProcessors;
								}

							break;
							case RLIMPARM:
							j = jobSubReqParams[i].fieldOffset;
							if (stringIsToken (line, "SUB_RESET"))
								{
								jobSubReq->rLimits[j] = DELETE_NUMBER;
								}
							else
								{
								if (!stringIsDigitNumber (line))
									{
									/* catgets 5557 ? */                                       
									// ls_syslog (LOG_WARNING, _i18n_msg_get(ls_catd,NL_SETN,msgid,msg), __func__, line, key);
									break;
									}

								v = atoi (line);
								jobSubReq->rLimits[j] = v;
								}
							break;
							case STRSPARM:
							if (checkEmptyString (line))
								{
								/* catgets 5557 */
								ls_syslog (LOG_WARNING, "catgets 5557: %s: The value of parameter %s is empty, the setting will be ignored.", __func__, key);
								break;
								}

							sValue = extractStringValue (line);
							if (sValue == NULL)
								{
								/* catgets 5556 */
								ls_syslog (LOG_WARNING, "catgets 5556: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored.", __func__, line, key);
								break;
								}

							if (strcmp (key, "LSB_SUB_HOSTS") == 0)
								{
								unsigned long *badIdx = 0;
								unsigned int numAskedHosts = 0;

								assert( jobSubReq->numAskedHosts <= UINT_MAX );
								numAskedHosts = jobSubReq->numAskedHosts;

								if (getAskedHosts_ (sValue, &jobSubReq->askedHosts, &numAskedHosts, badIdx, FALSE) < 0)
									{
									jobSubReq->options &= ~SUB_HOST;
									ls_syslog (LOG_WARNING, ls_sysmsg ());
									}
								else
									{
									jobSubReq->options |= SUB_HOST;
									}
								}
							break;
							default:
							/* catgets 55550 */
							ls_syslog (LOG_WARNING, "catgets 5550: %s: Bad parameter variable name(%s) read from $LSB_SUB_MODIFY_FILE, the setting will be ignored.", __func__, key);
							break;
						}
					break;
					}
				}

			if (!validKey)
				{
				/* catgets 5550 */    
				ls_syslog (LOG_WARNING, "catgets 5550: %s: Bad parameter variable name(%s) read from $LSB_SUB_MODIFY_FILE, the setting will be ignored.", __func__, key);
				}
			}
		fclose (fp);

		unlink (parmDeltaFile);
		}

	compactXFReq (jobSubReq);

	if (access (envDeltaFile, R_OK) == F_OK)
		{
		size_t linenum = 0;
		// lineNum = 0;
		fp = fopen (envDeltaFile, "r");

		while ((line = getNextLineC_ (fp, &linenum, TRUE)) != NULL)
			{

			key = getNextWordSet (&line, " \t =!@#$%^&*()");
			while (*line != '=') {
				line++;
			}

			line++;

			putEnv (key, getNextValueQ_ (&line, '"', '"'));
			}
		fclose (fp);
		unlink (envDeltaFile);
		}
}

char *
unwrapCommandLine (char *commandLine)
{
	static char *jobDespBuf = NULL;
	static char *lineStrBuf = NULL;
	char *jobdesp = NULL;
	char *sp = NULL;
	char *p1 = NULL;
	char *p2 = NULL;
	int hasNonSpaceC = 0;

	FREEUP (lineStrBuf);
	FREEUP (jobDespBuf);
	lineStrBuf = putstr_ (commandLine);
	if (!lineStrBuf)
		{
		lsberrno = LSBE_NO_MEM;
		return NULL;
		}
	jobdesp = lineStrBuf;
	sp = (char *) strstr (jobdesp, "SCRIPT_\n");
	if (sp == NULL)
		{
		jobDespBuf = putstr_ (jobdesp);
		return &jobDespBuf[0];
		}

	jobdesp = sp + strlen ("SCRIPT_\n");
	sp = NULL;
	sp = strstr (jobdesp, "SCRIPT_\n");
	if (sp == NULL)
		{
		jobDespBuf = putstr_ (jobdesp);
		return &jobDespBuf[0];
		}
	while (*sp != '\n') {
		sp--;
	}
	sp++;
	*sp = '\0';

	jobDespBuf = putstr_ (jobdesp);
	p1 = NULL;
	p2 = jobDespBuf;
	hasNonSpaceC = 0;
	while (*p2)
		{
		if (*p2 == '\n')
			{
			*p2 = ' ';
			if (p1 != NULL)
				{
				if (hasNonSpaceC)
					{
					*p1 = ';';
					}
				else
					*p1 = ' ';
				}
			p1 = p2;
			hasNonSpaceC = 0;
			}
		else if (!isspace ((int) *p2))
			hasNonSpaceC = 1;
		p2++;
		}
	return &jobDespBuf[0];
}

char *
wrapCommandLine (char *command)
{
	static char *szTmpShellCommands = "\n_USER_SCRIPT_\n) "
	"> $LSB_CHKFILENAME.shell\n"
	"chmod u+x $LSB_CHKFILENAME.shell\n"
	"$LSB_JOBFILENAME.shell\n"
	"saveExit=$?\n"
	"/bin/rm -f $LSB_JOBFILENAME.shell\n" "(exit $saveExit)\n";

	static char cmdString[MAX_LINE_LEN * 4];

	if (strchr (command, (int) '\n') == NULL)
		{
		strcpy (cmdString, command);
		return cmdString;
		}

	sprintf (cmdString, "(cat <<_USER_\\SCRIPT_\n%s\n%s", command, szTmpShellCommands);
	return cmdString;
}


void
compactXFReq (struct submit *jobSubReq)
{
	unsigned int i = 0;
	unsigned int j = 0;

	while (i < jobSubReq->nxf)
		{
		if (jobSubReq->xf[i].options != 0)
			{
			if (i != j)
				{
				memcpy (&(jobSubReq->xf[j]), &(jobSubReq->xf[i]), sizeof (struct xFile));
				}
			j++;
			}
		i++;
		}
	jobSubReq->nxf = j;
}

int
checkEmptyString (char *s)
{
	char *p = s;

	while (*p)
		{
		if (!isspace ((int) *p)) {
			return 0;
		}
		p++;
		}

	return 1;
}

int
stringIsToken (char *s, char *tok)
{
	char *s1 = s;

	while (isspace ((int) *s1) && (*s1)) {
		s1++;
	}

	if (strncmp (s1, tok, strlen (tok)) == 0)
		{
		char *p = s1 + strlen (tok);
		return checkEmptyString (p);
		}

	return 0;
}

int
stringIsDigitNumber (char *s)
{
	char *s1 = s;

	while (isspace ((int) *s1) && (*s1)) {
		s1++;
	}

	if (*s1 == 0x0) {
		return 0;
	}

	if (*s1 == '0')
		{
		return checkEmptyString (s1 + 1);
		}

	while (isdigit ((int) *s1)) {
		s1++;
	}

	return checkEmptyString (s1);
}

char *
extractStringValue (char *line)
{
	char *p;
	static char sValue[MAX_LINE_LEN];
	int i;

	p = line;
	while (isspace ((int) *p) && (*p != 0x0)) {
		p++;
	}

	if (*p != '\"')
		{

		return NULL;
		}

	p++;
	i = 0;
	while ((*p != 0x0) && (*p != '\"'))
		{
		sValue[i] = *p;
		p++;
		i++;
		}

	if (*p == 0) {
		return NULL;
	}

	p++;
	if (checkEmptyString (p))
		{
		sValue[i] = 0x0;
		return sValue;
		}

	return NULL;
}

int
processXFReq (char *key, char *line, struct submit *jobSubReq)
{
	int validKey = 0;
	unsigned int v = 0;
	char *sValue = NULL;

	if (strcmp (key, "LSB_SUB_OTHER_FILES") == 0)
		{
		validKey = 1;

		if (stringIsToken (line, "SUB_RESET"))
			{
			free (jobSubReq->xf);
			jobSubReq->xf = NULL;
			jobSubReq->options &= ~SUB_OTHER_FILES;
			jobSubReq->delOptions |= SUB_OTHER_FILES;
			jobSubReq->nxf = 0;
			}
		else
			{
			struct xFile *p;

			if (!stringIsDigitNumber (line))
				{
				/* catgets ? */
				// ls_syslog (LOG_WARNING, _i18n_msg_get(ls_catd,NL_SETN,msgid,msg), __func__, line, key);
				return -1;
				}

			assert( atoi( line ) >= 0 );
			v = (unsigned int) atoi (line);
			p = (struct xFile *) malloc (v * sizeof (struct xFile));

			if (p == NULL)
				{
				/* catgets 5552*/
				ls_syslog (LOG_ERR, "catgets 5552: %s: Memory allocate failed for file transfer request.", __func__);
				return -1;
				}
			else
				{
				free (jobSubReq->xf);
				jobSubReq->nxf = v;
				jobSubReq->options |= SUB_OTHER_FILES;
				jobSubReq->delOptions &= SUB_OTHER_FILES;
				jobSubReq->xf = p;
				memset (jobSubReq->xf, 0x0, v * sizeof (struct xFile));
				}
			}
		}
	else
		{

		char *sp = key + strlen ("LSB_SUB_OTHER_FILES_");
		char xfSeq[32];

		xfSeq[0] = 0x0;
		strncat (xfSeq, sp, sizeof (xfSeq) - 2);

		if (!stringIsDigitNumber (xfSeq))
			{
			/* catgets 5550 */
			ls_syslog (LOG_WARNING, "catgets 5550: %s: Bad parameter variable name(%s) read from $LSB_SUB_MODIFY_FILE, the setting will be ignored.", __func__, key);
			return -1;
			}
		assert( atoi(xfSeq) >= 0 );
		v = (unsigned int)atoi (xfSeq);
		validKey = 1;
		if (v < jobSubReq->nxf)
			{
			char op[20];
			char *txt, *srcf;

			jobSubReq->xf[v].options = 0;

			sValue = extractStringValue (line);
			if (sValue == NULL)
				{
				/* catgets 5556 */
				ls_syslog (LOG_WARNING, "catgets 5556: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored.", __func__, line, sValue);
				return -1;
				}

			txt = sValue;
			while (isspace ((int) *txt) && (*txt != 0x0)) {
				txt++;
			}

			srcf = getNextWordSet (&txt, " \t<>");

			if (srcf == NULL)
				{
				/* catgets 5556 */
				ls_syslog (LOG_WARNING, "catgets 5556: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored.", __func__, line, sValue);
				return -1;
				}

			strcpy (jobSubReq->xf[v].subFn, srcf);
			while (isspace ((int) *txt) && (*txt != 0x0)) {
				txt++;
			}


			op[0] = op[1] = op[2] = '\000';
			op[0] = *txt++;
			if (!isspace ((int) *txt)) {
				op[1] = *txt++;
			}

			while (isspace ((int) *txt) && (*txt != 0x0)) {
				txt++;
			}
			strcpy (jobSubReq->xf[v].execFn, txt);

			if (strcmp (op, "<") == 0)
				{
				jobSubReq->xf[v].options |= XF_OP_EXEC2SUB;
				}
			else if (strcmp (op, "<<") == 0)
				{
				jobSubReq->xf[v].options |= XF_OP_EXEC2SUB_APPEND;
				jobSubReq->xf[v].options |= XF_OP_EXEC2SUB;
				}
			else if ((strcmp (op, "<>") == 0) || (strcmp (op, "><") == 0))
				{
				jobSubReq->xf[v].options |= XF_OP_EXEC2SUB;
				jobSubReq->xf[v].options |= XF_OP_SUB2EXEC;
				}
			else if (strcmp (op, ">") == 0)
				{
				jobSubReq->xf[v].options |= XF_OP_SUB2EXEC;
				}
			else if (strcmp (op, ">>") == 0)
				{
				jobSubReq->xf[v].options |= XF_OP_SUB2EXEC_APPEND;
				jobSubReq->xf[v].options |= XF_OP_SUB2EXEC;
				}
			else
				{
				/* catgets 5553 */
				ls_syslog (LOG_WARNING, "catgets 5553: %s: unknown file transfer operator %s, this transfer request will be ignored.", __func__, op);
				}
			}
		else
			{
			/* catgets 5556 */
			ls_syslog (LOG_WARNING, "catgets 5556: %s: Bad value(%s) read from $LSB_SUB_MODIFY_FILE for parameter %s, the setting will be ignored.", __func__, line, key);
			}
		}

	return 0;
}


void
trimSpaces (char *str)
{
	char *ptr;

	if (!str || str[0] == '\0')
		{
		return;
		}


	while (isspace ((int) *str))
		{
		for (ptr = str; *ptr; ptr++)
			{
			*ptr = ptr[1];
			}
		}


	ptr = str;
	while (*ptr)
		{
		ptr++;
		}

	ptr--;
	while (ptr >= str && isspace ((int) *ptr))
		{
		*ptr = '\0';
		ptr--;
		}
}
