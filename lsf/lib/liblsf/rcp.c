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

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/rcp.h"
#include "lsf.h"

#define NL_SETN   23


int doXferRcp (lsRcpXfer * lsXfer, int option);

int
parseXferArg (char *arg, char **userName, char **hostName, char **fName)
{
        char *tmp_arg    = NULL;
        char *tmp_ptr    = NULL;
        char *user_arg   = NULL;
        char *host_arg   = NULL;
        char *freeup_tmp = NULL;
        char szOfficialName[MAXHOSTNAMELEN];

        freeup_tmp = tmp_arg = putstr_ (arg);

        tmp_ptr = strchr (tmp_arg, '@');

    if (tmp_ptr)
        {
            *tmp_ptr = '\0';
            user_arg = tmp_arg;
            tmp_arg = ++tmp_ptr;
        }

    if (!tmp_ptr || *user_arg == '\0')
        {
            char lsfUserName[MAX_LSF_NAME_LEN];
            if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) != 0)
        {
            free (freeup_tmp);
            return -1;
        }
            *userName = putstr_ (lsfUserName);
        }
    else
        *userName = putstr_ (user_arg);

    tmp_ptr = strchr (tmp_arg, ':');

    if (tmp_ptr)
        {
            *tmp_ptr = '\0';
            host_arg = tmp_arg;
            tmp_arg = ++tmp_ptr;
        }

    if (!tmp_ptr || *host_arg == '\0')
        {
            *hostName = putstr_ (ls_getmyhostname ());
        }
    else
        {
            strcpy (szOfficialName, host_arg);
            *hostName = putstr_ (szOfficialName);
        }

    if (tmp_arg)
        {
            *fName = putstr_ (tmp_arg);
        }
    else
        {
            free (freeup_tmp);
            return -1;
        }

    free (freeup_tmp);
    return 0;
}

int
doXferRcp (lsRcpXfer * lsXfer, int option)
{

        pid_t pid;
        int rcpp[2], sourceFh;
        char errMsg[1024];
        char szRshDest[MAXLINELEN];
        int local_errno = 0;
        // int n = 0 ;
        int status = 0;

        if ((lsXfer->iOptions & O_APPEND) || (option & SPOOL_BY_LSRCP)) {
                
                if (logclass & (LC_FILE)) {
                        ls_syslog (LOG_DEBUG, "%s: using %s to copy '%s' to '%s'", __func__, RSHCMD, lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0]);
                }
                
                if (pipe (rcpp) < 0) {
                        return -1;
                }

                switch (pid = fork ()) {

                        case 0:
                            close (rcpp[0]);

                            if (rcpp[1])
                            {
                                    if (logclass & (LC_FILE))
                                            ls_syslog (LOG_DEBUG, "%s: child: re-directing stdout, stderr", __func__);
                                    
                                    close (STDOUT_FILENO);
                                    close (STDERR_FILENO);
                                    
                                    if (dup2 (rcpp[1], STDOUT_FILENO) < 0)
                                            return -1;

                                    if (dup2 (rcpp[1], STDERR_FILENO) < 0)
                                            return -1;

                                    close (rcpp[1]);
                            }

                            if ((sourceFh = open (lsXfer->ppszHostFnames[0], O_RDONLY, 0)) < 0) {
                                    return -1;
                            }
                            else {
                                    close (STDIN_FILENO);
                                    
                                    if (dup2 (sourceFh, STDIN_FILENO)) {
                                            return -1;
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
                            
                            return -1;
                            
                            break;

                        case -1:
                        
                            if (logclass & (LC_FILE)) {
                                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
                            }
                            
                            close (rcpp[0]);
                            close (rcpp[1]);
                            return -1;

                        break;
                        default:
                            close (rcpp[1]); 
                            // FIXME FIXME FIXME FIXME FIXME 
                            //
                            //  DANGER WARNING DANGER: BREAK STATEMENT MISSING
                            //
                            // 

                            ssize_t cc = read (rcpp[0], errMsg, 1024);
                            ssize_t i = 0;
                            for ( i = cc; cc > 0;) {

                                    assert( 1024 - i >= 0 );
                                    cc = read (rcpp[0], errMsg + i, (size_t)(1024 - i));
                                    if (cc > 0) {
                                            i += cc;
                                    }
                            }

                            local_errno = errno;
                            close (rcpp[0]);

                            if (waitpid (pid, 0, 0) < 0 && errno != ECHILD) {
                                    return -1;
                            }

                            if (cc < 0) {
                                    fprintf (stderr, "%s\n", strerror (local_errno));
                                    return -1;
                            }

                            if (i > 0) {
                                    fprintf (stderr, "%s: %s", RSHCMD, errMsg);
                                    return -1;
                            }
                            
                            return 0;
                        break;
                }
        
        } else {

                if (logclass & (LC_FILE)) {
                        ls_syslog (LOG_DEBUG, "doXferRcp(), exec rcp");
                }

                switch (pid = fork ()) {
                    case 0:
                        execlp ("rcp", "rcp", "-p", lsXfer->szSourceArg, lsXfer->szDestArg, NULL);
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "execlp");
                        exit (-1);
                    // break;

                    case -1:
                        if (logclass & (LC_FILE)) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
                        }
                        return -1;
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
                    return -1;
                break;
                }

                // return -1;
        }
}

int
createXfer (lsRcpXfer * lsXfer)
{
    lsXfer->iNumFiles = 1;
    lsXfer->iOptions = 0;
    return 0;
}


int
destroyXfer (lsRcpXfer * lsXfer)
{

    int i;

    free (lsXfer->szSourceArg);
    free (lsXfer->szDestArg);
    free (lsXfer->szHostUser);
    free (lsXfer->szDestUser);
    free (lsXfer->szHost);
    free (lsXfer->szDest);
    for (i = 0; i < lsXfer->iNumFiles; i++)
        {
            free (lsXfer->ppszHostFnames[i]);
            free (lsXfer->ppszDestFnames[i]);
        }
    return 0;

}

int
equivalentXferFile (lsRcpXfer * lsXfer, char *szLocalFile, char *szRemoteFile, struct stat *psLstat, struct stat *psRstat, char *szRhost)
{
    char *pszH;
    char szHost1[MAXHOSTNAMELEN], szHost2[MAXHOSTNAMELEN];
    char *hostlist[1];
    struct hostInfo *hostinfo;
    char *szFileName1, *szFileName2;

    if (logclass & (LC_FILE))
        ls_syslog (LOG_DEBUG, "equivalentXferFile(), ls_getmnthost() for '%s'", szLocalFile);

    hostlist[0] = szRhost;
    hostinfo = ls_gethostinfo ((char *) NULL, (size_t *) NULL, (char **) hostlist, 1, 0);
    if (hostinfo == (struct hostInfo *) NULL)
        {
            return -1;
        }
    else
        {
            if (strcmp (hostinfo->hostType, "NTX86") == 0
            || strcmp (hostinfo->hostType, "NTALPHA") == 0)
        {
            return 1;
        }
        }

    if ((pszH = ls_getmnthost (szLocalFile)) == NULL)
        {
            return -1;
        }

    strcpy (szHost1, pszH);

    if (logclass & (LC_FILE))
        ls_syslog (LOG_DEBUG,
                     "equivalentXferFile(),ls_rgetmnthost() for '%s' on '%s'",
                     szLocalFile, szRhost);

    if ((pszH = ls_rgetmnthost (szRhost, szRemoteFile)) == NULL)
        {
            return -1;
        }

    strcpy (szHost2, pszH);

    szFileName1 = strrchr (lsXfer->ppszHostFnames[0], '/');
    szFileName2 = strrchr (lsXfer->ppszDestFnames[0], '/');
    if (szFileName1 == NULL) {
        szFileName1 = lsXfer->ppszHostFnames[0];
    }
    else {
        szFileName1++;
    }
    if (szFileName2 == NULL) {
        szFileName2 = lsXfer->ppszDestFnames[0];
    }
    else {
        szFileName2++;
    }
    if (psLstat->st_ino == psRstat->st_ino
            && (strcmp (szFileName1, szFileName2) == 0)
            && equalHost_ (szHost1, szHost2))
        {
            return 0;
        }
    return 1;

}

int
copyFile (lsRcpXfer * lsXfer, char *buf, int option)
{
    int s             = 0;
    int lfd           = 0;
    int rfd           = 0;
    int iRetVal       = 0;
    int file_no_exist = FALSE;
 
    ssize_t ret = 0;
    mode_t mode = 0x000;
    char *szThisHost = (char *) malloc( sizeof(MAXHOSTNAMELEN) + 1 );
    

    struct stat sLstat, sRstat;
    struct hostent *hp1, *hp2;

    static int first = 1;
    static struct hostent pheHostBuf, pheDestBuf;

    if (first) {
        memset (&pheHostBuf, 0, sizeof (struct hostent));
        memset (&pheDestBuf, 0, sizeof (struct hostent));
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
        ls_syslog (LOG_ERR, I18N (6050, "%s: %s does not support account mapping using rcp"), __func__, "RES");
        return -1;
    }

    hp1 = Gethostbyname_ (lsXfer->szHost);
    if ( NULL == hp1 ) {
        ls_syslog (LOG_ERR, "%s gethostbyname() failed for %s", __func__, lsXfer->szHost);
        return -1;
    }

    pheHostBuf.h_name = strdup (hp1->h_name);
    lsXfer->pheHost = &pheHostBuf;
    hp2 = Gethostbyname_ (lsXfer->szDest);
    if ( NULL == hp2 ) {
        ls_syslog (LOG_ERR, "%s gethostbyname() failed for %s", __func__, lsXfer->szDest);
        return -1;
    }

    pheDestBuf.h_name = strdup (hp2->h_name);
    lsXfer->pheDest = &pheDestBuf;

    if ((strcmp (szThisHost, lsXfer->szHost) == 0) && (strcmp (szThisHost, lsXfer->szDest) == 0)) {

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), mystat_() for file '%s' on '%s'", lsXfer->ppszHostFnames[0], lsXfer->szHost);
        }

        if (mystat_ (lsXfer->ppszHostFnames[0], &sLstat, lsXfer->pheHost) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "mystat_");
            return -1;
        }

        if (mystat_ (lsXfer->ppszDestFnames[0], &sRstat, lsXfer->pheDest) == 0) {

            if ((sLstat.st_ino == sRstat.st_ino) && (sLstat.st_dev == sRstat.st_dev)) {
                return s;
            }
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), myopen_() for file '%s' on '%s'", lsXfer->ppszHostFnames[0], lsXfer->szHost);
        }
        
        lfd = myopen_ (lsXfer->ppszHostFnames[0], O_RDONLY, 0600, lsXfer->pheHost);
        if( -1 == lfd ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "copyFile", "myopen_");
            return -1;
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, " copyFile(), myopen_() for file '%s' on '%s'", lsXfer->ppszDestFnames[0], lsXfer->szDest);
        }

        mode = sLstat.st_mode;
        rfd = myopen_ (lsXfer->ppszDestFnames[0], O_CREAT | O_WRONLY | (lsXfer->iOptions & O_APPEND ? O_APPEND : O_TRUNC), mode, lsXfer->pheDest);
        if ( -1 == rfd ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "copyFile", "myopen_");
            close (lfd);
            return -1;
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), begin copy from '%s' to '%s'", lsXfer->szHost, lsXfer->szDest);
        }

        for ( ssize_t len = LSRCP_MSGSIZE; len > 0;) {                      // FIXME FIXME FIXME FIXME re-write using while loop

            if ((len = read (lfd, buf, LSRCP_MSGSIZE)) > 0) {
                if ((ret = write (rfd, buf, (size_t) len)) != len) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "copyFile", "write", ret);
                    close (lfd);
                    close (rfd);
                    return -1;
                }
            }
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), end copy from '%s' to '%s'", lsXfer->szHost, lsXfer->szDest);
        }

        close (lfd);
        close (rfd);
    }
    else if (strcmp (szThisHost, lsXfer->szHost) == 0) {

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), mystat_() for file '%s' on '%s'", lsXfer->ppszHostFnames[0], lsXfer->szHost);
        }

        if (mystat_ (lsXfer->ppszHostFnames[0], &sLstat, lsXfer->pheHost) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "mystat_");
            return -1;
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), ls_rstat() for file '%s' on '%s'", lsXfer->ppszDestFnames[0], lsXfer->szDest);
        }

        if (ls_rstat (lsXfer->szDest, lsXfer->ppszDestFnames[0], &sRstat) == 0) {

            iRetVal = equivalentXferFile (lsXfer, lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0], &sLstat, &sRstat, lsXfer->szDest);
            if (iRetVal == 0) {
                // original: fprintf (stderr, I18N (2302, "%s and %s are identical\n"), lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0]);
                fprintf (stderr, "catgets 2302: %s and %s are identical\n" ,lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0] );
                return -1;
            }

            if ( -1 == iRetVal ) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "equivalentXferFile");
                ls_rfterminate (lsXfer->szDest);

                return -1;
            }
        }
    }
    else {
        file_no_exist = TRUE;
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "copyFile(), myopen_() for file '%s' on '%s'", lsXfer->ppszHostFnames[0], lsXfer->szHost);
    }

    if ((lfd = myopen_ (lsXfer->ppszHostFnames[0], O_RDONLY, 0600, lsXfer->pheHost)) == -1) { // FIXME FIXME why is there an octal value here? 
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "copyFile", "myopen_");
        return -1;
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "copyFile(), ls_ropen_() for file '%s' on '%s'", lsXfer->ppszDestFnames[0], lsXfer->szDest);
    }

    mode = (file_no_exist ? sLstat.st_mode : sRstat.st_mode);

    if (option & SPOOL_BY_LSRCP) {
        
        rfd = ls_ropen (lsXfer->szDest, lsXfer->ppszDestFnames[0], O_CREAT | O_RDWR | O_EXCL | LSF_O_CREAT_DIR, mode);
        if ( -1 == rfd ) {
            close (lfd);
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, "copyFile", "ls_ropen");
            return -1;
        }

    }
    else {

        rfd = ls_ropen (lsXfer->szDest, lsXfer->ppszDestFnames[0], O_CREAT | O_WRONLY | (lsXfer->iOptions & O_APPEND ? O_APPEND : O_TRUNC), mode);
        if( -1 == rfd ) {
            close (lfd);
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, "copyFile", "ls_ropen");
            return -1;
        }
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "copyFile(), begin copy from '%s' to '%s'", lsXfer->szHost, lsXfer->szDest);
    }

    for ( ssize_t len = LSRCP_MSGSIZE; len > 0;) {              // FIXME FIXME FIXME FIXME FIXME re-write with whiole loop and breaking condition
        if ((len = read (lfd, buf, LSRCP_MSGSIZE)) > 0) {

            assert( len >= 0);
            ret = ls_rwrite (rfd, buf, (size_t) len); 
            if ( ret != len)  {
                close (lfd);
                ls_rclose (rfd);
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_MM, "copyFIle", "ls_rwrite", ret);
                return -1;
            }
        }
    }

    if (logclass & (LC_FILE)) {
        ls_syslog (LOG_DEBUG, "copyFile(), end copy from '%s' to '%s'", lsXfer->szHost, lsXfer->szDest);

        close (lfd);
        ls_rclose (rfd);

    }
    else if (strcmp (szThisHost, lsXfer->szDest) == 0)
    {

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), ls_rstat() for file '%s' on '%s'", lsXfer->ppszHostFnames[0], lsXfer->szHost);
        }

        if (ls_rstat (lsXfer->szHost, lsXfer->ppszHostFnames[0], &sLstat) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, "copyFile", "ls_rstat");
            return -1;
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), mystat_() for file '%s' on '%s'", lsXfer->ppszDestFnames[0], lsXfer->szDest);
        }

        if (mystat_ (lsXfer->ppszDestFnames[0], &sRstat, lsXfer->pheDest) == 0) {

            iRetVal = equivalentXferFile (lsXfer, lsXfer->ppszDestFnames[0], lsXfer->ppszHostFnames[0], &sRstat, &sLstat, lsXfer->szHost);
            if ( 0 == iRetVal ) {

                fprintf (stderr, "catgets 2302: %s and %s are identical\n", lsXfer->ppszHostFnames[0], lsXfer->ppszDestFnames[0]);
                return -1;
            }

            if ( -1 == iRetVal ) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, "copyFile", "equivalentXferFile");
                ls_rfterminate (lsXfer->szHost);
                return -1;
            }
        }
        else {
            file_no_exist = TRUE;
        }


        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), ls_ropen() for file '%s' on '%s'", lsXfer->ppszHostFnames[0], lsXfer->szHost);
        }

        if ((lfd = ls_ropen (lsXfer->szHost, lsXfer->ppszHostFnames[0], O_RDONLY, 0)) == -1) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, "copyFile", "ls_ropen");
            return -1;
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, " copyFile(), myopen_() for file '%s' on '%s'", lsXfer->ppszDestFnames[0], lsXfer->szDest);
        }

        mode = (file_no_exist ? sLstat.st_mode : sRstat.st_mode);
        rfd = myopen_ (lsXfer->ppszDestFnames[0], O_CREAT | O_WRONLY | (lsXfer->iOptions & O_APPEND ? O_APPEND : O_TRUNC), mode, lsXfer->pheDest);
        if( -1 == rfd ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "copyFile", "myopen_");
            ls_rclose (lfd);
            return -1;
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, "copyFile(), begin copy from '%s' to '%s'", lsXfer->szHost, lsXfer->szDest);
        }

        for ( size_t len = LSRCP_MSGSIZE; len > 0;) {
            if ((len = (size_t) ls_rread (lfd, buf, LSRCP_MSGSIZE)) > 0) {
                ret = write (rfd, buf, len);
                if ( ret != (ssize_t) len) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "copyFile", "write", ret);
                    ls_rclose (lfd);
                    close (rfd);
                    return -1;
                }
            }
        }

        if (logclass & (LC_FILE)) {
            ls_syslog (LOG_DEBUG, " copyFile(), end copy from '%s' to '%s'", lsXfer->szHost, lsXfer->szDest);
        }

        ls_rclose (lfd);
        close (rfd);

    }
    else {
            if (logclass & (LC_FILE)) {
                    ls_syslog (LOG_DEBUG, "copyFile() does not support third party transfers.", " Using rcp");
            }
            
            lserrno = LSE_FILE_SYS;
            return -1;
    }

    return 0;

}


int
rmDirAndFiles (char *dir)
{
    return rmDirAndFilesEx (dir, 0);
}

int
rmDirAndFilesEx (char *dir, int recur)
{
    DIR *dirp;
    struct dirent *dp;
    char path[MAX_PATH_LEN];

    if ((dirp = opendir (dir)) == NULL)
        {
            return -1;
        }

    readdir (dirp);
    readdir (dirp);

    for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp))
        {
            sprintf (path, "%s/%s", dir, dp->d_name);
            if (recur)
        {
            struct stat stBuf;
            int doRecur = 0;

            if (lstat (path, &stBuf) == 0)
                {
#ifdef S_ISLNK
                    if (!S_ISLNK (stBuf.st_mode))
                {
                    doRecur = 1;
                }
#endif
                }

            if (doRecur)
                {
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
    long len = 0;
    int returnValue = 0;
    mode_t previousUmask = 0;

    if (spoolFileFullPath == NULL) {
            returnValue = -1;
            umask (previousUmask);
            return returnValue;

    }
    else {
            
        pBegin = (char *) spoolFileFullPath;
        if ((pEnd1 = strrchr (spoolFileFullPath, '/')) == NULL) {
                pEnd1 = (char *) spoolFileFullPath;
        }
        if ((pEnd2 = strrchr (spoolFileFullPath, '\\')) == NULL) {
                pEnd2 = (char *) spoolFileFullPath;
        }

        len = (pEnd2 > pEnd1) ? pEnd2 - spoolFileFullPath : pEnd1 - spoolFileFullPath;
        previousUmask = umask (000);

        if (len <= 0) {
                umask (previousUmask);
                return returnValue;

        }

        strncpy (subDirectory1, pBegin, len);
        subDirectory1[len] = '\0';

        if ((pDir = opendir (subDirectory1)) == NULL)
            {
                pBegin = (char *) &subDirectory1;

                if ((pEnd1 = strrchr (pBegin, '/')) == NULL)
            {
                pEnd1 = (char *) pBegin;
            }

                if ((pEnd2 = strrchr (pBegin, '\\')) == NULL)
            {
                pEnd2 = (char *) pBegin;
            }

                len = (pEnd2 > pEnd1) ? pEnd2 - pBegin : pEnd1 - pBegin;

                if (len == 0)
            {

                if (mkdir (subDirectory1, 0755) != 0)
                    {
                        returnValue = -1;
                            umask (previousUmask);
                            return returnValue;
                    }
                else
                    {
                        returnValue = 0;
                    }
                    umask (previousUmask);
                    return returnValue;

            }
                else
            {
                strncpy (subDirectory2, pBegin, len);
                subDirectory2[len] = '\0';

                if ((pDir = opendir (subDirectory2)) == NULL)
                    {
                        if (mkdir (subDirectory2, 0777) != 0)
                    {
                        returnValue = -1;
                            umask (previousUmask);
                            return returnValue;
                    }
                    }
                else
                    {
                        returnValue = 0;
                        closedir (pDir);
                    }

                if (mkdir (subDirectory1, 0755) != 0)
                    {
                        returnValue = -1;
                    }
                else
                    {
                        returnValue = 0;
                    }
            umask (previousUmask);
            return returnValue;

            }
        }
        else
            {
                closedir (pDir);
            }
    }

    return -1;
}
