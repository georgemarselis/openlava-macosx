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

#include <time.h>
#include <unistd.h>

#include "lib/control.h"
#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/initenv.h"
#include "lib/xdr.h"
#include "lib/eauth.h"
#include "daemons/liblimd/lim.h"
#include "lib/common_structs.h"
#include "lib/id.h"
#include "lib/host.h"

int
ls_limcontrol (const char *hname, int opCode)
{
    enum limReqCode limReqCode;
    struct lsfAuth auth ;
    const char user[ ] = "user";
    const char lim [ ] = "lim";

    memset (&auth, 0, sizeof (struct lsfAuth));

    switch (opCode) {
        case LIM_CMD_SHUTDOWN: {
            limReqCode = LIM_SHUTDOWN;
        }
        break;
        case LIM_CMD_REBOOT: {
            limReqCode = LIM_REBOOT;
        }
        break;
        default: {
            lserrno = LSE_BAD_OPCODE;
            return -1;
        }
        break;
    }

    putEauthClientEnvVar( user ); // FIXME FIXME FIXME FIXME put in configure.ac
    putEauthServerEnvVar( lim );  // FIXME FIXME FIXME FIXME put in configure.ac
    getAuth_lsf( &auth, hname );

    if (callLim_ (limReqCode, &auth, xdr_lsfAuth, NULL, NULL, hname, 0, NULL) < 0) {
        return -1;
    }

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
lockHost_ (time_t duration, const char *hname)
{
    return setLockOnOff_ (LIM_LOCK_USER, duration, hname);
}

int
unlockHost_ ( const char *hname)
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
setLockOnOff_ (int on_, time_t duration, const char *hname)
{
    struct limLock lockReq;
    const char *host = hname;

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
oneLimDebug (struct debugReq *pdebug, const char *hostname)
{
    struct debugReq debugData;
    enum limReqCode limReqCode;

    limReqCode            = LIM_DEBUGREQ;
    debugData.opCode      = pdebug->opCode;
    debugData.logClass    = pdebug->logClass;
    debugData.level       = pdebug->level;
    debugData.hostName    = NULL;
    debugData.options     = pdebug->options;
    debugData.logFileName = NULL;
    strcpy (debugData.logFileName, pdebug->logFileName);

    if (callLim_ (limReqCode, &debugData, xdr_debugReq, NULL, NULL, hostname, 0, NULL) < 0) {
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
