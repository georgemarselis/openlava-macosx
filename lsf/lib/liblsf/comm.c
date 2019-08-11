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
#include "lib/taskid.h"


static char amSlave_ = FALSE;
static int msock_    = -1;
static int myrpid_   = -1;
// static int lserrno   =  0;

/*
* FIXME FIXME the following function conflicts with the standard library
* function unsetenv in <stdlib>
*/
/* void unsetenv (char *); */

void
ls_minit ( void )
{
    char *c = NULL;

    if ((c = getenv ("LSF_RPID")) == NULL) {
        fprintf (stderr, "ls_minit: Internal error- don't know my rpid\n");
        exit (-1);
    }
    else {
        const char env_name[] = "LSF_RPID";
        myrpid_ = atoi (c);
        unsetenv( env_name );
    }

    if ((c = getenv ("LSF_CALLBACK_SOCK")) == NULL) {
        fprintf (stderr, "ls_minit: Internal error-no connection to master\n");
        exit (-1);
    }
    else{
        msock_ = atoi (c);
        amSlave_ = TRUE;
        unsetenv ("LSF_CALLBACK_SOCK");
    }
}

int
ls_getrpid (void)
{
    if (amSlave_ == TRUE) {
        return myrpid_;
    }

    return 0;

}

long
ls_sndmsg (unsigned long taskid, const char *buf, size_t length)
{
    int sock = 0;
    struct tid *taskid_ = NULL;

    if (amSlave_ == TRUE) {
        sock = msock_;
    }
    else {
        if ((taskid_ = tid_find ( taskid ))  == NULL) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        sock = taskid_->sock;
    }

    //FIXME fix this function to not return -1?
    return (long) b_write_fix (sock, buf, length);
}


long
ls_rcvmsg (unsigned long taskid, const char *buf, size_t length)
{
    int sock = -1;

    if (amSlave_ == TRUE) {
        sock = msock_;
    }
    else {
        struct tid *taskid_ = tid_find( taskid );
        sock = taskid_->sock;
        if( sock < 0) {
            lserrno = LSE_RES_INVCHILD;
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }

    // FIXME fix this function to not return -1?
    return (long) b_read_fix (sock, strdup(buf), length);
}
