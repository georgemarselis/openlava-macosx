/* $Id: lib.confmisc.c 397 2007-11-26 19:04:00Z mblack $
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

#include <grp.h>
#include <pwd.h>
#include <strings.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/confmisc.h"


char *
getNextValue (char **line)
{
	return getNextValueQ_ (line, '(', ')');
}

int
keyMatch (struct keymap *keyList, const char *line, int exact)
{
	char *sp       = line;
	char *word     = NULL;
	int pos        = 0;
	int found      = FALSE;
	unsigned int i = 0;


	i = 0;
	while (keyList[i].key != NULL)
	{
		keyList[i].position = -1;
		i++;
	}

	while ((word = getNextWord_ (&sp)) != NULL)
	{
		i = 0;
		found = FALSE;
		while (keyList[i].key != NULL)
		{
			if (strcasecmp (word, keyList[i].key) == 0)
			{
				if (keyList[i].position != -1) {
					return FALSE;
				}
				found = TRUE;
				keyList[i].position = pos;
				break;
			}
			i++;
		}
		if (!found) {
			return FALSE;
		}

		pos++;
	}

	if (!exact) {
		return TRUE;
	}

	i = 0;
	while (keyList[i].key != NULL)
	{
		if (keyList[i].position == -1) {
			return FALSE;
		}
		i++;
	}

	return TRUE;
}

int
isSectionEnd (char *linep, const char *lsfile, size_t *lineNum, const char *sectionName) // FIX
{
	char *word  = getNextWord_ (&linep);

	if (strcasecmp (word, "end") != 0) {
		return FALSE;
	}

	word = getNextWord_ (&linep);
	if (!word) {
		/* catgets 5400 */
		ls_syslog (LOG_ERR, "5400: %s(%d): section %s ended without section name, ignored", lsfile, *lineNum, sectionName);
		return TRUE;
	}

	if (strcasecmp (word, sectionName) != 0) {
		/* catgets 5401 */
		ls_syslog (LOG_ERR, "5401: %s(%d): section %s ended with wrong section name %s,ignored", lsfile, *lineNum, sectionName, word);    
	}

	return TRUE;
}


// DELETEME duplicated code found in intlib/conf.c
// char *
// getBeginLine (FILE * fp, size_t *lineNum)
// {
//  char *sp;
//  char *wp;

//  for (;;)
//  {
//      sp = getNextLineC_ (fp, lineNum, TRUE);
//      if (!sp) {
//          return NULL;
//      }

//      wp = getNextWord_ (&sp);
//      if (wp && (strcasecmp (wp, "begin") == 0)) {
//          return sp;
//      }
//  }

// }
char *
getBeginLine (FILE *fp, size_t *lineNum)
{
	char *sp           = NULL;
	char *wp           = NULL;
	const char begin[] = "begin";

	// for (;;)
	// {
	//  sp = getNextLineC_ (fp, lineNum, TRUE);
	//  if (!sp) {
	//      return NULL;
	//  }

	//  wp = getNextWord_ (&sp);
	//  if (wp && (strcasecmp (wp, "begin") == 0)) {
	//      return sp;
	//  }
	// }
	// FIXME FIXME refactoring to avoid infinite loop
	do {
		sp = getNextLineC_( fp, lineNum, TRUE);

		wp = getNextWord_( &sp );
		if (wp && (strcasecmp( wp, begin ) == 0)) {
			return sp;
		}
	} while( sp )

	return NULL;
}


int
readHvalues (struct keymap *keyList, char *linep, FILE * fp, const char *lsfile, size_t *lineNum, int exact, const char *section)
{
	char *key      = NULL;
	char *value    = NULL;
	char *sp       = NULL;
	char *sp1      = NULL;
	char error     = FALSE;
	unsigned int i = 0;

	sp = linep;
	key = getNextWord_ (&linep);
	if ((sp1 = strchr (key, '=')) != NULL) {
		*sp1 = '\0';
	}

	value = strchr (sp, '=');
	if (!value) {
		/* catgets 5402 */
		ls_syslog (LOG_ERR, "5402: %s: %s(%d): missing '=' after keyword %s, section %s ignoring the line", __func__, lsfile, *lineNum, key, section);
	}
	else {
		value++;
		while (*value == ' ') {
			value++;
		}

		if (value[0] == '\0')
		{
			/* catgets 5403 */
			ls_syslog (LOG_ERR, "5403: %s: %s(%d): null value after keyword %s, section %s ignoring the line", __func__, lsfile, *lineNum, key, section);
		}

		if (value[0] == '(')
		{
			value++;
			if ((sp1 = strrchr (value, ')')) != NULL) {
				*sp1 = '\0';
			}
		}
		if (putValue (keyList, key, value) < 0) {
			/* catgets 5404 */
			ls_syslog (LOG_ERR, "5404: %s: %s(%d): bad keyword %s in section %s, ignoring the line", __func__, lsfile, *lineNum, key, section);
		}
	}

	if ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {
		if (isSectionEnd (linep, lsfile, lineNum, section)) {
			if (!exact) {
				return 0;
			}

			i = 0;
			while (keyList[i].key != NULL) {
				if (keyList[i].val == NULL) {
					/* catgets 5405 */
					ls_syslog (LOG_ERR, "5405: %s: %s(%d): required keyword %s is missing in section %s, ignoring the section", __func__, lsfile, *lineNum, keyList[i].key, section);  
					error = TRUE;
				}
			  i++;
			}
			if (error) {
				i = 0;
				while (keyList[i].key != NULL) {
					FREEUP (keyList[i].val);
					i++;
				}
				return -1;
			}
			return 0;
		}

		return readHvalues (keyList, linep, fp, lsfile, lineNum, exact, section);
	}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, lsfile, *lineNum, section);
	return -1;
}

// int
// putValue (struct keymap *keyList, char *key, char *value)
// {
//     int i;

//     i = 0;
//     while (keyList[i].key != NULL) {

//         if (strcasecmp (keyList[i].key, key) == 0) {
//             FREEUP (keyList[i].val);
//             if (strcmp (value, "-") == 0) {
//                 keyList[i].val = putstr_ ("");
//             }
//             else {
//                 keyList[i].val = putstr_ (value);
//             }
//             return 0;
//         }
//         i++;
//     }

//   return -1;
// }

void
doSkipSection (FILE * fp, size_t *lineNum, const char *lsfile, const char *sectionName)
{
	char *word = NULL;
	char *cp   = NULL;

	while ((cp = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {
		word = getNextWord_ (&cp);
		if (strcasecmp (word, "end") == 0) {
			word = getNextWord_ (&cp);
			if (!word) {
				/* catgets 5407 */
				ls_syslog (LOG_ERR, "5407: %s(%d): Section ended without section name, ignored", lsfile, *lineNum);
			}
			else {
				if (strcasecmp (word, sectionName) != 0)
				ls_syslog (LOG_ERR, "5408: %s(%d): Section %s ended with wrong section name: %s, ignored", lsfile, *lineNum, sectionName, word);  /* catgets 5408 */
			}
			return;
		}
	}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, lsfile, *lineNum, sectionName);
	return;
}

int
mapValues (struct keymap *keyList, char *line) // FIXME FIXME should char *line be const char * ?
{
	long pos       = 0;
	char *value    = NULL;
	int found      = 0; // FALSE
	int numv       = 0;
	unsigned int i = 0;

	while (keyList[i].key != NULL) {

		FREEUP (keyList[i].val);
		if (keyList[i].position != -1) {
			numv++;
		}

		i++;
	}

	while ((value = getNextValue (&line)) != NULL) // FIXME FIXME FIXME why are we passing the address of the address of the line?
	{
		i = 0;
		found = FALSE;
		while (keyList[i].key != NULL)
		{
			if (keyList[i].position != pos)
			{
				i++;
				continue;
			}
			if (strcmp (value, "-") == 0) {
				keyList[i].val = putstr_ ("");
			}
			else
			{
				if (keyList[i].val != NULL) {
					FREEUP (keyList[i].val);
				}
				keyList[i].val = putstr_ (value);
			}
			found = TRUE;
			break;
		}
		if (!found) {
			i = 0;
			while (keyList[i].key != NULL) {
				if (keyList[i].val != NULL) {
					free (keyList[i].val);
					keyList[i].val = NULL;
				}

				i++;
			}
			return -1;
			// goto fail;
		}
		pos++;
	}

	if (pos != numv) {
		i = 0;
		while (keyList[i].key != NULL) {
			if (keyList[i].val != NULL) {
				free (keyList[i].val);
				keyList[i].val = NULL;
			}

			i++;
		}
		return -1;
		// goto fail;
	}

	return 0;

// fail: // FIXME FIXME FIXME remove goto from function
//   i = 0;
//   while (keyList[i].key != NULL)
//  {
//    if (keyList[i].val != NULL)
//  {
//    free (keyList[i].val);
//    keyList[i].val = NULL;
//  }

//    i++;
//  }
//   return -1;

}

int
putInLists (char *word, struct admins *admins, unsigned int *numAds, const char *forWhat)
{
	struct passwd *pw = NULL;
	char **tempNames  = NULL; 
	uid_t *tempIds    = NULL;
	gid_t *tempGids   = NULL;

	if ((pw = getpwlsfuser_ (word)) == NULL) {
	   
	   if (logclass & LC_TRACE) {
		   /* catgets 5410 */
		   ls_syslog (LOG_DEBUG, "5410: %s: <%s> is not a valid user on this host", __func__, word);
	   }
	}
	
	if (isInlist (admins->adminNames, word, admins->nAdmins)) {
		/* catgets 5411 */
		ls_syslog (LOG_WARNING, "5411: %s: Duplicate user name <%s> %s; ignored", __func__, word, forWhat);
		return 0;
	}

	// pw-->pw_[gu]id is always equal or greater than zero. so, the cast is ok. 
	// an extra assertion does not hurt, BUT
	// the code should be investagated as to why does it need a negative UID number.
	// is it a standard thing to do? is there any other way of representing not finding
	// a specific UID?
	if( NULL == pw )  // FIXME FIXME FIXME FIXME this is definatelly wrong
	{
		fprintf( stderr, "%s: error: pw is NULL", __func__ );
		exit( EXIT_FAILURE );
	}
	admins->adminIds[admins->nAdmins]   = pw->pw_uid;
	admins->adminGIds[admins->nAdmins]  = pw->pw_gid;
	admins->adminNames[admins->nAdmins] = putstr_ (word);
	admins->nAdmins += 1;

	if (admins->nAdmins >= *numAds)
	{
		*numAds    *= 2; 
		tempIds     = realloc (admins->adminIds,   *numAds * sizeof (int));
		tempGids    = realloc (admins->adminGIds,  *numAds * sizeof (int));
		tempNames   = realloc (admins->adminNames, *numAds * sizeof (char *));
		
		if (tempIds == NULL || tempGids == NULL || tempNames == NULL) {
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "realloc");
			FREEUP (tempIds);
			FREEUP (tempGids);
			FREEUP (tempNames);

			FREEUP (admins->adminIds);
			FREEUP (admins->adminGIds);
			for ( unsigned int i = 0; i < admins->nAdmins; i++) {
				FREEUP (admins->adminNames[i]);
			}
			FREEUP (admins->adminNames);
			admins->nAdmins = 0;
			lserrno = LSE_MALLOC;
			return -1;
		}
		else {
			admins->adminIds = tempIds;
			admins->adminGIds = tempGids;
			admins->adminNames = tempNames;
	   }
	}
	
  return 0;
}

int
isInlist ( const char **adminNames, const char *userName, unsigned int actAds)
{

	if ( 0 == actAds) {
		return FALSE;
	}
	
	for ( unsigned int i = 0; i < actAds; i++) {
	
		if (strcmp (adminNames[i], userName) == 0) {
			return TRUE;
		}
	}
		
		return FALSE;

}

char *
getBeginLine_conf (const struct lsConf *conf, size_t *lineNum)
{
	char *sp = NULL;
	char *wp = NULL;

	if (conf == NULL) {
		return NULL;
	}

	// for (;;)
	// {
	// 	sp = getNextLineC_conf (conf, lineNum, TRUE);
	// 	if (sp == NULL) {
	// 		return NULL;
	// 	}

	// 	wp = getNextWord_ (&sp);
	// 	if (wp && (strcasecmp (wp, "begin") == 0)) {
	// 		return sp;
	// 	}
	// }
	do
	{
		const char begin[] = "begin";
		sp = getNextLineC_conf (conf, lineNum, TRUE);
		if( !sp ) { break; }
		wp = getNextWord_ (&sp);
		if (wp && (strcasecmp (wp, begin) == 0)) {
			return sp;
		}
	} while( NULL != sp )

	return NULL;
}

void
doSkipSection_conf (const struct lsConf *conf, size_t *lineNum, const char *lsfile, const char *sectionName)
{
	char *word = NULL;
	char *cp   = NULL;

	if (conf == NULL) {
		return;
	}

	while ((cp = getNextLineC_conf (conf, lineNum, TRUE)) != NULL)
	{
		word = getNextWord_ (&cp);
		if (strcasecmp (word, "end") == 0)
		{
			word = getNextWord_ (&cp);
			if (!word)
			{
		   /* catgets 5419 */
				ls_syslog (LOG_ERR, "5419: %s(%d): Section ended without section name, ignored", lsfile, *lineNum);
			}
			else
			{
				if (strcasecmp (word, sectionName) != 0) {
			/* catgets 5420 */
					ls_syslog (LOG_ERR, "5420: %s(%d): Section %s ended with wrong section name: %s, ignored", lsfile, *lineNum, sectionName, word);
				}
			}
			return;
		}
	}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, lsfile, *lineNum, sectionName);

	return;
}
