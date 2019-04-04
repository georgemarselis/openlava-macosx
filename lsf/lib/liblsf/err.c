/*
 * Copyright (C) 2011 David Bigagli
 *
 * $Id: lib.err.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/err.h"


void
ls_errlog (FILE * fp, const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    verrlog_ (-1, fp, fmt, ap);
    va_end (ap);
}

/* err_str_()
 * %M appends the openlava ls_sysmg() message to the output,
 * %m appends the unix system sterror(errno) to the output.
 * GNU libc extensions specifies that sprintf() will print %m
 * as error number.
 */
// FIXME FIXME FIXME FIXME 
//  this shit needs undoing. no variable format specifiers.  
// const, left it out
char *
err_str_( unsigned int errnum, const char *fmt, char *buf)
{
    const char *b = NULL;
    char *f = NULL;

    b = strstr (fmt, "%M");
    if (b) {
        size_t foobar = 0;
        assert( b - fmt > 0 );
        foobar = (size_t)(b - fmt); // FIXME FIXME FIXME FIXME valgrind the shit out of this
        strncpy (buf, fmt, foobar);
        strcpy (buf + (b - fmt), ls_sysmsg ());
        strcat (buf + (b - fmt), b + 2);
        return (buf);
    }

    b = strstr (fmt, "%m");
    if (b) {
        int error = 0;
        size_t foobar = 0;
        assert( b - fmt > 0 );
        foobar = (size_t)(b - fmt); // FIXME FIXME FIXME FIXME valgrind the shit out of this
        strncpy (buf, fmt, foobar );
        f = buf + (b - fmt);
        assert( errnum <= INT_MAX ); // FIXME FIXME FIXME FIXME valgrind the shit out of this
        error = (int) errnum;
        if (strerror (error) == NULL) {
            sprintf (f, "error %d/errnum %ud", error, errnum);
        }
        else {
            int error = 0;
            assert( errnum <= INT_MAX );  // FIXME FIXME FIXME FIXME valgrind the shit out of this
            error = (int) errnum;
            strcpy (f, strerror (error));
        }

        f += strlen (f);
        strcat (f, b + 2);

        return buf;
    }

    return buf;
}

void
verrlog_ (int level, FILE * fp, const char *fmt, va_list ap)
{

    static int count       = 0;
    static time_t lastime  = 0;
    static time_t lastcall = 0;

    enum { BUFFSIZE = 16384 };
    static char *lastmsg = NULL;    // FIXME FIXME FIXME find out how large they have to be
    static char *tmpbuf  = NULL;    // FIXME FIXME FIXME find out how large they have to be
    static char *verBuf  = NULL;    // FIXME FIXME FIXME find out how large they have to be
    static char *buf     = NULL;    // FIXME FIXME FIXME find out how large they have to be
    time_t now;
    unsigned int save_errno = 0;

    assert( errno > 0 );

    save_errno = (unsigned int) errno;

    lastmsg = malloc( BUFFSIZE * sizeof( char ) + 1 );  // FIXME FIXME FIXME find out how large they have to be
    verBuf  = malloc( BUFFSIZE * sizeof( char ) + 1 );  // FIXME FIXME FIXME find out how large they have to be
//buf       = malloc( BUFFSIZE * sizeof( char ) + 1 );  // FIXME FIXME FIXME find out how large they have to be

    // FIXME FIXME FIXME FIXME
    // The correct solution to the problem below is to convert the varuable argument list to char * and copy it to tmpbuf
    // What are doing instead is creating a string out of the variable-length argument list.
    //      Edit: we should also change the function prototype completely, to something like:
    //      verrlog_ (int level, FILE * fp, const char *fmt, unsigned int argcount, ... )
    //      and then fix the above
    tmpbuf = err_str_(save_errno, fmt, tmpbuf); // FIXME FIXME FIXME malloc error

    tmpbuf  = realloc( tmpbuf, BUFFSIZE * sizeof( char ) + strlen( (char *) ap ) * sizeof( char ) + 1 );    // FIXME FIXME FIXME find out how large they have to be
    strcat( tmpbuf, (char *) ap );
    buf     = realloc( buf, strlen( tmpbuf ) + BUFFSIZE * sizeof( char ) + 1 );         // FIXME FIXME FIXME FIXME FIXME FIXME serious wtf
    vsnprintf( buf, strlen( tmpbuf ), "%s", ap );                                       // FIXME FIXME FIXME FIXME FIXME 
    // vsnprintf (buf, sizeof( buf ), err_str_ (save_errno, fmt, tmpbuf), ap);          // FIXME FIXME FIXME FIXME FIXME 
                                                                                        //  unroll this shit using the debugger
    now = time (0);

    if (lastmsg[0] && (strcmp (buf, lastmsg) == 0) && (now - lastime < 600)) {
        count++;
        lastcall = now;
        return;
    }
    else {

        if (count) {
            fprintf (fp, "%.15s %d ", ctime (&lastcall) + 4, (int) getpid ());
            fprintf (fp, "Last message repeated %d times\n", count);
        }
        
        fprintf (fp, "%.15s %d ", ctime (&now) + 4, (int) getpid ());
    }

    if (level >= 0) {
        snprintf (verBuf, strlen (verBuf), "%d %d %s", level, OPENLAVA_VERSION, buf);
    }
    else {
        snprintf (verBuf, strlen (verBuf), "%d %s", OPENLAVA_VERSION, buf);
    }

    fputs (verBuf, fp);
    putc ('\n', fp);
    fflush (fp);
    strcpy (lastmsg, buf);
    count = 0;
    lastime = now;

    free( lastmsg ); free( verBuf ); free( buf ); free( tmpbuf );

    return;
}

char *
ls_sysmsg (void)
{
    int save_errno = errno;
    static int ls_nerr = LSE_NERR;
    char *buf = NULL;
    char *please_do_not_smash_me = NULL;

    buf = malloc( sizeof(char) * 256 + 1 );
    please_do_not_smash_me = malloc( sizeof(char) * INT_MAX + 1 );
    memset( buf, '\0', strlen( buf ) );
    memset( please_do_not_smash_me, '\0', strlen( please_do_not_smash_me ) );

    if (lserrno >= ls_nerr || lserrno < 0) {
        sprintf (buf, "Error %d", lserrno);
        return buf;
    }

    if (LSE_SYSCALL (lserrno)) {
        if (strerror (save_errno) != NULL && save_errno > 0) {
            sprintf (buf, "%s: %s", ls_errmsg[lserrno], strerror (save_errno));
        }
        else {
            sprintf (buf, "%s: unknown system error %d", ls_errmsg[lserrno], save_errno);
        }

        return buf;
    }

    strncpy( please_do_not_smash_me, ls_errmsg[lserrno], strlen( ls_errmsg[lserrno] ) );

    return please_do_not_smash_me;
}

void
ls_perror( char *usrMsg )
{
    if (usrMsg) {
        fputs (usrMsg, stderr);
        fputs (": ", stderr);
    }

    fputs (ls_sysmsg (), stderr);
    putc ('\n', stderr);

    return;
}
