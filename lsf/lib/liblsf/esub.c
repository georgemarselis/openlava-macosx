/* $Id: lib.esub.c 397 2007-11-26 19:04:00Z mblack $
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
#include <fcntl.h>
#include <termios.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>

#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/mls.h"
#include "lib/syslog.h"
#include "lib/esub.h"
#include "lib/id.h"
#include "lib/init.h"
#include "lib/sig.h"
#include "lib/rdwr.h"
#include "libint/lsi18n.h"
#include "lib/structs/genParams.h"
#include "daemons/libniosd/niosd.h"
#include "daemons/libresd/resout.h"

/* #define exit(a)         _exit(a) */

int
runEsub_ (struct lenData *ed, const char *path)
{
    char esub[MAX_PATH_LEN];
    char *myargv[6];
    struct stat sbuf;
    char ESUB[] = "esub";

    assert( rootuid_ ); // NOTE bullshit op to hush up the compiler
    assert( NSIG_MAP );  // bullshit OP, make the compiler stop complaining
    assert( sigSymbol ); // bullshit OP, make the compiler stop complaining
    assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain

    ed->len = 0;
    ed->data = NULL;

    myargv[0] = esub; // FIXME FIXME FIXME figure out what myargv[0] is and replace the 0 with a label
    if (path == NULL) {
        sprintf (esub, "%s/%s", genParams_[LSF_SERVERDIR].paramValue, ESUB);
        myargv[1] = NULL;  // FIXME FIXME FIXME figure out what myargv[1] is and replace the 1 with a label
    }
    else {
        if (*path == '\0') {
            strcpy (esub, ESUB);
        }
        else {
            sprintf (esub, "%s/%s", path, ESUB);
        }
        strcpy( myargv[1], "-r"); // FIXME FIXME FIXME FIXME FIXME someone sure loves his default arguments
        myargv[2] = NULL;
    }

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "%s: esub=<%s>", __func__, esub);
    }

    if (stat (esub, &sbuf) < 0)
    {
        if (logclass & LC_TRACE) {
            ls_syslog (LOG_DEBUG, "%s: stat(%s) failed: %m", __func__, esub);
        }
        lserrno = LSE_ESUB;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    if (runEClient_ (ed, myargv) == -1) {
        return -2; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    return 0;
}

int
runEClient_ (struct lenData *ed, char **argv)
{
  char lsfUserName[MAX_LSF_NAME_LEN];

    if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value //
    }

    return getEData (ed, argv, lsfUserName);
}

int
getEData (struct lenData *ed, char **argv, const char *lsfUserName)
{
    int ePorts[2]  = { 0, 0 };
    long cc        = 0 ;
    uid_t uid      = 0;
    pid_t pid      = -1;
    size_t size    = 0;
    char *abortVal = NULL;
    char *buf      = NULL;
    char *sp       = NULL;
    LS_WAIT_T status;
  

  
    if (getOSUid_ (lsfUserName, &uid) < 0) {
        ls_syslog (LOG_DEBUG, I18N_FUNC_S_FAIL_MM, __func__, "getOSUid_", lsfUserName);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    if (pipe (ePorts) < 0) {
        if (logclass & (LC_TRACE | LC_AUTH)) {
            ls_syslog (LOG_DEBUG, "%s: pipe failed: %m", __func__);
        }

        lserrno = LSE_PIPE;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    if ((pid = fork ()) == 0) {
        close (ePorts[0]);
        dup2 (ePorts[1], 1);

        // lsfSetXUid(0, uid, uid, -1, setuid);
        lsfSetXUid(0, uid, uid, INT_MAX, setuid); // the 4th argument, INT_MAX here is totally ignored in the body of the function

        lsfExecX( NULL, &argv[0], execvp);
        ls_syslog (LOG_DEBUG, "%s: execvp(%s) failed: %m", __func__, argv[0]);
        exit (-1);
    }

    if (pid == -1) {
        if (logclass & (LC_TRACE | LC_AUTH)) {
            ls_syslog (LOG_DEBUG, "%s: fork failed aborting child", __func__);
        }
        close (ePorts[0]);
        close (ePorts[1]);
        lserrno = LSE_FORK;

        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    close (ePorts[1]);

    ed->len = 0;
    ed->data = NULL;

    buf = (char *) malloc (MSGSIZE + 1);
    if ( NULL == buf ) {
        if (logclass & (LC_TRACE | LC_AUTH)) {
            ls_syslog (LOG_DEBUG, "%s: malloc failed: %m", __func__);
        }
        
        lserrno = LSE_MALLOC;
        
        // FIXME FIXME FIXME FIXME
        // THIS SHIT IS THE FIRST THING THAT HAS TO GO
        // goto errorReturn;
        close (ePorts[0]);
        kill (pid, SIGKILL);
        while (waitpid (pid, 0, 0) == -1 && errno == EINTR) {
            ;
        }

        ed->len = 0;
        ed->data = NULL;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    for( size = MSGSIZE, ed->len = 0, sp = buf; (cc = read (ePorts[0], sp, size)); ) {
        if ( -1 == cc ) {

            if (logclass & (LC_TRACE | LC_AUTH)) {
                ls_syslog (LOG_DEBUG, "%s: read error: %m", __func__);
            }
            
            if (errno == EINTR) {
                continue;
            }

            /* catgets 5552 */
            ls_syslog( LOG_ERR, "catgets 5552: %s: <%s> read(%d): %m", __func__, argv[0], size );
            break;
        }
    }

    assert( cc >= 0);
    ed->len += (unsigned long) cc;
    sp += cc;
    assert( cc >= 0);
    size -= (unsigned long) cc;
    if ( 0 == size ) {

        sp = (char *) realloc (buf, ed->len + MSGSIZE + 1);
        if ( NULL == sp ) {
            
            if (logclass & (LC_TRACE | LC_AUTH)) {
                ls_syslog (LOG_DEBUG, "%s: realloc failed: %m", __func__);
            }
            
            lserrno = LSE_MALLOC;
            free (buf);
          
            // FIXME FIXME FIXME FIXME 
            // THIS SHIT HAS TO GO
            // goto errorReturn;
            close (ePorts[0]);
            kill (pid, SIGKILL);
            while (waitpid (pid, 0, 0) == -1 && errno == EINTR) {
                ;
            }

            ed->len = 0;
            ed->data = NULL;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
        }

        buf = sp;
        sp = buf + ed->len;
        size = MSGSIZE;
    }

    close (ePorts[0]);
    ed->data = buf;

    ed->data[ed->len] = '\0';

    while (waitpid (pid, &status, 0) == -1 && errno == EINTR) {
        ;
    }

    if ((abortVal = getenv ("LSB_SUB_ABORT_VALUE"))) {  // FIXME FIXME FIXME FIXME add to configure.ac

        if ((WIFEXITED (status) && WEXITSTATUS (status) == atoi (abortVal)) || WIFSIGNALED (status)) {
            FREEUP (ed->data);
        }

        ed->len = 0;
  
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }
    

    if (ed->len == 0) {
        FREEUP (ed->data);
    }

    return 0;

// errorReturn: // FIXME FIXME FIXME get rid of goto label

//     close (ePorts[0]);
//     kill (pid, SIGKILL);
//     while (waitpid (pid, 0, 0) == -1 && errno == EINTR) {
//         ;
//     }

//     ed->len = 0;
//     ed->data = NULL;
//     return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value

}

int
runEexec_ (char *option, unsigned int job, struct lenData *eexec, const char *path)
{
    int p[2]            = {0, 0};
    int isRenew         = FALSE;
    ssize_t cc          = 0;
    pid_t   pid         = -1;
    char    eexecPath[MAX_FILENAME_LEN];
    char    *myargv[3];
    struct  stat sbuf;
    const char EEXECNAME[] = "eexec";



    if (strcmp (option, "-r") == 0) {
        isRenew = TRUE;
    }

    if (isRenew == TRUE)
    {
        if (path[0] == '\0') {
            strcpy (eexecPath, EEXECNAME);
        }
        else {
            sprintf (eexecPath, "%s/%s", path, EEXECNAME);
        }
    }
    else {
        sprintf (eexecPath, "%s/%s", genParams_[LSF_SERVERDIR].paramValue, EEXECNAME);
    }

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "%s: eexec path, option and data length of job/task <%d> are <%s>, <%s> and <%d>", __func__, job, eexecPath, option, eexec->len); 
    }

    if (stat (eexecPath, &sbuf) < 0) {
        if (logclass & LC_TRACE) {
            ls_syslog (LOG_DEBUG, "%s: Job/task <%d> eexec will not be run, stat(%s) failed: %m", __func__, job, eexecPath);
        }
        
        lserrno = LSE_ESUB;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    //i = 0;
    myargv[0] = eexecPath;
    if (strcmp (option, "-r") == 0) {
        myargv[1] = malloc( sizeof(char)*strlen("-r") + 1 );
        strcpy( myargv[1], "-r");
    }
    myargv[2] = NULL;


    if (pipe (p) < 0) {
        lserrno = LSE_PIPE;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    if ((pid = fork ()) == 0) {
        char *user;
        uid_t uid;
    
        if ((user = getenv ("LSFUSER")) != NULL) { // FIXME FIXME FIXME FIXME add to configure.ac
        
            if (getOSUid_ (user, &uid) < 0) {
                ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_MM, __func__, "getOSUid_", user);
                exit (-1);
            }
        }
        else {

            struct passwd *pw;
            user = getenv ("USER");  // FIXME FIXME FIXME FIXME add to configure.ac


            if ((pw = getpwnam (user)) == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "getpwnam", user);  // FIXME FIXME FIXME FIXME add to configure.ac
                exit (-1);
            }
            
            uid = pw->pw_uid;
          
        }

        if ( lsfSetXUid(0, uid, uid, INT_MAX, setuid) < 0) { // FIXME FIXME the 4th arguement is ignored in the body of the function, so, passing INT_MAX as a sentinel flag is fine.
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "setuid", uid);  // FIXME FIXME FIXME FIXME add to configure.ac
            exit (-1);
        }

        if (setpgid (0, getpid ()) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "setpgid");  // FIXME FIXME FIXME FIXME add to configure.ac
            exit (-1);
        }

        for ( int i = 1; i < SIGRTMAX; i++) { // NSIG is in <signal.h>
            Signal_ ( (unsigned int)i, SIG_DFL); 
        }

        alarm (0);
        close (p[1]);
        if (dup2 (p[0], 0) == -1) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "dup2(stdin,0)");  // FIXME FIXME FIXME FIXME add to configure.ac
        }

        for (int i = 3; i < sysconf (_SC_OPEN_MAX); i++)  {
            close (i);
        }

        lsfExecX( myargv[0], myargv, execvp );
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "execvp", myargv[0]);  // FIXME FIXME FIXME FIXME add to configure.ac
        exit (-1);
    }

    if (pid == -1) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
        close (p[0]);
        close (p[1]);
        lserrno = LSE_FORK;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful *positive* return value
    }

    close (p[0]);

    if (eexec->len > 0) {

        cc = b_write_fix (p[1], eexec->data, eexec->len);
        assert( eexec->len <= LONG_MAX );
        if ( cc != (ssize_t) eexec->len && strcmp (option, "-p")) {  // FIXME FIXME FIXME FIXME add to configure.ac
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "b_write_fix", eexec->len);
        }
    }

    close (p[1]);

    while (waitpid (pid, NULL, 0) < 0 && errno == EINTR) {
        ;
    }

    return 0;
}

char *
runEGroup_ (char *type, char *gname)
{
    struct lenData ed;
    char lsfUserName[MAX_LSF_NAME_LEN];
    char egroupPath[MAX_FILENAME_LEN];
    char *argv[4] = { NULL, NULL, NULL, NULL };
    char *managerIdStr = NULL;
    uid_t uid = INT_MAX;
    struct stat sbuf;
    char EGROUPNAME[ ] = "egroup";

    sprintf (egroupPath, "%s/%s", genParams_[LSF_SERVERDIR].paramValue, EGROUPNAME);

    argv[0] = egroupPath;
    argv[1] = type;
    argv[2] = gname;
    argv[3] = NULL;

    uid = getuid ();
    if (uid == 0 && (managerIdStr = getenv ("LSB_MANAGERID")) != NULL)  // FIXME FIXME FIXME FIXME add to configure.ac
    {
        uid = (uid_t) atoi (managerIdStr);
        if (getLSFUserByUid_ (uid, lsfUserName, sizeof (lsfUserName)) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_MM, __func__, "getLSFUserByUid_", uid);  // FIXME FIXME FIXME FIXME add to configure.ac
            return NULL;
        }
    }
    else
    {
        if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "getLSFUser_");  // FIXME FIXME FIXME FIXME add to configure.ac
            return NULL;
        }
    }

    if (stat (egroupPath, &sbuf) < 0)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "stat", egroupPath);  // FIXME FIXME FIXME FIXME add to configure.ac
        return NULL;
    }

    if (getEData (&ed, argv, lsfUserName) < 0)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "getEData", egroupPath);  // FIXME FIXME FIXME FIXME add to configure.ac
        return NULL;
    }

    if (ed.len > 0) {
        ed.data[ed.len] = '\0';
        return ed.data;
    }

    return NULL;
}
