/* $Id: lib.pim.c 397 2007-11-26 19:04:00Z mblack $
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

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "daemons/libpimd/pimd.h"
#include "libint/lsi18n.h"
#include "lib/lib.h"
#include "lib/pim.h"
#include "lib/rdwr.h"
#include "lib/xdrrf.h"
#include "lib/initenv.h"
#include "lib/host.h"
#include "lib/sock.h"
#include "lib/usleep.h"
#include "lib/misc.h" // FIXME FIXME FIXME initLSFHeader_() is here must move to init.h or new file initlsf.h
#include "lsf.h"

void pim_c_bullshit( void )
{
    assert( pimDaemonParams );
    assert( PIM_PARAMS );
    assert( PIM_API );
    assert( INFINIT_LOAD );
    return;
}


struct jRusage *
getJInfo_ (pid_t npgid, pid_t *pgid, unsigned short options, gid_t cpgid)
{
    static char pfile[MAX_FILENAME_LEN];
    static struct sockaddr_in pimAddr;
    static time_t lastTime = 0;
    static time_t lastUpdateNow = 0;
    static time_t pimSleepTime = PIM_SLEEP_TIME;
    static bool_t periodicUpdateOnly = FALSE;
    struct jRusage   *jru = NULL;
    struct LSFHeader sendHdr;
    struct LSFHeader recvHdr;
    struct LSFHeader hdrBuf;
    struct timeval  timeOut;
    char *myHost = NULL;
    time_t now;
    int s = 0;
    int cc = 0;

    now = time (0);

    if (logclass & LC_PIM) {
        ls_syslog (LOG_DEBUG3, "now = %ld, lastTime = %ld, sleepTime = %ld", now, lastUpdateNow, pimSleepTime);
    }
  
    argOptions = options;

    if (lastTime == 0) {
        struct hostent *hp = NULL;
        struct config_param *plp = NULL;

        for (plp = pimParams; plp->paramName != NULL; plp++) {
            if (plp->paramValue != NULL) {
                FREEUP (plp->paramValue);
            }
        }

        if (initenv_ (pimParams, NULL) < 0) {
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: initenv_() failed: %M", __func__);
            }
            
            return NULL;
        }

        if ((myHost = ls_getmyhostname ()) == NULL) {
            
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: ls_getmyhostname() failed: %m", __func__);
            }
            
            return NULL;
        }

        if (pimParams[LSF_PIM_INFODIR].paramValue) {
            sprintf (pfile, "%s/pim.info.%s", pimParams[LSF_PIM_INFODIR].paramValue, myHost); // FIXME FIXME FIXME FIXME FIXME put into configure.ac
        }
        else {
            if (pimParams[LSF_LIM_DEBUG].paramValue) {

                if (pimParams[LSF_LOGDIR].paramValue) {
                    sprintf (pfile, "%s/pim.info.%s", pimParams[LSF_LOGDIR].paramValue, myHost); // FIXME FIXME FIXME FIXME FIXME put into configure.ac
                }
                else {
                    sprintf (pfile, "/tmp/pim.info.%s.%u", myHost, getuid ()); // FIXME FIXME FIXME FIXME FIXME put into configure.ac
                }
            }
            else {
                sprintf (pfile, "/tmp/pim.info.%s", myHost); // FIXME FIXME FIXME FIXME FIXME put into configure.ac
            }
        }

        if (pimParams[PIM_SLEEP_TIME].paramValue) {
            if ((pimSleepTime = atoi (pimParams[PIM_SLEEP_TIME].paramValue)) < 0) {
                if (logclass & LC_PIM) {
                    ls_syslog (LOG_DEBUG, "PIM_SLEEP_TIME value <%s> must be a positive integer, defaulting to %d", pimParams[PIM_SLEEP_TIME].paramValue, PIM_SLEEP_TIME);
                }
          
                pimSleepTime = PIM_SLEEP_TIME;
            }
        }

        if (pimParams[PIM_SLEEPTIME_UPDATE].paramValue != NULL && strcasecmp (pimParams[PIM_SLEEPTIME_UPDATE].paramValue, "y") == 0) {
            
            periodicUpdateOnly = TRUE;
            
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: Only to call pim each PIM_SLEEP_TIME interval", __func__);
            }
        }

        if ((hp = Gethostbyname_ (myHost)) == NULL) {
            return NULL;
        }

        memset ((char *) &pimAddr, 0, sizeof (pimAddr));
        assert( hp->h_length > 0 );
        memcpy ((char *) &pimAddr.sin_addr, (char *) hp->h_addr_list[0], (size_t) hp->h_length); // FIXME FIXME FIXME FIXME hp->h_addr_list[0] put a label and an explaination to the label next to it
        pimAddr.sin_family = AF_INET;
    }


    if (now - lastUpdateNow >= pimSleepTime || (options & PIM_API_UPDATE_NOW)) {
        if (logclass & LC_PIM) {
            ls_syslog (LOG_DEBUG, "%s: update now", __func__);
        }

        lastUpdateNow = now;

        if ((s = TcpCreate_ (FALSE, 0)) < 0) {
            
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: tcpCreate failed: %m", __func__);
            }
            return NULL;
        }

        if (pimPort (&pimAddr, pfile) == -1) {
            close (s);
            return NULL;
        }

        if (b_connect_ (s, (struct sockaddr *) &pimAddr, sizeof (pimAddr), 0) == -1) {
            
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: b_connect() failed: %m", __func__);
            }
            
            lserrno = LSE_CONN_SYS;
            close (s);
            
            return NULL;
        }

        initLSFHeader_ (&sendHdr);
        initLSFHeader_ (&recvHdr);

        sendHdr.opCode = options;
        // sendHdr.refCode = (short) now & 0xffff;
        sendHdr.refCode = now & 0xffff;
        sendHdr.reserved = cpgid;
        cc = writeEncodeHdr_ (s, &sendHdr, b_write_fix);
        if ( cc < 0) {
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: writeEncodeHdr failed cc=%d: %M", __func__, cc);
            }
            
            close (s);
            
            return NULL;
        }

        timeOut.tv_sec = 10;
        timeOut.tv_usec = 0;
        if ((cc = rd_select_ (s, &timeOut)) < 0) {

            if (logclass & LC_PIM)  {
                ls_syslog (LOG_DEBUG, "%s: rd_select_ cc=%d: %m", __func__, cc);
            }
            close (s);
            
            return NULL;
        }

        if ((cc = lsRecvMsg_ (s, (char *) &hdrBuf, sizeof (hdrBuf), &recvHdr, NULL, NULL, &b_read_fix)) < 0) { // FIXME FIXME FIXME FIXME FIXME this is def wrong, especially the sizeof()

            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: lsRecvMsg_ failed cc=%d: %M", __func__, cc);
            }

            close (s);
            return NULL;
        }
        close (s);

        if (recvHdr.refCode != sendHdr.refCode) {
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: recv refCode=%u not equal to send refCode=%u, server is not PIM", __func__,recvHdr.refCode, sendHdr.refCode);
            }

            return NULL;
        }

        if (logclass & LC_PIM) {
            ls_syslog (LOG_DEBUG, "%s updated now", __func__);
        }

        if (!readPIMFile (pfile)) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "readPIMFile");
            return NULL;
        }
    }

    lastTime = now;
    if ((jru = readPIMInfo (npgid, pgid)) == NULL && !(options & PIM_API_UPDATE_NOW) && (periodicUpdateOnly == FALSE || (periodicUpdateOnly == TRUE && now - lastUpdateNow >= pimSleepTime))) {
        if (hitPGid > 0) {
            jru = getJInfo_ (npgid, pgid, options | PIM_API_UPDATE_NOW, hitPGid);
            hitPGid = 0;
            return jru;
        }
        else {
            return getJInfo_(npgid, pgid, options | PIM_API_UPDATE_NOW, cpgid);
        }
    }

    return jru;
}

char *readPIMBuf ( const char *pfile)
{

    struct stat bstat;
    FILE *fp = NULL;

    FREEUP (pimInfoBuf);    // FIXME FIXME FIXME where on earth does this 
    pimInfoLen = 0;

    if (stat (pfile, &bstat) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "stat", pfile);
        return NULL;
    }

    pimInfoLen = (size_t) bstat.st_size; // NOFIX st_size can be negative as an offset, but no implementations exist, as per https://stackoverflow.com/questions/12275831/why-is-the-st-size-field-in-struct-stat-signed
    if ((pimInfoBuf = malloc( pimInfoLen + 1)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return NULL;
    }
    
    if ((fp = openPIMFile (pfile)) == NULL) {

        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "openPIMFile", pfile);
        return FALSE;
    }
    
    if (fread (pimInfoBuf, sizeof (char), (size_t) pimInfoLen, fp) <= 0) {  // FIXME FIXME verify usefulness of cast
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fread");
        FREEUP (pimInfoBuf);
        return NULL;
    }

    fclose (fp);

    pimInfoBuf[pimInfoLen] = '\0';
    return pimInfoBuf;

}

char *getNextString (const char *buf, char *string)
{
    char *tmp_buff = NULL;
    char *tmp_string = NULL;
    int i = 0;

    if ((*buf == EOF) || (*buf == '\0')) {
        return NULL;
    }

    tmp_buff = strdup( buf );
    tmp_string = strdup( string );
    while ((*tmp_buff != EOF) && (*tmp_buff != '\0') && (*tmp_buff != '\n')) {
        tmp_string[i++] = *tmp_buff;
        tmp_buff++;
    }

    tmp_string[i] = '\0';
    string[i] = '\0';
    if (*tmp_buff == '\n') {
        tmp_buff++;
    }

    return tmp_buff;
}

int readPIMFile ( const char *pfile)
{
    struct lsPidInfo *tmp = NULL;
    char *buffer          = NULL;
    char *tmpbuf          = NULL;
    char *pimString       = NULL;
    char piminfo[ ]       = "pim.info"; // FIXME FIXME FIXME FIXME FIXME put into configure.ac

    pimString = malloc( MAX_LINE_LEN * sizeof( *pimString ) );

    if ((buffer = readPIMBuf (pfile)) == NULL) {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "readPIMBuf");
      return FALSE;
    }

    FREEUP (pinfoList);
    npinfoList = 0; // global from lib/pim.h
    pinfoList = malloc (sizeof (struct lsPidInfo) * MAX_NUM_PID); // global from lib/pim.h
    if (pinfoList == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct lsPidInfo) * MAX_NUM_PID);
        return FALSE;
    }

    tmpbuf = getNextString (buffer, pimString);
    if (tmpbuf == NULL) {
        /* catgets 5908 */
        ls_syslog (LOG_ERR, "catgets 5908: %s format error", piminfo );
        return FALSE;
    }
    
    buffer = tmpbuf;

    while ((tmpbuf = getNextString (buffer, pimString)) != NULL) {

        buffer = tmpbuf;
        if (logclass & LC_PIM) {
            ls_syslog (LOG_DEBUG3, "pim info string is %s", pimString); 
        }

        sscanf (pimString, "%d %d %d %lu %ld %ld %ld %ld %lu %lu %lu %u",
            &pinfoList[npinfoList].pid,
            &pinfoList[npinfoList].ppid,
            &pinfoList[npinfoList].pgid,
            &pinfoList[npinfoList].jobid,
            &pinfoList[npinfoList].utime,
            &pinfoList[npinfoList].stime,
            &pinfoList[npinfoList].cutime,
            &pinfoList[npinfoList].cstime,
            &pinfoList[npinfoList].proc_size,
            &pinfoList[npinfoList].resident_size,
            &pinfoList[npinfoList].stack_size,
            &pinfoList[npinfoList].status
        );

        npinfoList++;
        if (npinfoList % MAX_NUM_PID == 0) {
            assert( npinfoList + MAX_NUM_PID > 0 );
            tmp = realloc (pinfoList, sizeof (struct lsPidInfo) * (npinfoList + MAX_NUM_PID));

            if (tmp == NULL) {
                assert( npinfoList + MAX_NUM_PID > 0 );
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "realloc", sizeof (struct lsPidInfo) * (npinfoList + MAX_NUM_PID));
                free( pimString );
                return FALSE;
            }
            pinfoList = tmp;
        }
    }

  return TRUE;
}

struct jRusage *
readPIMInfo ( pid_t inNPGids, pid_t *inPGid)
{
    //int cc = 0;
    //int pinfoNum = 0;
    int found = FALSE;
    int *activeInPGid = NULL;
    struct jRusage *jru = malloc( sizeof( struct jRusage ) );
    
    // FREEUP (pgidList); // SEEME SEEME SEEME why on earth are we using a global object
    // FREEUP (pidList);
    // FREEUP (activeInPGid);

    memset ( jru, '\0', sizeof( struct jRusage ) );
    activeInPGid = malloc( inNPGids * sizeof (int) );
    if (activeInPGid == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", (unsigned long)inNPGids * sizeof (int));
        return NULL;
    }

    memset ( activeInPGid, '\0', inNPGids * sizeof (int));

    pgidList = malloc( ( inNPGids < PGID_LIST_SIZE ? PGID_LIST_SIZE : (inNPGids + PGID_LIST_SIZE)) * sizeof (int));
    if (pgidList == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", (inNPGids < PGID_LIST_SIZE ? PGID_LIST_SIZE : (unsigned long)(inNPGids + PGID_LIST_SIZE)) * sizeof (int));
        return NULL;
    }

    npgidList = inNPGids;
    assert( npgidList >= 0 );
    memcpy ( pgidList, inPGid, (unsigned long)npgidList * sizeof (int));

    pidList = (struct pidInfo *) malloc (PID_LIST_SIZE * sizeof (struct pidInfo));

    if (pidList == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", PID_LIST_SIZE * sizeof (struct pidInfo));
        return NULL;
    }
    npidList = 0;

    for ( unsigned int pinfoNum = 0; pinfoNum < npinfoList; pinfoNum++)
    {
        int cc = 0;

        if (argOptions & PIM_API_TREAT_JID_AS_PGID) { // SEEME SEEME SEEME search for "PIM_API_TREAT_JID_AS_PGID"
            pinfoList[pinfoNum].pgid = pinfoList[pinfoNum].jobid;
        }

        for ( int i = 0; i < inNPGids; i++) {
            
            if (pinfoList[pinfoNum].pgid == inPGid[i]) {
                activeInPGid[i] = TRUE;
            }
        }

        cc = inAddPList (pinfoList + pinfoNum);
        if ( 1 == cc ) {

            if (pinfoList[pinfoNum].status != LS_PSTAT_ZOMBI && pinfoList[pinfoNum].status != LS_PSTAT_EXITING) {

                if (pinfoList[pinfoNum].stack_size == UINT_MAX ) {
                    assert( pinfoList[pinfoNum].pgid >= 0 );
                    hitPGid = (unsigned int) pinfoList[pinfoNum].pgid;
                    found = FALSE;
                    break;
                }

                jru->mem += pinfoList[pinfoNum].resident_size;
                jru->swap += pinfoList[pinfoNum].proc_size;
                jru->utime += pinfoList[pinfoNum].utime;
                jru->stime += pinfoList[pinfoNum].stime;

                if (logclass & LC_PIM) {
                    ls_syslog (LOG_DEBUG,
                        "%s: Got pid=%d ppid=%d pgid=%d utime=%d stime=%d cutime=%d cstime=%d proc_size=%d resident_size=%d stack_size=%d status=%d",
                        __func__,
                        pinfoList[pinfoNum].pid,
                        pinfoList[pinfoNum].ppid,
                        pinfoList[pinfoNum].pgid,
                        pinfoList[pinfoNum].utime,
                        pinfoList[pinfoNum].stime,
                        pinfoList[pinfoNum].cutime,
                        pinfoList[pinfoNum].cstime,
                        pinfoList[pinfoNum].proc_size,
                        pinfoList[pinfoNum].resident_size,
                        pinfoList[pinfoNum].stack_size,
                        pinfoList[pinfoNum].status
                    );
                }

                if (pinfoList[pinfoNum].pid > -1) {
                    found = TRUE;
                }
            }
        }
        else {
            if (cc == -1) {
                return NULL;
            }
        }
    }

    if (found) {
        
        unsigned int n = inNPGids; // FIXME FIXME FIXME this should be pid_t
        for ( unsigned int i = 0; i < n; i++) {

            if (!activeInPGid[i]) {
                npgidList--;

                for ( unsigned int j = i; j < npgidList; j++) {
                    pgidList[j] = pgidList[j + 1];
                }

                n--;
                for ( unsigned int j = i; j < n; j++) {
                    activeInPGid[j] = activeInPGid[j + 1];
                }
        }
    }

        jru->npids = npidList;
        jru->pidInfo = pidList;
        jru->npgids = npgidList;
        jru->pgid = pgidList;

        return jru;
    }

    return NULL;
}

int inAddPList (struct lsPidInfo *pinfo)
{

    for ( int i = 0; i < npgidList; i++) {

        if (pinfo->pgid == pgidList[i]) {
            if (pinfo->pid > -1) {
                if (intoPidList (pinfo) == -1) {
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            }

            return 1;
        }
    }

    if (pinfo->pid < 0) {
        return 0;
    }

    for ( unsigned int i = 0; i < npidList; i++)
    {
        if (pinfo->ppid == pidList[i].pid)
        {
            pgidList[npgidList] = pinfo->pgid;
            npgidList++;

            if (npgidList % PGID_LIST_SIZE == 0)
            {
                int *tmpPtr = 0;
                // assert( npgidList + PGID_LIST_SIZE >= 0 );
                tmpPtr = realloc ( pgidList, (npgidList + PGID_LIST_SIZE) * sizeof (int));
                
                if ( NULL == tmpPtr && ENOMEM == errno ) { 
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "inAddPList", "realloc", (npgidList + PGID_LIST_SIZE) * sizeof (int));
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            
                pgidList = tmpPtr;
            }

            if (intoPidList (pinfo) == -1) {
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }

            return 1;
        }
    }

    return 0;
}


int intoPidList (struct lsPidInfo *pinfo)
{
    pidList[npidList].pid = pinfo->pid;
    pidList[npidList].ppid = pinfo->ppid;
    pidList[npidList].pgid = pinfo->pgid;
    pidList[npidList].jobid = pinfo->jobid;

    npidList++;

    if (npidList % PID_LIST_SIZE == 0) {
        struct pidInfo *tmpPtr = NULL;

        tmpPtr = realloc ( pidList, (npidList + PID_LIST_SIZE) * sizeof( struct pidInfo ) );
        if ( NULL == tmpPtr && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "intoPidList", "realloc", (unsigned long)(npidList + PID_LIST_SIZE) * sizeof (struct pidInfo));
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

      pidList = tmpPtr;
    }

  return 0;
}

int pimPort (struct sockaddr_in *pimAddr, const char *pfile)
{
    FILE *fp = NULL;
    uint16_t port = 0;

    if ((fp = openPIMFile (pfile)) == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    fscanf (fp, "%hu", &port);
    fclose (fp);
    pimAddr->sin_port = htons (port);

    return 0;
}


FILE *openPIMFile ( const char *pfile)
{
    FILE *fp  = NULL;
    if ((fp = fopen (pfile, "r")) == NULL) {
        millisleep_ (1000);
        if ((fp = fopen (pfile, "r")) == NULL) {
            if (logclass & LC_PIM) {
                ls_syslog (LOG_DEBUG, "%s: fopen(%s) failed: %m", __func__, pfile);
            }

            lserrno = LSE_FILE_SYS;
            return NULL;
        }
    }

    return fp;
}
