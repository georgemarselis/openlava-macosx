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


int
errnoEncode_ (int eno)
{
    int NERRNO_MAP  = ELASTVALUE - 1; // ignore 0th value

    if (eno < 0) {
        return eno;
    }
    assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain
    // for ( int i = 0; i < NERRNO_MAP; i++) // FIXME FIXME FIXME FIXME this has to be revisted
    /* they are trying to encode the error into a number?
     * 
    */
    // {
    //     if ( ls_errmsg[i] == eno) { // ls_errmsg is from lib/err.h
    //         return i;
    //     }
    // }

    if (eno >= NERRNO_MAP) {
        return eno;
    }
    else {
        return 0;
    }

    return INT_MAX;
}

int
errnoDecode_ (int eno)
{
    int NERRNO_MAP  = ELASTVALUE - 1;

    assert( lserrno ); // BULLSHIT CODE so the compiler will not generate a warning.

    if (eno < 0) {
        return eno;
    }

    if (eno >= NERRNO_MAP) {
        if (strerror (eno) != NULL) {
            return eno;
        }
        else {
            return 0;
        }
    }

    // return ls_errmsg[eno];  // ls_errmsg is from lib/err.h
    return INT_MAX;
}
