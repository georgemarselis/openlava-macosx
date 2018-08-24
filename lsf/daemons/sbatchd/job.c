/*
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
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>

#include <dirent.h>
#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <strings.h>


#include "libint/jidx.h"
#include "lib/rcp.h"
#include "lib/mls.h"
#include "lib/misc.h"
#include "lib/lib.h"
#include "lib/syslog.h"
#include "lib/rcp.h"

#include "daemons/daemons.h"
#include "daemons/sbatchd/job.h"
#include "daemons/libsbatchd/sbatchd.h"
#include "libint/lsi18n.h"

/*************************************************************
 * build sbatchd: sbatchd.c job.c
 */


struct passwd *
my_getpwnam (const char *name, char *caller)
{
	int counter = 1;
	struct passwd *pw = NULL;

	while (((pw = getpwnam (name)) == NULL) && (counter < getpwnamRetry))
	{
		if (logclass & LC_EXEC) {
			ls_syslog (LOG_DEBUG1, "%s: getpwnam(%s) failed %d times:%m", caller, name, counter);
		}
		counter++;
		millisleep_ (1000);
	}
	return pw; 
}

void
sbdChildCloseChan (int exceptChan)
{
	struct clientNode *cliPtr = NULL;
	struct clientNode *nextClient = NULL;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG1, "sbdChildCloseChan: Entering...");
	}

	for (cliPtr = clientList->forw; cliPtr != clientList; cliPtr = nextClient)
	{
		nextClient = cliPtr->forw;

		if (cliPtr->chanfd != exceptChan) {
			shutDownClient (cliPtr);
		}
	}

	CLOSECD (statusChan);

	return;
}


sbdReplyType
job_exec (struct jobCard *jobCardPtr, int chfd)
{
	struct jobSpecs *jobSpecsPtr = NULL;
	pid_t pid = -1;

	jobSpecsPtr = &(jobCardPtr->jobSpecs);
	if (logclass & LC_EXEC)
	{
		ls_syslog (LOG_DEBUG, "%s: the Job's JobSpoolDir is %s \n", __func__, jobSpecsPtr->jobSpoolDir);
	}

	jobSpecsPtr->reasons = 0;
	jobSpecsPtr->subreasons = 0;

	pid = fork ();

	if (pid < 0) {
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jobSpecsPtr->jobId), "fork");
		return ERR_FORK_FAIL;
	}

	if (pid == 0)
	{
		closeBatchSocket ();
		sbdChildCloseChan (chfd);
		execJob (jobCardPtr, chfd);
		exit (-1);
	}

	jobSpecsPtr->jobPid = pid;

	if (jobSpecsPtr->options & SUB_RESTART) {
		jobSpecsPtr->jobPGid = 0;
	}
	else {
		jobSpecsPtr->jobPGid = pid;
	}

	jobCardPtr->missing = FALSE;
	jobCardPtr->notReported = 0;
	jobCardPtr->needReportRU = FALSE;

	return ERR_NO_ERROR;
}

int
sendNotification (struct jobCard *jobCardPtr) // FIXME FIXME FIXME replace sending individual emails for each job with, per run and per quartile finished
{
	struct jobSpecs *jobSpecsPtr = &(jobCardPtr->jobSpecs);
	FILE *mail = NULL;
	char hostname[MAXHOSTNAMELEN];
	char *temp1 = NULL;



	if (jobSpecsPtr->options & SUB_MAIL_USER) {
		mail = smail (jobSpecsPtr->mailUser, jobSpecsPtr->fromHost);
	}
	else {
		mail = smail (jobSpecsPtr->userName, jobSpecsPtr->fromHost);
	}


	if (gethostname (hostname, MAXHOSTNAMELEN) < 0) {
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jobSpecsPtr->jobId), "gethostname");
		STRNCPY (hostname, "localhost", sizeof (hostname));
	}
	/* catgets 400 */
	fprintf (mail, _i18n_msg_get (ls_catd, NL_SETN, 400, "Sender: LSF System <%s@%s>\n"), lsbManager, hostname);
	/* catgets 401 */
	fprintf (mail, _i18n_msg_get (ls_catd, NL_SETN, 401, "Subject: Job %s: <%s> started.\n"), lsb_jobid2str (jobSpecsPtr->jobId), jobSpecsPtr->jobName);

	fprintf (mail, "\n");
	/* catgets 402 */
	fprintf (mail, _i18n_msg_get (ls_catd, NL_SETN, 402, "Job <%s> was submitted from host <%s> by user <%s>. "), jobSpecsPtr->jobName, jobSpecsPtr->fromHost, jobSpecsPtr->userName);
	/* catgets 404 */
	temp1 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 404, "unknown time in the past"));
	/* catgets 405 */ // this is crazy: ls_catd is not used in _i18n_ctime, just need to be non-zero
	fprintf (mail, _i18n_msg_get (ls_catd, NL_SETN, 405, "Started at %sunder user <%s>, "), jobSpecsPtr->startTime ? _i18n_ctime ( 405, CTIME_FORMAT_DEFAULT, &jobSpecsPtr->startTime) : temp1, jobCardPtr->execUsername);
	free (temp1);

	/* catgets 407 */
	fprintf (mail, _i18n_msg_get (ls_catd, NL_SETN, 407, "on host(s) <%s>, while in queue <%s>.\n"),  hostname, jobSpecsPtr->queue);
	for( unsigned int k = 1; k < jobSpecsPtr->numToHosts; k++) {
		fprintf (mail, "                   <%s>\n", jobSpecsPtr->toHosts[k]);
	}
	now = time (0);

	fprintf (mail, _i18n_msg_get (ls_catd, NL_SETN, 408, "Notification reported at %s"), _i18n_ctime (408, CTIME_FORMAT_DEFAULT, &now));

	mclose (mail);
	return 0;
}

void
getJobTmpDir (char *tmpDirName, struct jobCard *jPtr)
{
	char jobId[16];

	memset( jobId, '\0', 16 );

	strcpy (tmpDirName, lsTmpDir_);
	strcat (tmpDirName, "/"); // FIXME FIXME replace with define
	sprintf (jobId, "%s", lsb_jobidinstr (jPtr->jobSpecs.jobId));
	strcat (tmpDirName, jobId);
	strcat (tmpDirName, ".tmpdir"); // FIXME FIXME FIXME

	return;
}

void
createJobTmpDir (struct jobCard *jobCardPtr) // FIXME FIXME FIXME clusterfuck of env vars.
{
	char tmpDirName[MAX_FILENAME_LEN];
	mode_t previousUmask;

	getJobTmpDir ( tmpDirName, jobCardPtr);
	previousUmask = umask (077);

	if ((mkdir (tmpDirName, 0700) == -1) && errno != EEXIST) {
		ls_syslog (LOG_ERR, "%s: Unable to create job tmp directory %s for job %s", __func__, tmpDirName, lsb_jobid2str (jobCardPtr->jobSpecs.jobId));
		tmpDirName[0] = 0;
	}

	umask (previousUmask);

	if (daemonParams[LSB_SET_TMPDIR].paramValue != NULL) {
		if (tmpDirName[0] != 0 && !strcasecmp (daemonParams[LSB_SET_TMPDIR].paramValue, "y")) {
			putEnv ("TMPDIR", tmpDirName); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}
	}

	return;
}

void
execJob (struct jobCard *jobCardPtr, int chfd)
{
	char *shellPath = NULL;
	sigset_t newmask;
	XDR xdrs;
	char buf[MSGSIZE];
	struct lenData jf;
	struct hostent *fromHp;
	struct jobSpecs *jobSpecsPtr = NULL;
	struct LSFHeader replyHdr;

	jobSpecsPtr = &(jobCardPtr->jobSpecs);
	jobSpecsPtr->jobPid = getpid ();
	jobSpecsPtr->jobPGid = jobSpecsPtr->jobPid;
	jobCardPtr->stdinFile = NULL;

	if (rcvJobFile (chfd, &jf) == -1) {
		ls_syslog (LOG_ERR, "%s: failed receiving job file job %s", __func__, lsb_jobid2str (jobSpecsPtr->jobId));
		jobSetupStatus (JOB_STAT_PEND, PEND_JOB_NO_FILE, jobCardPtr);
	}

	if (daemonParams[LSB_BSUBI_OLD].paramValue || !PURE_INTERACTIVE (jobSpecsPtr)) {

		xdrmem_create (&xdrs, buf, MSGSIZE, XDR_DECODE);
		if (readDecodeHdr_ (chfd, buf, chanRead_, &xdrs, &replyHdr) < 0) {
			ls_syslog (LOG_WARNING, "%s: Fail to get go-ahead from mbatchd; abort job %s", __func__, lsb_jobid2str (jobSpecsPtr->jobId));

			jobSetupStatus (JOB_STAT_PEND, PEND_JOB_START_FAIL, jobCardPtr);
		}

		xdr_destroy (&xdrs);
		chanClose_ (chfd);
	}

	ls_syslog (LOG_DEBUG, "%s: Got job start ok from mbatchd for job <%s>", __func__, lsb_jobid2str (jobSpecsPtr->jobId));

	if (acctMapTo (jobCardPtr) < 0) {
		jobSetupStatus (JOB_STAT_PEND, PEND_NO_MAPPING, jobCardPtr);
	}

	if ((jobCardPtr->jobSpecs.jobPGid = setPGid (jobCardPtr)) < 0) {
		jobSetupStatus (JOB_STAT_PEND, PEND_JOB_EXEC_INIT, jobCardPtr);
	}

	putEnv (LS_EXEC_T, "START"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	if (setJobEnv (jobCardPtr) < 0) {
		ls_syslog (LOG_DEBUG, "%s: setJobEnv() failed for job <%s>", __func__, lsb_jobid2str (jobSpecsPtr->jobId));
		jobSetupStatus (JOB_STAT_PEND, PEND_JOB_ENV, jobCardPtr);
	}

	umask (jobCardPtr->jobSpecs.umask);

	runQPre (jobCardPtr);
	nice (NICE_LEAST);

	if (!debug) {
		nice (NICE_MIDDLE);
	}
	nice (0);

	errno = 0;
	if (nice (jobCardPtr->jobSpecs.nice) == -1 && errno != 0)
	{
		ls_syslog (LOG_ERR, "%s: nice(%d) failed %m", __func__, jobSpecsPtr->nice);
	}

	if (setLimits (jobSpecsPtr) < 0) {
		jobSetupStatus (JOB_STAT_PEND, PEND_JOB_EXEC_INIT, jobCardPtr);
	}

	if (jobSpecsPtr->lsfLimits[LSF_RLIMIT_FSIZE].rlim_maxl != 0xffffffff
		|| jobSpecsPtr->lsfLimits[LSF_RLIMIT_FSIZE].rlim_maxh != 0x7fffffff
		|| jobSpecsPtr->lsfLimits[LSF_RLIMIT_FSIZE].rlim_curl != 0xffffffff
		|| jobSpecsPtr->lsfLimits[LSF_RLIMIT_FSIZE].rlim_curh != 0x7fffffff) {
		ls_closelog_ext ();
}

if (setIds (jobCardPtr) < 0) {
	jobSetupStatus (JOB_STAT_PEND, PEND_JOB_EXEC_INIT, jobCardPtr);
}

#if 0
	/* disabled for now... // FIXME FIXME FIXME FIXME what does this set the job to?
	*/
	if (acctMapOk (jobCardPtr) < 0) {
		jobSetupStatus (JOB_STAT_PEND, PEND_RMT_PERMISSION, jobCardPtr);
	}
#endif

if ((fromHp = Gethostbyname_ (jobSpecsPtr->fromHost)) == NULL) {
	jobSetupStatus (JOB_STAT_PEND, PEND_JOB_EXEC_INIT, jobCardPtr);
}

if (initPaths (jobCardPtr, fromHp, &jf) < 0) {
	jobSetupStatus (JOB_STAT_PEND, PEND_JOB_PATHS, jobCardPtr);
}

	for ( int i = 1; i < SIGRTMAX; i++) { // originally, NSIG was here, instead of SIGRTMAX; SIGRTMAX is the POSIX equivalent of NSIG
		Signal_ (i, SIG_DFL);
	}

	Signal_ (SIGHUP, SIG_IGN);

	sigemptyset (&newmask);
	sigprocmask (SIG_SETMASK, &newmask, NULL);

	/* unblock all signals */
	alarm (0);

	closeExceptFD (-1);

	createJobTmpDir (jobCardPtr);

	if (jobSpecsPtr->options & SUB_PRE_EXEC) {
		runUPre (jobCardPtr);
	}

	if ((jobSpecsPtr->options & SUB_LOGIN_SHELL) || UID_MAPPED (jobCardPtr))
	{
		if (createTmpJobFile (jobSpecsPtr, fromHp, jobCardPtr->stdinFile) < 0) {
			jobSetupStatus (JOB_STAT_PEND, PEND_JOB_NO_FILE, jobCardPtr);
		}
	}

	if (!(jobCardPtr->jobSpecs.options & SUB_RESTART)) {

		if (!(!daemonParams[LSB_BSUBI_OLD].paramValue && PURE_INTERACTIVE (&jobCardPtr->jobSpecs) && !UID_MAPPED (jobCardPtr))) {
			jobSetupStatus (JOB_STAT_RUN, 0, jobCardPtr);
		}
	}

	if (jobCardPtr->jobSpecs.options & SUB_NOTIFY_BEGIN) {
		sendNotification (jobCardPtr);
	}

	if (jobSpecsPtr->options & SUB_RESTART) {
		execRestart (jobCardPtr, fromHp);
	}


	if (!(jobSpecsPtr->options & SUB_RESTART)) {
		char *jobArgv[4];
		char **execArgv = NULL;
		char tmpJobFile[MAXLINELEN];

		if (!(jobSpecsPtr->options & SUB_LOGIN_SHELL) && !UID_MAPPED (jobCardPtr)) {

			sprintf (tmpJobFile, "%s%s", jobSpecsPtr->jobFile, JOBFILEEXT);
			osConvertPath_ (tmpJobFile);
			jobArgv[0] = tmpJobFile;
			jobArgv[1] = NULL;

			execArgv = execArgs (jobSpecsPtr, jobArgv);

			ls_syslog (LOG_DEBUG, "%s: job %s execArgv[0] is %s, execArg[1] is %s", __func__, lsb_jobid2str (jobSpecsPtr->jobId), execArgv[0], execArgv[1]);

			if ((jobSpecsPtr->options & SUB_INTERACTIVE) && (jobSpecsPtr->options & SUB_PTY)) {
				chuser (batchId);
				if (!REShasPTYfix (execArgv[0])) {
						// lsfSetUid (jobSpecsPtr->execUid);
					lsfSetXUid(0, jobSpecsPtr->execUid, jobSpecsPtr->execUid, -1, setuid);
				}
			}
				// lsfExecv (execArgv[0], execArgv);
			lsfExecX(execArgv[0], execArgv, execv);

			fprintf (stderr, "LSF: Unable to execute jobfile %s job %s: %s\n", execArgv[0], lsb_jobid2str (jobSpecsPtr->jobId), strerror (errno));
		}
		else {

			if ((jobSpecsPtr->options & SUB_INTERACTIVE) && (jobSpecsPtr->options & SUB_PTY)) {
				chuser (batchId);
					// lsfSetUid (jobSpecsPtr->execUid);
				lsfSetXUid(0, jobSpecsPtr->execUid, jobSpecsPtr->execUid, -1, setuid);
			}

			shellPath = "/bin/sh";  // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac, support more shells? or check shell capabilities?
			if (jobSpecsPtr->options & SUB_LOGIN_SHELL)
			{
				if (jobSpecsPtr->loginShell != NULL && jobSpecsPtr->loginShell[0] != '\0') {
					shellPath = jobSpecsPtr->loginShell;
				}
				else {
					shellPath = getLoginShell (NULL, jobSpecsPtr->jobFile, fromHp, 1);
				}
			}

			if (logclass & LC_EXEC) {
				ls_syslog (LOG_DEBUG2, "%s: options=%x sock=%d shellPath=%s", __func__, jobSpecsPtr->options, chanSock_ (chfd), shellPath);
			}

			putEnv ("PATH", "/bin:/usr/bin/:/local/bin:/usr/local/bin"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

			chdir (jobSpecsPtr->execHome);
			if (shellPath != NULL)
			{
				putEnv ("SHELL", shellPath); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

				jobArgv[0] = "-";

				jobArgv[1] = NULL;
				// lsfExecv (shellPath, jobArgv);
				lsfExecX(shellPath, jobArgv, execv);

				fprintf (stderr, "LSF: Unable to execute login shell %s for job %s: %s\n", shellPath, lsb_jobid2str (jobSpecsPtr->jobId), strerror (errno));
			}
			else
			{
				fprintf (stderr, "LSF: Unable to find login shell for job %s.\n", lsb_jobid2str (jobSpecsPtr->jobId));
			}
		}

	}

	exit (-1);
}


// sbatchd uses the following command-line argument -d, -p <port number>, -P, -i, -o, -e, -m
char **
execArgs (struct jobSpecs *jp, char **execArgv)
{
	unsigned int i = 0;
	static char portStr[10];
	static char *argv[64];
	static char debugStr[10];

	argv[i++] = getDaemonPath_ ("/res", daemonParams[LSF_SERVERDIR].paramValue); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	if (debug) {
		sprintf (debugStr, "-%d", debug);
		argv[i++] = debugStr;
	}

	if (env_dir != NULL) {
		argv[i] = "-d";
		i++;
		argv[i] = env_dir;
		i++;
	}

	if (jp->options & SUB_INTERACTIVE) {

		argv[i++] = "-p";
		sprintf (portStr, "%d", jp->niosPort);
		argv[i++] = portStr;

		if (jp->options & SUB_PTY) {
			argv[i++] = "-P";
		}
		if ((jp->options & SUB_IN_FILE) || (jp->options2 & SUB2_IN_FILE_SPOOL)) {
			argv[i++] = "-i";
		}
		if (jp->options & SUB_OUT_FILE) {
			argv[i++] = "-o";
		}
		if ((jp->options & SUB_ERR_FILE) || (jp->options & SUB_OUT_FILE)) {
			argv[i++] = "-e";
		}
	}

	argv[i++] = "-m";
	argv[i++] = jp->fromHost;
	for (; *execArgv; execArgv++) {
		argv[i++] = *execArgv;
	}

	argv[i] = NULL;

	return argv; 
}


void
resetEnv (void)
{
	char **env;
	env = my_calloc (2, sizeof (char *), __func__);

	env[0] = NULL;
	environ = env;

	return;
}


int
setJobEnv (struct jobCard *jp)
{
	const unsigned int TZSAVE_LENGTH = 255;
	const unsigned int EEXECTSTR_LENGTH  = 255; 
	char *hosts = NULL;
	char *sp = NULL;
	char *tz = NULL;
	char tzsave[ TZSAVE_LENGTH ];
	char *eexecT = NULL;
	char eexecTStr[ EEXECTSTR_LENGTH];
	char *eauthAuxData = NULL;
	char eauthAuxDataStr[MAX_PATH_LEN];
	char val[MAXLINELEN];
	char shellFile[MAX_FILENAME_LEN];
	char userName[MAXLINELEN];
	struct nameList *hostList = NULL; // was NAMELIST *hostList = NULL;

	memset( tzsave, 0, TZSAVE_LENGTH);
	memset( eauthAuxDataStr, 0, MAX_PATH_LEN);
	memset( val, 0, MAXLINELEN);
	memset( shellFile, 0, MAX_FILENAME_LEN);
	memset( userName, 0, MAXLINELEN);

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG, "setJobEnv: Job <%s> numEnv %d ...", lsb_jobid2str (jp->jobSpecs.jobId), jp->jobSpecs.numEnv);
	}


	if ((tz = getenv ("TZ"))) {
		strcpy (tzsave, tz);
	}

	if ((eexecT = getenv (LS_EXEC_T))) {
		strcpy (eexecTStr, eexecT);
	}

	if ((eauthAuxData = getenv ("LSF_EAUTH_AUX_DATA")) != NULL) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		if (logclass & LC_TRACE) {
			ls_syslog (LOG_DEBUG, "%s: LSF_EAUTH_AUX_DATA=%s", __func__, eauthAuxData); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}
		strcpy (eauthAuxDataStr, eauthAuxData);
	}

	resetEnv ();

	for ( unsigned int i = 0; i < jp->jobSpecs.numEnv; i++) {
		if ((sp = strchr (jp->jobSpecs.env[i], '=')) == NULL) {
			continue;
		}

		if ((jp->jobSpecs.options & SUB_LOGIN_SHELL)
			&& (strstr (jp->jobSpecs.env[i], "LSB_ACCT_MAP") == NULL)              // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			&& (strstr (jp->jobSpecs.env[i], "LSF_VERSION") == NULL)               // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			&& (strstr (jp->jobSpecs.env[i], "LSB_UNIXGROUP") == NULL)             // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			&& (strstr (jp->jobSpecs.env[i], "LSB_JOB_STARTER") == NULL)           // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			&& (strstr (jp->jobSpecs.env[i], "LSF_INTERACTIVE_STDERR") == NULL)    // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			&& (strstr (jp->jobSpecs.env[i], "LOGNAME") == NULL)                   // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			&& (strstr (jp->jobSpecs.env[i], "LSF_CUR_SECURITY_LABEL") == NULL)    // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			&& (strstr (jp->jobSpecs.env[i], "LSF_JOB_SECURITY_LABEL") == NULL))   // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		{

			if (!(jp->jobSpecs.options & SUB_INTERACTIVE)) {
				continue;
			}
			if ((strstr (jp->jobSpecs.env[i], "TERMCAP") == NULL) && (strstr (jp->jobSpecs.env[i], "TERM") == NULL)) {  // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				continue;
			}

		}

		*sp = '\0';
		sp++;
		putEnv (jp->jobSpecs.env[i], sp);
		sp--;
		*sp = '=';
	}

	if (tz) {
		putEnv ("TZ", tzsave); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	}

	sprintf (val, "%lu", LSB_ARRAY_JOBID (jp->jobSpecs.jobId));
	putEnv ("LSB_JOBID", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	sprintf (val, "%lu", LSB_ARRAY_IDX (jp->jobSpecs.jobId));
	putEnv ("LSB_JOBINDEX", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	if (atoi (val) != 0) {
		setJobArrayEnv (jp->jobSpecs.jobName, LSB_ARRAY_IDX (jp->jobSpecs.jobId));
	}

	sprintf (val, "%s", jp->jobSpecs.jobFile);
	putEnv ("LSB_JOBFILENAME", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	
// Checkpointing starts here
	if (strlen (jp->jobSpecs.chkpntDir) == 0) {

		if (jp->jobSpecs.jobSpoolDir[0] == '\0') {
			sprintf (shellFile, "%s/.lsbatch/%s", jp->jobSpecs.execHome, jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}
		else {
				sprintf (shellFile, "%s/%s", jp->jobSpecs.jobSpoolDir, jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			}
		}
		else {
			sprintf (shellFile, "%s/%s", jp->jobSpecs.chkpntDir, jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		if (strlen (jp->jobSpecs.chkpntDir) > 0) {
			char chkDir[MAX_FILENAME_LEN];
			char cwd[MAX_FILENAME_LEN];
			char *strPtr = NULL; // FIXME FIXME FIXME FIXME FIXME 100% mem leak
			// char *hostList = NULL; // was NAMELIST *hostList = NULL;

/****************************************************************
 * WARNING: DUE TO DANGLING ELSEs,THERE MIGHT BE A CHANGE IN THE
 * EXECUTION PATH, BUT THAT WILL HAVE TO BE INVESTIGATED WHEN DE-
 * BUGGING FIXME FIXME FIXME FIXME FIXME
 ****************************************************************
 */

			memset( chkDir, 0, MAX_FILENAME_LEN );
			memset( cwd,    0, MAX_FILENAME_LEN ); 

			if (((strPtr = strrchr (jp->jobSpecs.chkpntDir, '/')) != NULL) && (islongint_ (strPtr + 1))) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				if (jp->jobSpecs.chkpntDir[0] == '/') { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

					if (jp->jobSpecs.options & SUB_RESTART) {
						*strPtr = '\0';
						sprintf (chkDir, "%s/%s", jp->jobSpecs.chkpntDir, lsb_jobidinstr (jp->jobSpecs.jobId));
						*strPtr = '/'; // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
					}
					else {
						sprintf (chkDir, "%s", jp->jobSpecs.chkpntDir);
					}
				}
				else {

					if (jp->jobSpecs.cwd[0] == '/') { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						strcpy (cwd, jp->jobSpecs.cwd);
				}
				else {
						if (jp->jobSpecs.cwd[0] == '\0') { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
							strcpy (cwd, jp->jobSpecs.subHomeDir);
					}
					else {
							sprintf (cwd, "%s/%s", jp->jobSpecs.subHomeDir, jp->jobSpecs.cwd); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						}
					}
					if (jp->jobSpecs.options & SUB_RESTART) {
						*strPtr = '\0';
						sprintf (chkDir, "%s/%s/%s", cwd, jp->jobSpecs.chkpntDir,   lsb_jobidinstr (jp->jobSpecs.jobId)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						*strPtr = '/'; // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
					}
					else {
						sprintf (chkDir, "%s", jp->jobSpecs.chkpntDir);
					}
				}
				putEnv ("LSB_CHKPNT_DIR", chkDir); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				sprintf (shellFile, "%s/%s", chkDir, jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			}
			else {
				sprintf (shellFile, "%s/%s", jp->jobSpecs.chkpntDir, jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				/* catgets 430 */
				ls_syslog (LOG_ERR, I18N (430, "Failed to set LSB_CHKPNT_DIR, error in the chkpnt directory: %s"), jp->jobSpecs.chkpntDir);
			}

			sprintf (val, "%ld", jp->jobSpecs.chkPeriod);
			putEnv ("LSB_CHKPNT_PERIOD", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}
// Checkpointing finishes here

		putEnv ("LSB_CHKFILENAME", shellFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		sprintf (val, "trap # %d %d %d %d %d", SIGTERM, SIGUSR1, SIGUSR2, SIGINT, SIGHUP);
		putEnv ("LSB_TRAPSIGS", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		if (jp->jobSpecs.numToHosts <= (MAX_LSB_HOSTS_LEN / 2)) {

			unsigned int maxNumToHosts = 0;
			hosts = my_malloc (jp->jobSpecs.numToHosts * MAXHOSTNAMELEN, __func__ );
			memset( hosts, 0, jp->jobSpecs.numToHosts * MAXHOSTNAMELEN );
			for ( unsigned int i = 0; i < jp->jobSpecs.numToHosts - 1; i++) {
				strcat (hosts, jp->jobSpecs.toHosts[i]);
				strcat (hosts, " ");
				maxNumToHosts++;
			}
			strcat (hosts, jp->jobSpecs.toHosts[ maxNumToHosts ]);

			if (strlen (hosts) < MAX_LSB_HOSTS_LEN) {
				putEnv ("LSB_HOSTS", hosts); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			}
			free (hosts);
		}

		if ((jp->jobSpecs.options & SUB_OUT_FILE)) {
			sprintf (val, "%s", jp->jobSpecs.outFile);
			putEnv ("LSB_OUTPUTFILE", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		if ((jp->jobSpecs.options & SUB_ERR_FILE)) {
			sprintf (val, "%s", jp->jobSpecs.errFile);
			putEnv ("LSB_ERRORFILE", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		hostList = lsb_compressStrList (jp->jobSpecs.toHosts, jp->jobSpecs.numToHosts);
		putEnv ("LSB_MCPU_HOSTS", lsb_printNameList (hostList, PRINT_MCPU_HOSTS)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		sprintf (val, "%s", jp->jobSpecs.queue);
		putEnv ("LSB_QUEUE", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		sprintf (val, "%s", jp->jobSpecs.jobName);
		putEnv ("LSB_JOBNAME", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		strcpy (val, "Y"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		if (jp->jobSpecs.options & SUB_RESTART) {
			putEnv ("LSB_RESTART", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		putEnv ("LSFUSER", jp->execUsername); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		if (getOSUserName_ (jp->execUsername, userName, MAXLINELEN) == 0) {
			putEnv ("USER", userName); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}
		else {
			putEnv ("USER", jp->execUsername); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		sprintf (val, "%d", LSB_PRE_ABORT);
		putEnv ("LSB_EXIT_PRE_ABORT", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		// if (jp->jobSpecs.requeueEValues && jp->jobSpecs.requeueEValues[0] != '\0') {
		if ( jp->jobSpecs.requeueEValues[0] != '\0') {
			putEnv ("LSB_EXIT_REQUEUE", jp->jobSpecs.requeueEValues); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		if (eexecT) {
			putEnv (LS_EXEC_T, eexecTStr);
		}

		if (jp->jobSpecs.options & SUB_INTERACTIVE) {
			putEnv ("LSB_INTERACTIVE", "Y"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}


		if ((getenv ("WINDIR") != NULL) || (getenv ("windir") != NULL)) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			char tmppath[MAX_PATH_LEN];

			sprintf (tmppath, "/bin:/usr/bin:/sbin:/usr/sbin"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			if (daemonParams[LSF_BINDIR].paramValue != NULL) {
				strcat (tmppath, ":"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				strcat (tmppath, daemonParams[LSF_BINDIR].paramValue);
			}
			putEnv ("PATH", tmppath); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}
		else {

			char *tmppath = NULL;
			size_t len;
			char *envpath = NULL;
			int cc = TRUE;

			if (daemonParams[LSF_BINDIR].paramValue != NULL) {
				envpath = getenv ("PATH"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				if (envpath != NULL) {

					len = strlen (daemonParams[LSF_BINDIR].paramValue);
					cc = strncmp (envpath, daemonParams[LSF_BINDIR].paramValue, len);
					if (cc != 0) {
						len += strlen (envpath) + 2;
					}
				}
				else {
					len = strlen (daemonParams[LSF_BINDIR].paramValue) + 2;
				}
				if (cc != 0) {
					tmppath = malloc (len);
					if (tmppath == NULL) {
						ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						exit (-1);
					}
					strcpy (tmppath, daemonParams[LSF_BINDIR].paramValue);
					if (envpath != NULL) {
						strcat (tmppath, ":"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						strcat (tmppath, envpath);
					}
					putEnv ("PATH", tmppath); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
					FREEUP (tmppath);
				}
				else {
					putEnv ("PATH", envpath);
				}
			}
		}

		sprintf (val, "%d", (int) getpid ());
		putEnv ("LS_JOBPID", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		if (isAbsolutePathSub (jp, jp->jobSpecs.cwd)) {
			putEnv ("LS_SUBCWD", jp->jobSpecs.cwd); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}
		else {
			char cwd[MAX_FILENAME_LEN];

			memset( cwd, 0, MAX_FILENAME_LEN );
			sprintf (cwd, "%s/%s", jp->jobSpecs.subHomeDir, jp->jobSpecs.cwd);
			putEnv ("LS_SUBCWD", cwd); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		putEnv ("LSB_SUB_HOST", jp->jobSpecs.fromHost); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		putEnv ("LSF_ENVDIR", env_dir);                 // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		if (daemonParams[LSF_SERVERDIR].paramValue != NULL) {
			putEnv ("LSF_SERVERDIR", daemonParams[LSF_SERVERDIR].paramValue); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		if (daemonParams[LSF_BINDIR].paramValue != NULL) {
			putEnv ("LSF_BINDIR", daemonParams[LSF_BINDIR].paramValue); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		if (daemonParams[LSF_LIBDIR].paramValue != NULL) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			char path[MAX_FILENAME_LEN];
			memset( path, 0, MAX_FILENAME_LEN );

			putEnv ("LSF_LIBDIR", daemonParams[LSF_LIBDIR].paramValue); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

			sprintf (path, "%s/%s", daemonParams[LSF_LIBDIR].paramValue, "uid"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			putEnv ("XLSF_UIDDIR", path); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}


		if (eauthAuxData != NULL) {
			putEnv ("LSF_EAUTH_AUX_DATA", eauthAuxDataStr); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		sprintf (val, "%d", jp->w_status);
		putEnv ("LSB_JOBEXIT_STAT", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

#ifdef INTER_DAEMON_AUTH
		if (jp) {
			char bufUid[MAX_FILENAME_LEN];
			char bufGid[MAX_FILENAME_LEN];

			memset( bufUid, 0, MAX_FILENAME_LEN );
			memset( bufGid, 0, MAX_FILENAME_LEN );

			sprintf (bufUid, "LSB_EEXEC_REAL_UID=%d", jp->jobSpecs.execUid); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			sprintf (bufGid, "LSB_EEXEC_REAL_GID=%d", jp->execGid); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			putenv (bufUid);
			putenv (bufGid);
		}
#endif

	runEexec_ ("", jp->jobSpecs.jobId, &jp->jobSpecs.eexec, NULL);

#ifdef INTER_DAEMON_AUTH
	if (jp) {
		putenv ("LSB_EEXEC_REAL_UID="); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		putenv ("LSB_EEXEC_REAL_GID="); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	}
#endif
	return 0; 
}


int
setLimits (struct jobSpecs *jobSpecsPtr)
{
	return mysetLimits (jobSpecsPtr); 
}

int
mysetLimits (struct jobSpecs *jobSpecsPtr)
{
	struct rlimit rlimit;

#ifdef  RLIMIT_CPU

	if ((lsbJobCpuLimit != 1) ||
		(jobSpecsPtr->lsfLimits[LSF_RLIMIT_CPU].rlim_maxl == 0xffffffff &&
			jobSpecsPtr->lsfLimits[LSF_RLIMIT_CPU].rlim_maxh == 0x7fffffff &&
			jobSpecsPtr->lsfLimits[LSF_RLIMIT_CPU].rlim_curl == 0xffffffff &&
			jobSpecsPtr->lsfLimits[LSF_RLIMIT_CPU].rlim_curh == 0x7fffffff))
	{
		rlimitDecode_ (&jobSpecsPtr->lsfLimits[LSF_RLIMIT_CPU],
			&rlimit, LSF_RLIMIT_CPU);

		if (setrlimit (RLIMIT_CPU, &rlimit) < 0)
		{
			if (!debug) {
				/* catgets 5406 */
				// it was (long)
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5406, "%s: setrlimit(RLIMIT_CPU) failed for job <%s>: %m: soft %ld hard %ld"), __func__, lsb_jobid2str (jobSpecsPtr->jobId), rlimit.rlim_cur, rlimit.rlim_max);
			}
		}
	}
#endif

#ifdef  RLIMIT_FSIZE
	rlimitDecode_ (&jobSpecsPtr->lsfLimits[LSF_RLIMIT_FSIZE], &rlimit, LSF_RLIMIT_FSIZE);
	if (setrlimit (RLIMIT_FSIZE, &rlimit) < 0) {
		if (!debug) {
			/* catgets 5407 */ // it was (long)
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5407, "%s: setrlimit(RLIMIT_FSIZE) failed for job <%s>: %m: soft %ld hard %ld"), __func__, lsb_jobid2str (jobSpecsPtr->jobId), rlimit.rlim_cur, rlimit.rlim_max);
		}
	}
#endif

#ifdef RLIMIT_DATA
	rlimitDecode_ (&jobSpecsPtr->lsfLimits[LSF_RLIMIT_DATA], &rlimit, LSF_RLIMIT_DATA);
	if (setrlimit (RLIMIT_DATA, &rlimit) < 0) {
		if (!debug)
		/* catgets 5409 */ // it was (long)
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5409, "%s: setrlimit(RLIMIT_DATA) failed for job <%s>: %m: soft %ld hard %ld"), __func__, lsb_jobid2str (jobSpecsPtr->jobId), rlimit.rlim_cur, rlimit.rlim_max);
	}
#endif

#ifdef  RLIMIT_STACK
	rlimitDecode_ (&jobSpecsPtr->lsfLimits[LSF_RLIMIT_STACK], &rlimit, LSF_RLIMIT_STACK);
	if (setrlimit (RLIMIT_STACK, &rlimit) < 0) {
		if (!debug) {
			/* catgets 5410 */ // it was (long)
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5410, "%s: setrlimit(RLIMIT_STACK) failed for job <%s>: %m: soft %ld hard %ld"), __func__, lsb_jobid2str (jobSpecsPtr->jobId),rlimit.rlim_cur, rlimit.rlim_max);
		}
	}
#endif

#ifdef RLIMIT_CORE
	rlimitDecode_ (&jobSpecsPtr->lsfLimits[LSF_RLIMIT_CORE], &rlimit, LSF_RLIMIT_CORE);
	if (setrlimit (RLIMIT_CORE, &rlimit) < 0) {
		if (!debug) {
		/* catgets 5411 */ // it was (long)
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5411, "%s: setrlimit(RLIMIT_CORE) failed for job <%s>: %m: soft %ld hard %ld"), __func__, lsb_jobid2str (jobSpecsPtr->jobId), rlimit.rlim_cur, rlimit.rlim_max);
		}
	}
#endif

#ifdef RLIMIT_RSS

	if (lsbJobMemLimit != 1)
	{
		rlimitDecode_ (&jobSpecsPtr->lsfLimits[LSF_RLIMIT_RSS], &rlimit, LSF_RLIMIT_RSS);
		if (setrlimit (RLIMIT_RSS, &rlimit) < 0)
		{
			if (!debug) {
				/* catgets 5412 */ // it was (int)
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5412, "%s: setrlimit(RLIMIT_RSS) failed for job <%s>: %m: soft %ld hard %ld"), __func__, lsb_jobid2str (jobSpecsPtr->jobId), rlimit.rlim_cur, rlimit.rlim_max);
			}
		}
	}
#endif

	return 0; 
}

int
job_finish (struct jobCard *jobCard, int report)
{
	pid_t pid = -1;

	if (logclass & LC_EXEC) {
		ls_syslog (LOG_DEBUG, "%s: Entering ... jobId<%s> status<0x%x>", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), jobCard->jobSpecs.jStatus);
	}


	if (report && jobCard->jobSpecs.actPid) {
		return -1; 
	}

	unlockHosts (jobCard, jobCard->jobSpecs.numToHosts);

	if (report && status_job (BATCH_STATUS_JOB, jobCard, jobCard->jobSpecs.jStatus, (jobCard->jobSpecs.startTime > bootTime) ?  ERR_NO_ERROR : ERR_HOST_BOOT) < 0) {
		jobCard->notReported++;
		return -1; 
	}
	if ((jobCard->jobSpecs.jStatus & JOB_STAT_PEND) && (jobCard->jobSpecs.reasons == PEND_JOB_START_FAIL || jobCard->jobSpecs.reasons == PEND_JOB_NO_FILE)) {
		deallocJobCard (jobCard);
		return 0; 
	}

	if (!jobCard->postJobStarted) {
		pid = fork ();

		if (pid < 0) {
			ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "fork");
			jobCard->notReported++;
			return -1; 
		}

		if (pid == 0) {
			closeBatchSocket ();
			finishJob (jobCard);
			return 0; 
		}

		if (jobCard->userJobSucc == TRUE) {

			jobCard->postJobStarted = 1;
			jobCard->jobSpecs.jobPid = pid;
			jobCard->jobSpecs.jobPGid = pid;
			jobCard->jobSpecs.jStatus = JOB_STAT_RUN;

			return 0; 
		}
	}

	deallocJobCard (jobCard);

	return 0; 
}

void
unlockHosts (struct jobCard *jp, unsigned int num)
{
	if (jp == NULL || num <= 0) {
		return;
	}

	if (jp->jobSpecs.jAttrib & QUEUE_ATTRIB_EXCLUSIVE) {
		for( unsigned int i = 0; i < num; i++) { // FIXME FIXME FIXME FIXME FIXME this expects the first entry of i to has a value, at least
			// if ( i > 0 && strcmp (jp->jobSpecs.toHosts[i],jp->jobSpecs.toHosts[i - 1]) == 0) { // FIXME FIXME FIXME FIXME FIXME what if i = 0? 
			if ( strcmp (jp->jobSpecs.toHosts[i],jp->jobSpecs.toHosts[i - 1]) == 0) { // FIXME FIXME FIXME FIXME FIXME what if i = 0? 
				continue;
			}
			if (unlockHost_ (jp->jobSpecs.toHosts[i]) < 0 && lserrno != LSE_LIM_NLOCKED) {
				ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "unlockHost_", jp->jobSpecs.toHosts[i]);
			}
		}
		jp->jobSpecs.jAttrib &= ~QUEUE_ATTRIB_EXCLUSIVE;
	}

	return;
}

int
finishJob (struct jobCard *jobCard) // FIXME FIXME FIXME FIXME FIXME there are two places spotted below with "wtf?", that the return and exit conditions may not be valid. Build test case
{
	int doSendResults = 0;
	int hasError = 0;
	char tmpDirName[MAX_FILENAME_LEN];

	memset( tmpDirName, 0, MAX_FILENAME_LEN );
	doSendResults = (!jobCard->mbdRestarted && !(jobCard->jobSpecs.jStatus & JOB_STAT_PEND));
	putEnv (LS_EXEC_T, "END"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	if (postJobSetup (jobCard) == -1 && doSendResults) {
		hasError = -1; // FIXME FIXME FIXME negative error number
		chuser (batchId);
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "postJobSetup");
		/* catgets 412 */
		lsb_merr1 (_i18n_msg_get (ls_catd, NL_SETN, 412, "Unable to setup user's environment to report results for job %s\n"), lsb_jobid2str (jobCard->jobSpecs.jobId));
		exit ( hasError );
	}

	// getJobTmpDir((char *) tmpDirName, jobCard);
	getJobTmpDir( tmpDirName, jobCard);

	if (rmDirAndFilesEx (tmpDirName, 1) != 0) {
		ls_syslog (LOG_DEBUG, "%s: Fail to  delete TMPDIR=%s directory for job <%s>", __func__, tmpDirName, lsb_jobid2str (jobCard->jobSpecs.jobId)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	}

	if (doSendResults) {
		if (send_results (jobCard) != 0) {
			hasError = -1; // FIXME FIXME FIXME FIXME maybe errors should be made possitive
			/* catgets 5500 */
			ls_syslog (LOG_ERR, I18N (5500, "Has error send result for job<%s>"), lsb_jobid2str (jobCard->jobSpecs.jobId));
			return; // FIXME FIXME FIXME FIXME wtf?
		}
	}

	if (runQPost (jobCard) != 0) {
		hasError = -1;
		ls_syslog (LOG_ERR, "%s: failed to run queue post for job %s", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId));
	}

	if (jobCard->jobSpecs.reasons & EXIT_REQUEUE) {
		jobCard->jobSpecs.reasons &= ~EXIT_REQUEUE;
	}

	if ((!jobCard->mbdRestarted) && (!((jobCard->jobSpecs.reasons == PEND_QUEUE_PRE_FAIL) && (jobCard->jobSpecs.jStatus == JOB_STAT_PEND)))) {
		if (rmJobBufFiles (jobCard) != 0) {
			hasError = -1;
			/* catgets 5502 */
			ls_syslog (LOG_ERR, I18N (5502, "Has error to remove buffer file for job<%s>"), lsb_jobid2str (jobCard->jobSpecs.jobId));
			exit (hasError); // FIXME FIXME FIXME FIXME wtf?
		}
	}

	return 0;
}


void
status_report (void)
{
	struct jobCard *jp = NULL
	struct jobCard *next = NULL;
	static char mailed = TRUE;
	int rep = 0; 
	int allReported = TRUE;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG2, "%s: Entering..", __func__ );
	}

	for (jp = jobQueHead->back; (jp != jobQueHead); jp = next) {
		next = jp->back;

		if (!IS_START (jp->jobSpecs.jStatus)
			&& !(jp->jobSpecs.jStatus & JOB_STAT_PEND)
			&& !IS_FINISH (jp->jobSpecs.jStatus)
			&& !IS_POST_FINISH (jp->jobSpecs.jStatus))
		{
			/* catgets 5418 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5418, "%s: Illegal job status <%d> of job <%s> found; re-life"), __func__, jp->jobSpecs.jStatus,   lsb_jobid2str (jp->jobSpecs.jobId));
			relife ();
		}

		if (logclass & LC_TRACE) {
			ls_syslog (LOG_DEBUG3, "status_report: checking job %s notReproted=%d missing=%d needReportRU=%d now=%d startTime=%d status=%x", lsb_jobid2str (jp->jobSpecs.jobId), jp->notReported, jp->missing, jp->needReportRU, now, jp->jobSpecs.startTime, jp->jobSpecs.jStatus);
		}

		if (jp->notReported < 0) {
			continue;
		}

		if (jp->missing) {
			continue;
		}

		if (!jp->notReported && !jp->needReportRU) {
			continue;
		}

		if ((now >= jp->jobSpecs.startTime) && (now < jp->jobSpecs.startTime + 10)) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			continue;
			/* don't retry other jobs either */
	}

	if (IS_START (jp->jobSpecs.jStatus)) {
		mbdReqType reqType;

		if (!jp->notReported && jp->needReportRU) {
			reqType = BATCH_RUSAGE_JOB;
		}
		else {
			reqType = BATCH_STATUS_JOB;
		}

		rep = status_job (reqType, jp, jp->jobSpecs.jStatus, ERR_NO_ERROR);

		if (reqType == BATCH_STATUS_JOB) {
			if (rep >= 0) {
				if (jp->notReported > 0)
					jp->notReported = 0;
			}
			else {
				allReported = FALSE;
				jp->notReported++;
					if (jp->notReported == 40 && !mailed) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						mailed = TRUE;
						/* catgets 411 */
					lsb_merr (_i18n_printf (_i18n_msg_get (ls_catd, NL_SETN, 411, "%s: unable to report job %s status to master; retried %d times\n"), __func__, lsb_jobid2str (jp->jobSpecs.jobId),    jp->notReported));
				}
			}
		}
	}
}


if (allReported == TRUE) {
	mailed = FALSE;
}

return;
}

void
rmvJobStarterStr ( const char *line, const char *jobStarter)
{
	// static char *header = NULL;
	// static char *tailer = NULL;
	// static int header_len = 0;
	char *header = NULL;
	char *tailer = NULL;
	size_t header_len = 0;
	char *cmd = NULL;

	if (jobStarter == NULL || jobStarter[0] == '\0') {
		return;
	}
	if (header == NULL) {
		header = jobStarter;
		if ((tailer = strstr (jobStarter, JOB_STARTER_KEYWORD)) != NULL) {
			*tailer = '\0';
			tailer += strlen (JOB_STARTER_KEYWORD);

		}
		header_len = strlen (header);
	}
	if (strncmp (line, header, header_len) != 0) {
		return;
	}
	if (tailer != NULL) {
		if ((cmd = strstr (line, tailer)) != NULL) {

			*cmd = '\n';
			*(cmd + 1) = '\0';
		}
	}
	cmd = line + strlen (header);
	while (*cmd == ' ' && *cmd != '\0') {
		cmd++;
	}
	memmove (line, cmd, (strlen (cmd) + 1) * sizeof (char)); // FIXME FIXME FIXME FIXME FIXME why memmove?!

	return;
}


int
isLink ( const char *filename)
{
	struct stat statBuf;
	int lstatReturnValue = 0;
	int returnValue = FALSE;

	lstatReturnValue = lstat (filename, &statBuf);
	if (lstatReturnValue == 0) {

		if (S_ISLNK (statBuf.st_mode)) {
			returnValue = TRUE;
		}
	}

	if (logclass & LC_TRACE) {
		if (filename) {
			ls_syslog (LOG_DEBUG1, "%s: filename:%s, lstatReturnValue:%d, returnValue:%d", __func__, filename, lstatReturnValue, returnValue);
		}
		else {
			ls_syslog (LOG_DEBUG1, "%s: filename is null, lstatReturnValue:%d, returnValue:%d", __func__, lstatReturnValue, returnValue);
		}
	}

	return returnValue;
}

void
shouldCopyFromLsbatch (struct jobCard *jp, int *cpyStdoutFromLsbatch, int *cpyStderrFromLsbatch)
{

	assert( *cpyStdoutFromLsbatch ); 
	assert( *cpyStderrFromLsbatch );

	if (lsbStdoutDirect) {

		char filename[MAX_FILENAME_LEN];
		if (jp->jobSpecs.options & SUB_OUT_FILE) {
			sprintf (filename, "%s.out", jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

			if (isLink (filename)) {
				*cpyStdoutFromLsbatch = FALSE;
			}
		}


		if (jp->jobSpecs.options & SUB_ERR_FILE) {
			sprintf (filename, "%s.err", jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac


			if (isLink (filename)) {
				*cpyStderrFromLsbatch = FALSE;
			}

		}
	}

	if (logclass & LC_TRACE) {
		char errMsg[MAXLINELEN];
		memset( errMsg, 0, MAXLINELEN );

		sprintf (errMsg, "%s: leaving... cpyStdoutFromLsbatch = %d cpyStderrFromLsbatch = %d", __func__, cpyStdoutFromLsbatch, cpyStderrFromLsbatch);
		sbdSyslog (LOG_DEBUG, errMsg);
	}

	return;
}


int
send_results (struct jobCard *jp)
{

	FILE *mail = NULL;
	FILE *fp = NULL;
	FILE *errout = NULL;
	FILE *output = NULL;
	FILE *notif = NULL;
	char *result = NULL;
	char *hostname = NULL;
	char *sp = NULL;
	char *jobStarter = NULL;
	char withps = FALSE;
	char xfile = FALSE;
	char outputIsDirectory = FALSE;
	char errIsDirectory = FALSE;
	char rcpMsg[MSGSIZE];
	char ps[MSGSIZE * 4];
	char line[MSGSIZE * 2];
	char fileName[MAX_FILENAME_LEN];
	char ofileHost[MAX_FILENAME_LEN];
	char outputFileName[MAX_FILENAME_LEN];
	char errFileName[MAX_FILENAME_LEN];
	char mailSizeStr[MAX_FILENAME_LEN];
	char jobIdFile[16];
	char jobIdStr[32];
	int i = 0;
	int k = 0;
	int nItems = 0;
	int errFileError = FALSE;
	int ofIdx = 0;
	int hasError = 0;
	int copyStdoutFromLsbatch = TRUE;
	int copyStderrFromLsbatch = TRUE;
	int errorOpeningOutputFile = FALSE;
	int sendWarning = FALSE;
	LS_WAIT_T w_status; // defined in lsf.h
	float cpuTime = jp->cpuTime;
	long mailSizeLimit = 0;
	unsigned long submitJid = 0; // valid jobIds have values greater than 0
	struct hostent *hp = NULL;
	struct stat ostat;
	struct stat stb;
	struct stat outfileStat;

	shouldCopyFromLsbatch (jp, &copyStdoutFromLsbatch, &copyStderrFromLsbatch);

	outfileStat.st_size = -1;
	if (!(jp->jobSpecs.options & SUB_OUT_FILE)) {

		sprintf (mailSizeStr, "%s.out", jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		if (stat (mailSizeStr, &outfileStat) < 0) {
			ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "stat", mailSizeStr); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			outfileStat.st_size = -1;
		}
	}

	sprintf (mailSizeStr, "%jd", outfileStat.st_size);

	if (daemonParams[LSB_MAILSIZE_LIMIT].paramValue != NULL) {
		mailSizeLimit = atol (daemonParams[LSB_MAILSIZE_LIMIT].paramValue);

		if (mailSizeLimit <= 0) {
			/* catgets 5414 */
			ls_syslog (LOG_ERR, I18N (5414, "%s: Illegal value for LSB_MAILSIZE_LIMIT, ignoring"), __func__);
		}
	}

	LS_STATUS (w_status) = jp->w_status;

	ps[0] = '\0';

	if ((hostname = ls_getmyhostname ()) == NULL) {
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "ls_getmyhostname");
		hostname = "localhost";
	}

	if (jp->w_status) {
		result = putstr_ (I18N_Exited);
	}
	else {
		result = putstr_ (I18N_Done);
	}

	hp = Gethostbyname_ (jp->jobSpecs.fromHost);
	if (jp->jobSpecs.options & SUB_INTERACTIVE) {
		if (!(jp->jobSpecs.options & SUB_OUT_FILE)) {
			strcpy (jp->jobSpecs.outFile, LSDEVNULL);
			jp->jobSpecs.options |= SUB_OUT_FILE;
		}
	}

	if (jp->jobSpecs.options & SUB_OUT_FILE) {

		char chr = '\0';
		int lastSlash = FALSE;
		int outDirOk = FALSE;

		chr = jp->jobSpecs.outFile[strlen (jp->jobSpecs.outFile) - 1];

		if (chr == '/' || chr == '\\') {
			outputIsDirectory = TRUE;
			lastSlash = TRUE;
		}

		if (stat (jp->jobSpecs.outFile, &stb) == 0 && S_ISDIR (stb.st_mode)) {
			outputIsDirectory = TRUE;
			outDirOk = TRUE;
		}

		if (!outputIsDirectory) {
			strcpy (outputFileName, jp->jobSpecs.outFile);
			outDirOk = TRUE;
		}
		else {
			strcpy (outputFileName, jp->jobSpecs.outFile);
			if (!outDirOk) {

				if (mkdir (outputFileName, 0700) == 0) {
					outDirOk = TRUE;
				}
			}

			if (!lastSlash) {
				strcat (outputFileName, "/"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			}

			sprintf (jobIdFile, "%s.out", lsb_jobidinstr (jp->jobSpecs.jobId)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			strcat (outputFileName, jobIdFile);
		}

		if (outDirOk) {
			output = myfopen_ (outputFileName, "a", hp); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		}

		if (output == NULL) {
			errorOpeningOutputFile = TRUE;

			if (copyStdoutFromLsbatch) {
				/* catgets 414 */
				sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 414, "Fail to open output file %s: %s.\nOutput is stored in this mail.\n"), jp->jobSpecs.outFile, strerror (errno));
			}
			else {
				/* catgets 483 */
				sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 483, "Fail to open output file %s: %s.\n"), jp->jobSpecs.outFile, strerror (errno));
			}

			strcat (ps, line);
			withps = TRUE;

			sprintf (mailSizeStr, "%s", "y"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			putEnv ("LSB_OUTPUT_TARGETFAILED", mailSizeStr); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			sprintf (mailSizeStr, "%s.out", jp->jobSpecs.jobFile);
			if (stat (mailSizeStr, &outfileStat) < 0) {
				ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "stat", mailSizeStr); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				outfileStat.st_size = -1;
			}
			sprintf (mailSizeStr, "%jd", outfileStat.st_size);
			putEnv ("LSB_MAILSIZE", mailSizeStr); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			if (jp->jobSpecs.options & SUB_MAIL_USER) {
				output = (mail = smail (jp->jobSpecs.mailUser, jp->jobSpecs.fromHost));
			}
			else {
				output = (mail = smail (jp->jobSpecs.userName, jp->jobSpecs.fromHost));
			}
		}
	}
	else {
		putEnv ("LSB_MAILSIZE", mailSizeStr); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		if (jp->jobSpecs.options & SUB_MAIL_USER) {
			output = (mail = smail (jp->jobSpecs.mailUser, jp->jobSpecs.fromHost));
		}
		else {
			output = (mail = smail (jp->jobSpecs.userName, jp->jobSpecs.fromHost));
		}
	}

	if (jp->jobSpecs.options & SUB_NOTIFY_END) {
		if (jp->jobSpecs.options & SUB_MAIL_USER) { 
			notif = (output == mail) ? output : smail (jp->jobSpecs.mailUser, jp->jobSpecs.fromHost);
		}
		else {
			notif = (output == mail) ? output : smail (jp->jobSpecs.userName, jp->jobSpecs.fromHost);
		}
	}
	else {
		if (jp->jobSpecs.options & SUB_INTERACTIVE) {
			notif = myfopen_ (LSDEVNULL, "a", hp); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			sendWarning = TRUE;
		}
		else {
			notif = output;
		}
	}

	if (notif == output) {
		sendWarning = TRUE;
	}

	if (jp->jobSpecs.options & SUB_ERR_FILE) {
		char chr = '\0';
		int lastSlash = FALSE;
		int errDirOk = FALSE;

		chr = jp->jobSpecs.errFile[strlen (jp->jobSpecs.errFile) - 1];

		if (chr == '/' || chr == '\\') { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			errIsDirectory = TRUE;
		lastSlash = TRUE;
	}

	if (stat (jp->jobSpecs.errFile, &stb) == 0 && S_ISDIR (stb.st_mode)) {
		errIsDirectory = TRUE;
		errDirOk = TRUE;
	}

	if (!errIsDirectory) {
		strcpy (errFileName, jp->jobSpecs.errFile);
		errDirOk = TRUE;
	}
	else {
		strcpy (errFileName, jp->jobSpecs.errFile);
		if (!errDirOk) {
			if (mkdir (errFileName, 0700) == 0) {
				errDirOk = TRUE;
			}
		}

		if (!lastSlash) {
				strcat (errFileName, "/"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			}
			sprintf (jobIdFile, "%s.err", lsb_jobidinstr (jp->jobSpecs.jobId)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			strcat (errFileName, jobIdFile);
		}

		if ((errout = myfopen_ (errFileName, "a", hp)) == NULL) {
			/* catgets 415 */
			sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 415, "Fail to open stderr file %s: %s.\nThe stderr output is included in this report.\n"), jp->jobSpecs.errFile, strerror (errno));
			strcat (ps, line);
			withps = TRUE;
			errout = notif;
			errFileError = TRUE;
		}
	}
	else {
		errout = output;
	}

	if (!copyStdoutFromLsbatch && (output == notif) && !errorOpeningOutputFile) {

		fputs("\n------------------------------------------------------------\n", notif);
	}
	/* catgets 416 */
	fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 416, "Sender: LSF System <%s@%s>\n"), lsbManager, hostname);

	submitJid = jp->jobSpecs.jobId;

	sprintf (jobIdStr, "%s", lsb_jobid2str (submitJid));

	/* catgets 417 */
	fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 417, "Subject: Job %s: <%s> %s\n"), jobIdStr, jp->jobSpecs.jobName, result);
	free (result);
	fprintf (notif, "\n");
	/* catgets 418 */
	fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 418, "Job <%s> was submitted from host <%s> by user <%s>.\n"), jp->jobSpecs.jobName, jp->jobSpecs.fromHost, jp->jobSpecs.userName);

	if (jp->jobSpecs.numToHosts <= 1) {
		/* catgets 420 */
		fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 420, "Job was executed on host(s) <%s>, in queue <%s>, as user <%s>.\n"),  hostname, jp->jobSpecs.queue, jp->execUsername);
	}
	else
	{
		char *hostList; // was NAMELIST *hostList;
		char str[MAXLINELEN];

		memset( str, 0, MAXLINELEN );
		hostList = lsb_compressStrList (jp->jobSpecs.toHosts, jp->jobSpecs.numToHosts);

		sprintf (str, "%d*%s", hostList->counter[0], hostList->names[0]);
		/* catgets 420 */
		fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 420, "Job was executed on host(s) <%s>, in queue <%s>, as user <%s>.\n"), str, jp->jobSpecs.queue, jp->execUsername);

		for ( unsigned int k = 1; k < hostList->listSize; k++) {
			fprintf (notif, "                            <%d*%s>\n", hostList->counter[k], hostList->names[k]);
		}

		if (jp->jobSpecs.execHome[0] == '\0') { 
			/* catgets 421 */
			fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 421, "Execution home directory was not known.\n"));
		}
		else {
			/* catgets 422 */
			fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 422, "<%s> was used as the home directory.\n"), jp->jobSpecs.execHome);
		}

		if (jp->jobSpecs.execCwd[0] == '\0') {
			/* catgets 423 */
			fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 423, "Execution working directory was not known.\n")); 
		}
		else {
			/* catgets 424 */
			fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 424, "<%s> was used as the working directory.\n"), jp->jobSpecs.execCwd);
		}

		if (jp->jobSpecs.startTime != 0) {
			sp = putstr_ (_i18n_ctime (ls_catd, CTIME_FORMAT_DEFAULT, &jp->jobSpecs.startTime));
		}
		else {
			/* catgets 404 */
			sp = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 404, "unknown time in the past\n"));
		}

		/* catgets 426 */
		fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 426, "Started at %s"), sp);
		free (sp);
		now = time (0);
		/* catgets 487 */
		fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 487, "Results reported at %s"), _i18n_ctime (487, CTIME_FORMAT_DEFAULT, &now));
		sprintf (fileName, "%s%s", jp->jobSpecs.jobFile, JOBFILEEXT);
		if ((fp = myfopen_ (fileName, "r", hp)) == NULL) {
			/* catgets 427 */
			fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 427, "Cannot open your job file: %s\n"), fileName);
		}
		else {
			while (fgets (line, MAXLINELEN, fp) != NULL) {
				if (!strcmp (line, CMDSTART)) {
					break;
				}
			}
			if (line == NULL)  {
				/* catgets 428 */
				fprintf (notif, _i18n_msg_get (ls_catd, NL_SETN, 428, "Corrupted jobfile <%s>\n"), jp->jobSpecs.jobFile); // FIXME FIXME see how much of the corrupted jobfile we can recover
			}
			else {
				int find = FALSE;
				jobStarter = getenv ("LSB_JOB_STARTER"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				/* catgets 429 */
				fputs (_i18n_printf ("\n%s\n", _i18n_msg_get (ls_catd, NL_SETN, 429, "Your job looked like:")), notif);
				fputs("\n------------------------------------------------------------\n", notif);

				/* catgets 430 */
				fprintf (notif, "# LSBATCH: %s\n", I18N (430, "User input")); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

				k = 0;
				while (fgets (line, MAXLINELEN, fp) != NULL)
				{
					if (strstr (line, "# LOGIN_SHELL") != NULL) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						continue;
				}
					if (strstr (line, "SCRIPT_\n") != NULL) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						if (find == FALSE) {
							find = TRUE;
							continue;
						}

						break;
					}

					if (strstr (line, "ExitStat=$") != 0) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
						break;
				}
				else {
					if (++k > 50) {
						fputs (_i18n_printf ("\n(... %s ...)\n", I18N_more), notif);
						break;
					}
					if (k == 1) {
						rmvJobStarterStr (line, jobStarter);
					}
					ls_syslog (LOG_DEBUG, "Job Command: %s", jp->jobSpecs.command); 
					ls_syslog (LOG_DEBUG, "Job line: %s", line);

					if ((k == 1) && (jp->jobSpecs.options2 & SUB2_JOB_CMD_SPOOL)) {
						char *pSep = NULL;
							if ((pSep = strchr (jp->jobSpecs.command, ';')) == NULL) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

							strcpy (line, jp->jobSpecs.command);
						}
						else {
							strncpy (line, jp->jobSpecs.command, (pSep - jp->jobSpecs.command));
						}
						strcat (line, "\n");
					}
					fputs (line, notif);
				}
			}
				fprintf (notif, "------------------------------------------------------------\n\n"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			}
			FCLOSEUP (&fp);
		}

		if (cpuTime > MIN_CPU_TIME) {
			if (WIFEXITED (w_status) && !WEXITSTATUS (w_status)) {
				fprintf (notif, "Successfully completed"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			}
			else {
				if (WIFSIGNALED (w_status)) {
					int sig = WTERMSIG (w_status);

					if (sig > SIGRTMAX ) {
						fprintf (notif, "Exited with unknown status 0%o", jp->w_status); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
					}
					else {
						fprintf (notif, "Exited with signal termination: %s", strsignal (sig)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
					}

					if (WCOREDUMP (w_status)) {
						fprintf (notif, ", and core dumped"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
					}
				}
				else {
					if (jp->jobSpecs.reasons & EXIT_REQUEUE) {
						fprintf (notif, "Job was killed by user or admin, and will be re-queued with the same jobid."); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
					}
					else {
						fprintf (notif, "Exited with exit code %d", WEXITSTATUS (w_status));
					}
				}
			}
		}
		else {
			fprintf (notif, I18N_Exited);
		}

		if (jp->jobSpecs.lastCpuTime + cpuTime > MIN_CPU_TIME) {
			fprintf (notif, ".\n\n%s\n\n", "Resource usage summary:");
			fprintf (notif, "    %s   :%10.2f ", "CPU time", (cpuTime + jp->jobSpecs.lastCpuTime));

			fprintf (notif, "%s.\n", "sec");

			if (jp->maxRusage.mem > 1024) {
				fprintf (notif, "%s :%10d MB\n", "Max Memory", (jp->maxRusage.mem + 512) / 1024);
			}
			else if (jp->maxRusage.mem > 0) {
				fprintf (notif, "%s :%10d KB\n", "Max Memory", jp->maxRusage.mem);
			}

			if (jp->maxRusage.swap > 1024) { // FIXME FIXME FIXME FIXME 1024 should be put into configure.ac // why maxRusage swap is only 1024? why can it not be dynamic?
				fprintf (notif, "    %s   :%10d MB\n\n", "Max Swap", (jp->maxRusage.swap + 512) / 1024);
			}
			else if (jp->maxRusage.swap > 0) {
				fprintf (notif, "    %s   :%10d KB\n\n", "Max Swap", jp->maxRusage.swap);
			}
			if (jp->maxRusage.npids > 0) {
				fprintf (notif, "    %s  :%10d\n", "Max Processes", jp->maxRusage.npids);
			}
		}

		if (!copyStdoutFromLsbatch && (output == notif) && !errorOpeningOutputFile) {
			fprintf (notif, "\n%s\n\n", "The output (if any) is above this job summary.");
		}

		if (notif != output && strcmp (jp->jobSpecs.outFile, "/dev/null") { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			if (errout == output) {
				sprintf (line, "Read file <%s> for stdout and stderr ouput of this job.\n", jp->jobSpecs.outFile);
			}
			else {
				sprintf (line, "Read file <%s> for stdout output of this job.\n", jp->jobSpecs.outFile);
			}
			strcat (ps, line);
			withps = TRUE;
		}

		if (!((jp->jobSpecs.options & SUB_OUT_FILE) && !strcmp (jp->jobSpecs.outFile, "/dev/null")) && copyStdoutFromLsbatch) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

			sprintf (fileName, "%s.out", jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		if ((fp = myfopen_ (fileName, "r", hp)) == NULL) {
			sprintf (line, "Unable to read output data from the stdout buffer file <%s>: your job was probably aborted prematurely.\n", fileName);
			strcat (ps, line);
			withps = TRUE;
		}
		else {
			if ((output == mail) && (mailSizeLimit > 0)	&& (outfileStat.st_size > mailSizeLimit * 1024)) {
				fprintf (output,"\nOutput is larger than limit of %ld KB set by administrator.\n", mailSizeLimit);
					/* catgets 480 */
				fprintf (output, I18N (480, "Output will be saved at %s.out.\n"), jp->jobSpecs.jobFile);
				jp->jobSpecs.jAttrib |= JOB_SAVE_OUTPUT;
			}
			else {
				if (output == notif) {
						/* catgets 447 */
					fprintf (output, "\n%s\n\n", _i18n_msg_get (ls_catd, NL_SETN, 447, "The output (if any) follows:"));
				}
				while ((nItems = fread (line, sizeof (char), MAXLINELEN, fp))) {
					if (fwrite (line, sizeof (char), nItems, output) == 0) {

						sprintf (line, "\nWARNING: writing output file %s failed for job %s\nError message: %s", lsb_jobid2str (jp->jobSpecs.jobId), jp->jobSpecs.outFile, strerror (errno));

						if (sendWarning) {

							if (jp->jobSpecs.options & SUB_MAIL_USER) {
								merr_user (jp->jobSpecs.mailUser, jp->jobSpecs.fromHost, line, I18N_Warning);
							}
							else {
								merr_user (jp->jobSpecs.userName, jp->jobSpecs.fromHost, line, I18N_Warning);
							}
						}
						else {
							fprintf (notif, line);
						}

						break;
					}
				}
			}
			FCLOSEUP (&fp);
		}
	}

	if (!(jp->jobSpecs.options & SUB_ERR_FILE)) {

			sprintf (fileName, "%s.err", jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			if ((fp = myfopen_ (fileName, "r", hp)) != NULL) {
				while ((nItems = fread (line, sizeof (char), MAXLINELEN, fp))) {
					fwrite (line, sizeof (char), nItems, errout);
				}
				FCLOSEUP (&fp);
			}
		}
		else if (strcmp (jp->jobSpecs.errFile, "/dev/null")) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

			sprintf (fileName, "%s.err", jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		if (copyStderrFromLsbatch && (fp = myfopen_ (fileName, "r", hp)) == NULL) {
				/* catgets 448 */
			sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 448, "Unable to read stderr data from stderr buffer file; your job was probably aborted prematurely.\n"));
			strcat (ps, line);
			withps = TRUE;
		}
		else {
			if (errFileError) {
					/* catgets 449 */
				fprintf (errout, "\n\n%s\n\n", _i18n_msg_get (ls_catd, NL_SETN, 449, "PS: The stderr output (if any) follows:"));
			}
			else {
					/* catgets 450 */
				sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 450, "Read file <%s> for stderr output of this job.\n"), jp->jobSpecs.errFile);
				strcat (ps, line);
				withps = TRUE;
			}
			while (copyStderrFromLsbatch && (nItems = fread (line, sizeof (char), MAXLINELEN, fp))) {
				fwrite (line, sizeof (char), nItems, errout);
			}
			FCLOSEUP (&fp);
		}
	}

	fflush (output);
	fflush (errout);
	fflush (notif);

	ofileHost[0] = '\0';
	if (jp->jobSpecs.nxf && output != mail && notif == output) {
		char *host = NULL;
		fstat (fileno (output), &ostat);
		if ((host = ls_getmnthost (jp->jobSpecs.outFile)))
			strcpy (ofileHost, host);
	}
	ofIdx = -1;

	for ( unsigned int i = 0; i < jp->jobSpecs.nxf; i++) {
		struct stat st;
		char *host = NULL;

		if (jp->jobSpecs.xf[i].options & XF_OP_EXEC2SUB) {

			if (ofileHost[0] != '\0' && ofIdx == -1) {
				stat (jp->jobSpecs.xf[i].execFn, &st);
				if ((host = ls_getmnthost (jp->jobSpecs.xf[i].execFn)) &&
					ostat.st_dev == st.st_dev &&
					ostat.st_ino == st.st_ino && !strcmp (host, ofileHost))	{
					ofIdx = i;
				continue;
			}
		}

		xfile = TRUE;

		if (rcpFile (&jp->jobSpecs, jp->jobSpecs.xf + i, jp->jobSpecs.fromHost, XF_OP_EXEC2SUB, rcpMsg) == 0) {
			if (rcpMsg[0] != '\0') {
						/* catgets 451 */
				sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 451, "Copy file <%s> to <%s> on submission host <%s>: %s.\n"), jp->jobSpecs.xf[i].execFn, jp->jobSpecs.xf[i].subFn, jp->jobSpecs.fromHost, rcpMsg);
				strcat (ps, line);
				withps = TRUE;
			}
			continue;
		}

				/* catgets 452 */
		sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 452, "Unable to copy file <%s> to <%s> on submission host <%s>: %s.\n"), jp->jobSpecs.xf[i].execFn, jp->jobSpecs.xf[i].subFn, jp->jobSpecs.fromHost, rcpMsg);
		hasError = -1;
		strcat (ps, line);
		withps = TRUE;
	}
}
if (withps) {
	if (notif == output) {
		fprintf (notif, "\n\nPS:\n\n%s\n", ps);
	}
	else {
		fprintf (notif, "\n%s\n", ps);
	}
}

if (!errorOpeningOutputFile && (output == notif) && ferror (output)) {

	sprintf (line, "\nWARNING: writing to output file %s failed for job %s\nError message: %s", jp->jobSpecs.outFile, lsb_jobid2str (jp->jobSpecs.jobId), strerror (errno));

	if (jp->jobSpecs.options & SUB_MAIL_USER) {
		merr_user (jp->jobSpecs.mailUser, jp->jobSpecs.fromHost, line, I18N_Warning);
	}
	else {
		merr_user (jp->jobSpecs.userName, jp->jobSpecs.fromHost, line, I18N_Warning);
	}
}

mclose (output);
if (errout != output) {
	mclose (errout);
}
if (notif != output) {
	mclose (notif);
}

if (ofIdx >= 0) {
	if (rcpFile (&jp->jobSpecs, jp->jobSpecs.xf + ofIdx, jp->jobSpecs.fromHost, XF_OP_EXEC2SUB, rcpMsg) < 0) {
				/* catgets 454 */
		sprintf (ps, _i18n_msg_get (ls_catd, NL_SETN, 454, "We are unable to copy your output file <%s> to <%s> on submission host <%s> for job <%s>: %s.\n\n"), jp->jobSpecs.xf[ofIdx].execFn, jp->jobSpecs.xf[ofIdx].subFn, jp->jobSpecs.fromHost, lsb_jobid2str (jp->jobSpecs.jobId), rcpMsg);
		hasError = -1;

		if (jp->jobSpecs.options & SUB_MAIL_USER) {
			merr_user (jp->jobSpecs.mailUser, jp->jobSpecs.fromHost, ps, I18N_Warning);
		}
		else {
			merr_user (jp->jobSpecs.userName, jp->jobSpecs.fromHost, ps, I18N_Warning);
		}
	}
	else {
		if (rcpMsg[0] != '\0') {
					 /* catgets 456 */
			sprintf (line, _i18n_msg_get (ls_catd, NL_SETN, 456, "%s: Copy output file <%s> to <%s> on submission host <%s> for job <%s>: %s"),	__func__, jp->jobSpecs.xf[ofIdx].execFn, jp->jobSpecs.xf[ofIdx].subFn, jp->jobSpecs.fromHost, lsb_jobid2str (jp->jobSpecs.jobId), rcpMsg);
			sbdSyslog (LOG_DEBUG, line);
		}
	}
}
}

return hasError; 
}

struct jobCard *
addJob (struct jobSpecs *jobSpecs, int mbdVersion)
{

	struct jobCard *jp = NULL;
	struct passwd *pw = NULL;
	int reply = 0;
	int cc = 0;

	jp = my_calloc (1, sizeof (struct jobCard), __func__);
	memcpy (&jp->jobSpecs, jobSpecs, sizeof (struct jobSpecs));

	if (jobSpecs->execUsername[0] == '\0') {

		jp->execGid = 0;
		jp->execUsername[0] = '\0';
		jp->jobSpecs.execUid = -1;
	}
	else
	{


		if ((pw = getpwlsfuser_ (jobSpecs->execUsername)) == NULL || pw->pw_name == NULL)
		{
			ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobSpecs->jobId), "getpwlsfuser_", jobSpecs->execUsername);
			FREEUP (jp);
			return NULL;
		}
		jp->execGid = pw->pw_gid;
		strcpy (jp->execUsername, jobSpecs->execUsername);


		jp->jobSpecs.execUid = pw->pw_uid;
	}


	if (mbdVersion < 3)
		jp->execJobFlag = JOB_EXEC_STARTED;
	else
	{
		if (jp->jobSpecs.execHome[0] != '\0')
			jp->execJobFlag = JOB_EXEC_STARTED;
		else
			jp->execJobFlag = 0;
	}


	if (jp->jobSpecs.preCmd && jp->jobSpecs.preCmd[0] != '\0' && (jp->jobSpecs.execHome[0] != '\0' || jp->jobSpecs.execCwd[0] != '\0'))
		jp->execJobFlag |= JOB_EXEC_QPRE_OK | JOB_EXEC_QPRE_KNOWN;


	jp->jobSpecs.actValue = jobSpecs->actValue;

	cc = initJobCard (jp, jobSpecs, &reply);
	if (cc < 0)
	{
	  lsb_merr1 (_i18n_msg_get (ls_catd, NL_SETN, 457, "Got garbage jobSpecs for job <%s> from mbatchd upon start: die\n"), lsb_jobid2str (jobSpecs->jobId));   /* catgets 457 */
		die (SLAVE_FATAL);
	}

	if (jp->jobSpecs.jAttrib & QUEUE_ATTRIB_EXCLUSIVE)
		if (lockHosts (jp) < 0)
		{
	ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5421, "%s: lockHosts() failed for job <%s>; Host used by the job will not be locked"), __func__, lsb_jobid2str (jp->jobSpecs.jobId));  /* catgets 5421 */
		}
		renewJobStat (jp);


		if (!(daemonParams[LSB_RENICE_NEVER_AT_RESTART].paramValue))
		{
			if (reniceJob (jp) < 0)
				ls_syslog (LOG_DEBUG, "%s: renice job <%s> failed", __func__, lsb_jobid2str (jp->jobSpecs.jobId));
		}


		jp->runTime = jobSpecs->runTime;

		if (jp->runTime < 0)
		{
			jp->runTime = 0;
		}

		return jp;
	}

	void
	renewJobStat (struct jobCard *jp)
	{
		int k;
		int mbdJobStatus;
		int mbdReasons;
		int mbdSubReasons;
		int sbdLogFound = 0;


		mbdJobStatus = jp->jobSpecs.jStatus;
		mbdReasons = jp->jobSpecs.reasons;
		mbdSubReasons = jp->jobSpecs.subreasons;


		sbdLogFound = sbdread_jobstatus (jp);

		k = 0;

		if (jp->jobSpecs.actPid)
		{

			kill (-jp->jobSpecs.actPid, SIGCONT);
			k = jobsig (jp, SIGCONT, FALSE);
		}
		else
		{


			if (mbdJobStatus & JOB_STAT_EXIT)
			{




				if ((mbdReasons & EXIT_RESTART)
					|| ((mbdReasons == EXIT_ZOMBIE)
						&& !(jp->jobSpecs.options & SUB_CHKPNTABLE)
						&& (jp->jobSpecs.options & SUB_RERUNNABLE)))
				{
					jp->mbdRestarted = TRUE;
				}
				if (jp->jobSpecs.jobPid == 0)
				{

					jp->jobSpecs.jStatus = mbdJobStatus;
					return;
				}
				jobSigStart (jp, SIG_TERM_OTHER, 0, 0, SIGLOG);
			}
			else
			{
				if (sbdLogFound == 0)
				{

					switch (MASK_STATUS (jp->jobSpecs.jStatus))
					{
						case JOB_STAT_PEND:
						k = 0;

						if (!(mbdJobStatus & JOB_STAT_PEND))
							jp->notReported++;
						break;

						case JOB_STAT_RUN:
						jp->migCnt = 1;
						if (!(mbdJobStatus & JOB_STAT_RUN))
							jp->notReported++;
						break;

						case JOB_STAT_SSUSP:
						case JOB_STAT_USUSP:

						k = sbdStartupStopJob (jp, jp->jobSpecs.reasons,
							jp->jobSpecs.subreasons);

						if (!(IS_SUSP (mbdJobStatus)))
							jp->notReported++;
						break;

					}

				}
				else
				{


					switch (MASK_STATUS (mbdJobStatus))
					{
						case JOB_STAT_PEND:
						k = 0;
						break;
						case JOB_STAT_RUN:
						jp->migCnt = 1;
						k = resumeJob (jp, SIG_RESUME_OTHER, 0, SIGLOG);
						break;

						case JOB_STAT_SSUSP:
						case JOB_STAT_USUSP:
						k = sbdStartupStopJob (jp, mbdReasons, mbdSubReasons);
						break;

					}
				}
			}
		}

		if (k < 0)
			jobGone (jp);
	}


	int
	sbdStartupStopJob (struct jobCard *jp, int reasons, int subReasons)
	{
		int sigValue = 0;

		if (reasons & SUSP_USER_STOP)
			sigValue = SIG_SUSP_USER;
		else if (reasons & SUSP_QUEUE_WINDOW)
			sigValue = SIG_SUSP_WINDOW;
		else if (reasons & LOAD_REASONS)
			sigValue = SIG_SUSP_LOAD;
		else if (reasons & OTHER_REASONS)
			sigValue = SIG_SUSP_OTHER;

		if (sigValue == 0)
			return 0; 

		reasons |= SUSP_SBD_STARTUP;

		return (jRunSuspendAct (jp, sigValue, ((reasons & SUSP_USER_STOP) ? JOB_STAT_USUSP : JOB_STAT_SSUSP), reasons, subReasons, NO_SIGLOG));

	}

	void
	jobGone (struct jobCard *jp)
	{
		if (logclass & LC_EXEC)
			ls_syslog (LOG_DEBUG, "%s: Checking if job %s pid %d gone, jp->w_status=%d", __func__, lsb_jobid2str (jp->jobSpecs.jobId), jp->jobSpecs.jobPid, jp->w_status);



		if (!jp->missing)
		{
			jp->missing = TRUE;
			need_checkfinish = TRUE;
			if (jp->notReported > 0)
				jp->notReported = 0;
		}
		else
		{
			if (getJobVersion (&jp->jobSpecs) < 4)
			{

				if (jp->exitPid == 0)
				{
					if (jp->postJobStarted == 0)
					{
						jobFileExitStatus (jp);
					}
				}
				if (jp->exitPid >= 0)
					return;
			}
			else
			{
				jp->exitPid = -1;

				if (jp->postJobStarted == 0)
				{
					jobFinishRusage (jp);
				}
			}
			if (jp->jobSpecs.actPid == 0 && jp->exitPid == -1)
			{

				if (requeueJob (jp) == TRUE)
				{
					SBD_SET_STATE (jp, JOB_STAT_PEND);
					jp->jobSpecs.reasons = PEND_SBD_JOB_REQUEUE;
				}
				else
				{
					if (jp->postJobStarted != 0)
					{
						if (jp->w_status)
						{
							SBD_SET_STATE (jp, JOB_STAT_PERR);
						}
						else
						{
							SBD_SET_STATE (jp, JOB_STAT_PDONE);
						}
					}
					else
					{
						if (jp->w_status)
						{
							SBD_SET_STATE (jp, JOB_STAT_EXIT);
						}
						else
						{
							SBD_SET_STATE (jp, JOB_STAT_DONE);
						}
					}
				}
			}
			jp->notReported++;
		}
	}

void
refreshJob (struct jobSpecs *jobSpecs)
{

struct jobCard *jp;
char *word, *cp;
int i, j;

for (jp = jobQueHead->forw; jp != jobQueHead; jp = jp->forw)
{
	if (jp->jobSpecs.jobId != jobSpecs->jobId)
		continue;


	jp->jobSpecs.reasons = jobSpecs->reasons;
	jp->jobSpecs.subreasons = jobSpecs->subreasons;


	if (!(jp->postJobStarted)
		&& (jp->jobSpecs.jStatus != jobSpecs->jStatus))
	{

		jp->jobSpecs.jStatus = (jobSpecs->jStatus & ~JOB_STAT_MIG) |
		(jp->jobSpecs.jStatus & JOB_STAT_MIG);

		renewJobStat (jp);
	}
	strcpy (jp->jobSpecs.queue, jobSpecs->queue);
	strcpy (jp->jobSpecs.resumeCond, jobSpecs->resumeCond);
	strcpy (jp->jobSpecs.stopCond, jobSpecs->stopCond);
	strcpy (jp->jobSpecs.suspendActCmd, jobSpecs->suspendActCmd);
	strcpy (jp->jobSpecs.resumeActCmd, jobSpecs->resumeActCmd);
	strcpy (jp->jobSpecs.terminateActCmd, jobSpecs->terminateActCmd);

	for (i = 0; i < LSB_SIG_NUM; i++)
		jp->jobSpecs.sigMap[i] = jobSpecs->sigMap[i];

	jp->jobSpecs.priority = jobSpecs->priority;
	jp->jobSpecs.nice = jobSpecs->nice;
	jp->jobSpecs.migThresh = jobSpecs->migThresh;
	jp->jobSpecs.chkPeriod = jobSpecs->chkPeriod;


	freeToHostsEtc (&jp->jobSpecs);
	saveSpecs (&jp->jobSpecs, jobSpecs);

	jp->jobSpecs.jAttrib = jobSpecs->jAttrib;
	if (jp->jobSpecs.jAttrib & QUEUE_ATTRIB_EXCLUSIVE)
	{
		if (lockHosts (jp) < 0)
		{
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5421, "%s: lockHosts() failed for job <%s>; Host used by the job will not be locked"), __func__, lsb_jobid2str (jp->jobSpecs.jobId));
		}
	}
	for (j = 0; j < LSF_RLIM_NLIMITS; j++)
		memcpy ((char *) &jp->jobSpecs.lsfLimits[j], (char *) &jobSpecs->lsfLimits[j], sizeof (struct lsfLimit));
	setRunLimit (jp, FALSE);

	if ((strcmp (jp->jobSpecs.windows, jobSpecs->windows)) != 0)
	{
		freeWeek (jp->week);
		cp = jobSpecs->windows;
		while ((word = getNextWord_ (&cp)) != NULL)
		{
			if (addWindow (word, jp->week, "refreshJobs jobSpecs") < 0)
			{
				ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__,
					lsb_jobid2str (jp->jobSpecs.jobId), "addWindow",
					word);
		  lsb_merr (_i18n_msg_get (ls_catd, NL_SETN, 458, "Got garbage job bill from mbatchd on restart: die\n"));  /* catgets 458 */
				die (SLAVE_FATAL);
			}
		}
		strcpy (jp->jobSpecs.windows, jobSpecs->windows);
		jp->windEdge = now;
	}
	lsbFreeResVal (&jp->resumeCondVal);
	if (jobSpecs->resumeCond && jobSpecs->resumeCond[0] != '\0')
	{
		if ((jp->resumeCondVal = checkThresholdCond (jobSpecs->resumeCond)) == NULL)
			ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), __func__, "resumeCond");
	}

	lsbFreeResVal (&jp->stopCondVal);
	if (jobSpecs->stopCond && jobSpecs->stopCond[0] != '\0')
	{
		if ((jp->stopCondVal = checkThresholdCond (jobSpecs->stopCond))	== NULL)
			ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), __func__, "stopCond");
	}


	jp->needReportRU = TRUE;

	if (!(daemonParams[LSB_RENICE_NEVER_AT_RESTART].paramValue))
	{
		if (reniceJob (jp) < 0)
			ls_syslog (LOG_DEBUG, "refreshJob: reniceJob job <%s> failed",
				lsb_jobid2str (jp->jobSpecs.jobId));
	}
}
}


void
inJobLink (struct jobCard *jp)
{
	struct jobCard *jobp;


	for (jobp = jobQueHead->forw; jobp != jobQueHead; jobp = jobp->forw)
	{
		if (jp->jobSpecs.priority < jobp->jobSpecs.priority)
			break;
		else if ((jp->jobSpecs.priority == jobp->jobSpecs.priority)	&& (jp->jobSpecs.startTime - jobp->jobSpecs.startTime) >= 0)
			break;
	}
	inList ((struct listEntry *) jobp, (struct listEntry *) jp);

}

int
setIds (struct jobCard *jobCardPtr)
{
struct jobSpecs *jobSpecsPtr = &(jobCardPtr->jobSpecs);
char *tGname, *sGname = NULL;
struct group *grEntry;

if (!debug)
{

	if (initgroups (jobCardPtr->execUsername, jobCardPtr->execGid) < 0)
	{
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobSpecsPtr->jobId), "initgroups", jobCardPtr->execUsername);
		return -1; 
	}


	if (setgid (jobCardPtr->execGid) < 0)
	{
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_D_M, __func__, lsb_jobid2str (jobSpecsPtr->jobId), "setgid",(int) jobCardPtr->execGid);
		return -1; 
	}


	if ((tGname = getenv ("LSB_UNIXGROUP"))) // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	{
		if (logclass & (LC_TRACE | LC_EXEC))
			ls_syslog (LOG_DEBUG, "got the LSB_UNIXGROUP=%s", tGname);

		if (tGname != NULL)
		{
			sGname = putstr_ (tGname);
			if ((grEntry = getgrnam (sGname)) == NULL)
			{
				ls_syslog (LOG_DEBUG, "setIds: Job <%s> getgrnam(%s) failed: ", lsb_jobid2str (jobSpecsPtr->jobId), sGname);
			}
			else
			{

				char **memp = grEntry->gr_mem;
				while (memp != NULL && *memp != NULL)
				{
					if (strcmp (*memp, jobCardPtr->execUsername) == 0)
					{


						if (setgid (grEntry->gr_gid) < 0)
						{
							ls_syslog (LOG_DEBUG, "setIds: Job <%s> setgid(%d) for LSB_UNIXGROUP=%s  failed: %m", lsb_jobid2str (jobSpecsPtr->jobId),
								(int) grEntry->gr_gid, sGname);
						}
						else
						{

							jobCardPtr->execGid = grEntry->gr_gid;
						}
						break;
					}
					memp++;
				}
			}


			FREEUP (sGname);
		}
	}


	if ((jobSpecsPtr->options & SUB_INTERACTIVE) &&
		(jobSpecsPtr->options & SUB_PTY))
	{
		chuser (jobSpecsPtr->execUid);
	}
	else if (lsfSetUid (jobSpecsPtr->execUid) < 0)
	{
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobSpecsPtr->jobId), "setuid", jobCardPtr->execUsername);
		return -1; 
	}

	if (logclass & LC_EXEC)
		ls_syslog (LOG_DEBUG, "setIds: Job <%s> uid %d euid %d", lsb_jobid2str (jobSpecsPtr->jobId), getuid (), geteuid ());
}
return 0; 
}


void
deallocJobCard (struct jobCard *jobCard)
{
char fileBuf[MAX_FILENAME_LEN];

sprintf (fileBuf, "%s/.%s.%s.fail", LSTMPDIR, jobCard->jobSpecs.jobFile, lsb_jobidinstr (jobCard->jobSpecs.jobId));

if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

sprintf (fileBuf, "%s/.%s.%s.chk", LSTMPDIR, jobCard->jobSpecs.jobFile,	lsb_jobidinstr (jobCard->jobSpecs.jobId)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__,
		lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

sprintf (fileBuf, "%s/.%s.%s.resume", LSTMPDIR, jobCard->jobSpecs.jobFile, lsb_jobidinstr (jobCard->jobSpecs.jobId));
if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__,lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

sprintf (fileBuf, "%s/.%s.%s.suspend", LSTMPDIR, jobCard->jobSpecs.jobFile, lsb_jobidinstr (jobCard->jobSpecs.jobId)); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

sprintf (fileBuf, "%s/.%s.%s.terminate", LSTMPDIR,
	jobCard->jobSpecs.jobFile,
	lsb_jobidinstr (jobCard->jobSpecs.jobId));
if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

sprintf (fileBuf, "%s/.%s.acct", LSTMPDIR, jobCard->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

sprintf (fileBuf, "%s/.%s.%d.stt", LSTMPDIR, lsb_jobidinstr (jobCard->jobSpecs.jobId), jobCard->jobSpecs.jobPid);
if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

sprintf (fileBuf, "%s/.%s.sbd/jobstatus.%s", LSTMPDIR, clusterName,	lsb_jobidinstr (jobCard->jobSpecs.jobId));

if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);


sprintf (fileBuf, "/tmp/.sbd/%s.rusage", jobCard->jobSpecs.jobFile);
if (logclass & LC_MPI)
{
	ls_syslog (LOG_DEBUG3, "deallocJobCard: unlink(%s) for MPI job <%s>: %m", fileBuf,
		lsb_jobid2str (jobCard->jobSpecs.jobId));
}
if (unlink (fileBuf) < 0 && errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), "unlink", fileBuf);

offList ((struct listEntry *) jobCard);
freeWeek (jobCard->week);
freeToHostsEtc (&jobCard->jobSpecs);


if (jobCard->runRusage.npgids > 0)
{
	FREEUP (jobCard->runRusage.pgid);
}

if (jobCard->runRusage.npids > 0)
{
	FREEUP (jobCard->runRusage.pidInfo);
}

if (jobCard->mbdRusage.npgids > 0)
{
	FREEUP (jobCard->mbdRusage.pgid);
}

if (jobCard->mbdRusage.npids > 0)
{
	FREEUP (jobCard->mbdRusage.pidInfo);
}

if (jobCard->client)
{
	shutDownClient (jobCard->client);
}

lsbFreeResVal (&jobCard->resumeCondVal);
lsbFreeResVal (&jobCard->stopCondVal);

free (jobCard);
jobcnt--;
}

void
freeToHostsEtc (struct jobSpecs *jobSpecs)
{
	int i;

	if (jobSpecs == NULL)
		return;

	for (i = 0; i < jobSpecs->numToHosts; i++)
	{
		FREEUP (jobSpecs->toHosts[i]);
	}
	FREEUP (jobSpecs->toHosts);

	freeThresholds (&jobSpecs->thresholds);

	if (jobSpecs->nxf)
		FREEUP (jobSpecs->xf);

	if (jobSpecs->numEnv > 0)
	{
		for (i = 0; i < jobSpecs->numEnv; i++)
			FREEUP (jobSpecs->env[i]);
		FREEUP (jobSpecs->env);
		jobSpecs->numEnv = 0;
	}

	if (jobSpecs->eexec.len > 0)
	{
		FREEUP (jobSpecs->eexec.data);
		jobSpecs->eexec.len = 0;
	}
	FREEUP (jobSpecs->loginShell);
	FREEUP (jobSpecs->schedHostType);
	FREEUP (jobSpecs->execHosts);

}

void
saveSpecs (struct jobSpecs *jobSpecs, struct jobSpecs *specs)
{
int i;

jobSpecs->toHosts = my_calloc (specs->numToHosts,
	sizeof (char *), __func__);
for (i = 0; i < specs->numToHosts; i++)
{
	jobSpecs->toHosts[i] = safeSave (specs->toHosts[i]);
}
jobSpecs->numToHosts = specs->numToHosts;

saveThresholds (jobSpecs, &specs->thresholds);
if (specs->nxf)
	jobSpecs->xf = my_calloc (specs->nxf, sizeof (struct xFile), __func__);
else
	jobSpecs->xf = NULL;
for (i = 0; i < specs->nxf; i++)
	memcpy ((char *) &(jobSpecs->xf[i]), (char *) &specs->xf[i],
		sizeof (struct xFile));
jobSpecs->nxf = specs->nxf;

if (specs->numEnv > 0)
{
	jobSpecs->env = (char **)
	my_calloc (specs->numEnv, sizeof (char *), __func__);
	for (i = 0; i < specs->numEnv; i++)
		jobSpecs->env[i] = safeSave (specs->env[i]);
	jobSpecs->numEnv = specs->numEnv;
}
else
{
	jobSpecs->numEnv = 0;
	jobSpecs->env = NULL;
}

jobSpecs->eexec.len = specs->eexec.len;
if (jobSpecs->eexec.len > 0)
{
	jobSpecs->eexec.data = my_malloc (jobSpecs->eexec.len, __func__);
	memcpy (jobSpecs->eexec.data, specs->eexec.data, jobSpecs->eexec.len);
}
else
{
	jobSpecs->eexec.data = NULL;
	jobSpecs->eexec.len = 0;
}
jobSpecs->loginShell = safeSave (specs->loginShell);
jobSpecs->schedHostType = safeSave (specs->schedHostType);
if (specs->execHosts != NULL)
	jobSpecs->execHosts = safeSave (specs->execHosts);

}



void
setRunLimit (struct jobCard *jp, int initRunTime)
{
	if (jp->jobSpecs.lsfLimits[LSF_RLIMIT_RUN].rlim_curh != 0)
	{


		jp->jobSpecs.lsfLimits[LSF_RLIMIT_RUN].rlim_curl = 0x7fffffff;
	}

	if (initRunTime)
		jp->runTime = 0;
	return;

}

int
setPGid (struct jobCard *jc)
{
	static char __func__] = "setPGid()";


if (setpgid (0, getpid ()) < 0)
{
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jc->jobSpecs.jobId), "setpgid");
	return -1; 
}

return getpid (); 
}



char *
getLoginShell (char *jfData, char *jobFile, struct hostent *hp, int readFile)
{
	static char shellPath[MAX_FILENAME_LEN];
	int i = 0;
	char *sp;
	char line[MAXLINELEN];
	FILE *fp;


	if (!readFile)
	{
		if (jfData == NULL)
			return NULL; 
		if ((sp = strstr (jfData, "\n# LOGIN_SHELL ")) == NULL) // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			return NULL; 

		sp += strlen ("\n# LOGIN_SHELL "); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		while (*sp != '\n')
		{
			shellPath[i] = *sp;
			i++;
			sp++;
		}
		shellPath[i] = '\0';
		return shellPath; 
	}


	if ((fp = myfopen_ (jobFile, "r", hp)) == NULL)
	{
	  fprintf (stderr, _i18n_msg_get (ls_catd, NL_SETN, 459, "Cannot open your job file: %s\n"),    /* catgets 459 */
		jobFile);
	  exit (-1);
	}

	while (fgets (line, MAXLINELEN, fp) != NULL)
	{
		if (!strcmp (line, CMDSTART))
		{
			break;
		}
	}
	if (line == NULL)
	{
	  fprintf (stderr, _i18n_msg_get (ls_catd, NL_SETN, 460, "Corrupted jobfile <%s>\n"),   /* catgets 460 */
		jobFile);
	  exit (-1);
	}

	if (fgets (line, MAXLINELEN, fp) != NULL)
	{

		if ((sp = strstr (line, "# LOGIN_SHELL ")) == NULL) // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		{
			FCLOSEUP (&fp);
			return NULL; 
		}
		sp += strlen ("# LOGIN_SHELL "); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		while (*sp != '\n')
		{
			shellPath[i] = *sp;
			i++;
			sp++;
		}
		shellPath[i] = '\0';
		FCLOSEUP (&fp);
		return shellPath; 
	}
	FCLOSEUP (&fp);
	return NULL; 

}

int
createTmpJobFile (struct jobSpecs *jobSpecsPtr, struct hostent *hp, char *stdinFile)
{

char errMsg[MAXLINELEN];
char path[MAX_FILENAME_LEN];
char cmdBuf[MAXLINELEN];
char *sp = NULL;
int fd, len, size, i;
char **execArgv, *argv[2];

if (logclass & LC_EXEC)
{
	sprintf (errMsg, "%s: entering, jobFile=<%s>", __func__, jobSpecsPtr->jobFile);
	sbdSyslog (LOG_DEBUG3, errMsg);
}

sprintf (path, "%s.tmp", jobSpecsPtr->jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
if ((fd = myopen_ (path, O_CREAT | O_TRUNC | O_WRONLY, 0600, hp)) == -1)
{
	sprintf (cmdBuf, I18N_JOB_FAIL_S_S_M, __func__,
		lsb_jobid2str (jobSpecsPtr->jobId), "myopen_", path); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	goto Error;
}
if (stdinFile == NULL)
	stdinFile = "/dev/null"; // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac


sprintf (cmdBuf, "cd %s", jobSpecsPtr->execCwd); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

argv[0] = jobSpecsPtr->jobFile;
argv[1] = NULL;

execArgv = execArgs (jobSpecsPtr, argv);



size = 2 * (strlen (jobSpecsPtr->jobFile) + strlen (stdinFile))
+ strlen (cmdBuf) + 25;

for (i = 0; execArgv[i]; i++)
	size += 2 * (strlen (execArgv[i]) + 1);

sp = malloc (size);
if (sp == NULL)
{
	sprintf (cmdBuf, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jobSpecsPtr->jobId), "malloc");
	goto Error;
}

sprintf (sp, "%s\nexec", cmdBuf); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
for (i = 0; execArgv[i]; i++)
{
	strcat (sp, " ");
	strcat (sp, execArgv[i]);
}

strcat (sp, " <"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
strcat (sp, stdinFile);
strcat (sp, "\n");

for (i = 0; execArgv[i]; i++)
{
	strcat (sp, " ");
	strcat (sp, execArgv[i]);
}

strcat (sp, " <"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
strcat (sp, stdinFile);
strcat (sp, "\n");

len = strlen (sp);
if (write (fd, sp, len) != len)
{
	sprintf (cmdBuf, I18N_FUNC_S_FAIL_M, __func__, "write", jobSpecsPtr->jobFile);
	goto Error;
}
FREEUP (sp);
if (close (fd) == -1)
{
	sprintf (cmdBuf, I18N_FUNC_S_FAIL_M, __func__, "close",	jobSpecsPtr->jobFile);
	goto Error;
}

sprintf (path, "%s.tmp", jobSpecsPtr->jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
if ((fd = myopen_ (path, O_RDONLY, 0600, hp)) == -1)
{
	sprintf (cmdBuf, I18N_JOB_FAIL_S_S_M, __func__,  lsb_jobid2str (jobSpecsPtr->jobId), "myopen_", path);
	goto Error;
}

if (dup2 (fd, 0) == -1)
{
	sprintf (cmdBuf, I18N_FUNC_D_FAIL_M, __func__, "dup2", fd); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	goto Error;
}

if (logclass & LC_EXEC)
{
	sprintf (errMsg, "%s: leaving", __func__);
	sbdSyslog (LOG_DEBUG3, errMsg);
}
return 0; 

Error:
if (logclass & LC_EXEC)
{
	sprintf (errMsg, "%s: %s", __func__, cmdBuf);
	sbdSyslog (LOG_DEBUG3, errMsg);
}
fprintf (stderr, "%s\n", cmdBuf);
FREEUP (sp);

return -1; 

}


int
acctMapTo (struct jobCard *jobCard)
{
char *sp = NULL;
char *hostname = NULL;
char clusorhost[MAX_LSB_NAME_LEN];
char user[MAX_LSB_NAME_LEN];
char line[MAXLINELEN];
struct passwd *pw;
int num = 0;
int ccount;
int i = 0;
int found = FALSE;
char myhost = '\0';
char mycluster = '\0';
struct hostent *hp = NULL;

if (jobCard->jobSpecs.userId == PC_LSF_CUGID)
{
	goto trySubUser;
}

for (i = 0; i < jobCard->jobSpecs.numEnv; i++)
{
	if (logclass & LC_EXEC)
		ls_syslog (LOG_DEBUG2, "%s: Job <%s>, env[%d]=%s", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), i, jobCard->jobSpecs.env[i]);
	if ((sp = strstr (jobCard->jobSpecs.env[i], "LSB_ACCT_MAP=")) == NULL) // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		continue;
	break;
}

if ((hostname = ls_getmyhostname ()) == NULL)
{
	ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_getmyhostname");
	die (SLAVE_FATAL);
}

if (sp != NULL)
{
	if (strlen (sp) == 13)
	{
		sp = NULL;
	}
}
if (!sp)
{
	if (logclass & LC_EXEC)
	{
		ls_syslog (LOG_DEBUG1, "%s: Job <%s>, no user level account mapping. Trying submission user", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId));
	}
	goto trySubUser;
}

sp += 13;
i = 0;
while (*sp != '\n' && *sp != '\0' && i < MAXLINELEN)
{
	line[i++] = *sp++;
}
line[i] = '\0';
sp = line;

if (logclass & LC_EXEC)
	ls_syslog (LOG_DEBUG1, "%s: Job <%s>, using map <%s>", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), line);

while ((num = sscanf (sp, "%s %s%n", clusorhost, user, &ccount)) == 2)
{
	if (logclass & LC_EXEC)
		ls_syslog (LOG_DEBUG2, "%s: checking <%s> <%s>", __func__, clusorhost, user);
	sp += ccount + 1;


	if (strcmp (clusterName, clusorhost) == 0
		|| strcmp ("+", clusorhost) == 0)
	{
		mycluster = 1;
	}

	if (!mycluster)
	{
		if (((hp = Gethostbyname_ (clusorhost)) != NULL)
			&& equalHost_ (hp->h_name, hostname))
			myhost = TRUE;
		else
			myhost = FALSE;
	}

	if (!mycluster && !myhost)
	{
		if (logclass & LC_EXEC)
			ls_syslog (LOG_DEBUG2, "%s: <%s> not local cluster or host name", __func__, clusorhost);
		continue;
	}

	if ((pw = getpwlsfuser_ (user)) == (struct passwd *) NULL)
	{
		if (logclass & LC_EXEC)
			ls_syslog (LOG_DEBUG2, "%s: Account <%s> non-existent", __func__, user);
		continue;
	}

	found = TRUE;
	strcpy (jobCard->execUsername, user);
	strcpy (jobCard->jobSpecs.execUsername, user);
	jobCard->jobSpecs.execUid = pw->pw_uid;
	jobCard->execGid = pw->pw_gid;


	if (myhost)
		break;
}
if (found)
	return 0; 

if (logclass & LC_EXEC)
{
	ls_syslog (LOG_DEBUG1, "%s: No valid map found in user level account map for job <%s>. Trying submission user", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId));
}

trySubUser:

if ((pw = my_getpwnam (jobCard->jobSpecs.userName, "acctMapTo/trySubUser")) == NULL)
{
		/* catgets 5441 */
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5441, "%s: No valid user name found for job <%s>, userName <%s>. getpwnam() failed:%m"), __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), jobCard->jobSpecs.userName);
	}
	else
	{
		if (jobCard->jobSpecs.userId == PC_LSF_CUGID)
		{
			/* catgets 5497 */
	  ls_syslog (LOG_INFO, (_i18n_msg_get (ls_catd, NL_SETN, 5497, "%s: User %s is from a PC client host %s, will use uid %d on SBD host")), __func__, jobCard->jobSpecs.userName, jobCard->jobSpecs.fromHost, pw->pw_uid);
	}
	strcpy (jobCard->execUsername, jobCard->jobSpecs.userName);
	jobCard->execGid = pw->pw_gid;
	jobCard->jobSpecs.execUid = pw->pw_uid;
	strcpy (jobCard->jobSpecs.execUsername, jobCard->jobSpecs.userName);
	return 0; 
}
return -1; 

}



int
acctMapOk (struct jobCard *jobCard)
{
	char errMsg[MAXLINELEN];
	char hostfn[MAX_FILENAME_LEN];
	char msg[MAXLINELEN * 2];
	char clusorhost[MAX_LSB_NAME_LEN];
	char user[MAX_LSB_NAME_LEN];
	struct passwd *pw;
	struct stat statbuf;
	FILE *fp;
	char dir[30];
	char *line;
	int num;


	if ((!UID_MAPPED (jobCard)))
	{
		return 0; 
	}

	if ((pw = getpwuid (jobCard->jobSpecs.execUid)) == NULL)
	{
		sprintf (errMsg, "%s: Job %s getpwuid %d failed: %s", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), jobCard->jobSpecs.execUid, strerror (errno));
		sbdSyslog (LOG_ERR, errMsg);
		return -1; 
	}

	strcpy (hostfn, pw->pw_dir);
	strcat (hostfn, "/.lsfhosts"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	if ((fp = fopen (hostfn, "r")) == NULL)
	{
		sprintf (errMsg, "%s: Job %s fopen() for file %s failed: %s", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), hostfn, strerror (errno));

		sbdSyslog (LOG_ERR, errMsg);
		goto error;
	}
	if ((fstat (fileno (fp), &statbuf) < 0) ||
		(statbuf.st_uid != 0 && statbuf.st_uid != pw->pw_uid) ||
		(statbuf.st_mode & 066))
	{
		sprintf (errMsg, "%s: Job %s file %s is not owned or writable only by the user, file uid %d, file mode %03o, userId %d", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), hostfn, (int) statbuf.st_uid, (unsigned int) statbuf.st_mode, (int) pw->pw_uid);
		sbdSyslog (LOG_ERR, errMsg);
		FCLOSEUP (&fp);
		goto error;
	}

	while ((line = getNextLine_ (fp, TRUE)) != NULL)
	{
		struct hostent *hp;

		num = sscanf (line, "%s %s %s", clusorhost, user, dir);
		if (num < 2	|| (num == 3 && (strcmp (dir, "send") == 0 || strcmp (dir, "recv") != 0)))
			continue;

		if (strcmp (user, "+") == 0	|| strcmp (user, jobCard->jobSpecs.userName) == 0)
		{

			if (strcmp (jobCard->jobSpecs.clusterName, clusorhost) == 0 || strcmp (clusorhost, "+") == 0)
				return 0; 
			if ((hp = Gethostbyname_ (clusorhost)) == NULL)
				continue;
			if (equalHost_ (jobCard->jobSpecs.fromHost, hp->h_name))
				return 0; 
		}
	}

	sprintf (errMsg, "%s: Job %s no authorization for user name %s from cluster/host %s/%s in file %s", __func__, lsb_jobid2str (jobCard->jobSpecs.jobId), jobCard->jobSpecs.userName, clusterName, jobCard->jobSpecs.fromHost, hostfn);
	sbdSyslog (LOG_ERR, errMsg);

	error:
	sprintf (msg, "We are unable to start your job %s %s.\nThe error is: %s", lsb_jobid2str (jobCard->jobSpecs.jobId), jobCard->jobSpecs.command, errMsg);

	if (jobCard->jobSpecs.options & SUB_MAIL_USER)
		merr_user (jobCard->jobSpecs.mailUser, jobCard->jobSpecs.fromHost, msg, "error");
	else
		merr_user (jobCard->jobSpecs.userName, jobCard->jobSpecs.fromHost, msg, "error");

	return -1;
}

void
runQPre (struct jobCard *jp)
{
	pid_t pid;
	int i;
	char errMsg[MAXLINELEN];

	if (!jp->jobSpecs.preCmd || jp->jobSpecs.preCmd[0] == '\0')
		return;

	if ((pid = fork ()) == 0)
	{
	  /* child */
		sigset_t newmask;
		char *myargv[6];
		int maxfds = sysconf (_SC_OPEN_MAX);

		chdir ("/tmp"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		if (chPrePostUser (jp) < 0)
		{
			ls_syslog (LOG_ERR, "%s: queue's pre-exec chPrePostUser failed for job <%d>", __func__, jp->jobSpecs.jobId);
			exit (-1);
		}

		for (i = 1; i < NSIG; i++)
			Signal_ (i, SIG_DFL);

		sigemptyset (&newmask);
		sigprocmask (SIG_SETMASK, &newmask, NULL);

		alarm (0);

		putEnv ("PATH", "/bin:/usr/bin:/sbin:/usr/sbin"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

		i = open ("/dev/null", O_RDWR, 0); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		dup2 (i, 0);
		dup2 (i, 1);
		dup2 (i, 2);

		for (i = 3; i < maxfds; i++)
			close (i);

		myargv[0] = "/bin/sh"; // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		myargv[1] = "-c"; // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		myargv[2] = jp->jobSpecs.preCmd;
		myargv[3] = NULL;

		execvp ("/bin/sh", myargv); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		sprintf (errMsg, "%s: queue's pre-exec command %s failed for job %s: %s", __func__, jp->jobSpecs.preCmd, lsb_jobid2str (jp->jobSpecs.jobId), strerror (errno));
		sbdSyslog (LOG_ERR, errMsg);
		exit (-1);
	}

	if (pid < 0)
	{
		ls_syslog (LOG_ERR, "%s: Fork to run pre-exec for job <%d> failed: %m", __func__, jp->jobSpecs.jobId);
		jobSetupStatus (JOB_STAT_PEND, PEND_QUEUE_PRE_FAIL, jp);
	}

	collectPreStatus (jp, pid, "runQPre"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	jp->execJobFlag |= JOB_EXEC_QPRE_OK;
}

int
runQPost (struct jobCard *jp)
{
	int i;
	int pid;
	int maxfds;
	char val[MAXLINELEN];
	char *myargv[6];
	sigset_t newmask;

	if (logclass & LC_TRACE)
	{
		chuser (batchId);
		ls_syslog (LOG_DEBUG, "%s: queue's post-command %s pre-command %s execJobFlag %x for job %d status %x", __func__, jp->jobSpecs.postCmd ? jp->jobSpecs.postCmd : "NULL", jp->jobSpecs.preCmd ? jp->jobSpecs.preCmd : "NULL", jp->execJobFlag, lsb_jobid2str (jp->jobSpecs.jobId), jp->w_status);
		chuser (jp->jobSpecs.execUid);
	}

	if (!jp->jobSpecs.postCmd || jp->jobSpecs.postCmd[0] == '\0')
		return 0;

	if (jp->jobSpecs.preCmd && jp->jobSpecs.preCmd[0] != '\0')
	{

		if (!(jp->execJobFlag & JOB_EXEC_QPRE_KNOWN))
		{

			chuser (batchId);
			ls_syslog (LOG_WARNING, "%s: qpost is not run for job %d because status of qpre unknown", __func__, jp->jobSpecs.jobId);
			return -1;
		}

		if (!(jp->execJobFlag & JOB_EXEC_QPRE_OK))
		{

			chuser (batchId);
			ls_syslog (LOG_WARNING, "%s: qpost is not run for job % because qpre failed", __func__, lsb_jobid2str (jp->jobSpecs.jobId), chuser (jp->jobSpecs.execUid));
			return -1;
		}
	}

	if ((pid = fork ()) == -1)
	{
		chuser (batchId);
		ls_syslog (LOG_ERR, "%s: fork() failed for job %d: %m", __func__, jp->jobSpecs.jobId);
		chuser (jp->jobSpecs.execUid);
		return -1;
	}

	if (pid)
	{
		if (waitpid (pid, NULL, 0) < 0)
		{
			chuser (batchId);
			ls_syslog (LOG_ERR, "%s: waitpid(%d) failed for qpre for job <%d>: %m", __func__, pid, jp->jobSpecs.jobId);
			chuser (jp->jobSpecs.execUid);
		}
		return -1;
	}

	chdir ("/tmp"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	chuser (batchId);

	if (chPrePostUser (jp) < 0)
	{
		ls_syslog (LOG_ERR, "%s: queue's post-exec chPrePostUser failed for job <%d>", __func__, jp->jobSpecs.jobId);
		exit (-1);
	}

	for (i = 1; i < NSIG; i++) {
		Signal_ (i, SIG_DFL);
	}

	sigemptyset (&newmask);
	sigprocmask (SIG_SETMASK, &newmask, NULL);
	alarm (0);

	putEnv ("PATH", "/bin:/usr/bin:/sbin:/usr/sbin"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	i = open ("/dev/null", O_RDWR, 0); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	dup2 (i, 0);
	dup2 (i, 1);
	dup2 (i, 2);

	maxfds = sysconf (_SC_OPEN_MAX);
	for (i = 3; i < maxfds; i++)
	{
		close (i);
	}

	sprintf (val, "%d", jp->w_status);
	putEnv ("LSB_JOBEXIT_STAT", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	if (jp->jobSpecs.jStatus & JOB_STAT_PEND)
		putEnv ("LSB_JOBPEND", " "); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	myargv[0] = "/bin/sh"; // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	myargv[1] = "-c"; // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	myargv[2] = jp->jobSpecs.postCmd;
	myargv[3] = NULL;

	execvp ("/bin/sh", myargv); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac


	sprintf (val, "%s: queue's post-exec command %s failed for job %s: %s", __func__, jp->jobSpecs.postCmd, lsb_jobid2str (jp->jobSpecs.jobId), strerror (errno));
	sbdSyslog (LOG_ERR, val);

	exit (-1);
}

/* chPrePostUser()
 * Change the userid to the execution user.
 */
int
chPrePostUser (struct jobCard *jp)
{
	uid_t prepostUid;

	if (initgroups (jp->execUsername, jp->execGid) < 0)
	{
		ls_syslog (LOG_ERR, "%s: initgroups() failed for user %s uid %d gid %d %m", __func__, jp->execUsername, jp->jobSpecs.execUid, jp->execGid);
	}

	if (setgid (jp->execGid) < 0)
	{
		ls_syslog (LOG_ERR, "%s: setgid() failed for user %s uid %d gid %d %m", __func__, jp->execUsername, jp->jobSpecs.execUid, jp->execGid);
	}

	if (getOSUid_ (jp->jobSpecs.prepostUsername, &prepostUid) < 0)
	{
		prepostUid = jp->jobSpecs.execUid;
	}

	if (lsfSetUid (prepostUid) < 0)
	{
		ls_syslog (LOG_ERR, "%s: lsfSetUid() failed for uid %d gid %d %m", __func__, prepostUid, jp->execGid);
		return -1;
	}

	return 0;
}


int
postJobSetup (struct jobCard *jp)
{
struct hostent *hp;
char userName[MAXLINELEN];

closeBatchSocket ();
sbdChildCloseChan (-1);

Signal_ (SIGCHLD, SIG_DFL);
Signal_ (SIGTERM, SIG_IGN);
Signal_ (SIGINT, SIG_IGN);

if (setsid () == -1)
{

	if (getpid () != getpgrp ())
	{
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "setsid");
	}
}

if (setJobEnv (jp) < 0)
{
	ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "setJobEnv");
	return -1; 
}

if (jp->jobSpecs.execUsername[0] == '\0')
{

	if (acctMapTo (jp) < 0)
	{
		ls_syslog (LOG_DEBUG, "%s: acctMapTo() failed for job <%s>", __func__, lsb_jobid2str (jp->jobSpecs.jobId));
		return -1; 
	}


	putEnv ("LSFUSER", jp->execUsername); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac


	if (getOSUserName_ (jp->execUsername, userName, MAXLINELEN) == 0)
	{
		putEnv ("USER", userName); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	}
	else
	{
		putEnv ("USER", jp->execUsername); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	}

	chuser (jp->jobSpecs.execUid);

	if (acctMapOk (jp) < 0)
	{
		ls_syslog (LOG_DEBUG, "%s: acctMapOk() failed for job <%s>", __func__, lsb_jobid2str (jp->jobSpecs.jobId));
		return -1; 
	}

	chuser (batchId);
}

if ((hp = Gethostbyname_ (jp->jobSpecs.fromHost)) == NULL)
	ls_syslog (LOG_ERR, "%s: Gethostbyname_() failed %s job %s", __func__, jp->jobSpecs.fromHost, lsb_jobid2str (jp->jobSpecs.jobId));

if (!debug)
{

	if (initgroups (jp->execUsername, jp->execGid) < 0)
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "initgroups", jp->execUsername);

	if (setgid (jp->execGid) < 0)
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_D_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "setgid", (int) jp->execGid);
}

chuser (jp->jobSpecs.execUid);

if (initPaths (jp, hp, NULL) < 0)
{
	if (!(jp->jobSpecs.jStatus & JOB_STAT_PRE_EXEC)
		&& jp->jobSpecs.reasons != PEND_QUEUE_PRE_FAIL)
	{

		chuser (batchId);
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "initPaths");
		chuser (jp->jobSpecs.execUid);
	}
	return -2; 
}

return 0; 
}


void
runUPre (struct jobCard *jp)
{
int pid, i;
char errMsg[MAXLINELEN];

if ((pid = fork ()) < 0)
{
	sprintf (errMsg, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "fork");
	sbdSyslog (LOG_ERR, errMsg);
	jobSetupStatus (JOB_STAT_PEND, PEND_SBD_NO_PROCESS, jp);
}

if (pid == 0)
{

	closeBatchSocket ();
	if (getuid () == batchId)
	{

		chuser (batchId);
		lsfSetUid (jp->jobSpecs.execUid);
	}

	for (i = 1; i < NSIG; i++)
		Signal_ (i, SIG_DFL);

	Signal_ (SIGHUP, SIG_IGN);

	lsfExecLog (jp->jobSpecs.preExecCmd);

	execl ("/bin/sh", "/bin/sh", "-c", jp->jobSpecs.preExecCmd, NULL); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	sprintf (errMsg, I18N_JOB_FAIL_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "execl");
	sbdSyslog (LOG_ERR, errMsg);
	sbdSyslog (LOG_ERR, jp->jobSpecs.preExecCmd);
	exit (-1);
}

collectPreStatus (jp, pid, "runUPre"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

jp->jobSpecs.jStatus &= ~JOB_STAT_PRE_EXEC;
}

void
collectPreStatus (struct jobCard *jp, int pid, char *context)
{
int id;
LS_WAIT_T status;
struct lsfRusage lsfRusage;
struct rusage rusage;
char errMsg[MAXLINELEN];

while ((id = wait3 (&status, 0, &rusage)) != pid)
{
	if (id < 0)
	{
		sprintf (errMsg, I18N_JOB_FAIL_S_D_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId), "wait3", pid);
		sbdSyslog (LOG_ERR, errMsg);
		jobSetupStatus (JOB_STAT_PEND, PEND_JOB_EXEC_INIT, jp);
	}

	/* catgets 482 */
	  sprintf (errMsg, _i18n_msg_get (ls_catd, NL_SETN, 482, "%s: %s: wait3() got %d, not pid %d for job <%s>"), __func__, context, id, pid, lsb_jobid2str (jp->jobSpecs.jobId));
	  sbdSyslog (LOG_DEBUG, errMsg);
	}

	if (LS_STATUS (status))
	{
		if (logclass & LC_TRACE)
			ls_syslog (LOG_DEBUG, "collectPreStatus: %s: job <%s> status <%d>", context, lsb_jobid2str (jp->jobSpecs.jobId), LS_STATUS (status));

		ls_ruunix2lsf (&rusage, &lsfRusage);
		jp->w_status = LS_STATUS (status);
		jp->cpuTime = lsfRusage.ru_utime + lsfRusage.ru_stime;
		memcpy ((char *) &jp->lsfRusage, (char *) &lsfRusage, sizeof (struct lsfRusage));

		if ((WIFEXITED (status) && WEXITSTATUS (status) == LSB_PRE_ABORT) || WIFSIGNALED (status))
			jobSetupStatus (JOB_STAT_EXIT, 0, jp);
		else
		{
			if (!strcmp (context, "runUPre")) // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
				jobSetupStatus (JOB_STAT_PEND, PEND_JOB_PRE_EXEC, jp);
			else
				jobSetupStatus (JOB_STAT_PEND, PEND_QUEUE_PRE_FAIL, jp);
		}
	}
}


int
requeueJob (struct jobCard *jp)
{
char *sp, *cp;
int w_status;
LS_WAIT_T status;


LS_STATUS (status) = jp->w_status;
w_status = WEXITSTATUS (status);

if (logclass & LC_TRACE)
	ls_syslog (LOG_DEBUG, "%s: Exit status for job <%s> is <%d>, normal termination is <%d>, jp->w_status <%d>, requeue exit status is <%s>", __func__, lsb_jobid2str (jp->jobSpecs.jobId), w_status, WIFEXITED (status), jp->w_status, jp->jobSpecs.requeueEValues);

if (!WIFEXITED (status) ||
	!jp->jobSpecs.requeueEValues || jp->jobSpecs.requeueEValues[0] == '\0')
	return FALSE; 

sp = jp->jobSpecs.requeueEValues;

while ((cp = getNextWord_ (&sp)) != NULL)
{

	if ((!isdigit (cp[0])) || (atoi (cp) != w_status))
		continue;
	return TRUE; 
}
return FALSE; 
}



int
reniceJob (struct jobCard *jp)
{
int which, who, i;

if (logclass & LC_TRACE)
	ls_syslog (LOG_DEBUG, "%s: Job <%s> nice value %d", __func__, lsb_jobid2str (jp->jobSpecs.jobId), jp->jobSpecs.nice);




if (jp->runRusage.npgids == 0)
{
	if (jp->jobSpecs.jobPGid == 0)
	{

		who = jp->jobSpecs.jobPid;
		which = PRIO_PROCESS;
	}
	else
	{
		which = PRIO_PGRP;
		who = jp->jobSpecs.jobPGid;
	}


	if (setpriority (which, who, jp->jobSpecs.nice) == -1)
	{
		ls_syslog (LOG_DEBUG, "%s: prio_pgrp which %d who %d nice %d for job %s: %m", __func__, which, who, jp->jobSpecs.nice, lsb_jobid2str (jp->jobSpecs.jobId));
		which = PRIO_PROCESS;
		who = jp->jobSpecs.jobPid;

		if (setpriority (which, who, jp->jobSpecs.nice) == -1)
		{
			ls_syslog (LOG_DEBUG, "%s: prio_process which %d who %d nice %d for job %s: %m", __func__, which, who, jp->jobSpecs.nice, lsb_jobid2str (jp->jobSpecs.jobId));
			return -1; 
		}
	}
}
else
{
	which = PRIO_PGRP;
	for (i = 0; i < jp->runRusage.npgids; i++)
	{
		who = jp->runRusage.pgid[i];
		if (setpriority (which, who, jp->jobSpecs.nice) == -1)
			ls_syslog (LOG_DEBUG, "%s: prio_pgrp which %d who %d nice %d for job %s: %m", __func__, which, who, jp->jobSpecs.nice, lsb_jobid2str (jp->jobSpecs.jobId));
	}
}

return 0; 
}


int
updateRUsageFromSuper (struct jobCard *jp, char *mbuf)
{
static struct jRusage jusage;
int i, ret, cnt;

if (logclass & LC_TRACE) {
	ls_syslog (LOG_DEBUG, "%s: Entering this routine ...", __func__);
}



ret = sscanf (mbuf, "%d%d%d%d%d%n", &jusage.mem, &jusage.swap, &jusage.utime, &jusage.stime, &jusage.npids, &cnt);

if (ret != 5)
{
	// FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	/* catgets 5462 */
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5462, "%s: sscanf() got %d values from [%s], continuing"), __func__, ret, mbuf);
	  return LSBE_PROTOCOL;
	}

	mbuf += cnt + 1;

	if (jusage.npids > 0)
	{
		FREEUP (jusage.pidInfo);
		jusage.pidInfo = my_malloc (jusage.npids * sizeof (struct pidInfo), __func__);



		for (i = 0; i < jusage.npids; i++)
		{
			if ((ret = sscanf (mbuf, "%d%d%d%d%n",
				&jusage.pidInfo[i].pid, &jusage.pidInfo[i].ppid,
				&jusage.pidInfo[i].pgid,
				&jusage.pidInfo[i].jobid, &cnt)) != 4)
			{

				
				/* catgets 5463 */
		  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5463, "%s: sscanf() %s for job <%s> i=%d, ret=%d failed: %m"), __func__, "pid", lsb_jobid2str (jp->jobSpecs.jobId), i, ret);
		  FREEUP (jusage.pidInfo);
		  return LSBE_NO_JOB;
		}
		mbuf += cnt + 1;
	}
}

if ((ret = sscanf (mbuf, "%d%n", &jusage.npgids, &cnt)) != 1)
{
	ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5463, "%s: sscanf() %s for job <%s> i=%d, ret=%d failed: %m"), __func__, "npgids", lsb_jobid2str (jp->jobSpecs.jobId), 0, ret);
	FREEUP (jusage.pidInfo);
	return LSBE_NO_JOB;
}

mbuf += cnt + 1;
if (jusage.npgids > 0)
{
	FREEUP (jusage.pgid);
	jusage.pgid = my_malloc (jusage.npgids * sizeof (int), __func__);



	for (i = 0; i < jusage.npgids; i++)
	{
		if ((ret = sscanf (mbuf, "%d%n", &jusage.pgid[i], &cnt)) != 1)
		{
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5463, "%s: sscanf() %s for job <%s> i=%d, ret=%d failed: %m"), __func__, "pgid", lsb_jobid2str (jp->jobSpecs.jobId), i, ret);
			FREEUP (jusage.pgid);
			FREEUP (jusage.pidInfo);
			return LSBE_NO_JOB;
		}
		mbuf += cnt + 1;
	}
}


updateJUsage (jp, &jusage);

if (logclass & (LC_TRACE | LC_EXEC))
{
	ls_syslog (LOG_DEBUG, "%s: ru job %s mem %d swap %d utime %d stime %d npids %d", __func__, lsb_jobid2str (jp->jobSpecs.jobId), jp->runRusage.mem, jp->runRusage.swap, jp->runRusage.utime, jp->runRusage.stime, jp->runRusage.npgids);
	for (i = 0; i < jp->runRusage.npgids; i++) {
		ls_syslog (LOG_DEBUG, "... pgid[%d]=%d", i, jp->runRusage.pgid[i]);
	}
}


return LSBE_NO_ERROR;

}

void
updateJUsage (struct jobCard *jPtr, const struct jRusage *jRusage)
{

if (logclass & LC_EXEC)
{
	ls_syslog (LOG_DEBUG, "	%s: Update rusage for job=%d from supervisor (%x/%d) newutime=%d newstime=%d newmem=%d newswap=%d wrkutime=%d wrkstime=%d prevutime=%d prevstime=%d prevmem=%d prevswap=%d", __func__, jPtr->jobSpecs.jobId, jPtr->client, jPtr->newPam, jRusage->utime, jRusage->stime, jRusage->mem, jRusage->swap, jPtr->wrkRusage.utime, jPtr->wrkRusage.stime, jPtr->runRusage.utime, jPtr->runRusage.stime, jPtr->runRusage.mem, jPtr->runRusage.swap);
}

if (jPtr->newPam == TRUE)
{


	if (jPtr->runRusage.utime == -1 && jPtr->runRusage.stime == -1)
	{
		jPtr->runRusage.utime = jPtr->runRusage.stime = 0;
	}


	jPtr->runRusage.utime += jRusage->utime;
	jPtr->runRusage.stime += jRusage->stime;

}
else
{


	jPtr->runRusage.utime += (jRusage->utime - jPtr->wrkRusage.utime);
	jPtr->runRusage.stime += (jRusage->stime - jPtr->wrkRusage.stime);
}



jPtr->runRusage.mem = MAX (jPtr->runRusage.mem, jRusage->mem);
jPtr->runRusage.swap = MAX (jPtr->runRusage.swap, jRusage->swap);

if (logclass & LC_EXEC)
{
	ls_syslog (LOG_DEBUG, "\
		%s: current rusage of job %d utime=%d stime=%d mem=%d swap=%d", __func__, jPtr->jobSpecs.jobId, jPtr->runRusage.utime, jPtr->runRusage.stime, jPtr->runRusage.mem, jPtr->runRusage.swap);
}


jPtr->wrkRusage.utime = jRusage->utime;
jPtr->wrkRusage.stime = jRusage->stime;


copyPidInfo (jPtr, jRusage);


if (jPtr->newPam == TRUE)
{
	writePidInfoFile (jPtr, jRusage);
}


jPtr->newPam = FALSE;

}

void
copyPidInfo (struct jobCard *jPtr, const struct jRusage *jRusage)
{

FREEUP (jPtr->runRusage.pidInfo);
jPtr->runRusage.npids = 0;
FREEUP (jPtr->runRusage.pgid);
jPtr->runRusage.npgids = 0;


if (jRusage->npids > 0)
{

	jPtr->runRusage.pidInfo = my_calloc (jRusage->npids, sizeof (struct pidInfo), __func__);
	if (jPtr->runRusage.pidInfo == NULL)
	{
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5500, "%s: failed to malloc (%d) bytes.\n"), __func__, (jRusage->npids) * (sizeof (struct pidInfo)));    /* catgets 5500 */
		return;
	}


	jPtr->runRusage.npids = jRusage->npids;
	memcpy (jPtr->runRusage.pidInfo,
		jRusage->pidInfo, (jRusage->npids) * (sizeof (struct pidInfo)));
}


if (jRusage->npgids > 0)
{

	jPtr->runRusage.pgid =
	(int *) my_calloc (jRusage->npgids, sizeof (int), __func__);
	if (jPtr->runRusage.pgid == NULL)
	{
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5501, "%s: failed to malloc (%d) bytes.\n"), __func__, (jRusage->npgids) * (sizeof (int)));  /* catgets 5501 */
		return;
	}


	jPtr->runRusage.npgids = jRusage->npgids;
	memcpy (jPtr->runRusage.pgid, jRusage->pgid, (jRusage->npgids) * (sizeof (int)));
}

}

void
writePidInfoFile (const struct jobCard *jPtr, const struct jRusage *jRusage)
{

char buf[MAX_FILENAME_LEN];
FILE *fp;
int i;

if (jPtr->jobSpecs.jobFile[0] == '/')
{
	sprintf (buf, "%s/.%s.pidInfo", LSTMPDIR, strrchr (jPtr->jobSpecs.jobFile, '/') + 1); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
}
else
{
	sprintf (buf, "%s/.%s.pidInfo", LSTMPDIR, jPtr->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
}

fp = fopen (buf, "w");
if (fp == NULL)
{
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5502, "%s: Unable to fopen() pidInfo file (%s), %m.\n"), __func__, buf); /* catgets 5502 */
	return;
}

for (i = 0; i < jRusage->npids; i++)
{
	fprintf (fp, "%d %d %d\n",
		jRusage->pidInfo[i].pid,
		jRusage->pidInfo[i].ppid, jRusage->pidInfo[i].pgid);
}

FCLOSEUP (&fp);

}



void
jobFinishRusage (struct jobCard *jp)
{
char rufn[MAX_FILENAME_LEN];
char rufn30[MAX_FILENAME_LEN];
char tmpDirName[MAX_FILENAME_LEN];
struct lsfAcctRec *rec;
int lineNum = 0;
FILE *fp;

if (logclass & LC_TRACE)
	ls_syslog (LOG_DEBUG, "%s: Entering this routine ...", __func__);

if (!jp->collectedChild)
{

	LS_WAIT_T wStatus;

	LS_STATUS (wStatus) = 0xff00;
	jp->w_status = LS_STATUS (wStatus);
}

getJobTmpDir ( tmpDirName, jp);

if (jp->jobSpecs.jobFile[0] == '/')
{
	sprintf (rufn, "%s/.%s.acct", tmpDirName, strrchr (jp->jobSpecs.jobFile, '/') + 1); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	sprintf (rufn30, "%s/.%s.acct", LSTMPDIR, strrchr (jp->jobSpecs.jobFile, '/') + 1); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
}
else
{
	sprintf (rufn, "%s/.%s.acct", tmpDirName, jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	sprintf (rufn30, "%s/.%s.acct", LSTMPDIR, jp->jobSpecs.jobFile); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
}

if ((fp = fopen (rufn, "r")) == NULL && (fp = fopen (rufn30, "r")) == NULL)
{
}
if (fp == NULL)
{
	ls_syslog (LOG_DEBUG, "%s: fopen(%s) failed: %m", __func__, rufn);
}
else
{
	if ((rec = ls_getacctrec (fp, &lineNum)) == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "ls_getacctrec",
			rufn);
	}
	else
	{
		if (logclass & LC_EXEC)
		{
			LS_WAIT_T w_status;
			LS_STATUS (w_status) = rec->exitStatus;
		  ls_syslog (LOG_DEBUG, I18N (5495, "%s: Job <%s> status <%d> exitcode <%d>"),  /*catgets 5495 */
			__func__, lsb_jobid2str (jp->jobSpecs.jobId),
			rec->exitStatus, WEXITSTATUS (w_status));
		}
		if (jp->collectedChild)
		{

			if (jp->lsfRusage.ru_utime > rec->lsfRu.ru_utime)
				rec->lsfRu.ru_utime = jp->lsfRusage.ru_utime;
			if (jp->lsfRusage.ru_stime > rec->lsfRu.ru_stime)
				rec->lsfRu.ru_stime = jp->lsfRusage.ru_stime;
		}
		cleanLsfRusage (&jp->lsfRusage);
		jp->lsfRusage = rec->lsfRu;
		jp->w_status = rec->exitStatus;
	}
	FCLOSEUP (&fp);
}
if (jp->runRusage.utime > jp->lsfRusage.ru_utime)
	jp->lsfRusage.ru_utime = jp->runRusage.utime;
if (jp->runRusage.stime > jp->lsfRusage.ru_stime)
	jp->lsfRusage.ru_stime = jp->runRusage.stime;

jp->cpuTime = jp->lsfRusage.ru_utime + jp->lsfRusage.ru_stime;

}

int
initJobCard (struct jobCard *jp, struct jobSpecs *jobSpecs, int *reply)
{

	char *cp, *word;
	int j;


jp->resumeCondVal = NULL;
jp->stopCondVal = NULL;

cp = jp->jobSpecs.windows;
for (j = 0; j < 8; j++)
	jp->week[j] = NULL;
while ((word = getNextWord_ (&cp)) != NULL)
{
	if (addWindow (word, jp->week, "addJob jobSpecs") < 0) // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	{
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jobSpecs->jobId), "addWindow", word); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		freeWeek (jp->week);
		*reply = ERR_BAD_REQ;
		return -1; 
	}
}
if (jobSpecs->resumeCond && jobSpecs->resumeCond[0] != '\0')
{
	if ((jp->resumeCondVal = checkThresholdCond (jobSpecs->resumeCond))	== NULL)
	{
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId),	"checkThresholdCond", "resumeCond");
		freeWeek (jp->week);
		*reply = ERR_BAD_REQ;
		return -1; 
	}
}

if (jobSpecs->stopCond && jobSpecs->stopCond[0] != '\0')
{
	if ((jp->stopCondVal = checkThresholdCond (jobSpecs->stopCond)) == NULL)
	{
		ls_syslog (LOG_ERR, I18N_JOB_FAIL_S_S_M, __func__, lsb_jobid2str (jp->jobSpecs.jobId),	"checkThresholdCond", "stopCond");
		freeWeek (jp->week);
		*reply = ERR_BAD_REQ;
		return -1; 
	}
}


setRunLimit (jp, TRUE);
jp->windEdge = now;
jp->active = FALSE;
jp->windWarnTime = 0;
jp->w_status = 0;
jp->cpuTime = 0.0;
jp->notReported = 0;
jp->exitPid = 0;
jp->missing = FALSE;
jp->needReportRU = FALSE;
jp->mbdRestarted = FALSE;
jp->lastChkpntTime = now;

jp->migCnt = 1;

jp->cleanupPid = 0;
jp->collectedChild = FALSE;

cleanLsfRusage (&jp->lsfRusage);
jp->client = NULL;
jp->regOpFlag = 0;
jp->jSupStatus = -1;
initJRusage (&jp->runRusage);
initJRusage (&jp->mbdRusage);
initJRusage (&jp->maxRusage);
jp->lastStatusMbdTime = now;
jp->actStatus = ACT_NO;

jp->jobSpecs.execHosts = NULL;


ls_syslog (LOG_DEBUG, "options2=%x ", jobSpecs->options2);

jp->crossPlatforms = -1;
if (jobSpecs->options2 >= 0)
{
	if (jobSpecs->options2 & SUB2_HOST_UX)
		jp->crossPlatforms = FALSE;
	else
		jp->crossPlatforms = TRUE;

}


saveSpecs (&jp->jobSpecs, jobSpecs);
jobcnt++;
inJobLink (jp);
jp->spooledExec = NULL;

jp->postJobStarted = 0;
jp->userJobSucc = FALSE;

return 0; 
}

void
saveThresholds (struct jobSpecs *jobSpecs, struct thresholds *thresholds)
{

int i, j;


jobSpecs->thresholds.loadSched = (float **)
my_calloc (thresholds->nThresholds, sizeof (float *), __func__);
jobSpecs->thresholds.loadStop = (float **)
my_calloc (thresholds->nThresholds, sizeof (float *), __func__);
for (i = 0; i < thresholds->nThresholds; i++)
{
	jobSpecs->thresholds.loadSched[i] = (float *)
	my_calloc (thresholds->nIdx, sizeof (float), __func__);
	jobSpecs->thresholds.loadStop[i] = (float *)
	my_calloc (thresholds->nIdx, sizeof (float), __func__);
}
for (i = 0; i < thresholds->nThresholds; i++)
{
	for (j = 0; j < thresholds->nIdx; j++)
	{
		jobSpecs->thresholds.loadSched[i][j] = thresholds->loadSched[i][j];
		jobSpecs->thresholds.loadStop[i][j] = thresholds->loadStop[i][j];
	}
}

jobSpecs->thresholds.nIdx = thresholds->nIdx;
jobSpecs->thresholds.nThresholds = thresholds->nThresholds;

}

void
freeThresholds (struct thresholds *thresholds)
{
	int i;

	if (thresholds->nThresholds <= 0)
		return;
	for (i = 0; i < thresholds->nThresholds; i++)
	{
		FREEUP (thresholds->loadSched[i]);
		FREEUP (thresholds->loadStop[i]);
	}
	FREEUP (thresholds->loadSched);
	FREEUP (thresholds->loadStop);
	thresholds->nIdx = 0;
	thresholds->nThresholds = 0;

}

void
initJRusage (struct jRusage *jRusage)
{
	jRusage->mem = -1;
	jRusage->swap = -1;
	jRusage->utime = -1;
	jRusage->stime = -1;
	jRusage->npids = 0;
	jRusage->pidInfo = NULL;
	jRusage->npgids = 0;
	jRusage->pgid = NULL;
}


int
getJobVersion (struct jobSpecs *jobSpecs)
{
	int i, version;
	char *sp;


	if (jobSpecs->numEnv <= 0)

		return 2; 

	for (i = 0; i < jobSpecs->numEnv; i++)
	{
		if ((sp = strstr (jobSpecs->env[i], "LSF_VERSION=")) == NULL)
			continue;
		sp += strlen ("LSF_VERSION="); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		version = atoi (sp);
		if (logclass & LC_EXEC)
			ls_syslog (LOG_DEBUG3, "%s: LSF_VERSION of job <%s> is <%d>", __func__, lsb_jobid2str (jobSpecs->jobId), version);
		if (version <= 0)
			return -1; 
		return version; 
	}
	return 3; 

}

int
lockHosts (struct jobCard *jp)
{
	if (!(jp->jobSpecs.jAttrib & QUEUE_ATTRIB_EXCLUSIVE)) {
		return 0;
	}

	for (unsigned int i = 0; i < jp->jobSpecs.numToHosts; i++) {
		if (i > 0 && strcmp (jp->jobSpecs.toHosts[i], jp->jobSpecs.toHosts[i - 1]) == 0) {
			continue;
		}
		if (lockHost_ (0, jp->jobSpecs.toHosts[i]) < 0 && lserrno != LSE_LIM_ALOCKED) {
			ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_MM, __func__, "lockHost_", "jp->jobSpecs.toHosts[i]"); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
			unlockHosts (jp, i);
			return -1; 
		}
	}
	return 0; 
}


int
REShasPTYfix (char *resPath)
{
	FILE *fp;
	char str[256], cmd[MAX_FILENAME_LEN + 32];

	sprintf (cmd, "%s -PTY_FIX", resPath); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac

	if ((fp = popen (cmd, "r")) == NULL) {
		return FALSE;
	}

	if (fscanf (fp, "%s", str) != 1) {
		pclose (fp);
		return FALSE; 
	}

	pclose (fp);

	if (strcmp (str, "PTY_FIX")) { // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		return FALSE; 
	}

	return TRUE; 
}

void
setJobArrayEnv ( const char *jobName, int jobIndex)
{
	struct idxList *idxList = NULL, *idx;
	char *index, val[MAXLINELEN];
	int found = FALSE;
	int maxJLimit = 0;

	index = strchr (jobName, '['); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	if (!index) {
		return;
	}
	yybuff = index;
	if (idxparse (&idxList, &maxJLimit)) {
		ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "idxparse", index);
		return;
	}

	for (idx = idxList; idx; idx = idx->next) {
		int nstep;

		if ((jobIndex >= idx->start) && (jobIndex <= idx->end)) {
			nstep = (jobIndex - idx->start) / idx->step;
			if ((idx->start + nstep * idx->step) == jobIndex) {
				found = TRUE;
				break;
			}
		}
	}
	if (found) {
		sprintf (val, "%d", idx->step);
		putEnv ("LSB_JOBINDEX_STEP", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
		sprintf (val, "%d", idx->end);
		putEnv ("LSB_JOBINDEX_END", val); // FIXME FIXME FIXME FIXME "TMPDIR" should be put into configure.ac
	}
	else {
		/* catgets 5400 */
		ls_syslog (LOG_ERR, I18N (5400, "%s: Job %d not found in job index list"), __func__, jobIndex);
	}

	while (idxList) {
		idx = idxList->next;
		FREEUP (idxList);
		idxList = idx;
	}

	return;
}
