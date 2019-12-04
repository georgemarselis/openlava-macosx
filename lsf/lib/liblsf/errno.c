/* $Id: lib.errno.c 397 2007-11-26 19:04:00Z mblack $
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

#include <signal.h>
#include <errno.h>
#include <limits.h>

#include "lib/lib.h"
#include "lib/err.h"


unsigned int
errnoEncode_ ( unsigned int errnumber)
{
    unsigned int NERRNO_MAP  = ELASTVALUE - 1; // ignore 0th value

    if (errnumber == 255) {
        return errnumber;
    }
    // for ( int i = 0; i < NERRNO_MAP; i++) // FIXME FIXME FIXME FIXME this has to be revisted
    /* they are trying to encode the error into a number?
     * 
    */
    // {
    //     if ( ls_errmsg[i] == eno) { // ls_errmsg is from lib/err.h
    //         return i;
    //     }
    // }

    if (errnumber >= NERRNO_MAP) {
        return errnumber;
    }
    else {
        return 0;
    }

    return INT_MAX;
}

unsigned int
errnoDecode_ ( unsigned int errnumber )
{
    unsigned int NERRNO_MAP  = ELASTVALUE - 1;

    if (errnumber == 255 ) {
        return errnumber;
    }

    if (errnumber >= NERRNO_MAP) {
        assert( errnumber < INT_MAX );
        if (strerror( (int) errnumber) != NULL) {
            return errnumber;
        }
        else {
            return 0;
        }
    }

    // return ls_errmsg[eno];  // ls_errmsg is from lib/err.h
    return INT_MAX;
}
