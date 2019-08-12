/* $Id: lib.osal.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/osal.h"
#include "lib/channel.h"


void osal_c_bullshit( void )
{
    assert( chanMaxSize );
    assert( lserrno );
    assert( INFINIT_LOAD );
    return;
}


int
osInit_( void )
{
    static char first = TRUE;
    if (first) {
        chanInit_ ();
        first = FALSE;
    }

    return 0;
}

char *
osPathName_ (char *pathname) // FIXME FIXME FIXME FIXME where are you going with this? investigate and delete if not used
{
    return pathname;
}

char *
osHomeEnvVar_ (void)
{
    return getenv ("HOME"); // FIXME FIXME FIXME FIXME put this in configure.ac
}

void
osConvertPath_ (char *pathname) // FIXME FIXME FIXME FIXME FIXME use strsubstitute function
{
    for( register unsigned int i = 0; pathname[i] != '\0' && i < MAX_PATH_LEN; i++) {
        if (pathname[i] == '\\') {
            pathname[i] = '/';
        }
    }

    return;
}

int
osProcAlive_( pid_t pid)
{
    return (kill (pid, 0));
}
