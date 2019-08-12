/* $Id: lib.debug.c 397 2007-11-26 19:04:00Z mblack $
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
#include <unistd.h>

#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/initenv.h"
#include "lib/syslog.h"
#include "lib/debug.h"



int
ls_initdebug ( const char *appName)
{
    char *logMask = malloc( sizeof( char ) * MAX_LSF_NAME_LEN + 1);
    // struct config_param *pPtr = NULL;

    assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain

    if (initenv_ (debParams, NULL) < 0) {
        return -1; // FIXME FIXME FIXME FIXME return meaningful *positive* return value
    }

    if (debParams[LSF_CMD_LOG_MASK].paramValue != NULL) {
        logMask = strdup( debParams[LSF_CMD_LOG_MASK].paramValue );
    }
    else {
        logMask = strdup( debParams[LSF_LOG_MASK].paramValue )  ;
    }

    if (appName == NULL) {
        ls_openlog ("lscmd", debParams[LSF_CMD_LOGDIR].paramValue,(debParams[LSF_CMD_LOGDIR].paramValue == NULL), logMask); // FIXME FIXME FIXME static string fix
    }
    else {
        if (strrchr (appName, '/') != 0) { // FIXME FIXME FIXME static string fix
            appName = strrchr (appName, '/') + 1; // FIXME FIXME FIXME static string fix
        }
        ls_openlog (appName, debParams[LSF_CMD_LOGDIR].paramValue, (debParams[LSF_CMD_LOGDIR].paramValue == NULL), logMask);
    }

    getLogClass_ (debParams[LSF_DEBUG_CMD].paramValue, debParams[LSF_TIME_CMD].paramValue);


    // pPtr is a constant, so we don't need to free it.
    // for (pPtr = debParams; pPtr->paramName != NULL; pPtr++) {
    //     FREEUP (pPtr->paramValue);
    // }

    free( logMask );
    lserrno = LSE_NO_ERR;
    return 0;

}
