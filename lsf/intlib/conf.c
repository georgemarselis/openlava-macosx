/* $Id: conf.c 397 2007-11-26 19:04:00Z mblack $
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

#include <strings.h>

#include "libint/intlibout.h"
#include "lib/lproto.h"
#include "lib/words.h"
#include "lib/conf.h"
#include "lib/misc.h"

// #ifndef NL_SETN
// #define NL_SETN 22
// #endif


char *
getNextValue (char **line)
{
	return getNextValueQ_ (line, '(', ')');
}

int
keyMatch (struct keymap *keyList, char *line, int exact)
{
	int pos = 0;
	char *sp = line;
	char *word;
	int found;

	int i = 0;
	while (keyList[i].key != NULL) {
		keyList[i].position = -1;
		i++;
	}

	while ((word = getNextWord_ (&sp)) != NULL) {
		i = 0;
		found = FALSE;
		while (keyList[i].key != NULL) {
			if (strcasecmp (word, keyList[i].key) == 0) {
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
	while (keyList[i].key != NULL) {

		if (keyList[i].position == -1) {
			return FALSE;
		}
		
		i++;
	}

	return TRUE;
}

int
isSectionEnd (char *linep, char *lsfile, size_t *lineNum, char *sectionName)
{
	char *word = NULL;
	char *buff = malloc( sizeof(char) * 1024 + 1 );

	word = getNextWord_ (&linep);
	if (strcasecmp (word, "end") != 0) {
		return FALSE;
	}

	word = getNextWord_ (&linep);
	if (!word) {
		/* catgets 5400 */
		sprintf( buff, "NL_SETN, 5400, %s(%zu): section %s ended without section name, ignored", lsfile, *lineNum, sectionName);
		ls_syslog (LOG_ERR, buff  );
		return TRUE;
	}

	if (strcasecmp (word, sectionName) != 0) {
		/* catgets 5401 */
		sprintf( buff, "NL_SETN, 5401, %s(%zu): section %s ended with wrong section name %s, ignored", lsfile, *lineNum, sectionName, word );
		ls_syslog (LOG_ERR, buff );
	}

	return TRUE;
}

char *
getBeginLine (FILE *fp, size_t *lineNum)
{
	char *sp = NULL;
	char *wp = NULL;

	for (;;)
	{
		sp = getNextLineC_ (fp, lineNum, TRUE);
		if (!sp) {
			return NULL;
		}

	  wp = getNextWord_ (&sp);
	  if (wp && (strcasecmp (wp, "begin") == 0))
  return sp;
	}

}

int
readHvalues (struct keymap *keyList, char *linep, FILE *fp, char *lsfile, size_t *lineNum, int exact, char *section)
{
	char *key;
	char *value;
	char *sp, *sp1;
	char buff[2048];     // FIXME FIXME FIXME 2048 seems awfuly specific
	char error = FALSE;
	unsigned int i = 0;

	sp = linep;
	key = getNextWord_ (&linep);
	if ((sp1 = strchr (key, '=')) != NULL) {
		*sp1 = '\0';
	}

	value = strchr (sp, '=');
	if (!value)
	{
		/* catgets 5402  */
		sprintf( buff, "5402, %s: %s(%zu): missing '=' after keyword %s, section %s ignored", __PRETTY_FUNCTION__, lsfile, *lineNum, key, section);
		ls_syslog (LOG_ERR, buff );
		doSkipSection (fp, lineNum, lsfile, section);
		return -1;
	}
	value++;
  
	while (*value == ' ') {
		value++;
	}

	if (value[0] == '\0')
	{
		/* catgets 5403  */
		sprintf( buff, "5403, %s: %s(%zu): nul value after keyword %s, section %s ignored", __PRETTY_FUNCTION__, lsfile, *lineNum, key, section);
		ls_syslog (LOG_ERR, buff );
		return -1;
	}

	if( value[0] == '(' )
	{
		value++;
		if( ( sp1 = strrchr( value, ')' ) ) != NULL ) {
			*sp1 = '\0';
		}
	}

	if( putValue( keyList, key, value ) < 0)
	{
		char buff[2048] = "";
		/* catgets 5404  */
		sprintf( "5404, %s: %s(%d): bad keyword %s in section %s, ignoring the section", __PRETTY_FUNCTION__, lsfile, *lineNum, key, section );
		ls_syslog (LOG_ERR, buff );
		doSkipSection (fp, lineNum, lsfile, section);
		return -1;
	}

	if ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
	{
		if (isSectionEnd (linep, lsfile, lineNum, section))
		{
			if (!exact) {
				return 0;
			}

			i = 0;
			while (keyList[i].key != NULL)
			{
				if (keyList[i].val == NULL)
				{
					/* catgets 5405  */
					char buffer[2048] = "";
					sprintf( buffer, "5405: %s: %s(%zu): required keyword %s is missing in section %s, ignoring the section", __PRETTY_FUNCTION__, lsfile, *lineNum, keyList[i].key, section);
					ls_syslog (LOG_ERR, buff );
					error = TRUE;
				}
				i++;
			}

			if (error)
			{
				i = 0;
				while (keyList[i].key != NULL)
				{
					if (keyList[i].val != NULL) {
						free (keyList[i].val);
					}
					i++;
				}
				return -1;
			}

			return 0;
		}

		return readHvalues( keyList, linep, fp, lsfile, lineNum, exact, section );
	}

	/* catgets 5406  */
	sprintf( buff, "5406: %s: %s(%zu): Premature EOF in section %s", __PRETTY_FUNCTION__, lsfile, *lineNum, section );
	ls_syslog (LOG_ERR, buff); 

	return -1;
}

int
putValue (struct keymap *keyList, char *key, char *value)
{
	unsigned int i = 0;

	while (keyList[i].key != NULL)
	{
		if (strcasecmp (keyList[i].key, key) == 0)
  		{
			if (keyList[i].val != NULL) {
	  			free (keyList[i].val);
			}

			if (strcmp (value, "-") == 0) {
				keyList[i].val = putstr_ ("");
			}
			else {
				keyList[i].val = putstr_ (value);
			}

			return 0;
  		}

		i++;
	} // end while (keyList[i].key != NULL)

	return -1;
}

void
doSkipSection (FILE *fp, size_t *lineNum, char *lsfile, char *sectionName)
{
	char *word = NULL;
	char *cp   = NULL;
	char buffer[2048] = "";

	while( ( cp = getNextLineC_( fp, lineNum, TRUE ) ) != NULL )
	{
		word = getNextWord_ (&cp);
		if (strcasecmp (word, "end") == 0)
		{
			word = getNextWord_ (&cp);
			if (!word)
			{
				char buffer[2048] = "";
				sprintf( buffer,  "5400: %s(%zu): Section ended without section name, ignored", lsfile, *lineNum);
				ls_syslog (LOG_ERR, buffer) ;
			}
			else {
				if (strcasecmp (word, sectionName) != 0) {
					char buffer[2048] = "";
					sprintf( buffer,  "5401: %s(%zu): Section %s ended with wrong section name %s, ignored", lsfile, *lineNum, sectionName, word);
					ls_syslog (LOG_ERR, buffer );
				}
	  		}
		
			return;
		}
	}

	/* catgets 5409  */
	sprintf( buffer, "5409: %s: %s(%zu): premature EOF in section", "doSkipSection", lsfile, *lineNum);
	ls_syslog (LOG_ERR, buffer );

	return;
}

int
mapValues (struct keymap *keyList, char *line)
{
	int pos = 0;
	char *value;
	int found;
	int numv = 0;
	unsigned int i = 0;

	while( keyList[ i ].key != NULL )
	{
		keyList[i].val = NULL;
		if (keyList[i].position != -1) {
			numv++;
		}

		i++;
	}

	while( ( value = getNextValue( &line ) ) != NULL )
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
			else {
				keyList[i].val = putstr_ (value);
			}
			found = TRUE;
			break;
		}
	
		if( !found ) {
			i = 0;
			while (keyList[i].key != NULL)
			{
				if (keyList[i].val != NULL)
				{
					free (keyList[i].val);
					keyList[i].val = NULL;
				}

				i++;
			}
			return -1;
		}

		pos++;
	}

	if( pos != numv ) {
		unsigned int i = 0;
		while (keyList[i].key != NULL)
		{
			if (keyList[i].val != NULL)
			{
				free (keyList[i].val);
				keyList[i].val = NULL;
			}

			i++;
		}
		return -1;
	}

return 0;

// fail:
// 	i = 0;
// 	while (keyList[i].key != NULL)
// 	{
// 		if (keyList[i].val != NULL)
// 		{
// 			free (keyList[i].val);
// 			keyList[i].val = NULL;
// 		}

// 		i++;
// 	}
// 	return -1;

}

int
putInLists (char *word, struct admins *admins, unsigned int *numAds, char *forWhat)
{
	struct passwd *pw = NULL;
	char **tempNames;
	// int i = 0;
	uid_t *tempIds = NULL;
	gid_t *tempGids = NULL;

	if( !( pw = getpwnam( word ) ) ) {
		ls_syslog (LOG_ERR, "%s: <%s> is not a valid user name; ignored", __PRETTY_FUNCTION__, word);
		return 0;
	}

	if( isInlist( admins->adminNames, pw->pw_name, admins->nAdmins ) )
	{
		/* catgets 5411  */
		ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5411, "%s: Duplicate user name <%s> %s; ignored"), __PRETTY_FUNCTION__, word, forWhat);
		return 0;
	}

	admins->adminIds[admins->nAdmins] = pw->pw_uid;
	admins->adminGIds[admins->nAdmins] = pw->pw_gid;
	admins->adminNames[admins->nAdmins] = putstr_ (pw->pw_name);
	admins->nAdmins += 1;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG, "putInLists: uid %d gid %d name <%s>", pw->pw_uid, pw->pw_gid, pw->pw_name);
	}

  	if (admins->nAdmins >= *numAds)
	{
		*numAds   *= 2;
		tempIds   = realloc(admins->adminIds,   *numAds * sizeof( uid_t ) );
		tempGids  = realloc(admins->adminGIds,  *numAds * sizeof( gid_t ) );
		tempNames = realloc(admins->adminNames, *numAds * sizeof( char * ) );

		if (tempIds == NULL || tempGids == NULL || tempNames == NULL)
  		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __PRETTY_FUNCTION__, "realloc");
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
		else
		{
			admins->adminIds = tempIds;
			admins->adminGIds = tempGids;
			admins->adminNames = tempNames;
  		}
	}

	return 0;
}

int
isInlist (char **adminNames, char *userName, unsigned int actAds)
{

	if (actAds == 0) {
		return FALSE;
	}

	for( unsigned int i = 0; i < actAds; i++)
	{
		if (strcmp (adminNames[i], userName) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}
