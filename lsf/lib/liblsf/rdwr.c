/* $Id: lib.rdwr.c 397 2007-11-26 19:04:00Z mblack $
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

#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "lsf.h"
#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/rdwr.h"
#include "lib/usleep.h"
#include "lib/syslog.h"



// FIXME FIXME FIXME
// changed return type from into to size_t
//      ensure with the debugger/test case that this function
//      cannot return negative values
long
nb_write_fix (int fildes, const char *buf,  size_t len)
{
    ssize_t cc = 0;
    size_t length = 0;
    struct timeval *start = NULL;
    struct timeval *now   = NULL;
    struct timezone *junk = NULL;
    ssize_t return_value  = 0;

    gettimeofday (start, junk);

    for (length = len; len > 0;) {
        if ((cc = write (fildes, buf, len)) > 0) {
            len -= (size_t) cc; // NOFIX cast here is fine
            buf += cc;
        }
        else if (cc < 0 && BAD_IO_ERR (errno)) {
            if (errno == EPIPE) {
                lserrno = LSE_LOSTCON;
            }

            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        
        if (len > 0) {
            gettimeofday (now, junk);
            
            if (US_DIFF (now, start) > IO_TIMEOUT * 1000) {
                errno = ETIMEDOUT;
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        
            millisleep_ (IO_TIMEOUT / 20);
        }
    }

    assert( length <= LONG_MAX );
    return_value = (ssize_t) length;
    return return_value;
}

// FIXME FIXME FIXME
// changed return type from long into to size_t
//      ensure with the debugger/test case that this function
//      does return negative values
long
nb_read_fix (int fildes, char *buf, size_t len)
{
    long cc = 0;
    size_t length = 0;
    struct timeval  *start = NULL;
    struct timeval  *now   = NULL;
    struct timezone *junk  = NULL;
    ssize_t return_value   = 0;

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "nb_read_fix: Entering this routine...");
    }

    gettimeofday (start, junk);

    for (length = len; len > 0;) {
        if ((cc = read (fildes, buf, len)) > 0) {
            assert( cc >= 0 );
            len -= (size_t) cc;
        }
        else if (cc == 0 || BAD_IO_ERR (errno))
        {
            if (cc == 0) {
                errno = ECONNRESET;
            }
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        else  {
            printf( "You should not be here: nb_read_fix()\n");
        }

        if (len > 0)
        {
            gettimeofday (now, junk);
            if (US_DIFF (now, start) > IO_TIMEOUT * 1000) {
            
                errno = ETIMEDOUT;
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
            
            millisleep_ (IO_TIMEOUT / 20);
        }
    }

    assert( length <= LONG_MAX );
    return_value = (ssize_t) length;
    return return_value;
}


// FIXME FIXME FIXME
// changed return type from into to size_t
//      ensure with the debugger/test case that this function
//      cannot return negative values
long
b_read_fix (int fildes, char *buf, size_t len)
{
    int loop      = 0;
    int numLoop   = 0;
    size_t length = 0;
    ssize_t cc    = 0;
    ssize_t return_value   = 0;
        
    if (len > MAXLOOP * 1024) {  // FIXME FIXME FIXME FIXME this looks like it was experimented with to get right. Revisit and remove uncertainty
        numLoop = MAXLOOP * 100; // rdwr.h
    }
    else {
      numLoop = MAXLOOP; // rdwr.h
    }

    for (length = len, loop = 0; len > 0 && loop < numLoop; loop++) {

        if ((cc = read (fildes, buf, len)) > 0) {
            assert( cc >= 0);
            len -= (size_t) cc;
            buf += cc;
        }
        else if (cc == 0 || errno != EINTR) {
            if (cc == 0) {
                errno = ECONNRESET;
            }
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        else {
            printf( "we fucked up again!\n b_read_fix()");
        }
    }

    // FIXME FIXME FIXME
    // this loop is highly suspect. when the frak len gets down to 0?
    if (len > 0) {
      return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    assert( length <= LONG_MAX );
    return_value = (ssize_t) length;    
    return return_value;
}


// FIXME FIXME FIXME
// changed return type from into to size_t
//      ensure with the debugger/test case that this function
//      cannot return negative values
long
b_write_fix (int fildes, const char *buf, size_t len)
{

    ssize_t cc           = 0;
    size_t  loop         = 0;
    size_t  length       = 0;
    ssize_t return_value = 0;

    // for emphasis
    for ( length = (size_t) len; len > 0 && loop < MAXLOOP; loop++) // NOFIX len >= 0, always // MAXLOOP is in rdwr.h
    {
        if ((cc = write (fildes, buf, len)) > 0) {
            assert( cc >= 0);
            length -= (size_t) cc; // NOFIX assert should catch above
            buf += cc;
        }
        else if (cc < 0 && errno != EINTR) {
            lserrno = LSE_SOCK_SYS;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if (len > 0) {
        lserrno = LSE_SOCK_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    assert( length <= LONG_MAX );
    return_value = (ssize_t) length;    
    return return_value;
}

void
unblocksig (int sig)
{
    sigset_t blockMask, oldMask;
    sigemptyset (&blockMask);
    sigaddset (&blockMask, sig);
    sigprocmask (SIG_UNBLOCK, &blockMask, &oldMask);
}

int
b_connect_ (int s, struct sockaddr *name, socklen_t namelen, time_t timeout)
{

    unsigned int oldTimer = 0;
    sigset_t newMask; // Signal Sets: https://www.gnu.org/software/libc/manual/html_node/Signal-Sets.html
    sigset_t oldMask; // not octal, internal representation either as int or structure
    struct itimerval old_itimer;
    struct sigaction action;
    struct sigaction old_action;


    if (getitimer (ITIMER_REAL, &old_itimer) < 0)  {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }


    action.sa_flags = 0;
    action.sa_handler = (SIGFUNCTYPE) alarmer_;

    sigfillset (&action.sa_mask);
    sigaction (SIGALRM, &action, &old_action);

    unblocksig (SIGALRM);

    blockSigs_ (SIGALRM, &newMask, &oldMask);

    assert( timeout <= UINT_MAX  && timeout >= 0);
    oldTimer = alarm ( (unsigned int) timeout);

    if (connect (s, name, namelen) < 0) {
        if (errno == EINTR) {
            errno = ETIMEDOUT;
        }

        alarm (oldTimer);
        setitimer (ITIMER_REAL, &old_itimer, NULL);

        sigaction (SIGALRM, &old_action, NULL);
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    alarm (oldTimer);

    setitimer (ITIMER_REAL, &old_itimer, NULL);

    sigaction (SIGALRM, &old_action, NULL);
    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

int
rd_select_ (long rd, struct timeval *timeout)
{
    int cc   = 0;
    assert( rd <= INT_MAX );
    int ndfs = (int) rd; // FIXME FIXME FIXME remove cast
    fd_set rmask;

    do {
        FD_ZERO (&rmask);
        FD_SET (ndfs, &rmask);

        cc = select (ndfs + 1, &rmask, (fd_set *) 0, (fd_set *) 0, timeout);

        if (errno == EINTR) {
            continue;
        }

    } while ( cc < 0 );

    return cc;
}

/* b_accept_()
 */
int
b_accept_ (int s, struct sockaddr *addr, socklen_t *addrlen)
{
    sigset_t oldMask;
    sigset_t newMask;
    int cc = 0;

    blockSigs_ (0, &newMask, &oldMask);

    cc = accept (s, addr, addrlen);
    sigprocmask (SIG_SETMASK, &oldMask, NULL);

    return cc;

} /* b_accept_() */

int
detectTimeout_ (int s, int recv_timeout)
{
    struct timeval timeval;
    struct timeval *timep = NULL;
    int ready = 0;

    if (recv_timeout) {
        timeval.tv_sec = recv_timeout;
        timeval.tv_usec = 0;
        timep = &timeval;
    }

    ready = rd_select_ (s, timep);
    if (ready < 0) {
        lserrno = LSE_SELECT_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    else if (ready == 0) {
        lserrno = LSE_TIME_OUT;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return 0;
}

void
alarmer_ (void)
{
    return;     // empty
}


int
blockSigs_ (int sig, sigset_t *blockMask, sigset_t *oldMask)
{
    sigfillset (blockMask);

    if (sig)  {
        sigdelset (blockMask, sig);
    }

    sigdelset (blockMask, SIGHUP);
    sigdelset (blockMask, SIGINT);
    sigdelset (blockMask, SIGQUIT);
    sigdelset (blockMask, SIGILL);
    sigdelset (blockMask, SIGTRAP);
    sigdelset (blockMask, SIGFPE);
    sigdelset (blockMask, SIGBUS);
    sigdelset (blockMask, SIGSEGV);
    sigdelset (blockMask, SIGPIPE);
    sigdelset (blockMask, SIGTERM);

  return sigprocmask (SIG_BLOCK, blockMask, oldMask);

}

// FIXME timeout should be of type unsigned timeu_t or something
long
nb_read_timeout (int s, char *buf, size_t len, time_t timeout)
{
    ssize_t cc      = 0;
    int nReady      = 0;
    size_t length   = len;
    struct timeval timeval;
    ssize_t return_value = 0;

    timeval.tv_sec  = timeout;
    timeval.tv_usec = 0;

    for (;;) { // FIXME FIXME FIXME FIXME infinite for loop, has to go.
        nReady = rd_select_ (s, &timeval);
        if (nReady < 0) {
            lserrno = LSE_SELECT_SYS;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        else if (nReady == 0) {
            lserrno = LSE_TIME_OUT;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        else {
            if ((cc = recv (s, buf, len, 0)) > 0) {
                assert( cc >= 0 );
                length -= (size_t) cc;
                buf += cc;
            }
            else if (cc == 0 || BAD_IO_ERR (errno)) {
                if (cc == 0) {
                    errno = ECONNRESET;
                }
                
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
            else {
                printf( "We done goofed up! nb_read_timeout()\n");
            }
            
            if (len == 0) {
                break;
            }
        }
    }

    assert( length <= LONG_MAX );
    return_value = (ssize_t) length;    
    return return_value;
}
