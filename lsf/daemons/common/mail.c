/* $Id: mail.c 397 2007-11-26 19:04:00Z mblack $
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
#include <stdarg.h>

// #ifndef strchr
// #include <string.h>
// #endif


#include "daemons/daemons.h"
#include "lib/lib.h"
#include "lib/osal.h"

// #define NL_SETN         10

void
lsb_mperr (char *msg)
{
    static char fname[] = "lsb_mperr()";
    char *p;
    char err[MAXLINELEN];

    if (lsb_CheckMode)
        return;

    p = strchr (msg, '\n');
    if (p != NULL)  {
        *p = '\0';
    }

    if (strerror (errno) != NULL && errno >= 0) {
        strcpy (err, strerror (errno));
    }
    else {
        /* catgets 3200 */
        sprintf (err, "catgets 3200: %s: Unknown error", fname);
    }

    lsb_merr2 ("%s: %s\n", msg, err);
}

void
lsb_merr (char *s)
{
    char fname[] = "lsb_merr";
    FILE *mail;
    char *myhostnm;

    if (lsb_CheckMode) {
        return;
    }

    if ((myhostnm = ls_getmyhostname ()) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, fname, "ls_getmyhostname");
        if (masterme) {
            die (MASTER_FATAL);
        }
        else {
            die (SLAVE_FATAL);
        }
    }
    if (lsbManager == NULL || (getpwlsfuser_ (lsbManager)) == NULL)
        {
        if (lsbManager == NULL) {
            /* catgets 8601 */
            ls_syslog (LOG_ERR, "catgets 8601: %s: LSF administrator name is NULL", fname);
        }
        else
            /* catgets 8602 */
            ls_syslog (LOG_ERR, "catgets 8602: %s: Bad LSF administrator name \"<%s>\". Please select a valid user name.", fname, lsbManager);
        if (masterme)
            die (MASTER_FATAL);
        else
            die (SLAVE_FATAL);
        }
    mail = smail (lsbManager, myhostnm);

    if (masterme) {
        /* catgets 3201 */
        fprintf (mail, "Subject: mbatchd on %s: %s (catgets 3201)\n",  myhostnm, s);
    }
    else {
        /* catgets 3202 */
        fprintf (mail, "Subject: sbatchd on %s: %s (catgets 3202)\n", myhostnm, s); 
    }

    mclose (mail);
}

void
merr_user (char *user, char *host, char *msg, char *type)
{
    char fname[] = "merr_user";
    FILE *mail;
    char *myhostnm;

    if ((myhostnm = ls_getmyhostname ()) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, fname, "ls_getmyhostname");
        die (MASTER_FATAL);
        }

    mail = smail (user, host);
    /* catgets 3203 */
    fprintf (mail, "Subject: job %s report from %s (catgets 3203)\n", type, myhostnm);
    /* catgets 3204 */
    fprintf (mail, "catgets 3204\n\nDear %s,\n\n%s\n\n", user, msg);
    mclose (mail);
}

static void
addr_process (char *adbuf, char *user, char *tohost, char *spec)
{
    char *bp, *sp, *up;

    if (strrchr (user, '@') != NULL) {
        strcpy (adbuf, user);
        return;
    }

    bp = adbuf;
    for (sp = spec; *sp; sp++) {
        if ((*sp == '^') || (*sp == '!'))
            {
            switch (*++sp)
                {
                    case 'U':
                        for (up = user; *up;) {
                            *bp++ = *up++;
                        }
                    continue;
                    case 'H':
                        for (up = tohost; *up;) {
                            *bp++ = *up++;
                        }
                    continue;
                    default:
                        sp -= 1;

                }
            }
        *bp++ = *sp;
        }
    *bp = 0;
}

FILE *
smail (char *to, char *tohost)
{
    char fname[] = "smail";
    FILE *fmail = NULL;
    int maild[2]     = { 0, 0 };
    uid_t userid     = 0;
    pid_t pid        = 0;
    char *sendmailp  = NULL;
    char *osUserName = malloc( sizeof(char) * 500 + 1 );
    char *smcmd      = malloc( sizeof(char) * 500 + 1 );
    char *toaddr     = malloc( sizeof(char) * 500 + 1 );

    if( lsb_CheckMode ) {
        return stderr;
    }

    if ((to == NULL) || (tohost == NULL)) {
        /* catgets 8604 */
        ls_syslog (LOG_ERR, "catgets 8604: %s: Internal error: user name or host name is null", fname); 
        return stderr;
    }

    if (getOSUserName_ (to, osUserName, MAXLINELEN) != 0) {
        strncpy (osUserName, to, MAXLINELEN);
        assert( osUserName[MAXLINELEN - 1] ==  '\0' );
    }


    if (debug > 2) {
        return (stderr);
    }

    if (pipe (maild) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "pipe", osUserName);
        return stderr;
    }
    addr_process (toaddr, osUserName, tohost, daemonParams[LSB_MAILTO].paramValue);
    if (logclass & (LC_TRACE | LC_EXEC)) {
        ls_syslog (LOG_DEBUG1, "%s: user=%s host=%s toaddr=%s spec=%s", fname, osUserName, tohost, toaddr, daemonParams[LSB_MAILTO].paramValue);
    }
    switch (pid = fork ())
    {
        case 0:
        if (maild[0] != 0)
            {
            close (0);
            dup2 (maild[0], 0);
            close (maild[0]);
            }
        close (maild[1]);

        sendmailp = daemonParams[LSB_MAILPROG].paramValue;

        userid = geteuid ();
        chuser (getuid ());
        setuid (userid);

        sprintf (smcmd, "%s -oi -F%s -f%s %s", sendmailp, "'LSF'", lsbManager, toaddr);

        execle ("/bin/sh", "sh", "-c", smcmd, (char *) 0, environ);
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "execle", daemonParams[LSB_MAILPROG].paramValue);
        exit (-1);

        case -1:
        close (maild[1]);
        close (maild[0]);
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "fork", osUserName);
        return stderr;
        
        default:
        close (maild[0]);
    }
    fmail = fdopen (maild[1], "w");
    if (fmail == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "fdopen", osUserName);
        close (maild[1]);
        return stderr;
        }
    
    fprintf (fmail, "%s: %s\n", I18N_To, toaddr);
    fprintf (fmail, "%s: LSF <%s>\n", I18N_From, lsbManager);
    fflush (fmail);
    if (ferror (fmail))
        {
        fclose (fmail);
        ls_syslog (LOG_ERR, I18N_FUNC_S_S_FAIL_M, fname, "fprintf", "header", osUserName);
        return stderr;
        }

    // FIXME FIXME FIXME FIXME FIXME memory management!
    return fmail;
}

void
mclose (FILE * file)
{
    if (file != stderr) {
        (void) fclose (file);
    }
    else {
        (void) fflush (file);
    }
}