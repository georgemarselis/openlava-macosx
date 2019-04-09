/* $Id: lib.i18n.c 397 2007-11-26 19:04:00Z mblack $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// #include "lib/lproto.h"
#include "lsf.h"
#include "lib/i18n.h"
// #include "libint/lsi18n.h"

int _i18n_init (int modId)
{
    assert( modId );
    static int I18nInitFlag = 1;

    assert( I18nInitFlag < 0 );

    return 0;
}

int _i18n_end ( void )
{
    return 0;
}


void
_i18n_ctime_init (int catID)
{
    assert( catID ); // not used

    assert( lserrno ); // BULLSHIT OP, so the compiler will not complain
    assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain

    for ( int i = 0; i <= MAX_CTIME_FORMATID; i++) {
        i18nCTFormatStr[i] = i18n_ct_format[i];
    }
    return;
}

char *
_i18n_ctime (int catID, int formatID, const time_t * timer)
{
    static char timeStr[MAX_I18N_CTIME_STRING];

    assert( catID );

    strcpy (timeStr, ctime (timer));
    switch (formatID)
    {
        case CTIME_FORMAT_a_b_d_T_Y: {
            timeStr[24] = '\0';
            return timeStr;
        }
        break;
        case CTIME_FORMAT_b_d_T_Y: {
            timeStr[24] = '\0';
            return timeStr + 4;
        }
        break;
        case CTIME_FORMAT_a_b_d_T: {
            timeStr[19] = '\0';
            return timeStr;
        }
        break;
        case CTIME_FORMAT_b_d_H_M: {
            timeStr[16] = '\0';
            return timeStr + 4;
        }
        break;
        case CTIME_FORMAT_m_d_Y: {
            struct tm *timePtr;
            timePtr = localtime (timer);
            sprintf (timeStr, "%.2d/%.2d/%.4d", timePtr->tm_mon + 1, timePtr->tm_mday, timePtr->tm_year + 1900);
            return timeStr;
        }
        break;
        case CTIME_FORMAT_H_M_S: {
            struct tm *timePtr;
            timePtr = localtime (timer);
            sprintf (timeStr, "%.2d:%.2d:%.2d", timePtr->tm_hour, timePtr->tm_min, timePtr->tm_sec);
            return timeStr;
        }
        break;
        case CTIME_FORMAT_DEFAULT:
        default:
            return timeStr;
        break;
    }

    return NULL;
}

// FIXME FIXME
// variable format has to go
char *
_i18n_printf (const char *format, ...)
{
    static char i18nPrintBuffer[1024];
    va_list ap;

    va_start (ap, format);
    fprintf( stderr, "%s: format is: %s", __func__, format );
    vsprintf (i18nPrintBuffer, "%s", ap);     // FIXME FIXME FIXME FIXME FIXME 
                                              //    unroll with debugger
                                              //  also put it back the way it was
    va_end (ap);
    return i18nPrintBuffer;
}
