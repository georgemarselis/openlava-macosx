/* $Id: lib.rex.c 397 2007-11-26 19:04:00Z mblack $
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
#include <unistd.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/mls.h"

// extern int currentSN;

int
ls_rexecve ( const char *host, char **argv, int options, char **envp)
{
    int d = 0;
    int retsock = 0;
    struct timeval timeout;
    socklen_t len;
    struct sockaddr_in sin;
    long max = 0;
    char sock_buf[20];
    char *new_argv[5];
    char pathbuf[MAX_PATH_LEN];
    int s = 0;
    int descriptor[2];
    struct resCmdBill cmdmsg;
    int resTimeout;

    if (genParams_[RES_TIMEOUT].paramValue) {
        resTimeout = atoi (genParams_[RES_TIMEOUT].paramValue);
    }
    else {
        resTimeout = RES_TIMEOUT;
    }

    if (_isconnected_ (host, descriptor)) {
        s = descriptor[0];
    }
    else if ((s = ls_connect (host)) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (!FD_ISSET (s, &connection_ok_)) {
        FD_SET (s, &connection_ok_);
        if (ackReturnCode_ (s) < 0) {
            close (s);
            _lostconnection_ (host);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    cmdmsg.options = options & ~REXF_TASKPORT;
    if (cmdmsg.options & REXF_SHMODE) {
        cmdmsg.options |= REXF_USEPTY;
    }

    if (!isatty (0) && !isatty (1)) {
        cmdmsg.options &= ~REXF_USEPTY;
    }
    else if (cmdmsg.options & REXF_USEPTY) {
        if (do_rstty1_ (host, FALSE ) < 0) {// FALSE turns async off
            _lostconnection_ (host);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if ((genParams_[LSF_INTERACTIVE_STDERR].paramValue != NULL)  && (strcasecmp (genParams_[LSF_INTERACTIVE_STDERR].paramValue, "y") == 0)) {
        cmdmsg.options |= REXF_STDERR;
    }

    if (mygetwd_ (cmdmsg.cwd) == 0) {
        close (s);
        _lostconnection_ (host);
        lserrno = LSE_WDIR;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (envp) {
        if (ls_rsetenv (host, envp) < 0) {
            _lostconnection_ (host);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if ((retsock = TcpCreate_ (TRUE, 0)) < 0) {
        close (s);
        _lostconnection_ (host);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    len = sizeof (sin);
    if (getsockname (retsock, (struct sockaddr *) &sin, &len) < 0) {
        // WAS (void) close (retsock);
        close (retsock);
        close (s);
        _lostconnection_ (host);
        lserrno = LSE_SOCK_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    cmdmsg.retport  = sin.sin_port;
    cmdmsg.rpid     = 0;
    cmdmsg.argv     = argv;
    cmdmsg.priority = 0;

    timeout.tv_usec = 0;
    timeout.tv_sec = resTimeout;
    if (sendCmdBill_ (s, (enum resCmd) RES_EXEC, &cmdmsg, &retsock, &timeout) == -1) {
        close (retsock);
        close (s);
        _lostconnection_ (host);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }


    // WAS (void) sprintf (sock_buf, "%d", retsock);
    sprintf (sock_buf, "%d", retsock);

    if (initenv_ (NULL, NULL) < 0) { // DOCUMENT what does NULL, NULL do?
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    strcpy (pathbuf, genParams_[LSF_SERVERDIR].paramValue);
    strcat (pathbuf, "/nios"); // FIXME FIXME FIXME set in configure.ac
    new_argv[0] = pathbuf;  // FIXME FIXME FIXME what is [0]
    new_argv[1] = "-n";     // FIXME FIXME FIXME what is [1] and "-n"?
    new_argv[2] = sock_buf; // FIXME FIXME FIXME what is [2]

    if (cmdmsg.options & REXF_USEPTY) {
        if (cmdmsg.options & REXF_SHMODE) {
            new_argv[3] = "2"; // FIXME FIXME FIXME what is [3] and "2"?
        }
        else {
            new_argv[3] = "1"; // FIXME FIXME FIXME what is [3] and "1"?
        }
    }
    else {
        new_argv[3] = "0"; // FIXME FIXME FIXME what is [3] and "0"?
    }
    new_argv[4] = 0; // FIXME FIXME FIXME what is [4] and why is it 0?

    max = sysconf (_SC_OPEN_MAX);
    for (d = 3; d < max; ++d) {
        if (d != retsock) {
            // WAS (void) close (d);
            close (d);
        }
    }

    // WAS (void) lsfExecX(new_argv[0], new_argv, execvp);
    lsfExecX(new_argv[0], new_argv, execvp);  // FIXME FIXME FIXME what is [0]
    lserrno = LSE_EXECV_SYS;
    close (retsock);
    close (s);
    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
}

// int
// ls_rexecv (const char *host, char **argv, int options)
// {
//     ls_rexecve (host, argv, options, environ);
//     return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
// }

/* ls_startserver()
 */
int
ls_startserver (const char *host, char **server, int options)
{
    int retsock = 0;
    char official[MAXHOSTNAMELEN];
    struct timeval timeout;
    struct sockaddr_in sin;
    int s;
    int descriptor[2];
    struct resCmdBill cmdmsg;
    int resTimeout;
    socklen_t len;

    memset( official, '\0', strlen( official ) );

    if (genParams_[RES_TIMEOUT].paramValue) {
        resTimeout = atoi (genParams_[RES_TIMEOUT].paramValue);
    }
    else {
        resTimeout = RES_TIMEOUT;
    }

    if (_isconnected_ (host, descriptor)) {
        s = descriptor[0]; // FIXME FIXME FIXME what is [0]
    }
    else if ((s = ls_connect (host)) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (!FD_ISSET (s, &connection_ok_)) {
        FD_SET (s, &connection_ok_);
        if (ackReturnCode_ (s) < 0) {
            close (s);
            _lostconnection_ (host);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if (!isatty (0) && !isatty (1)) {// FIXME FIXME FIXME what is [0]
        options &= ~REXF_USEPTY;
    }
    else if (options & REXF_USEPTY) {
        if (do_rstty1_ (host, FALSE) < 0) { // FALSE turns async off
            _lostconnection_ (host);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if (mygetwd_ (cmdmsg.cwd) == 0) {
        close (s);
        _lostconnection_ (host);
        lserrno = LSE_WDIR;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if ((retsock = TcpCreate_ (TRUE, 0)) < 0) {
        close (s);
        _lostconnection_ (host);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    len = sizeof (sin);
    if (getsockname (retsock, (struct sockaddr *) &sin, &len) < 0) {
        close (retsock);
        close (s);
        _lostconnection_ (host);
        lserrno = LSE_SOCK_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    cmdmsg.retport = sin.sin_port;

    cmdmsg.options = options & ~REXF_TASKPORT;
    cmdmsg.rpid = 0;
    cmdmsg.argv = server;

    timeout.tv_usec = 0;
    timeout.tv_sec = resTimeout;

    if (sendCmdBill_ (s, (enum resCmd) RES_SERVER, &cmdmsg, &retsock, &timeout) == -1) {
        close (retsock);
        close (s);
        _lostconnection_ (host);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (ackReturnCode_ (s) < 0) {
        close (retsock);
        close (s);
        _lostconnection_ (host);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (retsock <= 2 && (retsock = get_nonstd_desc_ (retsock)) < 0) {
        close (s);
        _lostconnection_ (host);
        lserrno = LSE_SOCK_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    gethostbysock_ (s, official);
    //  WAS (void) connected_ (official, -1, retsock, currentSN);
    connected_ (official, -1, retsock, currentSN);

    return retsock;
}
