/* $Id: bitset.h 397 2007-11-26 19:04:00Z mblack $
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


#include "libint/list.h"
#include "lib/lproto.h"
#include "daemons/libresd/resd.h"

#define WORDLENGTH (sizeof(unsigned int)*8)
#define SET_DEFAULT_SIZE WORDLENGTH
#define BYTES_IN_MASK(x) (x)*(sizeof(unsigned int))
#define SET_GET_WORD(position) (position/WORDLENGTH)
#define SET_GET_BIT_IN_WORD(position) (position % WORDLENGTH);
#define SET_IS_VALID(set) (set != NULL)
#define SET_IS_EMPTY(set) (set->setNumElements == 0)
// #undef LS_BITSET_ERROR_CODE_ENTRY
#define LS_BITSET_ERROR_CODE_ENTRY(Id, Desc) Id,


struct LS_BITSET_T {
	char *setDescription;
	unsigned int *bitmask;
	unsigned int setSize;
	unsigned int setWidth;
	unsigned int setNumElements;
	bool_t allowObservers;
	struct _list *observers;
	int (*getIndexByObject) (void *);
	void *(*getObjectByIndex) (int);
};

struct LS_BITSET_ITERATOR_T {
	struct LS_BITSET_T *set;
	unsigned int setCurrentBit;
	unsigned int setSize;
};

enum LS_BITSET_EVENT_TYPE_T {// _bitsetEventType {
	LS_BITSET_EVENT_ENTER,
	LS_BITSET_EVENT_LEAVE,
	LS_BITSET_EVENT_NULL
};


struct _bitsetEvent {
	void *entry;
	enum LS_BITSET_EVENT_TYPE_T type;
	char padding[4];
};

typedef bool_t (*LS_BITSET_ENTRY_SELECT_OP_T) (void *extra, struct _bitsetEvent *);
typedef int (*LS_BITSET_EVENT_CALLBACK_FUNC_T) (struct LS_BITSET_T * set, void *extra, struct _bitsetEvent * event);

struct LS_BITSET_OBSERVER_T { //_bitsetObserver
	struct _bitsetObserver *forw;
	struct _bitsetObserver *back;
	char *name;
	struct LS_BITSET_T *set;
	void *extra;
	LS_BITSET_ENTRY_SELECT_OP_T select;
	LS_BITSET_EVENT_CALLBACK_FUNC_T enter;
	LS_BITSET_EVENT_CALLBACK_FUNC_T leave_;
};

#define BITSET_ITERATOR_ZERO_OUT(Iter) memset((void *)(Iter), 0, sizeof(struct LS_BITSET_ITERATOR_T));

enum setSize {
	SET_SIZE_DEFAULT,
	SET_SIZE_CONST,
	SET_SIZE_VAR
};

enum bitState {
	SET_BIT_OFF,
	SET_BIT_ON
};

const unsigned short SET_WORD_DEFAULT_EXTENT = 2;

// #define LS_SET_UNION      0
// #define LS_SET_INTERSECT  1
// #define LS_SET_DIFFERENCE 2
// #define LS_SET_ASSIGN     5
enum LS_SET {
	LS_SET_UNION      = 0,
	LS_SET_INTERSECT  = 1,
	LS_SET_DIFFERENCE = 2,
	LS_SET_ASSIGN     = 5
};

int bitseterrno;

enum _lsBitSetErrno_ {
/*#    include "lsbitseterr.def" */
/***************************************************************************
 *
 * Load Sharing Facility 
 *
 * List of error codes related to the bitset library.
 *
 ***************************************************************************
 *
 */

	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_NOERR,    "No Error"                   )
	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_BADARG,   "Bad Arguments"              )
	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_SETEMPTY, "Set Is Empty"               )
	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_NOMEM,    "Memory allocation failed"   )
	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_FUNC,     "User function failed"       )
	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_ISALREDY, " Object alredy in set"      ) // FIXME FIXME see if the extra space before "Object" has any use
	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_EINVAL,   "Invalid set operation"      )
	LS_BITSET_ERROR_CODE_ENTRY( LS_BITSET_ERR_NOOBSVR,  "Observer permission denied" )
	LS_BITSET_ERR_LAST
};

struct LS_BITSET_T          *setCreate                ( const size_t size, int (*directFunction )(void *), void *(*inverseFunction )(int ), char *caller);
struct LS_BITSET_T          *simpleSetCreate          ( const int size, char *caller );
int                          setDestroy               ( struct LS_BITSET_T *set );
struct LS_BITSET_T          *setDup                   ( struct LS_BITSET_T *set );
bool_t                       setTestValue             ( struct LS_BITSET_T *set, const int value );
bool_t                       setIsMember              ( struct LS_BITSET_T *set, void *obj );
int                          setAddElement            ( struct LS_BITSET_T *set, void *obj );
int                          setRemoveElement         ( struct LS_BITSET_T *set, void *obj );
int                          setClear                 ( struct LS_BITSET_T *set );
unsigned int                 getNum1BitsInWord        ( unsigned int *word );
unsigned int                 setGetNumElements        ( struct LS_BITSET_T *set );
struct LS_BITSET_T          *setEnlarge               ( struct LS_BITSET_T *set, unsigned int newSize );
void                         setOperate               ( struct LS_BITSET_T *dest, struct LS_BITSET_T *src, int op );
void                         setCat                   ( struct LS_BITSET_T *set, char *buffer, size_t bufferSize, char *(*catFunc )(void *, void *), void *hint);
int                          setIteratorAttach        ( struct LS_BITSET_ITERATOR_T *iter, struct LS_BITSET_T *set, char *func);
void                         setIteratorDetach        ( struct LS_BITSET_ITERATOR_T *iter );
void                        *setIteratorBegin         ( struct LS_BITSET_ITERATOR_T *iter );
void                        *setIteratorGetNextElement( struct LS_BITSET_ITERATOR_T *iter );
bool_t                       setIteratorIsEndOfSet    ( struct LS_BITSET_ITERATOR_T *iter );
void                         setIteratorDestroy       ( struct LS_BITSET_ITERATOR_T *iter );
char                        *setPerror                ( int errorNumber );
int                          setAllowObservers        ( struct LS_BITSET_T *set );
int                          setObserverAttach        ( struct LS_BITSET_OBSERVER_T *observer, struct LS_BITSET_T *set );
int                          setNotifyObservers       ( struct LS_BITSET_T *set, struct _bitsetEvent *event );
int                          setDumpSet               ( struct LS_BITSET_T *set, char *caller );
struct LS_BITSET_ITERATOR_T *setIteratorCreate        ( struct LS_BITSET_T *set );
struct LS_BITSET_OBSERVER_T *setObserverCreate        ( char *name, void *extra, LS_BITSET_ENTRY_SELECT_OP_T select, ...);
