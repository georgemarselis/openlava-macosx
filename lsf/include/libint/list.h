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



struct _listEntry
{
	struct _listEntry *forw;
	struct _listEntry *back;
};

struct _list
{
	struct _listEntry *forw;
	struct _listEntry *back;
	char *name;
	int numEnts;
	int allowObservers;
	struct _list *observers;
};


#define LIST_IS_EMPTY(List) ((List)->forw == (struct _listEntry *)List)
#define LIST_NUM_ENTRIES(List) ((List)->numEnts)

typedef void (*LIST_ENTRY_DESTROY_FUNC_T) (struct _listEntry *);
typedef int (*LIST_ENTRY_EQUALITY_OP_T) (void *entry, void *subject, int hint);
typedef void (*LIST_ENTRY_DISPLAY_FUNC_T) (struct _listEntry *, void *);
typedef char *(*LIST_ENTRY_CAT_FUNC_T) (struct _listEntry *, void *);


// #define struct _listRAVERSE_FORWARD              0x1
// #define struct _listRAVERSE_BACKWARD             0x2
enum LIST_TRAVERSE {
	TRAVERSE_FORWARD   = 0x1,
	TRAVERSE_BACKWARD  = 0x2
};

enum _listEventType
{
	LIST_EVENT_ENTER,
	LIST_EVENT_LEAVE,
	LIST_EVENT_NULL
};

struct _listEvent {
	char padding[4];
	enum _listEventType type;
	struct _listEntry *entry;
};

typedef int (*LIST_ENTRY_SELECT_OP_T) (void *extra, struct _listEvent *);
typedef int (*LIST_EVENT_CALLBACK_FUNC_T) (struct _list * list, void *extra, struct _listEvent * event);

struct _listObserver {
	struct _listObserver *forw;
	struct _listObserver *back;
	char *name;
	struct _list *list;
	void *extra;
	LIST_ENTRY_SELECT_OP_T select;
	LIST_EVENT_CALLBACK_FUNC_T enter;
	LIST_EVENT_CALLBACK_FUNC_T leave_;
};

struct _listIterator {
	char *name;
	struct _list *list;
	struct _listEntry *curEnt;
};

#define LIST_ITERATOR_ZERO_OUT(Iter)                            \
	{                                                           \
		memset((void *)(Iter), 0, sizeof(struct _listIterator));     \
		(Iter)->name = "";                                      \
	}

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


// static enum _listErrnoTypes{ LIST_ERR_NOERR = 0, LIST_ERR_BADARG, LIST_ERR_NOMEM, LIST_ERR_NOOBSVR  } listErrnoType; // FIXME FIXME FIXME FIXME this enum should not exist, but err codes should work with above . wtf is the above enum doing?

char *listStrError (int listerrno);
void listPError (char *usrmsg);
struct _listIterator *listIteratorCreate (char *name);
void listIteratorDestroy (struct _listIterator * iter);
int listIteratorAttach (struct _listIterator * iter, struct _list * list);
void listIteratorDetach (struct _listIterator * iter);
struct _list *listIteratorGetList (struct _listIterator * iter);
struct _listEntry *listIteratorGetCurEntry (struct _listIterator * iter);
int listIteratorSetCurEntry (struct _listIterator * iter, struct _listEntry * ent, int validateEnt);
void listIteratorNext (struct _listIterator * iter, struct _listEntry ** next);
void listIteratorPrev (struct _listIterator * iter, struct _listEntry ** prev);
int listIteratorIsEndOfList (struct _listIterator * iter);
struct _listObserver *listObserverCreate (char *name, void *extra, LIST_ENTRY_SELECT_OP_T select, ...);
void listObserverDestroy (struct _listObserver * observer);
int listObserverAttach (struct _listObserver * observer, struct _list * list);
void listObserverDetach (struct _listObserver * observer, struct _list * list);
struct _list *listCreate (char *name);
void listDestroy (struct _list * list, void (*destroy) (struct _listEntry *));
int listAllowObservers (struct _list * list);
struct _listEntry *listGetFrontEntry (struct _list * list);
struct _listEntry *listGetBackEntry (struct _list * list);
int listInsertEntryBefore (struct _list * list, struct _listEntry * succ, struct _listEntry * entry);
int listInsertEntryAfter (struct _list * list, struct _listEntry * pred, struct _listEntry * entry);
int listInsertEntryAtFront (struct _list * list, struct _listEntry * entry);
int listInsertEntryAtBack (struct _list * list, struct _listEntry * entry);
struct _listEntry *listSearchEntry (struct _list * list, void *subject, int (*equal) (void *, void *, int), int hint);
void listRemoveEntry (struct _list * list, struct _listEntry * entry);
int listNotifyObservers (struct _list * list, struct _listEvent * event);
void list2Vector (struct _list * list, int direction, void *vector, void (*putVecEnt) (void *vector, int index, struct _listEntry * entry));
void listDisplay (struct _list * list, int direction, void (*displayEntry) (struct _listEntry *, void *), void *hint);
void listCat (struct _list * list, int direction, char *buffer, size_t bufferSize, char *(*catEntry) (struct _listEntry *, void *), void *hint);
struct _list *listDup (struct _list *, int);
void listDump (struct _list *);
