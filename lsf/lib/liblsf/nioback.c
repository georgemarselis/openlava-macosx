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
#include "lib/lproto.h"
#include "lib/xdrnio.h"
#include "daemons/libresd/rescom.h"
#include "daemons/libresd/resd.h"

/* FIXME macro is not in use */
#define NL_SETN 23

int
niosCallback_ (struct sockaddr_in *from, u_short port, int rpid, int exitStatus, int terWhiPendStatus)
{
    static char __func__] = "niosCallback_";
    int s;
    struct niosConnect conn;
    struct {
        struct niosConnect conn;
        char padding[4];
        struct LSFHeader hdr;
    } reqBuf;
    struct LSFHeader reqHdr;
    int resTimeout;

    struct linger linstr = { 1, 1 };

    from->sin_port = port;

    if ((s = TcpCreate_ (FALSE, 0)) < 0) {
        if (logclass & LC_EXEC) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "tcpCreate");
        }
        
        return (-1);
    }

    if (genParams_[LSF_RES_TIMEOUT].paramValue) {
        resTimeout = atoi (genParams_[LSF_RES_TIMEOUT].paramValue);
    }
    else {
        resTimeout = RES_TIMEOUT;
    }

    assert( resTimeout >= 0);
    if (b_connect_ (s, (struct sockaddr *) from, sizeof (struct sockaddr_in), (unsigned int)resTimeout) < 0)
    {
        if (logclass & LC_EXEC) {
            ls_syslog (LOG_DEBUG, "%s: connect(s=%d,%s,len=%d) failed: %m", fname, s, sockAdd2Str_ (from), sizeof (struct sockaddr_in));
        }
        closesocket (s);
        
        return (-1);
    }

    fcntl (s, F_SETFD, fcntl (s, F_GETFD) | FD_CLOEXEC);
    setsockopt (s, SOL_SOCKET, SO_LINGER, (char *) &linstr, sizeof (linstr));

    if (rpid == 0) {
        return (s);
    }

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "%s: exitStatus <%d> terWhiPendStatus <%d>", fname, exitStatus, terWhiPendStatus);
    }

    initLSFHeader_ (&reqHdr);
    reqHdr.opCode = RES2NIOS_CONNECT;
    conn.rpid = rpid;
    
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
            ls_syslog (LOG_ERR, I18N (6201, "%s: writeEncodeMsg_(%d,%d) RES2NIOS_connect failed: %M"), fname, s, rpid);
        }
      
        closesocket (s);
        return (-1);
    }

  return (s);
}
