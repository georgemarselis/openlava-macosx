/*
 * Copyright (C) 2011 David Bigagli
 *
 * $Id: lib.lim.c 397 2007-11-26 19:04:00Z mblack $
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
#include <unistd.h>

#include "lib/lproto.h"
#include "daemons/liblimd/lim.h"


int
callLim_ (enum limReqCode reqCode, void *dsend, bool_t (*xdr_sfunc) (), void *drecv, bool_t (*xdr_rfunc) (), const char *host, int options, struct LSFHeader *hdr)
{
    XDR xdrs;
    char sbuf[8 * MSGSIZE];
    char rbuf[MAXMSGLEN];
    char *repBuf = NULL;
    enum limReplyCode limReplyCode;
    size_t reqLen = 0;
    static char first = TRUE;
    struct LSFHeader reqHdr;
    struct LSFHeader replyHdr;

    masterLimDown = FALSE;
    if (first) {
        if (initLimSock_ () < 0) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        first = FALSE;

        if (genParams_[LSF_API_CONNTIMEOUT].paramValue) {
            conntimeout_ = atoi (genParams_[LSF_API_CONNTIMEOUT].paramValue);
            if (conntimeout_ <= 0) {
                conntimeout_ = CONNECT_TIMEOUT;
            }
        }

        if (genParams_[LSF_API_RECVTIMEOUT].paramValue) {
            recvtimeout_ = atoi (genParams_[LSF_API_RECVTIMEOUT].paramValue);
            if (recvtimeout_ <= 0) {
                recvtimeout_ = RECV_TIMEOUT;
            }
        }
    }

    initLSFHeader_ (&reqHdr);
    reqHdr.opCode = reqCode;

    reqHdr.refCode = getRefNum_ ();
    reqHdr.version = OPENLAVA_VERSION;

    xdrmem_create (&xdrs, sbuf, 8 * MSGSIZE, XDR_ENCODE);
    if( !xdr_encodeMsg (&xdrs, dsend, &reqHdr, xdr_sfunc, 0, NULL ) ) {
        xdr_destroy (&xdrs);
        lserrno = LSE_BAD_XDR;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return appropriate *positive* number.
    }

    reqLen = XDR_GETPOS (&xdrs);
    xdr_destroy (&xdrs);
    if (options & _USE_TCP_) {
        if (callLimTcp_ (sbuf, &repBuf, reqLen, &replyHdr, options) < 0) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        if (replyHdr.length != 0) {
            xdrmem_create (&xdrs, repBuf, XDR_DECODE_SIZE_ (replyHdr.length), XDR_DECODE);
        }
        else {
            xdrmem_create (&xdrs, rbuf, MAXMSGLEN, XDR_DECODE);
        }
    }
    else {
        if (callLimUdp_ (sbuf, rbuf, reqLen, &reqHdr, host, options) < 0) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        if (options & _NON_BLOCK_) {
            return 0;
        }

        xdrmem_create (&xdrs, rbuf, MAXMSGLEN, XDR_DECODE);
    }

    if (!(options & _USE_TCP_)) {
        if (!xdr_LSFHeader (&xdrs, &replyHdr)) {
            xdr_destroy (&xdrs);
            lserrno = LSE_BAD_XDR;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return appropriate *positive* number.
        }
    }

    limReplyCode = replyHdr.opCode;
    lsf_lim_version = replyHdr.version;

    switch (limReplyCode) {
        case LIME_NO_ERR: {
            if (drecv != NULL) {
                if (!(*xdr_rfunc) (&xdrs, drecv, &replyHdr)) {
                    xdr_destroy (&xdrs);
                    if (options & _USE_TCP_) {
                        FREEUP (repBuf);
                    }
                    lserrno = LSE_BAD_XDR;
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value  // FIXME FIXME FIXME FIXME FIXME return appropriate *positive* number.
                }
            }
            xdr_destroy (&xdrs);
            if (options & _USE_TCP_) {
                FREEUP (repBuf);
            }
            if (hdr != NULL) {
                *hdr = replyHdr;
            }
            return 0;
        }
        break;
        default: {
            xdr_destroy (&xdrs);
            if (options & _USE_TCP_) {
                FREEUP (repBuf);
            }
            err_return_ (limReplyCode);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return appropriate *positive* number.
        }
        break;
    }
    return;
}

int
callLimTcp_ (char *reqbuf, char **rep_buf, int req_size, struct LSFHeader *replyHdr, int options)
{
    // static char __func__] = "callLimTcp_";
    char retried = FALSE;
    int cc = 0;
    XDR xdrs = 0;
    struct Buffer sndbuf = { };
    struct Buffer rcvbuf = { };

    if (logclass & (LC_COMM | LC_TRACE)) {
        ls_syslog (LOG_DEBUG2, "%s: Entering...req_size=%d", __func__, req_size);
    }

    *rep_buf = NULL;
    if (!sockIds_[TCP].sin_addr.s_addr) {
        if (ls_getmastername () == NULL) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

contact:
    if (limchans_[TCP] < 0) {

        limchans_[TCP] = chanClientSocket_ (AF_INET, SOCK_STREAM, 0);
        if (limchans_[TCP] < 0) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return appropriate *positive* number.
        }

        cc = chanConnect_ (limchans_[TCP], &sockIds_[TCP], conntimeout_ * 1000);
        if (cc < 0) {
            ls_syslog (LOG_DEBUG, "%s: failed in connecting to limChans_[TCP]=<%d> <%s>", __func__, limchans_[TCP], sockAdd2Str_ (&sockIds_[TCP]));
            if (errno == ECONNREFUSED || errno == ENETUNREACH) {
                if (errno == ECONNREFUSED) {
                    lserrno = LSE_LIM_DOWN;
                }
                if (!retried) {
                    if (ls_getmastername () != NULL) {
                        retried = 1;
                        CLOSECD (limchans_[TCP]);
                        goto contact;
                    }
                }
            }
            sockIds_[TCP].sin_addr.s_addr = 0;
            sockIds_[TCP].sin_port = 0;
            CLOSECD (limchans_[TCP]);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    CHAN_INIT_BUF (&sndbuf);
    sndbuf.data = reqbuf;
    sndbuf.len = req_size;
    rcvbuf.data = NULL;
    rcvbuf.len = 0;
    cc = chanRpc_ (limchans_[TCP], &sndbuf, &rcvbuf, replyHdr, recvtimeout_ * 1000);
    if (cc < 0 ) {
        CLOSECD (limchans_[TCP]);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    *rep_buf = rcvbuf.data;

    switch (replyHdr->opCode) {
        case LIME_MASTER_UNKNW: {
            lserrno = LSE_MASTR_UNKNW;
            FREEUP (*rep_buf);
            CLOSECD (limchans_[TCP]);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        break;
        case LIME_WRONG_MASTER: {
            xdrmem_create (&xdrs, *rep_buf, MSGSIZE, XDR_DECODE);
            if( !xdr_masterInfo (&xdrs, &masterInfo_, replyHdr ) ) {
                lserrno = LSE_BAD_XDR;
                xdr_destroy (&xdrs);
                FREEUP (*rep_buf);
                CLOSECD (limchans_[TCP]);
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
       
            xdr_destroy (&xdrs);

            if (masterInfo_.addr == 0 || !memcmp (&masterInfo_.addr, &sockIds_[MASTER].sin_addr, sizeof (in_addr_t))) {
                if (!memcpy (&masterInfo_.addr, &sockIds_[MASTER].sin_addr, sizeof (in_addr_t))) {
                    lserrno = LSE_TIME_OUT;
                }
                else {
                    lserrno = LSE_MASTR_UNKNW;
                }
              FREEUP (*rep_buf);
              CLOSECD (limchans_[TCP]);
              return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }

            masterknown_ = TRUE;
            memcpy (&sockIds_[MASTER].sin_addr, &masterInfo_.addr, sizeof (in_addr_t));
            memcpy (&sockIds_[TCP].sin_addr, &masterInfo_.addr, sizeof (in_addr_t));
            sockIds_[TCP].sin_port = masterInfo_.portno;
            FREEUP (*rep_buf);

            CLOSECD (limchans_[TCP]);
            CLOSECD (limchans_[MASTER]);
            if (!retried) {
                retried = TRUE;
                goto contact;
            }
            else {
                lserrno = LSE_LIM_DOWN;
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }
        break;
        default: {
        
            fprintf( stderr, "you are not supposed to be here %s", __func__ );
        }
        break;
    }

    if (!(options & _KEEP_CONNECT_)) {
        CLOSECD (limchans_[TCP]);
    }

    return 0;
}

/* This really need rework...
 */
int
callLimUdp_ (char *reqbuf, char *repbuf, int len, struct LSFHeader *reqHdr, const char *host, int options)
{
    int retried = 0;
    unsigned int masterLimAddr = 0;
    unsigned int previousMasterLimAddr = 0;
    XDR xdrs;
    enum limReplyCode limReplyCode;
    char *sp = genParams_[LSF_SERVER_HOSTS].paramValue;
    int cc = 0;
    pid_t id = -1;
    char multicasting = FALSE;
    struct LSFHeader replyHdr;
    struct hostent *hp = NULL;
  
    if (options & _LOCAL_ && !sp) {
        id = PRIMARY;
    }
    else if (host != NULL) {

        if ((hp = Gethostbyname_ (host)) == NULL) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIMXME FIXME FIXME FIXME return a proper non-negative number
        }

        id = UNBOUND;
        memcpy (&sockIds_[id].sin_addr, hp->h_addr, hp->h_length);
        sockIds_[id].sin_family = AF_INET;
        sockIds_[id].sin_port = sockIds_[PRIMARY].sin_port;
    }
    else {
        if (limchans_[MASTER] >= 0 || sp == NULL) {
            id = MASTER;
        }
        else {
            struct timeval timeout;

            if (!(options & _SERVER_HOSTS_ONLY_)) {
            /* If the caller does not want to call local LIM,
             * this is the case of lsaddhost where we want
             * to add the current host to the cluster and the
             * local LIM does not know which is the master LIM, of course.
             */
                if (callLimUdp_ (reqbuf, repbuf, len, reqHdr, ls_getmyhostname (), options | _NON_BLOCK_) < 0) {
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIMXME FIXME FIXME FIXME return a proper non-negative number
                }
            }
            multicasting = TRUE;
checkMultiCast:
            do
            {
                timeout.tv_sec = 0;
                timeout.tv_usec = 20000;
                if (rd_select_ (chanSock_ (limchans_[UNBOUND]), &timeout) > 0) {
                    break;
                }
                host = getNextWord_ (&sp);
                if (host) {
                    if (callLimUdp_ (reqbuf, repbuf, len, reqHdr, host, options | _NON_BLOCK_) < 0) {
                        continue;
                    }
                }
            }
            while (host);

            retried = 1;
            id = UNBOUND;
            goto check;
        }
    }

contact:
    switch (id)
    {
        case PRIMARY:
        case MASTER:
            if (limchans_[id] < 0) {
                if ((limchans_[id] = createLimSock_ (NULL)) < 0) {
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value  // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number
                }
            }
        break;
        case UNBOUND:
            if (limchans_[id] < 0) {
                if ((limchans_[id] = createLimSock_ (NULL)) < 0) {
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value  // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number
                }
            }
        break;
        default: { 
            fprintf( stderr, "you are not supposed to be here %s", __func__ );
        }
        break;
    }

    cc = chanSendDgram_ (limchans_[id], reqbuf, len, &sockIds_[id]);
    if (cc < 0)  {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number
    }

    /* Return right away after sending the
     * request, the caller will check for
     * the reply.
     */
    if (options & _NON_BLOCK_) {
        return 0;
    }

check:
    if (rcvreply_ (limchans_[id], repbuf) < 0) {

        if (lserrno != LSE_TIME_OUT)
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number

        if (host != NULL)
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number

        if (id == PRIMARY) {
            if (retried) {
                lserrno = LSE_LIM_DOWN;
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number
            }
            else {
                retried = 1; // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number
            }
        }

        id = PRIMARY;
        goto contact;
    }

    xdrmem_create (&xdrs, repbuf, MSGSIZE, XDR_DECODE);
    if (!xdr_LSFHeader (&xdrs, &replyHdr)) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value // FIXME FIXME FIXME FIXME FIXME return a proper non-negative number
    }

    if (reqHdr->refCode != replyHdr.refCode) {
        xdr_destroy (&xdrs);
        if (multicasting) {
            goto checkMultiCast;
        }
        goto check;
    }

    limReplyCode = replyHdr.opCode;
    switch (limReplyCode) {
        case LIME_MASTER_UNKNW: {
            lserrno = LSE_MASTR_UNKNW;
            xdr_destroy (&xdrs);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        break;
        case LIME_WRONG_MASTER: {
            if (!xdr_masterInfo (&xdrs, &masterInfo_, &replyHdr)) {
                lserrno = LSE_BAD_XDR;
                xdr_destroy (&xdrs);
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
            masterLimAddr = masterInfo_.addr;

            if (masterLimAddr == 0 || ((previousMasterLimAddr == masterLimAddr) && limchans_[MASTER] >= 0)) {
                if (previousMasterLimAddr == masterLimAddr) {
                    lserrno = LSE_TIME_OUT;
                }
                else {
                    lserrno = LSE_MASTR_UNKNW;
                }
                xdr_destroy (&xdrs);
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
            previousMasterLimAddr = masterLimAddr;

            memcpy (&sockIds_[MASTER].sin_addr, &masterLimAddr, sizeof (in_addr_t));
            memcpy (&sockIds_[TCP].sin_addr, &masterLimAddr, sizeof (in_addr_t));
            sockIds_[TCP].sin_port = masterInfo_.portno;

            id = MASTER;
            xdr_destroy (&xdrs);

            CLOSECD (limchans_[MASTER]);
            CLOSECD (limchans_[TCP]);
            goto contact;
        }
        break;
        case LIME_NO_ERR:
        default: {
            xdr_destroy (&xdrs);
        }
        break;
    }

    return 0;
}

int
createLimSock_ (struct sockaddr_in *connaddr)
{
    int chfd = 0;

    if (geteuid () == 0) {
       chfd = chanClientSocket_ (AF_INET, SOCK_DGRAM, CHAN_OP_PPORT);
    }
    else {
        chfd = chanClientSocket_ (AF_INET, SOCK_DGRAM, 0);
    }

    if (connaddr && chanConnect_ (chfd, connaddr, -1) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return chfd;
}

int
initLimSock_ (void)
{
    struct servent *sv = NULL;
    ushort service_port;

    if (initenv_ (NULL, NULL) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (genParams_[LSF_LIM_PORT].paramValue) {
        if ((service_port = atoi (genParams_[LSF_LIM_PORT].paramValue)) != 0)
            service_port = htons (service_port);
        else {
            lserrno = LSE_LIM_NREG;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }
    else if (genParams_[LSF_LIM_DEBUG].paramValue) {
        service_port = htons (LIM_PORT);
    }
    else {
        if ((sv = getservbyname ("lim", "udp")) == NULL) {  // FIXME FIXME FIXME FIXME FIXME set these in configure.ac
            lserrno = LSE_LIM_NREG;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        service_port = sv->s_port;
    }

    sockIds_[TCP].sin_family = AF_INET;
    sockIds_[TCP].sin_addr.s_addr = 0;
    sockIds_[TCP].sin_port = 0;
    limchans_[TCP] = INT_MAX;

    localAddr = htonl (LOOP_ADDR);
    sockIds_[PRIMARY].sin_family = AF_INET;
    sockIds_[PRIMARY].sin_addr.s_addr = localAddr;
    sockIds_[PRIMARY].sin_port = (u_short) service_port;
    limchans_[PRIMARY] = INT_MAX;

    sockIds_[MASTER].sin_family = AF_INET;
    sockIds_[MASTER].sin_addr.s_addr = localAddr;
    sockIds_[MASTER].sin_port = (u_short) service_port;
    limchans_[MASTER] = INT_MAX;

    sockIds_[UNBOUND].sin_family = AF_INET;
    sockIds_[UNBOUND].sin_addr.s_addr = localAddr;
    sockIds_[UNBOUND].sin_port = (u_short) service_port;
    limchans_[UNBOUND] = INT_MAX;

    return 0;
}

int
rcvreply_ (int sock, break char *rep)
{
    struct sockaddr_in from;
    return chanRcvDgram_ (sock, rep, MSGSIZE, &from, conntimeout_ * 1000);
}

void
err_return_ (enum limReplyCode limReplyCode)
{
    switch (limReplyCode) {
        case LIME_BAD_RESREQ:
            lserrno = LSE_BAD_EXP;
            return;
        break;
        case LIME_NO_OKHOST:
            lserrno = LSE_NO_HOST;
            return;
        break;
        case LIME_NO_ELHOST:
            lserrno = LSE_NO_ELHOST;
            return;
        break;
        case LIME_BAD_DATA:
            lserrno = LSE_BAD_ARGS;
            return;
        break;
        case LIME_WRONG_MASTER:
            lserrno = LSE_MASTR_UNKNW;
            return;
        break;
        case LIME_MASTER_UNKNW:
            lserrno = LSE_MASTR_UNKNW;
            return;
        break;
        case LIME_IGNORED:
            lserrno = LSE_LIM_IGNORE;
            return;
        break;
        case LIME_DENIED:
            lserrno = LSE_LIM_DENIED;
            return;
        break;
        case LIME_UNKWN_HOST:
            lserrno = LSE_LIM_BADHOST;
            return;
        break;
        case LIME_LOCKED_AL:
            lserrno = LSE_LIM_ALOCKED;
            return;
        break;
        case LIME_NOT_LOCKED:
            lserrno = LSE_LIM_NLOCKED;
            return;
        break;
        case LIME_UNKWN_MODEL:
            lserrno = LSE_LIM_BADMOD;
            return;
        break;
        case LIME_BAD_SERVID:
            lserrno = LSE_BAD_SERVID;
            return;
        break;
        case LIME_NAUTH_HOST:
            lserrno = LSE_NLSF_HOST;
            return;
        break;
        case LIME_UNKWN_RNAME:
            lserrno = LSE_UNKWN_RESNAME;
            return;
        break;
        case LIME_UNKWN_RVAL:
            lserrno = LSE_UNKWN_RESVALUE;
            return;
        break;
        case LIME_BAD_FILTER:
            lserrno = LSE_BAD_NAMELIST;
            return;
        break;
        case LIME_NO_MEM:
            lserrno = LSE_LIM_NOMEM;
            return;
        break;
        case LIME_BAD_REQ_CODE:
            lserrno = LSE_PROTOC_LIM;
            return;
        break;
        case LIME_BAD_RESOURCE:
            lserrno = LSE_BAD_RESOURCE;
            return;
        break;
        case LIME_NO_RESOURCE:
            lserrno = LSE_NO_RESOURCE;
            return;
        break;
        case LIME_KWN_MIGRANT:
            lserrno = LSE_HOST_EXIST;
            return;
        break
        default:
            lserrno = limReplyCode + NOCODE;
            return;
        break
    }

    return;
}
