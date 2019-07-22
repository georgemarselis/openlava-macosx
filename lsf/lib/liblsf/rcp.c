/*
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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "lib/dir.h"
#include "lib/rf.h"
#include "lib/whathost.h"
#include "lib/confmisc.h"
#include "lib/info.h"
#include "lib/host.h"
#include "libint/lsi18n.h"
#include "lib/syslog.h"
#include "lib/misc.h"
#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/rcp.h"
#include "lib/id.h"
#include "lsf.h"

// #define NL_SETN   23


// int doXferRcp (struct rcpXfer * lsXfer, int option);

int
parseXferArg (char *arg, char **userName, char **hostName, char **fName)
{
    char *tmp_arg    = NULL;
    char *tmp_ptr    = NULL;
    char *user_arg   = NULL;
    char *host_arg   = NULL;
    char *freeup_tmp = NULL;
    char szOfficialName[MAXHOSTNAMELEN];

    memset( szOfficialName, '\0', MAXHOSTNAMELEN );
    freeup_tmp = tmp_arg = putstr_ (arg);

    tmp_ptr = strchr (tmp_arg, '@');

    if (tmp_ptr) {
        *tmp_ptr = '\0';
        user_arg = tmp_arg;
        tmp_arg = ++tmp_ptr;
    }

    if (!tmp_ptr || *user_arg == '\0') {
        char lsfUserName[MAX_LSF_NAME_LEN];
        if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) != 0) {
            free (freeup_tmp);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        *userName = putstr_ (lsfUserName);
    }
    else {
        *userName = putstr_ (user_arg);
    }

    tmp_ptr = strchr (tmp_arg, ':');

    if (tmp_ptr) {
        *tmp_ptr = '\0';
        host_arg = tmp_arg;
        tmp_arg = ++tmp_ptr;
    }

    if (!tmp_ptr || *host_arg == '\0') {
        *hostName = putstr_ (ls_getmyhostname ());
    }
    else {
        strcpy (szOfficialName, host_arg);
        *hostName = putstr_ (szOfficialName);
    }

    if (tmp_arg) {
        *fName = putstr_ (tmp_arg);
    }
    else {
        free (freeup_tmp);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    free (freeup_tmp);
    return 0;
}

int
doXferRcp (struct rcpXfer *lsXfer, int option)
{
    pid_t pid = -1;
    // int n = 0 ;
    int status = 0;
    int sourceFh = 0;
    int local_errno = 0;
    int rcpp[2] = { 0,0 };
    char szRshDest[MAX_LINE_LEN];
    char errMsg[1024]; // FIXME FIXME why 1024 only?

    memset( szRshDest, '\0', MAX_LINE_LEN );
    memset( errMsg,    '\0', 1024 );

    if ((lsXfer->iOptions & O_APPEND) || (option & SPOOL_BY_LSRCP)) {
                
        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: using %s to copy '%s' to '%s'", __func__, RSHCMD, lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0]);
        }
                
        if (pipe (rcpp) < 0) {
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

            switch (pid = fork ()) {

                case 0: {
                    close (rcpp[0]); // FIXME FIXME replace 0 with appropriate label

                    if (rcpp[1]) {// FIXME FIXME replace 1 with appropriate label
                        if (logclass & (LC_FILE))  {
                            ls_syslog (LOG_DEBUG, "%s: child: re-directing stdout, stderr", __func__);
                        }
                        
                        close (STDOUT_FILENO);
                        close (STDERR_FILENO);
                        
                        if (dup2 (rcpp[1], STDOUT_FILENO) < 0) { // FIXME FIXME replace 1 with appropriate label
                            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                        }

                        if (dup2 (rcpp[1], STDERR_FILENO) < 0) { // FIXME FIXME replace 1 with appropriate label
                            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                        }

                        close (rcpp[1]); // FIXME FIXME replace 1 with appropriate label
                    }

                    if ((sourceFh = open (lsXfer->ppszHostFnames[0], O_RDONLY, 0)) < 0) {
                            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                    }
                    else {
                        close (STDIN_FILENO);
                        
                        if (dup2 (sourceFh, STDIN_FILENO)) {
                            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                        }

                        close (sourceFh);
                    }

                    if ((option & SPOOL_BY_LSRCP)) {
                        sprintf (szRshDest, "cat > %s", lsXfer->ppszDestFnames[0]);
                    }
                    else {
                       sprintf (szRshDest, "cat >>! %s", lsXfer->ppszDestFnames[0]);
                    }
                    
                    execlp (RSHCMD, RSHCMD, lsXfer->szDest, szRshDest, NULL);
                    
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                    
                    break;

                }
                case -1: {
                
                    if (logclass & (LC_FILE)) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
                    }
                    
                    close (rcpp[0]); // FIXME FIXME replace 0 with appropriate label
                    close (rcpp[1]); // FIXME FIXME replace 1 with appropriate label
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
                break;
                default: {
                    close (rcpp[1]); 
                    // FIXME FIXME FIXME FIXME FIXME 
                    //
                    //  DANGER WARNING DANGER: BREAK STATEMENT MISSING
                    //
                    // 

                    ssize_t cc = read (rcpp[0], errMsg, 1024); // FIXME FIXME replace 0 with appropriate label
                    ssize_t i = 0;
                    for ( i = cc; cc > 0;) {

                            assert( 1024 - i >= 0 ); // FIXME FIXME uuuuh, what?
                            cc = read (rcpp[0], errMsg + i, (size_t)(1024 - i)); // FIXME FIXME replace 0 with appropriate label
                            if (cc > 0) {
                                    i += cc;
                            }
                    }

                    local_errno = errno;
                    close (rcpp[0]); // FIXME FIXME replace 0 with appropriate label

                    if (waitpid (pid, 0, 0) < 0 && errno != ECHILD) {
                        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                    }

                    if (cc < 0) {
                        fprintf (stderr, "%s\n", strerror (local_errno));
                        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                    }

                    if (i > 0) {
                        fprintf (stderr, "%s: %s", RSHCMD, errMsg);
                        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                    }
                    
                    return 0;
                }
                break;
            }
        
        } else {

            if (logclass & (LC_FILE)) {
                ls_syslog (LOG_DEBUG, __func__, "exec rcp");
            }

            switch (pid = fork ()) {
                case 0: {
                    execlp ("rcp", "rcp", "-p", lsXfer->szSourceArg, lsXfer->szDestArg, NULL);
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "execlp");
                    exit (-1);
                }
                break;

                case -1: {
                    if (logclass & (LC_FILE)) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
                    }
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
                break;

                default:
                    while( wait (&status) < 0) { // SEEME SEEME SEEME this needs looking after
                        if (errno != EINTR) {
                            break;
                        }
                    }

                    if (WIFEXITED (status))  {
                        if (WEXITSTATUS (status) == 0) {
                            return 0;
                        }
                    }
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                break;
            }
                // return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

    return 0;
}

int
createXfer (struct rcpXfer *lsXfer )
{
    lsXfer->iNumFiles = 1;
    lsXfer->iOptions = 0;
    return 0;
}


int
destroyXfer (struct rcpXfer *lsXfer )
{
    // int i;

    memset( lsXfer->szSourceArg, '\0', strlen( lsXfer->szSourceArg ) );
    memset( lsXfer->szDestArg,   '\0', strlen( lsXfer->szDestArg   ) );
    memset( lsXfer->szHostUser,  '\0', strlen( lsXfer->szHostUser  ) );
    memset( lsXfer->szDestUser,  '\0', strlen( lsXfer->szDestUser  ) );
    memset( lsXfer->szHost,      '\0', strlen( lsXfer->szHost      ) );
    memset( lsXfer->szDest,      '\0', strlen( lsXfer->szDest      ) );

    free (lsXfer->szSourceArg);
    free (lsXfer->szDestArg);
    free (lsXfer->szHostUser);
    free (lsXfer->szDestUser);
    free (lsXfer->szHost);
    free (lsXfer->szDest);

    for ( size_t i = 0; i < lsXfer->iNumFiles; i++) {
        free (lsXfer->ppszHostFnames[i]);
        free (lsXfer->ppszDestFnames[i]);
    }

    return 0;
}

int
equivalentXferFile (struct rcpXfer * lsXfer, const char *szLocalFile, const char *szRemoteFile, struct stat *psLstat, struct stat *psRstat, const char *szRhost)
{
    char *pszH        = NULL;
    char *szFileName1 = NULL;
    char *szFileName2 = NULL;
    // char **hostlist   = NULL;

    struct hostInfo *hostinfo = NULL;

    char szHost1[MAXHOSTNAMELEN];
    char szHost2[MAXHOSTNAMELEN];

    memset( szHost1, '\0', MAXHOSTNAMELEN );
    memset( szHost2, '\0', MAXHOSTNAMELEN );

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "%s: ls_getmnthost() for '%s'", __func__, szLocalFile);
    }

    // hostlist[0] = szRhost; // FIXME FIXME replace 0 with appropriate label
    hostinfo = ls_gethostinfo ( NULL, NULL, &szRhost, 1, 0);
    if (hostinfo == NULL) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    else{
        if (strcmp (hostinfo->hostType, "NTX86") == 0 || strcmp (hostinfo->hostType, "NTALPHA") == 0) { // FIXME FIXME FIXME replace strings with label/char vars
            return 1;
        }
    }

    if ((pszH = ls_getmnthost (szLocalFile)) == NULL) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    strcpy (szHost1, pszH);

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, __func__, "ls_rgetmnthost() for '%s' on '%s'", szLocalFile, szRhost);
    }

    if ((pszH = ls_rgetmnthost (szRhost, szRemoteFile)) == NULL) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    strcpy (szHost2, pszH);

    szFileName1 = strrchr (lsXfer->ppszHostFnames[0], '/'); // FIXME FIXME replace 0 with appropriate label
    szFileName2 = strrchr (lsXfer->ppszDestFnames[0], '/'); // FIXME FIXME replace 0 with appropriate label
    if (szFileName1 == NULL) {
        szFileName1 = lsXfer->ppszHostFnames[0]; // FIXME FIXME replace 0 with appropriate label
    }
    else {
        szFileName1++;
    }
    if (szFileName2 == NULL) {
        szFileName2 = lsXfer->ppszDestFnames[0]; // FIXME FIXME replace 0 with appropriate label
    }
    else {
        szFileName2++;
    }
    if (psLstat->st_ino == psRstat->st_ino && (strcmp (szFileName1, szFileName2) == 0) && equalHost_ (szHost1, szHost2)) {
        return 0;
    }

    return 1;
}

int
copyFile (struct rcpXfer *lsXfer, const char *buf, int option)
{
    int s             = 0;
    int lfd           = 0;
    int rfd           = 0;
    int iRetVal       = 0;
    int file_no_exist = FALSE;
 
    ssize_t ret      = 0;
    mode_t mode      = 0x000;
    char *szThisHost = NULL; 

    struct stat sLstat;
    struct stat sRstat;
    struct hostent *hp1 = NULL;
    struct hostent *hp2 = NULL;

    static int first = 1;
    static struct hostent pheHostBuf;
    static struct hostent pheDestBuf;

    szThisHost = malloc( sizeof(MAXHOSTNAMELEN) + 1 );

    if (first) {
        memset (&pheHostBuf, '\0', sizeof (struct hostent));
        memset (&pheDestBuf, '\0', sizeof (struct hostent));
        first = 0;
    }

    if ( NULL != pheHostBuf.h_name ) {
        free (pheHostBuf.h_name);
    }

    if ( NULL != pheDestBuf.h_name ) {
        free (pheDestBuf.h_name);
    }

    strcpy (szThisHost, ls_getmyhostname ());
    lserrno = LSE_FILE_SYS;

    if (strcmp (lsXfer->szHostUser, lsXfer->szDestUser) != 0) {
        /* catgets 6050 */
        ls_syslog (LOG_ERR, "catgets 6050: %s: %s does not support account mapping using rcp", __func__, "RES");
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    hp1 = Gethostbyname_ (lsXfer->szHost);
    if ( NULL == hp1 ) {
        ls_syslog (LOG_ERR, "%s gethostbyname() failed for %s", __func__, lsXfer->szHost);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    pheHostBuf.h_name = strdup (hp1->h_name);
    lsXfer->pheHost = &pheHostBuf;
    hp2 = Gethostbyname_ (lsXfer->szDest);
    if ( NULL == hp2 ) {
        ls_syslog (LOG_ERR, "%s gethostbyname() failed for %s", __func__, lsXfer->szDest);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    pheDestBuf.h_name = strdup (hp2->h_name);
    lsXfer->pheDest = &pheDestBuf;

    if ((strcmp (szThisHost, lsXfer->szHost) == 0) && (strcmp (szThisHost, lsXfer->szDest) == 0)) {

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: mystat_() for file '%s' on '%s'", __func__, lsXfer->ppszHostFnames[0], lsXfer->szHost);
        }

        if (mystat_ (lsXfer->ppszHostFnames[0], &sLstat, lsXfer->pheHost) < 0) {  // FIXME FIXME replace 0 with appropriate label
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "mystat_");
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if (mystat_ (lsXfer->ppszDestFnames[0], &sRstat, lsXfer->pheDest) == 0) {  // FIXME FIXME replace 0 with appropriate label
            if ((sLstat.st_ino == sRstat.st_ino) && (sLstat.st_dev == sRstat.st_dev)) {
                return s;
            }
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s, myopen_() for file '%s' on '%s'", __func__, lsXfer->ppszHostFnames[0], lsXfer->szHost); // FIXME FIXME replace 0 with appropriate label
        }
        
        lfd = myopen_ (lsXfer->ppszHostFnames[0], O_RDONLY, 0600, lsXfer->pheHost);
        if( -1 == lfd ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myopen_");
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s, myopen_() for file '%s' on '%s'", __func__, lsXfer->ppszDestFnames[0], lsXfer->szDest);
        }

        mode = sLstat.st_mode;
        rfd = myopen_ (lsXfer->ppszDestFnames[0], O_CREAT | O_WRONLY | (lsXfer->iOptions & O_APPEND ? O_APPEND : O_TRUNC), mode, lsXfer->pheDest);
        if ( -1 == rfd ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myopen_");
            close (lfd);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s, begin copy from '%s' to '%s'", __func__, lsXfer->szHost, lsXfer->szDest);
        }

        for ( size_t len = LSRCP_MSGSIZE; len > 0;) { // FIXME FIXME FIXME FIXME re-write using while loop

            ssize_t read_return_value = (ssize_t) len; // NOFIX len will always be positive at the start
            if ((read_return_value = read (lfd, strdup(buf), LSRCP_MSGSIZE)) > 0) {
                if ((ret = write (rfd, buf, len)) != read_return_value) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "write", ret);
                    close (lfd);
                    close (rfd);
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            }

            len = (size_t) read_return_value; // NOFIX the only reason the assignment is here, is to create a terminating condition (len == 0)
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s, end copy from '%s' to '%s'", __func__, lsXfer->szHost, lsXfer->szDest);
        }

        close (lfd);
        close (rfd);
    }
    else if (strcmp (szThisHost, lsXfer->szHost) == 0) {

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s, mystat_() for file '%s' on '%s'", __func__, lsXfer->ppszHostFnames[0], lsXfer->szHost);
        }

        if (mystat_ (lsXfer->ppszHostFnames[0], &sLstat, lsXfer->pheHost) < 0) { // FIXME FIXME replace 0 with appropriate label
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "mystat_");
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: ls_rstat() for file '%s' on '%s'", __func__, lsXfer->ppszDestFnames[0], lsXfer->szDest); // FIXME FIXME replace 0 with appropriate label
        }

        if (ls_rstat (lsXfer->szDest, lsXfer->ppszDestFnames[0], &sRstat) == 0) {

            iRetVal = equivalentXferFile (lsXfer, lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0], &sLstat, &sRstat, lsXfer->szDest);
            if (iRetVal == 0) {
                // original: fprintf (stderr, I18N (2302, "%s and %s are identical\n"), lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0]);
                fprintf (stderr, "catgets 2302: %s and %s are identical\n" ,lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0] ); // FIXME FIXME replace 0 with appropriate label
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }

            if ( -1 == iRetVal ) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "equivalentXferFile");
                ls_rfterminate (lsXfer->szDest);

                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }
    }
    else {
        file_no_exist = TRUE;
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "%s: myopen_() for file '%s' on '%s'", __func__, lsXfer->ppszHostFnames[0], lsXfer->szHost); // FIXME FIXME replace 0 with appropriate label
    }

    if ((lfd = myopen_ (lsXfer->ppszHostFnames[0], O_RDONLY, 0600, lsXfer->pheHost)) == -1) { // FIXME FIXME why is there an octal value here? 
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myopen_");
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "%s: ls_ropen_() for file '%s' on '%s'", __func__, lsXfer->ppszDestFnames[0], lsXfer->szDest);
    }

    mode = (file_no_exist ? sLstat.st_mode : sRstat.st_mode);

    if (option & SPOOL_BY_LSRCP) {
        
        rfd = (int) ls_ropen (lsXfer->szDest, lsXfer->ppszDestFnames[0], O_CREAT | O_RDWR | O_EXCL | LSF_O_CREAT_DIR, mode); // FIXME FIXME replace 0 with appropriate label // FIXME FIXME remove cast
        if ( 255 == rfd ) {
            close (lfd);
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_ropen");
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

    }
    else {

        rfd = (int) ls_ropen (lsXfer->szDest, lsXfer->ppszDestFnames[0], O_CREAT | O_WRONLY | (lsXfer->iOptions & O_APPEND ? O_APPEND : O_TRUNC), mode); // FIXME FIXME replace 0 with appropriate label // FIXME FIXME remove cast
        if( 255 == rfd ) {
            close (lfd);
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_ropen");
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "%s, begin copy from '%s' to '%s'", __func__, lsXfer->szHost, lsXfer->szDest);
    }

    for ( size_t len = LSRCP_MSGSIZE; len > 0;) { // FIXME FIXME FIXME FIXME FIXME re-write with whiole loop and breaking condition
        ssize_t read_return_value = (ssize_t) len; // NOFIX len will always be positive at the start
        if ((read_return_value = read (lfd, strdup(buf), LSRCP_MSGSIZE)) > 0) { // FIXME IFXME why is the read length here LSRCP_MSGSIZE and not len?

            ret = ls_rwrite (rfd, strdup(buf), len); 
            if ( ret != read_return_value)  {
                close (lfd);
                ls_rclose (rfd);
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_MM, __func__, "ls_rwrite", ret);
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }

            len  = (size_t) read_return_value; // NOFIX the only reason the assignment is here, is to create a terminating condition (len == 0)
        }
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "%s: end copy from '%s' to '%s'", __func__, lsXfer->szHost, lsXfer->szDest);

        close (lfd);
        ls_rclose (rfd);

    }
    else if (strcmp (szThisHost, lsXfer->szDest) == 0)
    {

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: ls_rstat() for file '%s' on '%s'", __func__, lsXfer->ppszHostFnames[0], lsXfer->szHost);  // FIXME FIXME replace 0 with appropriate label
        }

        if (ls_rstat (lsXfer->szHost, lsXfer->ppszHostFnames[0], &sLstat) < 0) { // FIXME FIXME replace 0 with appropriate label
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_rstat");
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: mystat_() for file '%s' on '%s'", __func__, lsXfer->ppszDestFnames[0], lsXfer->szDest); // FIXME FIXME replace 0 with appropriate label
        }

        if (mystat_ (lsXfer->ppszDestFnames[0], &sRstat, lsXfer->pheDest) == 0) { // FIXME FIXME replace 0 with appropriate label

            iRetVal = equivalentXferFile (lsXfer, lsXfer->ppszDestFnames[0], lsXfer->ppszHostFnames[0], &sRstat, &sLstat, lsXfer->szHost); // FIXME FIXME replace 0 with appropriate label
            if ( 0 == iRetVal ) {

                fprintf (stderr, "catgets 2302: %s and %s are identical\n", lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0]); // FIXME FIXME replace 0 with appropriate label
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }

            if ( -1 == iRetVal ) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "equivalentXferFile");
                ls_rfterminate (lsXfer->szHost);
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }
        else {
            file_no_exist = TRUE;
        }


        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: ls_ropen() for file '%s' on '%s'", __func__, lsXfer->ppszHostFnames[0], lsXfer->szHost); // FIXME FIXME replace 0 with appropriate label
        }

        if ((lfd = (int) ls_ropen (lsXfer->szHost, lsXfer->ppszHostFnames[0], O_RDONLY, 0)) == -1) { // FIXME FIXME replace 0 with appropriate label // FIXME FIXME remove cast
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_ropen");
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: myopen_() for file '%s' on '%s'", __func__, lsXfer->ppszDestFnames[0], lsXfer->szDest); // FIXME FIXME replace 0 with appropriate label
        }

        mode = (file_no_exist ? sLstat.st_mode : sRstat.st_mode);
        rfd = myopen_ (lsXfer->ppszDestFnames[0], O_CREAT | O_WRONLY | (lsXfer->iOptions & O_APPEND ? O_APPEND : O_TRUNC), mode, lsXfer->pheDest); // FIXME FIXME replace 0 with appropriate label
        if( -1 == rfd ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myopen_");
            ls_rclose (lfd);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: begin copy from '%s' to '%s'", __func__, lsXfer->szHost, lsXfer->szDest);
        }

        for ( size_t len = LSRCP_MSGSIZE; len > 0;) {

            ssize_t read_return_value = (ssize_t) len; // NOFIX len will always be positive at the start
            if ((read_return_value = ls_rread (lfd, buf, LSRCP_MSGSIZE)) > 0) { // FIXME IFXME why is the read length here LSRCP_MSGSIZE and not len?
                ret = write (rfd, buf, len);
                if ( ret !=  read_return_value) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "write", ret);
                    ls_rclose (lfd);
                    close (rfd);
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            }

            len  = (size_t) read_return_value; // NOFIX the only reason the assignment is here, is to create a terminating condition (len == 0)
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "%s: end copy from '%s' to '%s'", __func__, lsXfer->szHost, lsXfer->szDest);
        }

        ls_rclose (lfd);
        close (rfd);

    }
    else {
        if (logclass & (LC_FILE)) {
                ls_syslog (LOG_DEBUG, "%s does not support third party transfers.", __func__, " Using rcp");
        }
        
        lserrno = LSE_FILE_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return 0;
}


int
rmDirAndFiles ( const char *dir)
{
    return rmDirAndFilesEx (dir, 0);
}

int
rmDirAndFilesEx ( const char *dir, int recur)
{
    DIR *dirp         = NULL ;
    struct dirent *dp = NULL;
    char path[MAX_PATH_LEN];

    memset( path, '\0', MAX_PATH_LEN );

    if ((dirp = opendir (dir)) == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    readdir (dirp);
    readdir (dirp);

    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp)) {
        sprintf (path, "%s/%s", dir, dp->d_name);

        if (recur) {
            struct stat stBuf;
            int doRecur = 0;

            if (lstat (path, &stBuf) == 0) {
#ifdef S_ISLNK
                if (!S_ISLNK (stBuf.st_mode)) {
                    doRecur = 1;
                }
#endif
            }

            if (doRecur) {
                rmDirAndFilesEx (path, 1);
            }
        }
            rmdir (path);
            unlink (path);
    }

    closedir (dirp);

    return rmdir (dir);
}

int
createSpoolSubDir (const char *spoolFileFullPath)
{
    char *pEnd1  = NULL;
    char *pEnd2  = NULL;
    char *pBegin = NULL;
    DIR *pDir    = NULL;
    char subDirectory1[MAX_FILENAME_LEN];
    char subDirectory2[MAX_FILENAME_LEN];
    size_t len = 0;
    int returnValue = 0;
    mode_t initialMask   = 0x000;
    mode_t previousUmask = 0x000;

    if (spoolFileFullPath == NULL) {
        returnValue = -1;
        umask (previousUmask);
        return returnValue;
    }
    else {
            
        pBegin = strdup( spoolFileFullPath );
        if ((pEnd1 = strrchr (spoolFileFullPath, '/')) == NULL) {
                pEnd1 = strdup( spoolFileFullPath );
        }
        if ((pEnd2 = strrchr (spoolFileFullPath, '\\')) == NULL) {
                pEnd2 = strdup( spoolFileFullPath );
        }

        assert( pEnd2 - spoolFileFullPath > 0 );
        assert( pEnd1 - spoolFileFullPath > 0 );
        // len = (size_t)(pEnd2 > pEnd1) ? (size_t)(pEnd2 - spoolFileFullPath) : (size_t)(pEnd1 - spoolFileFullPath);
        if( pEnd2 > pEnd1 ) {
            len = (size_t)(pEnd2 - spoolFileFullPath);
        }
        else {
            len = (size_t)(pEnd1 - spoolFileFullPath);
        }
        previousUmask = umask ( initialMask ); // FIXME FIXME FIXME replace numerical with appropriate label

        if (len <= 0) {
                umask (previousUmask);
                return returnValue;

        }

        strncpy (subDirectory1, pBegin, len);
        subDirectory1[len] = '\0';

        if ((pDir = opendir (subDirectory1)) == NULL) {
            // pBegin = &subDirectory1; 
            pBegin = subDirectory1; 

            if ((pEnd1 = strrchr (pBegin, '/')) == NULL) {
                pEnd1 = pBegin;
            }

            if ((pEnd2 = strrchr (pBegin, '\\')) == NULL) {
                pEnd2 = pBegin;
            }
         
            assert( pEnd2 - pBegin > 0 );
            assert( pEnd1 - pBegin > 0 );
            // len = (pEnd2 > pEnd1) ? pEnd2 - pBegin : pEnd1 - pBegin;
            if( pEnd2 > pEnd1 ) {
                len = (size_t)(pEnd2 - pBegin);
            }
            else {
                len = (size_t)(pEnd1 - pBegin);
            }

            if ( 0 == len ) {

                if (mkdir (subDirectory1, 0755) != 0) { // FIXME FIXME FIXME replace numerical with appropriate label
                    returnValue = -1;
                    umask (previousUmask);
                    return -1;
                }
                else {
                        returnValue = 0;
                }
                umask (previousUmask);
                return returnValue;

            }
            else {
                strncpy (subDirectory2, pBegin, len);
                subDirectory2[len] = '\0';

                if ((pDir = opendir (subDirectory2)) == NULL) {
                    if (mkdir (subDirectory2, 0777) != 0) { // FIXME FIXME FIXME replace numerical with appropriate label
                        returnValue = -1;
                        umask (previousUmask);
                        return returnValue;
                    }
                }
                else {
                    returnValue = 0;
                    closedir (pDir);
                }

                if (mkdir (subDirectory1, 0755) != 0) { // FIXME FIXME FIXME replace numerical with appropriate label
                    returnValue = -1;
                }
                else {
                    returnValue = 0;
                }

                umask (previousUmask);
                return returnValue;

            }
        }
        else {
            closedir (pDir);
        }
    }

    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
}
