/* $Id: lsb.init.c 397 2007-11-26 19:04:00Z mblack $
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

#include <netdb.h>

#include "lsb/sub.h"
#include "lsb/lsb.h"
#include "lib/lib.h"


static int _lsb_conntimeout = DEFAULT_API_CONNTIMEOUT;
static int _lsb_recvtimeout = DEFAULT_API_RECVTIMEOUT;

int lsb_init (char *appName);

int
lsb_init (char *appName)
{
	static int lsbenvset = FALSE;
	char *logMask = NULL;

	if( lsbenvset ) {
		return 0;
	}


	if( initenv_( lsbParams, NULL ) < 0 )
	{
		lsberrno = LSBE_LSLIB;
		return -1;
	}

	if( lsbParams[LSB_API_CONNTIMEOUT].paramValue )
	{
		_lsb_conntimeout = atoi( lsbParams[LSB_API_CONNTIMEOUT].paramValue );
		if (_lsb_conntimeout < 0) {
			_lsb_conntimeout = DEFAULT_API_CONNTIMEOUT;
		}
	}

	if (lsbParams[LSB_API_RECVTIMEOUT].paramValue)
	{
		_lsb_recvtimeout = atoi (lsbParams[LSB_API_RECVTIMEOUT].paramValue);
		if (_lsb_recvtimeout < 0) {
			_lsb_recvtimeout = DEFAULT_API_RECVTIMEOUT;
		}
	}

	if (!lsbParams[LSB_SHAREDIR].paramValue)
	{
		lsberrno = LSBE_NO_ENV;
		return -1;
	}

	lsbenvset = TRUE;

	if (lsbParams[LSB_CMD_LOG_MASK].paramValue != NULL) {
		logMask = lsbParams[LSB_CMD_LOG_MASK].paramValue;
	}
	else {
		logMask = lsbParams[LSF_LOG_MASK].paramValue;
	}

	if (appName == NULL) {
		ls_openlog ("bcmd", lsbParams[LSB_CMD_LOGDIR].paramValue, (lsbParams[LSB_CMD_LOGDIR].paramValue == NULL), logMask);
	}
	else {
		ls_openlog (appName, lsbParams[LSB_CMD_LOGDIR].paramValue, (lsbParams[LSB_CMD_LOGDIR].paramValue == NULL), logMask);
	}

	getLogClass_ (lsbParams[LSB_DEBUG_CMD].paramValue, lsbParams[LSB_TIME_CMD].paramValue);

	if (bExceptionTabInit ())
	{
		lsberrno = LSBE_LSBLIB;
		return -1;
	}

	if (lsb_catch ("LSB_BAD_BSUBARGS", mySubUsage_)) {
		return -1;
	}

  return 0;

}
