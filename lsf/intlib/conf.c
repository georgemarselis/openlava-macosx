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
keyMatch (struct keymap *keyList, const char *line, int exact)
{
	char *sp   = line;
	char *word = NULL;
	int pos    = 0;
	int found  = 0;

	unsigned int i = 0;
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
isSectionEnd (char *linep, const char *lsfile, const size_t *lineNum, const char *sectionName)
{
	char *word = NULL;
	char *buff = malloc( sizeof(char) * 1024 + 1 ); // FIXME FIXME FIXME 1024 is awfuly particular
	char end[] = "end";

	word = getNextWord_ (&linep);
	if (strcasecmp (word, end) != 0) {
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


// DELETEME code was duplicated in liblsf/confmisc.c : moved over there
// char *
// getBeginLine (FILE *fp, size_t *lineNum)
// {
// 	char *sp = NULL;
// 	char *wp = NULL;
// 	const char begin[] = "begin";

// 	// for (;;)
// 	// {
// 	// 	sp = getNextLineC_ (fp, lineNum, TRUE);
// 	// 	if (!sp) {
// 	// 		return NULL;
// 	// 	}

// 	// 	wp = getNextWord_ (&sp);
// 	// 	if (wp && (strcasecmp (wp, "begin") == 0)) {
// 	// 		return sp;
// 	// 	}
// 	// }
// 	// FIXME FIXME refactoring to avoid infinite loop
// 	do {
// 		sp = getNextLineC_( fp, lineNum, TRUE);

// 		wp = getNextWord_( &sp );
// 		if (wp && (strcasecmp( wp, begin ) == 0)) {
// 			return sp;
// 		}
// 	} while( sp )

// 	return NULL;
// }

int
readHvalues (struct keymap *keyList, char *linep, FILE *fp, const char *lsfile, size_t *lineNum, int exact, char *section)
{
	char *key   = NULL;
	char *value = NULL;
	char *sp    = NULL;
	char *sp1   = NULL;
	char buffer[MAXLINELEN];     // FIXME FIXME FIXME 2048 seems awfuly specific
	char error = FALSE;
	unsigned int i = 0;

	sp = linep;
	key = getNextWord_ (&linep);
	if ((sp1 = strchr (key, '=')) != NULL) {
		*sp1 = '\0';
	}

	value = strchr (sp, '=');
	if( !value )
	{
		/* catgets 5402  */
		sprintf( buffer, "5402, %s: %s(%zu): missing '=' after keyword %s, section %s ignored", __func__, lsfile, *lineNum, key, section);
		ls_syslog (LOG_ERR, buffer );
		doSkipSection (fp, lineNum, lsfile, section);
		return -1;
	}
	value++;
  
	while (*value == ' ') {
		value++;
	}

	if( !value )
	{
		/* catgets 5403  */
		sprintf( buffer, "5403, %s: %s(%zu): NULL value after keyword %s, section %s ignored", __func__, lsfile, *lineNum, key, section);
		ls_syslog (LOG_ERR, buffer );
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
		char buff[MAXLINELEN];
		memset ( buff, 0, strlen( buff ) );
		/* catgets 5404  */
		sprintf( "5404, %s: %s(%d): bad keyword %s in section %s, ignoring the section", __func__, lsfile, *lineNum, key, section );
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
					char buff[MAXLINELEN];
					memset ( buff, 0, strlen( buff ) );
					/* catgets 5405  */
					sprintf( buffer, "5405: %s: %s(%zu): required keyword %s is missing in section %s, ignoring the section", __func__, lsfile, *lineNum, keyList[i].key, section);
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
	sprintf( buffer, "5406: %s: %s(%zu): Premature EOF in section %s", __func__, lsfile, *lineNum, section );
	ls_syslog (LOG_ERR, buffer); 

	return -1;
}

int
putValue (struct keymap *keyList, const char *key, const char *value)
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
doSkipSection (FILE *fp, size_t *lineNum, const char *lsfile, const char *sectionName)
{
	char *word = NULL;
	char *cp   = NULL;
	char end[] = "end";
	char buffer[MAXLINELEN];
	memset( buffer, 0, strlen( buffer) );

	while( ( cp = getNextLineC_( fp, lineNum, TRUE ) ) != NULL )
	{
		word = getNextWord_ (&cp);
		if (strcasecmp (word, end ) == 0)
		{
			word = getNextWord_ (&cp);
			if (!word)
			{
				char buffer[MAXLINELEN];
				memset( buffer, 0, strlen( buffer) );
				sprintf( buffer,  "5400: %s(%zu): Section ended without section name, ignored", lsfile, *lineNum);
				ls_syslog (LOG_ERR, buffer) ;
			}
			else {
				if (strcasecmp (word, sectionName) != 0) {
					char buffer[MAXLINELEN] = "";
					memset( buffer, 0, strlen( buffer) );
					sprintf( buffer,  "5401: %s(%zu): Section %s ended with wrong section name %s, ignored", lsfile, *lineNum, sectionName, word);
					ls_syslog (LOG_ERR, buffer );
				}
	  		}
		
			return;
		}
	}

	/* catgets 5409  */
	sprintf( buffer, "5409: %s: %s(%zu): premature EOF in section", __func__, lsfile, *lineNum);
	ls_syslog (LOG_ERR, buffer );

	return;
}

int
mapValues (struct keymap *keyList, char *line)
{
	char *value = NULL;
	int pos     = 0;
	int found   = 0;
	int numv    = 0;
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
					// keyList[i].val = NULL;
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

// int putInLists (char *word, struct admins *admins, unsigned int *numAds, const char *forWhat)
// 			used to live here. Moved over to lib/liblsf/conf/putInLists.c

int
isInlist (char **adminNames, const char *userName, unsigned int actAds)
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
