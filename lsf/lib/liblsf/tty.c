/* $Id: lib.tty.c 397 2007-11-26 19:04:00Z mblack $
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

#include <termios.h>
#include <unistd.h>

#include "lib/lib.h"
#include "lib/term.h"
#include "lib/tty.h"

void ls_remtty( int ind, int enableIntSus )
{
    ttymode_( 1, ind, enableIntSus );

    return;
}

void ls_loctty( int ind )
{
    ttymode_( 0, ind, 0 );

    return;
}

void ttymode_ (int mode, int ind, int enableIntSus)
{
    static int lastmode;
    static int first = 1;

    static struct termios loxio;

    struct termios xio;
    tcflag_t tmpflag = 0;
    int i = 0;


    if (getpgrp () != tcgetpgrp( ind ) ) {
        return;
    }

    if (first && !mode) {
        return;
    }

    first = 0;
    switch (mode)
    {
        case 0: 
        {
            xio = loxio;
        }
        break;
        case 1:
        {
            if (!lastmode)
            {
                if (tcgetattr (ind, &loxio) == -1)
                {
                    perror ("ttymode_");
                    /* catgets 19 */
                    fprintf (stderr, "catgets 19: %s error %ld.", "tcgetattr", (long) ind);
                    fprintf (stderr, "\n");
                }
            }
            xio = loxio;

            xio.c_iflag &= (IXOFF | IXON | IXANY);

            tmpflag =
            xio.c_oflag & (NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY);
            xio.c_oflag &= (OPOST | OFILL | OFDEL | tmpflag);

#ifndef __CYGWIN__

 #ifndef __MACH__

#if defined(_AIX) || defined(__ANDROID__) || defined(__amigaos__) || defined(__bg__)  || defined(__hiuxmpp) || defined(_hpux) || defined(___hpux) || defined(__OS400__) || defined(sgi)  || defined(FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__bsdi__) || defined(__DragonFly__) || defined(EPLAN9) || defined(__QNX__) || defined(M_XENIX) || defined(sun) || defined(__sysv__) || defined(ultrix) || defined(__MVS__) 

            if (enableIntSus) {
                xio.c_lflag &= (XCASE | ISIG); // XCASE does not exist in Linux. See http://man7.org/linux/man-pages/man3/termios.3.html
            }
            else {
                xio.c_lflag &= (XCASE);
            }
#endif

#endif

#endif
            for (i = 0; i < NCCS; i++) {
                xio.c_cc[i] = _POSIX_VDISABLE;
            }


            if (enableIntSus)
            {
                xio.c_cc[VINTR] = loxio.c_cc[VINTR];
                xio.c_cc[VSUSP] = loxio.c_cc[VSUSP];
            }

            xio.c_cc[VMIN] = 0;
            xio.c_cc[VTIME] = 0;
        }
        break;

        default: 
        {
            fprintf( stderr, "%s: you are not supposed to be here\n", __func__ );
        }
    }

    if (tcsetattr (ind, TCSANOW, &xio) == -1)
    {
        if (errno != EINTR)
        {
            perror ("ttymode_");
            /* catgets 19 */
            fprintf (stderr, "catgets 19: %s error %ld.", "tcsetattr", (long) ind);
            fprintf (stderr, "\n");
        }
    }

    lastmode = mode;

    return;
}
