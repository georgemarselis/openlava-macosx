/* $Id: lib.xdrrf.c 397 2007-11-26 19:04:00Z mblack $
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

#include <limits.h>

#include "lib/lib.h"
#include "lib/xdr.h"
#include "lib/lproto.h"
#include "lib/xdrrf.h"
#include "daemons/libresd/resd.h"

int
lsRecvMsg_ (int sock, char *buf, unsigned int bufLen, struct LSFHeader *hdr, char *data, bool_t (*xdrFunc) (), size_t (*readFunc) ())
{
    XDR xdrs;
    int cc = 0;

    xdrmem_create (&xdrs, buf, bufLen, XDR_DECODE);
    if ((cc = readDecodeHdr_ (sock, buf, readFunc, &xdrs, hdr)) < 0) {
        xdr_destroy (&xdrs);
        return cc;
    }

    if (hdr->length == 0 || data == NULL) {
        xdr_destroy (&xdrs);
        return 0;
    }

    XDR_SETPOS (&xdrs, 0);
    if ((cc = readDecodeMsg_ (sock, buf, hdr, readFunc, &xdrs, data, xdrFunc, NULL)) < 0) {
        xdr_destroy (&xdrs);
        return cc;
    }

    return 0;
}

// FIXME FIXME FIXME FIXME replace long (*writeFunc) with size_t (*writeFunc)
int
lsSendMsg_ (int s, unsigned short opCode, size_t hdrLength, char *data, char *reqBuf, size_t reqLen, bool_t (*xdrFunc) (), long (*writeFunc) (),  struct lsfAuth *auth)
{
    struct LSFHeader hdr;
    XDR xdrs;

    initLSFHeader_ (&hdr);
    hdr.opCode = opCode;
    hdr.refCode = getCurrentSN( );

    if (!data) {
        hdr.length = hdrLength;
    }

    assert( reqLen <= UINT_MAX );
    xdrmem_create (&xdrs, reqBuf, (uint) reqLen, XDR_ENCODE);
    if (!xdr_encodeMsg (&xdrs, data, &hdr, xdrFunc, 0, auth)) {
        xdr_destroy (&xdrs);
        lserrno = LSE_BAD_XDR;
        return -1;
    }

    if ((*writeFunc) (s, (char *) reqBuf, XDR_GETPOS (&xdrs)) != XDR_GETPOS (&xdrs)) {
        xdr_destroy (&xdrs);
        lserrno = LSE_MSG_SYS;
        return -2;
    }
    
    xdr_destroy (&xdrs);
    
    return 0;
}
