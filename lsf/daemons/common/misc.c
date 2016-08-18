/* $Id: misc.c 397 2007-11-26 19:04:00Z mblack $
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
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>

#include "daemons/daemonout.h"
#include "daemons/daemons.h"
#include "libint/lsi18n.h"
#include "lib/osal.h"
#include "lib/mls.h"

#ifndef strchr
#include <string.h>
#endif

# include <stdarg.h>

// #define NL_SETN         10


#define BATCH_SLAVE_PORT        40001

static int chuserId (uid_t);

extern struct listEntry *mkListHeader (void);
extern int shutdown (int, int);

void die (int sig) __attribute__((noreturn));

void
die (int sig)
{
    static char fname[] = "die";
    char *myhost = malloc( sizeof( char ) * MAXHOSTNAMELEN + 1 );

    if (debug > 1) {
        fprintf (stderr, "%s: signal %d\n", fname, sig);
    }

    if (masterme)
        {
        releaseElogLock ();
        }

    if (gethostname (myhost, MAXHOSTNAMELEN) < 0)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "gethostname", myhost);
        strcpy (myhost, "localhost");
        }

    if (sig > 0 && sig < 100)
        {
        /* catgets 8216 */
        ls_syslog (LOG_ERR, "catgets 8216: Daemon on host <%s> received signal <%d>; exiting", myhost, sig);
        }
    else
        {
        switch (sig)
            {
                case MASTER_RESIGN:
                /* catgets 8272 */
                ls_syslog (LOG_INFO, "catgets 8272: Master daemon on host <%s> resigned; exiting", myhost);
                break;
                case MASTER_RECONFIG:
                /* catgets 8273 */
                ls_syslog (LOG_INFO, "catgets 8273: Master daemon on host <%s> exiting for reconfiguration", myhost);
                break;

                case SLAVE_MEM:
                /* catgets 8217 */
                ls_syslog (LOG_ERR, "catgets 8217: Slave daemon on host <%s> failed in memory allocation; fatal error - exiting", myhost);
                lsb_merr1 ("catgets 8217: Slave daemon on host <%s> failed in memory allocation; fatal error - exiting", myhost);
                break;
                case MASTER_MEM:
                /* catgets 8218 */
                ls_syslog (LOG_ERR, "catgets 8218: Master daemon on host <%s> failed in memory allocation; fatal error - exiting", myhost);   
                break;
                case SLAVE_FATAL:
                /* catgets 8219 */
                ls_syslog (LOG_ERR, "catgets 8219: Slave daemon on host <%s> dying; fatal error - see above messages for reason", myhost);
                break;
                case MASTER_FATAL:
                /* catgets 8220 */
                ls_syslog (LOG_ERR, "catgets 8220: Master daemon on host <%s> dying; fatal error - see above messages for reason", myhost);   
                break;
                case MASTER_CONF:
                /* catgets 8221 */
                ls_syslog (LOG_ERR, "catgets 8221: Master daemon on host <%s> died of bad configuration file", myhost);   
                break;
                case SLAVE_RESTART:
                /* catgets 8222 */
                ls_syslog (LOG_ERR, "catgets 8222: Slave daemon on host <%s> restarting", myhost);
                break;
                case SLAVE_SHUTDOWN:
                /* catgets 8223 */
                ls_syslog (LOG_ERR, "catgets 8223: Slave daemon on host <%s> shutdown", myhost);  
                break;
                default:
                /* catgets 8224 */
                ls_syslog (LOG_ERR, "catgets 8224: Daemon on host <%s> exiting; cause code <%d> unknown", myhost, sig);
                break;
            }
        }

    assert( batchSock <= INT_MAX );
    shutdown ((int)chanSock_ ( (uint)batchSock), 2);

    exit (sig);

}

int
portok (struct sockaddr_in *from)
{
    static char fname[] = "portok";
    if (from->sin_family != AF_INET) {
        ls_syslog (LOG_ERR, "%s: sin_family(%d) != AF_INET(%d)", fname, from->sin_family, AF_INET);
        return FALSE;
    }

    if (debug) {
        return TRUE;
    }

    if (ntohs (from->sin_port) >= IPPORT_RESERVED || ntohs (from->sin_port) < IPPORT_RESERVED / 2) {
        return FALSE;
    }

    return TRUE;
}

int
get_ports (void)
{

    static char fname[] = "get_ports";
    struct servent *sv;

    if (daemonParams[LSB_MBD_PORT].paramValue != NULL)
        {
        assert( atoi (daemonParams[LSB_MBD_PORT].paramValue) <= USHRT_MAX );
        if (!isint_ (daemonParams[LSB_MBD_PORT].paramValue) || (mbd_port = (ushort) atoi (daemonParams[LSB_MBD_PORT].paramValue)) <= 0)
            /* catgets 8226 */
            ls_syslog (LOG_ERR, "catgets 8226: %s: LSB_MBD_PORT <%s> in lsf.conf must be a positive number", fname, daemonParams[LSB_MBD_PORT].paramValue);
        else {
            mbd_port = htons (mbd_port);
        }
    }
    else if (debug) {
        mbd_port = htons (BATCH_MASTER_PORT);
    }
    else
        {
        sv = getservbyname (MBATCHD_SERV, "tcp");
        if (!sv)
            {
            /* catgets 8227 */
            ls_syslog (LOG_ERR, "catgets 8227: %s: %s service not registered", fname, MBATCHD_SERV);
            // lsb_merr ("catgets 3208: %s: %s service not registered", fname, MBATCHD_SERV);
            return (-1);
            }
        assert ( sv->s_port <= USHRT_MAX );
        mbd_port = (ushort) sv->s_port;
        }
    if (daemonParams[LSB_SBD_PORT].paramValue != NULL)
        {
        assert( atoi (daemonParams[LSB_SBD_PORT].paramValue) <= USHRT_MAX );
        if (!isint_ (daemonParams[LSB_SBD_PORT].paramValue) || (sbd_port = (ushort) atoi (daemonParams[LSB_SBD_PORT].paramValue)) <= 0)
            /* catgets 8229 */
            ls_syslog (LOG_ERR, "catgets 8229: %s: LSB_SBD_PORT <%s> in lsf.conf must be a positive number", fname, daemonParams[LSB_SBD_PORT].paramValue);
        else {
            sbd_port = htons (sbd_port);
        }
    }
    else if (debug) {
        sbd_port = htons (BATCH_SLAVE_PORT);
    }
    else {
        sv = getservbyname (SBATCHD_SERV, "tcp");
        if (!sv) {
            /* catgets 8231 */
            ls_syslog (LOG_ERR, "catgets 8231: %s: %s service not registered", fname, SBATCHD_SERV);
            //lsb_merr ( "catgets 3208: %s: %s service not registered", fname, SBATCHD_SERV);
            return (-1);
        }
        assert( sv->s_port <= USHRT_MAX );
        sbd_port = (ushort) sv->s_port;
    }

    return (0);
}

uid_t
chuser (uid_t uid)
{
    uid_t myuid;
    int errnoSv = errno;

    if (debug) {
        return (geteuid ());
    }

    if ((myuid = geteuid ()) == uid) {
        return (myuid);
    }

    if (myuid != 0 && uid != 0) {
        chuserId (0);
    }
    chuserId (uid);
    errno = errnoSv;
    return (myuid);
}

static int
chuserId (uid_t uid)
{
    static char fname[] = "chuserId";

    if (lsfSetXUid(0, 1, uid, -1, seteuid) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "setresuid/seteuid", (int) uid);
        if (lsb_CheckMode) {
            lsb_CheckError = FATAL_ERR;
            return -1;
        }
        else {
            die (MASTER_FATAL);
        }
    }

    if (uid == 0) {
        if (lsfSetXUid(0, 0, 0, -1, setreuid)< 0)
            {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "setresuid/setreuid", (int) uid);
            if (lsb_CheckMode)
                {
                lsb_CheckError = FATAL_ERR;
                return -1;
                }
            else if (masterme) {
                die (MASTER_FATAL);
            }
            else {
                die (SLAVE_FATAL);
            }
        }
    }
    return 0;
}

char *
safeSave (char *str)
{
    char *sp;
    char temp[256];

    sp = putstr_ (str);
    if (!sp)
        {
        sprintf (temp, "catgets 1: %s: %s() failed.", "safeSave", "malloc");
        lsb_merr (temp);
        if (masterme) {
            die (MASTER_MEM);
        }
        else {
            die (SLAVE_MEM);
        }
    }

    return sp;

}

/* my_malloc()
 */
void *
my_malloc (size_t len, const char *s)
{
    assert( *s );
    return (malloc (len));

}

/* my_calloc()
 */
void *
my_calloc (size_t nelem, size_t esize, const char *caller)
{
    void *p;

    p = calloc (nelem, esize);
    if (!p) {
        ls_syslog (LOG_ERR, "%s: failed %m %s", __func__, (caller ? caller : "unknown"));
    }

    return p;
}

void
daemon_doinit (void)
{

    if (!daemonParams[LSB_CONFDIR].paramValue || !daemonParams[LSF_SERVERDIR].paramValue || !daemonParams[LSB_SHAREDIR].paramValue)
        {
        /* catgets 8239 */
        ls_syslog (LOG_ERR, "catgets: 8239: One or more of the following parameters undefined: %s %s %s", daemonParams[LSB_CONFDIR].paramName, daemonParams[LSF_SERVERDIR].paramName, daemonParams[LSB_SHAREDIR].paramName);
        if (masterme) {
            die (MASTER_FATAL);
        }
        else {
            die (SLAVE_FATAL);
        }
    }



    if (daemonParams[LSB_MAILTO].paramValue == NULL) {
        daemonParams[LSB_MAILTO].paramValue = safeSave (DEFAULT_MAILTO);
    }

    if (daemonParams[LSB_MAILPROG].paramValue == NULL) {
        daemonParams[LSB_MAILPROG].paramValue = safeSave (DEFAULT_MAILPROG);
    }

    if (daemonParams[LSB_CRDIR].paramValue == NULL) {
        daemonParams[LSB_CRDIR].paramValue = safeSave (DEFAULT_CRDIR);
    }

}


void
relife (void)
{
    pid_t pid;
    char *margv[6];
    uint i = 0;

    pid = fork ();

    if (pid < 0) {
        return;
    }

    if (pid == 0)
        {
        sigset_t newmask;

        for ( int j = 0; j < NOFILE; j++) {
            close (j);
        }
        millisleep_ (3000);

        margv[0] = getDaemonPath_ ("/sbatchd", daemonParams[LSF_SERVERDIR].paramValue);

        i = 1;
        if (debug)
            {
            margv[i] = my_malloc (MAXFILENAMELEN, "relife");
            sprintf (margv[i], "-%d", debug);
            i++;
            }
        if (env_dir != NULL)
            {
            margv[i] = "-d";
            i++;
            margv[i] = env_dir;
            i++;
            }
        margv[i] = NULL;
        sigemptyset (&newmask);
        sigprocmask (SIG_SETMASK, &newmask, NULL);
        /* clear signal mask */


        execve (margv[0], margv, environ);
        /* catgets 8241 */
        ls_syslog (LOG_ERR, "catgets 8241: Cannot re-execute sbatchd: %m");
        /* catgets 3211 */
        lsb_mperr ("catgets 3211: sbatchd died in an accident, failed in re-execute");
        exit (-1);
        }

    die (SLAVE_RESTART);
}


struct listEntry *
tmpListHeader (struct listEntry *listHeader)
{
    static struct listEntry *tmp = NULL;

    if (tmp == NULL) {
        tmp = mkListHeader ();
    }

    tmp->forw = listHeader->forw;
    tmp->back = listHeader->back;
    listHeader->forw->back = tmp;
    listHeader->back->forw = tmp;
    listHeader->forw = listHeader;
    listHeader->back = listHeader;
    return tmp;

}


int
fileExist (char *file, uid_t uid, struct hostent *hp)
{
    static char fname[] = "fileExist";
    pid_t pid;
    int fds[2] = { 0, 0 };
    int answer = 0;
    int fd = 0;

    if (socketpair (AF_UNIX, SOCK_STREAM, 0, fds) < 0)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "socketpair");
        return TRUE;
        }

    pid = fork ();
    if (pid < 0)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "fork");
        return TRUE;
        }

    if (pid > 0)
        {
        close (fds[1]); // FIXME FIXME replace subscript 0 or 1 with appropriate const int
        // FIXME FIXME FIXME find altenrative mechanism to notify user of b_read_fix failing
        // if (b_read_fix (fds[0], &answer, sizeof (int)) < 0)
        //     {
        //     ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "read");
        //     answer = TRUE;
        //     }
        close (fds[0]);
        return answer;
        }
    else
        {
        close (fds[0]);
        if (lsfSetXUid(0, uid, uid, -1, setuid) < 0)
            {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "setuid", uid);
            answer = TRUE;
            write (fds[1], (char *) &answer, sizeof (int));
            close (fds[1]);
            exit (0);
            }
        if ((fd = myopen_ (file, O_RDONLY, 0, hp)) < 0) {
            ls_syslog (LOG_INFO, I18N_FUNC_S_FAIL_M, "fileExist", "myopen_", file);
            answer = FALSE;
        }
        else
            {
            close (fd);
            answer = TRUE;
            }
        write (fds[1], &answer, sizeof (int));
        close (fds[1]);
        exit (0);
        }

}

void
freeWeek (windows_t * week[])
{
    windows_t *wp, *wpp;

    for ( int j = 0; j < 8; j++)
        {
        for (wp = week[j]; wp; wp = wpp)
            {
            wpp = wp->nextwind;
            if (wp)
                free (wp);
            }
        week[j] = NULL;
        }

}

void
errorBack (uint chan, ushort replyCode, struct sockaddr_in *from)
{
    static char fname[] = "errorBack";
    struct LSFHeader replyHdr;
    XDR xdrs;
    char errBuf[MSGSIZE / 8];

    xdrmem_create (&xdrs, errBuf, MSGSIZE / 8, XDR_ENCODE);
    initLSFHeader_ (&replyHdr);
    replyHdr.opCode = replyCode;
    io_block_ ((int)chanSock_ (chan));
    if (xdr_encodeMsg (&xdrs, NULL, &replyHdr, NULL, 0, NULL)) {
        assert( chan <= INT_MAX );
        if (chanWrite_ ((int)chan, errBuf, XDR_GETPOS (&xdrs)) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "chanWrite_", sockAdd2Str_ (from));
        }
    }
    else {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "xdr_encodeMsg", sockAdd2Str_ (from));
    }

    xdr_destroy (&xdrs);
    return;

}

void
scaleByFactor (uint *h32, uint *l32, float cpuFactor)
{
    double limit = 0;
    double tmp   = 0;

    if (*h32 == 0x7fffffff && *l32 == 0xffffffff) {
        return;
    }

    limit = *h32;
    limit *= (1 << 16);
    limit *= (1 << 16);
    limit += *l32;

    limit = limit / cpuFactor + 0.5;
    if (limit < 1.0) {
        limit = 1.0;
    }

    tmp = limit / (double) (1 << 16);
    tmp = tmp / (double) (1 << 16);
    *h32 = (uint)tmp;
    tmp = (double) (*h32) * (double) (1 << 16);
    tmp *= (double) (1 << 16);
    *l32 = (uint)(limit - tmp);

    return;
}

struct tclLsInfo *
getTclLsInfo (void)
{
    static char fname[] = "getTclLsInfo";

    if (tclLsInfo) {
        freeTclLsInfo (tclLsInfo, 0);
    }

    tclLsInfo = my_malloc (sizeof (struct tclLsInfo), fname);
    tclLsInfo->numIndx = allLsInfo->numIndx;
    tclLsInfo->indexNames = my_malloc (allLsInfo->numIndx * sizeof (char *), fname);
    for ( uint resNo = 0; resNo < allLsInfo->numIndx; resNo++) {
        tclLsInfo->indexNames[resNo] = allLsInfo->resTable[resNo].name;
    }

    tclLsInfo->nRes = 0;
    tclLsInfo->resName           = my_malloc (allLsInfo->nRes * sizeof (char *), fname);
    tclLsInfo->stringResBitMaps  = my_malloc (GET_INTNUM (allLsInfo->nRes) * sizeof (int), fname);
    tclLsInfo->numericResBitMaps = my_malloc (GET_INTNUM (allLsInfo->nRes) * sizeof (int), fname);

    for ( uint i = 0; i < GET_INTNUM (allLsInfo->nRes); i++)
        {
        tclLsInfo->stringResBitMaps[i] = 0;
        tclLsInfo->numericResBitMaps[i] = 0;
        }
    for ( uint resNo = 0; resNo < allLsInfo->nRes; resNo++)
        {

        if ((allLsInfo->resTable[resNo].flags & RESF_BUILTIN) || ((allLsInfo->resTable[resNo].flags & RESF_DYNAMIC) && (allLsInfo->resTable[resNo].flags & RESF_GLOBAL))) {
            continue;
        }

        if (allLsInfo->resTable[resNo].valueType == LS_STRING) {
            SET_BIT (tclLsInfo->nRes, tclLsInfo->stringResBitMaps);
        }
        if (allLsInfo->resTable[resNo].valueType == LS_NUMERIC) {
            SET_BIT (tclLsInfo->nRes, tclLsInfo->numericResBitMaps);
        }
        tclLsInfo->resName[tclLsInfo->nRes++] = allLsInfo->resTable[resNo].name;
    }

    return (tclLsInfo);

}

struct resVal *
checkThresholdCond (char *resReq)
{
    static char fname[] = "checkThresholdCond";
    struct resVal *resValPtr;

    resValPtr = (struct resVal *) my_malloc (sizeof (struct resVal), "checkThresholdCond");
    initResVal (resValPtr);
    if (parseResReq (resReq, resValPtr, allLsInfo, PR_SELECT) != PARSE_OK) {

        lsbFreeResVal (&resValPtr);
        if (logclass & (LC_EXEC) && resReq) {
            ls_syslog (LOG_DEBUG1, "%s: parseResReq(%s) failed", fname, resReq);
        }
        return (NULL);
    }
    return (resValPtr);

}

uint *
getResMaps (uint nRes, char **resource)
{
    uint  *temp = 0;

    temp = (uint *) my_malloc (GET_INTNUM (allLsInfo->nRes) * sizeof (uint), "getResMaps");
    for ( uint i = 0; i < GET_INTNUM (allLsInfo->nRes); i++) {
        temp[i] = 0;
    }

    for ( uint i = 0; i < nRes; i++) {

        uint resNo = 0;
        for (resNo = 0; resNo < tclLsInfo->nRes; resNo++) {
            if (!strcmp (resource[i], tclLsInfo->resName[resNo])) {
                break;
            }
        }
        if (resNo < allLsInfo->nRes) {
            SET_BIT (resNo, temp);
        }
    }
    return (temp);

}


int
checkResumeByLoad (LS_LONG_INT jobId, int num, struct thresholds thresholds, struct hostLoad *loads, uint *reason, uint *subreasons, int jAttrib, struct resVal *resumeCondVal, struct tclHostData *tclHostData)
{
    static char fname[] = "checkResumeByLoad";
    int resume = TRUE;
    uint lastReason = *reason;

    if (logclass & (LC_SCHED | LC_EXEC)) {
        ls_syslog (LOG_DEBUG3, "%s: reason=%x, subreasons=%d, numHosts=%d", fname, *reason, *subreasons, thresholds.nThresholds);
    }

    if (num <= 0) {
        return FALSE;
    }

    for ( int j = 0; j < num; j++)
        {
        if (loads[j].loadIndex == NULL) {
            continue;
        }

        if (((*reason & SUSP_PG_IT) || ((*reason & SUSP_LOAD_REASON) && (*subreasons) == PG)) && loads[j].loadIndex[IT] < pgSuspIdleT / 60 && fabs( INFINIT_LOAD - thresholds.loadSched[j][PG] ) < 0.0000001 )
            {
            resume = FALSE;
            *reason = SUSP_PG_IT;
            *subreasons = 0;
            }
        else if (LS_ISUNAVAIL (loads[j].status))
            {
            resume = FALSE;
            *reason = SUSP_LOAD_UNAVAIL;
            }
        else if (LS_ISLOCKEDU (loads[j].status) && !(jAttrib & Q_ATTRIB_EXCLUSIVE))
            {
            resume = FALSE;
            *reason = SUSP_HOST_LOCK;
            }
        else if (LS_ISLOCKEDM (loads[j].status))
            {
            resume = FALSE;
            *reason = SUSP_HOST_LOCK_MASTER;
            }

        if (!resume) {
            if (logclass & (LC_SCHED | LC_EXEC)) {
                ls_syslog (LOG_DEBUG2, "%s: Can't resume job %s; *reason=%x", fname, lsb_jobid2str (jobId), *reason);
            }
            if (lastReason & SUSP_MBD_LOCK) {
                *reason |= SUSP_MBD_LOCK;
            }
            return FALSE;
        }



        if (resumeCondVal != NULL)
            {
            if (evalResReq (resumeCondVal->selectStr, &tclHostData[j], DFT_FROMTYPE) == 1)
                {
                resume = TRUE;
                break;
                }
            else
                {
                resume = FALSE;
                *reason = SUSP_QUE_RESUME_COND;
                if ((logclass & (LC_SCHED | LC_EXEC)) && !resume) {
                    ls_syslog (LOG_DEBUG2, "%s: Can't resume job %s; reason=%x", fname, lsb_jobid2str (jobId), *reason);
                }
                if (lastReason & SUSP_MBD_LOCK) {
                    *reason |= SUSP_MBD_LOCK;
                }
                return FALSE;
                }
            }


        if (loads[j].loadIndex[R15M] > thresholds.loadSched[j][R15M])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = R15M;
            }
        else if (loads[j].loadIndex[R1M] > thresholds.loadSched[j][R1M])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = R1M;
            }
        else if (loads[j].loadIndex[R15S] > thresholds.loadSched[j][R15S])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = R15S;
            }
        else if (loads[j].loadIndex[UT] > thresholds.loadSched[j][UT])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = UT;
            }
        else if (loads[j].loadIndex[PG] > thresholds.loadSched[j][PG])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = PG;
            }
        else if (loads[j].loadIndex[IO] > thresholds.loadSched[j][IO])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = IO;
            }
        else if (loads[j].loadIndex[LS] > thresholds.loadSched[j][LS])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = LS;
            }
        else if (loads[j].loadIndex[IT] < thresholds.loadSched[j][IT])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = IT;
            }
        else if (loads[j].loadIndex[MEM] < thresholds.loadSched[j][MEM])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = MEM;
            }
        
        else if (loads[j].loadIndex[TMP] < thresholds.loadSched[j][TMP])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = TMP;
            }
        else if (loads[j].loadIndex[SWP] < thresholds.loadSched[j][SWP])
            {
            resume = FALSE;
            *reason = SUSP_LOAD_REASON;
            *subreasons = SWP;
            }
        for ( uint i = MEM + 1; resume && i < MIN (thresholds.nIdx, allLsInfo->numIndx); i++ )
            {
            if (loads[j].loadIndex[i] >= INFINIT_LOAD || loads[j].loadIndex[i] <= -INFINIT_LOAD || thresholds.loadSched[j][i] >= INFINIT_LOAD || thresholds.loadSched[j][i] <= -INFINIT_LOAD)
            {
                continue;
            }
            
            if (allLsInfo->resTable[i].orderType == INCR)
                {
                if (loads[j].loadIndex[i] > thresholds.loadSched[j][i])
                    {
                    resume = FALSE;
                    *reason = SUSP_LOAD_REASON;
                    *subreasons = i;
                    }
                }
            else
                {
                if (loads[j].loadIndex[i] < thresholds.loadSched[j][i])
                    {
                    resume = FALSE;
                    *reason = SUSP_LOAD_REASON;
                    *subreasons = i;
                    }
                }
            }
        }

    if (lastReason & SUSP_MBD_LOCK) {
        *reason |= SUSP_MBD_LOCK;
    }

    if ((logclass & (LC_SCHED | LC_EXEC)) && !resume) {
        ls_syslog (LOG_DEBUG2, "%s: Can't resume job %s; reason=%x, subreasons=%d", fname, lsb_jobid2str (jobId), *reason, *subreasons);
    }

    return (resume);
}

void
closeExceptFD (int except_)
{
    for ( long i = sysconf (_SC_OPEN_MAX) - 1; i >= 3; i--) {
        if (i != except_) {
            assert( i <= INT_MAX );
            close ( (int) i);
        }
    }
}

void
freeLsfHostInfo (struct hostInfo *hostInfo, int num)
{
    if (hostInfo == NULL || num < 0) {
        return;
    }
    
    for ( int i = 0; i < num; i++)
        {
        if (hostInfo[i].resources != NULL)
            {
            for ( uint j = 0; j < hostInfo[i].nRes; j++) {
                FREEUP (hostInfo[i].resources[j]);
            }
            FREEUP (hostInfo[i].resources);
        }
        FREEUP (hostInfo[i].hostType);
        FREEUP (hostInfo[i].hostModel);
    }
    
}

void
copyLsfHostInfo (struct hostInfo *to, struct hostInfo *from)
{

    strcpy (to->hostName, from->hostName);
    to->hostType = safeSave (from->hostType);
    to->hostModel = safeSave (from->hostModel);
    to->cpuFactor = from->cpuFactor;
    to->maxCpus = from->maxCpus;
    to->maxMem = from->maxMem;
    to->maxSwap = from->maxSwap;
    to->maxTmp = from->maxTmp;
    to->nDisks = from->nDisks;
    to->nRes = from->nRes;
    if (from->nRes > 0) {
        to->resources = (char **) my_malloc (from->nRes * sizeof (char *), "copyLsfHostInfo");
        for ( uint i = 0; i < from->nRes; i++) {
            to->resources[i] = safeSave (from->resources[i]);
        }
    }
    else {
        to->resources = NULL;
    }
    
    to->isServer = from->isServer;
    to->rexPriority = from->rexPriority;
    
}

void
freeTclHostData (struct tclHostData *tclHostData)
{
    
    if (tclHostData == NULL) {
        return;
    }
    FREEUP (tclHostData->resPairs);
    FREEUP (tclHostData->loadIndex);
    
}

void
lsbFreeResVal (struct resVal **resVal)
{
    
    if (resVal == NULL || *resVal == NULL) {
        return;
    }
    freeResVal (*resVal);
    FREEUP (*resVal);
}

void
doDaemonHang (char *caller)
{
    char fname[] = "doDaemonHang()";
    struct timeval timeval;
    bool_t hanging = TRUE;
    
    while (hanging) {
        timeval.tv_sec = 20;
        timeval.tv_usec = 0;
        /* catgets 8271 */
        ls_syslog (LOG_ERR, "catgets 8271: %s hanging in %s", fname, caller);
        select (0, NULL, NULL, NULL, &timeval);
    }
}
