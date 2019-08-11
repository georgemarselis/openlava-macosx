/* $Id: lib.nioback.c 397 2007-11-26 19:04:00Z mblack $
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

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib/lib.h"
#include "lib/rdwr.h"
#include "lib/nioback.h"
#include "lib/sock.h"
#include "lib/xdrnio.h"
#include "lib/syslog.h"
#include "lib/host.h"
#include "lib/misc.h"
#include "libint/lsi18n.h"
#include "struct-config_param.h"
#include "lib/structs/genParams.h"
#include "daemons/libresd/init.h"
#include "daemons/libresd/rescom.h"
#include "lib/resd_globals.h"


/* FIXME macro is not in use */
// #define NL_SETN 23

int
niosCallback_ (struct sockaddr_in *from, u_short port, unsigned long rtaskid, int exitStatus, int terWhiPendStatus)
{
    int s = 0;
    struct niosConnect conn;
    struct {
        struct niosConnect conn;
        // char padding[4];
        struct LSFHeader hdr;
    } reqBuf;
    struct LSFHeader reqHdr;
    int resTimeout = 0;
    // struct gen

    struct linger linstr = { 1, 1 };

    from->sin_port = port;

    if ((s = TcpCreate_ (FALSE, 0)) < 0) {
        if (logclass & LC_EXEC) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "tcpCreate");
        }
        
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (genParams_[RES_TIMEOUT].paramValue) {
        resTimeout = atoi (genParams_[RES_TIMEOUT].paramValue);
    }
    else {
        resTimeout = RES_TIMEOUT;
    }

    assert( resTimeout >= 0);
    if (b_connect_ (s, (struct sockaddr *) from, sizeof (struct sockaddr_in), (unsigned int)resTimeout) < 0)
    {
        if (logclass & LC_EXEC) {
            ls_syslog (LOG_DEBUG, "%s: connect(s=%d,%s,len=%d) failed: %m", __func__, s, sockAdd2Str_ (from), sizeof (struct sockaddr_in));
        }
        close (s);
        
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    fcntl (s, F_SETFD, fcntl (s, F_GETFD) | FD_CLOEXEC);
    setsockopt (s, SOL_SOCKET, SO_LINGER, (char *) &linstr, sizeof (linstr));

    if (rtaskid == 0) {
        return s;
    }

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "%s: exitStatus <%d> terWhiPendStatus <%d>", __func__, exitStatus, terWhiPendStatus);
    }

    initLSFHeader_ (&reqHdr);
    reqHdr.opCode = RES2NIOS_CONNECT;
    conn.rtaskid = rtaskid;
    
    if (terWhiPendStatus == 1) {
        conn.exitStatus = 126;
    }
    else {
        conn.exitStatus = exitStatus;
    }
    
    conn.terWhiPendStatus = terWhiPendStatus;

    memset ((char *) &reqBuf, 0, sizeof (reqBuf));
    if (writeEncodeMsg_ (s, (char *) &reqBuf, sizeof (reqBuf), &reqHdr, (char *) &conn, nb_write_fix, xdr_niosConnect, 0) < 0) {
        if (logclass & LC_EXEC) {
            /* catgets 6201 */
            ls_syslog( LOG_ERR, "catgets 6201: %s: writeEncodeMsg_(%d,%d) RES2NIOS_connect failed: %M", __func__, s, rtaskid );
        }
      
        close (s);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return s;
}
