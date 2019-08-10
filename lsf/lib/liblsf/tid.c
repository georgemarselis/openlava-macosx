/*
 * $Id: lib.tid.c 397 2007-11-26 19:04:00Z mblack $ Copyright (C) 2007
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

#include <stdio.h>
#include <stdlib.h>

#include "lib/lib.h"
#include "lib/queue.h"
#include "lib/tid.h"

#define TID_BNUM   23
#define tid_index(x)   (x%TID_BNUM)

static struct tid *tid_buckets[TID_BNUM];

int
tid_register (pid_t taskid, int socknum, u_short taskPort, const char *host, bool_t doTaskInfo)
{
    int i = 0;
    struct tid *tidp = NULL;


    if ((tidp = malloc (sizeof (struct tid))) == NULL) {
        lserrno = LSE_MALLOC;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    tidp->rtid     = taskid;
    tidp->sock     = socknum;
    tidp->taskPort = taskPort;

    tidp->host = putstr_ (host);

    i = tid_index (taskid);
    tidp->link = tid_buckets[i];
    tid_buckets[i] = tidp;


    if (doTaskInfo) {
        lsQueueInit_ (&tidp->tMsgQ, NULL, tMsgDestroy_);
        if (tidp->tMsgQ == NULL) {
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
    }
    else {
        tidp->tMsgQ = NULL;
    }


    tidp->refCount = (doTaskInfo) ? 2 : 1;
    tidp->isEOF = (doTaskInfo) ? FALSE : TRUE;

    return 0;
}

int
tid_remove (unsigned int taskid)
{
    unsigned int i = tid_index (taskid);
    struct tid *p1 = NULL;
    struct tid *p2 = NULL;

    p1 = tid_buckets[i];

    while (p1 != NULL) {
        if (p1->rtid == taskid) {
            break;
        }
        p2 = p1;
        p1 = p2->link;
    }

    if (p1 == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    p1->refCount--;
    if (p1->refCount > 0) {
        return 0;
    }

    if (p1 == tid_buckets[i]) {
        tid_buckets[i] = p1->link;
    }
    else {
        p2->link = p1->link;
    }

    if (p1->tMsgQ) {
        lsQueueDestroy_ (p1->tMsgQ);
    }

    free( p1 );

    return 0;

}

struct tid *
tid_find (unsigned int taskid)
{
    unsigned int i = tid_index (taskid);
    struct tid *p1 = NULL;

    p1 = tid_buckets[i];
    while ( NULL != p1 ) {
        
        if (p1->rtid == taskid) {

            if (-1 == p1->sock ) {
                lserrno = LSE_LOSTCON;
                return NULL;
            }
            return p1;
        }

        p1 = p1->link;
    }

    lserrno = LSE_RES_INVCHILD;
    return NULL;
}

struct tid *
tidFindIgnoreConn_ ( unsigned int taskid)
{
    unsigned int i = tid_index (taskid);
    struct tid *p1 = NULL;

    p1 = tid_buckets[i];
    while (p1 != NULL) {
        if (p1->rtid == taskid) {
            return p1;
        }

        p1 = p1->link;
    }

    lserrno = LSE_RES_INVCHILD;
    return NULL;
}


void
tid_lostconnection (int socknum)
{
    struct tid *p1 = NULL;

    for ( unsigned int i = 0; i < TID_BNUM; i++) {
        p1 = tid_buckets[i];
        while ( NULL != p1 ) {
            if (p1->sock == socknum) {
                p1->sock = -1;
            }
            p1 = p1->link;
        }
    }

    return;
}

int
tidSameConnection_ (int socknum, unsigned int *ntids, unsigned int **tidArray)
{
    int tidCnt = 0;
    struct tid *p1 = 0;
    int *intp = 0;
    int i = 0;

    *tidArray = malloc (TID_BNUM * sizeof (int));

    if (!*tidArray) {
        lserrno = LSE_MALLOC;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    intp = *tidArray;
    for (i = 0; i < TID_BNUM; i++) {
        p1 = tid_buckets[i];
        while (p1 != NULL) {
            if (p1->sock == socknum) {
                *intp = p1->rtid;
                intp++;
                tidCnt++;
            }
            p1 = p1->link;
        }
    }

    if (ntids) {
        *ntids = tidCnt;
    }

    return 0;
}
