/* $Id: lib.words.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/words.h"

char *
getNextLine_ (FILE * fp, int confFormat)
{
  size_t *zeroLineCount = 0;
  return (getNextLineC_ (fp, zeroLineCount, confFormat));
}

char *
getNextWord_ (char **line)
{
  static char word[4 * MAXLINELEN];
  char *wordp = word;

  while (isspace (**line)) {
	(*line)++;
  }

  while (**line && !isspace (**line)) {
	*wordp++ = *(*line)++;
  }

  if (wordp == word) {
	return NULL;
  }

  *wordp = '\0';
  return (word);
}

char *
getNextWord1_ (char **line)
{
  static char word[4 * MAXLINELEN];
  char *wordp = word;

  while (isspace (**line)) {
	(*line)++;
  }


  while (**line && !isspace (**line) && (**line != ',') && (**line != ']') && (**line != '[')) {
	*wordp++ = *(*line)++;
  }

  if (wordp == word) {
	return NULL;
  }


  *wordp = '\0';
  return (word);

}

static int
charInSet (char c, const char *set)
{
  while (*set != '\0')
	{
	  if (c == *set)
	{
	  return TRUE;
	}
	  set++;
	}
  return FALSE;
}

char *
getNextWordSet (char **line, const char *set)
{
  static char word[4 * MAXLINELEN];
  char *wordp = word;


  while (charInSet (**line, set))
	(*line)++;


  while (**line && !charInSet (**line, set))
	*wordp++ = *(*line)++;

  if (wordp == word)

	return NULL;


  *wordp = '\0';
  return (word);

}

char *
getNextValueQ_ (char **line, char ch1, char ch2)
{
	char *sp = NULL;
	char str[4] = { "aaaa" };
	static char *value = NULL;
	unsigned long valuelen = 0;

	lserrno = LSE_NO_ERR;
	sp = getNextWord_(line);
	if( !sp ) {
		return NULL;
	}

	if (sp[0] != ch1) {
		return sp;
	}

	sprintf (str, "%c", ch1);
	if( strcmp( sp, str ) == 0 )
	{
		sp = getNextWord_( line );
		if( NULL == sp )
		{
			lserrno = LSE_CONF_SYNTAX;
			return NULL;
		}
	}
	else {
		sp++;
	}


	if( NULL != value ) {
		free (value);
	}
	if( strlen( sp ) > MAXLINELEN * sizeof( char ) ) {
		valuelen = 2 * strlen( sp ); 
		value = malloc( valuelen );
	}
	else {
		value = malloc( MAXLINELEN * sizeof( char ) + 1 );
		valuelen = MAXLINELEN * sizeof (char );
	}

	if( NULL == value )
	{
		lserrno = LSE_MALLOC;
		return NULL;
	}

	strcpy( value, sp ); // FIXME FIXME FIXME possible buffer overflow
	sp = strchr( value, ch2 );
	if (NULL != sp )
	{
	  sp = NULL;
	  return value;
	}

	sprintf( str, "%c", ch2 );
	sp = getNextWord_( line );
	while( NULL != sp )
	{
		if( strcmp( sp, str ) == 0 ) {
			return value;
		}

		if( strlen( value ) + strlen( sp ) + 2 > valuelen - 1 ) {
			char *newvp = NULL;
			newvp = myrealloc (value, valuelen + strlen (sp) + MAXLINELEN);  // FIXME FIXME FIXME FIXME no need to a programmer-defined realloc?
			if( NULL == newvp ) {
				return value;
			}
			value = newvp;
			valuelen += MAXLINELEN;
		}

		strcat (value, " ");
		strcat (value, sp);
		sp = strrchr (value, ch2);
		if (sp != NULL)
		{
			sp = NULL;
			return value;
		}

		sp = getNextWord_( line );
	}

	free( value );
	lserrno = LSE_CONF_SYNTAX;
	return NULL;
}

// i think they are trying to remove all double quotes
int
stripQStr (char *q, char *str) 
{
	char *fr = q;

	for (; *q != '"' && *q != '\0'; q++) {
		;
	}
	
	if (*q == '\0') {
		return (-1);
	}

	for (q++; *q != '\0'; q++, str++) {
	  
		if (*q == '"') {
			if (*(q + 1) == '"'){
			   q++;
			}
			else {
			  *str = '\0';
			  break;
			}
		}
		
		*str = *q;
	}

	if (*q == '\0') {
		return -1;
	}

	return (q - fr + 1); // FIXME FIXME FIXME ascii gymnastics must go
}

int
addQStr (FILE * log_fp, char *str)
{
	int j = 1;

	if (putc (' ', log_fp) == EOF) {
		return -1;
	}
	if (putc ('"', log_fp) == EOF) {
		return -1;
	}
  
	for (; *str != '\0'; str++, j++)
	{
		if (*str == '"') {
			if (putc ('"', log_fp) == EOF) {
				return -1;
			}
		}
		if (putc (*str, log_fp) == EOF) {
			return -1;
		}
	}
	if (putc ('"', log_fp) == EOF) {
		return -1;
	}

	return 0;
}

char *
getNextLineD_ (FILE * fp, size_t *LineCount, int confFormat)
{
	static char *line = NULL;
	int cin         = 0;
	int oneChar     = 0;
	int cinBslash   = 0;
	int quotes      = 0;
	size_t lpos     = 0;
	size_t linesize = MAXLINELEN;
	
	lserrno = LSE_NO_ERR;
	oneChar = -1;

	if( NULL != line) {
		FREEUP (line);
	}

	line = calloc (1, MAXLINELEN);
	if (NULL == line && ENOMEM == errno ) {
		lserrno = LSE_MALLOC;
		return NULL;
	}

	lpos = 0;
	while ((cin = getc (fp)) != EOF) {

		if (cin == '\n') {
			*LineCount += 1;
			break;
		}
		
		if (cin == '"') { 

			if (quotes == 0) {
				quotes++;
			}
			else {
				quotes--;
			}
		}
		
		if (confFormat && cin == '#' && quotes == 0)
		{

			while ((cin = getc (fp)) != EOF)  {
				
				if (cin == '\n') {
					*LineCount += 1;
					break;
				}
			}
			
			break;
		}

		if (confFormat && cin == '\\')
		{
			cinBslash = cin;
			if ((cin = getc (fp)) == EOF) {
				break;
			}

			if (cin == '\n') {
				*LineCount += 1;
			}
			else if (!isspace (cin))
			{
				if (lpos < linesize - 1) {
					assert( cinBslash >= 0);
					line[lpos++] = (char) cinBslash;
				}
				else {
					char *sp;
					linesize += MAXLINELEN;
					sp = myrealloc (line, linesize);

					if (sp == NULL) {
						break;
					}

					line = sp;
					assert( cinBslash >= 0);
					line[lpos++] = (char) cinBslash;
				}
			}
			else {
				fprintf( stderr, "%s: You are not supposed to be here.\n", __func__ );
			}
		}

		if (isspace (cin)) {
			cin = ' ';
		}
	  
		if (lpos < linesize - 1) {
			assert( cin >= 0);
			line[lpos++] = (char) cin;
		}
		else
		{
			char *sp;
			linesize += MAXLINELEN;
			sp = myrealloc (line, linesize);
			if ( NULL == sp && ENOMEM == errno ) {
				break;
			}
			line = sp;
			assert( cin >= 0);
			line[lpos++] = (char)cin;
		}
	}

	if ( 1 == lpos ) {
		oneChar = 1;
	}


	while (lpos > 0 && (line[--lpos] == ' ')) {
		;
	}

	if ((cin != EOF) || (oneChar == 1) || (cin == EOF && lpos > 0))
	{
		line[++lpos] = '\0';
		return (line);
	}

	return NULL;
}

char *
getNextLineC_ (FILE * fp, size_t *LineCount, int confFormat)
{
	char *nextLine;
	char *sp;

	nextLine = getNextLineD_ (fp, LineCount, confFormat);

	if (nextLine == NULL) {
		return NULL;
	}

	for (sp = nextLine; *sp != '\0'; sp++) {
		if (*sp != ' ') {
			return (nextLine);
		}
	}

	return (getNextLineC_ (fp, LineCount, confFormat));

}


void
subNewLine_ (char *instr)
{
	size_t strlength = strlen( instr );
	if (instr && strlength > 0) {
		for ( ssize_t i = (ssize_t) strlength - 1; i > -1; i--) {
			if (instr[i] == '\n') {
				for ( size_t k = i; k < strlength; k++) {
					instr[k] = instr[k + 1];
				}
			}
		}
	}

	return;
}

/* Get the next meaningful line from the configuration
 * file, meaningful is a line that is not isspace()
 * and that does not start with a comment # character.
 */
char *
nextline_ (FILE * fp)
{
	static char *buf = NULL;
	char *p = NULL;

	buf = malloc( BUFSIZ * sizeof( char ) + 1);
	while( fgets( buf, BUFSIZ, fp ) ) {
		p = buf;
		while( isspace( *p ) ) {
			++p;
		}
		if (*p == '#' || *p == 0)
		{
		/* If this is the last or only
		 * line do not return the
		 * previous buffer to the caller.
		 */
			p = NULL;
			continue;
		}
		break;
	}

	return (p);
}               /* nextline_() */
