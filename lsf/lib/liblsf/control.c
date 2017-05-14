/*
 * Copyright (C) 2011 David Bigagli
 *
 * $Id: lib.control.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/xdr.h"

// FIXME move function prototype to header file
int oneLimDebug (struct debugReq *pdebug, char *hostname);

int
ls_limcontrol (char *hname, int opCode)
{
  enum limReqCode limReqCode;
  struct lsfAuth auth ;

  memset (&auth, 0, sizeof (struct lsfAuth));

  switch (opCode)
	{
	case LIM_CMD_SHUTDOWN:
	  limReqCode = LIM_SHUTDOWN;
	  break;
	case LIM_CMD_REBOOT:
	  limReqCode = LIM_REBOOT;
	  break;
	default:
	  lserrno = LSE_BAD_OPCODE;
	  return -1;
	}

  putEauthClientEnvVar ("user");
  putEauthServerEnvVar ("lim");
  getAuth_ (&auth, hname);

  if (callLim_ (limReqCode,
		&auth, xdr_lsfAuth, NULL, NULL, hname, 0, NULL) < 0)
	return -1;

  return 0;

}

int
ls_lockhost (time_t duration)
{
  return setLockOnOff_ (LIM_LOCK_USER, duration, NULL);

}

int
ls_unlockhost (void)
{
  return setLockOnOff_ (LIM_UNLOCK_USER, 0, NULL);

}

int
lockHost_ (time_t duration, char *hname)
{
  return setLockOnOff_ (LIM_LOCK_USER, duration, hname);

}

int
unlockHost_ (char *hname)
{
  return setLockOnOff_ (LIM_UNLOCK_USER, 0, hname);

}

// FIXME FIXME int on_ was "int on". compiler complained about decleration here 
//    shadowing a variable in the global scope. remove '_' and investiage
//    error produced: 
/*liblsf/control.c:93:20: warning: declaration shadows a variable in the global scope [-Wshadow]
setLockOnOff_ (int on, time_t duration, char *hname)
				   ^
../include/daemons/libresd/resd.h:88:12: note: previous declaration is here
extern int on;
*/
int
setLockOnOff_ (int on_, time_t duration, char *hname)
{
	struct limLock lockReq;
	char *host = hname;

	if (initenv_ (NULL, NULL) < 0) {
		return -1;
	}

	lockReq.on = on_;

	lockReq.uid = getuid ();

	if (getLSFUser_ (lockReq.lsfUserName, sizeof (lockReq.lsfUserName)) < 0) {
	  return -1;
	}

	if (duration == 0)  {
		lockReq.time = 77760000;
	}
	else {
		lockReq.time = duration;
	}

	if (host == NULL) {
		host = ls_getmyhostname ();
	}

	if (callLim_ (LIM_LOCK_HOST, &lockReq, xdr_limLock, NULL, NULL, host, 0, NULL) < 0) {
		return -1;
	}

	return 0;
}

int
oneLimDebug (struct debugReq *pdebug, char *hostname)
{
	struct debugReq debugData;
	char *host = hostname;
	char space[] = " ";
	enum limReqCode limReqCode;

	limReqCode = LIM_DEBUGREQ;
	debugData.opCode = pdebug->opCode;
	debugData.logClass = pdebug->logClass;
	debugData.level = pdebug->level;
	debugData.hostName = space;
	debugData.options = pdebug->options;
	strcpy (debugData.logFileName, pdebug->logFileName);

	if (callLim_ (limReqCode, &debugData, xdr_debugReq, NULL, NULL, host, 0, NULL) < 0) {
		return -1;
	}

	return 0;

}

/* ls_servavail()
 * Send alive heartbeat from res to lim.
 */
int
ls_servavail (int servId, int nonblock)
{
	int options = 0;

	if (nonblock) {
		options |= _NON_BLOCK_;
	}

	if (initenv_ (NULL, NULL) < 0) {
		return -1;
	}

	if (callLim_ (LIM_SERV_AVAIL, &servId, xdr_int, NULL, NULL, ls_getmyhostname (), options, NULL) < 0) {
		return -1;
	}

	return 0;
}
