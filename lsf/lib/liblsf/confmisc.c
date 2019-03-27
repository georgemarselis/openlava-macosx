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
#include "lib/words.h"
#include "lib/conf.h"
#include "lib/misc.h"
#include "lib/wconf.h"


char *
getNextValue (char **line)
{
    return getNextValueQ_ (line, '(', ')');
}

int
keyMatch (struct keymap *keyList, const char *line, int exact)
{
    int found      = FALSE;
    size_t pos     = 0;
    unsigned int i = 0;
    char *sp       = NULL;
    char *word     = NULL;

    sp = malloc( strlen( line ) * sizeof( char ) + 1 );
    strcpy( sp, line );

    while (keyList[i].key != NULL)
    {
        keyList[i].position = strlen( keyList[i].key ) + 10; // strlen( keyList[i].key ) is already an upper bound.
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
                if ( keyList[i].position != strlen( keyList[i].key ) + 10 ) {
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
        if (keyList[i].position == strlen( keyList[i].key ) + 10) { // strlen( keyList[i].key ) is already an upper bound
            return FALSE;
        }
        i++;
    }

    sp = NULL; free( sp );

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
    } while( sp );

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

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, section); /*catgets33 */
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

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, sectionName); /*catgets33 */
    return;
}

int
mapValues (struct keymap *keyList, char *line) // FIXME FIXME should char *line be const char * ?
{
    char *value    = NULL;
    int found      = 0; // FALSE
    size_t pos     = 0;
    size_t numv    = 0;
    unsigned int i = 0;

    while (keyList[i].key != NULL) {

        FREEUP (keyList[i].val);
        if (keyList[i].position != strlen( keyList[i].key ) + 10 ) {
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
    //  sp = getNextLineC_conf (conf, lineNum, TRUE);
    //  if (sp == NULL) {
    //      return NULL;
    //  }

    //  wp = getNextWord_ (&sp);
    //  if (wp && (strcasecmp (wp, "begin") == 0)) {
    //      return sp;
    //  }
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
    } while( NULL != sp );

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

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, sectionName); /*catgets33 */

    return;
}



// int putInLists2 (char *word, struct admins *admins, unsigned int *numAds, const char *forWhat)
//      used to live here. moved to lsf/lib/liblsf/putInLists.c


/***************************************************************************************
 * Function prototype: int isInList ( const char *list, const char *string)
 *
 * INPUT ; const char *list, const char *string
 * OUTPUT: TRUE/FALSE
 *
 * WHAT DOES THIS FUNCTION DO:
 *  Compares the two input strings for equality
 * 
 */
int
isInList1 ( const char *list, const char *string)
{
    char *word = NULL;
    
    if (list == NULL || string == NULL || list[0] == '\0' || string[0] == '\0') {
        return FALSE;
    }
    
    while( ( word = getNextWord_( &list ) ) != NULL) {
        if (strcmp( word, string ) == 0) {
            return TRUE;
        }
    }
    return FALSE;
    
}

/********************************************************************************************
 *
 * Function prototype: int isInlist ( char **adminNames, char *userName, unsigned int actAds)
 *
 * INPUT : list of admin names, username strings, max size of admin names array
 * OUTPUT: TRUE/FALSE, depending if userName is in adminNames 
 */
int
isInlist2 ( char **adminNames, char *userName, unsigned int actAds) // FIXME FIXME FIXME FIXME FIXME turn this into const char references, since nothing gets altered in the function
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


// int
// isInlist3 ( char **adminNames, char *userName, unsigned int actAds)
// {

//     if (actAds == 0) {
//         return FALSE;
//     }

//     for( unsigned int i = 0; i < actAds; i++)
//     {
//         if (strcmp (adminNames[i], userName) == 0) {
//             return TRUE;
//         }
//     }

//     return FALSE;
// }


