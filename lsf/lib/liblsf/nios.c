/* $Id: lib.nios.c 397 2007-11-26 19:04:00Z mblack $
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
#include <sys/select.h>

#include "lib/nios.h"
#include "lib/rtask.h"
#include "lib/rdwr.h"
#include "lib/signal.h"
#include "daemons/libniosd/niosd.h"
// #include "daemons/libresd/resout.h"

void nios_c_bullshit( void ) {

    assert( INFINIT_LOAD );
    assert( sigSymbol );
    assert( NSIG_MAP );

    return;
}


int
ls_stdinmode( int onoff )
{
    fd_set rmask;
    struct timeval timeout;
    struct lslibNiosHdr reqHdr;
    struct lslibNiosHdr replyHdr;
    sigset_t newMask;
    sigset_t oldMask;
    int cc;

    if (!nios_ok_) {
        lserrno = LSE_NIOS_DOWN;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    SET_LSLIB_NIOS_HDR (reqHdr, (onoff ? LIB_NIOS_REM_ON : LIB_NIOS_REM_OFF), 0);

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask); // FIXME FIXME FIXME label [0]
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    if (b_write_fix (cli_nios_fd[0], (char *) &reqHdr, sizeof (reqHdr)) != sizeof (reqHdr)) { // FIXME FIXME FIXME label [0]
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if ((cc = select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout)) <= 0) {

        if (cc == 0) {
            lserrno = LSE_TIME_OUT;
        }
        else {
            lserrno = LSE_SELECT_SYS; 
        }

        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &replyHdr, sizeof (replyHdr)) == -1) {
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (replyHdr.opCode != REM_ONOFF) {
        lserrno = LSE_PROTOC_NIOS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

int
ls_donerex (void)
{
    fd_set rmask;
    struct timeval timeout;
    struct lslibNiosHdr reqHdr;
    struct lslibNiosHdr replyHdr;
    sigset_t newMask; 
    sigset_t oldMask;

    if (!nios_ok_) {
        lserrno = LSE_NIOS_DOWN;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    SET_LSLIB_NIOS_HDR (reqHdr, LIB_NIOS_EXIT, 0);

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask); // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    if (b_write_fix (cli_nios_fd[0], (char *) &reqHdr, sizeof (reqHdr)) != sizeof (reqHdr)) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }


    if (select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout) <= 0) {// FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_SELECT_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &replyHdr, sizeof (replyHdr)) == -1) {// FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (replyHdr.opCode != NIOS_OK) {
        lserrno = LSE_PROTOC_NIOS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    nios_ok_ = FALSE;

    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

int
ls_stoprex (void)
{
    fd_set rmask;
    struct timeval timeout;
    struct lslibNiosHdr reqHdr;
    struct lslibNiosHdr replyHdr;
    sigset_t newMask;
    sigset_t oldMask;

    if (!nios_ok_) {
        lserrno = LSE_NIOS_DOWN;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask);// FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    SET_LSLIB_NIOS_HDR (reqHdr, LIB_NIOS_SUSPEND, 0);

    if (b_write_fix (cli_nios_fd[0], (char *) &reqHdr, sizeof (reqHdr)) != sizeof (reqHdr)) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }


    if (select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout) <= 0) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_SELECT_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &replyHdr, sizeof (replyHdr)) == -1) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (replyHdr.opCode != NIOS_OK) {
        lserrno = LSE_PROTOC_NIOS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

int
ls_niossync (long numTasks)
{
    fd_set rmask;
    struct timeval timeout;
    struct lslibNiosHdr replyHdr;
    struct lslibNiosHdr reqHdr;
    sigset_t newMask;
    sigset_t oldMask;

    if (!nios_ok_) {
        lserrno = LSE_NIOS_DOWN;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (numTasks < 0) {
        lserrno = LSE_BAD_ARGS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask); // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    // checked above if possitive 
    SET_LSLIB_NIOS_HDR (reqHdr, LIB_NIOS_SYNC, (size_t) numTasks);

    if (b_write_fix (cli_nios_fd[0], (char *) &reqHdr, sizeof (reqHdr)) != sizeof (reqHdr)) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }


    if (select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout) <= 0) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_SELECT_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &replyHdr, sizeof (replyHdr))  == -1) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }
    
    switch (replyHdr.opCode) {
        case SYNC_FAIL: {
          lserrno = LSE_SETPARAM;
          sigprocmask (SIG_SETMASK, &oldMask, NULL);
          return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
        break;
        case SYNC_OK: { 
        }
        break;
        default: {
          lserrno = LSE_PROTOC_NIOS;
          sigprocmask (SIG_SETMASK, &oldMask, NULL);
          return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
        break;
    }

    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

int
ls_setstdout (int on_, char *format)
{
    fd_set rmask;
    struct timeval timeout;
    struct lslibNiosStdout req;
    struct lslibNiosHdr replyHdr;
    sigset_t newMask, oldMask;

    if (!nios_ok_) {
        lserrno = LSE_NIOS_DOWN;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask); // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    req.r.set_on = on_;
    req.r.len = (format == NULL) ? 0 : strlen (format);
    if (req.r.len > 0) {
        req.r.len++;
    }

    SET_LSLIB_NIOS_HDR (req.hdr, LIB_NIOS_SETSTDOUT, sizeof (req.r) + req.r.len * sizeof (char));

    if (b_write_fix (cli_nios_fd[0], (char *) &req, sizeof (req.hdr) + sizeof (req.r)) != sizeof (req.hdr) + sizeof (req.r)) { // FIXME FIXME FIXME FIXME what is really being passed with &req?
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (req.r.len <= LONG_MAX) {

        size_t b_write_fix_result = (size_t) b_write_fix( cli_nios_fd[0], format, req.r.len * sizeof( char ) ); // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label

        assert( req.r.len <= LONG_MAX );
        assert( req.r.len <= b_write_fix_result);
        if( b_write_fix_result != ( req.r.len * sizeof( char ) ) ) {
            lserrno = LSE_MSG_SYS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
    }


    if (select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout) <= 0) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_SELECT_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &replyHdr, sizeof (replyHdr)) == -1) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    switch (replyHdr.opCode) {
        case STDOUT_FAIL: {
            lserrno = LSE_SETPARAM;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
        break;
        case STDOUT_OK:
        { ; }
        break;
        default: {
            lserrno = LSE_PROTOC_NIOS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
        break;
    }

    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

int
ls_setstdin (int on_, int *rpidlist, size_t len)
{
    fd_set rmask;
    struct timeval timeout;
    struct lslibNiosStdin req;
    struct lslibNiosHdr replyHdr;
    sigset_t newMask;
    sigset_t oldMask;

    if (!nios_ok_)
    {
        lserrno = LSE_NIOS_DOWN;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

//  FIXME FIXME negative length? 
//      when does this function get a negative length size?
//  if ((rpidlist == NULL && len != 0) || (len < 0) || (len > NOFILE))
    if ((rpidlist == NULL && len != 0) || (len > NOFILE)) {
        lserrno = LSE_SETPARAM;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask);
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    SET_LSLIB_NIOS_HDR (req.hdr, LIB_NIOS_SETSTDIN, sizeof (req.r) + len * sizeof (int));
    req.r.set_on = on_;
    req.r.len = len;

    if (b_write_fix (cli_nios_fd[0], (char *) &req, sizeof (req.hdr) + sizeof (req.r)) != sizeof (req.hdr) + sizeof (req.r)) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (rpidlist != NULL && len != 0) {

        assert( len < LONG_MAX );
        if (b_write_fix (cli_nios_fd[0], (char *) rpidlist, len * sizeof (int)) != (long) (len * sizeof (int)) ) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
            lserrno = LSE_MSG_SYS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
    }


    if (select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout) <= 0) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
      lserrno = LSE_SELECT_SYS;
      sigprocmask (SIG_SETMASK, &oldMask, NULL);
      return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &replyHdr, sizeof (replyHdr)) == -1) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    switch (replyHdr.opCode) {
        case STDIN_FAIL: {
            lserrno = LSE_SETPARAM;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
        case STDIN_OK:
            { ; }
        break;
        default: {
            lserrno = LSE_PROTOC_NIOS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
        break;
    }

    sigprocmask (SIG_SETMASK, &oldMask, NULL);
    return 0;
}

size_t
ls_getstdin (int on_, unsigned long *rtaskidlist, long maxlen)
{
    fd_set rmask;
    struct timeval timeout;
    struct lslibNiosStdin req;
    struct lslibNiosGetStdinReply reply;
    sigset_t newMask, oldMask;
    size_t maxlen_new = 0;

    if (!nios_ok_)
    {
        lserrno = LSE_NIOS_DOWN;
        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (blockALL_SIGS_ (&newMask, &oldMask) < 0) {
        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    FD_ZERO (&rmask);
    FD_SET (cli_nios_fd[0], &rmask); // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
    timeout.tv_sec = NIOS_TIMEOUT;
    timeout.tv_usec = 0;

    SET_LSLIB_NIOS_HDR (req.hdr, LIB_NIOS_GETSTDIN, sizeof (req.r.set_on));
    req.r.set_on = on_;

    if (b_write_fix (cli_nios_fd[0], (char *) &req, sizeof (req.hdr) + sizeof (req.r.set_on)) != sizeof (req.hdr) + sizeof (req.r.set_on)) // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
    {
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }


    if (select (cli_nios_fd[0] + 1, &rmask, 0, 0, &timeout) <= 0) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_SELECT_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    if (b_read_fix (cli_nios_fd[0], (char *) &reply.hdr, sizeof (reply.hdr)) == -1) { // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        lserrno = LSE_MSG_SYS;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    switch (reply.hdr.opCode)
    {
        case STDIN_OK: 
            { ; }
        break;
        default: {
            lserrno = LSE_PROTOC_NIOS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
        break;
    }

    if (reply.hdr.len) {
        assert( b_read_fix (cli_nios_fd[0], (char *) reply.rtaskidlist, reply.hdr.len) >= 0 ); // FIXME FIXME FIXME FIXME find out what cli_nios_fd[0] represents, replace 0 with label
        assert( reply.hdr.len <= LONG_MAX );
        if (b_read_fix (cli_nios_fd[0], (char *) reply.rtaskidlist, reply.hdr.len) != (long) reply.hdr.len) { // FIXME FIXME FIXME FIXME remove the cast
            lserrno = LSE_MSG_SYS;
            sigprocmask (SIG_SETMASK, &oldMask, NULL);
            return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
        }
    }

    assert( maxlen >= 0);
    maxlen_new = (size_t) maxlen;
    if (reply.hdr.len <=  maxlen_new * sizeof (size_t)) { // FIXME FIXME FIXME FIXME what?
        memcpy ((char *) rtaskidlist, (char *) reply.rtaskidlist, reply.hdr.len);
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        assert( reply.hdr.len <= INT_MAX);
        return reply.hdr.len / sizeof (size_t);
    }
    else {
        lserrno = LSE_RPIDLISTLEN;
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value;
    }

    return LONG_MAX;
}
