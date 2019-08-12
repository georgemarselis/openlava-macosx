/* $Id: lib.rtask.c 397 2007-11-26 19:04:00Z mblack $
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
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/res.h"
#include "lib/mls.h"
#include "lib/queue.h"
#include "lib/xdrres.h"
#include "lib/rtask.h"
#include "lib/nios.h"
#include "lib/sig.h"
#include "lib/rdwr.h"
#include "lib/initenv.h"
#include "lib/init.h"
#include "lib/cwd.h"
#include "lib/taskid.h"
#include "lib/res.h"
#include "lib/structs/genParams.h"
#include "daemons/libniosd/niosd.h"
#include "daemons/libresd/resout.h"

extern char **environ; // man 7 environ: The  variable environ points to an array of pointers to strings called the "environment".

// what does this function do?
// This is a wrapper function that copies the current environment?
//    and then passes the copied environment to the real function.
//
// On return, frees up the copied environment and returns the returned pid.
//
int
ls_rtask ( const char *host, char **argv, int options) // FIXME FIXME FIXME FIXME what is the diff between ls_rtask and ls_rtaske
{
    pid_t rpid = 0; // store the returned pid
    char **envp = NULL;
    unsigned long numEnv = 0;

    for ( numEnv = 0; environ[numEnv]; numEnv++) // FIXME FIXME there has to be a better way to get the length of the array // FIXME FIXME FIXME FIXME where do we get environ[] from?
        ;
    envp = malloc( numEnv * sizeof (char *) + 1 );
    for (numEnv = 0; environ[numEnv]; numEnv++) { // FIXME FIXME since we know how many elements there are, see if you can replace the loop with a memcpy
        envp[numEnv] = strdup (environ[numEnv]);
    }
    envp[numEnv] = NULL;

    rpid = ls_rtaske (host, argv, options, envp);

    if (envp) {
        for (numEnv = 0; envp[numEnv]; numEnv++) {
            FREEUP (envp[numEnv]);
        }
        FREEUP (envp);
    }

    return rpid;
}

int
ls_rtaske (const char *host, char **argv, int options, char **envp) // FIXME FIXME FIXME FIXME what is the diff between ls_rtask and ls_rtaske
{
    static unsigned short retport = 0;
    static pid_t rpid = -1;
    static int reg_ls_donerex = FALSE;
    struct sockaddr_in sin;
    long max = 0;
    char c_chfd[8];  // FIXME FIXME FIXME FIXME why 8?
    char pathbuf[MAX_PATH_LEN];
    int d = 0;
    int niosOptions = 0;
    char *new_argv[5]; // FIXME FIXME FIXME FIXME why 5?
    pid_t pid = -1;
    int socket = 0;
    int descriptor[2] = { 0, 0 }; // FIXME FIXME describe the two items of this array
    struct resCmdBill cmdmsg;
    struct lslibNiosRTask taskReq;
    unsigned short taskPort = 0;
    sigset_t newMask;
    sigset_t oldMask;
    socklen_t len = 0;
    enum filedescriptor {
        SOCKET = 0
    };

    // set memory
    memset( pathbuf, '\0', strlen( pathbuf ) );
    sigemptyset( &newMask );
    sigemptyset( &oldMask );

    // start setup, before fork
    if (!reg_ls_donerex) {
        atexit ((void (*)(void)) ls_donerex);
        reg_ls_donerex = TRUE;
    }

    // see if a specified host is connected
    //      get the socket which connects to it
    // else
    //      try to connect to the host again
    //          else return to caller
    if (_isconnected_ (host, descriptor)) { 
        socket = descriptor[SOCKET];
    }
    else if ((socket = ls_connect (host)) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    else {
        fprintf( stderr, "ls_rtaske init : you are not supposed to be here" );
    }

    // FIXME FIXME block all incoming signals from caller ( why?)
    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (!FD_ISSET (socket, &connection_ok_)) {
        FD_SET (socket, &connection_ok_);
        if (ackReturnCode_ (socket) == UINT_MAX ) {
            close (socket);
            _lostconnection_ (host);
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if (!nios_ok_) {
        niosOptions = options & REXF_SYNCNIOS;
    }
    options &= ~REXF_SYNCNIOS;

    cmdmsg.options = options;
    if (cmdmsg.options & REXF_SHMODE) {
        cmdmsg.options |= REXF_USEPTY;
    }

    if (!isatty (0) && !isatty (1)) { // FIXME FIXME FIXME explain (0) and (1)
        cmdmsg.options &= ~REXF_USEPTY;
    }
    else if (cmdmsg.options & REXF_USEPTY) {
        if (options & REXF_TTYASYNC) {
            if (do_rstty1_ (host, TRUE ) < 0) { // TRUE turns async on
                sigprocmask (SIG_SETMASK, &oldMask, NULL);
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }
        else {
            if (do_rstty1_ (host, FALSE ) < 0) { // FALSE turns async off
                sigprocmask (SIG_SETMASK, &oldMask, NULL);
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }
    }

    if ((genParams_[LSF_INTERACTIVE_STDERR].paramValue != NULL) && (strcasecmp (genParams_[LSF_INTERACTIVE_STDERR].paramValue, "y") == 0)) {
        cmdmsg.options |= REXF_STDERR;
    }

    if (!nios_ok_) {

        initSigHandler (SIGTSTP);
        initSigHandler (SIGTTIN);
        initSigHandler (SIGTTOU);

        if (socketpair (AF_UNIX, SOCK_STREAM, 0, cli_nios_fd) < 0) {
            lserrno = LSE_SOCK_SYS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            close (socket);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if ((pid = fork ()) != 0) {
            pid_t mypid = -1;

            close (cli_nios_fd[1]); // FIXME FIXME FIXME FIXME what is cli_nios_fd[0] ? replace with LABEL
            mypid = getpid ();

            if (b_write_fix (cli_nios_fd[0], (char *) &mypid, sizeof (mypid)) != sizeof (mypid)) { // FIXME FIXME FIXME FIXME what is cli_nios_fd[0] ? replace with LABEL
                close (cli_nios_fd[0]); // FIXME FIXME FIXME FIXME what is cli_nios_fd[0] ? replace with LABEL
                sigprocmask (SIG_SETMASK, &oldMask, NULL);
                lserrno = LSE_MSG_SYS;
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }

            if (b_read_fix (cli_nios_fd[0], (char *) &retport, sizeof (unsigned short)) != sizeof (unsigned short)) { // FIXME FIXME FIXME FIXME what is cli_nios_fd[0] ? replace with LABEL
                    close (cli_nios_fd[0]); // FIXME FIXME FIXME FIXME what is cli_nios_fd[0] ? replace with LABEL
                    sigprocmask (SIG_SETMASK, &oldMask, NULL);
                    lserrno = LSE_MSG_SYS;
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }

            nios_ok_ = TRUE;

            if (waitpid (pid, 0, 0) < 0) {
                if (errno != ECHILD) {
                    close (cli_nios_fd[0]); // FIXME FIXME FIXME FIXME what is cli_nios_fd[0] ? replace with LABEL
                    nios_ok_ = FALSE;
                    sigprocmask (SIG_SETMASK, &oldMask, NULL);
                    lserrno = LSE_WAIT_SYS;
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            }
        }
        else {
            if( fork( ) ) {
                max = sysconf (_SC_OPEN_MAX);
                for (d = 3; d < max; ++d) {
                    close (d);
                }
                exit (0); // FIXME FIXME FIXME exit call in a library function
            }

            if (initenv_ (NULL, NULL) < 0) {
                exit (-1); // FIXME FIXME FIXME exit call in a library function
            }
            strcpy (pathbuf, genParams_[LSF_SERVERDIR].paramValue);
            strcat (pathbuf, "/nios"); // FIXME FIXME FIXME FIXME replace /nios with constant from configure.ac
            sprintf (c_chfd, "%d", cli_nios_fd[1]);  // FIXME FIXME FIXME FIXME what is cli_nios_fd[1] ? replace with LABEL
            new_argv[0] = pathbuf; // FIXME FIXME FIXME FIXME what is new_argv[0] ? replace with LABEL
            new_argv[1] = c_chfd; // FIXME FIXME FIXME FIXME what is new_argv[1] ? replace with LABEL
            if (cmdmsg.options & REXF_USEPTY) {
                if (cmdmsg.options & REXF_SHMODE) {
                        strcpy( new_argv[2], "2" ); // FIXME FIXME FIXME FIXME what is new_argv[1] ? replace with LABEL
                }
                else {
                        strcpy( new_argv[2], "1" ); // FIXME FIXME FIXME FIXME what is new_argv[1] ? replace with LABEL
                }
            }
            else {
                strcpy( new_argv[2], "0" ); // FIXME FIXME FIXME FIXME what is new_argv[1] ? replace with LABEL
            }
            new_argv[3] = NULL; // FIXME FIXME FIXME FIXME what is new_argv[1] ? replace with LABEL

            max = sysconf (_SC_OPEN_MAX);
            for (d = 3; d < max; ++d) {
                if (d != cli_nios_fd[1]) {
                    close (d);
                }
            }

            for (d = 1; d < SIGRTMAX; d++) { // SIGRTMAX is in <signal.h>
                Signal_ ((unsigned int)d, SIG_DFL);
            }


            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            // (void) lsfExecX (new_argv[0], new_argv, execvp);
            lsfExecX (new_argv[0], new_argv, execvp);
            exit (-1); // FIXME FIXME FIXME exit call in a library function
        }
    }

    if (envp) {
        // if (ls_rsetenv_async (host, envp) < 0)
        if (rsetenv_ (host, envp, RSETENV_ASYNC) < 0) {
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if (rexcwd_[0] != '\0') {
        strcpy (cmdmsg.cwd, rexcwd_);
    }
    else if (mygetwd_ (cmdmsg.cwd) == 0) {
        close (socket);
        _lostconnection_ (host);
        lserrno = LSE_WDIR;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }


    rpid++;

    cmdmsg.rpid = rpid;
    cmdmsg.retport = retport;

    cmdmsg.argv = argv;
    cmdmsg.priority = 0;

    // if (sendCmdBill_ (socket, (enum resCmd) RES_EXEC, &cmdmsg, NULL, NULL) == -1)
    if (sendCmdBill_ (socket, RES_EXEC, &cmdmsg, NULL, NULL) == -1) {
        close (socket);
        _lostconnection_ (host);
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (cmdmsg.options & REXF_TASKPORT) {
        if ((taskPort = getTaskPort (socket)) == 0) {
            close (socket);
            _lostconnection_ (host);
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    len = sizeof (sin);
    if (getpeername (socket, (struct sockaddr *) &sin, &len) < 0) {
        close (socket);
        _lostconnection_ (host);
        lserrno = LSE_SOCK_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    SET_LSLIB_NIOS_HDR (taskReq.hdr, LIB_NIOS_RTASK, sizeof (taskReq.r));
    taskReq.r.taskid = (unsigned long) rpid; // FIXME FIXME FIXME figure out of taskid/rpid should be unsigned long or pid_t
    taskReq.r.peer = sin.sin_addr;

    // taskReq.r.taskid = (niosOptions & REXF_SYNCNIOS) ? -rpid : rpid;
    taskReq.r.taskid = (niosOptions & REXF_SYNCNIOS) ? 0 : (unsigned long) rpid;

    if (b_write_fix (cli_nios_fd[0], (char *) &taskReq, sizeof (taskReq)) != sizeof (taskReq)) {
        close (socket);
        _lostconnection_ (host);
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (tid_register ((unsigned long)rpid, socket, taskPort, host, options & REXF_TASKINFO) == ULONG_MAX ) { // FIXME FIXME FIXME figure out if rpid should be pid_t or unsigned long
        close (socket);
        _lostconnection_ (host);
        lserrno = LSE_MALLOC;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    sigprocmask (SIG_SETMASK, &oldMask, NULL);

    return rpid;
}

int
rgetpidCompletionHandler_ (struct lsRequest *request)
{
    unsigned int rc = 0;
    XDR xdrs;
    struct resPid pidReply;

    assert ( request );

    rc = resRC2LSErr_ (request->rc);
    if (rc != 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    xdrmem_create (&xdrs, request->replyBuf, sizeof (struct resPid), XDR_DECODE);
    if (!xdr_resGetpid (&xdrs, &pidReply, NULL)) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    *((int *) request->extra) = pidReply.pid;
    xdr_destroy (&xdrs);
    return 0;

}

// void * // was
struct lsRequest *
lsRGetpidAsync_ (unsigned long taskid, pid_t *pid) // FIXME FIXME FIXME nothing is using this function.
{
    struct _buf_
    {
        struct LSFHeader hdrBuf;
        struct resPid pidBuf;
    } buf;
    struct lsRequest *request = NULL;

    struct resPid pidReq;
    int socket = 0;
    struct tid *tid = NULL;
    char host[MAXHOSTNAMELEN];

    memset( host, '\0', strlen( host ) );

    if ((tid = tid_find (taskid)) == NULL) {
        return NULL;
    }

    socket = tid->sock;
    gethostbysock_ (socket, host);

    if (!FD_ISSET (socket, &connection_ok_)) {
        FD_SET (socket, &connection_ok_);
        if (ackReturnCode_ (socket) == UINT_MAX ) {
            close (socket);
            _lostconnection_ (host);
            return NULL;
        }
    }

    pidReq.rtaskid = taskid;
    pidReq.pid = -1;

    if (callRes_ (socket, RES_GETPID, (char *) &pidReq, (char *) &buf, sizeof (buf), xdr_resGetpid, 0, 0, NULL) == -1) {
        close (socket);
        _lostconnection_ (host);
        return NULL;
    }
    request = lsReqHandCreate_ (taskid, globCurrentSN, socket, (void *) pid, rgetpidCompletionHandler_, (appCompletionHandler) NULL, NULL); // FIXME FIXME what is up with all these casts?

    if (request != NULL) {
        if (lsQueueDataAppend_ ((char *) request, requestQ)) {
            return NULL;
        }
    }

    return request;
}

pid_t
lsRGetpid_ ( unsigned long taskid, int options)
{
    struct _buf_
    {
        struct LSFHeader hdrBuf;
        struct resPid pidBuf;
    } buf;
    struct lsRequest *request = NULL;
    pid_t pid = -1;

    struct resPid pidReq;
    int socket = 0;
    struct tid *tid = NULL;
    char host[MAXHOSTNAMELEN];

    memset( host, '\0', strlen( host ) );
    assert( options );

    if ((tid = tid_find (taskid)) == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    socket = tid->sock;
    gethostbysock_ (socket, host);

    if (!FD_ISSET (socket, &connection_ok_)) {
        FD_SET (socket, &connection_ok_);
        if (ackReturnCode_ (socket) == UINT_MAX ) {
            close (socket);
            _lostconnection_ (host);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    pidReq.rtaskid = taskid;
    pidReq.pid = -1;

    if (callRes_ (socket, RES_GETPID, (char *) &pidReq, (char *) &buf, sizeof (buf), xdr_resGetpid, 0, 0, NULL) == -1) {
        close (socket);
        _lostconnection_ (host);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    request = lsReqHandCreate_ (taskid, globCurrentSN, socket, (void *) &pid, rgetpidCompletionHandler_, (appCompletionHandler) NULL, NULL); // FIXME FIXME what is up with all these casts?

    if (request == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsQueueDataAppend_ ((char *) request, requestQ)) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (lsReqWait_ (request, 0) < 0)
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value

    lsReqFree_ (request);

    return pid;
}

unsigned int
lsRGetpgrp_ (int socket, unsigned long taskid, pid_t pid)
{
    struct _buf_
    {
        struct LSFHeader hdrBuf;
        struct resPid pidBuf;
    } buf;

    struct lsRequest *request = NULL;
    char host[MAXHOSTNAMELEN];

    struct resPid pidReq;
    gid_t pgid = 0;

    memset( host, '\0', strlen( host ) );

    gethostbysock_ (socket, host);

    if (!FD_ISSET (socket, &connection_ok_)) {
        FD_SET (socket, &connection_ok_);
        if (ackReturnCode_ (socket) == UINT_MAX ) {
            close (socket);
            _lostconnection_ (host);
            return UINT_MAX;
        }
    }

    pidReq.rtaskid = taskid;
    pidReq.pid = pid;

    if (callRes_ (socket, RES_GETPID, (char *) &pidReq, (char *) &buf, sizeof (buf), xdr_resGetpid, 0, 0, NULL) == -1) {
        close (socket);
        _lostconnection_ (host);
        return UINT_MAX;
    }

    request = lsReqHandCreate_ (taskid, globCurrentSN, socket, (void *) &pgid, rgetpidCompletionHandler_, (appCompletionHandler) NULL, NULL);

    if (request == NULL) {
        return UINT_MAX;
    }

    if (lsQueueDataAppend_ ((char *) request, requestQ)) {
        return UINT_MAX;
    }

    if (lsReqWait_ (request, 0) < 0) {
        return UINT_MAX;
    }

    lsReqFree_ (request);

    return pgid;

}

void
initSigHandler (int sig)
{
    struct sigaction act, oact;

    act.sa_handler = (SIGFUNCTYPE) default_tstp_;
    sigemptyset (&act.sa_mask);
    sigaddset (&act.sa_mask, sig);
    act.sa_flags = 0;
    sigaction (sig, &act, &oact);

    if (oact.sa_handler != SIG_DFL) {
        sigaction (sig, &oact, NULL);
    }

    return;
}

void
// default_tstp_ (signo_t signo)
default_tstp_ ( int signo )
{
    assert( signo );
    // (void) ls_stoprex ();
    ls_stoprex ();
    kill (getpid (), SIGSTOP);
}

unsigned short
getTaskPort (int socket)
{
    unsigned int rc = 0;
    struct LSFHeader hdr;

    rc = expectReturnCode_ (socket, globCurrentSN, &hdr);
    if (rc == USHRT_MAX ) {
        return 0;
    }

    return htons (hdr.opCode);
}

// NOFIX DELETE! nothing calls this function
// void
// setRexWd_ (const char *wd)
// {
//     if (wd) {
//         strcpy (rexcwd_, wd); // rexcwd_ wtf is it?
//     }
//     else {
//         rexcwd_[0] = '\0';
//     }
//     return;
// }
