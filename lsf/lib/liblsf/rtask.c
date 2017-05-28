/* $Id: lib.rtask.c 397 2007-11-26 19:04:00Z mblack $
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

#include <pwd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/mls.h"
#include "lib/queue.h"
#include "lib/xdrres.h"
#include "lsf.h"
#include "daemons/libniosd/niosd.h"
#include "daemons/libresd/resout.h"

static void default_tstp_ (int signo);
static u_short getTaskPort (int s);
static char rexcwd_[MAXPATHLEN];
static void initSigHandler (int sig);

void setRexWd_ (char *wd);
int  lsRGetpgrp_ (int sock, int taskid, pid_t pid);
int  rgetpidCompletionHandler_ (struct lsRequest *request);

/* #define SIGEMT SIGBUS */

int short nios_ok_;
extern int currentSN;

int
ls_rtaske (char *host, char **argv, int options, char **envp)
{
	static u_short retport;
	static int rpid;
	static int reg_ls_donerex = FALSE;
	struct sockaddr_in sin;
	long max = 0;
	char c_chfd[8];
	char pathbuf[MAXPATHLEN];
	int d = 0;
	int niosOptions = 0;
	char *new_argv[5];
	pid_t pid = 0;
	int s = 0;
	int descriptor[2] = { 0, 0 };
	struct resCmdBill cmdmsg;
	struct lslibNiosRTask taskReq;
	u_short taskPort = 0;
	sigset_t newMask;
	sigset_t oldMask;
	socklen_t len;

	if (!reg_ls_donerex)
	{
		atexit ((void (*)()) ls_donerex);
		reg_ls_donerex = TRUE;
	}

	if (_isconnected_ (host, descriptor)) { 
		s = descriptor[0];
	}
	else if ((s = ls_connect (host)) < 0) {
		return (-1);
	}
	else {
		fprintf( stderr, "ls_rtaske init : you are not supposed to be here" );
	}

	if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
		return (-1);
	}

	if (!FD_ISSET (s, &connection_ok_))
	{
		FD_SET (s, &connection_ok_);
		if (ackReturnCode_ (s) < 0)
		{
			close (s);
			_lostconnection_ (host);
			sigprocmask (SIG_SETMASK, &oldMask, NULL);
			return (-1);
		}
	}

	if (!nios_ok_) {
		niosOptions = options & REXF_SYNCNIOS;
	}
	options &= ~REXF_SYNCNIOS;

	cmdmsg.options = options;
	if (cmdmsg.options & REXF_SHMODE) {
		cmdmsg.options |= REXF_USEPTY;
	}

	if (!isatty (0) && !isatty (1)) {
		cmdmsg.options &= ~REXF_USEPTY;
	}
	else if (cmdmsg.options & REXF_USEPTY)
	{
		if (options & REXF_TTYASYNC)
		{
			if (rstty_async_ (host) < 0)
			{
				sigprocmask (SIG_SETMASK, &oldMask, NULL);
				return (-1);
			}
		}
		else
		{
			if (rstty_ (host) < 0)
			{
				sigprocmask (SIG_SETMASK, &oldMask, NULL);
				return (-1);
			}
		}
	}

  if ((genParams_[LSF_INTERACTIVE_STDERR].paramValue != NULL)
	  && (strcasecmp (genParams_[LSF_INTERACTIVE_STDERR].paramValue,
			  "y") == 0))
	{
	  cmdmsg.options |= REXF_STDERR;
	}

  if (!nios_ok_)
	{

	  initSigHandler (SIGTSTP);
	  initSigHandler (SIGTTIN);
	  initSigHandler (SIGTTOU);


	  if (socketpair (AF_UNIX, SOCK_STREAM, 0, cli_nios_fd) < 0)
	{
	  lserrno = LSE_SOCK_SYS;
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  close (s);
	  return (-1);
	}

	  if ((pid = fork ()) != 0)
	{
	  int mypid;

	  close (cli_nios_fd[1]);
	  mypid = getpid ();
	  if (b_write_fix (cli_nios_fd[0], (char *) &mypid, sizeof (mypid)) !=
		  sizeof (mypid))
		{
		  close (cli_nios_fd[0]);
		  sigprocmask (SIG_SETMASK, &oldMask, NULL);
		  lserrno = LSE_MSG_SYS;
		  return (-1);
		}

	  if (b_read_fix (cli_nios_fd[0], (char *) &retport, sizeof (u_short))
		  != sizeof (u_short))
		{
		  close (cli_nios_fd[0]);
		  sigprocmask (SIG_SETMASK, &oldMask, NULL);
		  lserrno = LSE_MSG_SYS;
		  return (-1);
		}

	  nios_ok_ = TRUE;

	  if (waitpid (pid, 0, 0) < 0)
		{
		  if (errno != ECHILD)
		{
		  close (cli_nios_fd[0]);
		  nios_ok_ = FALSE;
		  sigprocmask (SIG_SETMASK, &oldMask, NULL);
		  lserrno = LSE_WAIT_SYS;
		  return (-1);
		}
		}
	}
	  else
	{
	  if (fork ())
		{
		  max = sysconf (_SC_OPEN_MAX);
		  for (d = 3; d < max; ++d)
		{
		  (void) close (d);
		}
		  exit (0);
		}

		if (initenv_ (NULL, NULL) < 0) {
			exit (-1);
		}
		strcpy (pathbuf, genParams_[LSF_SERVERDIR].paramValue);
		strcat (pathbuf, "/nios");
		sprintf (c_chfd, "%d", cli_nios_fd[1]);
		new_argv[0] = pathbuf;
		new_argv[1] = c_chfd;
		if (cmdmsg.options & REXF_USEPTY)
		{
			if (cmdmsg.options & REXF_SHMODE) {
				new_argv[2] = "2";
			}
			else {
				new_argv[2] = "1";
			}
		}
		else {
			new_argv[2] = "0";
		}
		new_argv[3] = NULL;

		max = sysconf (_SC_OPEN_MAX);
		for (d = 3; d < max; ++d)
		{
			if (d != cli_nios_fd[1]) {
				close (d);
			}
		}

	  for (d = 1; d < NSIG; d++) { // NSIG is in <signal.h>
		Signal_ (d, SIG_DFL);
	}


	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  (void) lsfExecX (new_argv[0], new_argv, execvp);
	  exit (-1);
	}
	}

  if (envp)
	{
	  if (ls_rsetenv_async (host, envp) < 0)
	{
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  return (-1);
	}
	}

  if (rexcwd_[0] != '\0')
	strcpy (cmdmsg.cwd, rexcwd_);
  else if (mygetwd_ (cmdmsg.cwd) == 0)
	{
	  close (s);
	  _lostconnection_ (host);
	  lserrno = LSE_WDIR;
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  return (-1);
	}


  rpid++;

  cmdmsg.rpid = rpid;
  cmdmsg.retport = retport;

  cmdmsg.argv = argv;
  cmdmsg.priority = 0;

  if (sendCmdBill_ (s, (enum resCmd) RES_EXEC, &cmdmsg, NULL, NULL) == -1)
	{
	  close (s);
	  _lostconnection_ (host);
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  return (-1);
	}

  if (cmdmsg.options & REXF_TASKPORT)
	{

	  if ((taskPort = getTaskPort (s)) == 0)
	{
	  close (s);
	  _lostconnection_ (host);
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  return (-1);
	}
	}

  len = sizeof (sin);
  if (getpeername (s, (struct sockaddr *) &sin, &len) < 0)
	{
	  close (s);
	  _lostconnection_ (host);
	  lserrno = LSE_SOCK_SYS;
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  return (-1);
	}

  SET_LSLIB_NIOS_HDR (taskReq.hdr, LIB_NIOS_RTASK, sizeof (taskReq.r));
  taskReq.r.pid = rpid;
  taskReq.r.peer = sin.sin_addr;

  taskReq.r.pid = (niosOptions & REXF_SYNCNIOS) ? -rpid : rpid;

  if (b_write_fix (cli_nios_fd[0], (char *) &taskReq, sizeof (taskReq))
	  != sizeof (taskReq))
	{
	  close (s);
	  _lostconnection_ (host);
	  lserrno = LSE_MSG_SYS;
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  return (-1);
	}

  if (tid_register (rpid, s, taskPort, host, options & REXF_TASKINFO) == -1)
	{
	  close (s);
	  _lostconnection_ (host);
	  lserrno = LSE_MALLOC;
	  sigprocmask (SIG_SETMASK, &oldMask, NULL);
	  return (-1);
	}

  sigprocmask (SIG_SETMASK, &oldMask, NULL);

  return (rpid);
}

int
rgetpidCompletionHandler_ (struct lsRequest *request)
{
  struct resPid pidReply;
  XDR xdrs;
  int rc;


  rc = resRC2LSErr_ (request->rc);
  if (rc != 0)
	return (-1);

  xdrmem_create (&xdrs, request->replyBuf, sizeof (struct resPid),
		 XDR_DECODE);
  if (!xdr_resGetpid (&xdrs, &pidReply, NULL))
	{
	  lserrno = LSE_BAD_XDR;
	  xdr_destroy (&xdrs);
	  return (-1);
	}

  *((int *) request->extra) = pidReply.pid;
  xdr_destroy (&xdrs);
  return (0);

}

void *
lsRGetpidAsync_ (int taskid, int *pid)
{
  struct _buf_
  {
	struct LSFHeader hdrBuf;
	struct resPid pidBuf;
  } buf;
  LS_REQUEST_T *request;

  struct resPid pidReq;
  int s;
  struct tid *tid;
  char host[MAXHOSTNAMELEN];

  if ((tid = tid_find (taskid)) == NULL)
	{
	  return (NULL);
	}

  s = tid->sock;
  gethostbysock_ (s, host);

  if (!FD_ISSET (s, &connection_ok_))
	{
	  FD_SET (s, &connection_ok_);
	  if (ackReturnCode_ (s) < 0)
	{
	  close (s);
	  _lostconnection_ (host);
	  return (NULL);
	}
	}

  pidReq.rpid = taskid;
  pidReq.pid = -1;

  if (callRes_ (s, RES_GETPID, (char *) &pidReq, (char *) &buf,
		sizeof (buf), xdr_resGetpid, 0, 0, NULL) == -1)
	{
	  close (s);
	  _lostconnection_ (host);
	  return (NULL);
	}


  request = lsReqHandCreate_ (taskid,
				  currentSN,
				  s,
				  (void *) pid,
				  rgetpidCompletionHandler_,
				  (appCompletionHandler) NULL, NULL);

  if (request != NULL)
	if (lsQueueDataAppend_ ((char *) request, requestQ))
	  return (NULL);

  return (void *) request;

}

int
lsRGetpid_ (int taskid, int options)
{
  struct _buf_
  {
	struct LSFHeader hdrBuf;
	struct resPid pidBuf;
  } buf;
  LS_REQUEST_T *request;
  pid_t pid = 0;

  struct resPid pidReq;
  int s = 0;
  struct tid *tid;
  char host[MAXHOSTNAMELEN];

  assert( options );

  if ((tid = tid_find (taskid)) == NULL)
	{
	  return (-1);
	}

  s = tid->sock;
  gethostbysock_ (s, host);

  if (!FD_ISSET (s, &connection_ok_))
	{
	  FD_SET (s, &connection_ok_);
	  if (ackReturnCode_ (s) < 0)
	{
	  close (s);
	  _lostconnection_ (host);
	  return (-1);
	}
	}

  pidReq.rpid = taskid;
  pidReq.pid = -1;

  if (callRes_ (s, RES_GETPID, (char *) &pidReq, (char *) &buf,
		sizeof (buf), xdr_resGetpid, 0, 0, NULL) == -1)
	{
	  close (s);
	  _lostconnection_ (host);
	  return (-1);
	}

  request = lsReqHandCreate_ (taskid,
				  currentSN,
				  s,
				  (void *) &pid,
				  rgetpidCompletionHandler_,
				  (appCompletionHandler) NULL, NULL);

  if (request == NULL)
	return (-1);

  if (lsQueueDataAppend_ ((char *) request, requestQ))
	return (-1);

  if (lsReqWait_ (request, 0) < 0)
	return (-1);

  lsReqFree_ (request);

  return (pid);

}

int
lsRGetpgrp_ (int sock, int taskid, pid_t pid)
{
  struct _buf_
  {
	struct LSFHeader hdrBuf;
	struct resPid pidBuf;
  } buf;
  LS_REQUEST_T *request;
  char host[MAXHOSTNAMELEN];

  struct resPid pidReq;
  int s, pgid;

  s = sock;
  gethostbysock_ (s, host);

  if (!FD_ISSET (s, &connection_ok_))
	{
	  FD_SET (s, &connection_ok_);
	  if (ackReturnCode_ (s) < 0)
	{
	  close (s);
	  _lostconnection_ (host);
	  return (-1);
	}
	}

  pidReq.rpid = taskid;
  pidReq.pid = pid;

  if (callRes_ (s, RES_GETPID, (char *) &pidReq, (char *) &buf,
		sizeof (buf), xdr_resGetpid, 0, 0, NULL) == -1)
	{
	  close (s);
	  _lostconnection_ (host);
	  return (-1);
	}

  request = lsReqHandCreate_ (taskid,
				  currentSN,
				  s,
				  (void *) &pgid,
				  rgetpidCompletionHandler_,
				  (appCompletionHandler) NULL, NULL);

  if (request == NULL)
	return (-1);

  if (lsQueueDataAppend_ ((char *) request, requestQ))
	return (-1);

  if (lsReqWait_ (request, 0) < 0)
	return (-1);

  lsReqFree_ (request);

  return (pgid);

}

static void
initSigHandler (int sig)
{
  struct sigaction act, oact;


  act.sa_handler = (SIGFUNCTYPE) default_tstp_;
  sigemptyset (&act.sa_mask);
  sigaddset (&act.sa_mask, sig);
  act.sa_flags = 0;
  sigaction (sig, &act, &oact);

  if (oact.sa_handler != SIG_DFL)
	sigaction (sig, &oact, NULL);
}

int
ls_rtask (char *host, char **argv, int options)
{
  char **envp;
  unsigned long numEnv = 0;
  int ret = 0;

  for (numEnv = 0; environ[numEnv]; numEnv++) // FIXME FIXME there has to be a better way to get the length of the array
	;
  envp = (char **) calloc (numEnv + 1, sizeof (char *));
  for (numEnv = 0; environ[numEnv]; numEnv++)
	envp[numEnv] = strdup (environ[numEnv]);
  envp[numEnv] = NULL;

  ret = ls_rtaske (host, argv, options, envp);

  if (envp)
	{
	  for (numEnv = 0; envp[numEnv]; numEnv++)
	FREEUP (envp[numEnv]);
	  FREEUP (envp);
	}

  return ret;

}

static void
default_tstp_ (int signo)
{
  assert( signo );
  (void) ls_stoprex ();
  kill (getpid (), SIGSTOP);
}

static u_short
getTaskPort (int s)
{
  struct LSFHeader hdr;
  int rc;

  rc = expectReturnCode_ (s, currentSN, &hdr);
  if (rc < 0)
	return (0);

  return (htons (hdr.opCode));
}

void
setRexWd_ (char *wd)
{
  if (wd)
	strcpy (rexcwd_, wd);
  else
	rexcwd_[0] = '\0';
}
