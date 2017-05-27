/* $Id: list.h 397 2007-11-26 19:04:00Z mblack $
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#pragma once

#include <stdlib.h>

typedef struct _listEntry LIST_ENTRY_T;
typedef struct _list LIST_T;
typedef struct _listEvent LIST_EVENT_T;
typedef struct _listObserver LIST_OBSERVER_T;
typedef struct _listIterator LIST_ITERATOR_T;

struct _listEntry
{
	struct _listEntry *forw;
	struct _listEntry *back;
};

struct _list
{
	LIST_ENTRY_T *forw;
	LIST_ENTRY_T *back;
	char *name;
	int numEnts;
	int allowObservers;
	LIST_T *observers;
};


#define LIST_IS_EMPTY(List) ((List)->forw == (LIST_ENTRY_T *)List)

#define LIST_NUM_ENTRIES(List) ((List)->numEnts)

typedef void (*LIST_ENTRY_DESTROY_FUNC_T) (LIST_ENTRY_T *);
typedef int (*LIST_ENTRY_EQUALITY_OP_T) (void *entry, void *subject, int hint);
typedef void (*LIST_ENTRY_DISPLAY_FUNC_T) (LIST_ENTRY_T *, void *);
typedef char *(*LIST_ENTRY_CAT_FUNC_T) (LIST_ENTRY_T *, void *);

LIST_T *listCreate (char *name);
void listDestroy (LIST_T * list, void (*destroy) (LIST_ENTRY_T *));
int listAllowObservers (LIST_T * list);
LIST_ENTRY_T *listGetFrontEntry (LIST_T * list);
LIST_ENTRY_T *listGetBackEntry (LIST_T * list);
int listInsertEntryBefore (LIST_T * list, LIST_ENTRY_T * succ, LIST_ENTRY_T * entry);
int listInsertEntryAfter (LIST_T * list, LIST_ENTRY_T * pred, LIST_ENTRY_T * entry);
int listInsertEntryAtFront (LIST_T * list, LIST_ENTRY_T * entry);
int listInsertEntryAtBack (LIST_T * list, LIST_ENTRY_T * entry);
LIST_ENTRY_T *listSearchEntry (LIST_T * list, void *subject, int (*equal) (void *, void *, int), int hint);
void listRemoveEntry (LIST_T * list, LIST_ENTRY_T * entry);
int listNotifyObservers (LIST_T * list, LIST_EVENT_T * event);
#define LIST_TRAVERSE_FORWARD              0x1
#define LIST_TRAVERSE_BACKWARD             0x2

void list2Vector (LIST_T * list, int direction, void *vector, void (*putVecEnt) (void *vector, int index, LIST_ENTRY_T * entry));

void listDisplay (LIST_T * list, int direction, void (*displayEntry) (LIST_ENTRY_T *, void *), void *hint);

void listCat (LIST_T * list, int direction, char *buffer, size_t bufferSize, char *(*catEntry) (LIST_ENTRY_T *, void *), void *hint);

LIST_T *listDup (LIST_T *, int);
void listDump (LIST_T *);

typedef enum _listEventType
{
	LIST_EVENT_ENTER,
	LIST_EVENT_LEAVE,
	LIST_EVENT_NULL
} LIST_EVENT_TYPE_T;

struct _listEvent
{
	char padding[4];
	LIST_EVENT_TYPE_T type;
	LIST_ENTRY_T *entry;
};

typedef int (*LIST_ENTRY_SELECT_OP_T) (void *extra, LIST_EVENT_T *);

typedef int (*LIST_EVENT_CALLBACK_FUNC_T) (LIST_T * list, void *extra, LIST_EVENT_T * event);

struct _listObserver
{
	struct _listObserver *forw;
	struct _listObserver *back;
	char *name;
	LIST_T *list;
	void *extra;
	LIST_ENTRY_SELECT_OP_T select;
	LIST_EVENT_CALLBACK_FUNC_T enter;
	LIST_EVENT_CALLBACK_FUNC_T leave_;
};


LIST_OBSERVER_T *listObserverCreate (char *name, void *extra, LIST_ENTRY_SELECT_OP_T select, ...);
void listObserverDestroy (LIST_OBSERVER_T * observer);
int listObserverAttach (LIST_OBSERVER_T * observer, LIST_T * list);
void listObserverDetach (LIST_OBSERVER_T * observer, LIST_T * list);


struct _listIterator
{
	char *name;
	LIST_T *list;
	LIST_ENTRY_T *curEnt;
};

#define LIST_ITERATOR_ZERO_OUT(Iter)                            \
	{                                                           \
		memset((void *)(Iter), 0, sizeof(LIST_ITERATOR_T));     \
		(Iter)->name = "";                                      \
	}


LIST_ITERATOR_T *listIteratorCreate (char *name);
void listIteratorDestroy (LIST_ITERATOR_T * iter);
int listIteratorAttach (LIST_ITERATOR_T * iter, LIST_T * list);
void listIteratorDetach (LIST_ITERATOR_T * iter);
LIST_T *listIteratorGetList (LIST_ITERATOR_T * iter);
LIST_ENTRY_T *listIteratorGetCurEntry (LIST_ITERATOR_T * iter);
int listIteratorSetCurEntry (LIST_ITERATOR_T * iter, LIST_ENTRY_T * ent, int validateEnt);
void listIteratorNext (LIST_ITERATOR_T * iter, LIST_ENTRY_T ** next);
void listIteratorPrev (LIST_ITERATOR_T * iter, LIST_ENTRY_T ** prev);
int listIteratorIsEndOfList (LIST_ITERATOR_T * iter);


int listerrno;

#undef LIST_ERROR_CODE_ENTRY
#define LIST_ERROR_CODE_ENTRY(Id, Desc)


enum _listErrno
{
/* this should be checked on a later day */
/*#   include "listerr.def" */
	LIST_ERROR_CODE_ENTRY(LIST_ERR_NOERR,   "No Error") 
	LIST_ERROR_CODE_ENTRY(LIST_ERR_BADARG,  "Bad arguments") 
	LIST_ERROR_CODE_ENTRY(LIST_ERR_NOMEM,   "Memory allocation failed") 
	LIST_ERROR_CODE_ENTRY(LIST_ERR_NOOBSVR, "Permission denied for attaching observers") 
    LIST_ERROR_LAST
};

enum _listErrnoTypes{ LIST_ERR_NOERR = 0, LIST_ERR_BADARG, LIST_ERR_NOMEM, LIST_ERR_NOOBSVR  }; // FIXME FIXME FIXME FIXME this enum should not exist, but err codes should work with above . wtf is the above enum doing?

enum _listErrno listErrnoType;

char *listStrError (int listerrno);
void listPError (char *usrmsg);
