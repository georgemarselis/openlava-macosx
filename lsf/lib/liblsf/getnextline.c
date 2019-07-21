// Added by George Marselis <george@marsel.is> Tue March 26 2019

#include "lsf.h"
#include "lib/misc.h"
#include "lib/wconf.h"
#include "lib/syslog.h"
#include "lib/getnextline.h"

// char *
// getNextLine_conf ( const struct lsConf *conf, int confFormat) // FIXME FIXME FIXME FIXME this wrapper function has to go
// {
//     size_t *dummy = 0;

//     assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain    
//     return getNextLineC_conf (conf, dummy, confFormat);
// }

char *
getNextLineD_ (FILE * fp, size_t *LineCount, int confFormat)
{
    int cin           = 0;
    int oneChar       = 0;
    int cinBslash     = 0;
    int quotes        = 0;
    size_t lpos       = 0;
    size_t linesize   = MAX_LINE_LEN;
    static char *line = NULL;
    
    lserrno = LSE_NO_ERR;
    oneChar = -1;

    if( NULL != line) {
        line = NULL;
        free( line );
    }

    line = calloc (1, MAX_LINE_LEN);
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
                    linesize += MAX_LINE_LEN;
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
            linesize += MAX_LINE_LEN;
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
        return line;
    }

    return NULL;
}

char *
getNextLineC_ (FILE *fp, size_t *lineCount, int confFormat)
{
    char *sp       = NULL;
    char *nextLine = NULL;

    nextLine = getNextLineD_ (fp, lineCount, confFormat);

    if (nextLine == NULL) {
        return NULL;
    }

    for (sp = nextLine; *sp != '\0'; sp++) {
        if (*sp != ' ') {
            return nextLine;
        }
    }

    return getNextLineC_ (fp, lineCount, confFormat);

}


// char *
// getNextLine_ (FILE * fp, int confFormat)
// {
//     size_t *zeroLineCount = 0;
//     return getNextLineC_ (fp, zeroLineCount, confFormat);
// }

// FIXME FIXME FIXME delete after cleanup 
//
// char *
// getNextLineC_conf( const struct lsConf *conf, size_t *LineCount, int confFormat)
// {
//     char *sp               = NULL;
//     char *cp               = NULL;
//     char *line             = NULL;
//     int toBeContinue       = 0;
//     int isUNCPath          = 0;
//     unsigned long len      = 0;
//     unsigned long linesize = 0;

//     static char *longLine = NULL;
//     static char *myLine = NULL;

//     if (conf == NULL) { 
//         return NULL;
//     }

//     FREEUP (longLine);

//     if (confFormat) {
//         do {
//             line = readNextLine (conf, LineCount);
//             if (line == NULL) {
//                 return NULL;
//             }

//             toBeContinue = 0;
//             FREEUP (myLine);
//             len = strlen (line) + 1;
//             myLine = malloc (len * sizeof (char));
//             if (myLine == NULL) {
//                 return NULL;
//             }

//             sp = line;
//             cp = myLine;
//             while (sp != &(line[len - 1]))
//             {
//                 if (*sp == '#')
//                 {
//                     break;
//                 }
//                 else if (*sp == '\\')
//                 {

//                     if (sp == &(line[len - 2]))
//                     {

//                         sp++;
//                         toBeContinue = 1;
//                     }
//                     else
//                     {

//                         if (!isUNCPath && *(sp + 1) == '\\' && !isspace (*(sp + 2))) {
//                             isUNCPath = 1;
//                         }
//                         if (!isspace (*(sp + 1)))
//                         {
//                             *cp = *sp;
//                             sp++;
//                             cp++;
//                         }
//                         else
//                         {
//                             sp++;
//                             sp++;
//                         }
//                     }
//                 }
//                 else if (isspace (*sp))
//                 {
//                     *cp = ' ';
//                     sp++;
//                     cp++;
//                 }
//                 else
//                 {
//                     *cp = *sp;
//                     sp++;
//                     cp++;
//                 }
//             }
//             *cp = '\0';

//             if (!toBeContinue)
//             {
//                 while (cp != myLine && *(--cp) == ' ');

//                 if (cp == myLine && (*cp == ' ' || *cp == '\0'))
//                 {
//                     *cp = '\0';
//                 }
//                 else
//                     *(++cp) = '\0';
//             }

//             if (!(myLine[0] == '\0' && !longLine))  // FIXME FIXME FIXME what line[0] represents?
//             {

//                 if (longLine)
//                 {
//                     linesize += strlen (myLine);
//                     sp = malloc (linesize * sizeof (char));
//                     if (sp == NULL) {
//                         return longLine;
//                     }

//                     strcpy (sp, longLine);
//                     strcat (sp, myLine);
//                     FREEUP (longLine);
//                     longLine = sp;
//                 }
//                 else
//                 {
//                     linesize = strlen (myLine) + 1;
//                     longLine = malloc (linesize * sizeof (char));
//                     strcpy (longLine, myLine);
//                 }
//             }

//         }
//         while ((myLine[0] == '\0' && !longLine) || toBeContinue);  // FIXME FIXME FIXME what line[0] represents?


//         return longLine;
//     }
//     else
//     {
//         do
//         {
//             line = readNextLine (conf, LineCount);
//             if (line == NULL) {
//                 return NULL;
//             }
//         }
//         while (line[0] == '\0'); { // FIXME FIXME FIXME what line[0] represents?
//             return line;
//         }
//     }

//     fprintf( stderr, "%s: you are not suppposed to be here!", __func__ );
//     ls_syslog( LOG_ERR, "%s: you are not suppposed to be here!", __func__ );
//     return NULL;
// }


char *
getNextLineC_conf ( const struct lsConf *conf, size_t *LineCount, int confFormat)
{
    char *sp               = NULL;
    char *cp               = NULL;
    char *line             = NULL;
    int toBeContinue       = 0;
    int isUNCPath          = 0;
    unsigned long len      = 0;
    unsigned long linesize = 0;

    static char *longLine = NULL;
    static char *myLine = NULL;

    if (conf == NULL) { 
        return NULL;
    }

    // longLine = NULL;
    // free(longLine);

    if (confFormat) {
        do {
            line = readNextLine (conf, LineCount);
            if (line == NULL) {
                return NULL;
            }

            toBeContinue = 0;
            FREEUP (myLine);
            len = strlen (line) + 1;
            myLine = malloc (len * sizeof (char));
            if (myLine == NULL) {
                return NULL;
            }

            sp = line;
            cp = myLine;
            while (sp != &(line[len - 1])) {
                if (*sp == '#') {
                    break;
                }
                else if (*sp == '\\') {

                    if (sp == &(line[len - 2])) {
                        sp++;
                        toBeContinue = 1;
                    }
                    else {

                        if (!isUNCPath && *(sp + 1) == '\\' && !isspace (*(sp + 2))) {
                            isUNCPath = 1;
                        }
                        if (!isspace (*(sp + 1))) {
                            *cp = *sp;
                            sp++;
                            cp++;
                        }
                        else {
                            sp++; // FIXME FIXME FIXME FIXME FIXME this first invocation might actually be cp++ ;
                            sp++;
                        }
                    }
                }
                else if (isspace (*sp)) {
                    *cp = ' ';
                    sp++;
                    cp++;
                }
                else {
                    *cp = *sp;
                    sp++;
                    cp++;
                }
            }
            *cp = '\0';

            if (!toBeContinue) {
                while (cp != myLine && *(--cp) == ' ');

                if (cp == myLine && (*cp == ' ' || *cp == '\0')) {
                    *cp = '\0';
                }
                else
                    *(++cp) = '\0';
            }

            if (!(myLine[0] == '\0' && !longLine)) {

                if (longLine) {
                    linesize += strlen (myLine);
                    sp = malloc (linesize * sizeof (char));
                    if (sp == NULL) {
                        return longLine;
                    }

                    strcpy (sp, longLine);
                    strcat (sp, myLine);
                    FREEUP (longLine);
                    longLine = sp;
                }
                else {
                    linesize = strlen (myLine) + 1;
                    longLine = malloc (linesize * sizeof (char));
                    strcpy (longLine, myLine);
                }
            }

        }
        while ((myLine[0] == '\0' && !longLine) || toBeContinue);

        return longLine;
    }
    else {
        do {
            line = readNextLine (conf, LineCount);
            if (line == NULL) {
                return NULL;
            }
        }
        while (line[0] == '\0'); {
            return line;
        }
    }

    fprintf( stderr, "%s: you are not suppposed to be here!\n", __func__ );
    ls_syslog( LOG_ERR, "%s: you are not suppposed to be here!", __func__ );
    return NULL;
}