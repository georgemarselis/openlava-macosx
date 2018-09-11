/* $Id: lib.channel.c 397 2007-11-26 19:04:00Z mblack $
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
#include <fcntl.h>
#include <syslog.h>
#include <sys/time.h> /* struct timeval */
#include <sys/socket.h> 

// #include "daemons/libresd/resd.h"
#include "lib/channel.h"
#include "lib/host.h"
// #include "lib/lib.h"
#include "lib/lproto.h"
// #include "lib/osal.h"
#include "lib/rdwr.h"
#include "lib/rwait.h"
#include "lib/sock.h"
#include "lib/xdr.h"

/**************************************************************
 * this file must be compiled together with host.c cuz there is
 * sockAdd2Str_() to resolve
 *
 */

/* FIXME FIXME
the following constant is not used in this file. 
it looks like it is uesd in the _i18n_msg_get() function 
See if it needs moving to the libint header file. 
*/

// FIXME FIXME FINALLY FOUND where it is used: libint/lsi18n.h:847
// #define NL_SETN 23

/*#define CLOSEIT(i) { \
close(channels[i].handle); \
channels[i].state = CH_DISC; \
channels[i].handle = INVALID_HANDLE; }
*/

// extern int errno;
// extern int CreateSock_ (int);


static unsigned int NET_INTSIZE_  = 4;

int
chanInit_ (void)
{
    static char first = TRUE;
    long result = 0;

    if (!first) {
        return 0;
    }

    first = FALSE;

    if( ( result = sysconf (_SC_OPEN_MAX)) >= 0) {
        chanMaxSize = (size_t)result;
    }
    else if ( -1 == result && errno == EINVAL ) {
        syslog( LOG_ERR, "%s: sysconf value passed was invalid", __func__ );
        return -1;
    }
    else {
        fprintf( stderr, "%s: you are not supposed to be here\n", __func__ );
    }

    channels = calloc ( chanMaxSize, sizeof (struct chanData)); // channels is global, located in <channel.h>
    if (channels == NULL) {
        return -1;
    }

    chanIndex = 0; // chanIndex is global, located in <channel.h>

    return 0;
}

int
chanServSocket_ (int type, unsigned short port, int backlog, int options)
{
    int ch = 0;
    int s  = 0;
    struct sockaddr_in sin;

    if ((ch = findAFreeChannel ()) < 0)
    {
        lserrno = LSE_NO_CHAN;
        return -1;
    }

    s = socket (AF_INET, type, 0);

    if (SOCK_INVALID (s))
    {
        lserrno = LSE_SOCK_SYS;
        return -1;
    }

    memset ((char *) &sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons (port);
    sin.sin_addr.s_addr = htonl (INADDR_ANY);

    if (options & CHAN_OP_SOREUSE)
    {
        int one = 1;

        setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (int));
    }

    if (bind (s, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
        (void) close (s);
        lserrno = LSE_SOCK_SYS;
        return -2;
    }

    if (backlog > 0)
    {
        if (listen (s, backlog) < 0)
        {
            (void) close (s);
            lserrno = LSE_SOCK_SYS;
            return -3;
        }
    }

    assert( s >= 0 );
    channels[ch].handle = (unsigned int)s;
    channels[ch].state = CH_WAIT;
    if (type == SOCK_DGRAM) {
        channels[ch].type = CH_TYPE_UDP;
    }
    else {
        channels[ch].type = CH_TYPE_PASSIVE;
    }

    return ch;
}

int
chanClientSocket_ (int domain, int type, int options)
{
    int ch = 0;
    int s0 = 0;
    int s1 = 0;
    unsigned int counter = 0;
    static char first  = TRUE;
    static unsigned short port = 0;
    struct sockaddr_in cliaddr;

    if (domain != AF_INET)
    {
        lserrno = LSE_INTERNAL;
        return -1;
    }

    if ((ch = findAFreeChannel ()) < 0)
    {
        lserrno = LSE_NO_CHAN;
        return -1;
    }

    if (type == SOCK_STREAM)
        channels[ch].type = CH_TYPE_TCP;
    else
        channels[ch].type = CH_TYPE_UDP;

    s0 = socket (domain, type, 0);

    if (SOCK_INVALID (s0))
    {
        lserrno = LSE_SOCK_SYS;
        return -1;
    }

    channels[ch].state = CH_DISC;
    assert( s0 >= 0 );
    channels[ch].handle = (unsigned int) s0;
    if (s0 < 3)
    {
        s1 = get_nonstd_desc_ (s0);
        if (s1 < 0){
            close (s0);
        }
        assert( s1 >= 0 );
        channels[ch].handle = (unsigned int)s1;
    }

    if (options & CHAN_OP_PPORT)
    {
        if (first)
        {
            first = FALSE;
            port = IPPORT_RESERVED - 1;
        }
        if (port < IPPORT_RESERVED / 2)
        {
            port = IPPORT_RESERVED - 1;
        }
    }

    assert( channels[ch].handle <= INT_MAX );
    s0 = (int)channels[ch].handle;
    memset ((char *) &cliaddr, 0, sizeof (cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = htonl (INADDR_ANY);

    for ( unsigned int i = 0; i < IPPORT_RESERVED / 2; i++)
    {

        if (options & CHAN_OP_PPORT)
        {
            cliaddr.sin_port = htons (port);
            port--;
        }
        else
            cliaddr.sin_port = htons (0);

        if (bind (s0, (struct sockaddr *) &cliaddr, sizeof (cliaddr)) == 0)
            break;


        if (!(options & CHAN_OP_PPORT))
        {

            if (errno == EADDRINUSE)
            {
                port = (unsigned short) (time (0) | getpid ());
                port = (unsigned short) ( port < 1024 ? (port + 1024) : port );
                cliaddr.sin_port = htons (port);
                if (bind (s0, (struct sockaddr *) &cliaddr, sizeof (cliaddr)) ==
                    0)
                    break;
            }

            assert( ch >= 0 );
            chanClose_ (ch);
            lserrno = LSE_SOCK_SYS;
            return -1;
        }
        if (errno != EADDRINUSE && errno != EADDRNOTAVAIL)
        {
            assert( ch >= 0 );
            chanClose_ (ch);
            lserrno = LSE_SOCK_SYS;
            return -1;
        }


        if ((options & CHAN_OP_PPORT) && port < IPPORT_RESERVED / 2) {
            port = IPPORT_RESERVED - 1;
        }

        counter = i;
    }

    if ((options & CHAN_OP_PPORT) && counter == IPPORT_RESERVED / 2)
    {
        assert( ch <= INT_MAX );
        chanClose_ (ch);
        lserrno = LSE_SOCK_SYS;
        return -1;
    }

# if defined(FD_CLOEXEC)
    fcntl (s0, F_SETFD, (fcntl (s0, F_GETFD) | FD_CLOEXEC));
# else
#  if defined(FIOCLEX) 
    ioctl (s0, FIOCLEX, (char *) NULL);
#  endif
# endif

    return ch;
}

int
chanAccept_ (int chfd, struct sockaddr_in *from)
{
    int s = 0;
    socklen_t len;

    if (channels[chfd].type != CH_TYPE_PASSIVE)
    {
        lserrno = LSE_INTERNAL;
        return -1;
    }

    len = sizeof (struct sockaddr);
    assert( channels[chfd].handle <= INT_MAX );
    s = accept ((int)channels[chfd].handle, (struct sockaddr *) from, &len);
    if (SOCK_INVALID (s))
    {
        lserrno = LSE_SOCK_SYS;
        return -1;
    }

    return chanOpenSock_ (s, CHAN_OP_NONBLOCK);

}

void
chanInactivate_ (unsigned int chfd)
{
    if ( chfd > chanMaxSize) {
        return;
    }

    if (channels[chfd].state != CH_INACTIVE)
    {
        channels[chfd].prestate = channels[chfd].state;
        channels[chfd].state = CH_INACTIVE;
    }
}

void
chanActivate_ (unsigned int chfd)
{
    if ( chfd > chanMaxSize) {
        return;
    }

    if (channels[chfd].state == CH_INACTIVE)
    {
        channels[chfd].state = channels[chfd].prestate;
    }
}

int
chanConnect_ (int chfd, struct sockaddr_in *peer, int timeout)
{
    int cc = 0;

    if (channels[chfd].state != CH_DISC)
    {
        lserrno = LSE_INTERNAL;
        return -1;
    }

    if (logclass & (LC_COMM | LC_TRACE)) {
        ls_syslog (LOG_DEBUG2, "%s: Connecting chan=%d to peer %s timeout %d", __func__, chfd, sockAdd2Str_ (peer), timeout);
    }

    if (channels[chfd].type == CH_TYPE_UDP)
    {
        assert( channels[chfd].handle <= INT_MAX );
        cc = connect ((int)channels[chfd].handle, (struct sockaddr *) peer, sizeof (struct sockaddr_in));
        if (SOCK_CALL_FAIL (cc))
        {
            lserrno = LSE_CONN_SYS;
            return -1;
        }
        channels[chfd].state = CH_CONN;
        return 0;
    }

    if (timeout >= 0)
    {
        assert( channels[chfd].handle <= INT_MAX );
        if (b_connect_ ((int)channels[chfd].handle, (struct sockaddr *) peer, sizeof (struct sockaddr_in), (unsigned int)timeout / 1000) < 0)
        {
            if (errno == ETIMEDOUT) {
                lserrno = LSE_TIME_OUT;
            }
            else {
                lserrno = LSE_CONN_SYS;
            }
            return -1;
        }
        channels[chfd].state = CH_CONN;
        return 0;
    }
    channels[chfd].state = CH_CONN;
    return 0;

}

int
chanSendDgram_ (int chfd, char *buf, int len, struct sockaddr_in *peer)
{
    int s = 0;
    ssize_t cc = 0;

    assert( channels[chfd].handle <= INT_MAX );
    s = (int) channels[chfd].handle;


    if (logclass & (LC_COMM | LC_TRACE)) {
        ls_syslog (LOG_DEBUG3, "chanSendDgram_: Sending message size=%d peer=%s chan=%d", len, sockAdd2Str_ (peer), chfd);
    }

    if (channels[chfd].type != CH_TYPE_UDP)
    {
        lserrno = LSE_INTERNAL;
        return -1;
    }

    if( len < 0 ) {
        lserrno = LSE_INTERNAL;
        return -1;
    }

    if (channels[chfd].state == CH_CONN)
        cc = send (s, buf, (size_t) len, 0);
    else if ( len >= 0 )
    {
        /* FIXME FIXME 
            the conversion of len from int to size_t here is problematic. on error len == -1,
            but the check above should be sufficient.
        */
        cc = sendto (s, buf, (size_t) len, 0, (struct sockaddr *) peer, sizeof (struct sockaddr_in) );

    }

    if (SOCK_CALL_FAIL (cc))
    {
        lserrno = LSE_MSG_SYS;
        return -1;
    }

    return 0;
}

/* chanRcvDgram_()
*/
int
chanRcvDgram_ (int chfd, char *buf, int len, struct sockaddr_in *peer, int timeout)
{
    int sock = 0;
    struct timeval timeval;
    struct timeval *timep = NULL;
    int nReady = 0;
    long cc = 0;
    socklen_t peersize;

    peersize = sizeof (struct sockaddr_in);
    assert( channels[chfd].handle <= INT_MAX );
    sock = (int)channels[chfd].handle;

    if (channels[chfd].type != CH_TYPE_UDP)
    {
        lserrno = LSE_INTERNAL;
        return -1;
    }

    if (logclass & (LC_COMM | LC_TRACE)){
        ls_syslog (LOG_DEBUG2, "chanRcvDgram_: Receive on chan %d timeout=%d", chfd, timeout);
    }

    if (timeout < 0)
    {
        if ( channels[chfd].state == CH_CONN) 
        {
            if( len < 0 ) {
                lserrno = LSE_INTERNAL;
                return -1;
            }

            cc = recv (sock, buf, (size_t) len, 0);

        }
        else {
            if( len < 0 ) {
                lserrno = LSE_INTERNAL;
                return -1;
            }

            cc = recvfrom (sock, buf, (size_t) len, 0, (struct sockaddr *) peer, &peersize);
        }
        if (SOCK_CALL_FAIL (cc))
        {
            lserrno = LSE_MSG_SYS;
            return -1;
        }
        return 0;
    }

    if (timeout > 0)
    {

        /* FIXME FIXME      the following two statements make no sense right now
            should be put under the debugger and figure out if 
            timeval.tv_usec resorts to anything other than zero

            FIXME FIXME the cast has also to go, as suspect for overflow
        */

        timeval.tv_sec = timeout / 1000;
        timeval.tv_usec = (int) (timeout - timeval.tv_sec * 1000);
        /* timeval.tv_usec *= 1000 maybe here? */
        timep = &timeval;
    }

    for (;;) /* FIXME FIXME FIXME all infinite loops must go */
    {

        nReady = rd_select_ (sock, timep);
        if (nReady < 0)
        {
            lserrno = LSE_SELECT_SYS;
            return -1;
        }
        if (nReady == 0)
        {
            lserrno = LSE_TIME_OUT;
            return -1;
        }
        if (channels[chfd].state == CH_CONN) {

            if( len < 0 ) { /* FIXME FIXME all these checks should be turned into n assert */
                lserrno = LSE_INTERNAL;
                return -1;
            }
            cc = recv (sock, buf, (size_t) len, 0);
        }
        else {
            if( len < 0 ) {
                lserrno = LSE_INTERNAL;
                return -1;
            }
            cc = recvfrom (sock, buf, (size_t) len, 0, (struct sockaddr *) peer, &peersize);
        }

        if (SOCK_CALL_FAIL (cc))
        {
            if (channels[chfd].state == CH_CONN && (errno == ECONNREFUSED)) {
                lserrno = LSE_LIM_DOWN;
            }
            else {
                lserrno = LSE_MSG_SYS;
            }

            return -1;
        }

        return 0;

    }
}               /* chanRcvDgram_() */

/* chanOpen_()
*/
int
chanOpen_ (unsigned int iaddr, unsigned short port, int options)
{
    int i      = 0;
    int cc     = 0;
    int oldOpt = 0;
    int newOpt = 0;
    int returnValue = 0;
    struct sockaddr_in addr;

    if ((i = findAFreeChannel ()) < 0)
    {
        chanerr = CHANE_NOCHAN;
        return -1;
    }

    channels[i].type = CH_TYPE_TCP;

    memset ((char *) &addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    memcpy ((char *) &addr.sin_addr, (char *) &iaddr, sizeof (unsigned int));
    addr.sin_port = port;

    newOpt = 0;
    if (options & CHAN_OP_PPORT)
    {
        newOpt |= LS_CSO_PRIVILEGE_PORT;
    }

    oldOpt = setLSFChanSockOpt_ (newOpt | LS_CSO_ASYNC_NT);
    returnValue = CreateSock_ (SOCK_STREAM);
    assert( returnValue >= 0 );
    channels[i].handle = (unsigned int) returnValue;
    setLSFChanSockOpt_ (oldOpt);
    if (returnValue < 0)
    {
        chanerr = CHANE_SYSCALL;
        return -1;
    }

    if (io_nonblock_ ((int)channels[i].handle) < 0)
    {
        close((int)channels[i].handle);
        channels[i].state = CH_DISC;
        channels[i].handle = INVALID_HANDLE;
        chanerr = CHANE_SYSCALL;
        return -1;
    }

    // channels[i].handle is unsigned int
    cc = connect( (int)channels[i].handle, (struct sockaddr *) &addr, sizeof (addr)); // FIXME cast here is correct
    if (cc < 0)
    {

        if (errno != EINPROGRESS)
        {
            struct sockaddr laddr;
            socklen_t len;

            /*catgets 5002 */
            ls_syslog (LOG_ERR, "catgets 5002: %s: connect() failed, cc=%d, errno=%d", __func__, cc, errno);

            len = sizeof (laddr);
            if (getsockname ((int)channels[i].handle, &laddr, &len) == 0){
                /*catgets 5003 */
                // FIXME how to fix cast error: https://gitorious.org/libvirt/libvirt/commit/82e4f85ce0ee65f90674ef4129c3bd4c43ca00c9
                ls_syslog (LOG_ERR, "catgets 5003: %s: connect() failed, laddr=%s, addr=%s", __func__, sockAdd2Str_ ((struct sockaddr_in *) &laddr), sockAdd2Str_ ((struct sockaddr_in *) &addr)); 
//                ls_syslog (LOG_ERR, I18N (5003, "chanOpen: connect() failed, laddr=%s, addr=%s"), sockAdd2Str_ ((struct sockaddr_in *) &laddr), sockAdd2Str_ ((struct sockaddr_in *) &addr)); 
            }

            close((int)channels[i].handle);
            channels[i].state = CH_DISC;
            channels[i].handle = INVALID_HANDLE;
            chanerr = CHANE_SYSCALL;
            return -1;
        }
        channels[i].state = CH_PRECONN;
        return i;
    }

    channels[i].state = CH_CONN;
    channels[i].send = newBuf ();
    channels[i].recv = newBuf ();

    if (!channels[i].send || !channels[i].recv)
    {
        close((int)channels[i].handle);
        channels[i].state = CH_DISC;
        channels[i].handle = INVALID_HANDLE;
        if ( channels[i].send != NULL) {
            free( channels[i].send );
            channels[i].send = NULL;
        }
        if ( channels[i].recv != NULL) {
            free( channels[i].recv );
            channels[i].recv = NULL;
        }
        lserrno = LSE_MALLOC;
        return -1;
    }

    return i;

}   /* chanOpen_() */


int
chanOpenSock_ (int s, int options)
{
    int i;

    if ((i = findAFreeChannel ()) < 0)
    {
        lserrno = LSE_NO_CHAN;
        return -1;
    }

    if ((options & CHAN_OP_NONBLOCK) && (io_nonblock_ (s) < 0))
    {
        lserrno = LSE_SOCK_SYS;
        return -1;
    }
    channels[i].type = CH_TYPE_TCP;
    channels[i].handle = (unsigned int)s;
    channels[i].state = CH_CONN;

    if (options & CHAN_OP_RAW) {
        return i;
    }

    channels[i].send = newBuf ();
    channels[i].recv = newBuf ();
    if (!channels[i].send || !channels[i].recv)
    {
        close((int)channels[i].handle);
        channels[i].state = CH_DISC;
        channels[i].handle = INVALID_HANDLE;
        if ( channels[i].send != NULL) {
            free( channels[i].send );
            channels[i].send = NULL;
        }
        if ( channels[i].recv != NULL) {
            free( channels[i].recv );
            channels[i].recv = NULL;
        }
        lserrno = LSE_MALLOC;
        return -1;
    }
    return i;
}

int
chanClose_ (int chfd)
{
    struct Buffer *buf     = NULL;
    struct Buffer *nextbuf = NULL;
    long maxfds   = 0;

    maxfds = sysconf (_SC_OPEN_MAX);

    if (chfd > maxfds)
    {
        lserrno = LSE_INTERNAL;
        return -1;
    }

/*    if (channels[chfd].handle < 0)
    {
        chanerr = CHANE_BADCHFD;
        return -1;
    }*/
    close ((int)channels[chfd].handle);

    if (channels[chfd].send && channels[chfd].send != channels[chfd].send->forw) {

        for (buf = channels[chfd].send->forw; buf != channels[chfd].send; buf = nextbuf) {
            nextbuf = buf->forw;
            if ( buf->data != NULL) {
                memset( buf->data, '\0', strlen( buf->data ) );
                free( buf->data );
                buf->data = NULL;
            }
            if ( buf != NULL) {
                free( buf );
                buf = NULL;
            }
        }
    }
    if (channels[chfd].recv && channels[chfd].recv != channels[chfd].recv->forw)
    {
        for (buf = channels[chfd].recv->forw;
            buf != channels[chfd].recv; buf = nextbuf)
        {
            nextbuf = buf->forw;
            if ( buf->data != NULL) {
                memset( buf->data, '\0', strlen( buf->data ) );
                free( buf->data );
                buf->data = NULL;
            }
            if ( buf != NULL) {
                free( buf );
                buf = NULL;
            }
        }
    }

    if ( channels[chfd].recv != NULL) {
        free( channels[chfd].recv );
        channels[chfd].recv = NULL;
    }

    if ( channels[chfd].send != NULL) {
        free( channels[chfd].send );
        channels[chfd].send = NULL;
    }
    channels[chfd].state = CH_FREE;
    channels[chfd].handle = INVALID_HANDLE;
    // channels[chfd].send = NULL;
    // channels[chfd].recv = NULL;
    return 0;
}

void
chanCloseAll_ (void)
{
    for ( unsigned long i = 0; i < chanIndex; i++) {
        if (channels[i].state != CH_FREE) {
            chanClose_ ((int)i); // FIXME FIXME FIXME FIXME FIXME FIXME does chanClose expect negative, and if yes, why?
        }
    }

}

void
chanCloseAllBut_ (int chfd)
{
    for (unsigned int i = 0; i < chanIndex; i++) {
        if ((channels[i].state != CH_FREE) && (i != (unsigned int)chfd)) {
            chanClose_ ((int)i); // FIXME FIXME FIXME FIXME FIXME FIXME does chanClose expect negative, and if yes, why?
        }
    }
}

int
chanSelect_ (struct Masks *sockmask, struct Masks *chanmask, struct timeval *timeout)
{
    int nReady = 0;
    int maxfds = 0;

    FD_ZERO (&sockmask->wmask);
    FD_ZERO (&sockmask->emask);

    for ( unsigned int i = 0; i < chanIndex; i++) // FIXME FIXME FIXME FIXME FIXME FIXME what is chanIndex and where is it defined?
    {
        if (channels[i].state == CH_INACTIVE) {
            continue;
        }

        if (channels[i].handle == INVALID_HANDLE) {
            continue;
        }
        if (channels[i].state == CH_FREE)
        {
            ls_syslog (LOG_ERR, "%s: channel %d has socket %d but in %s state", __func__, i, channels[i].handle, "CH_FREE");
            continue;
        }

        if (channels[i].type == CH_TYPE_UDP && channels[i].state != CH_WAIT)
            continue;

        if (logclass & LC_COMM) {
            ls_syslog (LOG_DEBUG3, "%s: Considering channel %d handle %d state %d type %d", __func__, i, channels[i].handle, (int) channels[i].state, (int) channels[i].type);
        }

        if (channels[i].type == CH_TYPE_TCP && channels[i].state != CH_PRECONN && !channels[i].recv && !channels[i].send) {
            continue;
        }

        if (channels[i].state == CH_PRECONN)
        {
            FD_SET (channels[i].handle, &(sockmask->wmask));
            continue;
        }

        if (logclass & LC_COMM) {
            ls_syslog (LOG_DEBUG3, "%s: Adding channel %d handle %d ", __func__, i, channels[i].handle);
        }
        FD_SET (channels[i].handle, &(sockmask->rmask));

        if (channels[i].type != CH_TYPE_UDP) {
            FD_SET (channels[i].handle, &(sockmask->emask));
        }

        if (channels[i].send && channels[i].send->forw != channels[i].send) {
            FD_SET (channels[i].handle, &(sockmask->wmask));
        }
    }

    maxfds = FD_SETSIZE;

    nReady = select (maxfds, &(sockmask->rmask), &(sockmask->wmask), &(sockmask->emask), timeout);
    if (nReady <= 0) {
        return nReady;
    }

    FD_ZERO (&(chanmask->rmask));
    FD_ZERO (&(chanmask->wmask));
    FD_ZERO (&(chanmask->emask));

    for ( unsigned int i = 0; i < chanIndex; i++)
    {

        if (channels[i].handle == INVALID_HANDLE) {
            continue;
        }

        if (FD_ISSET (channels[i].handle, &(sockmask->emask)))
        {
            ls_syslog (LOG_DEBUG, "%s: setting error mask for channel %d", __func__, channels[i].handle);
            FD_SET (i, &(chanmask->emask));
            continue;
        }

        if ((!channels[i].send || !channels[i].recv) && (channels[i].state != CH_PRECONN))
        {
            if (FD_ISSET (channels[i].handle, &(sockmask->rmask))) {
                FD_SET (i, &(chanmask->rmask));
            }
            if (FD_ISSET (channels[i].handle, &(sockmask->wmask))) {
                FD_SET (i, &(chanmask->wmask));
            }
 
            continue;
        }


        if (channels[i].state == CH_PRECONN)
        {

            if (FD_ISSET (channels[i].handle, &(sockmask->wmask)))
            {
                channels[i].state = CH_CONN;
                channels[i].send = newBuf ();
                channels[i].recv = newBuf ();
                FD_SET (i, &(chanmask->wmask));
            }

        }
        else
        {

            if (FD_ISSET (channels[i].handle, &(sockmask->rmask)))
            {
                doread ((int)i, chanmask);
                if (!FD_ISSET (i, &(chanmask->rmask))
                    && !FD_ISSET (i, &(chanmask->emask)))
                    nReady--;
            }

            if ((channels[i].send->forw != channels[i].send) && FD_ISSET (channels[i].handle, &(sockmask->wmask)))
            {
                dowrite ((int)i, chanmask);
            }
            FD_SET (i, &(chanmask->wmask));
        }

        FD_CLR (channels[i].handle, &(sockmask->rmask));
        FD_CLR (channels[i].handle, &(sockmask->wmask));
        FD_CLR (channels[i].handle, &(sockmask->emask));
    }

    return nReady;
}

int
chanEnqueue_ (int chfd, struct Buffer *msg)
{
    long maxfds;

    maxfds = sysconf (_SC_OPEN_MAX);
    maxfds = sysconf (_SC_OPEN_MAX);

    if (chfd < 0 || chfd > maxfds)
    {
        chanerr = CHANE_BADCHAN;
        return -1;
    }

    if (channels[chfd].handle == INVALID_HANDLE || channels[chfd].state == CH_PRECONN)
    {
        chanerr = CHANE_NOTCONN;
        return -1;
    }

    enqueueTail_ (msg, channels[chfd].send);
    return 0;
}

int
chanDequeue_ (int chfd, struct Buffer **buf)
{
    long maxfds;

    maxfds = sysconf (_SC_OPEN_MAX);

    if (chfd < 0 || chfd > maxfds)
    {
        chanerr = CHANE_BADCHAN;
        return -1;
    }
    if (channels[chfd].handle == INVALID_HANDLE || channels[chfd].state == CH_PRECONN)
    {
        chanerr = CHANE_NOTCONN;
        return -1;
    }

    if (channels[chfd].recv->forw == channels[chfd].recv)
    {
        chanerr = CHANE_NOMSG;
        return -1;
    }
    *buf = channels[chfd].recv->forw;
    dequeue_ (channels[chfd].recv->forw);
    return 0;
}

long
chanReadNonBlock_ (int chfd, char *buf, size_t len, int timeout)
{
    if (io_nonblock_ ((int)channels[chfd].handle) < 0)
    {
        lserrno = LSE_FILE_SYS;
        ls_syslog (LOG_ERR, "%s: %s failed, %m", __func__, "io_nonblock_", 2); // FIXME the 2 is from a leftover from the macro expansion
        return -1;
    }

    return nb_read_timeout( (int)channels[chfd].handle, buf, len, timeout);
}

long 
chanRead_ (int chfd, char *buf, size_t len)
{
    return b_read_fix ((int)channels[chfd].handle, buf, len);
}
    

// FIXME function should return size_t. after fixing, remove cast
long
chanWrite_ (int chfd, char *buf, size_t len)
{
    return (long) b_write_fix ((int)channels[chfd].handle, buf, len) ; // FIXME FIXME FIXME function should return int. after fixing, remove cast 
}

int
chanRpc_ (int chfd, struct Buffer *in, struct Buffer *out, struct LSFHeader *outhdr, int timeout)
{
    
    long cc;
    
    XDR xdrs;
    
    struct LSFHeader hdrBuf;
    struct timeval timeval, *timep = NULL;

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG1, "%s: Entering ... chfd=%d", __func__, chfd);
    }

    if (in)
    {
        if (logclass & LC_COMM) {
            ls_syslog (LOG_DEBUG1, "%s: sending %d bytes", __func__, in->len);
        }

        // FIXME fix chanWrite to return size_t, take out cast
        if (chanWrite_ (chfd, in->data, in->len) != (long) in->len) {
            return -1;
        }

        if (in->forw != NULL)
        {
            struct Buffer *buf = in->forw;
            size_t nlen = ntohl((unsigned int)buf->len);

            if (logclass & LC_COMM) {
                ls_syslog (LOG_DEBUG1, "%s: sending %d extra bytes", __func__, nlen);
            }
                        // #define NET_INTADDR_(a) ((char *) (a))
            // if (chanWrite_ (chfd, NET_INTADDR_(&nlen), NET_INTSIZE_) != NET_INTSIZE_) {
            if    (chanWrite_ (chfd, ((char *) (&nlen)), NET_INTSIZE_) != NET_INTSIZE_) {
                return -1;
            }

            // FIXME fix chanWrite to return size_t, take out cast
            if (chanWrite_ (chfd, buf->data, buf->len) != (long) buf->len) {
                return -1;
            }
        }
    }

    if (!out) {
        return 0;
    }

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG2, "%s: waiting for reply timeout=%d ms", __func__, timeout);
    }
    if (timeout > 0)
    {
        
        /* FIXME FIXME      the following two statements make no sense right now
                should be put under the debugger and figure out if 
                timeval.tv_usec resorts to anything other than zero
        */

        // FIXME FIME the cast has to go, as is suspect for bad arithmetic 

        timeval.tv_sec = timeout / 1000;
        timeval.tv_usec = (timeout - timeval.tv_sec * 1000);
        /* timeval.tv_usec *= 1000 maybe here? */
        timep = &timeval;
    }

    if ((cc = rd_select_ (channels[chfd].handle, timep)) <= 0)
    {
        if (cc == 0) {
            lserrno = LSE_TIME_OUT;
        }
        else {
            lserrno = LSE_SELECT_SYS;
        }
        return -1;
    }

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG2, "%s: reading reply header", __func__);
    }

    xdrmem_create (&xdrs, (char *) &hdrBuf, sizeof (struct LSFHeader), XDR_DECODE);      // FIXME FIXME FIXME FIXME FIXME (char *) &hdrBuf ; does the char need to be there?
    cc = readDecodeHdr_( chfd, (char *) &hdrBuf, (ssize_t (*)()) chanRead_, &xdrs, outhdr); // FIXME FIXME FIXME FIXME FIXME (char *) &hdrBuf ; does the char need to be there?
    
    if (cc < 0)
    {
        xdr_destroy (&xdrs);
        return -1;
    }
    xdr_destroy (&xdrs);

    #define MAXMSGLEN     (1<<28)
    if (outhdr->length > MAXMSGLEN)
    {
        lserrno = LSE_PROTOCOL;
        return -1;
    }

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG2, "%s: reading reply size=%d", __func__, outhdr->length);
    }
    out->len = outhdr->length;

    // FIXME out->len is always greater than zero. 
    //  confirm and take out.
    if (out->len > 0)
    {
        long chanReadout = 0 ;
        if ((out->data = malloc( (size_t) out->len)) == NULL)
        {
            lserrno = LSE_MALLOC;
            return -1;
        }

        chanReadout = chanRead_(chfd, out->data, out->len );
        if ( chanReadout != (long) out->len)
        {
            FREEUP (out->data);
            if (logclass & LC_COMM) {
                ls_syslog (LOG_DEBUG2, "%s: read only %d bytes", __func__, cc);
            }

            lserrno = LSE_MSG_SYS;
            return -1;
        }
    }
    else {
        out->data = NULL;
    }

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG1, "%s: Leaving...repy_size=%d", __func__, out->len);
    }

    return 0;
}

unsigned long chanSock_ (unsigned int chfd)
{
    if ( chfd > chanMaxSize)
    {
        lserrno = LSE_BAD_CHAN;
        return 0;
    }

    return channels[chfd].handle;
}

int
chanSetMode_ (unsigned int chfd, int mode)
{
    if ( chfd > chanMaxSize)
    {
        lserrno = LSE_BAD_CHAN;
        return -1;
    }

    if (channels[chfd].state == CH_FREE
        || channels[chfd].handle == INVALID_HANDLE)
    {
        lserrno = LSE_BAD_CHAN;
        return -1;
    }

    if (mode == CHAN_MODE_NONBLOCK)
    {

        if (io_nonblock_ ((int)channels[chfd].handle) < 0)
        {
            lserrno = LSE_SOCK_SYS;
            return -1;
        }
        if (!channels[chfd].send) {
            channels[chfd].send = newBuf ();
        }
        if (!channels[chfd].recv) {
            channels[chfd].recv = newBuf ();
        }
        if (!channels[chfd].send || !channels[chfd].recv)
        {
            lserrno = LSE_MALLOC;
            return -1;
        }

        return 0;
    }

    if (io_block_ ((int)channels[chfd].handle) < 0)
    {
        lserrno = LSE_SOCK_SYS;
        return -1;
    }

    return 0;
}

void
doread (int chfd, struct Masks *chanmask)
{
    long cc;
    struct Buffer *rcvbuf;

    if (channels[chfd].recv->forw == channels[chfd].recv)
    {
        rcvbuf = newBuf ();
        if (!rcvbuf)
        {
            FD_SET (chfd, &(chanmask->emask));
            channels[chfd].chanerr = LSE_MALLOC;
            return;
        }
        enqueueTail_ (rcvbuf, channels[chfd].recv);
    }
    else {
        rcvbuf = channels[chfd].recv->forw;
    }

    if (!rcvbuf->len)
    {
        rcvbuf->data = malloc (LSF_HEADER_LEN);
        if (!rcvbuf->data)
        {
            FD_SET (chfd, &(chanmask->emask));
            channels[chfd].chanerr = LSE_MALLOC;
            return;
        }
        rcvbuf->len = LSF_HEADER_LEN;
        rcvbuf->pos = 0;
    }

    // FIXME cast is ok, rcvbuf->len is always possitive or zero
    if ( rcvbuf->pos == (long) rcvbuf->len) {
        FD_SET (chfd, &(chanmask->rmask));
        return;
    }

    errno = 0;

    // just in case
    assert( rcvbuf->pos >= 0 );
    assert( rcvbuf->len - (size_t) rcvbuf->pos > 0 );
    assert( channels[chfd].handle <= INT_MAX );
    cc = read ( (int)channels[chfd].handle, rcvbuf->data + rcvbuf->pos, (rcvbuf->len - (size_t) rcvbuf->pos));
    if (cc == 0 && errno == EINTR) {
        ls_syslog (LOG_ERR, "%s: looks like read() has returned EOF when interrupted by a signal", __func__);
        return;
    }

    if (cc <= 0)
    {
        if (cc == 0 || BAD_IO_ERR (errno))
        {
            FD_SET (chfd, &(chanmask->emask));
            channels[chfd].chanerr = CHANE_CONNRESET;
        }
        return;
    }

    rcvbuf->pos += cc;

    // FIXME cast is ok, rcvbuf->len is always possitive or zero
    if ((rcvbuf->len == LSF_HEADER_LEN) && (rcvbuf->pos == (long) rcvbuf->len))
    {
        XDR xdrs;
        struct LSFHeader hdr;
        char *newdata;

        xdrmem_create (&xdrs,
            rcvbuf->data, sizeof (struct LSFHeader), XDR_DECODE);
        if (!xdr_LSFHeader (&xdrs, &hdr))
        {
            FD_SET (chfd, &(chanmask->emask));
            channels[chfd].chanerr = CHANE_BADHDR;
            xdr_destroy (&xdrs);
            return;
        }

        if (hdr.length)
        {
            rcvbuf->len = hdr.length + LSF_HEADER_LEN;
            newdata = realloc (rcvbuf->data, (size_t) rcvbuf->len);
            if (!newdata)
            {
                FD_SET (chfd, &(chanmask->emask));
                channels[chfd].chanerr = LSE_MALLOC;
                xdr_destroy (&xdrs);
                return;
            }
            rcvbuf->data = newdata;
        }
        xdr_destroy (&xdrs);
    }

    assert( rcvbuf->len <= LONG_MAX );
    if (rcvbuf->pos == (long) rcvbuf->len)
    {
        FD_SET (chfd, &(chanmask->rmask));
    }

    return;
}

void
dowrite (int chfd, struct Masks *chanmask)
{
    long cc = 0;
    struct Buffer *sendbuf = NULL;
    
    if (channels[chfd].send->forw == channels[chfd].send) {
        return;
    }
    else {
        sendbuf = channels[chfd].send->forw;
    }

    // paranoid
    assert( sendbuf->pos >= 0 );
    // assert( sendbuf->len - sendbuf->pos >= 0 );
    assert( channels[chfd].handle <= INT_MAX );
    cc = write ( (int)channels[chfd].handle, sendbuf->data + sendbuf->pos, (sendbuf->len - (size_t) sendbuf->pos) ) ;
    if (cc < 0 && BAD_IO_ERR (errno))
    {
        FD_SET (chfd, &(chanmask->emask));
        channels[chfd].chanerr = LSE_MSG_SYS;
        return;
    }
    sendbuf->pos += cc;
    // FIXME cast is ok, sendbuf->len is always greater or equal to zero
    if (sendbuf->pos == (long) sendbuf->len)
    {
        dequeue_ (sendbuf);
        free (sendbuf->data);
        free (sendbuf);
    }
    return;
}

struct Buffer *
newBuf (void)
{
    struct Buffer *newbuf = NULL;

    newbuf = calloc (1, sizeof (struct Buffer));
    if (!newbuf){
        return NULL;
    }

    newbuf->forw = newbuf->back = newbuf;
    newbuf->len = 0;
    newbuf->pos = 0;
    newbuf->data = NULL;
    newbuf->stashed = FALSE;

    return newbuf;
}

int
chanAllocBuf_ (struct Buffer **buf, int size)
{
    *buf = newBuf ();
    if (!*buf) {
        return -1;
    }

    (*buf)->data = calloc( (size_t) size, (size_t) sizeof (char));
    if ((*buf)->data == NULL)
    {
        free (*buf);
        return -1;
    }

    return 0;
}

int
chanFreeBuf_ (struct Buffer *buf)
{
    if (buf)
    {
        if (buf->stashed) {
            return 0;
        }

        if (buf->data) {
            free (buf->data);
        }

        free (buf);
    }
    return 0;
}

int
chanFreeStashedBuf_ (struct Buffer *buf)
{
    if (buf)
    {
        buf->stashed = FALSE;
        return chanFreeBuf_ (buf);
    }
    return -1;
}

void
dequeue_ (struct Buffer *entry)
{
    entry->back->forw = entry->forw;
    entry->forw->back = entry->back;
}

void
enqueueTail_ (struct Buffer *entry, struct Buffer *pred)
{
    entry->back = pred->back;
    entry->forw = pred;
    pred->back->forw = entry;
    pred->back = entry;
}

int
findAFreeChannel (void)
{
    unsigned int i = 0;
    int  returnValue = 0;

    if (chanIndex != 0)
    {
        for (i = 0; i < chanIndex; i++) {
            if (channels[i].handle == INVALID_HANDLE) {
                break;
            }
        }
    }

        if (i == chanIndex) {
            chanIndex++;
        }

        if (i == chanMaxSize)
        {
            assert( chanMaxSize <= UINT_MAX );
            chanIndex = (unsigned int)chanMaxSize;
            return -1;
        }

        channels[i].handle = INVALID_HANDLE;
        channels[i].state = CH_FREE;
        channels[i].send = NULL;
        channels[i].recv = NULL;
        channels[i].chanerr = CHANE_NOERR;

        assert( i <= INT_MAX );
        returnValue = (int)i;
        return returnValue;
    }
