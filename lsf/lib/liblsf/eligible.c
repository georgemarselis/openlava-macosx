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

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/table.h"
#include "lib/eligible.h"


// FIXME FIXME FIXME the task table might need a bit of test cases and
//    making sure it contains correct values
static hTab rtask_table;
static hTab ltask_table;

static char listok;

char *
ls_resreq (char *task)
{
  static char resreq[MAXLINELEN];

  if (!ls_eligible (task, resreq, LSF_LOCAL_MODE))
	return (NULL);
  else
	return (resreq);

}


int
ls_eligible (char *task, char *resreqstr, char mode)
{
  hEnt *mykey;
  char *p;

  resreqstr[0] = '\0';
  if (!listok)
	if (inittasklists_ () < 0)
	  return FALSE;

  lserrno = LSE_NO_ERR;

  if (!task)
	return FALSE;

  if ((p = getenv ("LSF_TRS")) == NULL)
	{
	  if ((p = strrchr (task, '/')) == NULL)
	p = task;
	  else
	p++;
	}
  else
	p = task;

  if (mode == LSF_REMOTE_MODE && h_getEnt_ (&ltask_table, (char *) p) != NULL)
	{
	  return (FALSE);
	}

  if ((mykey = h_getEnt_ (&rtask_table, (char *) p)) != NULL)
	{
	  if (mykey->hData)
	strcpy (resreqstr, (char *) mykey->hData);
	}

  return (mode == LSF_REMOTE_MODE || mykey != NULL);

}

static long
inittasklists_ (void)
{
  char filename[MAXFILENAMELEN];
  char *homep;
  char *clName;

  h_initTab_ (&rtask_table, 11);
  h_initTab_ (&ltask_table, 11);

  if (initenv_ (NULL, NULL) < 0)
	return -1;

  sprintf (filename, "%s/lsf.task", genParams_[LSF_CONFDIR].paramValue);
  if (access (filename, R_OK) == 0)
	if (readtaskfile_ (filename, NULL, NULL, &ltask_table, &rtask_table,
			   FALSE) >= 0)
	  listok = TRUE;

  clName = ls_getclustername ();
  if (clName != NULL)
	{
	  sprintf (filename, "%s/lsf.task.%s",
		   genParams_[LSF_CONFDIR].paramValue, clName);
	  if (access (filename, R_OK) == 0)
	{
	  if (readtaskfile_ (filename, NULL, NULL, &ltask_table, &rtask_table,
				 FALSE) >= 0)
		listok = TRUE;
	}
	}

  if ((homep = osHomeEnvVar_ ()) != NULL)
	{
	  strcpy (filename, homep);
	  strcat (filename, "/.lsftask");
	  if (access (filename, R_OK) == 0)
	{
	  if (readtaskfile_ (filename, NULL, NULL, &ltask_table, &rtask_table,
				 FALSE) >= 0)
		listok = TRUE;
	}
	}
  if (listok)
	return 0;

  lserrno = LSE_BAD_TASKF;
  return -1;

}

////////////////////////////////////////////////////////////
// readtaskfile_ : read in the tasks to be done from a file.
//	FIXME FIXME: there is parsing in here, needs to be replaced by BNF and bison'ed away
int
readtaskfile_ (char *filename, hTab *minusListl, hTab *minusListr, hTab *localList, hTab *remoteList, char useMinus)
{
	enum phase { ph_begin, ph_remote, ph_local } phase; // FIXME kinda wierd, but ok.
	FILE *fp   = NULL;
	char *line = NULL;
	char *word = NULL;
	char minus = ' ';

	phase = ph_begin;
	if ((fp = fopen (filename, "r")) == NULL)
	{

		lserrno = LSE_FILE_SYS;
		return (-1);
	}

	while ((line = getNextLine_ (fp, TRUE)) != NULL)
	{
		switch (phase)
		{
			case ph_begin:

				word = getNextWord_ (&line);
				if (strcasecmp (word, "begin") != 0)
		  		{
					fclose (fp);
					lserrno = LSE_BAD_TASKF;
					return (-1);
				}

				word = getNextWord_ (&line);
				if (word == NULL)
				{
					fclose (fp);
					lserrno = LSE_BAD_TASKF;
					return (-1);
				}
				if (strcasecmp (word, "remotetasks") == 0) {
					phase = ph_remote;
				}
				else if (strcasecmp (word, "localtasks") == 0) {
					phase = ph_local;
				}
				else {
					fclose (fp);
					lserrno = LSE_BAD_TASKF;
					return (-1);
				}
			break;

			case ph_remote:

				word = getNextValueQ_ (&line, '"', '"');
				if (strcasecmp (word, "end") == 0)
				{
					word = getNextWord_ (&line);
					if (word == NULL)
					{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
					if (strcasecmp (word, "remotetasks") == 0)
					{
						phase = ph_begin;
						break;
					}

					fclose (fp);
					lserrno = LSE_BAD_EXP;
					return -1;
				}

				minus = FALSE;
				if (strcmp (word, "+") == 0 || strcmp (word, "-") == 0)
				{
					minus = (strcmp (word, "-") == 0);
					word = getNextValueQ_ (&line, '"', '"');
					if (word == NULL)
					{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
				}
				else if (word[0] == '+' || word[0] == '-')
				{
					minus = word[0] == '-';
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
				if (strcasecmp (word, "end") == 0)
				{
					word = getNextWord_ (&line);
					if (word == NULL)
		  			{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
					if (strcasecmp (word, "localtasks") == 0)
					{
						phase = ph_begin;
						break;
					}

					fclose (fp);
					lserrno = LSE_BAD_EXP;
					return -1;
				}

				minus = FALSE;
				if (strcmp (word, "+") == 0 || strcmp (word, "-") == 0) {
					minus = (strcmp (word, "-") == 0);
					word = getNextWord_ (&line);
					if (word == NULL)
					{
						fclose (fp);
						lserrno = LSE_BAD_EXP;
						return -1;
					}
		  		}
				else if (word[0] == '+' || word[0] == '-')
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
		return (-1);
	}

	fclose (fp);
	return (0);
}

int
writetaskfile_ (char *filename, hTab * minusListl, hTab * minusListr, hTab * localList, hTab * remoteList)
{
	char **tlist;
	long num = 0;

	FILE *fp = fopen(filename, "w");
	if ( NULL == fp ) {
	  lserrno = LSE_FILE_SYS;
	  return (-1);
	}

	// FIXME FIXME return values disregarded?
	fprintf (fp, "Begin LocalTasks\n");
	num = listtask_ (&tlist, localList, TRUE);

	for ( int i = 0; i < num; i++) {
		fprintf (fp, "+ %s\n", tlist[i]);
	}

	num = listtask_ (&tlist, minusListl, TRUE);
	for ( int i = 0; i < num; i++) {
		fprintf (fp, "- %s\n", tlist[i]);
	}

	fprintf (fp, "End LocalTasks\n\n");

	fprintf (fp, "Begin RemoteTasks\n");
	num = listtask_ (&tlist, remoteList, TRUE);
	for ( int i = 0; i < num; i++) {
		fprintf (fp, "+ \"%s\"\n", tlist[i]);
	}

	num = listtask_ (&tlist, minusListr, TRUE);
	for ( int i = 0; i < num; i++) {
		fprintf (fp, "- %s\n", tlist[i]);
	}

	// FIXME FIXME return values disregarded?
	fprintf (fp, "End RemoteTasks\n\n");
	fclose (fp);
 
	return 0;
}

int
ls_insertrtask (char *task)
{
  char *sp;
  char *p;

  if (!listok)
	if (inittasklists_ () < 0)
	  return -1;

  if ((p = getenv ("LSF_TRS")) != NULL)
	sp = strchr (task, *p);
  else
	sp = strchr (task, '/');

  if (sp && expSyntax_ (sp + 1) < 0)
	{
	  return -1;
	}
  inserttask_ (task, &rtask_table);
  return 0;

}

int
ls_insertltask (char *task)
{

  if (!listok)
	if (inittasklists_ () < 0)
	  return -1;

  inserttask_ (task, &ltask_table);
  return 0;

}

void
inserttask_ (char *taskstr, hTab * tasktb)
{
  int succ;
  char *task;
  char *resreq;
  hEnt *hEntPtr;
  int *oldcp;
  char *p;
  int taskResSep;

  if ((p = getenv ("LSF_TRS")) != NULL)
	taskResSep = *p;
  else
	taskResSep = '/';

  task = putstr_ (taskstr);
  if ((resreq = strchr (task, taskResSep)) != NULL)
	{
	  *resreq++ = '\0';
	  if (*resreq == '\0')
	resreq = NULL;
	}

  hEntPtr = h_addEnt_ (tasktb, task, &succ);
  if (!succ)
	{
	  oldcp = hEntPtr->hData;
	  hEntPtr->hData = NULL;
	  if (oldcp != NULL)
	{
	  free (oldcp);
	}
	}

  hEntPtr->hData = ((resreq == NULL) ? NULL : (putstr_ (resreq)));
  free (task);
}

int
ls_deletertask (char *task)
{
  if (!listok)
	if (inittasklists_ () < 0)
	  return -1;

  return (deletetask_ (task, &rtask_table));

}

int
ls_deleteltask (char *task)
{
  if (!listok)
	if (inittasklists_ () < 0)
	  return -1;

  return (deletetask_ (task, &ltask_table));

}

int
deletetask_ (char *taskstr, hTab * tasktb)
{
  hEnt *hEntPtr;
  char *sp;
  char *task;
  char *p;

  task = putstr_ (taskstr);
  if ((p = getenv ("LSF_TRS")) != NULL)
	sp = strchr (task, *p);
  else
	sp = strchr (task, '/');

  if (sp != NULL)
	*sp = '\0';

  hEntPtr = h_getEnt_ (tasktb, (char *) task);
  if (hEntPtr == (hEnt *) NULL)
	{
	  lserrno = LSE_BAD_ARGS;
	  free (task);
	  return (-1);
	}

  h_delEnt_ (tasktb, hEntPtr);

  free (task);
  return (0);

}

long
ls_listrtask (char ***taskList, int sortflag)
{
  if (!listok && inittasklists_ () < 0) {
	  return -1;
  }

  return (listtask_ (taskList, &rtask_table, sortflag));

}

long
ls_listltask (char ***taskList, int sortflag)
{

	// FIXME FIXME FIXME check results here
	if (!listok && inittasklists_() < 0) {
		return -1;
	}

	return listtask_ (taskList, &ltask_table, sortflag) ;

}


long
listtask_ (char ***taskList, hTab *tasktb, int sortflag)
{
  static char **tlist;
  hEnt *hEntPtr;
  struct hLinks *hashLinks;
  char buf[MAXLINELEN];
  char *p;
  unsigned long index = 0;
  long nEntry;
  

	nEntry = tasktb->numEnts;
	if( nEntry <= 0) {
	  return 0;
	}

	if (tlist != (char **) NULL) {

		// FIXME i am sure this stops when tlist[infex] == NULL
		for ( int counter = 0; tlist[counter]; counter++) {
		  free (tlist[counter]);
		}
 
		free (tlist);
	}

	// FIXME cast. look above, nEntry has already been checked
	// FIXME FIXME FIXME
	//      the whole loop needs a look over to ensure the right result is produced
	//      issues with dangling brackets
	tlist = (char **) malloc (( ( unsigned long) nEntry + 1) * sizeof (char *));

	index = 0;
	for ( size_t listindex = 0; index < tasktb->size; index++)
	{
		hashLinks = &(tasktb->slotPtr[index]);
		for (hEntPtr = (hEnt *) hashLinks->bwPtr; hEntPtr != (hEnt *) hashLinks; hEntPtr = (hEnt *) ((struct hLinks *) hEntPtr)->bwPtr)
		{
			strcpy (buf, hEntPtr->keyname);
			if (hEntPtr->hData != (int *) NULL) {
				size_t tasklen = strlen (buf);

				if ((p = getenv ("LSF_TRS")) != NULL) {
					buf[tasklen] = *p;
				}
				else {
					buf[tasklen] = '/';
				}
				strcpy (buf + tasklen + 1, (char *) hEntPtr->hData);

				tlist[listindex] = putstr_ (buf);
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

static int
tcomp_ (const void *tlist1, const void *tlist2)
{
  return (strcmp (*(char **) tlist1, *(char **) tlist2));
}
