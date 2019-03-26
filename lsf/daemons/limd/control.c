/* $Id: lim.control.c 397 2007-11-26 19:04:00Z mblack $
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

#include <unistd.h>
#include <signal.h>


#include "daemons/liblimd/limd.h"
#include "lib/xdr.h"
#include "lib/mls.h"
#include "lib/lib.h"


// #define NL_SETN 24


// extern struct limLock limLock;
// extern char mustSendLoad;

static int userNameOk (uid_t, const char *);
static void doReopen (void);

void
reconfigReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
	enum limReplyCode limReplyCode;
	struct LSFHeader replyHdr = { };
	struct lsfAuth auth = { };
	XDR xdrs2 = { };
	char *mbuf = malloc( sizeof( char ) * MSGSIZE + 1 );

	initLSFHeader_ (&replyHdr);

	if (!xdr_lsfAuth (xdrs, &auth, reqHdr))
	{
		limReplyCode = LIME_BAD_DATA;
		xdrmem_create (&xdrs2, mbuf, MSGSIZE, XDR_ENCODE);
		replyHdr.opCode = limReplyCode;
		replyHdr.refCode = reqHdr->refCode;

  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
	{
	  ls_syslog (LOG_ERR, "failed at xdr_LSFHeader() in %s\n", __func__ );
	  xdr_destroy (&xdrs2);
	  reconfig ();
	}

  if (chanSendDgram_ (limSock, mbuf, XDR_GETPOS (&xdrs2), from) < 0)
	{
	  /* catgets 7300 */
	  ls_syslog (LOG_ERR, "7300: %s: Error sending reconfig acknowledgement to %s (len=%d): %m", __func__, sockAdd2Str_( from ), XDR_GETPOS( &xdrs2 ) );
	}
  xdr_destroy (&xdrs2);

  if (limReplyCode == LIME_NO_ERR) {
	reconfig ();
  }
	}

  if (!lim_debug)
	{
	  if (!limPortOk (from))
	{
	  limReplyCode = LIME_DENIED;
  xdrmem_create (&xdrs2, mbuf, MSGSIZE, XDR_ENCODE);
  replyHdr.opCode = (short) limReplyCode;
  replyHdr.refCode = reqHdr->refCode;

  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
	  xdr_destroy (&xdrs2);
	  reconfig ();
	}

  if (chanSendDgram_ (limSock, mbuf, XDR_GETPOS (&xdrs2), from) < 0)
	{
	  /* catgets 7300 */
	  ls_syslog (LOG_ERR, "7300: %s: Error sending reconfig acknowledgement to %s (len=%d): %m", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
	}
  xdr_destroy (&xdrs2);

  if (limReplyCode == LIME_NO_ERR) {
	reconfig ();
  }
	}
	}

  limReplyCode = LIME_NO_ERR;

// Reply:
//   xdrmem_create (&xdrs2, mbuf, MSGSIZE, XDR_ENCODE);
//   replyHdr.opCode = (short) limReplyCode;
//   replyHdr.refCode = reqHdr->refCode;

//   if (!xdr_LSFHeader (&xdrs2, &replyHdr))
//     {
//       ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
//       xdr_destroy (&xdrs2);
//       reconfig ();
//     }

//   if (chanSendDgram_ (limSock, mbuf, XDR_GETPOS (&xdrs2), from) < 0)
//     {
//       ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 7300, "%s: Error sending reconfig acknowledgement to %s (len=%d): %m"), __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2)); /* catgets 7300 */
//     }
//   xdr_destroy (&xdrs2);

//   if (limReplyCode == LIME_NO_ERR) {
//     reconfig ();
//   }

  return;
}

void
reconfig (void)
{
  char debug_buf[10] = "";
  char *myargv[5] = { NULL, NULL, NULL, NULL, NULL };
  int i = 0;
  int sdesc = 0;
  sigset_t newmask = 0;
  pid_t pid = 0;

  ls_syslog (LOG_INFO, "%s: Restarting LIM", __func__);

  sigemptyset (&newmask);
  sigprocmask (SIG_SETMASK, &newmask, NULL);

		if (elim_pid > 0)
		{
			kill (elim_pid, SIGTERM);
			millisleep_ (2000); // FIXME FIXME FIXME FIXME replace fixed number with var
		}

	chanClose_ (limSock);
	chanClose_ (limTcpSock);
	logLIMDown ();

	pid = fork ();
	switch (pid)
	{
		case 0:
			myargv[0] = getDaemonPath_ ("/lim", limParams[LSF_SERVERDIR].paramValue); // FIXME FIXME FIXME FIXME replace fixed string with var set in autoconf.ac
			ls_syslog (LOG_DEBUG, "%s: re-exec argv[0] %s", __func__, myargv[0]); // FIXME FIXME FIXM sanitize argv[]
			i = 1;
			if (lim_debug)
			{
				sprintf (debug_buf, "-%d", lim_debug);
				myargv[i] = debug_buf;
				i++;
			}
			if (env_dir != NULL) // FIXME FIXME FIXME explain what -d does
			{
				myargv[i] = "-d";
				myargv[i + 1] = env_dir;
				i += 2;
			}
			myargv[i] = NULL;

			if (lim_debug >= 2) {
				sdesc = 3;
			}
			else {
				sdesc = 0;
			}

			for (i = sdesc; i < sysconf (_SC_OPEN_MAX); i++) {
				close (i);
			}

			if (limLock.on)
			{
				char *lsfLimLock = malloc( sizeof( char ) * MAX_LINE_LEN + 1 );

				if (time (0) > limLock.time)
				{

					limLock.on &= ~LIM_LOCK_STAT_USER;
					if (limLock.on & LIM_LOCK_STAT_MASTER)
					{
						sprintf (lsfLimLock, "LSF_LIM_LOCK=%d %d", limLock.on, 0);
						putenv (lsfLimLock);
					}
					else
					{
						sprintf (lsfLimLock, "LSF_LIM_LOCK=");
						putenv (lsfLimLock);
					}
				}
				else
				{
					sprintf (lsfLimLock, "LSF_LIM_LOCK=%d %ld", limLock.on, limLock.time);
					putenv (lsfLimLock);
				}

				free( lsfLimLock );
			}
			else
			{
				char *lsfLimLock = malloc( sizeof( char ) * MAX_LINE_LEN + 1 );
				sprintf (lsfLimLock, "LSF_LIM_LOCK=");
				putenv (lsfLimLock);
				free( lsfLimLock );
			}

			putLastActiveTime ();
			lsfExecX(myargv[0], myargv, execvp); // FIXME FIXME FIXME FIXME sanitize myargv[0]
			// lsfExecvp (myargv[0], myargv);
			ls_syslog (LOG_ERR, "%s: execvp %s failed %m", myargv[0]);
			lim_Exit (__func__);
			break;

		default:
			exit (0);
			break;
	}

}

void
shutdownReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
	char *mbuf = malloc( sizeof ( char ) * MSGSIZE + 1 );
	enum limReplyCode limReplyCode;
	struct LSFHeader replyHdr = { };
	struct lsfAuth auth       = { };
	XDR xdrs2                 = { };
  
	initLSFHeader_ (&replyHdr);

	if (!xdr_lsfAuth (xdrs, &auth, reqHdr))
	{
		limReplyCode = LIME_BAD_DATA;
		xdrmem_create (&xdrs2, mbuf, MSGSIZE, XDR_ENCODE);
		replyHdr.opCode = (short) limReplyCode;
		replyHdr.refCode = reqHdr->refCode;

		if (!xdr_LSFHeader (&xdrs2, &replyHdr))
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
			xdr_destroy (&xdrs2);
			free( mbuf );
			return;
		}
		if (chanSendDgram_ (limSock, mbuf, XDR_GETPOS (&xdrs2), from) < 0)
		{
		    /* catgets 7302 */
			ls_syslog (LOG_ERR, "7302: %s: Error sending shutdown acknowledgement to %s (len=%d), shutdown failed : %m", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
			xdr_destroy (&xdrs2);
			free( mbuf );
			return;
		}

		xdr_destroy (&xdrs2);
		if (limReplyCode == LIME_NO_ERR) {
			free( mbuf );
			shutdownLim ();
		}
	}

	if (!lim_debug)
	{
		if (!limPortOk (from))
		{
			limReplyCode = LIME_DENIED;
			xdrmem_create (&xdrs2, mbuf, MSGSIZE, XDR_ENCODE);
			replyHdr.opCode = limReplyCode;
			replyHdr.refCode = reqHdr->refCode;

			if (!xdr_LSFHeader (&xdrs2, &replyHdr))
			{
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
				xdr_destroy (&xdrs2);
				free( mbuf );
				return;
			}

			if (chanSendDgram_ (limSock, mbuf, XDR_GETPOS (&xdrs2), from) < 0)
			{
				/* catgets 7302 */
				ls_syslog (LOG_ERR, "7302: %s: Error sending shutdown acknowledgement to %s (len=%d), shutdown failed : %m", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
				xdr_destroy (&xdrs2);
				free( mbuf );
				return;
			}

			xdr_destroy (&xdrs2);
			if (limReplyCode == LIME_NO_ERR) {
				free( mbuf );
				shutdownLim ();
			}
		}
	}

	limReplyCode = LIME_NO_ERR;

	return;

// Reply:
//   xdrmem_create (&xdrs2, mbuf, MSGSIZE, XDR_ENCODE);
//   replyHdr.opCode = (short) limReplyCode;
//   replyHdr.refCode = reqHdr->refCode;

//   if (!xdr_LSFHeader (&xdrs2, &replyHdr))
// 	{
// 	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
// 	  xdr_destroy (&xdrs2);
// 	  return;
// 	}
//   if (chanSendDgram_ (limSock, mbuf, XDR_GETPOS (&xdrs2), from) < 0)
// 	{
// 	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 7302, "%s: Error sending shutdown acknowledgement to %s (len=%d), shutdown failed : %m"),    /* catgets 7302 */
// 		 __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
// 	  xdr_destroy (&xdrs2);
// 	  return;
// 	}

//   	xdr_destroy (&xdrs2);

//   	if (limReplyCode == LIME_NO_ERR) {
// 		shutdownLim ();
// 	}
}


void
shutdownLim (void)
{
  chanClose_ (limSock);

  ls_syslog (LOG_ERR, "%s: LIM shutting down: shutdown request received", __func__);

  if (elim_pid > 0)
	{
	  kill (elim_pid, SIGTERM);
	}
  if (pimPid > 0)
	{
	  kill (pimPid, SIGTERM);
	}

  logLIMDown ();
  exit (EXIT_NO_ERROR);
}


void
lockReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
	char *buf = malloc( sizeof( char ) * MAXHOSTNAMELEN + 1 );
	XDR xdrs2;
	enum limReplyCode limReplyCode;
	struct limLock limLockReq = { };
	struct LSFHeader replyHdr = { };

	ls_syslog (LOG_DEBUG1, "%s: received request from %s", __func__, sockAdd2Str_ (from));

  initLSFHeader_ (&replyHdr);
  if (!xdr_limLock (xdrs, &limLockReq, reqHdr))
	{
	  limReplyCode = LIME_BAD_DATA;
		  xdrmem_create (&xdrs2, buf, MAXHOSTNAMELEN, XDR_ENCODE);
		  replyHdr.opCode = (short) limReplyCode;
		  replyHdr.refCode = reqHdr->refCode;
		  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
			  xdr_destroy (&xdrs2);
			  return;
			}
		  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_",
				 sockAdd2Str_ (from));
			  xdr_destroy (&xdrs2);
			  return;
			}
		  xdr_destroy (&xdrs2);
		  return;

	}

  if (!lim_debug)
	{
	  if (!limPortOk (from))
	{
	  limReplyCode = LIME_DENIED;
		  xdrmem_create (&xdrs2, buf, MAXHOSTNAMELEN, XDR_ENCODE);
		  replyHdr.opCode = (short) limReplyCode;
		  replyHdr.refCode = reqHdr->refCode;
		  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
			  xdr_destroy (&xdrs2);
			  return;
			}
		  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_",
				 sockAdd2Str_ (from));
			  xdr_destroy (&xdrs2);
			  return;
			}
		  xdr_destroy (&xdrs2);
		  return;

	}
	}

  if (!userNameOk (limLockReq.uid, limLockReq.lsfUserName))
	{
		/* catgets 7306 */ 
		ls_syslog( LOG_INFO, "7306: %s: lock/unlock request from uid %d rejected", __func__, limLock.uid);
		limReplyCode = LIME_DENIED;
		  xdrmem_create (&xdrs2, buf, MAXHOSTNAMELEN, XDR_ENCODE);
		  replyHdr.opCode = (short) limReplyCode;
		  replyHdr.refCode = reqHdr->refCode;
		  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
			  xdr_destroy (&xdrs2);
			  return;
			}
		  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_",
				 sockAdd2Str_ (from));
			  xdr_destroy (&xdrs2);
			  return;
			}
		  xdr_destroy (&xdrs2);
		  return;
	}


  if ((LOCK_BY_USER (limLock.on) && limLockReq.on == LIM_LOCK_USER)
	  || (LOCK_BY_MASTER (limLock.on) && limLockReq.on == LIM_LOCK_MASTER))
	{

	  limReplyCode = LIME_LOCKED_AL;
		  xdrmem_create (&xdrs2, buf, MAXHOSTNAMELEN, XDR_ENCODE);
		  replyHdr.opCode = (short) limReplyCode;
		  replyHdr.refCode = reqHdr->refCode;
		  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
			  xdr_destroy (&xdrs2);
			  return;
			}
		  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_",
				 sockAdd2Str_ (from));
			  xdr_destroy (&xdrs2);
			  return;
			}
		  xdr_destroy (&xdrs2);
		  return;

	}


  if ((!LOCK_BY_USER (limLock.on) && limLockReq.on == LIM_UNLOCK_USER)
	  || (!LOCK_BY_MASTER (limLock.on) && limLockReq.on == LIM_UNLOCK_MASTER))
	{
	  limReplyCode = LIME_NOT_LOCKED;
		  xdrmem_create (&xdrs2, buf, MAXHOSTNAMELEN, XDR_ENCODE);
		  replyHdr.opCode = (short) limReplyCode;
		  replyHdr.refCode = reqHdr->refCode;
		  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
			  xdr_destroy (&xdrs2);
			  return;
			}
		  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
			{
			  ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_",
				 sockAdd2Str_ (from));
			  xdr_destroy (&xdrs2);
			  return;
			}
		  xdr_destroy (&xdrs2);
		  return;

	}


  if (limLockReq.on == LIM_UNLOCK_MASTER)
	{
	  limLock.on &= ~LIM_LOCK_STAT_MASTER;
	  myHostPtr->status[0] &= ~LIM_LOCKEDM;
	}


  if (limLockReq.on == LIM_UNLOCK_USER)
	{
	  limLock.on &= ~LIM_LOCK_STAT_USER;
	  limLock.time = 0;
	  myHostPtr->status[0] &= ~LIM_LOCKEDU;
	}


  if (limLockReq.on == LIM_LOCK_MASTER)
	{
	  limLock.on |= LIM_LOCK_STAT_MASTER;
	  myHostPtr->status[0] |= LIM_LOCKEDM;
	}


  if (limLockReq.on == LIM_LOCK_USER)
	{
	  limLock.on |= LIM_LOCK_STAT_USER;
	  myHostPtr->status[0] |= LIM_LOCKEDU;
	  limLock.time = time (0) + limLockReq.time;
	}

  mustSendLoad = TRUE;
  limReplyCode = LIME_NO_ERR;

// Reply:
  xdrmem_create (&xdrs2, buf, MAXHOSTNAMELEN, XDR_ENCODE);
  replyHdr.opCode = (short) limReplyCode;
  replyHdr.refCode = reqHdr->refCode;
  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader"); xdr_destroy (&xdrs2);
	  return;
	}
  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_", sockAdd2Str_ (from));
	  xdr_destroy (&xdrs2);
	  return;
	}
  xdr_destroy (&xdrs2);
  return;

}

/* servAvailReq()
 */
void
servAvailReq (XDR * xdrs,  struct hostNode *hPtr, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
  int servId;

  if (hPtr != NULL && hPtr != myHostPtr)
	{
	  ls_syslog (LOG_WARNING, "%s: Request from non-local host: <%s>", __func__, hPtr->hostName);
	  return;
	}

  if (!lim_debug)
	{
	  if (ntohs (from->sin_port) >= IPPORT_RESERVED
	  || ntohs (from->sin_port) < IPPORT_RESERVED / 2)
	{
	  ls_syslog (LOG_WARNING, "\
%s: Request from non-privileged port: <%d>", __func__, ntohs (from->sin_port));
	  return;
	}
	}

  if (!xdr_int (xdrs, &servId))
	{
	  ls_syslog (LOG_ERR, "\
%s: failed decoding servID from host %s port %d", __func__, hPtr->hostName, ntohs (from->sin_port));
	  return;
	}

  switch (servId)
	{
	case 1:
	  resInactivityCount = 0;
	  myHostPtr->status[0] &= ~(LIM_RESDOWN);
	  break;
	case 2:
	  myHostPtr->status[0] &= ~(LIM_SBDDOWN);
	  lastSbdActiveTime = time (0);
	  break;
	default:
	  ls_syslog (LOG_WARNING, "\
%s: Invalid service  %d", __func__, servId);
	}
}

int
limPortOk (struct sockaddr_in *from)
{

  if (from->sin_family != AF_INET)
	{
	  ls_syslog (LOG_ERR, "%s: %s sin_family != AF_INET",
		 "limPortOk", sockAdd2Str_ (from));
	  return (FALSE);
	}


  if (from->sin_port == lim_port)
	return (TRUE);

#ifndef INSECURE
  if (!lim_debug)
	{
	  if (ntohs (from->sin_port) >= IPPORT_RESERVED
	  || ntohs (from->sin_port) < IPPORT_RESERVED / 2)
	return FALSE;
	}
#endif

  return (TRUE);
}

static int
userNameOk (uid_t uid, const char *lsfUserName)
{

	if (uid == 0 || nClusAdmins == 0)
	{
		return TRUE;
	}

	for( uint i = 0; i < nClusAdmins; i++ )
	{
		if (strcmp (lsfUserName, clusAdminNames[i]) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void
limDebugReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
	char *buf         = malloc( sizeof( char ) * MAXHOSTNAMELEN + 1 );// FIXME FIXME FIXME ; MAXHOSTNAMELEN? why?
	char *logFileName = malloc( sizeof( char ) * MAX_LSF_NAME_LEN + 1 ); // FIXME FIXME FIXME ; MAX_LSF_NAME_LEN? why?
	char *lsfLogDir   = malloc( sizeof( char ) * MAX_PATH_LEN + 1 );    // FIXME FIXME FIXME ; MAX_PATH_LEN? why? configurable from os limits
	char *dir         = NULL;
	enum limReplyCode limReplyCode;
	struct debugReq debugReq = { };
	struct LSFHeader replyHdr = { };
	XDR xdrs2 = { };

	// FIXME FIXME FIXME FIXME throw in debugger, see if the two memset below are needed	
	// memset (logFileName, 0, strlen (logFileName));
	// memset (lsfLogDir, 0, strlen (lsfLogDir));

  initLSFHeader_ (&replyHdr);
  if (!lim_debug)
	{
	  if (!limPortOk (from))
	{
	  limReplyCode = LIME_DENIED;
	  goto Reply;
	}
	}
  if (!xdr_debugReq (xdrs, &debugReq, reqHdr))
	{
	  limReplyCode = LIME_BAD_DATA;
	  goto Reply;
	}
  if (logclass & LC_TRACE)
	ls_syslog (LOG_DEBUG,
		   "New debug data is: class=%x, level=%d, options=%d,filename=%s \n",
		   debugReq.logClass, debugReq.level, debugReq.options,
		   debugReq.logFileName);
  if (((dir = strrchr (debugReq.logFileName, '/')) != NULL) ||
	  ((dir = strrchr (debugReq.logFileName, '\\')) != NULL))
	{
	  dir++;
	  ls_strcat (logFileName, sizeof (logFileName), dir);
	  *(--dir) = '\0';
	  ls_strcat (lsfLogDir, sizeof (lsfLogDir), debugReq.logFileName);
	}
  else
	{
	  ls_strcat (logFileName, sizeof (logFileName), debugReq.logFileName);

	  if (limParams[LSF_LOGDIR].paramValue
	  && *(limParams[LSF_LOGDIR].paramValue))
	{
	  ls_strcat (lsfLogDir, sizeof (lsfLogDir),
			 limParams[LSF_LOGDIR].paramValue);
	}
	  else
	{
	  lsfLogDir[0] = '\0';
	}
	}
  if (debugReq.options == 1)
	doReopen ();
  else if (debugReq.opCode == LIM_DEBUG)
	{
	  putMaskLevel (debugReq.level, &(limParams[LSF_LOG_MASK].paramValue));

	  if (debugReq.logClass >= 0)
	logclass = debugReq.logClass;

	  if (debugReq.level >= 0 || debugReq.logFileName[0] != '\0')
	{


	  closelog ();
	  if (lim_debug > 1)
		ls_openlog (logFileName, lsfLogDir,
			TRUE, limParams[LSF_LOG_MASK].paramValue);
	  else
		ls_openlog (logFileName, lsfLogDir,
			FALSE, limParams[LSF_LOG_MASK].paramValue);
	}

	}
  else
	{

		if (debugReq.level >= 0) {
			timinglevel = debugReq.level;
		}
	  if (debugReq.logFileName[0] != '\0')
	{

	  closelog ();
	  if (lim_debug > 1)
		ls_openlog (logFileName, lsfLogDir, TRUE, limParams[LSF_LOG_MASK].paramValue);
	  else
		ls_openlog (logFileName, lsfLogDir, FALSE, limParams[LSF_LOG_MASK].paramValue);
	}
	}
  limReplyCode = LIME_NO_ERR;

Reply:  // FIXME FIXME FIXME FIXME remove label
  xdrmem_create (&xdrs2, buf, MAXHOSTNAMELEN, XDR_ENCODE);
  replyHdr.opCode = (short) limReplyCode;
  replyHdr.refCode = reqHdr->refCode;
  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
	  xdr_destroy (&xdrs2);
	  return;
	}
  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "chanSendDgram_", sockAdd2Str_ (from));
	  xdr_destroy (&xdrs2);
	  return;
	}
  xdr_destroy (&xdrs2);
  return;

}

static void
doReopen (void)
{
	struct config_param *plp = NULL;
	char *sp = NULL;

  for (plp = limParams; plp->paramName != NULL; plp++)
	{
	  if (plp->paramValue != NULL)
	FREEUP (plp->paramValue);
	}
  if (initenv_ (limParams, env_dir) < 0)
	{

	  sp = getenv ("LSF_LOGDIR");
	  if (sp != NULL)
	limParams[LSF_LOGDIR].paramValue = sp;
	  ls_openlog ("lim", limParams[LSF_LOGDIR].paramValue, (lim_debug == 2),
		  limParams[LSF_LOG_MASK].paramValue);
	  ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_MM, __func__, "ls_openlog",
		 limParams[LSF_LOGDIR].paramValue);
	  lim_Exit (__func__);
	}


  getLogClass_ (limParams[LSF_DEBUG_LIM].paramValue,
		limParams[LSF_TIME_LIM].paramValue);
  closelog ();

  if (lim_debug > 1)
	ls_openlog ("lim", limParams[LSF_LOGDIR].paramValue, TRUE, "LOG_DEBUG");
  else
	ls_openlog ("lim", limParams[LSF_LOGDIR].paramValue, FALSE,
		limParams[LSF_LOG_MASK].paramValue);
  if (logclass & (LC_TRACE | LC_HANG))
	ls_syslog (LOG_DEBUG, "doReopen: logclass=%x", logclass);

  return;
}
