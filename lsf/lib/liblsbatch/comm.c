/* $Id: lsb.comm.c 397 2007-11-26 19:04:00Z mblack $
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

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <math.h>
#include <pwd.h>
#include <sys/ioctl.h>

#include "lsb/lsb.h"
#include "lib/lib.h"


extern int _lsb_conntimeout;
extern int _lsb_recvtimeout;

static int mbdTries (void);

int lsb_mbd_version = -1;


#define   NL_SETN     13
#define MAXMSGLEN     (1<<24)

int
serv_connect (char *serv_host, unsigned short serv_port, int timeout)
{
    int chfd    = 0;
    int cc      = 0;
    int options = 0;
    struct sockaddr_in serv_addr = { };
    const struct hostent *hp;

    memset ( &serv_addr, 0, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    if ((hp = Gethostbyname_ (serv_host)) == 0) {
        lsberrno = LSBE_BAD_HOST;
        return -1;
    }

    memcpy( &serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
    serv_addr.sin_port = serv_port;

    if (geteuid () == 0) {
        options = CHAN_OP_PPORT;
    }
    else {
        options = 0;
    }

    chfd = chanClientSocket_ (AF_INET, SOCK_STREAM, options);
    if (chfd < 0) {
        lsberrno = LSBE_LSLIB;
        return (-1);
    }

    cc = chanConnect_ (chfd, &serv_addr, timeout * 1000);
    if (cc < 0) {
        switch (lserrno) {
            case LSE_TIME_OUT:
                lsberrno = LSBE_CONN_TIMEOUT;
                break;
            case LSE_CONN_SYS:
                if (errno == ECONNREFUSED || errno == EHOSTUNREACH) {
                    lsberrno = LSBE_CONN_REFUSED;
                }
                else {
                    lsberrno = LSBE_SYS_CALL;
                }
                break;
            default:
                lsberrno = LSBE_SYS_CALL;
        }
        chanClose_ (chfd);
        return -1;
    }

    return (chfd);
}

int
call_server (char *host, unsigned short serv_port, char *req_buf, size_t req_size,  char **rep_buf, struct LSFHeader *replyHdr, int conn_timeout, int recv_timeout, int *connectedSock, int (*postSndFunc) (), int *postSndFuncArg, int flags)
{
    int cc;
    static char __func__] = "call_server";
    struct Buffer *sndBuf;
    struct Buffer reqbuf, reqbuf2, replybuf;
    struct Buffer *replyBufPtr;
    int serverSock;

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG1, "callserver: Entering this routine...");
    }

    *rep_buf = NULL;
    lsberrno = LSBE_NO_ERROR;

    if (!(flags & CALL_SERVER_USE_SOCKET)) {
        if ((serverSock = serv_connect (host, serv_port, conn_timeout)) < 0) {
            return (-2);
        }
    }
    else {
        if (connectedSock == NULL) {
            /* catgets 5000 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5000, "%s: CALL_SERVER_USE_SOCKET defined, but %s is NULL"), fname, "connectedSock");
            lsberrno = LSBE_BAD_ARG;
            return (-2);
        }
        serverSock = *connectedSock;
    }

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG1, "%s: serv_connect() get server sock <%d>", fname, serverSock);
    }

    if (!(flags & CALL_SERVER_NO_HANDSHAKE)) {

        if (handShake_ (serverSock, TRUE, conn_timeout) < 0) {
            CLOSECD (serverSock);
            if (logclass & LC_COMM) {
                ls_syslog (LOG_DEBUG, "%s: handShake_(socket=%d, conn_timeout=%d) failed", fname, serverSock, conn_timeout);
            }
            return (-2);
        }

        if (logclass & LC_COMM) {
            ls_syslog (LOG_DEBUG1, "%s: handShake_() succeeded", fname);
        }
    }

    CHAN_INIT_BUF (&reqbuf);
    reqbuf.len = req_size;
    reqbuf.data = req_buf;

    if (postSndFunc) {
        CHAN_INIT_BUF (&reqbuf2);
        reqbuf2.len = ((struct lenData *) postSndFuncArg)->len;     // FIXME FIXME FIXME run in the debugger, need to trace what is passed on
        reqbuf2.data = ((struct lenData *) postSndFuncArg)->data;   // FIXME FIXME FIXME run in the debugger, need to trace what is passed on
        reqbuf.forw = &reqbuf2;
    }

    if (flags & CALL_SERVER_NO_WAIT_REPLY) {
        replyBufPtr = NULL;
    }
    else {
        replyBufPtr = &replybuf;
    }

    if (flags & CALL_SERVER_ENQUEUE_ONLY) {
        size_t tsize = req_size;

        if (logclass & LC_COMM) {
            ls_syslog (LOG_DEBUG2, "callserver: Enqueue only");
        }

        if (chanSetMode_ (serverSock, CHAN_MODE_NONBLOCK) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, "callserver", "chanSetMode");
            CLOSECD (serverSock);
            return -2;
        }

        if (postSndFunc) {
            tsize += ((struct lenData *) postSndFuncArg)->len + NET_INTSIZE_;  // FIXME FIXME FIXME run in the debugger, need to trace what is passed on
        }

        assert( tsize <= INT_MAX );
        if (chanAllocBuf_ (&sndBuf, (int)tsize) < 0)    {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "chanAllocBuf_",  tsize);
            CLOSECD (serverSock);
            return -2;
        }

        sndBuf->len = (size_t)tsize;
        memcpy ((char *) sndBuf->data, (char *) req_buf, req_size);

        if (postSndFunc) {

            unsigned int nlen = htonl (((struct lenData *) postSndFuncArg)->len); // FIXME FIXME FIXME run in the debugger, need to trace what is passed on

            memcpy ((char *) sndBuf->data + req_size, (char *) NET_INTADDR_ (&nlen), NET_INTSIZE_);
            memcpy ((char *) sndBuf->data + req_size + NET_INTSIZE_, (char *) ((struct lenData *) postSndFuncArg)->data, ((struct lenData *) postSndFuncArg)->len);
        }

        if (chanEnqueue_ (serverSock, sndBuf) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_ENO_D, fname, "chanEnqueue_", cherrno);
            chanFreeBuf_ (sndBuf);
            CLOSECD (serverSock);
            return -2;
        }
    }
    else {
        cc = chanRpc_ (serverSock, &reqbuf, replyBufPtr, replyHdr, recv_timeout * 1000);
        if (cc < 0) {
            lsberrno = LSBE_LSLIB;
            CLOSECD (serverSock);
            return (-1);
        }
    }

    if (flags & CALL_SERVER_NO_WAIT_REPLY) {
        *rep_buf = NULL;
    }
    else {
        *rep_buf = replybuf.data;
    }

    if (connectedSock) {
        *connectedSock = serverSock;
    }
    else {
        chanClose_ (serverSock);
        }

    if (flags & CALL_SERVER_NO_WAIT_REPLY) {
        return (0);
    }
    else {
        assert( replyHdr->length <= INT_MAX );
        return ((int)replyHdr->length);
    }

}

int
getServerMsg (int serverSock, struct LSFHeader *replyHdr, char **rep_buf)
{
    static char __func__] = "getServerMsg";
    size_t len;
    struct LSFHeader hdrBuf;
    XDR xdrs;

    xdrmem_create (&xdrs, (char *) &hdrBuf, sizeof (struct LSFHeader), XDR_DECODE);
    if (readDecodeHdr_ (serverSock, (char *) &hdrBuf, b_read_fix, &xdrs, replyHdr) < 0) {
        if (LSE_SYSCALL (lserrno))  {
            lsberrno = LSBE_SYS_CALL;
            if (logclass & LC_COMM) {
                ls_syslog (LOG_DEBUG1, "%s: readDecodeHdr_() failed for read reply header from mbatchd: %m", fname);
            }
        }
        else {
            lsberrno = LSBE_XDR;
        }
        closesocket (serverSock);
        xdr_destroy (&xdrs);
        return (-1);
    }

    xdr_destroy (&xdrs);
    len = replyHdr->length;
    lsb_mbd_version = replyHdr->version;
    if (len > 0) {
        if (len > MAXMSGLEN) {
            closesocket (serverSock);
            lsberrno = LSBE_PROTOCOL;
            if (logclass & LC_COMM) {
                ls_syslog (LOG_DEBUG1, "%s: mbatchd's reply header length <%d> is greater than <%d>", fname, len, MAXMSGLEN);
            }
            return (-1);
        }
        *rep_buf = (char *) malloc( len );
        if ( NULL == *rep_buf && ENOMEM == errno ) {
            closesocket (serverSock);
            lsberrno = LSBE_NO_MEM;
            if (logclass & LC_TRACE) {
                ls_syslog (LOG_DEBUG1, "call_server: malloc (%d) failed:%m", len);
            }

            return (-1);
        }

        if (b_read_fix (serverSock, *rep_buf, len) == -1) {
            closesocket (serverSock);
            free (*rep_buf);
            *rep_buf = NULL;
            lsberrno = LSBE_SYS_CALL;
            if (logclass & LC_COMM) {
                ls_syslog (LOG_DEBUG1, "%s: b_read_fix() failed for read message from mbatchd: %m", fname);
            }

            return (-1);
        }
    }

    assert( len <= MAXMSGLEN);
    return len;
}


unsigned short
get_mbd_port (void)
{
    struct servent *sv;
    static unsigned short mbd_port = 0;
    int temp_port = 0;

    if( mbd_port != 0 ) {
        return (mbd_port);
    }

    if( isint_( lsbParams[LSB_MBD_PORT].paramValue ) ) {
        temp_port = atoi (lsbParams[LSB_MBD_PORT].paramValue);
        assert( temp_port <= USHRT_MAX );
        mbd_port = (unsigned short) temp_port;
        if( mbd_port > 0) {
            return ((mbd_port = htons (mbd_port)));
        }
        else {
            mbd_port = 0;
            lsberrno = LSBE_SERVICE;
            return (0);
            }
        }

    if (lsbParams[LSB_DEBUG].paramValue != NULL) {
        return (mbd_port = htons (BATCH_MASTER_PORT));
    }

    sv = getservbyname ("mbatchd", "tcp");
    if (!sv) {
        lsberrno = LSBE_SERVICE;
        return (0);
    }
    assert( sv->s_port >= 0 && sv->s_port <= USHRT_MAX );
    return (mbd_port = (unsigned short)sv->s_port);
}


unsigned short
get_sbd_port (void)
{
    struct servent *sv;
    int             tempPort = 0;
    unsigned short  sbd_port = 0;

    // first, look at the conf to sse if the sbatch port exists there
    // base10
    tempPort = atoi(lsbParams[LSB_SBD_PORT].paramValue);
    if( tempPort < 0 ) {
        tempPort = 0;
        lsberrno = LSBE_SERVICE;
        return (0);
    }

    if( 0 == tempPort ) { // then, look up the netservices file
#  ifdef _WIN32 // microsoft
                // http://sourceforge.net/p/predef/wiki/OperatingSystems/
        tempPort = get_port_number ("sbatchd", (char *) NULL);
        if ( tempPort < 0) {
            lsberrno = LSBE_SERVICE;
            tempPort = 0;
            return (0);
        }
#  else // *nix*
        sv = getservbyname ("sbatchd", "tcp");
        if (!sv) {
            lsberrno = LSBE_SERVICE;
            return (0);
        }
        tempPort = sv->s_port;
#endif
    }

    assert( tempPort >= 0 && tempPort <= INT_MAX );
    sbd_port = (unsigned short )tempPort;
    return htons(sbd_port);
}

int
callmbd (char *clusterName, char *request_buf, int requestlen, char **reply_buf, struct LSFHeader *replyHdr, int *serverSock, int (*postSndFunc) (), int *postSndFuncArg)
{
    static char __func__] = "callmbd";
    char *masterHost;
    unsigned short mbd_port;
    int cc;
    unsigned int num = 0;
    int try = 0;
    struct clusterInfo *clusterInfo;
    XDR xdrs;
    struct LSFHeader reqHdr;

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", fname);
    }


    do { //  do .. while ( );
        // setup: get lsf master
        masterHost = getMasterName ();
        if( NULL == clusterName && NULL == masterHost ) {
            if (logclass & LC_TRACE) {
                ls_syslog (LOG_DEBUG1, "%s: getMasterName() failed", fname);
            }
            return (-1);
        }
        else {
            clusterInfo = ls_clusterinfo (NULL, &num, &clusterName, 1, 0);
            if( NULL == clusterInfo ) {
                if (logclass & LC_TRACE) {
                    ls_syslog (LOG_DEBUG1, "%s: ls_clusterinfo() failed", fname);
                }
                lsberrno = LSBE_BAD_CLUSTER;
                return (-1);
            }

            if (clusterInfo[0].status & CLUST_STAT_OK) {
                masterHost = clusterInfo[0].masterName;
                if (logclass & LC_TRACE) {
                    ls_syslog (LOG_DEBUG1, "%s: master host identified", fname);
                }
            }
            else {
                lsberrno = LSBE_BAD_CLUSTER;
                if (logclass & LC_TRACE) {
                    ls_syslog (LOG_DEBUG1, "%s: Remote cluster not OK", fname);
                }
                return (-1);
            }
        }


        if (logclass & LC_TRACE) {
            ls_syslog (LOG_DEBUG1, "%s: masterHost=%s", fname, masterHost);
        }

        assert( requestlen >= 0);
        xdrmem_create (&xdrs, request_buf, requestlen, XDR_DECODE);
        if (!xdr_LSFHeader (&xdrs, &reqHdr)) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "xdr_LSFHeader");
            xdr_destroy (&xdrs);
            return (-1);
        }

        mbd_port = get_mbd_port();
        xdr_destroy (&xdrs);
        if (logclass & LC_TRACE) {
            ls_syslog (LOG_DEBUG1, "%s: mbd_port=%d", fname, ntohs (mbd_port));
        }

        assert( requestlen >= 0);
        // call server returns hdr->lenght if successful
        //                       0 if flags & CALL_SERVER_NO_WAIT_REPLY ?
        //                      -1 if the rpc call was not successful
        //                      -2 if cannot connect/timeout/misconfigure
        cc = call_server (masterHost, mbd_port, request_buf, (size_t) requestlen, reply_buf, replyHdr,
                            _lsb_conntimeout,  _lsb_recvtimeout,  serverSock,  postSndFunc, postSndFuncArg, CALL_SERVER_NO_HANDSHAKE);
        if (logclass & LC_TRACE) {
            ls_syslog (LOG_DEBUG3, "call_server: cc=%d lsberrno=%d lserrno=%d", cc, lsberrno, lserrno);
        }

        // select() for a bit, in case the daemon needs time to come up
        if (lsberrno == LSBE_CONN_REFUSED) {
            millisleep_ (_lsb_conntimeout * 1000);
        }

        if (cc == -2  &&
                ( lsberrno == LSBE_CONN_TIMEOUT || lsberrno == LSBE_CONN_REFUSED ||
                    ( lsberrno == LSBE_LSLIB &&
                        ( lserrno == LSE_TIME_OUT || lserrno == LSE_LIM_DOWN || lserrno == LSE_MASTR_UNKNW || lserrno == LSE_MSG_SYS )
                    )
                ) &&
                try < mbdTries()
           )
        {
            /* catgets 1 */
            fprintf (stderr, "catgets 1: batch system daemon not responding ... still trying\n");
            if (logclass & LC_TRACE) {
                ls_syslog (LOG_DEBUG1, "%s: callmbd() failed: %M", fname);
            }


        }
        if (cc == -1 ) {
            return (-1);
        }

        try++;
    } while( cc < 1 ); // condition is satisfied whem master batch daemon is called/identified

    return (cc);
}


int
cmdCallSBD_ (char *sbdHost, char *request_buf, int requestlen, char **reply_buf, struct LSFHeader *replyHdr, int *serverSock)
{
    static char __func__] = "cmdCallSBD_";
    unsigned short sbdPort;
    int cc;

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG, "%s: Entering this routine... host=<%s>", fname, sbdHost);
    }

    sbdPort = get_sbd_port ();

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG, "%s: sbd_port=%d", fname, ntohs (sbdPort));
    }

    assert( requestlen >= 0);
    cc = call_server (sbdHost, sbdPort, request_buf, (size_t) requestlen, reply_buf,
                      replyHdr, _lsb_conntimeout,
                      _lsb_recvtimeout ? _lsb_recvtimeout : 30,
                      serverSock, NULL, NULL, CALL_SERVER_NO_HANDSHAKE
                    );
    if (cc < 0) {
        if (logclass & LC_COMM) {
            ls_syslog (LOG_DEBUG, "%s: cc=%d lsberrno=%d lserrno=%d", fname, cc, lsberrno, lserrno);
        }
        return (-1);
    }

    return (cc);
}


static int
mbdTries (void)
{
    char *tries;
    static int ntries = -1;

    if (ntries >= 0) {
        return (ntries);
    }

    if ((tries = getenv ("LSB_NTRIES")) == NULL) {
        ntries = INFINIT_INT;
    }
    else {
        ntries = atoi (tries);
    }

    return (ntries);
}



char *
getMasterName (void)
{
    char *masterHost;
    int try = 0;

    do {
        masterHost = ls_getmastername( );
        if( NULL == masterHost ) {
            /* catgets 2 */
            fprintf (stderr, "catgets 2: LSF daemon LIM  is not responding; still trying\n");
            millisleep_ (_lsb_conntimeout * 1000);
            try++;
            lsberrno = LSBE_LSLIB;
        }
        else if ( NULL != masterHost ) {
            break;
        }

    } while( try < mbdTries() && (lserrno == LSE_TIME_OUT || lserrno == LSE_LIM_DOWN || lserrno == LSE_MASTR_UNKNW ) );

    return masterHost;
}


int
readNextPacket (char **msgBuf, int timeout, struct LSFHeader *hdr, int serverSock)
{
    struct Buffer replyBuf;
    int cc;
    
    if (serverSock < 0 ) {
        lsberrno = LSBE_CONN_NONEXIST;
        return -1;
    }
    
    cc = chanRpc_ (serverSock, NULL, &replyBuf, hdr, timeout * 1000);
    if (cc < 0) {
        lsberrno = LSBE_LSLIB;
        return (-1);
    }
    
    if (hdr->length == 0) {
        CLOSECD (serverSock);
        lsberrno = LSBE_EOF;
        return (-1);
    }
    *msgBuf = replyBuf.data;

    return hdr->reserved;
}

void
closeSession (int serverSock)
{
    chanClose_ (serverSock);
    
}

int
handShake_ (int s, char client, int timeout)
{
    struct LSFHeader hdr, buf;
    int cc;
    XDR xdrs;
    struct Buffer reqbuf, replybuf;
    
    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "handShake_: Entering this routine...");
    }
    
    if (client) {
        
        memset ((char *) &hdr, 0, sizeof (struct LSFHeader));
        hdr.opCode = PREPARE_FOR_OP;
        hdr.length = 0;
        xdrmem_create (&xdrs, (char *) &buf, sizeof (struct LSFHeader), XDR_ENCODE);
        if (!xdr_LSFHeader (&xdrs, &hdr))  {
            lsberrno = LSBE_XDR;
            xdr_destroy (&xdrs);
            return (-1);
        }

        xdr_destroy (&xdrs);
        
        CHAN_INIT_BUF (&reqbuf);
        reqbuf.data = (char *) &buf;
        reqbuf.len = LSF_HEADER_LEN;
        
        cc = chanRpc_(s, &reqbuf, &replybuf, &hdr, timeout * 1000);
        if (cc < 0) {
            lsberrno = LSBE_LSLIB;
            return (-1);
        }
        if (hdr.opCode != READY_FOR_OP) {

            xdr_destroy (&xdrs);
            lsberrno = hdr.opCode;
            if (logclass & LC_TRACE) {
                ls_syslog (LOG_DEBUG1, "handShake_: mbatchd returned error code <%d>", hdr.opCode);
            }
            return (-1);
        }
        
    }
    
    return (0);
    
}

int
authTicketTokens_ (struct lsfAuth *auth, char *toHost)
{
    if (toHost == NULL) {

        char *clusterName;
        char buf[1024];
        if ((toHost = getMasterName ()) == NULL) {
            return (-1);
        }
        if ((clusterName = ls_getclustername ()) == NULL) {
            return -1;
        }
        sprintf (buf, "mbatchd@%s", clusterName);
        putEnv ("LSF_EAUTH_SERVER", buf);
    }
    else {
        putEnv ("LSF_EAUTH_SERVER", "sbatchd");
    }

    putEnv ("LSF_EAUTH_CLIENT", "user");

    if (getAuth_ (auth, toHost) == -1) {
        lsberrno = LSBE_LSLIB;
        return (-1);
    }

    return (0);
}

// getCpuFactor:
//      get a float value representing the weight of the Cpu of the host
//          the float value is used to

float *
getCpuFactor (char *host, int name)
{
    float *tempPtr;

    do {
        // checking condition here
        if (name == TRUE) {
            tempPtr = ls_gethostfactor( host );
        }
        else {
            tempPtr = ls_getmodelfactor( host );
        }

        if (lserrno == LSE_TIME_OUT ) {
            fprintf( stderr, "catgets 2: LIMd has timed out\n");
        }
        else if( lserrno == LSE_LIM_DOWN ) {
            fprintf( stderr, "catgets 2: LIMd is down\n");
        }
        else if( lserrno == LSE_MASTR_UNKNW ) {
            fprintf( stderr, " catgets 2: Master host unknown\n");
        }
        else {
            lsberrno = LSBE_LSLIB;
        }

        millisleep_ (_lsb_conntimeout * 1000);

    } while ( NULL == tempPtr );

    return (tempPtr);
}
