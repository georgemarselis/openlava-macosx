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
#include <sys/time.h>
#include <unistd.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/rdwr.h"

#define IO_TIMEOUT  2000

#define US_DIFF(t1, t2) (((t1).tv_sec - (t2).tv_sec) * 1000000 + (t1).tv_usec - (t2).tv_usec)

static void alarmer_ (void);

// FIXME FIXME FIXME
// changed return type from into to size_t
//      ensure with the debugger/test case that this function
//      cannot return negative values
long
nb_write_fix (int s, char *buf,  size_t len)
{
    ssize_t cc;
    size_t length = 0;
    struct timeval start, now;

    struct timezone junk;

    gettimeofday (&start, &junk);

    for (length = len; len > 0;) {
        if ((cc = write (s, buf, len)) > 0) {
            assert( cc >= 0);
            assert( len - (size_t)cc >= 0 );
            len -= (size_t) cc;
            buf += cc;
            }
        else if (cc < 0 && BAD_IO_ERR (errno)) {
            if (errno == EPIPE) {
                lserrno = LSE_LOSTCON;
            }

            return (-1);
        }
        
        if (len > 0) {
            gettimeofday (&now, &junk);
            
            if (US_DIFF (now, start) > IO_TIMEOUT * 1000) {
                errno = ETIMEDOUT;
                return (-1);
            }
        
            millisleep_ (IO_TIMEOUT / 20);
        }
    }
    
    return (ssize_t) length;
}

// FIXME FIXME FIXME
// changed return type from long into to size_t
//      ensure with the debugger/test case that this function
//      does return negative values
size_t
nb_read_fix (int s, char *buf, size_t len)
{
    long cc = 0;
    size_t length = 0;
    struct timeval start, now;
    struct timezone junk;

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "nb_read_fix: Entering this routine...");
    }

    gettimeofday (&start, &junk);

    for (length = len; len > 0;) {
        if ((cc = read (s, buf, len)) > 0) {
            assert( cc >= 0);
            assert( len - (size_t)cc >= 0 );
            len -= (size_t) cc;
        }
        else if (cc == 0 || BAD_IO_ERR (errno))
        {
            if (cc == 0) {
                errno = ECONNRESET;
            }
            return (-1);
        }
        else  {
            printf( " you should not be here: nb_read_fix()\n");
        }

        if (len > 0)
        {
            gettimeofday (&now, &junk);
            if (US_DIFF (now, start) > IO_TIMEOUT * 1000) {
            
                errno = ETIMEDOUT;
                return (-1);
            }
            
            millisleep_ (IO_TIMEOUT / 20);
        }
    }

    return length;
}

#define MAXLOOP 3000

// FIXME FIXME FIXME
// changed return type from into to size_t
//      ensure with the debugger/test case that this function
//      cannot return negative values
long
b_read_fix (int s, char *buf, size_t len)
{
    int loop      = 0;
    int numLoop   = 0;
    size_t length = 0;
    ssize_t cc     = 0;
        
    if (len > MAXLOOP * 1024) {
        numLoop = MAXLOOP * 100;
    }
    else {
      numLoop = MAXLOOP;
    }

    for (length = len, loop = 0; len > 0 && loop < numLoop; loop++) {

        if ((cc = read (s, buf, len)) > 0) {
            assert( cc >= 0);
            len -= (size_t) cc;
            buf += cc;
        }
        else if (cc == 0 || errno != EINTR) {
            if (cc == 0) {
                errno = ECONNRESET;
            }
            return (-1);
        }
        else {
            printf( "we fucked up again!\n b_read_fix()");
        }
    }

    // FIXME FIXME FIXME
    // this loop is highly suspect. wtf len gets down to 0?
    if (len > 0) {
      return (-1);
    }

    assert( length >= 0 );
    return (long)length;
}


// FIXME FIXME FIXME
// changed return type from into to size_t
//      ensure with the debugger/test case that this function
//      cannot return negative values
long
b_write_fix (int s, char *buf, size_t len)
{

    int cc  = 0;
    size_t loop = 0;
    size_t length = 0;

    // for emphasis
    for ( length = len; len > 0 && loop < MAXLOOP; loop++)
    {
        if ((cc = write (s, buf, len)) > 0) {
            assert( cc >= 0);
            len -= cc;
            buf += cc;
        }
        else if (cc < 0 && errno != EINTR) {
            lserrno = LSE_SOCK_SYS;
            return (-1);
        }
    }

    if (len > 0) {
        lserrno = LSE_SOCK_SYS;
        return (-1);
    }

    assert( length >= 0 );
    return (long)length;
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
b_connect_ (int s, struct sockaddr *name, socklen_t namelen, unsigned int timeout)
{

    unsigned int oldTimer;
    sigset_t newMask;
    sigset_t oldMask;
    struct itimerval old_itimer;
    struct sigaction action;
    struct sigaction old_action;


    if (getitimer (ITIMER_REAL, &old_itimer) < 0)  {
        return -1;
    }


    action.sa_flags = 0;
    action.sa_handler = (SIGFUNCTYPE) alarmer_;

    sigfillset (&action.sa_mask);
    sigaction (SIGALRM, &action, &old_action);

    unblocksig (SIGALRM);

    blockSigs_ (SIGALRM, &newMask, &oldMask);

    oldTimer = alarm (timeout);

    if (connect (s, name, namelen) < 0) {
        if (errno == EINTR) {
            errno = ETIMEDOUT;
        }

        alarm (oldTimer);
        setitimer (ITIMER_REAL, &old_itimer, NULL);

        sigaction (SIGALRM, &old_action, NULL);
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1;
    }

    alarm (oldTimer);

    setitimer (ITIMER_REAL, &old_itimer, NULL);

    sigaction (SIGALRM, &old_action, NULL);
    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

int
rd_select_ (int rd, struct timeval *timeout)
{
  int cc;
  fd_set rmask;

  for (;;)
    {
      FD_ZERO (&rmask);
      FD_SET (rd, &rmask);

      cc = select (rd + 1, &rmask, (fd_set *) 0, (fd_set *) 0, timeout);
      if (cc >= 0)
    return cc;

      if (errno == EINTR)
    continue;
      return (-1);
    }

}

/* b_accept_()
 */
int
b_accept_ (int s, struct sockaddr *addr, socklen_t * addrlen)
{
  sigset_t oldMask;
  sigset_t newMask;
  int cc;

  blockSigs_ (0, &newMask, &oldMask);

  cc = accept (s, addr, addrlen);
  sigprocmask (SIG_SETMASK, &oldMask, NULL);

  return (cc);

}               /* b_accept_() */

int
detectTimeout_ (int s, int recv_timeout)
{
  struct timeval timeval;
  struct timeval *timep = NULL;
  int ready;

  if (recv_timeout)
    {
      timeval.tv_sec = recv_timeout;
      timeval.tv_usec = 0;
      timep = &timeval;
    }
  ready = rd_select_ (s, timep);
  if (ready < 0)
    {
      lserrno = LSE_SELECT_SYS;
      return (-1);
    }
  else if (ready == 0)
    {
      lserrno = LSE_TIME_OUT;
      return (-1);
    }
  return (0);
}

static void
alarmer_ (void)
{
}


int
blockSigs_ (int sig, sigset_t * blockMask, sigset_t * oldMask)
{
  sigfillset (blockMask);

  if (sig)
    sigdelset (blockMask, sig);

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

  return (sigprocmask (SIG_BLOCK, blockMask, oldMask));

}

// FIXME timeout should be of type unsigned timeu_t or something
long
nb_read_timeout (int s, char *buf, size_t len, int timeout)
{
    ssize_t cc      = 0 ;
    int nReady      = 0;
    size_t length   = len;
    struct timeval timeval;

    timeval.tv_sec  = timeout;
    timeval.tv_usec = 0;

    for (;;) {
        nReady = rd_select_ (s, &timeval);
        if (nReady < 0) {
            lserrno = LSE_SELECT_SYS;
            return (-1);
        }
        else if (nReady == 0) {
            lserrno = LSE_TIME_OUT;
            return (-1);
        }
        else {
            if ((cc = recv (s, buf, len, 0)) > 0) {
                assert( cc >= 0 );
                len -= (size_t) cc;
                buf += cc;
            }
            else if (cc == 0 || BAD_IO_ERR (errno)) {
                if (cc == 0) {
                    errno = ECONNRESET;
                }
                
                return (-1);
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
    return (long) length;
}
