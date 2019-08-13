/* $Id: lib.eauth.c 397 2007-11-26 19:04:00Z mblack $
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

#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <termios.h>

#include "lib/lib.h"
#include "lib/id.h"
// #include "lib/lproto.h"
#include "lib/init.h"
#include "lib/eauth.h"
#include "lib/rdwr.h"
#include "lib/misc.h"
#include "lib/syslog.h"
// #include "lsb/sig.h"    // SIGRTMAX 
#include "daemons/libniosd/niosd.h"
#include "daemons/libresd/resout.h"
#include "libint/lsi18n.h"
#include "lib/structs/genParams.h"
#include "lib/esub.h"
#include "lib/sig.h"
#include "lib/info.h"

// #define NL_SETN 23

// #define _DARWIN_C_SOURCE

int
getAuth_lsf( struct lsfAuth *auth, const char *host )
{
    assert( rootuid_ ); // NOTE bullshit op to hush up the compiler
    auth->uid = getuid ();

    assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain
    if (getLSFUser_ (auth->lsfUserName, sizeof (auth->lsfUserName)) < 0) {
        ls_syslog (LOG_DEBUG, I18N_FUNC_FAIL_MM, __func__, "getLSFUser_");
        lserrno = LSE_BADUSER;
        return -1; // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    auth->gid = getgid ();

    if (!genParams_[LSF_AUTH].paramValue) {
        auth->kind = CLIENT_SETUID;
    }
    else if (!strcmp (genParams_[LSF_AUTH].paramValue, AUTH_IDENT)) {
        auth->kind = CLIENT_IDENT;
    }
    else if (!strcmp (genParams_[LSF_AUTH].paramValue, AUTH_PARAM_EAUTH)) {
        auth->kind = CLIENT_EAUTH;
        return getEAuth (&auth->k.eauth, host);
    }
    else {
        auth->kind = CLIENT_SETUID;
    }

  return 0;
}


int
getEAuth (struct eauth *eauth, const char *host)
{
    char *argv[4];
    char path[MAX_PATH_LEN];
    struct lenData ld;
    static const  char EAUTHNAME[] = "eauth"; // FIXME FIXME FIXME FIXME put in configure.ac

    assert( host );

    memset (path, 0, sizeof (path));
    ls_strcat (path, sizeof (path), genParams_[LSF_SERVERDIR].paramValue);
    ls_strcat (path, sizeof (path), "/");
    ls_strcat (path, sizeof (path), EAUTHNAME); 
    // argv[0] = path; // FIXME FIXME FIXME FIXME FIXME   wut
    // argv[1] = "-c"; // FIXME FIXME FIXME FIXME FIXME   the
    // argv[2] = host; // FIXME FIXME FIXME FIXME FIXME  frack
    // argv[3] = NULL; // FIXME FIXME FIXME FIXME FIXME    ?

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "runEAuth(): path=<%s>", path);
    }

    if (runEClient_ (&ld, argv) == -1) {

      if (logclass & (LC_AUTH | LC_TRACE)) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, "runEAuth", "runEClient", path);
      }

      lserrno = LSE_EAUTH;
      return -1; // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    if (ld.length == 0) {

      if (logclass & (LC_AUTH | LC_TRACE)) {
            ls_syslog (LOG_DEBUG, "runEAuth: <%s> got no data", path);
        }
        FREEUP (ld.data);
        lserrno = LSE_EAUTH;
        return -1; // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    if (ld.length > EAUTH_SIZE) {

        if (logclass & (LC_AUTH | LC_TRACE)) {
            ls_syslog (LOG_DEBUG, "runEAuth: <%s> got too much data, size=%d", path, ld.length);
        }

        FREEUP (ld.data);
        lserrno = LSE_EAUTH;
        return -1; // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    memcpy (eauth->data, ld.data, ld.length);
    eauth->data[ld.length] = '\0';
    if (logclass & (LC_AUTH | LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "runEAuth: <%s> got data=%s", path, ld.data);
    }
    
    eauth->length = ld.length;

    FREEUP (ld.data);
    
    if (logclass & (LC_AUTH | LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "runEAuth: <%s> got length=%d", path, ld.length);
    }

  return 0;

}

int
verifyEAuth_ (struct lsfAuth *auth, struct sockaddr_in *from)
{

    char path[MAX_PATH_LEN] = "";
    char uData[256] = "";
    char ok = '\0';
    char *eauth_client      = NULL;
    char *eauth_server      = NULL;
    char *eauth_aux_data    = NULL;
    char *eauth_aux_status  = NULL;
    size_t cc = 0;
    int returnvalue = 0;
    size_t uData_length = 0;
    long b_write_fix_returnvalue = 0;
    static const char EAUTHNAME[] = "eauth"; // FIXME FIXME FIXME FIXME put in configure.ac

    static int connected = FALSE;
    static int in[2], out[2];

    if (logclass & LC_TRACE) {
        ls_syslog (LOG_DEBUG, "%s ...", __func__);
    }

    if (!(genParams_[LSF_AUTH].paramValue && !strcmp (genParams_[LSF_AUTH].paramValue, AUTH_PARAM_EAUTH))) {
        return -1;  // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    // 
    // FIXME FIXME FIXME getting environmental variables has to go.
    //      get them from a conf file.
    eauth_client     = getenv ("LSF_EAUTH_CLIENT");      // FIXME FIXME FIXME FIXME FIXME LSF_EAUTH_CLIENT must go to configure.ac
    eauth_server     = getenv ("LSF_EAUTH_SERVER");      // FIXME FIXME FIXME FIXME FIXME LSF_EAUTH_SERVER must go to configure.ac
    eauth_aux_data   = getenv ("LSF_EAUTH_AUX_DATA");    // FIXME FIXME FIXME FIXME FIXME LSF_EAUTH_AUX_DATA must go to configure.ac
    eauth_aux_status = getenv ("LSF_EAUTH_AUX_STATUS");  // FIXME FIXME FIXME FIXME FIXME LSF_EAUTH_AUX_STATUS must go to configure.ac

    assert( auth->k.eauth.length <= INT_MAX ); // FIXME FIXME why does it need to be less than INT_MAX?S
    sprintf (uData, "%ud %ud %s %s %u %lu %s %s %s %s\n", auth->uid, auth->gid,
        auth->lsfUserName, inet_ntoa (from->sin_addr),
            ntohs (from->sin_port), auth->k.eauth.length,
            (eauth_client ? eauth_client : "NULL"),
            (eauth_server ? eauth_server : "NULL"),
            (eauth_aux_data ? eauth_aux_data : "NULL"),
            (eauth_aux_status ? eauth_aux_status : "NULL"));

    memset (path, 0, sizeof (path));
    ls_strcat (path, sizeof (path), genParams_[LSF_SERVERDIR].paramValue);
    ls_strcat (path, sizeof (path), "/");
    ls_strcat (path, sizeof (path), EAUTHNAME); // FIXME FIXME FIXME fix function call, pointer to *

    if (logclass & (LC_AUTH | LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "%s: <%s> path <%s> connected=%d", __func__, uData, path, connected);
    }

    if (connected) {
        struct timeval tv;
        fd_set mask;

        FD_ZERO (&mask);
        FD_SET (out[0], &mask);

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        if ((returnvalue = select (out[0] + 1, &mask, NULL, NULL, &tv)) > 0) {
            if (logclass & (LC_AUTH | LC_TRACE)) {
               ls_syslog (LOG_DEBUG, "%s: <%s> got exception", __func__, uData);
            }

            connected = FALSE;
            close (in[1]);
            close (out[0]);
        }
        else {
           if (returnvalue < 0 ) { // FIXME FIXME 
               ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "select", uData);
           }
       }
    
        if (logclass & (LC_AUTH | LC_TRACE)) {
           ls_syslog (LOG_DEBUG, "%s: <%s> select returned cc=%d", __func__, uData, returnvalue);
       }

    }

    if (!connected) {
        pid_t pid = -1;
        char *user = NULL;

        if ((user = getLSFAdmin ()) == NULL) {
           return -1;
        }
    
        if (pipe (in) < 0) {
          ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "pipe(in)", uData);
          lserrno = LSE_SOCK_SYS;
          return -1;
        }

        if (pipe (out) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "pipe(out)", uData);
            lserrno = LSE_SOCK_SYS;
            return -1;
        }


        if ((pid = fork ()) == 0) {
            char *myargv[3];
            struct passwd *pw;

            if ((pw = getpwlsfuser_ (user)) == (struct passwd *) NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "getpwlsfuser_", user);
                exit (-1);
            }

            // FIXME investigate if the third argument to lsfSetXUid can be set to the appropriate
            // [s]uid_t type. if yes, try to see if there is an alternative to passing -1.
            if (lsfSetXUid(0, pw->pw_uid, pw->pw_uid, 4294967295, setuid)  < 0) {
              ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "setuid", pw->pw_uid);
              exit (-1);
            }

            for ( int i = 1; i < SIGRTMAX; i++) {  // SIGRTMAX is in a linux thing. SIGRTMAX is POSIX compliant // FIXME FIXME FIXME FIXME see if this works in other opearting systems
                Signal_ ( (unsigned int) i, SIG_DFL);               // IF you get stuck here and try to throw the kitchen sink on it in order to compile, disregard -D__SIGNAL_H if you see it in <signal.h>
            }

            alarm (0);

            close (in[1]);
            if (dup2 (in[0], 0) == -1) {
                ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "dup2(in[0])", uData);
            }

            close (out[0]);
            if (dup2 (out[1], 1) == -1) {
                ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "dup2(out[1])", uData);
            }

            for ( int i = 3; i < sysconf (_SC_OPEN_MAX); i++) {
                close (i);
            }

            // myargv[0] = path;
            // myargv[1] = "-s"; // FIXME FIXME FIXME FIXME the fuck is this? default arguments?
            // myargv[2] = NULL;

            lsfExecX( myargv[0], myargv, execvp);
            ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "execvp", myargv[0]);
            exit (-1);  // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
        }

        close (in[0]);
        close (out[1]);

        if (pid == -1)
        {
            ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "fork", path);
            close (in[1]);
            close (out[0]);
            lserrno = LSE_FORK;
            return -1;  // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
        }

        connected = TRUE;
    }

    // FIXME refactor till end of function as a separate function
    uData_length = strlen (uData);
    // FIXME following cast is ok. b_write_fix returns size_t, which is always 0 or possitive but
    //        i wanna see if i can turn cc from type long to type size_t
    //
    //  that depends on the use of cc, of course. Investigate
    // looks like it is done
    b_write_fix_returnvalue = b_write_fix (in[1], uData, uData_length);
    // like above: investigate, remove cast
    assert( b_write_fix_returnvalue >  0);
    if (uData_length != (size_t) b_write_fix_returnvalue)
    {
        /* catgets 5513 */
        ls_syslog (LOG_ERR, "5513: %s: b_write_fix <%s> failed, cc=%ld, i=%d: %ld", __func__, uData, b_write_fix_returnvalue, uData_length);
        close (in[1]);
        close (out[0]);
        connected = FALSE;
        return -1;  // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }
    
    if (logclass & (LC_AUTH | LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "5514: %s: b_write_fix <%s> ok, cc=%ld, i=%d", __func__, uData, b_write_fix_returnvalue, uData_length);
    }

    b_write_fix_returnvalue = b_write_fix (in[1], auth->k.eauth.data, (size_t) auth->k.eauth.length); // FIXME FIXME FIXME FIXME investigate the type of auth->k.eauth.length and figure out if length should be a size_t type. remove cast after you are done
    if ( cc != auth->k.eauth.length)
    {
         /* catgets 5515 */
        ls_syslog (LOG_ERR, "5515: %s: b_write_fix <%s> failed, eauth.length=%d, cc=%ld", __func__, uData, auth->k.eauth.length, b_write_fix_returnvalue);
        close (in[1]);
        close (out[0]);
        connected = FALSE;
        return -1;  // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    if (logclass & (LC_AUTH | LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "5516: %s: b_write_fix <%s> ok, eauth.length=%d, eauth.data=%.*s cc=%ld:", __func__, uData, auth->k.eauth.length, auth->k.eauth.length, auth->k.eauth.data, b_write_fix_returnvalue);
    }

    b_write_fix_returnvalue = b_read_fix (out[0], &ok, 1);
    if ( b_write_fix_returnvalue != 1) {
        /* catgets 5517 */
        ls_syslog (LOG_ERR, "5517: %s: b_read_fix <%s> failed, cc=%ld: %ld", __func__, uData, b_write_fix_returnvalue);
        close (in[1]);
        close (out[0]);
        connected = FALSE;
        return -1;  // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    if (ok != '1') {
        /* catgets 5518 */
        ls_syslog (LOG_ERR, "5518: %s: eauth <%s> length=%d failed, rc=%c", __func__, uData, auth->k.eauth.length, ok);
        return -1;  // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    return 0; 
}

char *
getLSFAdmin (void)
{
    static char admin[MAX_LSF_NAME_LEN];
    struct clusterInfo *clusterInfo = NULL;
    struct passwd *pw = NULL;
    const char *lsfUserName = NULL;
    char *mycluster = NULL;

    if (admin[0] != '\0') { // FIXME FIXME FIXME clean up, after managing to compile everything.
        return admin;
    }

    if ((mycluster = ls_getclustername ()) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_getclustername");
        return NULL;
    }

    if ((clusterInfo = ls_clusterinfo (NULL, NULL, NULL, 0, 0)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_clusterinfo");
        return NULL;
    }

    lsfUserName = (clusterInfo->nAdmins == 0 ? clusterInfo->managerName : clusterInfo->admins[0]);

    if ((pw = getpwlsfuser_ (lsfUserName)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "getpwlsfuser_", lsfUserName);
        return NULL;
    }

    strcpy (admin, lsfUserName);

    return admin;
}


int
putEnvVar (char *buf, const char *envVar, const char *envValue)
{
    int rc = 0 ;
    size_t str_size = 0;

    sprintf (buf, "%s=", envVar);
    if (envValue && strlen (envValue)) {

        str_size = strlen (buf) + strlen (envValue) + 1;
        
        if (str_size > EAUTH_ENV_BUF_LEN) {
            return -2; // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
        }
        
        strcat (buf, envValue);
    }

    rc = putenv (buf);
    
    if (rc != 0) {
        return -1; // FIXME FIXME FIXME FIXME ALTER THIS FROM NEGATIVE TO APPROPRIATELLY POSITIVE
    }

    return 0;
}

int
putEauthClientEnvVar( const char *client )
{
    static char eauth_client[2048];
    return putEnvVar (eauth_client, "LSF_EAUTH_CLIENT", client); // FIXME FIXME FIXME FIXME FIXME put LSF_EAUTH_CLIENT into configure.ac
}

int
putEauthServerEnvVar( const char *server )
{
    assert( NSIG_MAP );  // FIXME FIXME FIXME FIXME BULLSHIT CODE, compiler will not complain with strictest settings
    assert( sigSymbol ); // FIXME FIXME FIXME FIXME BULLSHIT CODE, compiler will not complain with strictest settings

    static char eauth_server[2048];
    return putEnvVar (eauth_server, "LSF_EAUTH_SERVER", server); // FIXME FIXME FIXME FIXME FIXME put LSF_EAUTH_SERVER into configure.ac
}
