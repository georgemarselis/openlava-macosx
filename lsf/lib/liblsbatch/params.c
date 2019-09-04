/* $Id: lsb.params.c 397 2007-11-26 19:04:00Z mblack $
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
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <pwd.h>

#include "lsb/lsb.h"
#include "lib/xdr.h"


// struct parameterInfo *lsb_parameterinfo (char **names, unsigned int *numUsers, int options)
// Description: Get the parameters under which each mbd will run with


struct parameterInfo *
lsb_parameterinfo (char **names, unsigned int *numUsers, int options)
{
    struct parameterInfo paramInfo;
    struct infoReq infoReq;
    // static bool alloc = false;
    mbdReqType mbdReqtype;
    XDR xdrs;
    struct parameterInfo *reply = NULL;
    struct LSFHeader hdr;
    char *request_buf = NULL;
    char *reply_buf   = NULL;
    unsigned int cc = 0;

    infoReq.options = options;
    // if (alloc == true) {
    //     alloc = false;
    //     FREEUP (infoReq.names);
    // }

    if (numUsers) {
        infoReq.numNames = *numUsers;
    }
    else {
        infoReq.numNames = 0;
    }
    
    if( names) {
        infoReq.names = names;
    }
    else {
        infoReq.names =  malloc ( strlen ( names ) );
        if ( NULL == infoReq.names && ENOMEM == errno ) {
           lsberrno = LSBE_NO_MEM;
           return NULL;
        }

        // alloc = true;
        memset( infoReq.names, '\0', strlen( infoReq.names ) );
        cc = 1;
    }

    // infoReq.resReq = NULL;
    infoReq->resReq = NULL;
    mbdReqtype      = BATCH_PARAM_INFO;
    assert( cc >= 0 );
    assert( ( sizeof (struct infoReq) + cc * MAXHOSTNAMELEN + cc + 100 ) < INT_MAX );
    cc =    ( sizeof (struct infoReq) + cc * MAXHOSTNAMELEN + cc + 100 );
    request_buf = malloc( cc * sizeof( request_buf ) );
    if ( NULL == request_buf && ENOMEM == errno ) {
        lsberrno = LSBE_NO_MEM;
        return NULL;
    }

    assert( cc >= 0 );
    xdrmem_create (&xdrs, request_buf,  cc, XDR_ENCODE);

    hdr.opCode = mbdReqtype;
    if (!xdr_encodeMsg (&xdrs, (char *) &infoReq, &hdr, xdr_infoReq, 0, NULL) ) {
        xdr_destroy (&xdrs);
        free (request_buf);
        lsberrno = LSBE_XDR;
        return NULL;
    }

    assert( XDR_GETPOS (&xdrs) <= INT_MAX );
    cc = callmbd (NULL, request_buf, (int) XDR_GETPOS (&xdrs), &reply_buf, &hdr, NULL, NULL, NULL);
    if ( -1 == cc ) {
        xdr_destroy (&xdrs);
        free (request_buf);
        return NULL;
    }

    xdr_destroy (&xdrs);
    free (request_buf);

    lsberrno = hdr.opCode;
    if (lsberrno == LSBE_NO_ERROR || lsberrno == LSBE_BAD_USER) {
        assert( XDR_DECODE_SIZE_ (cc) >= 0);
        xdrmem_create (&xdrs, reply_buf, (unsigned int)XDR_DECODE_SIZE_ (cc), XDR_DECODE);
        reply = &paramInfo;
        if( !xdr_parameterInfo( &xdrs, reply, &hdr ) ) {
           lsberrno = LSBE_XDR;
           xdr_destroy (&xdrs);
           if (cc) {
               free (reply_buf);
           }
           return NULL;
        }
        xdr_destroy (&xdrs);
        if (cc) {
            free (reply_buf);
        }
        return reply;
    }

    if (cc)  {
        free (reply_buf);
    }

    return NULL;
}
