/* $Id: lsb.signals.c 397 2007-11-26 19:04:00Z mblack $
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
#include <string.h>
#include <pwd.h>

#include "lib/syslog.h"
#include "lsb/lsb.h"
#include "lsb/signals.h"

int
sigNameToValue_ ( const char *sigString)
{
    int sigValue = 0;

    if ((sigString == NULL) || (sigString[0] == '\0')) {
        return INFINIT_INT;
    }

    if ((sigValue = getSigVal (sigString)) > 0) {
        return sigValue;
    }

    for ( unsigned int i = 0; i < LSB_SIG_NUM; i++) {
        if (strcmp (lsbSigSymbol[i], sigString) == 0) {
            return lsbSig_map[i];
        }
    }

    return INFINIT_INT;
}


const char *
getLsbSigSymbol (int sigValue)
{
    static char symbol[30]; // FIXME FIXME FIXME FIXME why 30 chars only?

    // symbol[0] = '\0';

    if (sigValue >= 0) {
        return getSigSymbol (sigValue);
    }
    // else { // change the sig value from negative to positive // FIXME FIXME FIXME FIXME turn sigValue to unsigned int
    //     if (-sigValue < LSB_SIG_NUM)
    //         strcpy (symbol, lsbSigSymbol[-sigValue]);
    //     else
    //         strcpy (symbol, "UNKNOWN");
    //     return symbol;
    // }
    else { 
        if (-sigValue < LSB_SIG_NUM ) {
            strcpy (symbol, lsbSigSymbol[-sigValue]);
        }
        else {
            strcpy (symbol, "UNKNOWN");
        }
        return symbol;
    }

    return NULL;
}

int
getDefSigValue_ (int sigValue, const char *actCmd)
{
    int defSigValue;

    if (sigValue >= 0) {
        return sigValue;
    }

    switch (sigValue) { // NOTE NOTE NOTE NOTE chaeck out all the signal names 
        case SIG_CHKPNT:
        case SIG_CHKPNT_COPY:
        case SIG_DELETE_JOB:
            return sigValue;

        break;
        case SIG_SUSP_USER:
        case SIG_SUSP_LOAD:
        case SIG_SUSP_WINDOW:
        case SIG_SUSP_OTHER:

        case SIG_RESUME_USER:
        case SIG_RESUME_LOAD:
        case SIG_RESUME_WINDOW:
        case SIG_RESUME_OTHER:

        case SIG_TERM_USER:
        case SIG_TERM_LOAD:
        case SIG_TERM_WINDOW:
        case SIG_TERM_OTHER:
        case SIG_TERM_RUNLIMIT:
        case SIG_TERM_DEADLINE:
        case SIG_TERM_PROCESSLIMIT:
        case SIG_TERM_CPULIMIT:
        case SIG_TERM_MEMLIMIT:
        case SIG_TERM_FORCE:
            // if ((actCmd == NULL) || (actCmd[0] == '\0')) {
            if( NULL == actCmd )  {
                return defaultSigValue[-sigValue];
            }
            else if ((defSigValue = sigNameToValue_ (actCmd)) == INFINIT_INT) {
                return sigValue;
            }
            else {
                if ((defSigValue == SIG_CHKPNT) || (defSigValue == SIG_CHKPNT_COPY)) {
                    return sigValue;
                }
                else {
                    return defSigValue;
                }
            }
        break;
        default:
            ls_syslog( LOG_ERR, "I don't think we are supposed to be at the default for %s(): sigvalue: %d", __func__, sigValue );
        break;
    }

    return sigValue;
}

bool
isSigTerm (int sigValue)
{
    switch (sigValue) {
        case SIG_DELETE_JOB:
        case SIG_TERM_USER:
        case SIG_TERM_LOAD:
        case SIG_TERM_WINDOW:
        case SIG_TERM_OTHER:
        case SIG_TERM_RUNLIMIT:
        case SIG_TERM_DEADLINE:
        case SIG_TERM_PROCESSLIMIT:
        case SIG_TERM_CPULIMIT:
        case SIG_TERM_MEMLIMIT:
        case SIG_TERM_FORCE:
        case SIG_KILL_REQUEUE: 
            return true;
        break;
        default:
            return false;
        break;
    }

    return false;
}

bool
isSigSusp (int sigValue)
{
    switch (sigValue) {
        case SIG_SUSP_USER:
        case SIG_SUSP_LOAD:
        case SIG_SUSP_WINDOW:
        case SIG_SUSP_OTHER:
            return true;
        break;
        default:
            return false;
        break;
    }

    return false;
}

int
terminateWhen_ (int *sigMap, const char *name)
{
    if (strcmp (name, "WINDOW") == 0) {
        if (sigMap[-SIG_SUSP_WINDOW] != 0) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (strcmp (name, "USER") == 0) {
        if (sigMap[-SIG_SUSP_USER] != 0) {
            return true;
        }
        else {
            return false;
        }
    }
    else if (strcmp (name, "LOAD") == 0) {
        if (sigMap[-SIG_SUSP_LOAD] != 0) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    return false;
}
