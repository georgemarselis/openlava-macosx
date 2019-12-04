// Added by George Marselis <george@marsel.is> 


#include <stdio.h>
#include <strings.h>

#include "lsf.h"
#include "lib/doSkipSection.h"
#include "lib/getnextline.h"
#include "lib/words.h"
#include "lib/syslog.h"

void
doSkipSection_ (FILE *fp, size_t *lineNum, const char *lsfFileName, const char *sectionName)
{
    char *word = NULL;
    const char *cp   = NULL;
    char end[] = "end";
    char buffer[MAX_LINE_LEN];

    memset( buffer, '\0', strlen( buffer) );

    while( ( cp = getNextLineC_( fp, lineNum, TRUE ) ) != NULL ) {  // ONLY diff so far
        word = getNextWord_ (&cp);
        if (strcasecmp (word, end ) == 0) {
            word = getNextWord_ (&cp);
            if (!word) {
                /* catgets 5400 */
                sprintf( buffer, "5400: %s at line %lu: Section ended without section name, ignored", lsfFileName, *lineNum);
                ls_syslog (LOG_ERR, buffer) ;
            }
            else {
                if (strcasecmp (word, sectionName) != 0) {
                    char buffer[MAX_LINE_LEN];
                    memset( buffer, 0, strlen( buffer) );
                    /* catgets 5401 */
                    sprintf( buffer,  "5401: %s at line %lu: Section %s ended with wrong section name %s, ignored", lsfFileName, *lineNum, sectionName, word);
                    ls_syslog (LOG_ERR, buffer );
                }
            }
        
            return;
        }
    }

    /* catgets 5409  */
    sprintf( buffer, "5409: %s: %s(%zu): premature EOF in section", __func__, lsfFileName, *lineNum);
    ls_syslog (LOG_ERR, buffer );

    return;
}


void
doSkipSection_conf (const struct lsConf *conf, size_t *lineNum, const char *lsfFileName, const char *sectionName)
{
    char *word = NULL;
    const char *cp   = NULL;
    char end[] = "end";

    if (conf == NULL) {
        return;
    }

    while ((cp = getNextLineC_conf (conf, lineNum, TRUE)) != NULL) { // ONLY diff so far
        word = getNextWord_ (&cp);
        if (strcasecmp (word, end) == 0) {
            word = getNextWord_ (&cp);
            if (!word) {
                char buffer[MAX_LINE_LEN];
                memset( buffer, 0, strlen( buffer) );
                /* catgets 5419 */
                sprintf (buffer, "5419: %s at line %lu: Section ended without section name, ignored", lsfFileName, *lineNum);
                ls_syslog (LOG_ERR, buffer );
            }
            else {
                if (strcasecmp (word, sectionName) != 0) {
                    char buffer[MAX_LINE_LEN];
                    memset( buffer, 0, strlen( buffer) );
                    /* catgets 5420 */
                    ls_syslog (LOG_ERR, "5420: %s at line %d: Section %s ended with wrong section name: %s, ignored", lsfFileName, *lineNum, sectionName, word);
                }
            }
            return;
        }
    }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfFileName, *lineNum, sectionName); /*catgets33 */

    return;
}

void
doSkipSection (FILE * fp, size_t *lineNum, const char *lsfFileName, const char *sectionName)
{
    char *word = NULL;
    const char *cp   = NULL;
    char end[] = "end";

    while ((cp = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {
        word = getNextWord_ (&cp);
        if (strcasecmp (word, end) == 0) {
            word = getNextWord_ (&cp);
            if (!word) {
                char buffer[MAX_LINE_LEN];
                memset( buffer, 0, strlen( buffer) );
                /* catgets 5407 */
                sprintf( buffer, "5407: %s at line %lu: Section ended without section name, ignored", lsfFileName, *lineNum);
                ls_syslog (LOG_ERR, buffer);
            }
            else {
                if (strcasecmp (word, sectionName) != 0) {
                    char buffer[MAX_LINE_LEN];
                    memset( buffer, 0, strlen( buffer) );
                    /* catgets 5408 */
                    sprintf( buffer, "5408: %s at line %lu: Section %s ended with wrong section name: %s, ignored", lsfFileName, *lineNum, sectionName, word);
                    ls_syslog (LOG_ERR, buffer );
                }
            }
            return;
        }
    }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfFileName, *lineNum, sectionName); /*catgets33 */

    assert( lserrno ); // NOFIX make the compiler not complain

    return;
}