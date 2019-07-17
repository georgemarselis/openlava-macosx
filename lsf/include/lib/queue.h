/* $Id: lib.queue.h 397 2007-11-26 19:04:00Z mblack $
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

#pragma once

// #include "lsf.h"

struct lsQueueEntry
{
    struct lsQueueEntry *forw;
    struct lsQueueEntry *back;
    const char *data;
};

typedef void (*lsQueueDestroyFuncType) (void *);

struct lsQueue
{
    struct lsQueueEntry *start;
    int (*compare) (const char *val, struct lsRequest *reqEnt, int hint);
    lsQueueDestroyFuncType destroy;
};

#define LS_QUEUE_EMPTY(Head) ((Head)->start->forw == (Head)->start)

// void tMsgDestroy_ (void *);
// int lsReqCmp_ (char *, char *, int);
int lsQueueInit_ (struct lsQueue **head, int (*compare) (const char *val, struct lsRequest *reqEnt, int hint), lsQueueDestroyFuncType destroy);

int lsQueueEntryAddFront_ (struct lsQueueEntry *entry, struct lsQueue *head);
int lsQueueDataAddFront_ (char *data, struct lsQueue *head);

int lsQueueEntryAppend_ (struct lsQueueEntry *entry, struct lsQueue *head);
int lsQueueDataAppend_ (char *data, struct lsQueue *head);
void lsQueueEntryRemove_ (struct lsQueueEntry *entry);
void lsQueueEntryDestroy_ (struct lsQueueEntry *entry, struct lsQueue *head);
void lsQueueEntryDestroyAll_ (struct lsQueue *head);
void lsQueueDestroy_ (struct lsQueue *head);
struct lsQueueEntry *lsQueueDequeue_ (struct lsQueue *head);
struct lsQueueEntry *lsQueueSearch_ (int hint, char *data, struct lsQueue *head);
char *lsQueueDataGet_ (int, struct lsQueue *head);
void lsQueueSetAdd_ (struct lsQueue *q1, struct lsQueue *q2, bool_t (*memberFunc) (struct lsQueueEntry *, struct lsQueue *));
void lsQueueSort_ (struct lsQueue *q, int hint);
int lsQueueDequeueData_ (struct lsQueue *head, char **data);
void lsQueueIter_ (struct lsQueue *head, void (*func) (char *data, void *hdata), void *hdata);

