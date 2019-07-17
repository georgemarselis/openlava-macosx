/* $Id: lib.queue.c 397 2007-11-26 19:04:00Z mblack $
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
#include <stdlib.h>

#include "lib/lib.h"
#include "lib/queue.h"

// int
// lsQueueInit_ (struct lsQueue **head, int (*compareFunc) (char *data1, char *data2, int hint), void (*destroyFunc) (void *data))
int 
lsQueueInit_ (struct lsQueue **head, int (*compareFunc) (const char *val, struct lsRequest *reqEnt, int hint), lsQueueDestroyFuncType destroyFunc)
{
    struct lsQueueEntry *qPtr = NULL;

    qPtr = malloc (sizeof (struct lsQueueEntry));
    if (qPtr == NULL) {
        *head = NULL;
        lserrno = LSE_MALLOC;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    *head = malloc (sizeof (struct lsQueue));
    if (*head == NULL) {
        lserrno = LSE_MALLOC;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    (*head)->compareFunc = compareFunc;
    if (destroyFunc == NULL) {
        // (*head)->destroy = (lsQueueDestroyFuncType) free;
        (*head)->destroyFunc = free;
    }
    else{
        // (*head)->destroy = (lsQueueDestroyFuncType) destroyFunc;
        (*head)->destroyFunc = destroyFunc;
    }

    (*head)->start = qPtr;
    qPtr->forw = qPtr->back = qPtr;
    return 0;
}

int
lsQueueEntryAppend_ (struct lsQueueEntry *entry, struct lsQueue *head)
{
    struct lsQueueEntry *qPtr = NULL;

    if (head->start == NULL) {
        lserrno = LSE_MSG_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    qPtr             = head->start;
    entry->back      = qPtr->back;
    entry->forw      = qPtr;
    qPtr->back->forw = entry;
    qPtr->back       = entry;

    return 0;
}

int
lsQueueDataAppend_ ( const char *data, struct lsQueue *head)
{
    int rc = 0;
    struct lsQueueEntry *entry = NULL;

    entry = malloc (sizeof (struct lsQueueEntry));
    if (entry == NULL) {
        lserrno = LSE_MALLOC;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    rc = lsQueueEntryAppend_ (entry, head);
    entry->data = strdup( data );

    return rc;
}

struct lsQueueEntry *
lsQueueDequeue_ (struct lsQueue *head)
{
    struct lsQueueEntry *entry = NULL;
    struct lsQueueEntry *start = NULL;

    if (!head) {
        return NULL;
    }

    if (!LS_QUEUE_EMPTY (head))  {
            start = head->start;
            entry = start->forw;
            lsQueueEntryRemove_ (entry);
            return entry;
        }
    else {
        return NULL;
    }

    return NULL;
}

int
lsQueueEntryAddFront_ (struct lsQueueEntry *entry, struct lsQueue *head)
{
    struct lsQueueEntry *qPtr = NULL;

    if (head->start == NULL) {
        lserrno = LSE_MSG_SYS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    qPtr             = head->start;
    entry->forw      = qPtr->forw;
    entry->back      = qPtr;
    qPtr->forw->back = entry;
    qPtr->forw       = entry;

    return 0;
}

int
lsQueueDataAddFront_ ( const char *data, struct lsQueue *head)
{
    int rc = 0;
    struct lsQueueEntry *entry = NULL;

    entry = malloc (sizeof (struct lsQueueEntry));
    if (entry == NULL) {
        lserrno = LSE_MALLOC;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    rc = lsQueueEntryAddFront_ (entry, head);
    entry->data = data;

    return rc;
}

void
lsQueueEntryRemove_ (struct lsQueueEntry *entry)
{
    entry->back->forw = entry->forw;
    entry->forw->back = entry->back;
    entry->forw = entry->back = NULL;

    return;
}

void
lsQueueEntryDestroy_ (struct lsQueueEntry *entry, struct lsQueue *head)
{
    if (entry->forw != NULL) {
        lsQueueEntryRemove_ (entry);
    }

    if (entry->data) {
        if (head->destroyFunc) {
            (*(head->destroyFunc)) (strdup( entry->data ) );
        }
    }

    free (entry);

    return;
}

void
lsQueueEntryDestroyAll_ (struct lsQueue *head)
{
    struct lsQueueEntry *start    = NULL; 
    struct lsQueueEntry *qPtr     = NULL;
    struct lsQueueEntry *nextQPtr = NULL;

    start = head->start;
    for (qPtr = start->forw; qPtr != start; qPtr = nextQPtr) {
        nextQPtr = qPtr->forw;
        lsQueueEntryDestroy_ (qPtr, head);
    }

    return;
}

void
lsQueueDestroy_ (struct lsQueue *head)
{
    struct lsQueueEntry *start    = NULL; 
    struct lsQueueEntry *qPtr     = NULL; 
    struct lsQueueEntry *nextQPtr = NULL;

    start = head->start;
    for (qPtr = start->forw; qPtr != start; qPtr = nextQPtr) {
        nextQPtr = qPtr->forw;
        lsQueueEntryDestroy_ (qPtr, head);
    }
    free (head->start);
    free (head);

    return;
}


const char *
lsQueueDataGet_ (int i, struct lsQueue *head)
{
    int n = -1;
    struct lsQueueEntry *start = NULL;
    struct lsQueueEntry *qPtr  = NULL;

    if (head == NULL || i < 0) {
        return NULL;
    }

    start = head->start;
    for (qPtr = start->forw; qPtr != start; qPtr = qPtr->forw) {
        n++;
        if (i == n) {
            break;
        }
    }

    if (n < i) {
        return NULL;
    }
    else {
        return qPtr->data;
    }

    return NULL; // we are not supposed to be here
}

struct lsQueueEntry *
lsQueueSearch_ (int hint, const char *val, struct lsQueue *head)
{
    int rc    = 0;
    int found = 0;
    struct lsQueueEntry *start    = NULL;
    struct lsQueueEntry *qPtr     = NULL; 
    struct lsQueueEntry *nextQPtr = NULL;

    if( !val ){
        return NULL;
    }

    if( !head ){
        return NULL;
    }

    if( head->compareFunc == NULL ) {
        return NULL;
    }

    start = head->start;
    found = FALSE;

    for (qPtr = start->forw; qPtr != start; qPtr = nextQPtr) {

        nextQPtr = qPtr->forw;
        // rc = (*(head->compareFunc)) (val, qPtr->data, hint);
        rc = (*(head->compareFunc)) (val, (struct lsRequest *)strdup(qPtr->data), hint); // FIXME IFXME FIXME FIXME FIXME just fix this
        
        if (rc == 0) {
            found = TRUE;
            break;
        }
    }

    if (found == FALSE) {
        return NULL;
    }

    return qPtr;
}

void
lsQueueSetAdd_ (struct lsQueue *head1, struct lsQueue *head2, bool_t (*memberFunc) (struct lsQueueEntry *q, struct lsQueue *head))
{
    struct lsQueueEntry *start = NULL;
    struct lsQueueEntry *qEnt  = NULL;

    start = head2->start;
    for (qEnt = start->forw; qEnt != start; qEnt = qEnt->forw)  {

        if ((*memberFunc) (qEnt, head1) == FALSE) {
            lsQueueEntryRemove_ (qEnt);
            lsQueueEntryAppend_ (qEnt, head1);
        }
        else {
            lsQueueEntryRemove_ (qEnt);
        }
    }

    return;
}

void
lsQueueSort_ (struct lsQueue *head, int hint)
{
    int rc = 0;
    struct lsQueueEntry *start    = NULL;
    struct lsQueueEntry *q1       = NULL;
    struct lsQueueEntry *q2       = NULL;
    struct lsQueueEntry *nq1      = NULL;
    struct lsQueueEntry *selected = NULL;

    start = head->start;
    for (q1 = start->forw; q1 != start; q1 = nq1) {
        
        selected = q1;
        for (q2 = q1->forw; q2 != start; q2 = q2->forw) {

            // rc = (*head->compareFunc) ((char *) selected, (char *) q2, hint);
            rc = (*head->compareFunc) ((char *) selected, (struct lsRequest *) q2, hint); // FIXME IFXME FIXME FIXME FIXME just fix this
            if (rc > 0) {
                selected = q2;
            }
        }

            lsQueueEntryRemove_ (selected);
            lsQueueEntryAddFront_ (selected, head);

            nq1 = selected->forw;
    }

    return;
}

int
lsQueueDequeueData_ (struct lsQueue *head, char **data)
{
    struct lsQueueEntry *ent = lsQueueDequeue_ (head);

    if (ent == NULL) {
        return 0;
    }

    *data = strdup( ent->data );
    free (ent);
    return 1;
}

void
lsQueueIter_ (struct lsQueue *head, void (*func) (const char *data, void *hdata), void *hdata) 
{
    struct lsQueueEntry *start = NULL; 
    struct lsQueueEntry *qPtr  = NULL;

    start = head->start;

    for (qPtr = start->forw; qPtr != start; qPtr = qPtr->forw) {
        (*func) (qPtr->data, hdata);
    }

    return;
}
