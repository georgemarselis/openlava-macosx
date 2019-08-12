/* $Id: lib.init.c 397 2007-11-26 19:04:00Z mblack $
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
#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>

#include "lib/res.h"
#include "lib/lib.h"
#include "lib/mls.h"
#include "lib/init.h"
#include "lib/initenv.h"
// // #include "lib/lproto.h"
#include "lib/structs/genParams.h"
#include "lib/queue.h"
#include "lib/syslog.h"
#include "daemons/libniosd/niosd.h"
#include "daemons/liblimd/lim.h"
#include "lib/sock.h"
#include "lib/channel.h"
#include "lib/conn.h"
#include "lib/id.h"

unsigned int
ls_initrex ( unsigned int num, int options)
{
    struct servent *sv = NULL;

    if (geteuid () == 0){
        rootuid_ = TRUE;
    }

    if (initenv_ (NULL, NULL) < 0) {
        if (rootuid_ && !(options & KEEPUID)) {
            lsfSetXUid(0, getuid(), getuid(), INT_MAX, setuid);
        }

        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    inithostsock_ ();
    lsQueueInit_ (&requestQ, lsReqCmp_, NULL);
    if (requestQ == NULL) {
        lserrno = LSE_MALLOC;
        return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    res_addr_.sin_family = AF_INET;

    if (genParams_[LSF_RES_PORT].paramValue) {

        in_port_t envport = (in_port_t) atoi( genParams_[LSF_RES_PORT].paramValue );  // FIXME cast. res_addr_ is of type sockaddres_in, which is a system struct.
        assert( envport > 0 );
        res_addr_.sin_port = envport;
        if ( res_addr_.sin_port ) {
             res_addr_.sin_port = htons (res_addr_.sin_port);
        }
        else {
            goto res_init_fail;
        }
    }
    else if (genParams_[LSF_RES_DEBUG].paramValue) {
        res_addr_.sin_port = htons (LSF_RES_PORT);
    }
    else {

#ifdef _COMPANY_X_
        if ((res_addr_.sin_port = get_port_number (RES_SERVICE, (char *) NULL)) == -1) { // FIXME replace -1 with label
#else
        if ((sv = getservbyname ("res", "tcp")) != NULL) { // FIXME FIXME FIXME what is this "res" and "tcp"
            int kot = sv->s_port;
            assert(  kot <= USHRT_MAX  && kot > 0 );
            res_addr_.sin_port = ( in_port_t ) kot;
        }
        else {
#endif
            res_init_fail:
            lserrno = LSE_RES_NREG;
            
            if (rootuid_ && !(options & KEEPUID)) {
                lsfSetXUid(0, getuid(), getuid(), INT_MAX, setuid);
            }

            return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    initconntbl_ ();
    FD_ZERO (&connection_ok_);

    if ((rootuid_) && (genParams_[LSF_AUTH].paramValue == NULL)) {
        unsigned int i = opensocks_ (num);
            if (!(options & KEEPUID)) {
                lsfSetXUid(0, getuid(), getuid(), INT_MAX, setuid);
            }
            return i;
    }
    else {
        return num;
    }
}

unsigned int
opensocks_ (unsigned int num)
{
    unsigned int nextdescr = 0;
    // int i;

    totsockets_ = (num <= 0 || num > MAXCONNECT) ? LSF_DEFAULT_SOCKS : num; // totsockets_ is in lib/init.h

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG, "%s: try to allocate num <%d> of socks", __func__, num);
    }

    nextdescr = FIRST_RES_SOCK; // NOFIX : cast is perfectly fine here, FIRST_RES_SOCK has a const value of 20, from lsf.h
    for( unsigned int i = 0; i < totsockets_; i++) {
        unsigned int s = (unsigned int) CreateSock_ (SOCK_STREAM); // FIXME FIXME FIXME the cast is probably fine, but might as well take a look later       
        if( UINT_MAX == s ) {
            if (logclass & LC_COMM) {
                ls_syslog (LOG_DEBUG, "%s: CreateSock_ failed, iter:<%d> %s",  __func__, i, strerror (errno));
            }
            totsockets_ = i;
            if (i > 0) {
                break;
            }
            else {
                return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }

        assert( s <= INT_MAX );
        assert( nextdescr <= INT_MAX );
        if (s != nextdescr)
        {
            if( dup2 ( (int) s, (int) nextdescr ) < 0 )   {
                if (logclass & LC_COMM) {
                    ls_syslog (LOG_DEBUG, "%s: dup2() failed, old:<%d>, new<%d>, iter:<%d>  %s", __func__, s, nextdescr, i, strerror (errno));
                }
                close ( (int) s);
                lserrno = LSE_SOCK_SYS;
                totsockets_ = i;
                if (i > 0) {
                    break;
                }
                else {
                    return UINT_MAX; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            }

#if defined(FD_CLOEXEC)
            fcntl ( (int)nextdescr, F_SETFD, (fcntl ( (int) nextdescr, F_GETFD) | FD_CLOEXEC));
#else
#if defined(FIOCLEX)
            // (void) ioctl (nextdescr, FIOCLEX, (char *) NULL);
            ioctl (nextdescr, FIOCLEX, (char *) NULL);
#endif
#endif

            close ((int)s); // NOFIX cast is fine here
        }
        nextdescr++;
    }

    currentsocket_ = FIRST_RES_SOCK;

    if (logclass & LC_COMM) {
        ls_syslog (LOG_DEBUG, "%s: returning num=<%d>", __func__, totsockets_);
    }

    return totsockets_;
}

/* ls_fdbusy()
 */
int
ls_fdbusy (int fd)
{
    struct sTab hashSearchPtr;
    struct hEnt *hEntPtr = 0;
    int foo    = 0;

    assert (fd > 0 );

    foo = ( unsigned long )fd;

    if ( foo == chanSock_ (limchans_[PRIMARY]) || foo == chanSock_ (limchans_[MASTER])  || foo == chanSock_ (limchans_[UNBOUND])) {
        return TRUE;
    }

    if ( fd == cli_nios_fd[0]) { // FIXME FIXME FIXME find out what [0] represents and label it.
        return TRUE;
    }

    hEntPtr = h_firstEnt_ (&conn_table, &hashSearchPtr);
    while (hEntPtr)  {
        int *pfd = NULL;

        pfd = hEntPtr->hData;
        assert( pfd[0] >= 0);
        assert( pfd[1] >= 0 );
        if (fd == pfd[0] || fd == pfd[1])  {
            return TRUE;
        }

        hEntPtr = h_nextEnt_ (&hashSearchPtr);
    }

    assert( fd >= 0 );
    if (rootuid_ && (unsigned int)fd >= currentsocket_ && (unsigned int)fd < FIRST_RES_SOCK +  totsockets_) {
        return TRUE;
    }

    return FALSE;
}

int
lsfSetXUid (int flag, uid_t ruid, uid_t euid, uid_t suid, int (*func) ())
{

    int rtrn = -1;

    assert( flag );
    assert( suid );

    if (func == setuid) {
            rtrn = setuid (ruid);
    }
    else if (func == seteuid) {
            rtrn = seteuid (euid);
    }
    else if (func == setreuid) {
        rtrn = setreuid (ruid, euid);
    }
    else {
            printf( "wtf am i doing here? lsfSetXUid()\n");
    }

    return rtrn;
}

void
lsfExecLog (const char *cmd)
{
    char *lsfUserName   = malloc( sizeof( char ) * MAX_LSF_NAME_LEN + 1 );

    if (genParams_[LSF_MLS_LOG].paramValue && ((genParams_[LSF_MLS_LOG].paramValue[0] == 'y') || (genParams_[LSF_MLS_LOG].paramValue[0] == 'Y'))) {  // FIXME FIXME FIXME find out what [0] represents and label it.

            getLSFUser_ (lsfUserName, sizeof (lsfUserName));
            /* catgets 6259 */
            syslog (LOG_INFO, "catgets 6259: %s: user - %s cmd - '%s'", __func__, lsfUserName, cmd);

    }
    free(  lsfUserName );
    return;
}

int
lsfExecX ( const char *path, char **argv, int (*func) ())
{
    return func (path, argv);
}
