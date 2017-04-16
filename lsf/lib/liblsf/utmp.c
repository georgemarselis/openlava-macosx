/* $Id: lib.utmp.c 397 2007-11-26 19:04:00Z mblack $
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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "libint/lsi18n.h"

/*#ifndef UTMP_FILENAME
#define UTMP_FILENAME "/var/adm/utmpx"
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
*/


int createUtmpEntry (char *uname, pid_t job_pid, char *current_tty);
int removeUtmpEntry (pid_t job_pid);

int
createUtmpEntry (char *uname, pid_t job_pid, char *current_tty)
{

	int err = 0;
	assert( uname   );
	assert( job_pid );
	assert( current_tty );

	return err;

}


int
removeUtmpEntry (pid_t job_pid)
{
	assert( job_pid );
	return 0;
}
