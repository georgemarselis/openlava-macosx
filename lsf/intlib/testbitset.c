/* $Id: testbitset.c 397 2007-11-26 19:04:00Z mblack $
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
// #if _BITSET_TEST_

#include <stdio.h>

#include "libint/bitset.h"
#include "lsf.h"


#define VECT_SIZE 15
#define OBJ_SIZE 2000

extern int bitseterrno;

void test_1 (void); 
void test_2 (void);
void test_3 (void); 
void test_4 (void);


struct X
{
	int index;
	char *blaBla;
};

struct X objs[OBJ_SIZE];
struct X **table;




int main (int argc, char **argv)
{

	assert( argc );

	if (ls_initdebug (argv[0]) < 0)
	{
		ls_perror ("ls_initdebug");
		exit (-1);
	}

	test_4 ();

	exit (0);
}


void test_1 ()
{
	int vect[VECT_SIZE];
	LS_BITSET_T *set;
	char *fname  = malloc( sizeof(char) * strlen( __PRETTY_FUNCTION__ ) + 1);
	strcpy( fname, __PRETTY_FUNCTION__ );

	for ( unsigned int i = 0; i < VECT_SIZE; i++) {
		vect[i] = i + 1;
	}

	set = simpleSetCreate (VECT_SIZE, fname);
	if (!set)
	{
		ls_syslog (LOG_ERR, "%s failed in creating set", __PRETTY_FUNCTION__);
		exit (-1);
	}

	for ( unsigned int i = 0; i < VECT_SIZE; i++)
	{
		if (setAddElement (set, &vect[i]) < 0)
		{
			ls_syslog (LOG_ERR, "%s setAddElement failed for i=%d vect[i]=%d", __PRETTY_FUNCTION__, i, vect[i]);
			setDumpSet (set, fname);
			exit (-1);
		}
	}

	setDumpSet (set, fname);

	ls_syslog (LOG_ERR, "%s setGetNumElements=%d", __PRETTY_FUNCTION__, setGetNumElements (set));

	for ( unsigned int i = 0; i < VECT_SIZE; i++)
	{
		if (setRemoveElement (set, &vect[i]) < 0)
		{
			ls_syslog (LOG_ERR, "%s setAddElement failed for i=%d vect[i]=%d", __PRETTY_FUNCTION__, i, vect[i]);
			setDumpSet (set, fname);
			exit (-1);
		}
	}
	setDumpSet (set, fname);

	free( fname );

	return;
}

void test_2 ()
{
	int vect[VECT_SIZE];
	LS_BITSET_T *set = NULL;
	LS_BITSET_ITERATOR_T *iterator = NULL;
	int *p = NULL;

	char *fname  = malloc( sizeof(char) * strlen( __PRETTY_FUNCTION__ ) + 1);
	strcpy( fname, __PRETTY_FUNCTION__ );

	for( unsigned int i = 0; i < VECT_SIZE; i++) {
		vect[i] = i + 1;
	}

	set = simpleSetCreate (VECT_SIZE, fname);
	if (!set)
	{
		ls_syslog (LOG_ERR, "%s failed in creating set", __PRETTY_FUNCTION__);
		exit (-1);
	}

	for ( unsigned int i = 0; i < VECT_SIZE; i++)
	{
		if (setAddElement (set, &vect[i]) < 0)
		{
			ls_syslog (LOG_ERR, "%s setAddElement failed for i=%d vect[i]=%d", __PRETTY_FUNCTION__, i, vect[i]);
			setDumpSet (set, fname);
			exit (-1);
		}
	}
	iterator = setIteratorCreate (set);
	if (!iterator) {
		ls_syslog (LOG_ERR, "%s setIteratorCreate failed", __PRETTY_FUNCTION__);
		exit (-1);
	}

	while ( ( p = setIteratorGetNextElement (iterator) ) ) {     // FIXME FIXME FIXME this may actually be wrong, and just require == instead of =
		ls_syslog (LOG_ERR, "%s next element in set %d", __PRETTY_FUNCTION__, *p);
	}

	free( fname );

	return;
}



int fun (void *hux)
{
	struct X p;

	memcpy (&p, (struct X *) hux, sizeof (struct X));

	return (p.index);
}

void * gun (int zug)
{
	return (table[zug]);
}

void test_3 ()
{
	int (*directFun)  (void *);
	void *(*inverseFun) (int);
	LS_BITSET_T *set = NULL;
	LS_BITSET_ITERATOR_T *iterator = NULL;
	struct X *gimmeObject = NULL;

	char *fname  = malloc( sizeof(char) * strlen( __PRETTY_FUNCTION__ ) + 1);
	strcpy( fname, __PRETTY_FUNCTION__ );

	directFun = fun;
	inverseFun = gun;


	table = malloc( sizeof (struct X) * OBJ_SIZE);
	if (!table) {
		ls_syslog (LOG_ERR, "%s failed malloc for %d bytes", __PRETTY_FUNCTION__, OBJ_SIZE);
		exit (-1);
	}


	for( unsigned int i = 0; i < OBJ_SIZE; i++) {
		char buf[1024];							// FIXME FIXME FIXME oddly specific
		memset (buf, 0, sizeof (buf));
		sprintf (buf, "You couldn't disagree %d", i);
		objs[i].index = i;
		objs[i].blaBla = strdup (buf);
		table[i] = &objs[i];
	}

	set = setCreate (OBJ_SIZE, directFun, inverseFun, fname);
	if (!set) {
		ls_syslog (LOG_ERR, "%s failed allocating %d bytes", __PRETTY_FUNCTION__, OBJ_SIZE);
		exit (-1);
	}

	for ( unsigned int i = 0; i < OBJ_SIZE; i++) {

		if (setAddElement (set, table[i]) < 0) {
			ls_syslog (LOG_ERR, "%s setAddElement failed, index=%d", __PRETTY_FUNCTION__, i);
			exit (-1);
		}
		ls_syslog (LOG_ERR, "%s adding element obj=%x  index=%d", __PRETTY_FUNCTION__, table[i], i);
	}

	iterator = setIteratorCreate (set);
	if (!iterator) {
		ls_syslog (LOG_ERR, "%s setIteratorCreate() failed", __PRETTY_FUNCTION__);
		exit (-1);
	}

	bitseterrno = LS_BITSET_ERR_NOERR;
	while( ( gimmeObject = (struct X *) setIteratorGetNextElement (iterator))) {
		ls_syslog (LOG_ERR, "%s has an object with index %d = <%s>", __PRETTY_FUNCTION__, gimmeObject->index, gimmeObject->blaBla);
	}
	if (bitseterrno != LS_BITSET_ERR_NOERR) {
		ls_syslog (LOG_ERR, "%s %s", __PRETTY_FUNCTION__, setPerror (bitseterrno));
		exit (-1);
	}

	free( fname );

	return;
}

void test_4 ()
{
	LS_BITSET_T *set;
	LS_BITSET_T *set1;
	LS_BITSET_ITERATOR_T iter;
	char foo[] = "this string does nothing, please ignore";
	int cc = 0;

	char *fname  = malloc( sizeof(char) * strlen( __PRETTY_FUNCTION__ ) + 1);
	strcpy( fname, __PRETTY_FUNCTION__ );

	set = simpleSetCreate (1, "Just one");

	for( size_t i = 0; i < 100; i++) { 						// FIXME FIXME FIXME oddly specific
		cc = setAddElement (set, (void *) (size_t) i); 		// cast is ok: upscales the type so it can be cast to void
		if (cc < 0)
		{
			ls_syslog (LOG_ERR, "%s setAddElement() failed fuck item=%d", __PRETTY_FUNCTION__, i);
			exit (-1);
		}
	}

	cc = setGetNumElements (set);
	printf ("Suck element has %d members\n", cc);

	set1 = simpleSetCreate (3, "do od");
	for ( unsigned int i = 0; i < 30000; i++) {			// FIXME FIXME FIXME 30000 seems oddly specific
		cc = setAddElement (set1, (void *) (size_t) i); // cast is ok: upscales the type so it can be cast to void
		if (cc < 0) {
			ls_syslog (LOG_ERR, "%s setAddElement() failed fuck item=%d", __PRETTY_FUNCTION__, i);
			exit (-1);
		}
	}
	cc = setGetNumElements (set1);
	printf ("Fuck element has %d members\n", cc);

	setOperate (set, set1, LS_SET_UNION);

												// FIXME FIXME FIXME FIXME alter the signature of setIteratorAttach( )
	cc = setIteratorAttach (&iter, set1, foo ); // ignore foo[]: in the body of setIteratorAttach, the 3rd argument gets ignored.
	if (cc < 0) {
		ls_syslog (LOG_ERR, "%s %s", __PRETTY_FUNCTION__, setPerror (bitseterrno));
		exit (-1);
	}

	for ( setIteratorBegin (&iter); setIteratorIsEndOfSet (&iter) == FALSE; setIteratorGetNextElement (&iter)) {
		setIteratorDetach (&iter);
	}
	setDestroy (set);
	setDestroy (set1);

	free( fname );

	return;
}
// #endif
