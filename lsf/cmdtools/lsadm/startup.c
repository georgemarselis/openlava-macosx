/* $Id: startup.c 397 2007-11-26 19:04:00Z mblack $
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
#include <sys/wait.h>

#include "intlib/intlibout.h"
#include "lib/lproto.h"
#include "lib/mls.h"
#include "lsadmin/lsadmin.h"
#include "lsf.h"
#include "lsi18n.h"

// #define RSHCMD "rsh"

// #define NL_SETN 25

int getConfirm (char *);
void startupAllHosts (int, int);
void startupLocalHost (int);
void startupRemoteHost (char *, int, int);
int execDaemon (int, char **);
int getLSFenv (void);
struct clusterConf *findMyCluster (char *, struct sharedConf *);
char *daemonPath (char *);
int setStartupUid (void);

uid_t startupUid;
struct clusterConf *myClusterConf = NULL;

// static struct config_param myParamList[] = {
// #define LSF_CONFDIR     0
//   {"LSF_CONFDIR", NULL},
// #define LSF_SERVERDIR     1
//   {"LSF_SERVERDIR", NULL},
// #define LSF_BINDIR     2
//   {"LSF_BINDIR", NULL},
// #define LSF_LIM_DEBUG     3
//   {"LSF_LIM_DEBUG", NULL},
// #define LSF_RES_DEBUG     4
//   {"LSF_RES_DEBUG", NULL},
// #define LSB_DEBUG     5
//   {"LSB_DEBUG", NULL},
// #define LSF_LINK_PATH     6
//   {"LSF_LINK_PATH", NULL},
// #define LSF_CONF_LAST  7
//   {NULL, NULL}
// };

// #define BADMIN_HSTARTUP 11
const unsigned short BADMIN_HSTARTUP = 11

int
getLSFenv (void)
{
    char *envDir                    = NULL;
    struct sharedConf *mySharedConf = NULL;
    char lsfSharedFile[MAXLINELEN];

    for ( unsigned int i = 0; i < LSF_CONF_LAST; i++) { // FIXME FIXME FIXME FIXME LSF_CONF_LAST 
        FREEUP (myParamList[i].paramValue);
    }

    if ((envDir = getenv ("LSF_ENVDIR")) == NULL) {  // FIXME FIXME FIXME FIXME FIXME
        envDir = LSETCDIR;
    }

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "LSF_ENVDIR is %s", envDir);  // FIXME FIXME FIXME FIXME FIXME
    }

    if (initenv_ (myParamList, envDir) < 0) {
        ls_perror (envDir);
        return -1;
    }

    if (myParamList[LSF_CONFDIR].paramValue == NULL) {
        /* catgets 400 */ 
        fprintf (stderr, "%s %s %s/lsf.conf\n", "LSF_CONFDIR", I18N (400, "not defined in"), envDir);  // FIXME FIXME FIXME FIXME FIXME 
        return -1;
    }

    if (myParamList[LSF_SERVERDIR].paramValue == NULL && myParamList[LSF_LINK_PATH].paramValue != NULL) {
        fprintf (stderr, "%s %s %s/lsf.conf or environment\n", "LSF_SERVERDIR", I18N (400, "not defined in"), envDir); // FIXME FIXME FIXME FIXME FIXME 
        return -1;
    }

    if (myParamList[LSF_BINDIR].paramValue == NULL && myParamList[LSF_LINK_PATH].paramValue != NULL) {
        fprintf (stderr, "%s %s %s/lsf.conf  or environment\n", "LSF_BINDIR", I18N (400, "not defined in"), envDir);  // FIXME FIXME FIXME FIXME FIXME 
        return -1;
    }

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "LSF_CONFDIR=<%s>, LSF_BINDIR=<%s>, LSF_SERVERDIR=<%s>",
            myParamList[LSF_CONFDIR].paramValue,
            myParamList[LSF_BINDIR].paramValue,
            myParamList[LSF_SERVERDIR].paramValue
        );
    }

    memset (lsfSharedFile, 0, sizeof( lsfSharedFile ) );
    ls_strcat (lsfSharedFile, sizeof( lsfSharedFile ), myParamList[LSF_CONFDIR].paramValue);
    ls_strcat (lsfSharedFile, sizeof( lsfSharedFile ), "/lsf.shared");  // FIXME FIXME FIXME FIXME FIXME 


    if (access (lsfSharedFile, R_OK)) {
        ls_perror ("Can't access lsf.shared.");  // FIXME FIXME FIXME FIXME FIXME 
        return -1;
    }

    mySharedConf = ls_readshared (lsfSharedFile);
    if (mySharedConf == NULL) {
        ls_perror ("ls_readshared");  // FIXME FIXME FIXME FIXME FIXME 
        return -1;
    }

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "My lsf.shared file is: %s", lsfSharedFile); // FIXME FIXME FIXME FIXME FIXME 
        ls_syslog (LOG_DEBUG, "Clusters name is: %s\n", mySharedConf->clusterName);
    }

    if ((myClusterConf = findMyCluster (mySharedConf->clusterName, mySharedConf))) {
        return 0;
    }
    else if (lserrno && lserrno != LSE_NO_FILE) {
        return -1;
    }
    else {

    }

    /* catgets 401 */
    fprintf (stderr, "%s\n", I18N (401, "Host does not belong to LSF cluster."));
    return -1;
}

char *
daemonPath ( const char *daemon)
{
    char *srvdir = NULL;
    static char path[MAX_FILENAME_LEN];
  
    srvdir = myParamList[LSF_SERVERDIR].paramValue;

    memset (path, 0, sizeof (path) );
    ls_strcat (path, sizeof (path), srvdir);
    ls_strcat (path, sizeof (path), "/");
    ls_strcat (path, sizeof (path), daemon);

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "%s: daemonPath %s", __func__, path); // FIXME FIXME FIXME FIXME FIXME 
    }

    return path;
}


int
setStartupUid (void)
{
    startupUid = getuid ();

    if (  startupUid == 0                     ||
        myParamList[LSF_LIM_DEBUG].paramValue ||
        myParamList[LSF_RES_DEBUG].paramValue ||
        myParamList[LSB_DEBUG].paramValue
    ) {
      return 0;
    }

    return -1;
}


struct clusterConf *
findMyCluster ( const char *CName, struct sharedConf *mySharedConf)
{
    char *lhost = NULL;
    struct clusterConf *cl = NULL;
    char lsfClusterFile[MAXLINELEN];

    if ((lhost = ls_getmyhostname ()) == NULL) {
        ls_perror ("ls_getmyhostname"); // FIXME FIXME FIXME FIXME FIXME
        return NULL;
    }


    memset (lsfClusterFile, 0, sizeof (lsfClusterFile));
    ls_strcat (lsfClusterFile, sizeof (lsfClusterFile), myParamList[LSF_CONFDIR].paramValue);
    ls_strcat (lsfClusterFile, sizeof (lsfClusterFile), "/lsf.cluster.");  // FIXME FIXME FIXME FIXME FIXME
    ls_strcat (lsfClusterFile, sizeof (lsfClusterFile), CName);

    cl = ls_readcluster (lsfClusterFile, mySharedConf->lsinfo);
    if (cl == NULL) {
        if (logclass & LC_TRACE) {
            ls_syslog (LOG_DEBUG, "ls_readcluster <%s> failed: %M", lsfClusterFile);
        }


        if (lserrno == LSE_NO_ERR) {
            lserrno = LSE_BAD_ENV;
        }

        return NULL;
    }


    for ( unsigned int k = 0; k < cl->numHosts; k++) {
        if (logclass & (LC_TRACE)) {
            ls_syslog (LOG_DEBUG, " Host[%d]: %s", k, cl->hosts[k].hostName);
        }

        if (strcmp (lhost, cl->hosts[k].hostName) == 0) {
            if (logclass & (LC_TRACE)) {
                ls_syslog (LOG_DEBUG, "Local host %s belongs to cluster %s, nAdmins %d", lhost, CName, cl->clinfo->nAdmins);
            }

            for ( unsigned int j = 0; j < cl->clinfo->nAdmins; j++) {
                if (logclass & (LC_TRACE)) {
                    ls_syslog (LOG_DEBUG, "Admin[%d]: %s", j, cl->clinfo->admins[j]);
                }
            }

            return cl;
        }
    }

    return NULL;
}



void
startupAllHosts (int opCode, int confirm)
{
    char msg[MAXLINELEN];


    if (confirm) {
        if (opCode == LSADM_LIMSTARTUP) {
            /* catgets 404 */
            sprintf (msg, I18N (404, "Do you really want to start up LIM on all hosts ? [y/n]"));
        }
        else if (opCode == LSADM_RESSTARTUP) {
            /* catgets 405 */
            sprintf (msg, I18N (405, "Do you really want to start up RES on all hosts ? [y/n]"));
        }
        else if (opCode == BADMIN_HSTARTUP) {
            /* catgets 406 */
            sprintf (msg, I18N (406, "Do you really want to start up slave batch daemon on all hosts ? [y/n] "));
        }
        else {
            /* catgets 407 */
            fprintf (stderr, "%s: %s %d\n", I18N (407, "Unknown operation code"), __func__, opCode);
            return;
        }
        confirm = (!getConfirm (msg));
    }


    for ( unsigned int nh = 0; nh < myClusterConf->numHosts; nh++) {

        if ((myClusterConf->hosts[nh].isServer) && (strncmp (myClusterConf->hosts[nh].hostType, "NT", 2) != 0)) {
            startupRemoteHost (myClusterConf->hosts[nh].hostName, opCode, confirm);
        }

        if (logclass & LC_TRACE) {
            ls_syslog (LOG_DEBUG1, "Number of Hosts is: %d\n", myClusterConf->numHosts);
            ls_syslog (LOG_DEBUG1, "Hostname: %s\n", myClusterConf->hosts[nh].hostName);
            ls_syslog (LOG_DEBUG1, "isServer?:  %d\n", myClusterConf->hosts[nh].isServer);
        }
    }

    return;
}

void
startupLocalHost (int opCode)
{
    int  *myargc     = 10;
    char *host       = NULL;
    char *myargv[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }; // FIXME FIXME FIXME FIXME FIXME 10 seems awfully particular
    char operation[MAXLINELEN / 2];

    if ((host = ls_getmyhostname ()) == NULL) {
        host = "localhost";
    }

    if (opCode == LSADM_LIMSTARTUP) {
        /* catgets 408 */
        sprintf (operation, I18N (408, "Starting up LIM on"));
    }
    else if (opCode == LSADM_RESSTARTUP) {
        /* catgets 409 */
        sprintf (operation, I18N (409, "Starting up RES on"));
    }
    else if (opCode == BADMIN_HSTARTUP) {
        /* catgets 410 */
        sprintf (operation, I18N (410, "Starting up slave batch daemon on"));
    }
    else {
        fprintf (stderr, "%s %d\n", I18N (407, "Unknown operation code"), opCode);
        return;
    }

    fprintf (stderr, "%s <%s> ...... ", operation, host);
    fflush (stderr);

    myargv[1] = NULL;
    myargv[2] = NULL;
    myargv[3] = NULL;
    myargv[4] = NULL;

    switch (opCode) {
        case LSADM_LIMSTARTUP:
            myargv[0] = daemonPath ("lim");
        break;
        case LSADM_RESSTARTUP:
            myargv[0] = daemonPath ("res");
        break;
        case BADMIN_HSTARTUP:
            myargv[0] = daemonPath ("sbatchd");
        break;
        default:
            fprintf (stderr, "%s: %s %d\n", I18N (407, "Unknown operation code"), __func__, opCode);
            return;
        break;
    }

    if (execDaemon (startupUid, myargv, myargc ) == 0) {
        fprintf (stderr, "%s\n", I18N_done);
    }

    fflush (stderr);

    return;
}

void
startupRemoteHost ( const char *host, int opCode, int ask)
{
    int cc           = 0; 
    int symbolic     = FALSE;
    char *envDir     = NULL;
    char *myargv[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }; // FIXME FIXME FIXME FIXME FIXME 10 seems awfully particular
    char msg[2 * MAXLINELEN];

    if (opCode == LSADM_LIMSTARTUP) {  // FIXME FIXME FIXME FIXME FIXME where does the label come from?
        /* catgets 411 */
        sprintf (msg, I18N (411, "Start up LIM on"));
    }
    else if (opCode == LSADM_RESSTARTUP) {  // FIXME FIXME FIXME FIXME FIXME where does the label come from?
        /* catgets 412 */
        sprintf (msg, I18N (412, "Start up RES on"));
    }
    else if (opCode == BADMIN_HSTARTUP) {  // FIXME FIXME FIXME FIXME FIXME where does the label come from?
        sprintf (msg, I18N (413, "Start up slave batch daemon on"));
    }
    else {
        fprintf (stderr, "%s: %s %d\n", I18N (407, "Unknown operation code"), __func__, opCode);
        return;
    }

    if (ask) {
        sprintf (msg, "%s <%s> ? [y/n] ", msg, host);
        if (!getConfirm (msg)) {
            return;
        }
    }


    if ( ( myParamList[LSF_LINK_PATH].paramValue != NULL && strcmp (myParamList[LSF_LINK_PATH].paramValue, "n") != 0) ||
       ( getenv ("LSF_SERVERDIR") == NULL && getenv ("LSF_BINDIR") == NULL )
    ) {
        symbolic = TRUE;
    }

    cc = 0;
    myargv[cc++] = RSHCMD;  // FIXME FIXME FIXME FIXME FIXME no malloc
    myargv[cc++] = host;    // FIXME FIXME FIXME FIXME FIXME no malloc
    myargv[cc++] = "-n";    // FIXME FIXME FIXME FIXME FIXME no malloc

    if( (envDir = getenv ("LSF_ENVDIR") ) ) {  // FIXME FIXME FIXME FIXME FIXME
        myargv[cc++] = "/bin/sh ";             // FIXME FIXME FIXME FIXME FIXME no malloc
        myargv[cc++] = "-c ";                  // FIXME FIXME FIXME FIXME FIXME no malloc

        if (strlen (envDir) > MAXLINELEN) {
            fprintf (stderr, "LSF_ENVDIR is longer than <%d> chars <%s> \n", MAXLINELEN, envDir);
            exit (-1);
        }

        memset (msg, 0, sizeof (msg));
        ls_strcat (msg, sizeof (msg), "'LSF_ENVDIR=");  // FIXME FIXME FIXME FIXME FIXME
        ls_strcat (msg, sizeof (msg), envDir);

        switch (opCode) {
            case LSADM_LIMSTARTUP:  // FIXME FIXME FIXME FIXME FIXME where does the label come from?
                if (symbolic == TRUE) {
                    ls_strcat (msg, sizeof (msg), "; export LSF_ENVDIR; . $LSF_ENVDIR/lsf.conf; $LSF_BINDIR/lsadmin limstartup'");  // FIXME FIXME FIXME FIXME FIXME
                }
                else {
                    ls_strcat (msg, sizeof (msg), "; export LSF_ENVDIR; . $LSF_ENVDIR/lsf.conf; . $LSF_CONFDIR/profile.lsf; $LSF_BINDIR/lsadmin limstartup'");  // FIXME FIXME FIXME FIXME FIXME
                }
            break;
            case LSADM_RESSTARTUP: // FIXME FIXME FIXME FIXME FIXME where does the label come from?
                if (symbolic == TRUE) {
                    ls_strcat (msg, sizeof (msg), "; export LSF_ENVDIR; . $LSF_ENVDIR/lsf.conf; $LSF_BINDIR/lsadmin resstartup'"); // FIXME FIXME FIXME FIXME FIXME
                }
                else {
                    ls_strcat (msg, sizeof (msg), "; export LSF_ENVDIR; . $LSF_ENVDIR/lsf.conf; . $LSF_CONFDIR/profile.lsf; $LSF_BINDIR/lsadmin resstartup'");  // FIXME FIXME FIXME FIXME FIXME
                }
            break;
            case BADMIN_HSTARTUP: // FIXME FIXME FIXME FIXME FIXME where does the label come from?
                    if (symbolic == TRUE) {
                        ls_strcat (msg, sizeof (msg), "; export LSF_ENVDIR; . $LSF_ENVDIR/lsf.conf; $LSF_BINDIR/badmin hstartup'"); // FIXME FIXME FIXME FIXME FIXME
                    }
                    else {
                        ls_strcat (msg, sizeof (msg), "; export LSF_ENVDIR; . $LSF_ENVDIR/lsf.conf; . $LSF_CONFDIR/profile.lsf; $LSF_BINDIR/badmin hstartup'"); // FIXME FIXME FIXME FIXME FIXME
                    }
            break;
            default:
                fprintf (stderr, "%s: %s %d", I18N (407, __func__, "Unknown operation  code"), opCode);
                exit (-1);
            break;
        }

        myargv[cc++] = msg; // FIXME FIXME FIXME FIXME FIXME malloc
    }
    else {
        myargv[cc++] = "/bin/sh ";
        myargv[cc++] = "-c ";

        switch (opCode) {
            case LSADM_LIMSTARTUP:
                if (symbolic == TRUE) {
                    myargv[cc++] = "'. /etc/lsf.conf; $LSF_BINDIR/lsadmin limstartup'"; // FIXME FIXME FIXME FIXME FIXME 
                }
                else {
                    myargv[cc++] = "'. /etc/lsf.conf; . $LSF_CONFDIR/profile.lsf;  lsadmin limstartup'"; // FIXME FIXME FIXME FIXME FIXME 
                }
            break;
            case LSADM_RESSTARTUP:
                if (symbolic == TRUE) {
                    myargv[cc++] = "'. /etc/lsf.conf; $LSF_BINDIR/lsadmin resstartup'"; // FIXME FIXME FIXME FIXME FIXME 
                }
                else {
                    myargv[cc++] = "'. /etc/lsf.conf; . $LSF_CONFDIR/profile.lsf; lsadmin resstartup'"; // FIXME FIXME FIXME FIXME FIXME 
                }
            break;
            case BADMIN_HSTARTUP:
                if (symbolic == TRUE) {
                    myargv[cc++] = "'. /etc/lsf.conf; $LSF_BINDIR/badmin hstartup'"; // FIXME FIXME FIXME FIXME FIXME 
                }
                else {
                    myargv[cc++] = "'. /etc/lsf.conf; . $LSF_CONFDIR/profile.lsf; badmin hstartup'";  // FIXME FIXME FIXME FIXME FIXME 
                }
            break;
            default:
                fprintf (stderr, "%s: %s %d\n", __func__, I18N (407, "Unknown operation  code"), opCode);
            return;
            break;
        }
    }

    myargv[cc] = NULL;

    execDaemon (getuid (), myargv);
    fflush (stderr);

    return;
}


int
execDaemon ( uid_t uid, char **myargv)
{
    pid_t childpid   = 0;
    LS_WAIT_T status = FALSE;

    if ((childpid = fork ()) < 0) {
        perror ("fork");
        return -1;
    }
    else if (childpid > 0) {
        while (wait (&status) != childpid) {
            ;
        }

        if (!WEXITSTATUS (status)) {
            return 0;
        }
        else {
            return -1;
        }
    }

    if (lsfSetUid (uid) < 0) {
        perror ("setuid");
        exit (-1);
    }

    lsfExecvp (myargv[0], myargv);
    perror (myargv[0]);
    exit (-2);

    return 255;
}


int
startup ( int argc, char **argv, int opCode )
{
    char *optName = NULL;
    int confirm   = FALSE;

    if( ls_initdebug (argv[0]) < 0 ) {
        ls_perror ("ls_initdebug");
        return -1;
    }

    if( getLSFenv() < 0 ) {
        return -1;
    }


    if (setStartupUid () < 0) {
        /* catgets 414 */
        fprintf (stderr, "%s\n", I18N (414, "Not authorized to start up as root"));  
        return -1;
    }

    confirm = TRUE;
    while ((optName = myGetOpt (argc, argv, "f|")) != NULL) {
        switch (optName[0]) {
            case 'f':
                confirm = FALSE;
            break;
            default:
                return -1;
            break;
        }
    }

    if (optind == argc) {
        startupLocalHost (opCode);
    }
    else if (optind == argc - 1 && strcmp (argv[optind], "all") == 0) {
        startupAllHosts (opCode, confirm);
    }
    else {
        for (; optind < argc; optind++) {
            startupRemoteHost (argv[optind], opCode, confirm);
        }
    }

    return 0;
}
