/* $Id: bitset.c 397 2007-11-26 19:04:00Z mblack $
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

#include <limits.h>

#include "libint/bitset.h"


static void setObserverDestroy (LS_BITSET_T *);
static void observerDestroy (LS_BITSET_OBSERVER_T *);


// #define NL_SETN      22

/* setCreate()
 */
LS_BITSET_T *
setCreate (const size_t size, int (*directFunction) (void *),  void *(*inverseFunction) (int), char *caller)
{
	int sz;
	LS_BITSET_T *set;

	set = calloc (1, sizeof (LS_BITSET_T));
	if (!set)
	{
		bitseterrno = LS_BITSET_ERR_NOMEM;
		ls_syslog (LOG_ERR, "%s %s", __func__, setPerror (bitseterrno));
		return NULL;
	}

	set->setDescription = putstr_ (caller);

  /* Size is the number of elements the caller wants
   * to store in the set.
   */
	sz = (size > 0) ? size : SET_DEFAULT_SIZE; // SET_DEFAULT_SIZE is defined in bitset.h

  /* Width is how many bytes we need to store
   * size number of elements.
   */
	set->setWidth = (sz + WORDLENGTH - 1) / WORDLENGTH;

  /* This is the new size eventually rounded up by 1.
   */
	set->setSize = (set->setWidth) * WORDLENGTH;

	set->setNumElements = 0;

	set->bitmask = calloc (set->setWidth, sizeof (unsigned int));
	if (!set->bitmask)
	{
		bitseterrno = LS_BITSET_ERR_NOMEM;
		return NULL;
	}

	if (directFunction) {
		set->getIndexByObject = directFunction;
	}
	else {
		set->getIndexByObject = NULL;
	}

	if (inverseFunction) {
		set->getObjectByIndex = inverseFunction;
	}
	else {
		set->getObjectByIndex = NULL;
	}

	return set;
}

/* simpleSetcreate()
 */
LS_BITSET_T *
simpleSetCreate (const int size, char *caller)
{
	return setCreate (size, NULL, NULL, caller);
}

int
setDestroy (LS_BITSET_T * set)
{


	if (!SET_IS_VALID (set)) {
		/* catgets 5300 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}


	if (set->allowObservers == TRUE) {
		setObserverDestroy (set);
	}

	free (set->setDescription);
	free (set->bitmask);
	free (set);
	set = NULL;

	return 0;

}

static void
setObserverDestroy (LS_BITSET_T * set)
{
	if (set->allowObservers == FALSE) {
		return;
	}

	listDestroy (set->observers, (LIST_ENTRY_DESTROY_FUNC_T) & observerDestroy);

	return;
}

static void
observerDestroy (LS_BITSET_OBSERVER_T * observer)
{
	free (observer->name);
	free (observer);
	observer = NULL;

	return;
}

/* setDup()
 * Duplicate a given set.
 */
LS_BITSET_T *
setDup (LS_BITSET_T * set)
{
  LS_BITSET_T *set2;

  set2 = calloc (1, sizeof (LS_BITSET_T));
  if (!set2)
	{
	  bitseterrno = LS_BITSET_ERR_NOMEM;
	  ls_syslog (LOG_ERR, "%s %s", __func__, setPerror (bitseterrno));
	  return NULL;
	}

  memcpy( set2, set, sizeof (LS_BITSET_T));
  set2->setDescription = putstr_ (set->setDescription);
  set2->bitmask = calloc (set->setWidth, sizeof (unsigned int));

  memcpy ( set2->bitmask, set2->bitmask, set2->setWidth * sizeof (unsigned int));

  return set2;

}

/* setTestValue()
 * Test if a bit is set in the set mask.
 */
bool_t
setTestValue (LS_BITSET_T * set, const int value)
{
  int word;
  int offset;
  bool_t trueORfalse;

  word = SET_GET_WORD (value);

  offset = SET_GET_BIT_IN_WORD (value);

  trueORfalse = ((*(set->bitmask + word) & (SET_BIT_ON << offset)) != 0);

  return trueORfalse;

}

bool_t
setIsMember (LS_BITSET_T * set, void *obj)
{
	int *value = NULL;
	int x = 0;

	if (!SET_IS_VALID (set)) {
		bitseterrno = LS_BITSET_ERR_BADARG;
		return FALSE;
	}

	if (SET_IS_EMPTY (set)) {
		bitseterrno = LS_BITSET_ERR_SETEMPTY;
		return FALSE;
	}

	if (set->getIndexByObject == NULL) {
		value = obj;
		x = *value;
	}
	else
	{
		x = (*set->getIndexByObject) (obj);
		if (x < 0) {
			bitseterrno = LS_BITSET_ERR_FUNC;
			ls_syslog (LOG_ERR, "%s %s", __func__, setPerror (bitseterrno));
			return -1;
		}
	}

	assert( set->setSize <= INT_MAX );
	if (x >= (int) set->setSize) {
		return FALSE;
	}

	return setTestValue (set, x);
}

int
setAddElement (LS_BITSET_T * set, void *obj)
{

	int *value = 0;
	int word = 0;
	int offset = 0;
	int x = 0;

	if (!SET_IS_VALID (set)) {
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}

	if (set->getIndexByObject == NULL) {
		value = obj;
		x = *value;
	}
	else
	{
		x = (*set->getIndexByObject) (obj);
		if (x < 0) {
			bitseterrno = LS_BITSET_ERR_FUNC;
			return -1;
		}
	}

	assert( set->setSize <= INT_MAX );
	if (x >= (int) set->setSize) {
		ls_syslog (LOG_DEBUG3, "%s: realloc set when size=%d", __func__, value);
		setEnlarge (set, x);
		if (set == NULL) {
			return -1;
		}
	}
	else {
		if (setTestValue (set, x) == TRUE) {
			bitseterrno = LS_BITSET_ERR_ISALREDY;
			return -1;
		}
	}

	word = SET_GET_WORD (x);
	offset = SET_GET_BIT_IN_WORD (x);

	*(set->bitmask + word) |= (SET_BIT_ON << offset);

	set->setNumElements++;

	if (set->allowObservers) {
		LS_BITSET_EVENT_T event;

		event.type = LS_BITSET_EVENT_ENTER;
		event.entry = obj;

		setNotifyObservers (set, &event);
	}

  return 0;
}

/* setRemoveElement()
 */
int
setRemoveElement (LS_BITSET_T * set, void *obj)
{

	int *value = NULL;
	int word = 0;
	int offset = 0;
	int x = 0;

	if (!SET_IS_VALID (set)) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}

	if (SET_IS_EMPTY (set)) {
		bitseterrno = LS_BITSET_ERR_SETEMPTY;
		return -1;
	}

	if (set->getIndexByObject == NULL) {
		value = obj;
		x = *value;
	}
	else {
		x = set->getIndexByObject (obj);
		if ( *value < 0) {
			bitseterrno = LS_BITSET_ERR_FUNC;
			return -1;
		}
	}

	assert( set->setSize <= INT_MAX );
	if (x >= (int) set->setSize) {
		return FALSE;
	}

	word = SET_GET_WORD (x);
	offset = SET_GET_BIT_IN_WORD (x);
	*(set->bitmask + word) &= ~(SET_BIT_ON << offset);
	set->setNumElements-- ;

	return 0;
}

int
setClear (LS_BITSET_T * set)
{

  if (!SET_IS_VALID (set))
	{
	  ls_syslog (LOG_ERR, "setClear: expected a non-NULL set pointer but got a NULL one");
	  bitseterrno = LS_BITSET_ERR_BADARG;
	  return -1;
	}

  memset (set->bitmask, 0, set->setWidth * sizeof (unsigned int));

  set->setNumElements = 0;

  return 0;
}

unsigned int
getNum1BitsInWord (unsigned int *word)
{
	unsigned char *p = NULL;
	unsigned int numElements = 0;

	static unsigned char nbits[] = {
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};

	assert( *word <= CHAR_MAX );
	p = ( unsigned char * )word;  // FIXME FIXME FIXME FIXME must create some kind of functional test to ensure this cast is right
	numElements = 0;

	for( unsigned int i = ( sizeof( nbits ) / sizeof( nbits[0] ) ); i > 0; i-- ) {
		numElements += nbits[*p++];
	}

	return numElements;
}


unsigned int
setGetNumElements (LS_BITSET_T *set)
{
	unsigned char *p = NULL;
	unsigned int numElements = 0;

	static unsigned char nbits[] = {
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
	};


	if (!SET_IS_VALID (set)) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}

	assert( *set->bitmask <= UCHAR_MAX );
	p = (unsigned char *) set->bitmask;
	numElements = 0;
	for( unsigned int i = BYTES_IN_MASK (set->setWidth); i > 0; --i) {
		numElements += nbits[*p++];
	}

	return numElements;
}

LS_BITSET_T *
setEnlarge (LS_BITSET_T * set, unsigned int newSize)
{
  unsigned int oldWidth;

  oldWidth = set->setWidth;
  set->setWidth += newSize + SET_WORD_DEFAULT_EXTENT;

  set->bitmask = (unsigned int *)
	realloc (set->bitmask, sizeof (unsigned int) * set->setWidth);
  if (set->bitmask == NULL)
	{
	  bitseterrno = LS_BITSET_ERR_NOMEM;
	  return NULL;
	}

  memset (set->bitmask + oldWidth, 0,
	  sizeof (unsigned int) * (set->setWidth - oldWidth));

  set->setSize = (set->setWidth) * WORDLENGTH;

  return set;
}

void
setOperate (LS_BITSET_T * dest, LS_BITSET_T * src, int op)
{
  unsigned int *d;
  unsigned int *s;
  int nwords;
  int tail;

  if (op != LS_SET_UNION &&
	  op != LS_SET_INTERSECT &&
	  op != LS_SET_DIFFERENCE && op != LS_SET_ASSIGN)
	{

	  bitseterrno = LS_BITSET_ERR_EINVAL;
	  return;
	}

  if (dest->setWidth < src->setWidth)
	setEnlarge (dest, src->setWidth);
  if (dest == NULL)
	return;

  nwords = src->setWidth;
  tail = dest->setWidth - nwords;

  d = dest->bitmask;
  s = src->bitmask;

  switch (op)
	{
	case LS_SET_UNION:

	  while (--nwords >= 0)
	*d++ |= *s++;
	  break;

	case LS_SET_INTERSECT:

	  while (--nwords >= 0)
	*d++ &= *s++;
	  while (--tail >= 0)
	*d++ = 0;
	  break;

	case LS_SET_DIFFERENCE:

	  while (--nwords >= 0)
	*d++ ^= *s++;
	  break;

	case LS_SET_ASSIGN:

	  while (--nwords >= 0)
	*d++ = *s++;
	  while (--tail >= 0)
	*d++ = 0;
	  break;
	}

  return;
}

void
setCat (LS_BITSET_T * set, char *buffer, size_t bufferSize, char *(*catFunc) (void *, void *), void *hint)
{
	LS_BITSET_ITERATOR_T iter;
	void *entry = NULL;
	size_t curSize = 0;
	char *fname = malloc( strlen( __func__ ) + 1 );
	strcpy( fname, __func__ );

	if (!set || !catFunc) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set but get a NULL one"),  __func__);
		return;
	}

	assert( !buffer ); // FIXME FIXME FIXME FIXME FIXME not quite sure if buffer is supposed to be NULL'ed or not

	BITSET_ITERATOR_ZERO_OUT (&iter);
	setIteratorAttach (&iter, set, fname); // ya this call is correct

	curSize = 0;
	for (entry = setIteratorBegin (&iter);	entry != NULL && (setIteratorIsEndOfSet (&iter) == FALSE);	entry = setIteratorGetNextElement (&iter)) {

		char *str = (*catFunc) (entry, hint);
		if (str == NULL) {
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5304, "%s: catFunc returned NULL string"), __func__);   /* catgets 5304 */
			continue;
		}

		if (curSize + strlen (str) > bufferSize - 1) {
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5305, "%s: the provided buffer is not big enough"), __func__);  /* catgets 5305 */
			break;
		}

		strcat (buffer, str);
		curSize += strlen (str);
	}

	return;
}

LS_BITSET_ITERATOR_T *
setIteratorCreate (LS_BITSET_T * set)
{
	LS_BITSET_ITERATOR_T *iter;

	if (!SET_IS_VALID (set)) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return NULL;
	}

	iter = calloc (1, sizeof (LS_BITSET_ITERATOR_T));
	if (!iter){
		bitseterrno = LS_BITSET_ERR_NOMEM;
		return NULL;
	}
	iter->set = set;
	iter->setCurrentBit = 0;
	iter->setSize = set->setSize;

	return iter;
}

int setIteratorAttach (LS_BITSET_ITERATOR_T * iter, LS_BITSET_T * set, char *func)
{
	assert( *func );

	if (!iter || !set) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one "), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}

	iter->set = set;
	iter->setCurrentBit = 0;
	iter->setSize = set->setSize;

	return 0;
}

void
setIteratorDetach (LS_BITSET_ITERATOR_T * iter)
{

  if (!SET_IS_VALID (iter->set))
	{
	  bitseterrno = LS_BITSET_ERR_BADARG;
	  return;
	}

  iter->set = NULL;
  iter->setCurrentBit = 0;
  iter->setSize = 0;
}

void *
setIteratorBegin( LS_BITSET_ITERATOR_T * iter )
{
	return (void *)setIteratorGetNextElement( iter );
}

void *
setIteratorGetNextElement (LS_BITSET_ITERATOR_T *iter)
{
	void *object;

	if (!SET_IS_VALID (iter->set))
	{
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return NULL;
	}

	if (SET_IS_EMPTY (iter->set))
	{
		bitseterrno = LS_BITSET_ERR_SETEMPTY;
		return NULL;
	}

	while (iter->setCurrentBit < iter->set->setSize)
	{

		if (setTestValue (iter->set, iter->setCurrentBit) == TRUE)
		{

			if (iter->set->getObjectByIndex == NULL) {
				object = &(iter->setCurrentBit);
			}
			else
			{
				object = (*iter->set->getObjectByIndex) (iter->setCurrentBit);
				if (!object)
				{
					bitseterrno = LS_BITSET_ERR_FUNC;
					return NULL;
				}
			}

			++iter->setCurrentBit;
			return object;
		}

		++iter->setCurrentBit;
	}

  return NULL;
}


bool_t
setIteratorIsEndOfSet (LS_BITSET_ITERATOR_T * iter)
{
	bool_t rval = 0;

	if (!SET_IS_VALID (iter->set)) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return FALSE;
	}

	if (iter->setCurrentBit >= iter->setSize) {
		return TRUE;
	}
	else {
		return FALSE;
	}

	return rval; // you should never be here
}

void
setIteratorDestroy (LS_BITSET_ITERATOR_T * iter)
{

	if (!SET_IS_VALID (iter->set)) {
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-NULL set pointer but got a NULL one"), __func__);
	  bitseterrno = LS_BITSET_ERR_BADARG;
	  return;
	}

	setDestroy (iter->set);
	free (iter);
	iter = NULL;

	return;
}

int bitseterrno;

#undef LS_BITSET_ERROR_CODE_ENTRY
#define LS_BITSET_ERROR_CODE_ENTRY(Id, Desc) Desc,

static char *bitSetErrList[] = {
	"No Error",           /* catgets 5311 */
	"Bad Arguments",      /* catgets 5312 */
	"Set Is Empty",       /* catgets 5313 */
	"Memory allocation failed",   /* catgets 5314 */
	"User function failed",   /* catgets 5315 */
	"Object alredy in set",   /* catgets 5316 */
	"Invalid set operation",  /* catgets 5317 */
	"Observer permission denied", /* catgets 5318 */
	"Last Error (no error)"   /* catgets 5319 */
};

//#ifdef  I18N_COMPILE
static int bitSetErrListID[] = {
	5311,
	5312,
	5313,
	5314,
	5315,
	5316,
	5317,
	5318,
	5319
};
//#endif

char *
setPerror (int errorNumber)
{
	char *buf = malloc( sizeof( char ) * 216 + 1 ); // FIXME FIXME FIXME FIXME FIXME 216 chars: odd specificity

	if (errorNumber < 0 || errorNumber > LS_BITSET_ERR_LAST) {
			/* catgets 5320 */
			sprintf( buf, "catgets 5320: Unknown error number %d", errorNumber );
			return buf;
	}

	sprintf( buf, "%d %d %d %s", ls_catd, NL_SETN, bitSetErrListID[errorNumber], bitSetErrList[ errorNumber ] );

	return buf; 
}


int
setAllowObservers (LS_BITSET_T * set)
{

	if (!set) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5321, "%s: expected a non null set but got a null one"), __func__); /* catgets 5321 */
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}

 	if (set->allowObservers) { 
  		return 0;
  	}

	set->allowObservers = TRUE;
	set->observers = listCreate ("Observer list");

	if (!set->observers) {
		return -1;
	}

	return 0;
}


LS_BITSET_OBSERVER_T *
setObserverCreate (char *name, void *extra, LS_BITSET_ENTRY_SELECT_OP_T select, ...)
{
	LS_BITSET_OBSERVER_T *observer;
	LS_BITSET_EVENT_TYPE_T etype;
	LS_BITSET_EVENT_CALLBACK_FUNC_T callback;
	va_list ap;
	unsigned short breakloop = 0;

	observer = calloc (1, sizeof (LS_BITSET_OBSERVER_T));
	if (observer == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
		bitseterrno = LS_BITSET_ERR_NOMEM;
		FREEUP (observer);
		return NULL;
	}

	observer->name = putstr_ (name);
	observer->select = select;
	observer->extra = extra;

	va_start (ap, select);

	while (TRUE) {
		etype = va_arg (ap, LS_BITSET_EVENT_TYPE_T);
		callback = va_arg (ap, LS_BITSET_EVENT_CALLBACK_FUNC_T);
		
		switch (etype) {
			case LS_BITSET_EVENT_NULL:
				breakloop = TRUE;
			break;

			case LS_BITSET_EVENT_ENTER:
				observer->enter = callback;
			break;

			case LS_BITSET_EVENT_LEAVE:
				observer->leave_ = callback;
			break;

			default:
				/* catgets 5323 */
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5323, "%s: invalid event ID (%d) from the invoker"), __func__, etype);
				bitseterrno = LS_BITSET_ERR_BADARG;
				FREEUP (observer->name);
				FREEUP (observer);
				return NULL;
			break;
		}

		if( breakloop ) {
			break;
		}

	}

	return observer;
// Fail:
//   FREEUP (observer);
//   return NULL;
}

int
setObserverAttach (LS_BITSET_OBSERVER_T * observer, LS_BITSET_T * set)
{

	int rc = 0;

	if (!observer || !set) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300, "%s: expected a non-null set but got a null one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}


 	if (!set->allowObservers) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5325, "%s: list \"%s\" does not accept observers"), __func__, set->setDescription); /* catgets 5325 */
		bitseterrno = (int) LS_BITSET_ERR_NOOBSVR;
		return -1;
	}


	rc = listInsertEntryBefore (set->observers, (LIST_ENTRY_T *) set->observers, (LIST_ENTRY_T *) observer);
	if (rc < 0) {
		return rc;
	}

	observer->set = set;
	return 0;
}

int
setNotifyObservers (LS_BITSET_T * set, LS_BITSET_EVENT_T * event)
{
	LS_BITSET_OBSERVER_T *observer;
	LIST_ITERATOR_T iter;

	if (!set || !event) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5300,	"%s: expected a non-null set but got a null one"), __func__);
		bitseterrno = LS_BITSET_ERR_BADARG;
		return -1;
	}

	listIteratorAttach (&iter, set->observers);

	for (observer = (LS_BITSET_OBSERVER_T *) listIteratorGetCurEntry (&iter);
	   !listIteratorIsEndOfList (&iter);
	   listIteratorNext (&iter, (LIST_ENTRY_T **) & observer))
	{

		ls_syslog (LOG_DEBUG3, "%s: Notifying observer \"%s\" of the event <%d>", __func__, observer->name, event->type);

		if (observer->select != NULL) {  
			if (!(*observer->select) (observer->extra, event)) {
				continue;
			}
		}
		switch (event->type) {
			case LS_BITSET_EVENT_ENTER:
				if (observer->enter) {
					(*observer->enter) (set, observer->extra, event);
				}
			break;

			case LS_BITSET_EVENT_LEAVE:
				if (observer->leave_) {
					(*observer->leave_) (set, observer->extra, event);
				}
			break;

			case LS_BITSET_EVENT_NULL: // FIXME FIXME FIXME FIXME this piece of code may actually be wrong way of handling LS_BITSET_EVENT_NULL
				ls_syslog( LOG_ERR, "%s: event->type is LS_BITSET_EVENT_NULL, on observer \"%s\"", __func__, observer->name );
				break;

			default:
				/* catgets 5328 */
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5327, "%s: invalide event type (%d)"), __func__, event->type);
				bitseterrno = LS_BITSET_ERR_BADARG;
			return -1;
		}
	}


  return 0;

}



int
setDumpSet (LS_BITSET_T * set, char *caller)
{

	char buf[3072];

	memset( buf, '\0', strlen( buf ) );

	if (!SET_IS_VALID (set)) {
		bitseterrno = LS_BITSET_ERR_BADARG;
		/*catgets 5329 */
		ls_syslog (LOG_ERR, I18N (5329, "%s caller %s %s"), __func__, caller, setPerror (bitseterrno));
		return -1;
	}

	/* catgets 5330 */
	ls_syslog (LOG_ERR, I18N (5330, "%s Dumping set <%lx> <%s>"), __func__, set, set->setDescription);

	/* catgets 5331 */
	ls_syslog (LOG_ERR, I18N (5331, "%s setSize = %u setWidth = %u setNumElements = %u directFunction = %lx inverseFunction = %lx"), __func__, set->setSize, set->setWidth, set->setNumElements, set->getIndexByObject, set->getObjectByIndex);

	ls_syslog (LOG_ERR, I18N (5332, "%s Begin DumpMask"), __func__); /*catgets 5332 */
	memset (buf, 0, sizeof (buf));

	for ( unsigned int i = 0; i < set->setWidth; i++) {
		sprintf (buf + strlen (buf), "word <%d> decimal <%d> bits: ", i, set->bitmask[i]);

		for( unsigned int j = 0; j < 8; j++) {  // FIXME FIXME FIXME FIXME '8' seems awfully particular
			(*(set->bitmask + i) & (SET_BIT_ON << j))
					? sprintf (buf + strlen (buf), "1")
					: sprintf (buf + strlen (buf), "0");
		}

		sprintf (buf + strlen (buf), "\n");
	}

	ls_syslog (LOG_ERR, "%s", buf);
	/*catgets 5333 */
	ls_syslog (LOG_ERR, I18N (5333, "%s End DumpMask"), __func__);

	return 0;
}
