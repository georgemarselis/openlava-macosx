/*
 * Copyright (C) 2011-2012 David Bigagli
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

#include <unistd.h>
#ifndef __USE_MISC
#define __USE_MISC // FIXME FIXME FIXME related to syslog.h via vsyslog(); not quite sure if portable
#endif
#include <syslog.h>
#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>

#include "lib/syslog.h"
// #include "lib/lproto.h"
#include "lib/osal.h"
#include "lib/lib.h"
#include "lib/err.h"
#include "lib/host.h"
#include "lib/misc.h"
#include "lib/words.h"
#include "lsf.h"


char *
argvmsg_ (int argc, char **argv)
{
    static char avbuffer[128];
    char *bp = avbuffer;
    const char *ap = NULL;
    int i = 0;

    ap = argv[0]; // FIXME FIXME FIXME mark with label what is argv[0]
    while( ( i < argc ) && ( bp < ( avbuffer + sizeof( avbuffer ) - 1 ) ) ) {
        if (!*ap) {
            i++;
            if( ( i < argc ) && ( ( ap = argv[i] ) != NULL ) ) {
                *bp = ' ';
                bp++;
            }
        }
        else {
            *bp = *ap;
            bp++;
            ap++;
        }
    }

    *bp = '\0';
    return avbuffer;
}

void
ls_openlog (const char *ident, const char *path, int use_stderr, const char *logMask)
{
    const char *msg = NULL;

    // strncpy (logident, ident, 9); // FIXME FIXME FIXME why only 9 characters?
    // logident[9] = '\0';    
    logident = strdup( ident ); // FIXME FIXME FIXME why only 9 characters?
    logmask = getLogMask (&msg, logMask);

    if (use_stderr) {
        log_dest = LOGTO_STDERR;
        return;
    }

    if (path && *path) {
        char *myname = NULL;
        FILE *lfp = NULL;
        struct stat st;

        if ((myname = ls_getmyhostname ()) == NULL) {
            // goto syslog;
            log_dest = LOGTO_SYS;
            logfile[0] = '\0';

            openlog ( ident, LOG_PID, LOG_DAEMON);
            setlogmask (logmask);

            if (msg != NULL) {
                ls_syslog (LOG_ERR, "%s", msg);
            }

            return;
        }

        sprintf (logfile, "%s/%s.log.%s", path, ident, myname);

        if (lstat (logfile, &st) < 0) {
            if (errno == ENOENT) {
                if (openLogFile (ident, myname) == 0) {
                    if (msg != NULL) {
                        ls_syslog (LOG_ERR, "%s", msg);
                    }

                    return;
                }
            }
            else {
                sprintf (logfile, "%s/%s.log.%s", LSTMPDIR, ident, myname);
                if (lstat (logfile, &st) < 0) {
                    if (errno == ENOENT) {
                        if ((lfp = fopen (logfile, "a")) != NULL) {
                            fclose (lfp);
                            if (!strcmp (ident, "res") || (logmask >= LOG_UPTO (LOG_DEBUG) && logmask <= LOG_UPTO (LOG_DEBUG3))) {
                                chmod (logfile, 0666); // FIXME FIXME FIXME change 0666 with label // FIXME FIXME FIXME why do they set it ugo+rw?
                            }
                            else {
                                chmod (logfile, 0644); // FIXME FIXME FIXME change 0644 with label
                            }
                            log_dest = LOGTO_FILE;
                            if (msg != NULL)  {
                                ls_syslog (LOG_ERR, "%s", msg);
                            }
                            return;
                        }
                    }
                }
                else if (S_ISREG (st.st_mode) && st.st_nlink == 1) {
                    if ((lfp = fopen (logfile, "a")) != NULL) {
                        fclose (lfp);
                        if (!strcmp (ident, "res") || (logmask >= LOG_UPTO (LOG_DEBUG) && logmask <= LOG_UPTO (LOG_DEBUG3))) {
                            chmod (logfile, 0666);
                        }
                        else {
                            chmod (logfile, 0644);
                        }
                        log_dest = LOGTO_FILE;
                        if (msg != NULL) {
                            ls_syslog (LOG_ERR, "%s", msg);
                        }
                        return;
                    }
                }
            }
        }
        else if (S_ISREG (st.st_mode) && st.st_nlink == 1) {
            if (openLogFile (ident, myname) == 0) {
                if (msg != NULL) {
                    ls_syslog (LOG_ERR, "%s", msg);
                }

                return;
            }
        }
    }

// syslog:     // FIXME FIXME FIXME FIXME substitute where appropriate and get rid of it

    log_dest = LOGTO_SYS;
    logfile[0] = '\0';

    openlog ( ident, LOG_PID, LOG_DAEMON);
    setlogmask (logmask);

    if (msg != NULL) {
        ls_syslog (LOG_ERR, "%s", msg);
    }

    return;
}

int
openLogFile (const char *ident, const char *myname)
{
    FILE *lfp = NULL;
    struct stat st;
    const char APPEND_AT_END[] = "a";

    if( ( lfp = fopen( logfile, APPEND_AT_END ) ) == NULL ) {

        sprintf (logfile, "%s/%s.log.%s", LSTMPDIR, ident, myname);
        if (lstat (logfile, &st) < 0) {
            if (errno == ENOENT) {
                if ((lfp = fopen (logfile, "a")) == NULL) {
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            }
            else {
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }
        else if (S_ISREG (st.st_mode) && st.st_nlink == 1) {

            if ((lfp = fopen (logfile, "a")) == NULL) {
                return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
            }
        }
        else {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    if (lfp != NULL) {
        fclose (lfp);

        if (!strcmp (ident, "res") || (logmask >= LOG_UPTO (LOG_DEBUG) && logmask <= LOG_UPTO (LOG_DEBUG3))) {
            chmod (logfile, 0666); // FIXME FIXME FIXME replace 0666 with appropriate O_LABEL | O_LABEL
        }
        else {
            chmod (logfile, 0644); // FIXME FIXME FIXME replace 0666 with appropriate O_LABEL | O_LABEL
        }
        log_dest = LOGTO_FILE;
        return 0;
    }

    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
}

void
ls_syslog (int level, const char *fmt, ...) // FIXME FIXME convert variable argument list to static
{
    unsigned int save_errno = (unsigned int) errno;
    va_list ap; // FIXME FIXME full initilization
    // static char lastMsg[16384];
    // static int counter = 0;
    char *buf = malloc( sizeof( char ) * 1024 + 1 );

    va_start (ap, fmt);

    if (log_dest == LOGTO_STDERR) {

        if ((logmask & LOG_MASK (level)) != 0) {
            assert( save_errno <= INT_MAX );
            errno = (int) save_errno;
            verrlog_ (level, stderr, fmt, ap);
        }

    }
    else if (logfile[0]) { // FIXME FIXME FIXME replace logfile[0] with appropriate label

        if ((logmask & LOG_MASK (level)) != 0) {

           FILE *lfp = NULL;
           struct stat st;

           if (lstat (logfile, &st) < 0) {

                if (errno == ENOENT) {

                    if ((lfp = fopen (logfile, "a")) == NULL) {

                        char *kot = malloc( sizeof( char ) * strlen( buf ) + 1 );

                        if (log_dest == LOGTO_FILE) {
                            log_dest = LOGTO_SYS;
                            openlog (logident, LOG_PID, LOG_DAEMON);
                            setlogmask (logmask);
                        }
                        
                        if (level > LOG_DEBUG) {
                            level = LOG_DEBUG;
                        }

                        sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
                        kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
                        strcat( kot, (char *) ap );
                        syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throing errors. must revisit function
                        vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
                        closelog ();
                        free( kot );
                    }
                }
                else {

                    err_str_ (save_errno, fmt, buf);
                    char *kot = malloc( sizeof( char ) * strlen( buf ) + 1 );
                    if (log_dest == LOGTO_FILE) {
                        log_dest = LOGTO_SYS;
                        openlog (logident, LOG_PID, LOG_DAEMON);
                        setlogmask (logmask);
                    }
                    
                    if (level > LOG_DEBUG) {
                        level = LOG_DEBUG;
                    }

                    err_str_ (save_errno, fmt, buf); // this maybe wrong
                    sprintf( kot, "%s", buf ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
                    kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
                    strcat( kot, (char *) ap );
                    syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throwing errors. must revisit function
                    vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
                    closelog( );
                    free( kot );
                }
            }
            else if (!(S_ISREG (st.st_mode) && st.st_nlink == 1)) {

                char *kot = malloc( sizeof( char ) * strlen( err_str_ (save_errno, fmt, buf) ) + 1 );
                if (log_dest == LOGTO_FILE) {

                    log_dest = LOGTO_SYS;
                    openlog (logident, LOG_PID, LOG_DAEMON);
                    setlogmask (logmask);
                }
                
                if (level > LOG_DEBUG) {
                    level = LOG_DEBUG;
                }
                    
                sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
                kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
                strcat( kot, (char *) ap );
                syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throing errors. must revisit function
                vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
                closelog ();
                free( kot );
            }
            else {

                if ((lfp = fopen (logfile, "a")) == NULL) {

                    char *kot = malloc( sizeof( char ) * strlen( err_str_ (save_errno, fmt, buf) ) + 1 );
                    if (log_dest == LOGTO_FILE) {
                        log_dest = LOGTO_SYS;
                        openlog (logident, LOG_PID, LOG_DAEMON);
                        setlogmask (logmask);
                    }
                    
                    if (level > LOG_DEBUG) {
                        level = LOG_DEBUG;
                    }

                    sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
                    kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
                    strcat( kot, (char *) ap );
                    syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throing errors. must revisit function
                    vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
                    closelog ();
                    free( kot );
                }
            }

            if (log_dest == LOGTO_SYS) {

                closelog ();
                log_dest = LOGTO_FILE;
            }

            assert( save_errno <= INT_MAX );
            errno = (int) save_errno;
            verrlog_ (level, lfp, fmt, ap);
            fclose (lfp);
        }
    }
    else if ((logmask & LOG_MASK (level)) != 0) {
        char *kot = malloc( sizeof( char ) * strlen( err_str_ (save_errno, fmt, buf) ) + 1 );
        if (level > LOG_DEBUG) {
            level = LOG_DEBUG;
        }
        sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
        kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
        strcat( kot, (char *) ap );
        syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throwing errors. must revisit function
        vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
        closelog ();
        free( kot );
    }

    free( buf );
    va_end (ap);
}

void
ls_closelog (void)
{
    if (log_dest == LOGTO_SYS) {
        closelog ();
    }
}

int
ls_setlogmask ( const int maskpri)
{
    int oldmask = logmask;

    logmask = maskpri;
    oldmask = setlogmask (logmask);

    return oldmask;
}

int
getLogMask ( const char **msg, const char *logMask)
{
    static char msgbuf[MAX_LINE_LEN];

    // *msg = NULL;
    assert( *msg );

    if (logMask == NULL) {
        // return LOG_UPTO (DEF_LOG_MASK);
        return ( 1 << (( DEF_LOG_MASK + 1 ) - 1));
    }
#ifdef LOG_ALERT
    if (strcmp (logMask, "LOG_ALERT") == 0) {
        return LOG_UPTO (LOG_ALERT);
    }
#endif

#ifdef LOG_SALERT
    if (strcmp (logMask, "LOG_SALERT") == 0) {
        return LOG_UPTO (LOG_SALERT);
    }
#endif

#ifdef LOG_EMERG
    if (strcmp (logMask, "LOG_EMERG") == 0) {
        return LOG_UPTO (LOG_EMERG);
    }
#endif

    if (strcmp (logMask, "LOG_ERR") == 0) {
        return LOG_UPTO (LOG_ERR);
    }

#ifdef LOG_CRIT
    if (strcmp (logMask, "LOG_CRIT") == 0) {
        return LOG_UPTO (LOG_CRIT);
    }
#endif

    if (strcmp (logMask, "LOG_WARNING") == 0) {
        return LOG_UPTO (LOG_WARNING);
    }

    if (strcmp (logMask, "LOG_NOTICE") == 0) {
        return LOG_UPTO (LOG_NOTICE);
    }

    if (strcmp (logMask, "LOG_INFO") == 0) {
        return LOG_UPTO (LOG_INFO);
    }

    if (strcmp (logMask, "LOG_DEBUG") == 0) {
        return LOG_UPTO (LOG_DEBUG);
    }

    if (strcmp (logMask, "LOG_DEBUG1") == 0) {
        return LOG_UPTO (LOG_DEBUG1);
    }

    if (strcmp (logMask, "LOG_DEBUG2") == 0) {
        return LOG_UPTO (LOG_DEBUG2);
    }

    if (strcmp (logMask, "LOG_DEBUG3") == 0) {
        return LOG_UPTO (LOG_DEBUG3);
    }

    sprintf (msgbuf, "Invalid log mask %s defined, default to %s", logMask, DEF_LOG_MASK_NAME);

    *msg = msgbuf;

    return ( 1 << ( ( DEF_LOG_MASK + 1 ) - 1 ) );
}

int
getLogClass_ ( const char *lsp, const char *tsp)
{
    char *word = NULL;
    int class = 0;

    timinglevel = 0;
    logclass = 0;

    if (tsp != NULL && isint_ (tsp)) {
        timinglevel = atoi (tsp);
    }

    while (lsp != NULL && (word = getNextWord_ (&lsp))) {
        if (strcmp (word, "LC_SCHED") == 0) {
            class |= LC_SCHED;
        }
        if (strcmp (word, "LC_PEND") == 0) {
            class |= LC_PEND;
        }
        if (strcmp (word, "LC_JLIMIT") == 0) {
            class |= LC_JLIMIT;
        }
        if (strcmp (word, "LC_EXEC") == 0) {
            class |= LC_EXEC;
        }
        if (strcmp (word, "LC_TRACE") == 0) {
            class |= LC_TRACE;
        }
        if (strcmp (word, "LC_COMM") == 0) {
            class |= LC_COMM;
        }
        if (strcmp (word, "LC_XDR") == 0) {
            class |= LC_XDR;
        }
        if (strcmp (word, "LC_CHKPNT") == 0) {
            class |= LC_CHKPNT;
        }
        if (strcmp (word, "LC_FILE") == 0) {
            class |= LC_FILE;
        }
        if (strcmp (word, "LC_AUTH") == 0) {
            class |= LC_AUTH;
        }
        if (strcmp (word, "LC_HANG") == 0) {
            class |= LC_HANG;
        }
        if (strcmp (word, "LC_SIGNAL") == 0) {
            class |= LC_SIGNAL;
        }
        if (strcmp (word, "LC_PIM") == 0) {
            class |= LC_PIM;
        }
        if (strcmp (word, "LC_SYS") == 0) {
            class |= LC_SYS;
        }
        if (strcmp (word, "LC_LOADINDX") == 0) {
            class |= LC_LOADINDX;
        }
        if (strcmp (word, "LC_JGRP") == 0) {
            class |= LC_JGRP;
        }
        if (strcmp (word, "LC_JARRAY") == 0) {
            class |= LC_JARRAY;
        }
        if (strcmp (word, "LC_MPI") == 0) {
            class |= LC_MPI;
        }
        if (strcmp (word, "LC_ELIM") == 0) {
            class |= LC_ELIM;
        }
        if (strcmp (word, "LC_M_LOG") == 0) {
            class |= LC_M_LOG;
        }
        if (strcmp (word, "LC_PERFM") == 0) {
            class |= LC_PERFM;
        }
    }
    logclass = class;

    return 0;
}

void
ls_closelog_ext (void)
{
    // logfile[0] = '\0'; // FIXME FIXME FIXME label what logfile[0] is
    memset( logfile, '\0', strlen( logfile ) );
    return;
}
