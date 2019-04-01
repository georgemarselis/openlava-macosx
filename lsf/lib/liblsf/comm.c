/*
* $Id: lib.comm.c 397 2007-11-26 19:04:00Z mblack $ Copyright (C) 2007
* Platform Computing Inc
* 
* This program is free software; you can redistribute it and/or modify it under
* the terms of version 2 of the GNU General Public License as published by
* the Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
* 
* You should have received a copy of the GNU General Public License along with
* this program; if not, write to the Free Software Foundation, Inc., 51
* Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
* 
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lsf.h"
#include "lib/lib.h"
#include "lib/comm.h"
#include "lib/rdwr.h"
#include "lib/tid.h"


static char amSlave_ = FALSE;
static int msock_    = -1;
static int myrpid_   = -1;
static int lserrno   =  0;

/*
* FIXME FIXME the following function conflicts with the standard library
* function unsetenv in <stdlib>
*/
/* void unsetenv (char *); */

void
ls_minit ( void )
{
	char *c;

	if ((c = getenv ("LSF_RPID")) == NULL)
	{
		fprintf (stderr, "ls_minit: Internal error- don't know my rpid\n");
		exit (-1);
	}
	else
	{

		const char env_name[] = "LSF_RPID";
		myrpid_ = atoi (c);
		unsetenv( env_name );
	}

	if ((c = getenv ("LSF_CALLBACK_SOCK")) == NULL)
	{
		fprintf (stderr, "ls_minit: Internal error-no connection to master\n");
		exit (-1);
	}
	else
	{
		msock_ = atoi (c);
		amSlave_ = TRUE;
		unsetenv ("LSF_CALLBACK_SOCK");
	}
}

int
ls_getrpid (void)
{
	if (amSlave_ == TRUE) {
		return (myrpid_);
	}

	return (0);

}

long
ls_sndmsg (int tid, char *buf, size_t count)
{
	int sock;
	struct tid *tid_;

	if (amSlave_ == TRUE)
	{
		sock = msock_;
	}
	else
	{
		if ((tid_ = tid_find (tid)) == NULL)
		{
			return (-1);
		}
		sock = tid_->sock;
	}

	//FIXME fix this function to not return -1?
	return (long) b_write_fix (sock, buf, count);
}


long
ls_rcvmsg (int tid, char *buf, size_t count)
{
	int sock;

	if (amSlave_ == TRUE)
	{
		sock = msock_;
	}
	else
	{
		struct tid *tid_ = tid_find( tid );
		sock = tid_->sock;
		if( sock < 0)
		{
			lserrno = LSE_RES_INVCHILD;
			return (-1);
		}
	}

	// FIXME fix this function to not return -1?
	return (long) b_read_fix (sock, buf, count);
}
