/* $Id: lsb.msg.c 397 2007-11-26 19:04:00Z mblack $
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
#include <netdb.h>
#include <string.h>
#include <pwd.h>

#include "lsb/lsb.h"
#include "lib/xdr.h"

int
lsb_msgjob (LS_LONG_INT jobId, char *msg)
{
    int cc;
    char *reply_buf;
    char request_buf[MSGSIZE];
    char dest[LSB_MAX_SD_LENGTH];
    char src[LSB_MAX_SD_LENGTH];
    struct passwd *pw;
    struct lsbMsg jmsg;
    struct LSFHeader hdr;
    struct lsbMsgHdr header;
    XDR xdrs;
    mbdReqType mbdReqtype;

    header.src = src;
    header.dest = dest;
    jmsg.header = &header;

    // TIMEIT (0, (
    pw = getpwuid (getuid ());
    //) , "getpwuid");
    if( NULL == pw ) {
        lsberrno = LSBE_BAD_USER;
        return -1;
    }

    jmsg.header->usrId = pw->pw_uid;
    jmsg.header->jobId = jobId;
    jmsg.msg = msg;
    strcpy (jmsg.header->src, "lsbatch");
    strcpy (jmsg.header->dest, "user job");
    jmsg.header->msgId = 999;
    jmsg.header->type = -1;

    mbdReqtype = BATCH_JOB_MSG;
    xdrmem_create (&xdrs, request_buf, MSGSIZE, XDR_ENCODE);

    hdr.opCode = mbdReqtype;
    if (!xdr_encodeMsg (&xdrs, (char *) &jmsg, &hdr, xdr_lsbMsg, 0, NULL)) {
        lsberrno = LSBE_XDR;
        xdr_destroy (&xdrs);
        return -1;
    }

    assert( XDR_GETPOS (&xdrs) <= INT_MAX );
    cc = callmbd (NULL, request_buf, XDR_GETPOS (&xdrs), &reply_buf, &hdr, NULL, NULL, NULL);

    if (cc < 0) {
        xdr_destroy (&xdrs);
        return -1;
    }

    xdr_destroy (&xdrs);
    if (cc != 0) {
        free (reply_buf);
    }

    lsberrno = hdr.opCode;
    if (lsberrno == LSBE_NO_ERROR) {
        return 0;
    }
    else {
        return -1;
    }
}
