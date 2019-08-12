/* $Id: lib.wconf.c 397 2007-11-26 19:04:00Z mblack $
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

#include <ctype.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <strings.h>

#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/syslog.h"
#include "lib/wconf.h"
#include "lib/words.h"
#include "lsf.h"
#include "lib/misc.h"
#include "libint/lsi18n.h"
#include "lib/i18n.h"
#include "lib/getnextline.h"

 /*
  *     Yup. Someone coded their own recursive decent parser...
  *
  */

struct lsConf *
ls_getconf ( const char *filename)
{
    // unsigned int i      = 0;
    size_t defsize      = 0;
    size_t numDefs      = 0;
    size_t numLines     = 0;
    size_t oldLineNum   = 0; 
    size_t beginLineNum = 0;
    size_t lineNum      = 0;
    size_t len          = 0;
    long offset         = 0;
    char flag           = '\0';
    char *linep         = NULL;
    char *sp            = NULL;
    char *word          = NULL;
    char *word1         = NULL;
    char *cp            = NULL;
    char *ptr           = NULL;
    char **lines        = NULL;
    char **tmpPtr       = NULL;
    char **defNames     = NULL;
    char **defConds     = NULL;
    struct lsConf *conf = NULL;
    struct confNode *temp = NULL;
    struct confNode *node = NULL;
    struct confNode *prev = NULL;
    struct confNode *rootNode = NULL;
    FILE *fp = NULL;

    lserrno = LSE_NO_ERR;
    if (filename == NULL)
    {
        char message[ ] = "Null filename";
        /* catgets 6000 */
        ls_syslog (LOG_ERR, "%s: %s.", __func__, _i18n_msg_get (ls_catd, NL_SETN, 6000, message));
        lserrno = LSE_NO_FILE;
        return NULL;
    }

    conf = malloc( sizeof( struct lsConf ) );
    if (conf == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof( struct lsConf ) );
        lserrno = LSE_MALLOC;
        return NULL;
    }

    conf->confhandle = NULL;
    conf->numConds   = 0;
    conf->conds      = NULL;
    conf->values     = NULL;
    blockStack       = initStack ();
    if (blockStack == NULL) {
        ls_freeconf (conf);
        lserrno = LSE_MALLOC;
        return NULL;
    }
    ptrStack = initStack ();
    if (ptrStack == NULL)
    {
        freeStack (blockStack);
        ls_freeconf (conf);
        lserrno = LSE_MALLOC;
        return NULL;
    }
    defsize = 5;
    defNames = malloc (defsize * sizeof (char *));
    if (defNames == NULL)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", sizeof (defsize * sizeof (char *)));
        lserrno = LSE_MALLOC;
        goto Error;
    }
    defConds = malloc (defsize * sizeof (char *));
    if (defConds == NULL)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", sizeof (defsize * sizeof (char *)));
        lserrno = LSE_MALLOC;
        goto Error;
    }
    numDefs = 0;

    fp = fopen (filename, "r");
    if (fp == NULL)
    {
        char message [ ] = "Can't open configuration file";
        /* catgets 6001 */
        ls_syslog (LOG_ERR, "%s: %s <%s>.", __func__, _i18n_msg_get (ls_catd, NL_SETN, 6001, message), filename );
        lserrno = LSE_NO_FILE;
        goto Error;
    }

    while ((linep = getNextLineD_ (fp, &lineNum, FALSE)) != NULL)
    {

        const char **topass = NULL;
        sp = linep;

        memcpy(topass, &linep, strlen( linep ) );
        word = getNextWord_ (topass);
        if (word && word[0] == '#')
        {

            cp = word;
            cp++;

            if (strcasecmp (cp, "define") == 0)
            {
                unsigned long i = 0;

                word = getNextWord_ (topass);

                if (word == NULL)
                {
                    char message[ ] = "Both macro and condition name expected after #define";
                    /* catgets 6002 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6002, message)) ;
                    goto Error;
                }

                word1 = putstr_ (word);
                if (word1 == NULL)
                {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (word) + 1);
                    lserrno = LSE_MALLOC;
                    goto Error;
                }

                while (isspace (*linep)) {
                    linep++;
                }
                word = linep;

                if (*word != '\0')
                {
                    if ((ptr = strchr (word, '#')) != NULL) {
                        *ptr = '\0';
                    }

                    i = strlen (word) - 1;
                    while (isspace (word[i])) {
                        i--;
                    }
                    word[i + 1] = '\0';
                }

                if (*word == '\0')
                {   char message[ ] = "Both macro and condition name expected after #define";
                    /* catgets 6003 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6003, message ) );
                    FREEUP (word1);
                    goto Error;
                }

                for (i = 0; i < numDefs; i++)
                {
                    if (!strcmp (defNames[i], word)) {
                        break;
                    }
                }

                if (i < numDefs) {
                    word = defConds[i];
                }


                if (numDefs == defsize)
                {
                    tmpPtr = myrealloc (defNames, defsize * 2 * sizeof (char *));
                    if (tmpPtr != NULL) {
                        defNames = tmpPtr;
                    }
                    else
                    {
                        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "realloc", defsize * 2 * sizeof (char *));
                        FREEUP (word1);
                        lserrno = LSE_MALLOC;
                        goto Error;
                    }
                    tmpPtr = myrealloc (defConds, defsize * 2 * sizeof (char *));
                    if (tmpPtr != NULL) {
                        defConds = tmpPtr;
                    }
                    else
                    {
                        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "realloc", defsize * 2 * sizeof (char *));
                        FREEUP (word1);
                        lserrno = LSE_MALLOC;
                        goto Error;
                    }
                    defsize *= 2;
                }
                defNames[numDefs] = putstr_ (word1);
                if (defNames[numDefs] == NULL)
                {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", strlen (word1) + 1);
                    FREEUP (word1);
                    lserrno = LSE_MALLOC;
                    goto Error;
                }
                defConds[numDefs] = putstr_ (word);
                if (defConds[numDefs] == NULL)
                {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", strlen (word) + 1);
                    FREEUP (defNames[numDefs]);
                    FREEUP (word1);
                    lserrno = LSE_MALLOC;
                    goto Error;
                }
                numDefs++;

                FREEUP (word1);
                continue;
            }
            else if (strcasecmp (cp, "if") == 0)
            {
                unsigned long i = 0;

                while (isspace (*linep)) {
                    linep++;
                }
                word = linep;

                if (*word != '\0')
                {
                    if ((ptr = strchr (word, '#')) != NULL) {
                        *ptr = '\0';
                    }

                    i = strlen (word) - 1;
                    while (isspace (word[i])) {
                        i--;
                    }
                    word[i + 1] = '\0';
                }

                if (*word == '\0')
                {
                    char message[ ] = "Condition name expected after #if.";
                    /* catgets 6004 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6003, message) );    
                    goto Error;
                }

                if ((node = newNode ()) == NULL)
                {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", sizeof (struct confNode));
                    lserrno = LSE_MALLOC;
                    goto Error;
                }

                for (i = 0; i < numDefs; i++)
                {
                    if (!strcmp (defNames[i], word)) {
                        break;
                    }
                }

                if (i < numDefs)
                {

                    flag = addCond (conf, defConds[i]);
                    node->cond = putstr_ (defConds[i]);
                }
                else
                {
                    flag = addCond (conf, word);
                    node->cond = putstr_ (word);
                }
                if ( '\0' == flag || node->cond == NULL)
                {
                    const char malloc[ ] = "malloc";
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, malloc, sizeof (word));
                    lserrno = LSE_MALLOC;
                    goto Error;
                }

                if (prev != NULL) {
                    linkNode (prev, node);
                }
                prev = node;
                PUSH_STACK (blockStack, node);
                PUSH_STACK (ptrStack, node);

                if (rootNode == NULL) {
                    rootNode = node;
                }

                continue;
            }
            else if (strcasecmp (cp, "elif") == 0)
            {

                unsigned long i = 0;
                temp = popStack (blockStack);
                if (temp == NULL)
                {
                    char message[ ] = "If-less elif";
                    /* catgets 6007 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6007, message) );    
                    goto Error;
                }
                PUSH_STACK (blockStack, temp);

                while (isspace (*linep)) {
                    linep++;
                }
                word = linep;

                if (*word != '\0')
                {
                    const char pound = '#';
                    if ((ptr = strchr (word, pound )) != NULL) {
                        *ptr = '\0';
                    }

                    i = strlen (word) - 1;
                    while (isspace (word[i])) {
                        i--;
                    }
                    word[i + 1] = '\0';
                }

                if (*word == '\0')
                {
                    char message[ ] = "ICondition name expected after #elif";
                    /* catgets 6005 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6005, message) );
                    goto Error;
                }

                if ((node = newNode ()) == NULL)
                {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__,  "malloc", sizeof (struct confNode));
                    lserrno = LSE_MALLOC;
                    goto Error;
                }

                for (i = 0; i < numDefs; i++)
                {
                    if (!strcmp (defNames[i], word)) {
                        break;
                    }
                }

                if (i < numDefs)
                {

                    flag = addCond (conf, defConds[i]);
                    node->cond = putstr_ (defConds[i]);
                }
                else
                {
                    flag = addCond (conf, word);
                    node->cond = putstr_ (word);
                }
                if ( '\0' == flag || node->cond == NULL)
                {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
                    lserrno = LSE_MALLOC;
                    goto Error;
                }

                prev = popStack (ptrStack);
                prev->tag = NODE_LEFT_DONE;
                if (prev != NULL) {
                    linkNode (prev, node);
                }
                prev = node;
                PUSH_STACK (ptrStack, node);

                continue;
            }
            else if (strcasecmp (cp, "else") == 0)
            {

                temp = popStack (blockStack);
                if (temp == NULL)
                {
                    char message[ ] = "If-less else";
                    /* catgets 6008 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6008, message) );
                    goto Error;
                }
                PUSH_STACK (blockStack, temp);

                prev = popStack (ptrStack);
                prev->tag = NODE_LEFT_DONE;
                PUSH_STACK (ptrStack, prev);

                continue;
            }
            else if (strcasecmp (cp, "endif") == 0)
            {

                temp = popStack (blockStack);
                if (temp == NULL)
                {
                    char message[ ] = "If-less endif";
                    /* catgets 6009 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6009, message) );
                    goto Error;
                }
                PUSH_STACK (blockStack, temp);

                prev = popStack (blockStack);
                popStack (ptrStack);

                prev->tag = NODE_ALL_DONE;

                continue;
            }
        }

        beginLineNum = lineNum;
        numLines = 0;
        lines = NULL;
        for (;;) {  // FIXME FIXME FIXME FIXME FIXME put terminating conditions instead of infinite loop

            const char **topass = NULL;

            lines = myrealloc (lines, (numLines + 1) * sizeof (char *));
            if (lines == NULL)
            {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", (numLines + 1) * sizeof (char *));
                lserrno = LSE_MALLOC;
                goto Error;
            }
            lines[numLines] = malloc ((strlen (sp) + 1) * sizeof (char));
            if (lines[numLines] == NULL)
            {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", (strlen (sp) + 1) * sizeof (char));
                lserrno = LSE_MALLOC;
                goto Error;
            }
            strcpy (lines[numLines], sp);
            numLines++;
            offset = ftell (fp);
            oldLineNum = lineNum;
            linep = getNextLineD_ (fp, &lineNum, FALSE);
            if (linep == NULL)
            {
                if ((node = newNode ()) == NULL)
                {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", sizeof (struct confNode));
                    lserrno = LSE_MALLOC;
                    goto Error;
                }
                break;
            }

            memcpy(topass, &linep, strlen( linep ) );

            sp = linep;
            word = getNextWord_ (topass);

            if (word && word[0] == '#')
            {

                cp = word;
                cp++;

                if (   strcasecmp (cp, "define") == 0
                    || strcasecmp (cp, "if"    ) == 0
                    || strcasecmp (cp, "elif"  ) == 0
                    || strcasecmp (cp, "else"  ) == 0
                    || strcasecmp (cp, "endif" ) == 0
                    )
                {
                    fseek (fp, offset, SEEK_SET);
                    assert( oldLineNum <= UINT_MAX );
                    lineNum = oldLineNum;


                    if ((node = newNode ()) == NULL)
                    {
                        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", sizeof (struct confNode));
                        lserrno = LSE_MALLOC;
                        goto Error;
                    }
                    break;
                }
            }
        }

        node->beginLineNum = beginLineNum;
        node->numLines = numLines;
        node->lines = lines;
        if (prev != NULL) {
            linkNode (prev, node);
        }
        prev = node;

        if (rootNode == NULL) {
            rootNode = node;
        }
    }

    temp = popStack (blockStack);
    if (temp != NULL)
    {
        char message[ ] = "Missing endif";
        /* catgets 6006 */
        ls_syslog (LOG_ERR, "%s: %s(%d): %s.", __func__, filename, lineNum, _i18n_msg_get (ls_catd, NL_SETN, 6006, message) );
        goto Error;
    }

    for ( unsigned long i = 0; i < numDefs; i++)
    {
        FREEUP (defNames[i]);
        FREEUP (defConds[i]);
    }
    FREEUP (defNames);
    FREEUP (defConds);
    freeStack (blockStack);
    freeStack (ptrStack);
    conf->confhandle =  malloc (sizeof (struct confHandle));
    if (conf->confhandle == NULL)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", sizeof (struct confHandle));
        fclose (fp);
        lserrno = LSE_MALLOC;
        return NULL;
    }
    len = strlen (filename);
    conf->confhandle->rootNode = rootNode;
    conf->confhandle->fname = malloc ((len + 1) * sizeof (char));
    if (conf->confhandle->fname == NULL)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", (len + 1) * sizeof (char));
        fclose (fp);
        lserrno = LSE_MALLOC;
        return NULL;
    }
    strcpy (conf->confhandle->fname, filename);
    conf->confhandle->curNode = rootNode;
    conf->confhandle->lineCount = 0;
    conf->confhandle->ptrStack = initStack ();
    fclose (fp);
    return conf;

Error: // FIXME FIXME FIXME remove goto label

    for( unsigned long k = 0; k < numDefs; k++)
    {
        FREEUP (defNames[k]);
        FREEUP (defConds[k]);
    }
    FREEUP (defNames);
    FREEUP (defConds);
    freeStack (blockStack);
    freeStack (ptrStack);
    conf->confhandle = malloc( sizeof( struct confHandle ) );

    if (conf->confhandle == NULL)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, "malloc", sizeof( struct confHandle ) );
        if (fp) {
            fclose (fp);
        }
        lserrno = LSE_MALLOC;
        return NULL;
    }

    conf->confhandle->rootNode  = rootNode;
    conf->confhandle->fname     = NULL;
    conf->confhandle->curNode   = NULL;
    conf->confhandle->lineCount = 0;
    conf->confhandle->ptrStack  = NULL;
    ls_freeconf (conf);
    if (fp) {
        fclose (fp);
    }

    return NULL;
}

struct confNode *
newNode( void )
{
    struct confNode *node = NULL;

    node = malloc (sizeof (struct confNode));
    if (node == NULL) {
        return NULL;
    }

    node->leftPtr = NULL;
    node->rightPtr = NULL;
    node->fwPtr = NULL;
    node->cond = NULL;
    node->beginLineNum = 0;
    node->numLines = 0;
    node->lines = NULL;
    node->tag = 0;

    return node;
}

struct pStack *
initStack (void)
{
    struct pStack *stack = NULL;

    stack = malloc( sizeof( struct pStack ) );
    if (stack == NULL) {
        return NULL;
    }

    stack->top = -1;
    stack->nodes = malloc (5 * sizeof (struct confNode *));
    if (stack->nodes == NULL) {
        return NULL;
    }
    stack->size = 5;

    return stack;
}

int
pushStack (struct pStack *stack, struct confNode *node)
{
    struct confNode **sp = NULL;

    if (stack == NULL || node == NULL) {
        return -1;
    }

    assert( stack->size <= LONG_MAX );
    assert( stack->top >= 0 );
    if( stack->size == (size_t) stack->top + 1) { // FIXME FIXME FIXME this cast is probably right, but needs to be checked
        sp = myrealloc (stack->nodes, stack->size * 2 * sizeof (struct confNode *));
        if (sp == NULL) {
            ls_syslog( LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc" );
            return -1;
        }
        stack->size *= 2;
        stack->nodes = sp;
    }

    stack->nodes[++stack->top] = node;

    return 0;
}

struct confNode *popStack( struct pStack *stack )
{
    if (stack == NULL || stack->top < 0) {
        return NULL;
    }

    return stack->nodes[stack->top--];
}

void
freeStack (struct pStack *stack)
{
    if( stack != NULL ) {
        FREEUP( stack->nodes );
        FREEUP( stack );
    }
}

char
addCond( struct lsConf *conf, char *cond )
{
    size_t i = 0;
    char **newlist = NULL;
    int *values = NULL;

    if (conf == NULL || cond == NULL) {
        return FALSE;
    }

    for( i = 0; i < conf->numConds; i++ ) {
        if (strcmp (conf->conds[i], cond) == 0) {
            break;
        }
    }

    if( i < conf->numConds ) {
        return TRUE;
    }

    newlist = malloc( ( conf->numConds + 1 ) * sizeof( char  ) + 1 );
    if (newlist == NULL) {
        return FALSE;
    }
    values = malloc( ( conf->numConds + 1 ) * sizeof( int ) );

    if (values == NULL) {
        return FALSE;
    }
    for (i = 0; i < conf->numConds; i++) {
        newlist[i] = conf->conds[i];
        values[i] = conf->values[i];
    }

    newlist[conf->numConds] = putstr_ (cond);

    if (newlist[conf->numConds] == NULL) {
        return FALSE;
    }
    values[conf->numConds] = 0;
    FREEUP (conf->conds);
    FREEUP (conf->values);
    conf->conds = newlist;
    conf->values = values;
    conf->numConds++;

    return TRUE;
}

char
checkCond (struct lsConf *conf, char *cond)
{
    size_t i = 0;

    if (conf == NULL || cond == NULL) {
        return FALSE;
    }

    for (i = 0; i < conf->numConds; i++)
    {
        if (strcmp (conf->conds[i], cond) == 0) {
            break;
        }
    }

    if (i >= conf->numConds)  {
        return FALSE;
    }
    if (conf->values[i]) {
        return TRUE;
    }
    else {
        return FALSE;
    }

    return FALSE;
}

char
linkNode (struct confNode *prev, struct confNode *node)
{
    if (prev == NULL || node == NULL) {
        return FALSE;
    }

    if (prev->cond) {
        if (prev->tag == NODE_ALL_DONE) {

            if (prev->fwPtr == NULL) {
                prev->fwPtr = node;
            }
            else {
                return FALSE;
            }
        }
        else if (prev->tag == NODE_LEFT_DONE ){

            if (prev->rightPtr == NULL) {
                prev->rightPtr = node;
            }
            else if (prev->fwPtr == NULL) {
                prev->fwPtr = node;
            }
            else {
                return FALSE;
            }
        }
        else {
            if (prev->leftPtr == NULL) {
                prev->leftPtr = node;
            }
            else if (prev->rightPtr == NULL) {
                prev->rightPtr = node;
            }
            else if (prev->fwPtr == NULL) {
                prev->fwPtr = node;
            }
            else {
                return FALSE;
            }
        }
    }
    else {
        if (prev->fwPtr == NULL) {
            prev->fwPtr = node;
        }
        else {
            return FALSE;
        }
    }

    return TRUE;
}

void ls_freeconf (struct lsConf *conf)
{
    if (conf == NULL) {
        return;
    }
    freeNode (conf->confhandle->rootNode);
    FREEUP (conf->confhandle->fname);
    freeStack (conf->confhandle->ptrStack);
    FREEUP (conf->confhandle);
    for (size_t i = 0; i < conf->numConds; i++) {
        FREEUP (conf->conds[i]);
    }
    FREEUP (conf->conds);
    FREEUP (conf->values);
    FREEUP (conf);

    return;
}

void
freeNode (struct confNode *node)
{
    if (node == NULL) {
        return;
    }

    freeNode (node->leftPtr);
    freeNode (node->rightPtr);
    freeNode (node->fwPtr);
    FREEUP (node->cond);
    for( size_t i = 0; i < node->numLines; i++ ) {
        FREEUP (node->lines[i]);
    }
    FREEUP (node->lines);
    FREEUP (node);

    return;
}

// char *
// getNextLine_conf (struct lsConf *conf, int confFormat)
// {
//  size_t *dummy = 0;
//  return getNextLineC_conf (conf, dummy, confFormat);
// }

char *
readNextLine ( struct lsConf *conf, size_t *lineNum)
{
    struct confNode *node = NULL;
    struct confNode *prev = NULL;
    char *line = NULL;

    if( NULL == conf ) {
        return NULL;
    }

    node = conf->confhandle->curNode;
    if (node == NULL) {
        return NULL;
    }

    if (node->cond) {

        if (node->tag != NODE_PASED)
        {

            node->tag = NODE_PASED;
            pushStack (conf->confhandle->ptrStack, node);


            if (checkCond (conf, node->cond))
            {
                conf->confhandle->curNode = node->leftPtr;
                conf->confhandle->lineCount = 0;
            }
            else
            {
                conf->confhandle->curNode = node->rightPtr;
                conf->confhandle->lineCount = 0;
            }
            line = readNextLine (conf, lineNum);
            if (line)
                return line;
        }
        popStack (conf->confhandle->ptrStack);


        node->tag &= ~NODE_PASED;
        conf->confhandle->curNode = node->fwPtr;
        conf->confhandle->lineCount = 0;
        line = readNextLine (conf, lineNum);

        if (line) {
            return line;
        }
        else
        {
            prev = popStack (conf->confhandle->ptrStack);
            conf->confhandle->curNode = prev;
            conf->confhandle->lineCount = 0;
            pushStack (conf->confhandle->ptrStack, prev);
            return readNextLine (conf, lineNum);
        }
    }
    else {

        if (conf->confhandle->lineCount <= node->numLines - 1)
        {
            line = node->lines[conf->confhandle->lineCount];
            assert( node->beginLineNum + conf->confhandle->lineCount <= UINT_MAX);
            *lineNum = (node->beginLineNum + conf->confhandle->lineCount);
            conf->confhandle->lineCount++;
            return line;
        }
        else
        {
            conf->confhandle->curNode = node->fwPtr;
            conf->confhandle->lineCount = 0;
            line = readNextLine (conf, lineNum);
            if (line) {
                return line;
            }
            else
            {
                prev = popStack (conf->confhandle->ptrStack);
                conf->confhandle->curNode = prev;
                conf->confhandle->lineCount = 0;
                pushStack (conf->confhandle->ptrStack, prev);
                return readNextLine (conf, lineNum);
            }
        }
    }

    fprintf( stderr, "%s: you are not suppposed to be here!", __func__ );
    ls_syslog( LOG_ERR, "%s: you are not suppposed to be here!", __func__ );
    return NULL;
}
