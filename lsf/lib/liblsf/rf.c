/*
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

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/rf.h"
#include "lib/host.h"
#include "lib/misc.h"
#include "lib/xdrres.h"
#include "lib/rtask.h"
#include "lib/xdrrf.h"
#include "lib/rdwr.h"
#include "lib/err.h"
#include "lib/sock.h"
#include "lib/xdrmisc.h"
#include "lib/resd_globals.h"
#include "lib/taskid.h"
#include "lib/structs/genParams.h"
#include "daemons/libresd/resout.h"
#include "daemons/libpimd/pimd.h"

struct rfTab *ft      = NULL; // NOFIX
struct rHosts *rHosts = NULL; // NOFIX

struct rHosts *
rhConnect ( const char *host)
{
    struct hostent *hp   = NULL;
    struct rHosts  *rh   = NULL;
    char  *argv[2]       = { NULL, NULL }; // FIXME FIXME FIXME label the two elements of the array
    char  *hname         = NULL;
    unsigned long taskid = 0;
    int    sock          = 0;


    if (ft == NULL) {
        ft = calloc ( maxOpen, sizeof (struct rfTab));
        if ( NULL == ft ) {
            lserrno = LSE_MALLOC;
            return NULL;
        }

        for ( unsigned int i = 0; i < maxOpen; i++) {
            ft[i].host = NULL;
        }
    }

    if ((hp = Gethostbyname_ (host)) == NULL) {
        lserrno = LSE_BAD_HOST;
        return NULL;
    }

    if ((rh = rhFind (hp->h_name))) {
        rh->atime = time (NULL);
        return rh;
    }

    strcpy( argv[0], RF_SERVERD); // FIXME FIXME FIXME label argv[0]
    argv[1] = NULL;
    taskid = (unsigned long)ls_rtask (host, argv, REXF_TASKPORT | rxFlags); // FIXME FIXME FIXME return type
    // if ((taskid = (unsigned long)ls_rtask (host, argv, REXF_TASKPORT | rxFlags)) < 0) { // FIXME FIXME FIXME FIXME fix return type ls_rtask
    //     return NULL;
    // }

    sock = ls_conntaskport (taskid); // FIXME FIXME FIXME return type
    // if ((sock = ls_conntaskport (taskid)) < 0) {
    //     return NULL;
    // }

    if ((hname = putstr_ (hp->h_name)) == NULL) {
        close (sock);
        lserrno = LSE_MALLOC;
        return NULL;
    }

    if (nrh >= maxnrh) {

        for (rh = rHosts->next; rh; rh = rh->next) {
            if (rh->nopen == 0) {
                break;
            }
        }

        if (rh) {

            struct rHosts *lrurh = rh;

            for (rh = rh->next; rh; rh = rh->next)  {
                if (rh->atime < lrurh->atime && rh->nopen == 0) {
                    lrurh = rh;
                }
            }
            if (rhTerminate (lrurh->hname) < 0) {
                close (sock);
                free (hname);
                return NULL;
            }
        }
    }

    if ((rh = allocRH ()) == NULL) {
        free (hname);
        close (sock);
        lserrno = LSE_MALLOC;
        return NULL;
    }

    rh->sock = sock;
    rh->atime = time (NULL);
    rh->hname = hname;
    rh->nopen = 0;

    return rh;
}

struct rHosts *
allocRH (void)
{
    struct rHosts *rh  = NULL;
    struct rHosts *tmp = NULL;

    if ((rh = malloc (sizeof (struct rHosts))) == NULL) {
        return NULL;
    }

    tmp = rHosts;
    rHosts = rh;
    rh->next = tmp;
    nrh++;

    return rh;
}

struct rHosts *
rhFind ( const char *host)
{
    struct rHosts *rh = NULL;

    for( rh = rHosts; rh; rh = rh->next) {
        if (equalHost_ (rh->hname, host)) {
            return rh;
        }
    }

  return NULL;
}


unsigned int
ls_ropen (const char *host, const char *fn, int flags, mode_t mode)
{
    char buf[MSGSIZE]; // FIXME FIXME FIXME FIXME dynamic message allocation
    struct ropenReq req;
    struct LSFHeader hdr;
    struct rHosts *rh = NULL;
    unsigned int fd = 0;


    if ((rh = rhConnect (host)) == NULL) {
        return 255;
    }

    for (fd = 0; fd < maxOpen && ft[fd].host; fd++) { // NOTE maxOpen is global in rf.h
        ;
    } // FIXME EMTPY BODY

    if (fd == maxOpen)
    {
        struct rfTab *tmpft = NULL;

        // assert( maxOpen >= 0 );
        tmpft = realloc( ft, ( maxOpen + NOFILE) *  sizeof (struct rfTab));
        if ( NULL == tmpft ) {
            lserrno = LSE_MALLOC;
            return 255;
        }

        ft = tmpft;
        for ( unsigned int i = maxOpen; i < maxOpen + NOFILE; i++) {
            ft[i].host = NULL;
        }

        maxOpen += NOFILE;
    }

    req.fn = strdup(fn);

    req.flags = flags;
    req.mode = mode;

    if (lsSendMsg_ (rh->sock, RF_OPEN, 0, (char *) &req, buf, sizeof (struct LSFHeader) + MAX_FILENAME_LEN + sizeof (req), xdr_ropenReq, b_write_fix, NULL) < 0)
    {
        return 255;
    }

    if (lsRecvMsg_(rh->sock, buf, sizeof (hdr), &hdr, NULL, NULL, b_read_fix) < 0)
    {
        return 255; 
    }

    if (hdr.opCode == 255 ) // FIXME FIXME FIXME FIXME hdr.opCode must be uint and sett to 255
    {
        // errno = errnoDecode_ (ABS (hdr.opCode));
        errno = (int) errnoDecode_( hdr.opCode ); // NOFIX
        lserrno = LSE_FILE_SYS;
        return 255;
    }

    ft[fd].host = rh;
    ft[fd].fd = (int) hdr.opCode; // FIX FIX FIX opCode is used as a file descriptor? what? (remove cast when done)
    rh->nopen++;

    return fd;
}

int
ls_rclose (int fd)
{
    char buf[MSGSIZE];
    int reqfd = -1;
    struct LSFHeader hdr;
    struct rHosts *rh = NULL;
    long lsSendMsg_Result = 0;

    memset( buf, '\0', MSGSIZE );

    assert( maxOpen <= INT_MAX );
    if (fd < 0 || fd >= (int) maxOpen || ft[fd].host == NULL) {
        lserrno = LSE_BAD_ARGS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    rh = ft[fd].host;
    reqfd = ft[fd].fd;
    lsSendMsg_Result = lsSendMsg_( rh->sock, RF_CLOSE, 0, (char *) &reqfd, buf, sizeof (struct LSFHeader) + sizeof (reqfd), xdr_int, b_write_fix, NULL );

    if ( lsSendMsg_Result < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsRecvMsg_(rh->sock, buf, sizeof (hdr), &hdr, NULL, NULL, b_read_fix) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    ft[fd].host = NULL;
    rh->nopen--;
    if (rh->nopen == 0 && nrh > maxnrh) {
        rhTerminate (rh->hname);
    }

    if (hdr.opCode == 255 ) {
        // errno = errnoDecode_ (ABS (hdr.opCode));
        errno = (int) errnoDecode_ ( hdr.opCode);
        lserrno = LSE_FILE_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return 0;
}


unsigned int
ls_rwrite (int fd, const char *buf, size_t length)
{
    struct {
        struct LSFHeader _;
        struct rrdwrReq __;
    } msgBuf;
    struct rrdwrReq req;
    struct LSFHeader hdr;
    struct rHosts *rh  = NULL;

    assert( maxOpen <= INT_MAX );
    if (fd < 0 || fd >= (int) maxOpen || ft[fd].host == NULL) {
        lserrno = LSE_BAD_ARGS;
        return INT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    rh = ft[fd].host;

    req.fd = ft[fd].fd;
    req.length = length;

    if (lsSendMsg_ (rh->sock, RF_WRITE, 0, (char *) &req, (char *) &msgBuf, sizeof (struct LSFHeader) + sizeof (req),  xdr_rrdwrReq, b_write_fix, NULL) < 0) {
        return INT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    assert( length <= LONG_MAX );
    if (b_read_fix (rh->sock, strdup( buf ), length) != (long) length) {
        lserrno = LSE_MSG_SYS;
        return INT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsRecvMsg_ (rh->sock, (char *) &msgBuf, sizeof (hdr), &hdr, NULL, NULL, b_read_fix) < 0) {
        return INT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

        if (hdr.opCode == 255) {
            // errno = errnoDecode_ (ABS (hdr.opCode));  // FIXME FIXME FIXME FIXME opCode has to be signed, not usigned :(
            errno = (int) errnoDecode_ ( hdr.opCode );  // FIXME FIXME FIXME FIXME opCode has to be signed, not usigned :(
            lserrno = LSE_FILE_SYS;
            return INT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

    assert( hdr.length <= UINT_MAX ); // FIXME FIXME FIXME return type must be corrected
    return (unsigned int) hdr.length;
}


unsigned int
ls_rread (int fd, const char *buf, size_t length)
{
    struct rrdwrReq req;
    struct LSFHeader hdr;
    struct {
        struct LSFHeader _;
        struct rrdwrReq __;
    } msgBuf;
    struct rHosts *rh = NULL;

    if (fd < 0 || fd >= (int) maxOpen || ft[fd].host == NULL) {
        lserrno = LSE_BAD_ARGS;
        return 255;
    }

  rh = ft[fd].host;

  req.fd = ft[fd].fd;
  req.length = length;

    if (lsSendMsg_ (rh->sock, RF_READ, 0, (char *) &req, (char *) &msgBuf, sizeof (struct LSFHeader) + sizeof (req), xdr_rrdwrReq, b_write_fix, NULL) < 0) {
        return 255;
    }

    if (lsRecvMsg_ (rh->sock, (char *) &msgBuf, sizeof (hdr), &hdr, NULL, NULL, b_read_fix) < 0) {
        return 255;
    }

    if (hdr.opCode == 255 )
    {
        // errno = errnoDecode_ (ABS (hdr.opCode));
        errno = (int) errnoDecode_ ( hdr.opCode );
        lserrno = LSE_FILE_SYS;
        return 255;
    }

    assert( hdr.length <= LONG_MAX );
    if ( b_read_fix(rh->sock, strdup( buf ), hdr.length) != (long) hdr.length) {
        lserrno = LSE_MSG_SYS;
        return 255;
    }

    // return (int) hdr.length;
    assert( hdr.length <= UINT_MAX );
    return (u_int) hdr.length;
}


off_t
ls_rlseek (int fd, off_t offset, int whence)
{
    struct {
        struct LSFHeader _;
        struct rlseekReq __;
    } msgBuf;
    struct rlseekReq req;
    struct LSFHeader hdr;
    struct rHosts *rh = NULL;

    if (fd < 0 || fd >= (int) maxOpen || ft[fd].host == NULL) {
        lserrno = LSE_BAD_ARGS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    rh = ft[fd].host;

    req.fd      = ft[fd].fd;
    req.offset  = offset;
    req.whence  = whence;

    if (lsSendMsg_ (rh->sock, RF_LSEEK, 0, (char *) &req, (char *) &msgBuf, sizeof (msgBuf), xdr_rlseekReq, b_write_fix, NULL) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsRecvMsg_ (rh->sock, (char *) &msgBuf, sizeof (hdr), &hdr, NULL, NULL, b_read_fix) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (hdr.opCode == 255 ) {
        // errno = errnoDecode_ (ABS (hdr.opCode));
        errno = (int) errnoDecode_ ( hdr.opCode );
        lserrno = LSE_FILE_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

  // return (off_t) hdr.length;
  return (off_t) hdr.length;
}


int
ls_rfstat (int fd, struct stat *st)
{
    char buf[MSGSIZE];
    int reqfd = 0;
    struct LSFHeader hdr;
    struct rHosts *rh = NULL;

    memset( buf, '\0', strlen( buf ) );

    if (fd < 0 || fd >= (int) maxOpen || ft[fd].host == NULL)
    {
        lserrno = LSE_BAD_ARGS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    rh = ft[fd].host;
    reqfd = ft[fd].fd;

    if (lsSendMsg_ (rh->sock, RF_FSTAT, 0, (char *) &reqfd, buf, sizeof (struct LSFHeader) + sizeof (reqfd), xdr_int, b_write_fix, NULL) < 0) { // FIXME FIXME suspicious cast
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsRecvMsg_ (rh->sock, buf, MSGSIZE, &hdr, (char *) st, xdr_stat, b_read_fix) < 0) { // FIXME FIXME suspicious cast
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (hdr.opCode== 255 ) {
        errno = (int) errnoDecode_ ( hdr.opCode );
        lserrno = LSE_FILE_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return 0;
}


int
ls_rfcontrol (int command, int arg)
{
    switch (command) {
        case RF_CMD_MAXHOSTS: {
            if (arg < 1) {
                lserrno = LSE_BAD_ARGS;
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
            assert( arg > 0);
            maxnrh = (unsigned int) arg;
            return 0;
        }
        break;

        case RF_CMD_RXFLAGS: {
            rxFlags = arg;
            return 0;
        }
        break;
        default: {
            lserrno = LSE_BAD_ARGS;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        break;
    }

    return 0;
}

// int
// ls_rfterminate ( const char *host)
// {
//     return rhTerminate (host);
// }

int
rhTerminate ( const char *host)
{
  struct hostent *hp  = NULL;
  struct rHosts *rh   = NULL;
  struct rHosts *prev = NULL;
  struct LSFHeader buf;
  // int i;

    if ((hp = Gethostbyname_ (host)) == NULL) {
        lserrno = LSE_BAD_HOST;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    for (prev = NULL, rh = rHosts; rh; prev = rh, rh = rh->next) {
        if (equalHost_ (hp->h_name, rh->hname)) {
            if (prev == NULL) {
                rHosts = rh->next;
            }
            else {
                prev->next = rh->next;
            }

            lsSendMsg_ (rh->sock, RF_TERMINATE, 0, NULL, (char *) &buf, sizeof (buf), NULL, b_write_fix, NULL);
            close (rh->sock);

            for ( size_t i = 0; i < maxOpen; i++) {
                if (ft[i].host == rh) {
                    ft[i].host = NULL;
                }
            }

            free (rh->hname);
            free (rh);
            nrh--;
            return 0;
        }
    }

    lserrno = LSE_BAD_HOST;
    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
}


int
ls_rstat (const char *host, const char *fn, struct stat *st)
{
    char buf[MSGSIZE];
    struct LSFHeader hdr;
    struct rHosts *rh = NULL;
    struct stringLen fnStr;

    memset( buf, '\0', strlen( buf ) );

    if ((rh = rhConnect (host)) == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    fnStr.length = MAX_FILENAME_LEN;
    fnStr.name = strdup( fn );
    if (lsSendMsg_ (rh->sock, RF_STAT, 0, (char *) &fnStr, buf, sizeof (struct LSFHeader) + MAX_FILENAME_LEN, xdr_stringLen, b_write_fix, NULL) < 0) { // FIXME FIXME suspicious cast
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsRecvMsg_ (rh->sock, buf, MSGSIZE, &hdr, (char *) st, xdr_stat, b_read_fix) < 0) { // FIXME FIXME suspicious cast
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }


    if (hdr.opCode == 255 ) {
        // errno = errnoDecode_ (ABS (hdr.opCode));
        errno = (int) errnoDecode_ ( hdr.opCode );
        lserrno = LSE_FILE_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return 0;
}

char *
ls_rgetmnthost ( const char *host, const char *fn)
{
    char buf[MSGSIZE];
    char *hostname = malloc( MAXHOSTNAMELEN * sizeof (char) );
    struct LSFHeader hdr;
    struct stringLen fnStr;
    struct stringLen hostStr;
    struct rHosts *rh = NULL;

    memset( buf,      '\0', strlen( buf ) );
    memset( hostname, '\0', strlen( hostname ) );    

    if ((rh = rhConnect (host)) == NULL) {
        return NULL;
    }

    hostStr.length  = MAXHOSTNAMELEN;
    hostStr.name = hostname;
    fnStr.length    = MAX_FILENAME_LEN;
    fnStr.name   = strdup( fn );

    if (lsSendMsg_ (rh->sock, RF_GETMNTHOST, 0, (char *) &fnStr, buf, sizeof (struct LSFHeader) + MAX_FILENAME_LEN, xdr_stringLen, b_write_fix, NULL) < 0) { // FIXME FIXME suspicious cast
        return NULL;
    }

    if (lsRecvMsg_ (rh->sock, buf, MSGSIZE, &hdr, (char *) &hostStr, xdr_stringLen, b_read_fix) < 0) { // FIXME FIXME suspicious cast
        return NULL;
    }

    if (hdr.opCode == 255 ) {
        // errno = errnoDecode_ (ABS (hdr.opCode));
        errno = (int) errnoDecode_( hdr.opCode );
        lserrno = LSE_FILE_SYS;
        return NULL;
    }

    return hostname;
}

/* ls_conntaskport()
 */
int
ls_conntaskport (unsigned long taskid )
{ 
    int sock        = 0;
    int cc          = 0;
    time_t resTimeout = 0;
    socklen_t sinLen = 0;
    struct tid *tid = NULL;
    struct sockaddr_in sin;

    if ( genParams_[RES_TIMEOUT].paramValue) {
        // assert( genParams_[RES_TIMEOUT].paramValue >= 0 ); // paranoia
        resTimeout = atoi (genParams_[RES_TIMEOUT].paramValue);
    }
    else {
        resTimeout = RES_TIMEOUT;
    }

    if ((tid = tid_find (taskid)) == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    sinLen = sizeof (sin);
    if (getpeername (tid->sock, (struct sockaddr *) &sin, &sinLen) < 0) {
        lserrno = LSE_SOCK_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if ((sock = (int) CreateSock_ (SOCK_STREAM)) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    sin.sin_port = tid->taskPort;

    cc = b_connect_ (sock, (struct sockaddr *) &sin, sizeof (sin), resTimeout);
    if (cc < 0) {
        close (sock);
        lserrno = LSE_CONN_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return sock;
}               /* ls_conntaskport()  */


int
ls_runlink (const char *host, const char *fn)
{
    char buf[MSGSIZE];
    char hostname[MAXHOSTNAMELEN];
    struct rHosts *rh = NULL;
    struct LSFHeader hdr;
    struct stringLen hostStr;
    struct stringLen fnStr;

    memset( buf,      '\0', MSGSIZE );
    memset( hostname, '\0', MAXHOSTNAMELEN );    

    if ((rh = rhConnect (host)) == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    hostStr.length  = MAXHOSTNAMELEN;
    hostStr.name = hostname;
    fnStr.length    = MAX_FILENAME_LEN;
    fnStr.name   = strdup( fn );

    if (lsSendMsg_ (rh->sock, RF_UNLINK, 0, (char *) &fnStr, buf, sizeof (struct LSFHeader) + MAX_FILENAME_LEN, xdr_stringLen, b_write_fix, NULL) < 0) { // FIXME FIXME suspicious cast
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsRecvMsg_ (rh->sock, buf, MSGSIZE, &hdr, (char *) &hostStr, xdr_stringLen, b_read_fix) < 0) // FIXME FIXME suspicious cast
    {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (hdr.opCode == 255 ) {
        // errno = errnoDecode_ (ABS (hdr.opCode));
        errno = (int) errnoDecode_(  hdr.opCode );
        lserrno = LSE_FILE_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return 0;
}
