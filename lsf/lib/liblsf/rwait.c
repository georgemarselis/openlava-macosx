/* $Id: lib.rwait.c 397 2007-11-26 19:04:00Z mblack $
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

#include <pwd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/rwait.h"
#include "lib/lib.h"
// // #include "lib/lproto.h"
#include "lib/sig.h"
#include "lib/rdwr.h"
#include "lib/taskid.h"
#include "lib/rtask.h"
#include "daemons/libniosd/niosd.h"
#include "daemons/libresd/resout.h"


// NOTE following two are wrapper functions. Taken out of the code base, but keeping the calls just in case.

// int
// ls_rwait (LS_WAIT_T * status, mode_t options, struct rusage *ru)
// {
//     return rwait_ (0, status, options, ru);
// }

// int
// ls_rwaittid (int tid, LS_WAIT_T * status, mode_t options, struct rusage *ru)
// {
//     return rwait_ (tid, status, options, ru);
// }

int
rwait_ ( unsigned long taskid, LS_WAIT_T * status, mode_t options, struct rusage *ru)
{
    struct lslibNiosWaitReq req;
    struct lslibNiosHdr hdr;
    fd_set rmask;
    struct timeval timeout;
    sigset_t newMask;
    sigset_t oldMask;
    int cc = 0;

    // if (tid < 0) {
    //     lserrno = LSE_BAD_ARGS;
    //     return -1; // FIXME FIXME FIXME replace with appropriate positive number
    // }

    if (!nios_ok_) {
        lserrno = LSE_NORCHILD;
        return -1; // FIXME FIXME FIXME replace with appropriate positive number
    }


    blockALL_SIGS_ (&newMask, &oldMask);

Start:

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask);
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    SET_LSLIB_NIOS_HDR (req.hdr, LIB_NIOS_RWAIT, sizeof (req.r));
    req.r.options = options;
    req.r.taskid = taskid;

    if (b_write_fix (cli_nios_fd[0], (char *) &req, sizeof (req)) != sizeof (req)) {
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME replace with appropriate positive number
    }


    cc = select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout);
    if (cc <= 0) {
        if (cc < 0) {
            lserrno = LSE_SELECT_SYS;
        }
        else {
            lserrno = LSE_TIME_OUT;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME replace with appropriate positive number
        }
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &hdr, sizeof (hdr)) == -1) {
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME replace with appropriate positive number
    }

    if (WAIT_BLOCK (options) && hdr.opCode == NONB_RETRY) {

        restartRWait (oldMask);
        if (!isPamBlockWait) {
            goto Start;
        }
    }

    switch (hdr.opCode) {
        case CHILD_FAIL:
            lserrno = LSE_NORCHILD;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return INT_MAX; // FIXME FIXME FIXME replace with appropriate positive number

        break;
        case NONB_RETRY:
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return 0;

        break;
        case CHILD_OK:
            taskid = readWaitReply (status, ru); // FIXME FIXME FIXME is taskid unsigned long or pid_t?
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return (int) taskid; // FIXME FIXME FIXME is taskid unsigned long or pid_t?

        break;
        default:

            lserrno = LSE_PROTOC_NIOS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return INT_MAX;
    }

    return INT_MAX; // error
}

unsigned long 
readWaitReply (LS_WAIT_T * status, struct rusage *ru)
{
    struct lslibNiosWaitReply reply;

    if (b_read_fix (cli_nios_fd[0], (char *) &reply.r, sizeof (reply.r)) != sizeof (reply.r)) { // FIXME FIXME FIXME describe [0]
        lserrno = LSE_MSG_SYS;
        return ULONG_MAX; // FIXME FIXME FIXME replace with appropriate positive number
    }

    tid_remove (reply.r.taskid);
    if (status) {
        LS_STATUS (*status) = (int) reply.r.status; // FIXME FIXME investiage why we need this.
    }
    if (ru) {
        *ru = reply.r.ru;
    }

    return reply.r.taskid;
}


void
restartRWait (sigset_t oldMask)
{
    int usr1handler = FALSE;
    struct sigaction act, oact, usr1sigact;
    sigset_t pauseMask;


    sigaction (SIGUSR1, NULL, &oact);

    if (oact.sa_handler == SIG_ERR ||
#ifdef SIG_HOLD
            oact.sa_handler == SIG_HOLD ||
#endif
#ifdef SIG_CATCH
            oact.sa_handler == SIG_CATCH ||
#endif
            oact.sa_handler == SIG_IGN || oact.sa_handler == SIG_DFL)
        {

            usr1handler = TRUE;
            usr1sigact = oact;
            act.sa_handler = (SIGFUNCTYPE) usr1Handler;
            sigfillset (&act.sa_mask);
            act.sa_flags = 0;
            sigaction (SIGUSR1, &act, NULL);
        }


    pauseMask = oldMask;
    sigdelset (&pauseMask, SIGUSR1);
    sigsuspend (&pauseMask);

    lserrno = LSE_SIG_SYS;

    if (usr1handler) {
        sigaction (SIGUSR1, &usr1sigact, NULL);
    }

    return;
}

// void
// usr1Handler (int sig)
// {
//     assert( sig );

//     return;
// }
