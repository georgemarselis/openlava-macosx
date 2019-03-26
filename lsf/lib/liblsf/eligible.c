/* $Id: lib.eligible.c 397 2007-11-26 19:04:00Z mblack $
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
#include <strings.h>
#include <unistd.h>

#include "lsb/sub.h"
#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/misc.h"
#include "lib/table.h"
#include "lib/eligible.h"


// FIXME FIXME FIXME the task table might need a bit of test cases and
//    making sure it contains correct values
struct hTab rtask_table;
struct hTab ltask_table;

char listok;

char *
ls_resreq ( const char *task)
{
	static char resreq[MAX_LINE_LEN];

	if (!ls_eligible (task, resreq, LSF_LOCAL_MODE)) {
		return NULL;
	}
	else {
		return resreq;
	}

	return NULL;
}


int
ls_eligible ( const char *task, char *resreqstr, const char mode)
{
	struct hEnt *mykey = NULL;
	char *p = NULL;

	assert( strlen( resreqstr ) > 0 ); // initiall

	if (!listok) {
		if (inittasklists_ () < 0) {
			return FALSE;
		}
	}

	// lserrno = LSE_NO_ERR; // we are not throwing lserrno if an error happens below, why are we setting it to no error here, then?

	if (!task) {
		return FALSE;
	}

	if ((p = getenv ("LSF_TRS")) == NULL) // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
	{
		if ((p = strrchr (task, '/')) == NULL) {
			p = (char *) task; // cast is fine, we are just putting the pointer in the hash, not altering contents
		}
		else {
			p++;
		}
	}
	else {
		p = (char *) task; // cast is fine, we are just putting the pointer in the hash, not altering contents
	}

	if (mode == LSF_REMOTE_MODE && h_getEnt_ (&ltask_table, (char *) p) != NULL)
	{
		return FALSE;
	}

	if ((mykey = h_getEnt_ (&rtask_table, (char *) p)) != NULL) // FIXME FIXME FIXME this cast may not be neded here
	{
		if (mykey->hData)
			strncpy (resreqstr, mykey->hData, strlen( mykey->hData ) ); // copying, not altering
	}

	return mode == LSF_REMOTE_MODE || mykey != NULL;
}

long
inittasklists_ (void)
{
	char filename[MAX_FILENAME_LEN];
	char *homep = NULL;
	char *clName = NULL;

	h_initTab_ (&rtask_table, 11);
	h_initTab_ (&ltask_table, 11);

	memset( filename, 0, MAX_FILENAME_LEN);

	if (initenv_ (NULL, NULL) < 0) {
		return -1;
	}

	sprintf (filename, "%s/lsf.task", genParams_[LSF_CONFDIR].paramValue); // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
	if (access (filename, R_OK) == 0) {
		if (readtaskfile_ (filename, NULL, NULL, &ltask_table, &rtask_table, FALSE) >= 0) {
			listok = TRUE;
		}
	}

	clName = ls_getclustername ();
	if (clName != NULL) {
		sprintf (filename, "%s/lsf.task.%s", genParams_[LSF_CONFDIR].paramValue, clName);// FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
		if (access (filename, R_OK) == 0) {
			if (readtaskfile_ (filename, NULL, NULL, &ltask_table, &rtask_table, FALSE) >= 0) {
				listok = TRUE;
			}
		}
	}

	if ((homep = osHomeEnvVar_ ()) != NULL) {
		strcpy (filename, homep);
		strcat (filename, "/.lsftask"); // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
		if (access (filename, R_OK) == 0) {
			if (readtaskfile_ (filename, NULL, NULL, &ltask_table, &rtask_table, FALSE) >= 0) {
				listok = TRUE;
			}
		}
	}
	if (listok) {
		return 0;
	}

	lserrno = LSE_BAD_TASKF;

	return -1;
}

////////////////////////////////////////////////////////////
// readtaskfile_ : read in the tasks to be done from a file.
//  FIXME FIXME: there is parsing in here, needs to be replaced by BNF and bison'ed away
int
readtaskfile_ ( const char *filename, struct hTab *minusListl, struct hTab *minusListr, struct hTab *localList, struct hTab *remoteList, const char useMinus)
{
	enum phase { ph_begin, ph_remote, ph_local } phase; // FIXME kinda wierd, but ok.
	FILE *fp   = NULL;
	char *line = NULL;
	char *word = NULL;
	char minus = ' ';

	phase = ph_begin;
	if ((fp = fopen (filename, "r")) == NULL) // FIXME FIXME FIXME replace "r" with a #define?
	{

		lserrno = LSE_FILE_SYS;
		return -1;
	}

	while ((line = getNextLine_ (fp, TRUE)) != NULL)
	{
		switch (phase)
		{
			case ph_begin:

				word = getNextWord_ (&line);
				if (strcasecmp (word, "begin") != 0) { // FIXME FIXME FIXME "begin" has to go into lex/configure.ac 
					fclose (fp);
					lserrno = LSE_BAD_TASKF;
					return -1;
				}

				word = getNextWord_ (&line);
				if (word == NULL)
				{
					fclose (fp);
					lserrno = LSE_BAD_TASKF;
					return -1;
				}
				if (strcasecmp (word, "remotetasks") == 0) { // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					phase = ph_remote;
				}
				else if (strcasecmp (word, "localtasks") == 0) { // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					phase = ph_local;
				}
				else {
					fclose (fp);
					lserrno = LSE_BAD_TASKF;
					return -1;
				}
			break;

			case ph_remote:

				word = getNextValueQ_ (&line, '"', '"'); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
				if (strcasecmp (word, "end") == 0) // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
				{
					word = getNextWord_ (&line);
					if (word == NULL)
					{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
					if (strcasecmp (word, "remotetasks") == 0) // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					{
						phase = ph_begin;
						break;
					}

					fclose (fp);
					lserrno = LSE_BAD_EXP;
					return -1;
				}

				minus = FALSE;
				if (strcmp (word, "+") == 0 || strcmp (word, "-") == 0) // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
				{
					minus = (strcmp (word, "-") == 0);  // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					word = getNextValueQ_ (&line, '"', '"'); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					if (word == NULL)
					{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
				}
				else if (word[0] == '+' || word[0] == '-') // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
				{
					minus = word[0] == '-'; // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					word++;
				}

				if (minus)
				{
					if( ( deletetask_ ( word, remoteList ) < 0 ) && useMinus ) {
						inserttask_ (word, minusListr);
					}
				}
				else {
					inserttask_ (word, remoteList);
				}
			break;

			case ph_local:

				word = getNextWord_ (&line);
				if (strcasecmp (word, "end") == 0) // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
				{
					word = getNextWord_ (&line);
					if (word == NULL)
					{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
					if (strcasecmp (word, "localtasks") == 0) // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					{
						phase = ph_begin;
						break;
					}

					fclose (fp);
					lserrno = LSE_BAD_EXP;
					return -1;
				}

				minus = FALSE;
				if (strcmp (word, "+") == 0 || strcmp (word, "-") == 0) { // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					minus = (strcmp (word, "-") == 0); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
					word = getNextWord_ (&line);
					if (word == NULL)
					{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
				}
				else if (word[0] == '+' || word[0] == '-') // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
				{
					minus = word[0] == '-';
					word++;
				}

				if (minus)
				{
					if( ( deletetask_( word, localList ) < 0 ) && useMinus ) {
						inserttask_ (word, minusListl);
					}
				}
				else {
					inserttask_( word, localList );
				}
			break;

			default: {
				;
			}
		}
	}

	if (phase != ph_begin) {
		fclose (fp);
		lserrno = LSE_BAD_TASKF;
		return -1;
	}

	fclose (fp);
	return 0;
}

int
writetaskfile_ ( const char *filename, struct hTab * minusListl, struct hTab * minusListr, struct hTab *localList, struct hTab * remoteList)
{
	char **tlist = NULL;
	unsigned long num = 0;

	FILE *fp = fopen(filename, "w"); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
	if ( NULL == fp ) {
		lserrno = LSE_FILE_SYS;
		return -1;
	}

	// FIXME FIXME return values disregarded?
	fprintf (fp, "Begin LocalTasks\n"); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
	num = listtask_ (&tlist, localList, TRUE);

	for ( unsigned long i = 0; i < num; i++) {
		fprintf (fp, "+ %s\n", tlist[i]); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
	}

	num = listtask_ (&tlist, minusListl, TRUE);
	for ( unsigned long i = 0; i < num; i++) {
		fprintf (fp, "- %s\n", tlist[i]); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
	}

	fprintf (fp, "End LocalTasks\n\n"); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac

	fprintf (fp, "Begin RemoteTasks\n"); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
	num = listtask_ (&tlist, remoteList, TRUE);
	for ( unsigned long i = 0; i < num; i++) {
		fprintf (fp, "+ \"%s\"\n", tlist[i]);
	}

	num = listtask_ (&tlist, minusListr, TRUE);
	for ( unsigned long i = 0; i < num; i++) {
		fprintf (fp, "- %s\n", tlist[i]);
	}

	// FIXME FIXME return values disregarded?
	fprintf (fp, "End RemoteTasks\n\n"); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
	fclose (fp);

	return 0;
}

int
ls_insertrtask ( const char *task)
{
	char *sp = NULL;
	char *p = NULL;

	if (!listok) {
		if (inittasklists_ () < 0) {
			return -1;
		}
	}

	if ((p = getenv ("LSF_TRS")) != NULL) { // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
		sp = strchr (task, *p);
	}
	else {
		sp = strchr (task, '/'); // FIXME FIXME FIXME "begin" has to go into lex/configure.ac
	}

	if (sp && expSyntax_ (sp + 1) < 0) {
		return -1;
	}

	inserttask_ (task, &rtask_table);
	return 0;
}

int
ls_insertltask ( const char *task)
{

	if (!listok) {
		if (inittasklists_ () < 0) {
			return -1;
		}
	}

	inserttask_ (task, &ltask_table);

	return 0;
}

void
inserttask_ ( const char *taskstr, struct hTab *tasktb)
{
	int succ = 0;
	char *task = NULL;
	char *resreq = NULL;
	struct hEnt *hEntPtr = NULL;
	int *oldcp = NULL;
	char *p = NULL;
	int taskResSep = 0;

	if ((p = getenv ("LSF_TRS")) != NULL) { // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
		taskResSep = *p;
	}
	else {
		taskResSep = '/'; // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
	}

	task = putstr_ (taskstr);
	if ((resreq = strchr (task, taskResSep)) != NULL) {
		*resreq++ = '\0';
		if (*resreq == '\0') // FIXME FIXME wtf?
			resreq = NULL;
	}

	hEntPtr = h_addEnt_ (tasktb, task, &succ);
	if (!succ) {
		oldcp = hEntPtr->hData;
		hEntPtr->hData = NULL;
		if (oldcp != NULL)		{
			free (oldcp);
		}
	}

	hEntPtr->hData = ((resreq == NULL) ? NULL : (putstr_ (resreq)));
	free (task);

	return;
}

int
ls_deletertask ( const char *task)
{
	if (!listok) {
		if (inittasklists_ () < 0) {
			return -1;
		}
	}

	return deletetask_ (task, &rtask_table);

}

int
ls_deleteltask ( const char *task)
{
	if (!listok) {
		if (inittasklists_ () < 0) {
			return -1;
		}
	}

	return deletetask_ (task, &ltask_table);

}

int
deletetask_ ( const char *taskstr, struct hTab * tasktb)
{
	struct hEnt *hEntPtr = NULL;
	char *sp = 0;
	char *task = 0;
	char *p = 0;

	task = putstr_ (taskstr);
	if ((p = getenv ("LSF_TRS")) != NULL) { // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
		sp = strchr (task, *p);
	}
	else {
		sp = strchr (task, '/'); // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
	}

	if (sp != NULL) { // FIXME FIXME wtf?
		*sp = '\0';
	}

	hEntPtr = h_getEnt_ (tasktb, (char *) task);
	if (hEntPtr == NULL) {
		lserrno = LSE_BAD_ARGS;
		free (task);
		return -1;
	}

	h_delEnt_ (tasktb, hEntPtr);

	free (task);
	return 0;
}

long
ls_listrtask (char ***taskList, int sortflag)
{
	if (!listok && inittasklists_ () < 0) {
		return -1;
	}

	return listtask_ (taskList, &rtask_table, sortflag);
}

long
ls_listltask (char ***taskList, int sortflag)
{
	if (!listok && inittasklists_() < 0) { 	// FIXME FIXME FIXME check results here
		return -1;
	}

	return listtask_ (taskList, &ltask_table, sortflag) ;
}


long
listtask_ (char ***taskList, struct hTab *tasktb, int sortflag)
{
	static char **tlist;
	struct hEnt *hEntPtr = NULL;
	struct hLinks *hashLinks = NULL;
	char buff[MAX_LINE_LEN];
	char *p = NULL;
	unsigned long index = 0;
	long nEntry;

	memset( buff, 0, MAX_LINE_LEN );


	nEntry = tasktb->numEnts;
	if( nEntry <= 0) {
		return 0;
	}

	if (tlist != (char **) NULL) {

		// FIXME i am sure this stops when tlist[infex] == NULL
		for ( unsigned int counter = 0; tlist[counter]; counter++) {
			free (tlist[counter]);
		}

		free (tlist);
	}

	// FIXME cast. look above, nEntry has already been checked
	// FIXME FIXME FIXME
	//      the whole loop needs a look over to ensure the right result is produced
	//      issues with dangling brackets
	tlist = malloc (( nEntry + 1) * sizeof (char *));

	index = 0;
	for ( size_t listindex = 0; index < tasktb->size; index++)
	{
		hashLinks = &(tasktb->slotPtr[index]);
		for (hEntPtr = ( struct hEnt *) hashLinks->bwPtr; hEntPtr != ( struct hEnt *) hashLinks; hEntPtr = ( struct hEnt *) ((struct hLinks *) hEntPtr)->bwPtr)
		{
			strcpy (buff, hEntPtr->keyname);
			if (hEntPtr->hData != NULL) {
				size_t tasklen = strlen (buff);

				if ((p = getenv ("LSF_TRS")) != NULL) { // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
					buff[tasklen] = *p;
				}
				else {
					buff[tasklen] = '/'; // FIXME FIXME FIXME FIXME replace "LSF_TRS" with a variable from a hash and an enum subscript
				}
				strcpy (buff + tasklen + 1, (char *) hEntPtr->hData);

				tlist[listindex] = putstr_ (buff);
				listindex++;
			}
		}

		tlist[listindex] = NULL;
		if (sortflag && listindex != 0) {
			qsort (tlist, listindex, sizeof (char *), tcomp_);
		}
	}
	
	*taskList = tlist;
	
	return nEntry;
}

int
tcomp_ (const void *tlist1, const void *tlist2)
{
	return strcmp (*(char **) tlist1, *(char **) tlist2);
}
