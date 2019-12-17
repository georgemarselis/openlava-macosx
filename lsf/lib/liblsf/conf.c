/* $Id: lib.conf.c 397 2007-11-26 19:04:00Z mblack $
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
#include <stdbool.h>

#include "lsf.h"
#include "lib/conf.h"
#include "lib/confmisc.h"
// #include "lib/lproto.h"
#include "lib/words.h"
#include "lib/putInLists.h"
#include "lib/misc.h"
#include "lib/err.h"
#include "lib/id.h"
#include "lib/host.h"
#include "lib/do_hosts.h"
#include "libint/lsi18n.h"
#include "lib/getnextline.h"
#include "lib/syslog.h"
#include "lib/resnameDefined.h"

// #define NL_SETN 42

// const int NEGATIVERESULT = 1;
static const char N[] = "N";
static const char Y[] = "Y";
// static int lserrno;

int
initResTable_ (void)
{
    unsigned int i             = 0;
    struct resItem  **resTable = NULL;

    resTable = malloc (1000 * sizeof (struct resItem) + 1 * sizeof( struct resItem ) ); // FIXME FIXME FIXME FIXME '1000' is awfuly partifular
    if (!resTable) {
        ls_syslog (LOG_ERR, "%s: %s() failed", __func__, "malloc");
        return -1;
    }

    lsinfo.numIndx = 0;
    lsinfo.numUsrIndx = 0;
    while (builtInRes[i].name != NULL) {       // FIXME FIXME FIXME FIXME what the fuck is this builtInRes and where is it?
        char *number_buffer    = malloc( 10 ); // 10 bytes should be enough to sprintf 4 characters;
        char *buff             = NULL;
        int  length            = sprintf( number_buffer, "%d", builtInRes_ID[i] );
        size_t numlength       = 0;

        if( length < 0 ) {
            ls_syslog (LOG_ERR, "%s: %s", __func__, "length var is less than zero, aborting" );
            exit( -1 );
        }
        numlength       = (size_t) length; // FIXME (size_t) is fine here, but verify

        assert( numlength > 0 );
        buff = malloc(  (numlength + strlen( builtInRes[i].des ))*sizeof( char ) + 4 );
        sprintf( buff, "%d", builtInRes_ID[i] );

        sprintf( (buff + numlength ) , "%s", builtInRes[i].des  );
        *(buff +  numlength + strlen(builtInRes[i].des) + 1 )  = '\0';
        resTable[i]->des = buff;

        resTable[i]->name      = builtInRes[i].name;
        resTable[i]->valueType = builtInRes[i].valuetype;
        resTable[i]->orderType = builtInRes[i].ordertype;
        resTable[i]->interval  = builtInRes[i].interval;
        resTable[i]->flags     = builtInRes[i].flags;

        if (resTable[i]->flags & RESF_DYNAMIC) {
            lsinfo.numIndx++;
        }

        i++;

        free( buff );
    }
    lsinfo.nRes = i;
    lsinfo.resTable = resTable;

    return 0;
}

struct sharedConf *
ls_readshared ( const char *filename)
{
    FILE *fp       = NULL;
    // char *cp       = NULL;
    char *word     = NULL;
    char modelok   = 'a';
    char resok     = 'a';
    char clsok     = 'a';
    char typeok    = 'a';
    size_t lineNum = 0;

    const char unknown_section[ ]   = "unknown";   // FIXME FIXME FIXME FIXME FIXME section must be configurable from configure.ac
    const char host_section[ ]      = "host";      // FIXME FIXME FIXME FIXME FIXME section must be configurable from configure.ac
    const char hosttype_section[ ]  = "hosttype";  // FIXME FIXME FIXME FIXME FIXME section must be configurable from configure.ac
    const char hostmodel_section[ ] = "hostmodel"; // FIXME FIXME FIXME FIXME FIXME section must be configurable from configure.ac
    const char resource_section[ ]  = "resource";  // FIXME FIXME FIXME FIXME FIXME section must be configurable from configure.ac
    const char cluster_section[ ]   = "cluster";   // FIXME FIXME FIXME FIXME FIXME section must be configurable from configure.ac
    const char newindex_section[ ]  = "newindex";  // FIXME FIXME FIXME FIXME FIXME section must be configurable from configure.ac


    const char READONLY[] = "r";

    lserrno = LSE_NO_ERR;  // lserror is global
    if( filename == NULL) {
        /* catgets 5050 */
        ls_syslog (LOG_ERR, "catgets 5050: %s: filename is NULL", __func__);
        lserrno = LSE_NO_FILE;
        return NULL;
    }

    lsinfo.nRes = 0;
    FREEUP (lsinfo.resTable);
    lsinfo.nTypes = 0;
    lsinfo.nModels = 0;
    lsinfo.numIndx = 0;
    lsinfo.numUsrIndx = 0;

    if (sConf == NULL) {
        sConf = malloc (sizeof (struct sharedConf));
        if (NULL == sConf && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, "%s: %s() failed", __func__, "malloc");
            lserrno = LSE_MALLOC;
            return NULL;
        }
        sConf->lsinfo = &lsinfo;
        sConf->clusterName = NULL;
        sConf->servers = NULL;
    }
    else {
        FREEUP (sConf->clusterName);
        FREEUP (sConf->servers);
        sConf->lsinfo = &lsinfo;
    }

    modelok = FALSE;
    resok   = FALSE;
    clsok   = FALSE;
    typeok  = FALSE;

    if (initResTable_() < 0) {
        lserrno = LSE_MALLOC;
        return NULL;
    }
    fp = fopen ( filename, READONLY ); // FIXME FIXME FIXME FIXME FIXME 1. test of file exists before opening it 2. read whole file to buffer, do not read-and-parse
    if (fp == NULL) {
        /* catgets 5052 */
        ls_syslog (LOG_ERR, "catgets 5052: %s: Can't open configuration file <%s>.", __func__, filename);
        lserrno = LSE_NO_FILE;
        return NULL;
    }

    for (;;) {  // FIXME FIXME FIXME FIXME replace infinite loop with a ccertain-to-terminate condition
        const char *copyPtr = NULL;
        if ((copyPtr = getBeginLine (fp, &lineNum)) == NULL) {
            fclose (fp);
            if (!modelok)
            {
                /* catgets 5053 */
                ls_syslog (LOG_ERR, "catgets 5053: %s: HostModel section missing or invalid", filename);
            }
            if (!resok)
            {
                /* catgets 5054 */
                ls_syslog (LOG_ERR, "catgets 5054: %s: Resource section missing or invalid", filename);
            }
            if (!typeok)
            {
                /* catgets 5055 */
                ls_syslog (LOG_ERR, "catgets 5055: %s: HostType section missing or invalid", filename);
            }
            if (!clsok)
            {
                /* catgets 5056 */
                ls_syslog (LOG_ERR, "catgets 5056: %s: Cluster section missing or invalid", filename);
            }
            return sConf;
        }

        word = getNextWord_ ( &copyPtr );
        if (!word) {
            /* catgets 5057 */
            ls_syslog (LOG_ERR, "catgets 5057: %s: %s(%d): Section name expected after Begin; ignoring section", __func__, filename, lineNum);
            doSkipSection (fp, &lineNum, filename, unknown_section);
            continue;
        }
        else
        {
            if (strcasecmp (word, host_section ) == 0) 
            {
                /* catgets 5103 */
                ls_syslog (LOG_INFO, "catgets 5103: %s:%s: %s(%d): section %s no longer needed in this version, ignored", __func__, filename, lineNum, word);
                continue;
            }

            if (strcasecmp (word, hosttype_section ) == 0)
            {
                if (do_HostTypes (fp, &lineNum, filename)) {
                    typeok = TRUE;
                }
                continue;
            }

            if (strcasecmp (word, hostmodel_section ) == 0)
            {
                if (do_HostModels (fp, &lineNum, filename)) {
                    modelok = TRUE;
                }
                continue;
            }

            if (strcasecmp( word, resource_section ) == 0)
            {
                if (do_Resources (fp, &lineNum, filename)) {
                    resok = TRUE;
                }
                continue;
            }

            if (strcasecmp (word, cluster_section ) == 0)
            {
                if (do_Cluster (fp, &lineNum, filename)) {
                    clsok = TRUE;
                }
                continue;
            }

            if (strcasecmp (word, newindex_section ) == 0)
            {
                do_Index (fp, &lineNum, filename);
                continue;
            }

            /* catgets 5058 */
            ls_syslog (LOG_ERR, "catgets 5058: %s: %s(%d): Invalid section name %s; ignoring section", __func__, filename, lineNum, word);
            doSkipSection (fp, &lineNum, filename, word);
        }
    }

    return NULL;
}

char do_Index (FILE * fp, size_t *lineNum, const char *filename)
{
    char *linep     = NULL;

    enum {
        INTERVAL,
        INCREASING,
        DESCRIPTION,
        NAME,
        KEYNULL
    };

    const char *keyValues[ ] = { 
        "INTERVAL",
        "INCREASING",
        "DESCRIPTION",
        "NAME",
        NULL
    };

    struct keymap keyList[ ] = {
        { INTERVAL,    keyValues[INTERVAL],    NULL },
        { INCREASING,  keyValues[INCREASING],  NULL },
        { DESCRIPTION, keyValues[DESCRIPTION], NULL },
        { NAME,        keyValues[NAME],        NULL },
        { KEYNULL,     NULL, NULL }
    };

    const char newindex[ ] = "newindex";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
    {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, newindex); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, newindex))
    {
        /* catgets 5061 */
        ls_syslog (LOG_WARNING, "catgets 5061: %s: %s(%d): empty section",  __func__, filename, *lineNum);
        return FALSE;
    }

    if (strchr (linep, '=') == NULL)
    {
        if (!keyMatch (keyList, linep, TRUE))
        {
            /* catgets 5062 */
            ls_syslog (LOG_ERR, "catgets 5062: %s: %s(%d): keyword line format error for section newindex; ignoring section", __func__, filename, *lineNum);
            doSkipSection (fp, lineNum, filename, newindex);
            return FALSE;
        }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
        {
            if (isSectionEnd (linep, filename, lineNum, newindex )) {
                return TRUE;
            }
            if (mapValues (keyList, linep) < 0)
            {
                /* catgets 5063 */
                ls_syslog (LOG_ERR, "catgets 5063: %s: %s(%d): values do not match keys for section newindex; ignoring line", __func__, filename, *lineNum);
                continue;
            }

            setIndex (keyList, filename, *lineNum);
        }
    }
    else {
        if (readHvalues (keyList, linep, fp, filename, lineNum, TRUE, newindex) < 0) {
            return FALSE;
        }
        setIndex (keyList, filename, *lineNum);
        return TRUE;
    }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, newindex); /*catgets33 */
    return TRUE;
}

bool lsf_setIndex (struct keymap *keyList, const char *filename, size_t lineNum)
{
    int BASE10          = 10;
    char **endptr       = NULL;
    unsigned int resIdx = 0;
    unsigned long value = 0;

    const int NEGATIVERESULT = 1;

    enum {
        INTERVAL,
        INCREASING,
        DESCRIPTION,
        NAME,
        KEYNULL
    };

    if (keyList == NULL) {
        return false;
    }

    if (strlen (keyList[NAME].value) >= MAX_LSF_NAME_LEN) {
        /* catgets 5065 */
        ls_syslog (LOG_ERR, "catgets 5065: %s: %s(%d): Name %s is too long (maximum is %d chars); ignoring index", __func__, filename, lineNum, keyList[NAME].value, MAX_LSF_NAME_LEN - 1);
        return false;
    }

    if (strpbrk (keyList[NAME].value, ILLEGAL_CHARS) != NULL) {
        /* catgets 5066 */
        ls_syslog (LOG_ERR, "catgets 5066 %s: %s(%d): illegal character (one of %s), ignoring index %s", __func__, filename, lineNum, ILLEGAL_CHARS, keyList[NAME].value); // keyList[3]
        return false;
    }

    // assert( resNameDefined (keyList[3].value) > 0 ); // FIXME FIXME FIXME 3? change that to a enum
    // resIdx = (unsigned int) resNameDefined (keyList[3].value); // FIXME FIXME FIXME check if there is a reason for the cast here
    errno = 0;
    resIdx = resNameDefined (keyList[NAME].value); // FIXME FIXME FIXME check if there is a reason for the cast here
    if( NEGATIVERESULT == errno ) {
        ls_syslog( LOG_ERR, "nocatgets: %s: resNameDefined() returned negative result", __func__ );
        // errno = 0;
        return false;
    } 

    if (!(lsinfo.resTable[resIdx]->flags & RESF_DYNAMIC)) {
        /* catgets 5067 */
        ls_syslog (LOG_ERR, "catgets 5067: %s: %s at line %lu: Name %s is not a dynamic resource; ignored", __func__, filename, lineNum, keyList[NAME].value); // keyList[NAME]
        return false;
    }
    else {
        resIdx = lsinfo.nRes;
    }

    value = strtoul( keyList[INTERVAL].value, endptr, BASE10 );
    if ( value == ULONG_MAX && errno ==  ERANGE ) {
        /* catgets 5067 */ // FIXME FIXME FIXME FIXME get new catgets number
        ls_syslog (LOG_ERR, "catgets 5067: %s at line %lu: value of %s for keyList[INTERVAL] is overflowed.", __func__, filename, lineNum, keyList[INTERVAL].key );
        return false;
    }

    lsinfo.resTable[resIdx]->interval = (unsigned int) value;
    lsinfo.resTable[resIdx]->orderType =(strcasecmp (keyList[INCREASING].value, "y") == 0) ? INCR : DECR; // FIXME FIXME FIXME change 1 to appropriate label in enum

    lsinfo.resTable[resIdx]->des       = keyList[DESCRIPTION].value;
    lsinfo.resTable[resIdx]->name      = keyList[NAME].value;
    lsinfo.resTable[resIdx]->valueType = LS_NUMERIC;
    lsinfo.resTable[resIdx]->flags     = RESF_DYNAMIC | RESF_GLOBAL;

    if (resIdx == lsinfo.nRes) {
        lsinfo.numUsrIndx++;
        lsinfo.numIndx++;
        lsinfo.nRes++;
    }

    FREEUP (keyList[INTERVAL].value);
    FREEUP (keyList[INCREASING].value);
    FREEUP (keyList[DESCRIPTION].value);
    FREEUP (keyList[NAME].value);

    return true;
}

char do_HostTypes (FILE * fp, size_t *lineNum, const char *filename)
{
    char *linep = NULL;

    enum {
        TYPENAME = 0,
        KEYNULL = 255
    };

    const char *keylist[] = {
        "TYPENAME",
        NULL
    };

    struct keymap keyList[] = {
        { TYPENAME, keylist[TYPENAME], NULL },
        { KEYNULL,  NULL, NULL }
    };

    const char hosttype[] = "HostType";


    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
    {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, hosttype); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, hosttype)) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL)
    {
        if (!keyMatch (keyList, linep, TRUE))
        {
            /* catgets 5070 */
            ls_syslog (LOG_ERR, "catgets 5070: %s: %s(%d): keyword line format error for section HostType, ignoring section", __func__, filename, *lineNum);
            doSkipSection (fp, lineNum, filename, hosttype);
            return FALSE;
        }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
        {
            if (isSectionEnd (linep, filename, lineNum, hosttype)) {
                return TRUE;
            }
            if (mapValues (keyList, linep) < 0)
            {
                /* catgets 5071 */
                ls_syslog (LOG_ERR, "catgets 5071: %s: %s(%d): values do not match keys for section cluster, ignoring line", __func__, filename, *lineNum);
                continue;
            }

            if (strpbrk (keyList[TYPENAME].value, ILLEGAL_CHARS) != NULL ) 
            {
                /* catgets 5072 */
                ls_syslog (LOG_ERR, "catgets 5072: %s: %s(%d): illegal character (one of %s), ignoring type %s", __func__, filename, *lineNum, ILLEGAL_CHARS, keyList[0].value); // FIXME FIXME FIME replace [0] with enum LABEL
                FREEUP (keyList[TYPENAME].value);
                continue;
            }

            addHostType (keyList[TYPENAME].value); // FIXME FIXME FIXME replace [0] wtih enum label
            FREEUP (keyList[TYPENAME].value);
        }
    }
    else
    {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, hosttype);
        doSkipSection (fp, lineNum, filename, hosttype);
        return FALSE;
    }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, hosttype); /*catgets33 */

    return TRUE;
}

char
ls_addHostType (char *type) // duplicate function name from limd: lim_addHostType
{

    if (type == NULL) {
        return FALSE;
    }

    if (lsinfo.nTypes == MAX_TYPES)
    {
        /* catgets 5075 */
        ls_syslog (LOG_ERR, "catgets 5075: %s: Too many host types defined in section HostType. You can only define up to %d host types; host type %s ignored", __func__, MAX_TYPES, type);
        return FALSE;
    }

    for (unsigned int i = 0; i < lsinfo.nTypes; i++) {
        if (strcmp (lsinfo.hostTypes[i], type) != 0) {
            continue;
        }
        /* catgets 5076 */
        ls_syslog (LOG_ERR, "catgets 5076: %s: host type %s multiply defined", __func__, type);
        return FALSE;
    }

    strcpy (lsinfo.hostTypes[lsinfo.nTypes], type);
    lsinfo.nTypes++;

    return TRUE;
}

char do_HostModels (FILE * fp, size_t *lineNum, const char *filename)
{
    char *sp    = NULL;
    char *word  = NULL;
    char *linep = NULL;

    enum {
        MODELNAME,
        CPUFACTOR,
        ARCHITECTURE,
        KEYNULL
    };

    const char *keyvalues[ ] = {
        "MODELNAME",
        "CPUFACTOR",
        "ARCHITECTURE",
        NULL
    };

    struct keymap keyList[]  = {
        { MODELNAME,    keyvalues[MODELNAME],    NULL },
        { CPUFACTOR,    keyvalues[CPUFACTOR],    NULL },
        { ARCHITECTURE, keyvalues[ARCHITECTURE], NULL },
        { KEYNULL,      NULL, NULL }
    };

    const char hostModel[] = "HostModel";


    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, hostModel); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, hostModel)) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL) {
        if (!keyMatch (keyList, linep, FALSE)) {
            /* catgets 5078 */
            ls_syslog (LOG_ERR, "catgets 5078: %s: %s(%d): keyword line format error for section hostmodel, ignoring section", __func__, filename, *lineNum);
            doSkipSection (fp, lineNum, filename, filename);
            return FALSE;
        }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {

            if (isSectionEnd (linep, filename, lineNum, hostModel)) {
                return TRUE;
            }

            if (mapValues (keyList, linep) < 0) {
                /* catgets 5079 */
                ls_syslog (LOG_ERR, "catgets 5079: %s: %s(%d): values do not match keys for section hostmodel, ignoring line", __func__, filename, *lineNum);
                continue;
            }

            if (!isanumber_ (keyList[CPUFACTOR].value) || atof (keyList[CPUFACTOR].value) <= 0) {
                /* catgets 5080 */
                ls_syslog (LOG_ERR, "catgets 5080: %s: %s(%d): Bad cpuFactor for host model %s, ignoring line", __func__, filename, *lineNum, keyList[MODELNAME].value);
                FREEUP (keyList[MODELNAME].value);
                FREEUP (keyList[CPUFACTOR].value);
                FREEUP (keyList[ARCHITECTURE].value);
                continue;
            }

            if (strpbrk (keyList[MODELNAME].value, ILLEGAL_CHARS) != NULL) {
                /* catgets 5081 */
                ls_syslog (LOG_ERR, "catgets 5081: %s: %s(%d): illegal character (one of %s), ignoring model %s", __func__, filename, *lineNum, ILLEGAL_CHARS, keyList[MODELNAME].value);
                FREEUP (keyList[MODELNAME].value);
                FREEUP (keyList[CPUFACTOR].value);
                FREEUP (keyList[ARCHITECTURE].value);
                continue;
            }

            sp = keyList[ARCHITECTURE].value;
            if (sp && sp[0]) {
                const char *pointer = sp;
                while ((word = getNextWord_ (&pointer)) != NULL) {
                    addHostModel (keyList[MODELNAME].value, word, atof (keyList[CPUFACTOR].value)); // FIXME FIXME FIXME replace 1 with uname label
                }
            }
            else {
                addHostModel (keyList[MODELNAME].value, NULL, atof (keyList[CPUFACTOR].value)); // FIXME FIXME FIXME replace 1 with uname label
                FREEUP (keyList[MODELNAME].value);
                FREEUP (keyList[CPUFACTOR].value);
                FREEUP (keyList[ARCHITECTURE].value);
            }
        }
    }
    else {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, hostModel);
        doSkipSection (fp, lineNum, filename, hostModel);
        return FALSE;
    }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, hostModel); /*catgets33 */
    return TRUE;
}

char ls_addHostModel ( const char *model, const char *arch, double factor) // FIXME FIXME FIXME FIXME duplicate function name from lim: lim_addHostModel
{
    if ( NULL == model || NULL == arch) {
        ls_syslog (LOG_ERR, "%s: model (%s) or arch (%s) is NULL", __func__, model, arch );
        return FALSE;
    }

    if (lsinfo.nModels == MAX_MODELS) {
        /* catgets 5084 */
        ls_syslog (LOG_ERR, "catgets 5084: %s: Too many host models defined in section HostModel. You can only define up to %d host models; host model %s ignored", __func__, MAX_MODELS, model);
        return FALSE;
    }

    for (unsigned int i = 0; i < lsinfo.nModels; ++i) {
        if (arch == 0 || strcmp (lsinfo.hostArchs[i], arch) != 0) {
            continue;
        }
        /* FIXME FIXME FIXME two lines below:
         left over from previously dangling if above. might need to be moved inside the if brackets */
        /* catgets 5085 */
        ls_syslog (LOG_ERR, "catgets 5085: %s:%s: Duplicate host architecture type found in section HostModel. Architecture type must be unique; host model %s ignored", __func__, model);

        return FALSE;
    }

    strcpy (lsinfo.hostModels[lsinfo.nModels], model );
    strcpy (lsinfo.hostArchs[lsinfo.nModels], arch ? arch : NULL);
    lsinfo.cpuFactor[lsinfo.nModels] = factor;
    lsinfo.nModels++;

    return TRUE;
}

unsigned int parseHostList (const char *hostList, const char *lsfile, const size_t lineNum, char ***hosts)
{
    char *host       = NULL;
    char **hostTable = NULL;
    unsigned int  numHosts = 0;

    assert( lsfile );  // FIXME all three assertions got to go
    assert( lineNum ); // investigate if a file and a line number
                       // are indeed passed over

    if ( NULL == hostList ) {
        lsferrno = 9000; // FIXME FIXME FIXME return proper labeled error
        return 0;
    }

    while ( NULL != (host = getNextWord_ (&hostList)) ) {
        numHosts++;
    }

    assert( numHosts );
    hostTable = malloc( numHosts * sizeof (char *));
    if( NULL == hostTable && ENOMEM == errno) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        lserrno = 9001; // FIXME FIXME FIXME return proper labeled error
        return 0;
    }

    numHosts = 0;
    while ((host = getNextWord_ (&hostList)) != NULL) {

        if ( NULL == (hostTable[numHosts] = putstr_ (host)) ) {

            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            for ( unsigned int i = 0; i < numHosts; i++) {
                FREEUP (hostTable[i]);
            }

            FREEUP (hostTable);

            lserrno = 9002; // FIXME FIXME FIXME return proper labeled error
            return 0;
        }
        numHosts++;
    }
    if ( 0 == numHosts ) {
        FREEUP (hostTable);
        lserrno = 9003; // FIXME FIXME FIXME return proper labeled error
        return 0; 
    }

    *hosts = hostTable;

    return numHosts;
}

// #define RESOURCENAME 0
// #define TYPE         1
// #define INTERVAL     2
// #define INCREASING   3
// #define RELEASE      4
// #define DESCRIPTION  5

/* FIXME FIXME
 This function will need
 1. test cases to make sure it does what it is supposed to do
 2. testing to see it does what it is supposed to do
 3. debugging and profing to make sure nothing spills out of bounds or rolls over

 */
char do_Resources (FILE * fp, size_t *lineNum, const char *filename)
{
    int nres = 0;
    char *linep = NULL;

    enum {
        RESOURCENAME = 0,
        TYPE,
        INTERVAL,
        INCREASING,
        RELEASE,
        DESCRIPTION,
        KEYNULL
    };

    const char *keylist[] = {
        "RESOURCENAME",
        "TYPE",
        "INTERVAL",
        "INCREASING",
        "RELEASE",
        "DESCRIPTION",
        NULL
    };

    struct keymap keyList[] = {
        { RESOURCENAME, keylist[RESOURCENAME], NULL },
        { TYPE,         keylist[TYPE],         NULL },
        { INTERVAL,     keylist[INTERVAL],     NULL },
        { INCREASING,   keylist[INCREASING],   NULL },
        { RELEASE,      keylist[RELEASE],      NULL },
        { DESCRIPTION,  keylist[DESCRIPTION],  NULL },
        { KEYNULL,      NULL, NULL }
    };

    const char resource[] = "resource";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, resource); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, resource)) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL) {
        if (!keyMatch (keyList, linep, FALSE)) {
            /* catgets 5086 */
            ls_syslog (LOG_ERR, "catgets 5086: %s: %s(%d): keyword line format error for section resource, ignoring section", __func__, filename, *lineNum);
            ls_syslog (LOG_ERR, "%s: %s", __func__, linep);
            doSkipSection (fp, lineNum, filename, resource);
            return FALSE;
        }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {
            if (isSectionEnd (linep, filename, lineNum, resource )) {
                return TRUE;
            }

            if (mapValues (keyList, linep) < 0) {
                /* catgets 5087 */
                ls_syslog (LOG_ERR, "catgets 5087: %s: %s(%d): values do not match keys for section resource, ignoring line", __func__, filename, *lineNum);
                continue;
            }

            if (strlen (keyList[RESOURCENAME].value) >= MAX_LSF_NAME_LEN - 1) {
                /* catgets 5088 */
                ls_syslog (LOG_ERR, "catgets 5088: %s: %s(%d): Resource name %s too long in section resource. Should be less than %d characters,  ignoring line", __func__, filename, *lineNum, keyList[RESOURCENAME].value, MAX_LSF_NAME_LEN - 1);
                freeKeyList (keyList);
                continue;
            }

            lserrno = ENOERROR; // FIXME FIXME FIXME FIXME save old value, restore after if condition
            resNameDefined (keyList[RESOURCENAME].value);
            if ( ENEGATIVERESULT == lserrno) {
                lserrno = ENOERROR;
                /* catgets 5089 */
                ls_syslog (LOG_ERR, "catgets 5089: %s: %s(%d): Resource name %s reserved or previously defined. Ignoring line", __func__, filename, *lineNum, keyList[RESOURCENAME].value);
                freeKeyList (keyList);
                continue;
            }

            if (strpbrk (keyList[RESOURCENAME].value, ILLEGAL_CHARS) != NULL) {
                /* catgets 5090 */
                ls_syslog (LOG_ERR, "catgets 5090: %s: %s(%d): illegal character (one of %s): in resource name:%s, section resource, ignoring line", __func__, filename, *lineNum, ILLEGAL_CHARS, keyList[RESOURCENAME].value);
                freeKeyList (keyList);
                continue;
            }

            if (isdigit (keyList[RESOURCENAME].value[0])) {
                /* catgets 5091 */
                ls_syslog (LOG_ERR, "catgets 5091: %s: %s(%d): Resource name <%s> begun with a digit is illegal; ignored", __func__, filename, *lineNum, keyList[RESOURCENAME].value);
                freeKeyList (keyList);
                continue;
            }

            // memset( (char ) lsinfo.resTable[lsinfo.nRes]->name, '\0', strlen( lsinfo.resTable[lsinfo.nRes]->name ) );
            // memset( (char ) lsinfo.resTable[lsinfo.nRes]->des,  '\0', strlen( lsinfo.resTable[lsinfo.nRes]->des  ) );
            lsinfo.resTable[lsinfo.nRes]->flags = RESF_GLOBAL;
            lsinfo.resTable[lsinfo.nRes]->valueType = LS_BOOLEAN;
            lsinfo.resTable[lsinfo.nRes]->orderType = NA;
            lsinfo.resTable[lsinfo.nRes]->interval = 0;

            // FIXME FIXME FIXME FIXME FIXME must institute a global check to see if a value is in valid range
            lsinfo.resTable[lsinfo.nRes]->name = keyList[RESOURCENAME].value;


            if (keyList[TYPE].value != NULL && keyList[TYPE].value[RESOURCENAME] != '\0') {
                int type = 0;

                /* FIXME
                 following line needs to go; be assigned to declaration of type and
                 use the assert() to check if it is greater or equal to zero
                 */
                if ((type = validType (keyList[TYPE].value)) >= 0) {
                    assert( type >= 0 );
                    lsinfo.resTable[lsinfo.nRes]->valueType = (enum valueType) type; // FIXME FIXME FIXME FIXME cast is correct
                }
                else {
                    /* catgets 5092 */
                    ls_syslog (LOG_ERR, "catgets 5092: %s: %s(%d): resource type <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource", __func__, filename, *lineNum, keyList[TYPE].value, keyList[RESOURCENAME].value, keyList[RESOURCENAME].value);
                    freeKeyList (keyList);
                    continue;
                }
            }

            if ( keyList[INTERVAL].value != NULL ) {

                unsigned int interval = 0 ;

                if ((interval = (unsigned int)atoi( keyList[INTERVAL].value ) ) > 0) { // FIXME FIXME FIXME FIXME add check for legit range of value // cast is OK here

                    lsinfo.resTable[lsinfo.nRes]->interval = interval;
                    if (lsinfo.resTable[lsinfo.nRes]->valueType == LS_NUMERIC) {
                        lsinfo.resTable[lsinfo.nRes]->flags |= RESF_DYNAMIC;
                    }
                    else {
                        /* catgets 5093 */
                        ls_syslog (LOG_ERR, "catgets 5093: %s: %s(%d): INTERVAL <%s> for resource <%s> should be a integer greater than 0; ignoring resource <%s> in section resource", __func__, filename, *lineNum, keyList[INTERVAL].value, keyList[RESOURCENAME].value, keyList[RESOURCENAME].value);
                        freeKeyList (keyList);
                        continue;
                    }
                }
            }

            if (keyList[INCREASING].value != NULL && keyList[INCREASING].value[0] != '\0') {

                if (lsinfo.resTable[lsinfo.nRes]->valueType == LS_NUMERIC) {
                    if (!strcasecmp (keyList[INCREASING].value, N)) { // FIXME FIXME FIXME FIXME change N to variable
                        lsinfo.resTable[lsinfo.nRes]->orderType = DECR;

                    }
                    else if (!strcasecmp (keyList[INCREASING].value, Y)) {
                        lsinfo.resTable[lsinfo.nRes]->orderType = INCR;
                    }
                    else {
                        /* catgets 5094 */
                        ls_syslog (LOG_ERR, "catgets 5094: %s: %s(%d): INCREASING <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource", __func__, filename, *lineNum, keyList[INCREASING].value, keyList[RESOURCENAME].value, keyList[RESOURCENAME].value);
                        freeKeyList (keyList);
                        continue;
                    }
                }
                else {
                    char *syslog_result = NULL;
                    unsigned int valueTypeResult = LS_BOOLEAN;
                    unsigned int orderTypeResult = lsinfo.resTable[lsinfo.nRes]->orderType;
                    syslog_result = malloc( sizeof( char ) * 10 + 1);
                    if( valueTypeResult == orderTypeResult )  {
                        strcpy( syslog_result, "BOOLEAN" );
                    }
                    else {
                        strcpy( syslog_result, "STRING" );
                    };

                    /* catgets 5095 */
                    ls_syslog (LOG_ERR, "%s: %s(%d): INCREASING <%s> is not used by the resource <%s> with type <%s>; ignoring INCREASING", __func__, filename, *lineNum, keyList[INCREASING].value, keyList[RESOURCENAME].value, syslog_result );
                    free( syslog_result );
                }
            }
            else if (lsinfo.resTable[lsinfo.nRes]->valueType == LS_NUMERIC) {
                /* catgets 5096 */
                ls_syslog (LOG_ERR, "catgets 5096: %s: %s(%d): No INCREASING specified for a numeric resource <%s>; ignoring resource <%s> in section resource", __func__, filename, *lineNum, keyList[RESOURCENAME].value, keyList[RESOURCENAME].value);
                freeKeyList (keyList);
                continue;
            }
            else {
                fprintf(stderr, "%s: man this is some fucked up code...\n", __func__ );
                exit( 255 );
            }

            /* FIXME FIXME possible dangling if issue:
             if bellow, might be part of the else block above

             this shit will need the debugger to make sense
             (or a boatload of test cases)
             */
            if (keyList[RELEASE].value != NULL && keyList[RELEASE].value[0] != '\0') {

                if (lsinfo.resTable[lsinfo.nRes]->valueType == LS_NUMERIC) {

                    if (!strcasecmp (keyList[RELEASE].value, Y)) {
                        lsinfo.resTable[lsinfo.nRes]->flags |= RESF_RELEASE;
                    }
                    else if (strcasecmp (keyList[RELEASE].value, N)) {
                        /*catgets 5212 */
                        ls_syslog (LOG_ERR, "catgets 5212: %s:%s:%s(%d): RELEASE defined for resource <%s> should be 'Y', 'y', 'N' or 'n' not <%s>; ignoring resource <%s> in section resource", __func__, filename, *lineNum, keyList[RESOURCENAME].value, keyList[RELEASE].value, keyList[RESOURCENAME].value);
                        freeKeyList (keyList);
                        continue;
                    }
                    else {
                        printf( "WTF am i doing here? dangling else problem or parameters out of wack. Investige in conf.c\n");
                    }
                }
                else {
                    /*catgets 5213 */
                    ls_syslog (LOG_ERR, "catgets 5213: %s:%s:%s(%d): RELEASE cannot be defined for resource <%s> which isn't a numeric resource; ignoring resource <%s> in section resource", __func__, filename, *lineNum, keyList[RESOURCENAME].value, keyList[RESOURCENAME].value);
                    freeKeyList (keyList);
                    continue;
                }
            }
            else {
                if (lsinfo.resTable[lsinfo.nRes]->valueType == LS_NUMERIC) {
                    lsinfo.resTable[lsinfo.nRes]->flags |= RESF_RELEASE;
                }
            }

        } /* end while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) */

        }

    /* FIXME FIXME possible dangling if issue:
     if bellow, might be part of the else block above
     */

        lsinfo.resTable[lsinfo.nRes]->des = keyList[DESCRIPTION].value;
        if (lsinfo.resTable[lsinfo.nRes]->interval > 0 && (lsinfo.resTable[lsinfo.nRes]->valueType == LS_NUMERIC)) {
            lsinfo.numUsrIndx++;
            lsinfo.numIndx++;
            lsinfo.nRes++;
            nres++;
            freeKeyList (keyList);
        }
        else {
            ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "resource");
            return FALSE;
        }

        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, "resource"); /*catgets33 */
        return TRUE;
    }

struct clusterConf *ls_readcluster ( const char *filename, struct lsInfo *info, int lookupAdmins)
{
    FILE *fp    = NULL;
    char *word  = NULL;
    int Error   = FALSE;
    int aorm    = FALSE;
    int count1  = 0;
    int count2  = 0;
    int counter = 0;
    size_t lineNum = 0;
    struct lsInfo myinfo;

    const char readonly[] = "r";

    lserrno = LSE_NO_ERR;
    if ( filename  == NULL) {
    /* catgets 5050 */
        ls_syslog (LOG_ERR, "catgets 5050: %s: filename is NULL", __func__ );
        lserrno = LSE_NO_FILE;
        return NULL;
    }

    if (info == NULL) {
    /* catgets 5100 */
        ls_syslog (LOG_ERR, "catgets 5100: %s: LSF information is NULL", __func__ );
        lserrno = LSE_NO_FILE;
        return NULL;
    }


    if (cConf == NULL) {
        cConf = malloc (sizeof (struct clusterConf));
        if ( NULL == cConf && ENOMEM == errno )
        {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            lserrno = LSE_MALLOC;
            return NULL;
        }
        cConf->clinfo = NULL;
        cConf->hosts = NULL;
        cConf->numHosts = 0;
        cConf->numShareRes = 0;
        cConf->shareRes = NULL;
    }
    else
    {
        for ( unsigned int i = 0; i < cConf->numHosts; i++)  {
            freeHostInfo (&cConf->hosts[i]);
        }

        FREEUP (cConf->hosts);
        cConf->numHosts = 0;
        for ( unsigned int i = 0; i < cConf->numShareRes; i++) // FIXME FIXME FIXME can cConf->numShareRes be unsigned int?
        {
            FREEUP (cConf->shareRes[i].resourceName);
            for ( unsigned int j = 0; j < cConf->shareRes[i].nInstances; j++)
            {
                FREEUP (cConf->shareRes[i].instances[j].value);
                for ( unsigned int k = 0; k < cConf->shareRes[i].instances[j].nHosts; k++)
                {
                    FREEUP (cConf->shareRes[i].instances[j].hostList[k]);
                }
                FREEUP (cConf->shareRes[i].instances[j].hostList);
            }
            FREEUP (cConf->shareRes[i].instances);
        }

        FREEUP (cConf->shareRes);
        cConf->shareRes = NULL;
        cConf->numShareRes = 0;
    }

    freeClusterInfo (&clinfo);
    initClusterInfo (&clinfo);
    cConf->clinfo = &clinfo;
    count1 = 0;
    count2 = 0;

    myinfo = *info;
    assert( info->nRes > 0 );
    myinfo.resTable = malloc( info->nRes * sizeof (struct resItem));

    if (info->nRes && (myinfo.resTable == NULL) ) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        lserrno = LSE_MALLOC;
        return NULL;
    }

    counter = 0;
    for ( unsigned int i = 0; i < info->nRes; i++)
    {
        if (info->resTable[i]->flags & RESF_DYNAMIC) {
            memcpy (&myinfo.resTable[counter], &info->resTable[i], sizeof (struct resItem));
            counter++;
        }
    }

    for ( unsigned int i = 0; i < info->nRes; i++)
    {
        if (!(info->resTable[i]->flags & RESF_DYNAMIC)) {
            memcpy (&myinfo.resTable[counter], &info->resTable[i], sizeof (struct resItem));
            counter++;
        }
    }

    if ((fp = fopen (filename, readonly)) == NULL) {

        const char fopen[] = "fopen";
        FREEUP (myinfo.resTable);
        ls_syslog (LOG_INFO, I18N_FUNC_S_FAIL, filename, fopen, __func__);
        lserrno = LSE_NO_FILE;
        return NULL;
    }

    for (;;) {  // FIXME FIXME FIXME FIXME replace infinite loop with a ccertain-to-terminate condition

        const char clustermanager[] = "clustermanager";
        const char clusteradmins[]  = "clusteradmins";
        const char parameters[]     = "parameters";
        const char host[]           = "host";
        const char resourceMap[]    = "resourceMap";

        const char *cp = getBeginLine (fp, &lineNum);

        if (!cp) {

            fclose (fp);
            if (cConf->numHosts) {

                FREEUP (myinfo.resTable);
                if (Error) {
                    return NULL;
                }
                else {
                    return cConf;
                }
            }
        }
        else {

            FREEUP (myinfo.resTable);
            /* catgets 5104 */
            ls_syslog (LOG_ERR, "catgets 5104: %s: %s(%d): No hosts configured.", __func__, filename, lineNum);
            return cConf;
        }

        word = getNextWord_ (&cp);
        if (!word)
        {
            const char unknown[] = "unknown";
            /* catgets 5105 */
            ls_syslog (LOG_ERR, "catgets 5105: %s: %s(%d): Keyword expected after Begin. Ignoring section", __func__, filename, lineNum);
            doSkipSection (fp, &lineNum, filename, unknown);
        }
        else if (strcasecmp (word, clustermanager) == 0)
        {
            count1++;
            if (count1 > 1)
            {
                /* catgets 5106 */
                ls_syslog (LOG_ERR, "catgets 5106: %s: %s(%d): More than one %s section defined; ignored.",  __func__, filename, lineNum, word);
                doSkipSection (fp, &lineNum, filename, word);
            }
            else
            {
                if (!do_Manager(fp, filename, &lineNum, clustermanager, lookupAdmins) && aorm != TRUE)
                {
                    Error = TRUE;
                }
                else{
                    aorm = TRUE;
                }
            }
            continue;
        }
        else if (strcasecmp (word, clusteradmins) == 0)
        {
            count2++;
            if (count2 > 1)
            {
                /* catgets 5107 */
                ls_syslog (LOG_ERR, "catgets 5107: %s: %s(%d): More than one %s section defined; ignored.", __func__, filename, lineNum, word);
                doSkipSection (fp, &lineNum, filename, word);
            }
            else
            {
                if (!do_Manager(fp, filename, &lineNum, clusteradmins, lookupAdmins) && aorm != TRUE)
                {
                    Error = TRUE;
                }
                else {
                    aorm = TRUE;
                }
            }
            continue;
        }
        else if (strcasecmp (word, parameters) == 0)
        {
            if (!do_Clparams (fp, filename, &lineNum)) {
                Error = TRUE;
            }
            continue;
        }
        else if (strcasecmp (word, host) == 0)
        {
            if (!do_Hosts_lsf (fp, filename, &lineNum, &myinfo)) {
                Error = TRUE;
            }
            continue;
        }
        else if (strcasecmp (word, resourceMap ) == 0)
        {
            if (doResourceMap (fp, filename, &lineNum) < 0) {
                Error = TRUE;
            }
            continue;
        }
        else
        {
            /* catgets 5108 */
            ls_syslog (LOG_ERR, "catgets 5108: %s %s(%d): Invalid section name %s, ignoring section", __func__, filename, lineNum, word );
            doSkipSection (fp, &lineNum, filename, word);
        }
    }

    return NULL;
}


// struct clusterConf *ls_readcluster ( const char *filename, struct lsInfo *info)  // FIXME FIXME FIXME FIXME move to cluster.c file
// {
//  return ls_readcluster_ex (filename, info, TRUE);
// }


void freeClusterInfo (struct clusterInfo *cls) // FIXME FIXME move to cluster.c file
{

    if (cls != NULL) {
        for ( unsigned int i = 0; i < cls->nRes; i++) {
            free( cls->resources );
            cls->resources++;
        }
        for ( unsigned int i = 0; i < cls->nTypes; i++) {
            free( cls->hostTypes );
            cls->hostTypes++;
        }
        for ( unsigned int i = 0; i < cls->nModels; i++) {
            free( cls->hostModels );
            cls->hostModels++;
        }
        for ( unsigned int i = 0; i < cls->nAdmins; i++) {
            free( cls->admins );
            cls->admins++;
        }

        free( cls->admins );
        free( cls->adminIds );
    }

    return;
}


void initClusterInfo (struct clusterInfo *cls)  // FIXME FIXME FIXME FIXME move to cluster.c file
{
    if (cls != NULL) {
        cls->clusterName = NULL;
        cls->status      = 0;
        cls->masterName  = NULL ;
        cls->managerName = NULL;
        cls->managerId   = 0;
        cls->numServers  = 0;
        cls->numClients  = 0;
        cls->nRes        = 0;
        cls->resources   = NULL;
        cls->nTypes      = 0;
        cls->hostTypes   = NULL;
        cls->nModels     = 0;
        cls->hostModels  = NULL;
        cls->nAdmins     = 0;
        cls->adminIds    = NULL;
        cls->admins      = NULL;
    }
}


void freeHostInfo (struct hostInfo *host)
{
    if (host != NULL) {
        FREEUP (host->hostType);
        FREEUP (host->hostModel);
        for ( unsigned int i = 0; i < host->nRes; i++) {
            FREEUP (host->resources[i]);
        }
        FREEUP (host->resources);
        FREEUP (host->windows);
        FREEUP (host->busyThreshold);
    }

    return;
}


void initHostInfo (struct hostInfo *host)
{
    if (host != NULL) {
        host->hostName      = NULL;
        host->hostType      = NULL;
        host->hostModel     = NULL;
        host->cpuFactor     = 0.0F;
        host->maxCpus       = 0;
        host->maxMem        = 0;
        host->maxSwap       = 0;
        host->maxTmp        = 0;
        host->nDisks        = 0;
        host->nRes          = 0;
        host->resources     = NULL;
        host->windows       = NULL;
        host->numIndx       = 0;
        host->busyThreshold = NULL;
        host->isServer      = 0;
        host->rexPriority   = 0;
    }

    return;
}


int ls_getClusAdmins (char *line, const char *filename, size_t *lineNum, const char *secName, int lookupAdmins) // function name is replicated in limd/conf.c
{
    struct admins *admins = NULL;
    static char lastSecName[40]; // FIXME FIXME FIXME FIXME FIXME 40 chars are awfully specific
    const char clustermanager[] = "clustermanager";
    const char clusteradmins [] = "clusteradmins";

    admins = liblsf_getAdmins( line, filename, lineNum, secName, lookupAdmins );
    if (admins->nAdmins <= 0)
    {
        /* catgets 5118 */
        ls_syslog (LOG_ERR, "catgets 5118: %s: %s(%d): No valid user for section %s: %s", __func__, filename, *lineNum, secName, line);
        return -1;
    }
    if (strcmp (secName, clustermanager ) == 0 && strcmp (lastSecName, clusteradmins ) == 0)
    {
        strcpy (lastSecName, "");
        if (ls_setAdmins (admins, A_THEN_M) < 0)
            return -1;
    }
    else if (strcmp (lastSecName, clustermanager ) == 0 && strcmp (secName, clusteradmins ) == 0)
    {
        strcpy (lastSecName, "");
        if ( ls_setAdmins (admins, M_THEN_A) < 0)
            return -1;
    }
    else
    {
        if ( ls_setAdmins (admins, M_OR_A) < 0)
            return -1;
    }
    strcpy (lastSecName, secName);
    return 0;
}

struct admins *liblsf_getAdmins ( const char *line, const char *filename, size_t *lineNum, const char *secName, int lookupAdmins)
{

    static int first = TRUE;
    static struct admins admins;

    char *word            = NULL;
    unsigned int numAds   = 0;
    struct passwd *pw     = NULL;
    struct group *unixGrp = NULL;

    assert( filename != NULL );  /* FIXME these three asserts are here so the compiler will not moan */
    assert( lineNum  != NULL );  /* Closer investgation is needed as to where are these three vars used in */
    assert( secName  != NULL );


    /* FIXME FIXME
     this 'first' deal must go. i suspect that the reason it exists is to limit
     contention between re-reading. or some sort of SIGHUP
     */
    if (first == FALSE) {

        for ( unsigned int i = 0; i < admins.nAdmins; i++) { // FIXME FIXME FIXME FIXME admins.nAdmins will probably go caboom cuz not defined
            FREEUP (admins.adminNames[i]);
        }
        FREEUP (admins.adminNames);

        FREEUP (admins.adminIds);
        FREEUP (admins.adminGIds);
    }

    first = FALSE;
    admins.nAdmins = 0;
    while ((word = getNextWord_ (&line)) != NULL) {
        numAds++;
    }

    if (numAds)
    {
        admins.adminIds   = malloc( numAds * sizeof (uid_t)  );
        admins.adminGIds  = malloc( numAds * sizeof (gid_t)  );
        admins.adminNames = malloc( numAds * sizeof (char *) );
        if (admins.adminIds == NULL || admins.adminGIds == NULL || admins.adminNames == NULL)
        {
            const char mallocString[] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString );
            FREEUP (admins.adminIds);
            FREEUP (admins.adminGIds);
            FREEUP (admins.adminNames);
            admins.nAdmins = 0;
            lserrno = LSE_MALLOC;
            return &admins;
        }
    }
    else {
        return &admins;
    }

    while ((word = getNextWord_ (&line)) != NULL) {
        const char *forWhat = "for LSF administrator";

        if (lookupAdmins) {
            if ((pw = getpwlsfuser_ (word)) != NULL) {
                if (putInLists (word, &admins, &numAds, forWhat) < 0) {
                    return &admins;
                }
            }
            else if ((unixGrp = getgrnam (word)) != NULL) {
                int i = 0;
                while (unixGrp->gr_mem[i] != NULL) {
                    if( putInLists(unixGrp->gr_mem[i++], &admins, &numAds, forWhat) < 0) {
                        return &admins;
                    }
                }
            }
            else {
                if (putInLists (word, &admins, &numAds, forWhat) < 0) {
                    return &admins;
                }
            }
        }
        else {
            if (putInLists (word, &admins, &numAds, forWhat) < 0) {
                return &admins;
            }
        }
    }

    return &admins;
}


/***************************************************************************************
 * int ls_setAdmins (struct admins *admins, int mOrA)
 *
 * INPUT ; struct admins, int mOrA
 * OUTPUT:
 *
 * WHAT DOES THIS FUNCTION DO:
 *      grab the already existing admins from memory.
 *      If new admins are registerd in the configuration files, reconstruct the admins from scratch
 *      n order to add/delete entries instead of proper insertion/deletion from a hash.
 * 
 */
int
ls_setAdmins (struct admins *admins, int mOrA)
{
    uid_t workNAdmins     = 0;
    uid_t tempNAdmins     = 0;
    uid_t *tempAdminIds   = NULL;
    uid_t *workAdminIds   = NULL;
    char **tempAdminNames = NULL;
    char **workAdminNames = NULL;

    const char mallocString[]   = "malloc";

    tempNAdmins = admins->nAdmins + clinfo.nAdmins; // include/lib/conf.h: struct clusterInfo clinfo;
    tempAdminIds   =  malloc ( tempNAdmins * sizeof ( tempAdminIds ) );
    tempAdminNames =  malloc ( tempNAdmins * sizeof ( *tempAdminNames ) + 1 ); // FIXME FIXME FIXME FIXME FIXME check if this malloc allocation is correct

    if (!tempAdminIds || !tempAdminNames) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
        FREEUP (tempAdminIds);
        FREEUP (tempAdminNames);
        return -1;
    }

    if (mOrA == M_THEN_A) { // if true, use the command-line provided info
        workNAdmins    = clinfo.nAdmins;
        workAdminIds   = clinfo.adminIds;
        workAdminNames = clinfo.admins;
    }
    else { // otherwise, use what is in memory
        workNAdmins    = admins->nAdmins;
        workAdminIds   = admins->adminIds;
        workAdminNames = admins->adminNames; // FIXME FIXME FIXME FIXME this needs verification
    }

    for ( unsigned int i = 0; i < workNAdmins; i++) {

        tempAdminIds[i] = workAdminIds[i];
        if ((tempAdminNames[i] = putstr_ (workAdminNames[i])) == NULL)
        {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString );

            for ( unsigned int k = 0; k < i; k++) {
                free( (char *)tempAdminNames[k]);
            }

            free( (char *) tempAdminIds );
            free( (char *) tempAdminNames );
            return -1;
        }
    }

    tempNAdmins = workNAdmins;
    if (mOrA == M_THEN_A) {
        workNAdmins    = admins->nAdmins;
        workAdminIds   = admins->adminIds;
        workAdminNames = admins->adminNames;
    }
    else if (mOrA == A_THEN_M) {
        workNAdmins    = clinfo.nAdmins;
        workAdminIds   = clinfo.adminIds;
        workAdminNames = clinfo.admins;
    }
    else {
        workNAdmins = 0;
    }

    for (unsigned i = 0; i < workNAdmins; i++) {

        if (isInlist2 ( tempAdminNames, workAdminNames[i], tempNAdmins)) {
            continue;
        }

        tempAdminIds[tempNAdmins] = workAdminIds[i];
        if ((tempAdminNames[tempNAdmins] = putstr_ (workAdminNames[i])) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString );
            for ( unsigned int k = 0; k < tempNAdmins; k++) {
                free( (char *) tempAdminNames[k] );
            }

            free( (char *) tempAdminIds );
            free( (char *) tempAdminNames );
            return -1;
        }

        tempNAdmins++;
    }

    if (clinfo.nAdmins > 0) {
        for ( unsigned int i = 0; i < clinfo.nAdmins; i++) {
            free( (char *) clinfo.admins[i]);
        }

        FREEUP (clinfo.adminIds);
        FREEUP (clinfo.admins);
    }
    else {
        ls_syslog (LOG_ERR, "%s: clinfo.nAdmins: value is not greater than zero! (%d)\n", __func__, clinfo.nAdmins);
    }

    clinfo.nAdmins  = tempNAdmins;
    clinfo.adminIds = tempAdminIds;
    clinfo.admins   = tempAdminNames;

    return 0;
}


void
liblsf_putThreshold (unsigned int indx, const struct hostInfo *host, const size_t position, const char *value, double def)
{
    if ( NULL == host ) {
        return;
    }

    if (position != UINT_MAX) {
        if (strcmp (value, "") == 0) {
            host->busyThreshold[indx] = def;
        }
        else {
            host->busyThreshold[indx] = (float) atof( value ); // FIXME FIXME must investigate upper and lower limits of host->busyThreshold and set the type appropriately
        }
    }
    else {
        host->busyThreshold[indx] = def;
    }

    return;

}

bool addHost (struct hostInfo *host, const char *filename, size_t *lineNum)
{
    struct hostInfo *newlist = NULL;

    if (NULL == host ) {
        return false;
    }

    for ( unsigned int i = 0; i < cConf->numHosts; i++) {

        if (! equalHost_(cConf->hosts[i].hostName, host->hostName) ) {
            continue;
        }

        /* catgets 5163 */
        ls_syslog (LOG_WARNING, "catgets 5163: %s: %s(%d): host <%s> redefined, using previous definition", __func__, filename, *lineNum, host->hostName);
        freeHostInfo (host);

        return false;
    }

    cConf->numHosts++;

    newlist = malloc( cConf->numHosts * sizeof( struct hostInfo ) );
    if ( ( NULL == newlist ) && ENOMEM == errno ) {

        assert( cConf->numHosts );  // FIXME FIXME if numHosts is always >= 0
                                    //      change numHosts type to unsigned long
                                    //      remove assertion
                                    //      remove cast
                                    // search for more shit like that

        // FIXME FIXME FIXME
        //      ALL CASTS WITHIN THIS FILE SHOULD BE TREATED WITH SUSPICION
        //      DELETE CASTS, RECOMPILE AND ASSIGN PROPER TYPES TO EACH

        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", cConf->numHosts * sizeof (struct hostInfo));
        cConf->numHosts--;
        lserrno = LSE_MALLOC;
        return false;
    }

    for (unsigned int i = 0; i < cConf->numHosts - 1; i++) {
        newlist[i] = cConf->hosts[i];
    }

    newlist[cConf->numHosts - 1] = *host;
    FREEUP (cConf->hosts);
    cConf->hosts = newlist;

    return true;
}

void 
initkeylist (struct keymap keyList[], unsigned int m, unsigned int n, struct lsInfo *info)
{

    assert( m ); // check if these two amigos are 0
    assert( n ); 
    for ( unsigned int i = 0; i < m - 1; i++) {
        keyList[i].key = "";
    }

    for ( unsigned int i = 0; i < n; i++) {
        keyList[i].value = NULL;
        keyList[i].position = 0;
    }

    if (NULL == info) {
        int i = 0;
        unsigned int index = 0;

        while ( NULL != builtInRes[i].name ) {

            if (builtInRes[i].flags & RESF_DYNAMIC) {
                keyList[index++].key = builtInRes[i].name;
            }
            i++;
        }
    }
    else
    {
        unsigned int index = 0;
        for (unsigned int i = 0; i < info->nRes; i++) {
            if ((info->resTable[i]->flags & RESF_DYNAMIC) && index < info->numIndx) {
                keyList[index++].key = info->resTable[i]->name;
            }
        }
    }

    return;
}

void freekeyval (struct keymap keylist[])
{
    for ( int cc = 0; NULL != keylist[cc].key; cc++) {
        if ( NULL != keylist[cc].value ) {
            FREEUP (keylist[cc].value);
        }
    }

    return;
}

char *parsewindow ( const char *linep, const char *filename, const size_t *lineNum, const char *section)
{
    char *word    = NULL;
    char *save    = NULL;
    char *windows = NULL;

    if (linep == NULL){
        return NULL;
    }

    windows = putstr_ (linep);
    if (windows == NULL) {
        return NULL;
    }

    windows[ strlen( windows + 1) ] = '\0';
    while ((word = getNextWord_ (&linep)) != NULL)
    {
        save = putstr_ (word);
        if (save == NULL)
        {
            FREEUP (windows);
            return NULL;
        }
        if (validWindow (word, section) < 0)
        {
            /* catgets 5165 */
            ls_syslog (LOG_ERR, "catgets 5165: File %s section %s at line %d: Bad time expression <%s>; ignored.", filename, section, *lineNum, save );
            lserrno = LSE_CONF_SYNTAX;
            FREEUP (save);
            continue;
        }
        if (*windows != '\0') {
            strcat (windows, " ");
        }
        strcat (windows, save);
        FREEUP (save);
    }

    if (windows[0] == '\0')
    {
        FREEUP (windows);
    }

    return windows;
}

int validWindow ( const char *wordpair, const char *context)
{
    unsigned int oday = 0;
    unsigned int cday = 0;
    float ohour       = 0.0F;
    float chour       = 0.0F;
    char *sp          = NULL;
    char *word        = NULL;

    sp = strchr (wordpair, '-');
    if (!sp)
    {
        /* catgets 5166 */
        ls_syslog (LOG_ERR, "catgets 5166: Bad time expression in %s", context );
        return -1;
    }

    *sp = '\0';
    sp++;
    word = sp;

    if (parse_time (word, &chour, &cday) < 0)
    {
        ls_syslog (LOG_ERR, "catgets 5166: Bad time expression in %s", context );
        return -1;
    }

    // word = wordpair;

    if (parse_time (wordpair, &ohour, &oday) < 0)
    {
        ls_syslog (LOG_ERR, "catgets 5166: Bad time expression in %s", context );
        return -1;
    }

    if (((oday && cday) == 0) && (oday != cday))
    {
        /* catgets 5169 */
        ls_syslog (LOG_ERR, "catgets 5169: Ambiguous time in %s", context );
        return -1;
    }

    return 0;
}


//  FIXME FIXME FIXME Why on earth is this function parsing time? there are standard functions to do that
int parse_time (const char *word, float *hour, unsigned int *day) // FIXME FIXME
{

    float min = 0.0f;
    char *sp  = NULL;

    
    // FIXME FIXME FIXME wat? assignment to 0? not my code
    // *hour = 0.0;
    // *day  = 0;


    sp = strrchr (word, ':');
    if (!sp) {

        if (!isint_ (word) || atoi (word) < 0) {
            return -1;  // FIXME FIXME FIXME 
        }

        *hour = (float) atof (word);  // FIXME FIXME FIXME FIXME check if cast can be removed
        if (*hour > 23) {
            return -1;  // FIXME FIXME FIXME 
        }

    }
    else {
        *sp = '\0';
        sp++;

        if (!isint_ (sp) || atoi (sp) < 0) {
            return -1;  // FIXME FIXME FIXME 
        }

        min = (float) atof (sp); // FIXME FIXME FIXME the (float) cast here is probably correct, but try to find a way to get rid of it, without turning min to double

        if (min > 59.0f) {
            return -1;  // FIXME FIXME FIXME 
        }

        sp = strrchr (word, ':');

        if (!sp) {

            if (!isint_ (word) || atoi (word) < 0) {
                return -1; // FIXME FIXME FIXME
            }

            *hour = (float) atof (word);

            if (*hour > 23.0f) {
                return -1; // FIXME FIXME FIXME
            }
        }
        else {

            *sp = '\0';
            sp++;

            if (!isint_ (sp) || atoi (sp) < 0) {
                return -1; // FIXME FIXME FIXME
            }

            *hour = (float) atof (sp);
            if (*hour > 23.0f) {
                return -1; // FIXME FIXME FIXME
            }

            if (!isint_ (word) || atoi (word) < 0){
                return -1; // FIXME FIXME FIXME
            }

            assert( atoi( word ) >= 0 );
            *day = (unsigned int) atoi (word);
            if (*day == 0) {
                *day = 7;
            }

            if (*day < 1 || *day > 7) {
                return -1; // FIXME FIXME FIXME
            }
        }
    }

    *hour += min / 60.0f;

    return 0;
}


char do_Cluster (FILE * fp, size_t *lineNum, const char *filename)
{
    char *linep                = NULL;
    char *servers              = NULL;
    bool_t found               = FALSE;

    enum {
        CLUSTERNAME,
        SERVERS,
        KEYNULL
    };

    const char *keylist[ ] = { 
        "CLUSTERNAME",
        "SERVERS",
        NULL
    };

    struct keymap keyList[] = {
        { CLUSTERNAME, keylist[ CLUSTERNAME ], NULL },
        { SERVERS,     keylist[ SERVERS ],     NULL },
        { KEYNULL,     NULL, NULL }
    };

    const char cluster[]       = "cluster";
    const char mallocString[]  = "malloc";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
    {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, cluster); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, cluster)) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL)
    {
        if (!keyMatch (keyList, linep, FALSE))
        {
            /* catgets 5171 */
            ls_syslog (LOG_ERR, "catgets 5171: %s: %s(%d): keyword line format error for section cluster; ignoring section", __func__, filename, *lineNum);
            doSkipSection (fp, lineNum, filename, cluster);
            return FALSE;
        }

        // if (keyList[CLUSTERNAME].position == -1)
        if (keyList[CLUSTERNAME].position == 255 ) // FIXME FIXME FIXME FIXME turn into label, global
        {
             /* catgets 5172 */
            ls_syslog (LOG_ERR, "catgets 5172: %s: %s(%d): keyword line: key %s is missing in section cluster; ignoring section", __func__, filename, *lineNum, keyList[CLUSTERNAME].key);
            doSkipSection (fp, lineNum, filename, cluster);
            return FALSE;
        }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
        {
            if (isSectionEnd (linep, filename, lineNum, cluster))
                return TRUE;
            if (found)
                return TRUE;

            if (mapValues (keyList, linep) < 0)
            {
                 /* catgets 5173 */
                ls_syslog (LOG_ERR, "catgets 5173: %s: %s(%d): values do not match keys for section cluster, ignoring line", __func__, filename, *lineNum);
                continue;
            }

            if (keyList[SERVERS].position != 255 ) // FIXME FIXME FIXME FIXME turn into label, global
                servers = keyList[SERVERS].value;
            else
                servers = NULL;

            if ((sConf->clusterName = putstr_ (keyList[CLUSTERNAME].value)) == NULL)
            {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
                FREEUP (keyList[CLUSTERNAME].value);
                if (keyList[SERVERS].position != 255) { // FIXME FIXME FIXME FIXME turn into label, global
                    FREEUP (keyList[SERVERS].value);
                }
                return FALSE;
            }

            if ((sConf->servers = putstr_ (servers)) == NULL)
            {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
                FREEUP (keyList[CLUSTERNAME].value);
                if (keyList[SERVERS].position != 255 ) { // FIXME FIXME FIXME FIXME turn into label, global
                    FREEUP (keyList[SERVERS].value);
                }
                return FALSE;
            }

            found = TRUE;
            FREEUP (keyList[CLUSTERNAME].value);
            if (keyList[SERVERS].position != 255 ) { // FIXME FIXME FIXME FIXME turn into label, global
                FREEUP (keyList[SERVERS].value);
            }
        }
    }
    else
    {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, cluster);
        doSkipSection (fp, lineNum, filename, cluster);
        return FALSE;
    }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, cluster); /*catgets33 */
    return FALSE;
}



char do_Clparams (FILE * clfp, const char *lsfile, size_t *lineNum)
{
    char *linep = NULL;

// #define EXINTERVAL              0
// #define ELIMARGS                1
// #define PROBE_TIMEOUT           2
// #define ELIM_POLL_INTERVAL      3
// #define HOST_INACTIVITY_LIMIT   4
// #define MASTER_INACTIVITY_LIMIT 5
// #define RETRY_LIMIT             6
// #define ADJUST_DURATION         7
// #define LSF_ELIM_DEBUG          8
// #define LSF_ELIM_BLOCKTIME      9
// #define LSF_ELIM_RESTARTS       10

    enum {
        EXINTERVAL,
        ELIMARGS,
        PROBE_TIMEOUT,
        ELIM_POLL_INTERVAL,
        HOST_INACTIVITY_LIMIT,
        MASTER_INACTIVITY_LIMIT,
        RETRY_LIMIT,
        ADJUST_DURATION,
        LSF_ELIM_DEBUG,
        LSF_ELIM_BLOCKTIME,
        LSF_ELIM_RESTARTS,
        KEYNULL = 255
    };

    const char *keylist[ ] = {
        "EXINTERVAL",
        "ELIMARGS",
        "PROBE_TIMEOUT",
        "ELIM_POLL_INTERVAL",
        "HOST_INACTIVITY_LIMIT",
        "MASTER_INACTIVITY_LIMIT",
        "RETRY_LIMIT",
        "ADJUST_DURATION",
        "LSF_ELIM_DEBUG",
        "LSF_ELIM_BLOCKTIME",
        "LSF_ELIM_RESTARTS",
        NULL
    };

    struct keymap keyList[ ] = {
        { EXINTERVAL,              keylist[ EXINTERVAL ],              NULL },
        { ELIMARGS,                keylist[ ELIMARGS ],                NULL },
        { PROBE_TIMEOUT,           keylist[ PROBE_TIMEOUT ],           NULL },
        { ELIM_POLL_INTERVAL,      keylist[ ELIM_POLL_INTERVAL ],      NULL },
        { HOST_INACTIVITY_LIMIT,   keylist[ HOST_INACTIVITY_LIMIT ],   NULL },
        { MASTER_INACTIVITY_LIMIT, keylist[ MASTER_INACTIVITY_LIMIT ], NULL },
        { RETRY_LIMIT,             keylist[ RETRY_LIMIT ],             NULL },
        { ADJUST_DURATION,         keylist[ ADJUST_DURATION ],         NULL },
        { LSF_ELIM_DEBUG,          keylist[ LSF_ELIM_DEBUG ],          NULL },
        { LSF_ELIM_BLOCKTIME,      keylist[ LSF_ELIM_BLOCKTIME ],      NULL },
        { LSF_ELIM_RESTARTS,       keylist[ LSF_ELIM_RESTARTS ],       NULL },
        { KEYNULL,                 NULL, NULL }
    };

    const char parameters[] = "parameters";

    linep = getNextLineC_ (clfp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, parameters); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, lsfile, lineNum, parameters)) {
        return TRUE;
    }

    if (strchr (linep, '=') == NULL) {
        /* catgets 5195 */
        ls_syslog (LOG_ERR, "catgets 5195: %s: %s(%d): vertical section not supported, ignoring section", __func__, lsfile, *lineNum);
        doSkipSection (clfp, lineNum, lsfile, parameters);
        return FALSE;
    }
    else {
        if (readHvalues (keyList, linep, clfp, lsfile, lineNum, FALSE, parameters) < 0)  {
            return FALSE;
        }
        return TRUE;
    }
}

void freeKeyList (struct keymap *keyList)
{
    for ( unsigned int i = 0; keyList[i].key != NULL; i++) {
        if ( keyList[i].position != 255 ) { // FIXME FIXME FIXME FIXME turn into label, global
            FREEUP ( keyList[i].value );
        }
    }
}


int validType (char *type)
{
    if (type == NULL)  {
        return -1;
    }

    if (!strcasecmp (type, "Boolean")) {
        return LS_BOOLEAN;
    }

    if (!strcasecmp (type, "String")) {
        return LS_STRING;
    }

    if (!strcasecmp (type, "Numeric")) {
        return LS_NUMERIC;
    }

    if (!strcmp (type, "!")) {
        return LS_EXTERNAL;
    }

    return -1;
}


int doResourceMap (FILE * fp, const char *lsfile, size_t *lineNum)
{
    char *linep = NULL;
    unsigned int resNo   = 0;

    enum {
        RESOURCENAME,
        LOCATION,
        KEYNULL = 255
    };

    const char *keylist[ ] = {
        "RESOURCENAME",
        "LOCATION",
        NULL
    };

    struct keymap keyList[] = {
        { RESOURCENAME, keylist[ RESOURCENAME ], NULL },
        { LOCATION,     keylist[ LOCATION ],     NULL },
        { KEYNULL,      NULL, NULL }
    };

    const char resourceMapString[] = "resourceMap";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, resourceMapString); /*catgets33 */
        return -1;
    }

    if (isSectionEnd (linep, lsfile, lineNum, resourceMapString)) {
        char buffer[MAX_FILENAME_LEN];
        /* catgets 5109 */
        sprintf( buffer, "catgets 5109: %s: %s(%lu): Empty resourceMap, no keywords or resources defined.", __func__, lsfile, *lineNum);
        ls_syslog (LOG_WARNING, buffer );
        return -1;
    }

    if (strchr (linep, '=') == NULL) {
        if (!keyMatch (keyList, linep, TRUE)) {
            /* catgets 5197 */
            ls_syslog (LOG_ERR, "catgets 5197: %s: %s(%d): keyword line format error for section resource, ignoring section", __func__, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, resourceMapString);
            return -1;
        }


        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
        {
            if (isSectionEnd (linep, lsfile, lineNum, resourceMapString)) {
                return 0;
            }
            if (mapValues (keyList, linep) < 0) {
                /* catgets 5198 */
                ls_syslog (LOG_ERR, "catgets 5198: %s: %s(%d): values do not match keys for resourceMap section, ignoring line", __func__, lsfile, *lineNum);
                continue;
            }

            resNo = resNameDefined (keyList[RESOURCENAME].value);
            if ( !resNo ) {
                char buffer[MAX_FILENAME_LEN];
                memset( buffer, '\0', MAX_FILENAME_LEN );
                sprintf( buffer, "catgets 5199: %s: %s(%lu): Resource name <%s> is not defined; ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].value );
                ls_syslog (LOG_ERR, buffer);
                freeKeyList (keyList);
                continue;
            }
            if (keyList[LOCATION].value != NULL )
            {

                if (strstr (keyList[LOCATION].value, "all ") && strchr (keyList[LOCATION].value, '~')) // FIXME FIXME "all " ? what's with the extra space
                {

                    struct HostsArray array;
                    int result = 0;

                    array.size = 0;
                    array.hosts = malloc( cConf->numHosts * sizeof( array.hosts ) ); // FIXME FIXME FIXME almost certain buffer overflow
                    if (!array.hosts)
                    {
                        const char mallocString[ ] = "malloc";
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
                        freeKeyList (keyList);
                        return -1;
                    }
                    for ( unsigned int cnt = 0; cnt < cConf->numHosts; cnt++)
                    {
                        array.hosts[array.size] =
                        strdup (cConf->hosts[cnt].hostName);
                        if (!array.hosts[array.size])
                        {
                            const char mallocString[] = "malloc";
                            freeSA_ (array.hosts, array.size);
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
                            freeKeyList (keyList);
                            return -1;
                        }
                        array.size++;
                    }

                    char *kot = strdup( keyList[ LOCATION ].value );
                    result = convertNegNotation_( &kot, &array );
                    if (result == 0) {
                        /* catgets 5397 */
                        ls_syslog (LOG_WARNING, "catgets 5397: %s:%s: %s(%d): convertNegNotation_: all the hosts are to be excluded %s !", __func__, lsfile, *lineNum, keyList[LOCATION].value);
                    }
                    else if (result < 0) {
                        /* catgets 5398 */
                        ls_syslog (LOG_WARNING, "catgets 5398: %s:%s: %s(%d): convertNegNotation_: Wrong syntax \'%s\'", __func__, lsfile, *lineNum, keyList[LOCATION].value);
                    }
                    freeSA_ (array.hosts, array.size);
                }

                if ( addResourceMap (keyList[RESOURCENAME].value, keyList[LOCATION].value, lsfile, *lineNum) < 0)
                {
                    /* catgets 5200 */
                    ls_syslog (LOG_ERR, "catgets 5200: %s:%s: %s(%d): addResourceMap() failed for resource <%s>; ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].value);
                    freeKeyList (keyList);
                    continue;
                
}
                lsinfo.resTable[resNo]->flags &= ~RESF_GLOBAL;
                lsinfo.resTable[resNo]->flags |= RESF_SHARED;
                resNo = 0;
            }
            else
            {
                /* catgets 5201 */
                ls_syslog (LOG_ERR, "catgets 5201: %s: %s(%d): No LOCATION specified for resource <%s>; ignoring the line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].value);
                freeKeyList (keyList);
                continue;
            }
            freeKeyList (keyList);
        }
    }
    else
    {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, lsfile, *lineNum, resourceMapString );
        return -1;
    }
    return 0;

}


int addResourceMap ( const char *resName, const char *location, const char *lsfile, const size_t lineNum)
{
    // int j                                = 0;
    int i                                 = 0;
    int error                             = 0; // no error
    int first                             = TRUE;
    unsigned int numHosts                 = 0;
    char *sp                              = NULL;
    char *cp                              = NULL;
    char *ssp                             = NULL;
    char *tempHost                        = NULL;
    char initValue[MAX_FILENAME_LEN];
    char externalResourceFlag[]           = "!";
    char **hosts                          = NULL;
    struct lsSharedResourceInfo *resource = NULL;

    // const char addResourceMap[] = "addResourceMap";
    memset( initValue, '\0', MAX_FILENAME_LEN );

    if (resName == NULL || location == NULL) {

        char buffer[MAXHOSTNAMELEN];
        char null[] = "NULL";
        lsferrno = ENORESNAME;  // lsferrno is a global reference

        memset( buffer, '\0', MAXHOSTNAMELEN );

        if( NULL == location ) {
            // lsferrno |=  lsferr | ENOLOCATION; // original, but lsferr does not really exist
            // lsferrno |= ENOLOCATION;
            lsferrno = ENOLOCATION;
        }

        /* catgets 5203 */
        sprintf( buffer, "catgets 5203: %s: %s(%lu): Resource name <%s> location <%s>", __func__, lsfile, lineNum, (resName ? resName : null ), (location ? location : null ) );
        ls_syslog (LOG_ERR, buffer);
        return -1;
    }

    if (!strcmp (location, "!"))
    {
        memset( initValue, '\0', MAX_FILENAME_LEN );
        tempHost = (char *) externalResourceFlag; // FIXME FIXME FIXME is this cast justified?
        hosts = &tempHost;
        if ((resource = addResource (resName, 1, hosts, initValue, lsfile, lineNum)) == NULL)
        {
            /* catgets 5209 */
            ls_syslog (LOG_ERR, "catgets 5209: %s:%s: %s(%d): %s() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, addResource, "!");
            return -1;
        }
        return 0;
    }

    resource = NULL;
    sp = strdup( location ); // FIXME FIXME FIXME FIXME this cast oughta be verified

    i = 0;
    while (*sp != '\0')
    {
        if (*sp == '[') {
            i++;
        }
        else if (*sp == ']') {
            i--;
        }
        sp++;
    }
    free( sp );
    sp = strdup( location ); // FIXME FIXME FIXME FIXME this cast oughta be verified
    if (i != 0)
    {
        /* catgets 5204 */
        ls_syslog (LOG_ERR, "catgets 5204: %s: %s(%d): number of '[' is not match that of ']' in <%s> for resource <%s>; ignoring", __func__, lsfile, lineNum, location, resName);
        return -1;
    }

    while (sp != NULL && sp[0] != '\0')
    {
        for ( unsigned int j = 0; j < numHosts; j++) { // Cast is ok: by now we have established that numHosts > 0
            FREEUP (hosts[j]);
        }
        FREEUP (hosts);
        error = FALSE;
        memset( initValue, '\0', MAX_FILENAME_LEN );
        while (*sp == ' ' && *sp != '\0') {
            sp++;
        }
        if (*sp == '\0') {
            if (first == TRUE) {
                return -1;
            }
            else {
                return 0;
            }
        }
        cp = sp;
        while (isalnum (*cp)) {
            cp++;
        }
        if (cp != sp)
        {
            ssp = cp;
            cp = NULL;
            strcpy (initValue, sp);
            cp = ssp;
            if (isspace (*cp)) {
                cp++;
            }
            if (*cp != '@') {
                error = TRUE;
            }
            sp = cp + 1;
        }
        if (isspace (*sp)) {
            sp++;
        }

        if (*sp != '[' && *sp != '\0')
        {
            /* catgets 5205 */
            ls_syslog (LOG_ERR, "catgets 5205: %s: %s(%d): Bad character <%c> in instance; ignoring", __func__, lsfile, lineNum, *sp);
            sp++;
        }
        if (isspace (*sp)) {
            sp++;
        }
        if (*sp == '[')
        {
            sp++;
            cp = sp;
            while (*sp != ']' && *sp != '\0') {
                sp++;
            }
            if ( *sp == '\0') // FIXME FIXME yeah this probably attempts to check if the first char is replaced by nothing
            {
                /* catgets 5206 */
                ls_syslog (LOG_ERR, "catgets 5206: %s: %s(%d): Bad format for instance <%s>; ignoring the instance", __func__, lsfile, lineNum, sp);
                return -1;
            }
            if (error == TRUE)
            {
                sp++;
                ssp = sp;
                sp = NULL; // FIXME FIXME FIXME probable segfault
                /* catgets 5207 */
                ls_syslog (LOG_ERR, "catgets 5207: %s: %s(%d): Bad format for instance <%s>; ignoringthe instance", __func__, lsfile, lineNum, sp);
                sp = ssp;
                continue;
            }
            *sp = '\0';
            sp++;
            if (!(numHosts = parseHostList (cp, lsfile, lineNum, &hosts))) {
                const char *parseHostList = "parseHostList";
                /* catgets 5208 */
                ls_syslog (LOG_ERR, "catgets 5208: %s: %s(%d): %s(%s) failed; ignoring the instance <%s%s>", __func__, lsfile, lineNum, parseHostList, cp, sp, "]");
                continue;
            }

            if (resource == NULL) {
                if ((resource = addResource (resName, numHosts, hosts, initValue, lsfile, lineNum)) == NULL)
                    /* catgets 5209 */
                    ls_syslog (LOG_ERR, "catgets 5209: %s: %s(%d): %s() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, addResource, sp);
            }
            else {
                // if (liblsf_addHostInstance (resource, numHosts, hosts, initValue) < 0)
                if ( addHostInstance (resource, numHosts, hosts, initValue) < 0)
                    /* catgets 5210 */
                    ls_syslog (LOG_ERR, "catgets 5210: %s: %s(%d): %s() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, __func__, sp);
            }
            continue;
        }
        else
        {
            /* catgets 5211 */
            ls_syslog (LOG_ERR, "catgets 5211: %s: %s(%d): No <[>  for instance in <%s>; ignoring", __func__, lsfile, lineNum, location);
            while (*sp != ']' && *sp != '\0') {
                sp++;
            }
            if (*sp == '\0') {
                return -1;
            }
            sp++;
        }
    }
    for ( unsigned int j = 0; j < numHosts; j++) {
        FREEUP (hosts[j]);
    }

    FREEUP (hosts);

    return 0;
}

//     limd equiv:       int liblsf_addResourceMap ( const char *resName, const char *location, const char *lsfile, size_t lineNum)
// lsf/lib/liblsf/conf.c:int addResourceMap        ( const char *resName, const char *location, const char *lsfile, size_t lineNum, int *isDefault)
//      used to be here. Moved over to lsf/lib/liblsf/addResourceMap.c

struct lsSharedResourceInfo *addResource ( const char *resName, unsigned long nHosts, char **hosts, char *value, const char *filename, size_t lineNum)
{
    struct lsSharedResourceInfo *resInfo = NULL;

    assert( filename ); // FIXME FIXME FIXME these asserts got to go
    assert( lineNum );  // FIXME FIXME FIXME these asserts got to go

    if (resName == NULL || hosts == NULL) {
        return NULL;
    }

    assert( cConf->numShareRes ); // struct clusterConf *cConf global
    resInfo = myrealloc( cConf->shareRes, sizeof (struct lsSharedResourceInfo) *( cConf->numShareRes + 1 ));
    if (NULL == resInfo && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myrealloc");
        return NULL;
    }

    cConf->shareRes = resInfo;
    nRes = cConf->numShareRes;

    if ((resInfo[nRes].resourceName = putstr_ (resName)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return NULL;
    }

    resInfo[nRes].nInstances = 0;
    resInfo[nRes].instances = NULL;
    if ( addHostInstance (resInfo + nRes, nHosts, hosts, value) < 0) {
        free (resInfo[nRes].resourceName);
        return NULL;
    }

    cConf->numShareRes++;

    return resInfo + nRes;
}


// int liblsf_addHostInstance (struct lsSharedResourceInfo *sharedResource, unsigned int nHosts, char **hostNames, char *value)
int addHostInstance( struct lsSharedResourceInfo *sharedResource, unsigned long nHosts, char **hostNames, const char *value)
{
    unsigned int inst = 0;
    struct lsSharedResourceInstance *instance = NULL;
    
    if (nHosts <= 0 || hostNames == NULL) {
        lserrno = ENOHOSTADDED; // FIXME change lserrno to lsferrno
        return 0;
    }
    
    assert( sharedResource->nInstances ); // FIXME has to go.
    instance = myrealloc (sharedResource->instances, sizeof (struct lsSharedResourceInstance) * ( sharedResource->nInstances + 1));
    
    if (NULL == instance ) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myrealloc" );
        lserrno = ENOHOSTADDED; // FIXME change lserrno to lsferrno
        return 0;
    }
    
    sharedResource->instances = instance;
    inst = sharedResource->nInstances;
    
    if ((instance[inst].value = putstr_ (value)) == NULL) {
        char putstr_[] = "putstr_";
        lserrno = ENOHOSTADDED; // FIXME change lserrno to lsferrno
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, putstr_);
        return 0;
    }
    
    assert( nHosts );
    instance[inst].nHosts = nHosts;
    instance[inst].hostList = malloc (sizeof (char *) * nHosts);
    if( NULL == instance[inst].hostList && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc");
        free (instance[inst].value);
        return -1;
    }
    
    for ( unsigned int i = 0; i < nHosts; i++) {
        instance[inst].hostList[i] = putstr_ (hostNames[i]);
        if ( NULL ==  instance[inst].hostList[i] ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, putstr_);
            for (i--; i > 0; i--) {
                free (instance[inst].hostList[i]);
            }
            
            free (instance[inst].hostList);
            free (instance[inst].value);
            return -1;
        }
    }
    
    sharedResource->nInstances++;

    lserrno = ENOHOSTADDED; // FIXME change lserrno to lsferrno
    return 0;
}


int convertNegNotation_ (char **value, struct HostsArray *array)
{
    int cnt        = 0;
    int result     = -1;
    // const char *ptr      = NULL;
    char *save     = NULL;
    char *outHosts = NULL;
    unsigned int LOCATION = 0;
    char *buffer    = strdup( value[ LOCATION ] );
    const char *sp1 = strstr( buffer, "all " );
    char *sp2       = strdup( sp1 );
    
    if (!buffer) {
        lserrno = LSE_MALLOC;
        // goto clean_up;
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        FREEUP (buffer);
        FREEUP (outHosts);

        return result;
    }
    
    for (cnt = 0; (sp2 > buffer) && sp2[ LOCATION ] != '['; cnt++) {
        sp2--;
    }
    
    if (!sp2 || sp2 < buffer) {
        // goto clean_up;
        FREEUP (buffer);
        FREEUP (outHosts);

        return result;
    }
    
    if (cnt > 1) {
        memmove (sp2 + 1, sp1, strlen (sp1) + 1);
    }
    
    sp1 = sp2;
    while (sp2 && sp2[LOCATION] != ']') {
        sp2++;
    }
    
    if (!sp1 || !sp2) {
        // goto clean_up;
        FREEUP (buffer);
        FREEUP (outHosts);

        return result;
    }

    ls_syslog (LOG_DEBUG, "%s: the original string is \'%s\'", __func__, value[LOCATION]);  // FIXME FIXME FIXME FIXME wrap this around debug conditional
    
    // ptr = sp1;
    save = getNextValueQ_ (&sp1, '[', ']');
    if (!save) {
        // goto clean_up;
        FREEUP (buffer);
        FREEUP (outHosts);

        return result;
    }
    
    result = resolveBaseNegHosts (save, &outHosts, array);
    if (result >= 0) {
        char *new_value = NULL;
        
        // *ptr = 0;
        new_value = malloc (strlen (buffer) + strlen (outHosts) + strlen (sp2) + 2);
        if (!new_value) {
            lserrno = LSE_MALLOC;
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            // goto clean_up;

            FREEUP (buffer);
            FREEUP (outHosts);

            return result;
        }
        strcpy (new_value, buffer);
        strcat (new_value, "[");
        strcat (new_value, outHosts);
        strcat (new_value, sp2);
        
        FREEUP( value[0] ); // FIXME FIXME FIXME find out what value[0] is, create a union and mark the array subscript appropriatelly
        value[0] = new_value;
    }
    
// clean_up: // FIXME FIXME FIXME remove goto label
    
    if (lserrno == LSE_MALLOC) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
    }
    
    FREEUP (buffer);
    FREEUP (outHosts);
    
    return result;
}


void freeSA_ (char **list, unsigned int num)
{
    if (list == NULL || num <= 0) {
        return;
    }
    
    for ( unsigned int i = 0; i < num; i++) {
        FREEUP (list[i]);
    }
    FREEUP (list);

    return;
}


int resolveBaseNegHosts ( const char *inHosts, char **outHosts, struct HostsArray *array)
{
    unsigned int in_num     = 0;
    unsigned int neg_num    = 0;
    size_t size             = 0;
    unsigned int counter    = 0;
    char *word              = NULL;
    char **inTable          = NULL;
    char **outTable         = NULL;
    
    inTable = array->hosts;
    in_num = array->size;
    
    outTable = malloc( array->size * sizeof( outTable ) + 1 ); // FIXME FIXME FIXME certain buffer overflow
    if( !inTable || !outTable) {
        lserrno = LSE_MALLOC;
        freeSA_ (outTable, neg_num);
        FREEUP (outHosts[0]);
        return -1;
    }
    
    if ((word = getNextWord_ (&inHosts)) != NULL)
    {
        if (strcmp (word, "all")) {
            freeSA_ (outTable, neg_num);
            FREEUP (outHosts[0]);
            return -1;            
        }
        for ( unsigned int j = 0; j < in_num; j++) {
            size += strlen (inTable[j]);
        }
    }
    else {
        freeSA_ (outTable, neg_num);
        FREEUP (outHosts[0]);
        return -1;
    }
    
    while ((word = getNextWord_ (&inHosts)) != NULL) {
        if (word[0] == '~') {
            word++;
            if (!isalnum (word[0])) {
                freeSA_ (outTable, neg_num);
                FREEUP (outHosts[0]);
                return -1;
            }
        }
        else {
            continue;
        }
        
        outTable[neg_num] = strdup (word);
        if (!outTable[neg_num]) {
            lserrno = LSE_MALLOC;
            freeSA_ (outTable, neg_num);
            FREEUP (outHosts[0]);
            return -1;
        }
        neg_num++;
        if (((neg_num - array->size) % array->size) == 0)
        {
            outTable = realloc( outTable, ( array->size + neg_num) * sizeof ( outTable ));
            if (!outTable)
            {
                lserrno = LSE_MALLOC;
                freeSA_ (outTable, neg_num);
                FREEUP (outHosts[0]);
                return -1;
            }
        }
    }
    
    for ( unsigned int j = 0; j < neg_num; j++)
    {
        for ( unsigned int k = 0; k < in_num; k++)
        {
            if (inTable[k] && equalHost_ (inTable[k], outTable[j]))
            {
                size -= strlen (inTable[k]);
                free (inTable[k]);
                inTable[k] = NULL;
            }
        }
        FREEUP (outTable[j]);
    }
    
    outHosts[0] = malloc( size + in_num ); // FIXME FIXME FIXME find out what outHosts[0] is, create a union and mark the array subscript appropriatelly
    if (!outHosts[0]) {                    // FIXME FIXME FIXME find out what outHosts[0] is, create a union and mark the array subscript appropriatelly
        lserrno = LSE_MALLOC;
        freeSA_ (outTable, neg_num);
        FREEUP (outHosts[0]);
        return -1;
    }
    
    outHosts[0][0] = 0; // FIXME FIXME FIXME find out what outHosts[0][0] is, create a union and mark the array subscript appropriatelly
    for ( unsigned int j = 0; j < in_num; j++) {
        if (inTable[j])
        {
            strcat (outHosts[0], inTable[j]);
            FREEUP (inTable[j]);
            strcat (outHosts[0], " ");
            counter++;
        }
    }
    
    if (outHosts[0][0]) { // FIXME FIXME FIXME find out what outHosts[0][0] is, create a union and mark the array subscript appropriatelly
        outHosts[0][strlen (outHosts[0]) - 1] = '\0';
    }
    
    FREEUP (outTable);
    
    assert( counter <= INT_MAX);
    return (int) counter; // FIXME FIXME FIXME FIXME remove the cast
}

// unsigned int resNameDefined ( const char *name ) used to live here;
//            moved to lsf/lib/liblsf/resNameDefined.
