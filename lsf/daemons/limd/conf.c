/* $Id: lim.conf.c 397 2007-11-26 19:04:00Z mblack $
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

#include "daemons/liblimd/limd.h"
#include "daemons/liblimd/conf.h"
#include "lib/xdrlim.h"
#include "lib/conf.h"
#include "lib/lib.h"

// #define NL_SETN 24


/* getHostType()
 */
char *
getHostType (void)
{
    return HOST_TYPE_STRING; // FIXME FIXME FIXME FIXME FIXME from configure script
}

int
limd_readShared (void)
{
    FILE *fp     = NULL;
    char *cp     = NULL;
    char *word   = NULL;
    char modelok = FALSE;
    char resok   = FALSE;
    char clsok   = FALSE;
    char indxok  = TRUE;
    char typeok  = FALSE;
    unsigned int lineNum = 0;
    char lsfile[MAX_FILENAME_LEN];
    const char filename = "lsf.shared";
    const char configCheckSum[] = "configCheckSum";
    const char readonly[] = "r";

    initResTable ();

    sprintf (lsfile, "%s/%s", limParams[LSF_CONFDIR].paramValue, filename); // FIXME FIXME FIXME FIXME the fuck is this about?

    if (configCheckSum (lsfile, &lsfSharedCkSum) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, configCheckSum);
        return -1;
    }
    fp = confOpen (lsfile, readonly );
    if (!fp) {
        /* catgets 5200 */
        ls_syslog (LOG_ERR, "catgets 5200: %s: Can't open configuration file <%s>: file pointer returns as NULL", __func__, lsfile);
        return -1;
    }

    for (;;) { // FIXME FIXME FIXME FIXME remove infinite for loop
        if ((cp = getBeginLine (fp, &lineNum)) == NULL) {
            FCLOSEUP (&fp);
            if (!modelok) {
                /* catgets 5201 */
                ls_syslog (LOG_ERR, "catgets 5201: %s: HostModel section missing or invalid in %s", __func__, lsfile);
            }
            if (!resok) {
                /* catgets 5202 */
                ls_syslog (LOG_ERR, "catgets 5202: %s: Warning: Resource section missing or invalid in %s", __func__, lsfile);
            }
            if (!typeok) {
                /* catgets 5203 */
                ls_syslog (LOG_ERR, "catgets 5203: %s: HostType section missing or invalid", lsfile);
            }
            if (!indxok) {
                /* catgets 5204 */
                ls_syslog (LOG_ERR, "catgets 5204: %s: Warning: attempt to define too many new indices", lsfile);
            }
            if (!clsok) {
                /* catgets 5205 */
                ls_syslog (LOG_ERR, "catgets 5205: %s: Cluster section missing or invalid", lsfile);
            }
            if (modelok && resok && clsok && typeok) {
                return 0;
            }
            else {
                return -1;
            }
        }

        word = getNextWord_ (&cp);
        if (!word) {
            /* catgets 5206 */
            ls_syslog (LOG_ERR, "catgets 5206: %s: %s(%d): Section name expected after Begin; ignoring section", __func__, lsfile, lineNum);
            lim_CheckError = WARNING_ERR;
            doSkipSection (fp, &lineNum, lsfile, "unknown");
            continue;
            }
        else
            {
            if (strcasecmp (word, "host") == 0)
                {
                /* catgets 5380 */
                ls_syslog (LOG_INFO, "catgets 5380: %s: %s(%d): section %s no longer needed in this version, ignored", "readShared", lsfile, lineNum, word);
                continue;
                }

            if (strcasecmp (word, "hosttype") == 0) {
                assert( lineNum <= INT_MAX );
                if (dotypelist (fp, &lineNum, lsfile)) {
                    typeok = TRUE;
                }
                continue;
            }

            if (strcasecmp (word, "hostmodel") == 0)
                {
                if (dohostmodel (fp, &lineNum, lsfile)) {
                    modelok = TRUE;
                }
                continue;
                }

            if (strcasecmp (word, "resource") == 0)
                {
                if (doresources (fp, &lineNum, lsfile)) {
                    resok = TRUE;
                }
                continue;
                }

            if (strcasecmp (word, "cluster") == 0)
                {
                if (doclist (fp, &lineNum, lsfile)) {
                    clsok = TRUE;
                }
                continue;
                }

            if (strcasecmp (word, "newindex") == 0)
                {
                if (!doindex (fp, &lineNum, lsfile)) {
                    indxok = FALSE;
                }
                continue;
                }

            /* catgets 5207 */
            ls_syslog (LOG_ERR, "catgets 5207: %s: %s(%d): Invalid section name %s; ignoring section", __func__, lsfile, lineNum, word);
            lim_CheckError = WARNING_ERR;
            doSkipSection (fp, &lineNum, lsfile, word);
            }
        }
}

char
doindex (FILE * fp, unsigned int *lineNum, char *lsfile)
{
    char *linep = NULL;

    enum {
        INTERVAL,
        INCREASING,
        DESCRIPTION,
        NAME
    };

    const char *keylist[] = {
        "INTERVAL",
        "INCREASING",
        "DESCRIPTION",
        "NAME",
        NULL
    };

    struct keymap keyList[] = {
        {  INTERVAL,    "    ", keylist[ INTERVAL ],    NULL },
        {  INCREASING,  "    ", keylist[ INCREASING ],  NULL },
        {  DESCRIPTION, "    ", keylist[ DESCRIPTION ], NULL },
        {  NAME,        "    ", keylist[ NAME ],        NULL },
        { -1,           "    ", NULL, NULL }
    };

    const char newinded[] = "newindex";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
        {
        /* catgets 5208 */
        ls_syslog (LOG_ERR, "5208: %s: %s(%d): Premature EOF", __func__, lsfile, *lineNum);
        lim_CheckError = WARNING_ERR;
        return TRUE;
        }

    if (isSectionEnd (linep, lsfile, lineNum, newindex ))
        {
        /* catgets 5209 */
        ls_syslog (LOG_WARNING, "5209, %s: %s(%d): empty section", __func__, lsfile, *lineNum);
        return TRUE;
        }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, TRUE))
            {
            /* catgets 5210 */
            ls_syslog (LOG_ERR, "5210: %s: %s(%d): keyword line format error for section newindex; ignoring section", __func__, lsfile, *lineNum);
            lim_CheckError = WARNING_ERR;
            doSkipSection (fp, lineNum, lsfile, newindex );
            return TRUE;
            }


        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, newindex )) {
                return TRUE;
            }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5211 */
                ls_syslog (LOG_ERR, "5211: %s: %s(%d): values do not match keys for section newindex; ignoring line", __func__, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (!setIndex (keyList, lsfile, *lineNum))
                {
                doSkipSection (fp, lineNum, lsfile, "newindex");
                freeKeyList (keyList);
                return FALSE;
                }
            freeKeyList (keyList);
            }
        }
    else {
        if (readHvalues (keyList, linep, fp, lsfile, lineNum, TRUE, newindex) < 0) {
            return TRUE;
        }
        if (!setIndex (keyList, lsfile, *lineNum)) {
            return FALSE;
        }
        return TRUE;
    }

    /* catgets 5212 */
    ls_syslog (LOG_ERR, "5212: %s: %s(%d): Premature EOF", __func__, lsfile, *lineNum);
    lim_CheckError = WARNING_ERR;
    return TRUE;

}

char
lim_setIndex (struct keymap *keyList, const char *lsfile, size_t linenum)
{
    // called by doindex( ) 

    int resIdx = 0;

    enum {
        INTERVAL,
        INCREASING,
        DESCRIPTION,
        NAME
    };


    if (strlen (keyList[NAME].val) >= MAX_LSF_NAME_LEN)
        {
            /* catgets 5213 */
            ls_syslog (LOG_ERR, "5213: %s: %s(%d): Name %s is too long (maximum is %d chars); ignoring index", __func__, lsfile, linenum, keyList[NAME].val, MAX_LSF_NAME_LEN - 1);
            lim_CheckError = WARNING_ERR;
            return TRUE;
        }

    if (strpbrk (keyList[NAME].val, ILLEGAL_CHARS) != NULL)
        {
            /* catgets 5214 */
            ls_syslog (LOG_ERR, "5214: %s: %s(%d): illegal character (one of %s)", __func__, lsfile, linenum, ILLEGAL_CHARS);
            lim_CheckError = WARNING_ERR;
            return TRUE;
        }
    if (IS_DIGIT (keyList[NAME].val[INTERVAL]))
        {
            /* catgets 5215 */
            ls_syslog (LOG_ERR, "5215: %s: %s(%d): Index name <%s> begun with a digit is illegal; ignored", __func__, lsfile, linenum, keyList[NAME].val);
            lim_CheckError = WARNING_ERR;
            return TRUE;
        }

    if ((resIdx = resNameDefined (keyList[NAME].val)) >= 0)
        {
        if (!(allInfo.resTable[resIdx].flags & RESF_DYNAMIC))
            {
                /* catgets 5216 */
                ls_syslog (LOG_ERR, "5216: %s: %s(%d): Name <%s> is not a dynamic resource; ignored", __func__, lsfile, linenum, keyList[NAME].val);
                return TRUE;
            }
        if ((allInfo.resTable[resIdx].flags & RESF_BUILTIN))
            /* catgets 5217 */
            ls_syslog (LOG_ERR, "5217: %s: %s(%d): Name <%s> reserved or previously defined;", __func__, lsfile, linenum, keyList[NAME].val);
        else
            {
            /* catgets 5219 */
            ls_syslog (LOG_ERR, "5219: %s: %s(%d): Name <%s> reserved or previously defined; ignoring" , __func__, lsfile, linenum, keyList[NAME].val);
            lim_CheckError = WARNING_ERR;
            return TRUE;
            }
        }
    else {
        resIdx = allInfo.nRes;
    }
    assert( sizeOfResTable <= INT_MAX );
    if (resIdx >= sizeOfResTable && doubleResTable (lsfile, linenum) < 0) {
        return FALSE;
    }

    initResItem (&allInfo.resTable[resIdx]);
    allInfo.resTable[resIdx].interval = atoi (keyList[INTERVAL].val);
    allInfo.resTable[resIdx].orderType = (strcasecmp (keyList[INCREASING].val, "y") == 0) ? INCR : DECR;

    strcpy (allInfo.resTable[resIdx].des, keyList[DESCRIPTION].val);
    strcpy (allInfo.resTable[resIdx].name, keyList[NAME].val);
    allInfo.resTable[resIdx].valueType = LS_NUMERIC;
    allInfo.resTable[resIdx].flags = RESF_DYNAMIC | RESF_GLOBAL;

    if (allInfo.numUsrIndx + NBUILTINDEX >= li_len - 1) {
        li_len <<= 1;
        li = realloc (li, li_len * sizeof (struct liStruct));
        if ( NULL == li && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
            return TRUE;
        }
    }

    assert( allInfo.nRes <= INT_MAX );
    if (resIdx == (int)allInfo.nRes) {

        li[NBUILTINDEX + allInfo.numUsrIndx].increasing = (strcasecmp (keyList[INCREASING].val, "y") == 0);
        if ((li[NBUILTINDEX + allInfo.numUsrIndx].name = putstr_ (keyList[NAME].val)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
            return TRUE;
        }
        allInfo.numUsrIndx++;
        allInfo.numIndx++;
        allInfo.nRes++;
    }
    else {

        for (unsigned int i = 0; i < NBUILTINDEX + allInfo.numUsrIndx; i++) {
            if (!strcasecmp (keyList[NAME].val, li[i].name)) {
                li[i].increasing = (strcasecmp (keyList[INCREASING].val, "y") == 0);
                break;
            }
        }
    }

    defaultRunElim = TRUE;

    return TRUE;
}

char
doclist (FILE * fp, unsigned int *lineNum, const char *lsfile)
{
    char *linep   = NULL;
    char *servers = NULL;
    bool_t clusterAdded = FALSE;

    enum {
        CLUSTERNAME = 0,
        SERVERS
    };

    const char *keylist[ ] = {
        "CLUSTERNAME",
        "SERVERS",
        NULL
    };

    struct keymap keyList[] = {
        { CLUSTERNAME, "    ", keylist[ CLUSTERNAME ], NULL },
        { SERVERS,     "    ", keylist[ SERVERS ],     NULL },
        { -1,          "    ", NULL, NULL }
    };

    const char cluster[] = "cluster";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
        {
        /* catgets 5222 */
        ls_syslog (LOG_ERR, "5222: %s: %s(%d): section cluster: Premature EOF", __func__, lsfile, *lineNum);
        return FALSE;
        }

    if (isSectionEnd (linep, lsfile, lineNum, cluster )) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, FALSE))
            {
            /* catgets 5223 */
            ls_syslog (LOG_ERR, "5223: %s: %s(%d): keyword line format error for section cluster; ignoring section", __func__, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, cluster );
            return FALSE;
            }

        if (keyList[CLUSTERNAME].position == -1) {
            /* catgets 5224 */
            ls_syslog (LOG_ERR, "5224: %s: %s(%d): keyword line: key %s is missing in section cluster; ignoring section", __func__, lsfile, *lineNum, keyList[CLUSTERNAME].key);
            doSkipSection (fp, lineNum, lsfile, cluster );
            return FALSE;
        }


        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, cluster )) {
                return TRUE;
            }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5225 */
                ls_syslog (LOG_ERR, "5225: %s: %s(%d): values do not match keys for section cluster, ignoring line",  __func__, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (keyList[SERVERS].position != -1)
                {
                servers = keyList[SERVERS].val;
                mcServersSet = TRUE;
                }
            else {
                servers = NULL;
            }

            if (!clusterAdded && !addCluster (keyList[CLUSTERNAME].val, servers))
                {
                /* catgets 5226 */
                ls_syslog (LOG_ERR, "5226: %s: Ignoring cluster %s", __func__, keyList[CLUSTERNAME].val);
                lim_CheckError = WARNING_ERR;
                }
            else if (clusterAdded)
                {
                /* catgets 5226 */
                ls_syslog (LOG_ERR, "5226: %s: Ignoring cluster %s", __func__, keyList[CLUSTERNAME].val);
                lim_CheckError = WARNING_ERR;
                }

            FREEUP (keyList[CLUSTERNAME].val);
            if (keyList[SERVERS].position != -1) {
                FREEUP (keyList[SERVERS].val);
            }
            clusterAdded = TRUE;
        }
    }
    else
        {
        /* catgets 5227 */
        ls_syslog (LOG_ERR, "catgets 5227: %s: %s(%d): horizontal cluster section not implemented yet, ignoring section", __func__, lsfile, *lineNum);
        doSkipSection (fp, lineNum, lsfile, cluster );
        return FALSE;
        }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, cluster ); /*catgets33 */
    return FALSE;
}

char
dotypelist (FILE * fp, unsigned int *lineNum, const char *lsfile)
{
    char *linep = NULL;

    enum {
        TYPENAME = 0,
        FIXME_FIXME_FIXME_FIXME = 1
    };

    const char *keylist [ ] = {
        "TYPENAME",
        "FIXME_FIXME_FIXME_FIXME"
        NULL
    };

    struct keymap keyList[] = {
        { TYPENAME,                "    ", keylist[ TYPENAME ],                NULL },
        { FIXME_FIXME_FIXME_FIXME, "    ", keylist[ FIXME_FIXME_FIXME_FIXME ], NULL },
        { -1,                      "    ", NULL, NULL }
    };

    const char HostType[] = "HostType";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
        {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, HostType); /*catgets33 */
        return FALSE;
        }

    if (isSectionEnd (linep, lsfile, lineNum, HostType )) {
        return FALSE;
    }

    if (allInfo.nTypes <= 0)
        {
        allInfo.nTypes = 2;
        }

    if (shortInfo.nTypes <= 0)
        {
        shortInfo.nTypes = 2;
        }

    strcpy (allInfo.hostTypes[TYPENAME], "UNKNOWN_AUTO_DETECT");                                // FIXME FIXME FIXME FIXME where is 0 from ?
    shortInfo.hostTypes[TYPENAME] = allInfo.hostTypes[TYPENAME];                                // FIXME FIXME FIXME FIXME where is 0 from ?
    strcpy (allInfo.hostTypes[FIXME_FIXME_FIXME_FIXME], "DEFAULT");                             // FIXME FIXME FIXME FIXME where is 1 from ?
    shortInfo.hostTypes[FIXME_FIXME_FIXME_FIXME] = allInfo.hostTypes[FIXME_FIXME_FIXME_FIXME];  // FIXME FIXME FIXME FIXME where is 1 from ?

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, TRUE))
            {
            /* catgets 5230 */
            ls_syslog (LOG_ERR, "5230: %s: %s(%d): keyword line format error for section HostType, ignoring section", __func__, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, HostType);
            return FALSE;
            }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, HostType))
                {
                return TRUE;
                }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5231 */
                ls_syslog (LOG_ERR, "5231: %s: %s(%d): values do not match keys for section cluster, ignoring line", __func__, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (strpbrk (keyList[TYPENAME].val, ILLEGAL_CHARS) != NULL)
                {
                /* catgets 5232 */
                ls_syslog (LOG_ERR, "5232: %s: %s(%d): illegal character (one of %s), ignoring type %s", __func__, lsfile, *lineNum, ILLEGAL_CHARS, keyList[TYPENAME].val);
                lim_CheckError = WARNING_ERR;
                FREEUP (keyList[TYPENAME].val);
                continue;
                }
            if (IS_DIGIT (keyList[TYPENAME].val[TYPENAME])) // FIXME FIXME FIXME FIXME
                {
                /* catgets 5233 */
                ls_syslog (LOG_ERR, "5233: %s: %s(%d): Type name <%s> begun with a digit is illegal; ignored", __func__, lsfile, *lineNum, keyList[TYPENAME].val);
                lim_CheckError = WARNING_ERR;
                FREEUP (keyList[TYPENAME].val);
                continue;
                }
            if (!addHostType (keyList[TYPENAME].val)) {
                lim_CheckError = WARNING_ERR;
            }

            FREEUP (keyList[TYPENAME].val);
            }
        }
    else
        {
        /* catgets 5234 */
        ls_syslog (LOG_ERR, "5234: %s: %s(%d): horizontal HostType section not implemented yet, ignoring section", __func__, lsfile, *lineNum);
        doSkipSection (fp, lineNum, lsfile, HostType );
        return FALSE;
        }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, HostType ); /*catgets33 */

    return FALSE;
}

char
dohostmodel (FILE * fp, unsigned int *lineNum, const char *lsfile)
{
    char *linep = NULL;
    char *sp    = NULL;
    char *word  = NULL;
    char first  = TRUE;
    float *floatp = 0.0f;
    int new = 0;
    struct hEnt *hashEntPtr = NULL;

    enum {
        MODELNAME,
        CPUFACTOR,
        ARCHITECTURE
    };

    const char *keylist [ ] = {
        "MODELNAME",
        "CPUFACTOR",
        "ARCHITECTURE",
        NULL
    };

    struct keymap keyList[] = {
        { MODELNAME,    "    ", keylist[MODELNAME],    NULL },
        { CPUFACTOR,    "    ", keylist[CPUFACTOR],    NULL },
        { ARCHITECTURE, "    ", keylist[ARCHITECTURE], NULL },
        { -1,           "    ", NULL, NULL}
    };

    const char HostModel[] = "HostModel";

    if (first) {
        for ( int i = 0; i < MAX_MODELS; ++i) {
            allInfo.cpuFactor[i] = 1.0;
            allInfo.modelRefs[i] = 0;
        }

        h_initTab_ (&hostModelTbl, 11);
        first = FALSE;
    }

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, HostModel); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, lsfile, lineNum, HostModel )) {
        return FALSE;
    }

    if (allInfo.nModels <= 0)
        {
        memset (allInfo.modelRefs, 0, sizeof (int) * MAX_MODELS);
        allInfo.nModels = 2;
        }
    if (shortInfo.nModels <= 0)
        {
        shortInfo.nModels = 2;
        }
 
    strcpy (allInfo.hostModels[0], "UNKNOWN_AUTO_DETECT"); // FIXME FIXME FIXME FIXME where is 0 from ?
    strcpy (allInfo.hostArchs[0], "UNKNOWN_AUTO_DETECT");  // FIXME FIXME FIXME FIXME where is 0 from ?
    allInfo.cpuFactor[0] = 1;                              // FIXME FIXME FIXME FIXME where is 0 from ?
    shortInfo.hostModels[0] = allInfo.hostModels[0];       // FIXME FIXME FIXME FIXME where is 0 from ?
    strcpy (allInfo.hostModels[1], "DEFAULT");             // FIXME FIXME FIXME FIXME where is 1 from ?
    strcpy (allInfo.hostArchs[1], "");                     // FIXME FIXME FIXME FIXME where is 1 from ?
    allInfo.cpuFactor[1] = 1;                              // FIXME FIXME FIXME FIXME where is 1 from ?
    shortInfo.hostModels[1] = allInfo.hostModels[1];       // FIXME FIXME FIXME FIXME where is 1 from ?

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, FALSE))
            {
            /* catgets 5237 */
            ls_syslog (LOG_ERR, "5237: %s: %s(%d): keyword line format error for section hostmodel, ignoring section", __func__, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, __func__ );
            return FALSE;
            }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, HostModel ))
                {
                return TRUE;
                }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5238 */
                ls_syslog (LOG_ERR, "5238: %s: %s(%d): values do not match keys for section hostmodel, ignoring line", __func__, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (!isanumber_ (keyList[CPUFACTOR].val) || atof (keyList[CPUFACTOR].val) <= 0)
                {
                /* catgets 5239 */
                ls_syslog (LOG_ERR, "5239: %s: %s(%d): Bad cpuFactor for host model %s, ignoring line", __func__, lsfile, *lineNum, keyList[MODELNAME].val);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (strpbrk (keyList[MODELNAME].val, ILLEGAL_CHARS) != NULL)
                {
                /* catgets 5240 */
                ls_syslog (LOG_ERR, "5240: %s: %s(%d): illegal character (one of %s), ignoring model %s", __func__, lsfile, *lineNum, ILLEGAL_CHARS, keyList[MODELNAME].val);
                lim_CheckError = WARNING_ERR;
                continue;
                }
            if (IS_DIGIT (keyList[MODELNAME].val[0]))
                {
                /* catgets 5241 */
                ls_syslog (LOG_ERR, "5241: %s: %s(%d): Model name <%s> begun with a digit is illegal; ignored", __func__, lsfile, *lineNum, keyList[MODELNAME].val);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            sp = keyList[ARCHITECTURE].val;
            if (sp && sp[0])
                {
                while ((word = getNextWord_ (&sp)) != NULL)
                    {
                    if (!addHostModel (keyList[MODELNAME].val, word, atof (keyList[CPUFACTOR].val)))
                        {
                        ls_syslog (LOG_ERR, "5242: %s: %s(%d): Too many host models, ignoring model %s", __func__, lsfile, *lineNum, keyList[MODELNAME].val);    /* catgets 5242 */
                        lim_CheckError = WARNING_ERR;
                        goto next_value;
                        }
                    }
                }
            else
                {
                if (!addHostModel (keyList[MODELNAME].val, NULL, atof (keyList[CPUFACTOR].val)))
                    {
                    /* catgets 5242 */
                    ls_syslog (LOG_ERR, "5242: %s: %s(%d): Too many host models, ignoring model %s", __func__, lsfile, *lineNum, keyList[MODELNAME].val);
                    lim_CheckError = WARNING_ERR;
                    goto next_value;
                    }

                }

            hashEntPtr = h_addEnt_ (&hostModelTbl, keyList[MODELNAME].val, &new);
            if (new)
                {
                floatp = malloc (sizeof (float));
                if (floatp == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (float));
                    doSkipSection (fp, lineNum, lsfile, HostModel );
                    return FALSE;
                    }
                *floatp = (float)atof (keyList[CPUFACTOR].val);
                hashEntPtr->hData = floatp;
                }
            else
                {
                floatp = hashEntPtr->hData;
                *floatp = (float)atof (keyList[CPUFACTOR].val);
                hashEntPtr->hData = floatp;
                }

            next_value:
            }
        }
    else
        {
        /* catgets 5244 */
        ls_syslog (LOG_ERR, "5244: %s: %s(%d): horizontal HostModel section not implemented yet, ignoring section", __func__, lsfile, *lineNum);
        doSkipSection (fp, lineNum, lsfile, HostModel);
        return FALSE;
        }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, HostModel ); /*catgets33 */
    return FALSE;
}

void
initResTable (void)
{
    struct resItem *resTable;
    unsigned int i = 0;

    /* 300 like the spartans agains
     * the persians...
     */
    resTable = calloc (300, sizeof (struct resItem));
    sizeOfResTable = 300;
    allInfo.numIndx = 0;
    allInfo.numUsrIndx = 0;

    while (builtInRes[i].name) {

        strcpy (resTable[i].name, builtInRes[i].name);
        strcpy (resTable[i].des, builtInRes[i].des);
        resTable[i].valueType = builtInRes[i].valuetype;
        resTable[i].orderType = builtInRes[i].ordertype;
        resTable[i].interval  = builtInRes[i].interval;
        resTable[i].flags     = builtInRes[i].flags;

        if ((resTable[i].flags & RESF_DYNAMIC) && (resTable[i].valueType == LS_NUMERIC)) {
            allInfo.numIndx++;
        }

        i++;
    }

    allInfo.nRes = i;
    allInfo.resTable = resTable;

    return;
}

//
// int resNameDefined ( const char *name) { } lived here . now lives in lsf/lib/liblsf/conf.c . Requiescat in Pace
// 


char
doresources (FILE * fp, unsigned int *lineNum, char *lsfile) // FIXME FIXME FIXME FIXME FIXME duplicated function from lsf/lib/liblsf/conf.c . Must de-dup.
{

    int nres = 0;
    int resIdx = 0;
    char *linep = NULL;

    enum {
        RESOURCENAME = 0,
        TYPE,
        INTERVAL,
        INCREASING,
        RELEASE,
        DESCRIPTION,
        NULLKEY
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
        { RESOURCENAME, "    " , keylist[RESOURCENAME], NULL },
        { TYPE,         "    " , keylist[TYPE],         NULL },
        { INTERVAL,     "    " , keylist[INTERVAL],     NULL },
        { INCREASING,   "    " , keylist[INCREASING],   NULL },
        { RELEASE,      "    " , keylist[RELEASE],      NULL },
        { DESCRIPTION,  "    " , keylist[DESCRIPTION],  NULL },
        { KEYNULL,      "    " , NULL, NULL}
    };

    const char resource[] = "resource";


    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, resource ); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, lsfile, lineNum, resource ) ) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, FALSE)) {
            /* catgets 5248 */
            ls_syslog (LOG_ERR, "5248: %s: %s(%d): keyword line format error for section resource, ignoring section", __func__, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, resource);
            return FALSE;
        }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {

            if (isSectionEnd (linep, lsfile, lineNum, resource )) {
                return TRUE;
            }

            if (mapValues (keyList, linep) < 0) {
                /* catgets 5249 */
                ls_syslog (LOG_ERR, "5249: %s: %s(%d): values do not match keys for section resource, ignoring line", __func__, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
            }

            if (strlen (keyList[RKEY_RESOURCENAME].val) >= MAX_LSF_NAME_LEN - 1) {
                /* catgets 5250 */
                ls_syslog (LOG_ERR, "5250: %s: %s(%d): Resource name %s too long in section resource. Should be less than %d characters. Ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val, MAX_LSF_NAME_LEN - 1);
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
            }

            if ((resIdx = resNameDefined (keyList[RKEY_RESOURCENAME].val)) >= 0)  {
                if ((allInfo.resTable[resIdx].flags & RESF_BUILTIN) && (allInfo.resTable[resIdx].flags & RESF_DYNAMIC)) {

                    if (    keyList[RKEY_TYPE].val && *keyList[RKEY_TYPE].val && 
                            allInfo.resTable[resIdx].valueType == validType (keyList[RKEY_TYPE].val) && 
                            allInfo.resTable[resIdx].orderType == !strcasecmp (keyList[RKEY_INCREASING].val, "N")
                        ) 
                    {
                        allInfo.resTable[resIdx].flags &= ~RESF_BUILTIN;
                    }
                    else {
                        /* catgets 5251 */
                        ls_syslog (LOG_ERR, "5251: %s: %s(%d): Built-in resource %s can't be overrided with different type or increasing. Ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val);
                        lim_CheckError = WARNING_ERR;
                    }
                }
                else {
                    /* catgets 5251 */
                    ls_syslog (LOG_ERR, "5251: %s: %s(%d): Resource name %s reserved or previously defined. Ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                }
                freeKeyList (keyList);
                continue;
                }

            if (strpbrk (keyList[RKEY_RESOURCENAME].val, ILLEGAL_CHARS) != NULL) {
                /* catgets 5252 */
                ls_syslog (LOG_ERR, "5252: %s: %s(%d): illegal character (one of %s): in resource name:%s, section resource, ignoring line", __func__, lsfile, *lineNum, ILLEGAL_CHARS, keyList[RESOURCENAME].val);
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
            }
            if (IS_DIGIT (keyList[RKEY_RESOURCENAME].val[0])) {
                /* catgets 5253 */
                ls_syslog (LOG_ERR, "5253: %s: %s(%d): Resource name <%s> begun with a digit is illegal; ignored", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val); 
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
            }
            if (allInfo.nRes >= sizeOfResTable  && doubleResTable (lsfile, *lineNum) < 0) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "doubleResTable");
                return FALSE;
            }
            initResItem (&allInfo.resTable[allInfo.nRes]);
            strcpy (allInfo.resTable[allInfo.nRes].name, keyList[RKEY_RESOURCENAME].val);


            if ( keyList[RKEY_TYPE].val != NULL && ) {

                int type = 0;

                if (strcmp (keyList[RKEY_TYPE].val, LSF_LIM_ERES_TYPE) == 0)
                    {
                    if (setExtResourcesDef (keyList[RKEY_RESOURCENAME].val) != 0) {
                        /* catgets 5256 */
                        ls_syslog (LOG_ERR, "5256: %s: Ignoring the external resource <%s>(%d) in section resource of file %s", __func__, keyList[RKEY_RESOURCENAME].val, *lineNum, lsfile);
                        lim_CheckError = WARNING_ERR;
                        freeKeyList (keyList);
                        continue;
                    }
                    allInfo.nRes++;
                    nres++;
                    freeKeyList (keyList);
                    continue;
                }
                if ((type = validType (keyList[RKEY_TYPE].val)) >= 0) {
                    assert( type >= 0);
                    allInfo.resTable[allInfo.nRes].valueType = (unsigned int)type;
                }
                else {
                    /* catgets 5257 */
                    ls_syslog (LOG_ERR, "5257: %s: %s(%d): resource type <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource", __func__, lsfile, *lineNum, keyList[RKEY_TYPE].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                }
            }
            else {
                if (logclass & LC_TRACE) {
                    ls_syslog (LOG_DEBUG3, "doresources: %s(%d): Resource type is not defined for resource <%s>; The resource will be assigned type <boolean>", lsfile, *lineNum, keyList[RKEY_RESOURCENAME].val);
                }
                allInfo.resTable[allInfo.nRes].valueType = LS_BOOLEAN;
            }


            if (keyList[RKEY_INTERVAL].val != NULL && keyList[RKEY_INTERVAL].val[0] != '\0') {
                int interval = 0;
                if ((interval = atoi (keyList[RKEY_INTERVAL].val)) > 0) {
                    allInfo.resTable[allInfo.nRes].interval = interval;
                    allInfo.resTable[allInfo.nRes].flags |= RESF_DYNAMIC;
                }
                else {
                    /* catgets 5258 */
                    ls_syslog (LOG_ERR, "5258: %s: %s(%d): INTERVAL <%s> for resource <%s> should be a integer greater than 0; ignoring resource <%s> in section resource", __func__, lsfile, *lineNum, keyList[RKEY_INTERVAL].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                    }
                }

            if( keyList[RKEY_INCREASING].val != NULL ) {
                if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC) {
                    if (!strcasecmp (keyList[RKEY_INCREASING].val, "N")) {
                        allInfo.resTable[allInfo.nRes].orderType = DECR;
                    }
                    else if (!strcasecmp (keyList[RKEY_INCREASING].val, "Y")) {
                        allInfo.resTable[allInfo.nRes].orderType = INCR;
                    }
                    else
                        {
                        /* catgets 5259 */
                        ls_syslog (LOG_ERR, "5259: %s: %s(%d): INCREASING <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource", __func__, lsfile, *lineNum, keyList[RKEY_INCREASING].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                        lim_CheckError = WARNING_ERR;
                        freeKeyList (keyList);
                        continue;
                        }
                    }
                else
                    /* catgets 5260 */
                    ls_syslog (LOG_ERR, "5260: %s: %s(%d): INCREASING <%s> is not used by the resource <%s> with type <%s>; ignoring INCREASING", __func__, lsfile, *lineNum, keyList[RKEY_INCREASING].val, keyList[RKEY_RESOURCENAME].val, (allInfo.resTable[allInfo.nRes].orderType == LS_BOOLEAN) ? "BOOLEAN" : "STRING");
                }
            else
                {
                if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC) {
                    ls_syslog (LOG_ERR, "5261: %s: %s(%d): No INCREASING specified for a numeric resource <%s>; ignoring resource <%s> in section resource", __func__, lsfile, *lineNum, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                }
            }

            if( keyList[RKEY_RELEASE].val != NULL )
                {
                if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC)
                    {
                    if (!strcasecmp (keyList[RKEY_RELEASE].val, "Y"))
                        {
                        allInfo.resTable[allInfo.nRes].flags |= RESF_RELEASE;
                        }
                    else if (strcasecmp (keyList[RKEY_RELEASE].val, "N"))
                        {
                          /*catgets 5474 */
                        ls_syslog (LOG_ERR, "5474: doresources:%s(%d): RELEASE defined for resource <%s> should be 'Y', 'y', 'N' or 'n' not <%s>; ignoring resource <%s> in section resource",  lsfile, *lineNum, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RELEASE].val, keyList[RKEY_RESOURCENAME].val);
                        lim_CheckError = WARNING_ERR;
                        freeKeyList (keyList);
                        continue;
                        }
                    }
                else
                    {
                    /*catgets 5475 */
                    ls_syslog (LOG_ERR, "5475: doresources:%s(%d): RELEASE cannot be defined for resource <%s> which isn't a numeric resource; ignoring resource <%s> in section resource", lsfile, *lineNum, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                    }
                }
            else
                {

                if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC)
                    {
                    allInfo.resTable[allInfo.nRes].flags |= RESF_RELEASE;
                    }
                }

            strncpy (allInfo.resTable[allInfo.nRes].des,
                     keyList[RKEY_DESCRIPTION].val, MAXRESDESLEN);

            if (allInfo.resTable[allInfo.nRes].interval > 0
                && (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC))
                {

                if (allInfo.numUsrIndx + NBUILTINDEX >= li_len - 1)
                    {
                    li_len *= 2;
                    if (!(li = realloc (li, li_len * sizeof (struct liStruct))))
                        {
                        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", li_len * sizeof (struct liStruct));
                        return FALSE;
                        }
                    }
                if ((li[NBUILTINDEX + allInfo.numUsrIndx].name =
                     putstr_ (allInfo.resTable[allInfo.nRes].name)) == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (allInfo.resTable[allInfo.nRes].name));
                    return FALSE;
                    }

                li[NBUILTINDEX + allInfo.numUsrIndx].increasing =
                (allInfo.resTable[allInfo.nRes].orderType == INCR) ? 1 : 0;
                allInfo.numUsrIndx++;
                allInfo.numIndx++;
                }
            allInfo.nRes++;
            nres++;
            freeKeyList (keyList);
            }
        }
    else
        {
        ls_syslog (LOG_ERR, "5264: %s: %s(%d): horizontal resource section not implemented yet", __func__, lsfile, *lineNum);    /* catgets 5264 */
        return FALSE;
        }


    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, resource ); /*catgets33 */
    return FALSE;

}

void
chkUIdxAndSetDfltRunElim (void)
{
    if (defaultRunElim == FALSE && allInfo.numUsrIndx > 0) {

        for ( unsigned int i = NBUILTINDEX; i < allInfo.numIndx; i++) {
            if (allInfo.resTable[i].flags & (RESF_DYNAMIC | RESF_GLOBAL)) {
                if (allInfo.resTable[i].flags & RESF_DEFINED_IN_RESOURCEMAP) {
                    defaultRunElim = TRUE;
                    break;
                }
            }
        }
    }

    return;
}

int
doresourcemap (FILE * fp, const char *lsfile, unsigned int *lineNum)
{
    int isDefault = 0;
    char *linep = NULL;
    int resNo = 0;
    unsigned int cc = 0;

    enum {
        RESOURCENAME,
        LOCATION
    };

    const char *keylist[] = {
        "RESOURCENAME",
        "LOCATION",
        NULL
    };



    struct keymap keyList[] = {
        { RESOURCENAME, "    ", keylist[RESOURCENAME], NULL },
        { LOCATION,     "    ", keylist[LOCATION],     NULL },
        { -1,           "    ", NULL, NULL}
    };

    const char resourceMap[] = "resourceMap";

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, resourceMap); /*catgets33 */
        return -1;
    }

    if (isSectionEnd (linep, lsfile, lineNum, resourceMap)) {
        /* catgets 5477 */
        ls_syslog (LOG_WARNING, "5477: %s: %s(%d): Empty resourceMap, no keywords or resources defined.", __func__, lsfile, *lineNum);
        return -1;
    }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, TRUE))
            {
            /* catgets 5267 */
            ls_syslog (LOG_ERR, "5267: %s: %s(%d): keyword line format error for section resource, ignoring section", __func__, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, resourceMap );
            return -1;
            }


        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, resourceMap ) )
                {
                return 0;
                }

            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5268 */
                ls_syslog (LOG_ERR, "5268: %s: %s(%d): values do not match keys for resourceMap section, ignoring line", __func__, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if ((resNo = resNameDefined (keyList[RESOURCENAME].val)) < 0)
                {
                /* catgets 5269 */
                ls_syslog (LOG_ERR, "5269: %s: %s(%d): Resource name <%s> is  not defined; ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val);
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
                }
            else
                {

                if (resNo < NBUILTINDEX) {
                    /* catgets 5296 */
                    ls_syslog (LOG_ERR, "5296: %s: %s(%d): Built-in resource %s can't be redefined as shared resource here. Ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val);
                    continue;
                }
            }

            if (keyList[RKEY_LOCATION].val != NULL
                && strcmp (keyList[RKEY_LOCATION].val, LSF_LIM_ERES_TYPE) == 0)
                {
                if (setExtResourcesLoc (keyList[RKEY_RESOURCENAME].val, resNo) != 0)
                    {
                    /* catgets 5270 */
                    ls_syslog (LOG_ERR, "5270: %s: Ignoring the external resource location <%s>(%d) in section resourceMap of file %s", __func__, keyList[RKEY_RESOURCENAME].val, *lineNum, lsfile);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                    }
                freeKeyList (keyList);
                continue;
                }

            if (keyList[LOCATION].val != NULL && keyList[LOCATION].val[0] != '\0')
                {

                if (strstr (keyList[RKEY_LOCATION].val, "all ") &&
                    strchr (keyList[RKEY_LOCATION].val, '~'))
                    {

                    struct HostsArray array;
                    struct hostNode *hPtr;
                    int result;

                    array.size = 0;
                    array.hosts = malloc (numofhosts * sizeof (char *));
                    if (!array.hosts)
                        {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                        return -1;
                        }
                    for (hPtr = myClusterPtr->hostList; hPtr; hPtr = hPtr->nextPtr)
                        {

                        array.hosts[array.size] = strdup (hPtr->hostName);
                        if (!array.hosts[array.size]) {
                            for (cc = 0; cc < array.size; cc++) {
                                FREEUP (array.hosts[cc]);
                            }
                            FREEUP (array.hosts);
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            return -1;
                        }
                        array.size++;
                    }

                    result = convertNegNotation_ (&(keyList[LOCATION].val), &array);
                    if (result == 0)
                        {
                        /* catgets 5397 */
                        ls_syslog (LOG_WARNING, "5397: %s: %s(%d): convertNegNotation_: all the hosts are to be excluded %s !", __func__, lsfile, *lineNum, keyList[LOCATION].val);
                        }
                    else if (result < 0)
                        {
                        ls_syslog (LOG_WARNING, "5398: %s: %s(%d): convertNegNotation_: Wrong syntax \'%s\'", __func__, lsfile, *lineNum, keyList[LOCATION].val);
                        }
                    for (cc = 0; cc < array.size; cc++){
                        FREEUP (array.hosts[cc]);
                    }
                    FREEUP (array.hosts);
                    }

                if (addResourceMap (keyList[RESOURCENAME].val, keyList[LOCATION].val, lsfile, *lineNum, &isDefault) < 0)
                    {
                    /* catgets 5271 */
                    ls_syslog (LOG_ERR, "5271: %s: %s(%d): addResourceMap() failed for resource <%s>; ignoring line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                    }

                if (!(allInfo.resTable[resNo].flags & RESF_BUILTIN))
                    {
                    allInfo.resTable[resNo].flags
                    |= RESF_DEFINED_IN_RESOURCEMAP;
                    }

                if (!(isDefault &&
                      (allInfo.resTable[resNo].flags & RESF_DYNAMIC) &&
                      (allInfo.resTable[resNo].valueType == LS_NUMERIC)))
                    {

                    allInfo.resTable[resNo].flags &= ~RESF_GLOBAL;
                    allInfo.resTable[resNo].flags |= RESF_SHARED;
                    }

                resNo = 0;
                }
            else
                {
                /* catgets 5272 */
                ls_syslog (LOG_ERR, "5272: %s: %s(%d): No LOCATION specified for resource <%s>; ignoring the line", __func__, lsfile, *lineNum, keyList[RESOURCENAME].val);
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
                }
            freeKeyList (keyList);
            }
        }
    else
        {
        /* catgets 5273 */
        ls_syslog (LOG_ERR, "5273: %s: %s(%d): horizontal resource section not implemented yet", __func__, lsfile, *lineNum); 
        return -1;
        }

    return 0;
}


int
addSharedResourceInstance (unsigned int nHosts, char **hosts, const char *resName)
{
    int resNo             = 0;
    int firstFlag         = 1;
    unsigned int cnt      = 0;
    struct hostNode *hPtr = NULL;
    struct sharedResourceInstance *tmp = NULL;

    const char malloc[] = "malloc";

    if ((resNo = resNameDefined (resName)) < 0) {
         /* catgets 5274 */
        ls_syslog (LOG_ERR, "5274: %s: Resource name <%s> not defined", __func__, resName);
        return -1;
    }

    if (!(allInfo.resTable[resNo].flags & RESF_DYNAMIC)) {
        return 0;
    }

    tmp = (sharedResourceInstance *) malloc (sizeof (struct sharedResourceInstance));
    if (tmp == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, malloc, sizeof (struct sharedResourceInstance));
        return -1;
    }
    else {
        tmp->nextPtr = NULL;
        tmp->resName = putstr_ (resName);
        tmp->hosts = malloc (nHosts * sizeof (struct hostNode *));
        if (tmp->hosts == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, malloca, nHosts * sizeof (struct hostNode));
            return -1;
        }
        else {
            cnt = 0;
            for ( unsigned int i = 0; i < nHosts; i++) {
                if ((hPtr = findHostbyList (myClusterPtr->hostList, hosts[i])) != NULL) {
                    tmp->hosts[cnt++] = hPtr;
                }
            }
            tmp->nHosts = cnt;
        }
        if (firstFlag) {
            firstFlag = 0;
            sharedResourceHead = tmp;
        }
        else {
            tmp->nextPtr = sharedResourceHead;
            sharedResourceHead = tmp;
        }
    }

    if (logclass & LC_ELIM) {
        char str[256];
        cnt = 0;
        for (tmp = sharedResourceHead; tmp; tmp = tmp->nextPtr) {
            sprintf (str, "%d %s: ", cnt++, tmp->resName);
            for (unsigned int i = 0; i < tmp->nHosts; i++) {
                sprintf (str, "%s %s ", str, tmp->hosts[i]->hostName);
            }
            ls_syslog (LOG_DEBUG, "%s", str);
        }
    }
    return 1;
}


// lsf/daemons/limd/conf.c:int addResourceMap ( const char *resName, const char *location, const char *lsfile, size_t lineNum, int *isDefault)
//      used to be here. Moved over to lsf/lib/liblsf/addResourceMap.c


// unsigned int
// parseHostList ( const char *hostList, const char *lsfile, unsigned int lineNum, char ***hosts, int *hasDefault)
// {
//  char *host       = NULL;
//  char *sp         = NULL;
//  char *hostName   = NULL;
//  char **hostTable = NULL;
//  unsigned int numHosts = 0;

//  if (hostList == NULL) {
//      return 0;
//  }

//  sp = hostList;
//  while ((host = getNextWord_ (&sp)) != NULL) {
//      numHosts++;
//  }
//  hostTable = (char **) calloc (numHosts, sizeof (char *));
//  if (NULL == hostTable && ENOMEM == errno ) {
//      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
//      return 0;
//  }
//  sp = hostList;
//  numHosts = 0;
//  while ((host = getNextWord_ (&sp)) != NULL) {

//      if ((hostName = validLocationHost (host)) == NULL) {
//          /* catgets 5278 */
//          ls_syslog (LOG_ERR, "5278: %s: %s(%d): Invalid hostname <%s>;ignoring the host", __func__, lsfile, lineNum, host);
//          lim_CheckError = WARNING_ERR;
//          continue;
//      }

//      if ((hostTable[numHosts] = putstr_ (hostName)) == NULL) {
//          ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
//          for (unsigned int i = 0; i < numHosts; i++) {
//              FREEUP (hostTable[i]);
//          }
//          FREEUP (hostTable);
//          return 0;
//      }

//      if (!strcmp (hostName, "default")) {
//          *hasDefault = TRUE;
//      }
//      numHosts++;
//  }

//  if (numHosts == 0) {
//      FREEUP (hostTable);
//      return 0;
//  }

//  *hosts = hostTable;
//  return numHosts;
// }

char *
validLocationHost ( const char *hostName)
{
    int num = 0;

    if (!strcmp (hostName, "default") || !strcmp (hostName, "others") || !strcmp (hostName, "all")) {
        return hostName;
    }

    if (Gethostbyname_ (hostName)) {

        if (findHostbyList (myClusterPtr->hostList, hostName)) {
            return hostName;
        }

        ls_syslog (LOG_ERR, "%s: Host %s is not used by cluster %s; ignored", __func__, hostName, myClusterName);
        return NULL;
        }

    if ((num = typeNameToNo (hostName)) > 0) {
        return hostName;
    }

    if ((num = modelNameToNo (hostName)) > 0) {
        return hostName;
    }

    return NULL;
}

void
initResItem (struct resItem *resTable)
{
    if (resTable == NULL) {
        return;
    }

    resTable->name      = NULL;
    resTable->de        = NULL;
    resTable->valueType = 0;
    resTable->orderType = NA;
    resTable->flags = RESF_GLOBAL;
    resTable->interval = 0;

    return;
}

int
validType (char *type)
{
    if (type == NULL) {
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

    return -1;
}

int
readCluster (int checkMode)  // FIXME FIXME move to a cluster.c file
{
    char *hname = NULL;
    unsigned int counter = 0;
    const char readCluster[] = __func__

    if (!myClusterPtr) {
        /* catgets 5279 */
        ls_syslog (LOG_ERR, "5279: Cluster name %s is not configured in lsf.shared", myClusterName);
        lim_Exit ( readCluster );
    }

    if (readCluster2 (myClusterPtr) < 0) {
        lim_Exit ( readCluster );
    }

    myClusterPtr->loadIndxNames = calloc (allInfo.numIndx, sizeof (char *));

    for (unsigned int i = 0; i < allInfo.numIndx; i++) {
        myClusterPtr->loadIndxNames[i] = putstr_ (li[i].name);
    }

    if ((hname = ls_getmyhostname ()) == NULL) {
        char exitmsg[MAX_LINE_LEN/4];
        const char ls_getmyhostname[] = "ls_getmyhostname";
        memset( exitmsg, 0, MAX_LINE_LEN/4 );
        sprintf( exitmsg, "%s: failed at %s", __func__, ls_getmyhostname );
        lim_Exit ( exitmsg ); // FIXME FIXME FIXME FIXME lim_Exit all the things!
    }

    myHostPtr = findHostbyList (myClusterPtr->hostList, hname);
    if (!myHostPtr) {
        myHostPtr = findHostbyList (myClusterPtr->clientList, hname);
        if (!myHostPtr) {
            /* catgets 5281 */
            ls_syslog (LOG_ERR, "5281: %s: Local host %s not configured in Host section of file lsf.cluster.%s", __func__, hname, myClusterName);
            if (checkMode) {
                return -1;
            }
            else {
                lim_Exit (readCluster);
            }
        }
        else
            {
            /* catgets 5282 */
            ls_syslog (LOG_ERR, "5282: %s: Local host %s is configured as client-only in file lsf.cluster.%s; LIM will not run on a client-only host", __func__, hname, myClusterName);
            if (!checkMode) {
                lim_Exit (readCluster);
            }
        }
    }

    for (unsigned int i = 1; i < 8; i++) { / FIXME FIXME FIXME FIXME FIXME 8 seems awfully specific
        if (myHostPtr->week[i] != NULL) {
            break;
        }
        counter = i;
    }

    if (counter == 8) { // FIXME FIXME FIXME FIXME FIXME 8 seems awfully specific
        for (unsigned int i = 1; i < 8; i++) { // FIXME FIXME FIXME FIXME FIXME 8 seems awfully specific
            insertW (&(myHostPtr->week[i]), -1.0, 25.0); // FIXME FIXME FIXME FIXME FIXME -1.0, 25.0 seems awfully specific
        }
    }

    for ( unsigned int i = 0; i < GET_INTNUM (allInfo.numIndx) + 1; i++) {
        myHostPtr->status[i] = 0;
    }
    checkHostWd ();

    if (nClusAdmins == 0) {
        size_t len = sizeof( char ) * MAX_LSF_NAME_LEN + 1; // FIXME what is the correct size?
        char *rootName = malloc( len );

        /* catgets 5396 */
        ls_syslog (LOG_ERR, "5396: %s: No LSF managers are specified in file lsf.cluster.%s, default cluster manager is root.", __func__, myClusterName);

        clusAdminIds = malloc (sizeof (int));
        clusAdminIds[0] = 0;
        nClusAdmins = 1;
        clusAdminNames = malloc (sizeof (char *));
        getLSFUserByUid_ (0, rootName, len);
        clusAdminNames[0] = putstr_ (rootName);
    }

    myClusterPtr->status = CLUST_STAT_OK | CLUST_ACTIVE | CLUST_INFO_AVAIL;
    myClusterPtr->managerName = clusAdminNames[0];
    myClusterPtr->managerId = clusAdminIds[0];

    myClusterPtr->nAdmins = nClusAdmins;
    myClusterPtr->adminIds = clusAdminIds;
    myClusterPtr->admins = clusAdminNames;

    return 0;
}

int
readCluster2 (struct clusterNode *clPtr) // FIXME FIXME FIXME FIXME move to cluster.c file
{
    char fileName[MAX_FILENAME_LEN];
    char *word = NULL;
    FILE *clfp = NULL;
    char *cp   = NULL;
    int Error  = FALSE;
    unsigned int lineNum = 0;

    const char lsf_cluster[] = "lsf.cluster";
    const char readonly[]    = "r";

    sprintf (fileName, "%s/%s.%s", limParams[LSF_CONFDIR].paramValue, lsf_cluster, clPtr->clName); // FIXME FIXME FIXME FIXME filename is expressed as a const string here, move to configure.ac

    if (configCheckSum (fileName, &clPtr->checkSum) < 0)
    {
        return -1;
    }
    if ((clfp = confOpen (fileName, readonly )) == NULL)
    {
        const char confOpen[] = "confOpen";
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, confOpen, fileName);
        return -1;
    }

    for (;;) { // FIXME FIXME FIXME FIXME remove infinite for loop
        cp = getBeginLine (clfp, &lineNum);
        if (!cp)
        {
            FCLOSEUP (&clfp);
            if (clPtr->hostList)
            {
                if (Error) {
                    return -1;
                }
                else {
                    adjIndx ();
                    chkUIdxAndSetDfltRunElim ();
                    return 0;
                }
            }
            else if (!(clPtr->hostList))
            {
                /* catgets 5285 */
                ls_syslog (LOG_ERR, "5285: %s: %s(%d): No hosts configured for cluster %s", __func__, fileName, lineNum, clPtr->clName);
                return -1;
            }
        }

        word = getNextWord_ (&cp);
        if (!word)
        {
            const char unknown[] = "unknown";
            /* catgets 5286 */
            ls_syslog (LOG_ERR, "5286: %s: %s(%d): Keyword expected after Begin. Ignoring section", __func__, fileName, lineNum);
            lim_CheckError = WARNING_ERR;
            doSkipSection (clfp, &lineNum, fileName, unknown);
        }
        else if (strcasecmp (word, "clusteradmins") == 0)
        {
            if (clPtr != myClusterPtr)
            {
                doSkipSection (clfp, &lineNum, fileName, "clusteradmins");
                continue;
            }
            if ( lim_domanager (clfp, fileName, &lineNum, "clusteradmins") == FALSE) {
                Error = TRUE;
            }
            continue;
        }
        else if (strcasecmp (word, "parameters") == 0)
        {
            if (doclparams (clfp, fileName, &lineNum) < 0)
                lim_CheckError = WARNING_ERR;
            continue;
        }
        else if (strcasecmp (word, "host") == 0)
        {
            if (dohosts (clfp, clPtr, fileName, &lineNum) < 0)
                Error = TRUE;
            continue;
        }
        else if (strcasecmp (word, "resourceMap") == 0)
        {
            if (doresourcemap (clfp, fileName, &lineNum) < 0) {
                Error = TRUE;
            }
            continue;
        }
        else
        {
            /* catgets 5287 */
            ls_syslog (LOG_ERR, "5287: %s %s(%d): Invalid section name %s, ignoring section", /* catgets 5287 */ __func__, fileName, lineNum, word);
            lim_CheckError = WARNING_ERR;
            doSkipSection (clfp, &lineNum, fileName, word);
        }
    }

    ls_syslog( LOG_ERR, "%s: I do not think we were supposed to reach the end of the function.", __func__ );
    fprintf( stderr, "%s: I do not think we were supposed to reach the end of the function.", __func__ )
    return 255;
}

void
adjIndx (void)
{
    int returnResult      = 0;
    unsigned int resNo    = 0;
    char **temp           = NULL;
    struct hostNode *hPtr = NULL;
    struct resItem tmpTable;

    if (numHostResources <= 0) {
        return;
    }

    for ( unsigned int i = 0; i < numHostResources; i++) {
        if ((returnResult = resNameDefined (hostResources[i]->resourceName)) < 0) {
            continue;
        }
        else {
            resNo = (unsigned int)returnResult; // FIXME FIXME FIXME check utility of cast here
        }

        if ((allInfo.resTable[resNo].valueType != LS_NUMERIC) || !(allInfo.resTable[resNo].flags & RESF_SHARED)) {
            continue;
        }


        memcpy ((char *) &tmpTable, (char *) &allInfo.resTable[resNo], sizeof (struct resItem));
        for ( unsigned int j = resNo; j < allInfo.nRes - 1; j++) {
            memcpy ((char *) &allInfo.resTable[j], (char *) &allInfo.resTable[j + 1], sizeof (struct resItem));
        }
        memcpy ((char *) &allInfo.resTable[allInfo.nRes - 1], (char *) &tmpTable, sizeof (struct resItem));


        if ((temp = realloc (shortInfo.resName, (shortInfo.nRes + 1) * sizeof (char *))) == NULL)
        {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            lim_Exit (__func__);
        }
        shortInfo.resName = temp;
        if ((shortInfo.resName[shortInfo.nRes] = putstr_ (tmpTable.name)) == NULL)
        {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            lim_Exit (__func__);
        }
        SET_BIT (shortInfo.nRes, shortInfo.numericResBitMaps);
        shortInfo.nRes++;

        if (tmpTable.flags & RESF_DYNAMIC)
        {

            for (unsigned int k = NBUILTINDEX; k < allInfo.numIndx; k++)
            {
                if (strcasecmp (li[k].name, tmpTable.name) != 0) {
                    continue;
                }
                FREEUP (li[k].name);
                for (unsigned int j = k; j < allInfo.numIndx - 1; j++)
                {
                    memcpy (&li[j], &li[j + 1], sizeof (struct liStruct));
                }
                break;
            }



            for (hPtr = myClusterPtr->hostList; hPtr != NULL; hPtr = hPtr->nextPtr)
            {
                for (unsigned int j =  resNo; j < allInfo.numIndx - 1; j++)
                    hPtr->busyThreshold[j] = hPtr->busyThreshold[j + 1];
            }
            allInfo.numUsrIndx--;
            allInfo.numIndx--;
        }

    } // end for ( unsigned int i = 0; i < numHostResources; i++)

    return;
}

int
lim_domanager (FILE * fp, const char *lsfile, unsigned int *lineNum, const char *secName) // function name (only) replicated liblsf/conf.c
{
    char *linep = NULL;
    struct keymap *keyList = NULL;

    char clustermanager[]  = "clustermanager";

    enum managers {
        MANAGERS
    };

    enum administrators {
        ADMINISTRATORS
    };

    const char *keylist[] = {
        "MANAGERS",
        "ADMINISTRATORS",
        NULL
    };



    struct keymap keyList1[] = {
        { MANAGERS, "    ", NULL, keylist[MANAGERS] },
        { -1,       "    ", NULL, NULL }
    };

    struct keymap keyList2[] = {
        { ADMINISTRATORS, "    ", NULL, keylist[ADMINISTRATORS] },
        { -1,             "    ", NULL, NULL}
    };



    if (lim_debug > 0 && lim_debug < 3)
    {
        char lsfUserName[MAX_LSF_NAME_LEN];

        nClusAdmins   = 1;
        clusAdminIds  = malloc (sizeof (uid_t));
        clusAdminGids = malloc (sizeof (uid_t));
        if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) < 0)
        {
            const char getLSFUser[] = "getLSFUser_";
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, getLSFUser );
            return FALSE;
        }
        clusAdminIds[ADMINISTRATORS]   = getuid ();
        clusAdminGids[ADMINISTRATORS]  = getgid ();
        clusAdminNames                 = malloc (sizeof (char *));
        clusAdminNames[ADMINISTRATORS] = putstr_ (lsfUserName);
        doSkipSection (fp, lineNum, lsfile, secName);
        if (lim_CheckMode > 0) {
            /* catgets 5289 */
            ls_syslog (LOG_ERR, "5289: %s: %s(%d): The cluster manager is the invoker <%s> in debug mode", __func__, lsfile, *lineNum, lsfUserName);
            return TRUE; // return here may be outside the conditional
        }
            // FIXME FIXME FIXME some calls to free() oughta be here
    }

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
    {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, secName); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, lsfile, lineNum, secName)) {
        return FALSE;
    }

    if (strcmp (secName, clustermanager) == 0) {  
        keyList = keyList1;
    }
    else {
        keyList = keyList2;
    }

    if (strchr (linep, '=') == NULL)
    {
        if (!keyMatch (keyList, linep, TRUE))
        {
            /* catgets 5291 */
            ls_syslog (LOG_ERR, "5291: %s: %s(%d): keyword line format error for section %s, ignoring section", __func__, lsfile, *lineNum, secName);
            doSkipSection (fp, lineNum, lsfile, secName);
            return FALSE;
        }

        if ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
        {
            if (isSectionEnd (linep, lsfile, lineNum, secName)) {
                return FALSE;
            }
            if (mapValues (keyList, linep) < 0)
            {
                /* catgets 5292 */
                ls_syslog (LOG_ERR, "5292: %s: %s(%d): values do not match keys for section %s, ignoring section", __func__, lsfile, *lineNum, secName);
                doSkipSection (fp, lineNum, lsfile, secName);
                return FALSE;
            }
            if (lim_getClusAdmins (keyList[MANAGERS].val, lsfile, lineNum, secName) < 0)
            {
                FREEUP (keyList[MANAGERS].val);
                return FALSE;
            }
            else
            {
                FREEUP (keyList[MANAGERS].val);
                return TRUE;
            }
        }
    }
    else
    {
        if (readHvalues (keyList, linep, clfp, lsfile, lineNum, TRUE, secName) < 0) {
            return FALSE;
        }
        if (lim_getClusAdmins (keyList[0].val, lsfile, lineNum, secName) < 0)
        {
            FREEUP (keyList[MANAGERS].val);
            return FALSE;
        }
        else
        {
            FREEUP (keyList[MANAGERS].val);
            return TRUE;
        }
    }
    return TRUE;
}

int
lim_getClusAdmins (char *line, const char *lsfile, unsigned int *lineNum, const char *secName) // function name (only)is replicated in liblsf/conf.c
{
    int count = 0;
    struct admins *admins = NULL;
    static char lastSecName[40];
    const char clustermanager[] = "clustermanager";
    const char clusteradmins[]  = "clusteradmins";

    assert( strlen ( secName ) <= strlen( clustermanager ) );

    if (count > 1)
        {
        /* catgets 5293 */
        ls_syslog (LOG_ERR, "5293: %s: %s(%d): More than one %s section defined; ignored.", __func__, lsfile, *lineNum, secName);
        return -1;
        }
    count++;
    if (strncmp (lastSecName, secName, strlen( lastSecName) ) == 0)
        {
        /* catgets 5294 */
        ls_syslog (LOG_ERR, "5294: %s: %s(%d): section <%s> is multiply specified; ignoring the section", __func__, lsfile, *lineNum, secName);
        return -1;
        }
    lserrno = LSE_NO_ERR;
    admins = lim_getAdmins (line, lsfile, lineNum, secName);
    if (admins->nAdmins <= 0)
        {
        /* catgets 5295 */
        ls_syslog (LOG_ERR, "5295: %s: %s(%d): No valid user for section %s: %s", __func__, lsfile, *lineNum, secName, line);
        return -1;
        }
    if (strncmp (secName, clustermanager, strlen( clustermanager ) ) == 0 && strcmp (lastSecName, clusteradmins) == 0)
        {
        if (lim_setAdmins (admins, A_THEN_M) < 0) {
            return -1;
        }
    }
    else if (strcmp (lastSecName, clustermanager ) == 0 && strcmp (secName, clusteradmins ) == 0)
        {
        if (lim_setAdmins (admins, M_THEN_A) < 0) {
            return -1;
        }
    }
    else
        {
        if (lim_setAdmins (admins, M_OR_A) < 0) {
            return -1;
        }
    }
    strcpy (lastSecName, secName);
    return 0;

}

int
lim_setAdmins (struct admins *admins, int mOrA)
{
    unsigned int workNAdmins    = 0;
    unsigned int tempNAdmins    = 0;
    uid_t *tempAdminIds         = 0;
    gid_t *tempAdminGids        = 0;
    unsigned int *workAdminIds  = 0;
    unsigned int *workAdminGids = 0;
    char **tempAdminNames       = NULL;
    char **workAdminNames       = NULL;
    const char malloc[]         = "malloc";

    tempNAdmins    = admins->nAdmins + nClusAdmins;
    tempAdminIds   = malloc (tempNAdmins * sizeof (uid_t)  );
    tempAdminGids  = malloc (tempNAdmins * sizeof (gid_t)  );
    tempAdminNames = malloc (tempNAdmins * sizeof (char *) );
    if (!tempAdminIds || !tempAdminGids || !tempAdminNames) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, malloc );
        FREEUP (tempAdminIds);
        FREEUP (tempAdminGids);
        FREEUP (tempAdminNames);
        return -1;
    }
    if (mOrA == M_THEN_A) {
        workNAdmins    = nClusAdmins;
        workAdminIds   = clusAdminIds;
        workAdminGids  = clusAdminGids;
        workAdminNames = clusAdminNames;
    }
    else {
        workNAdmins    = admins->nAdmins;
        workAdminIds   = admins->adminIds;
        workAdminGids  = admins->adminGIds;
        workAdminNames = admins->adminNames;
    }

    for ( unsigned int i = 0; i < workNAdmins; i++) {
        tempAdminIds[i]   = workAdminIds[i];
        tempAdminGids[i]  = workAdminGids[i];
        tempAdminNames[i] = putstr_ (workAdminNames[i]);
    }
    tempNAdmins = workNAdmins;
    if (mOrA == M_THEN_A) {
        workNAdmins    = admins->nAdmins;
        workAdminIds   = admins->adminIds;
        workAdminGids  = admins->adminGIds;
        workAdminNames = admins->adminNames;
    }
    else if (mOrA == A_THEN_M)
        {
        workNAdmins    = nClusAdmins;
        workAdminIds   = clusAdminIds;
        workAdminGids  = clusAdminGids;
        workAdminNames = clusAdminNames;
        }
    else {
        workNAdmins = 0;
    }
    for ( unsigned int i = 0; i < workNAdmins; i++)
        {
        if (isInlist (tempAdminNames, workAdminNames[i], tempNAdmins)) {
            continue;
        }
        tempAdminIds[tempNAdmins]   = workAdminIds[i];
        tempAdminGids[tempNAdmins]  = workAdminGids[i];
        tempAdminNames[tempNAdmins] = putstr_ (workAdminNames[i]);
        tempNAdmins++;
    }
    if (nClusAdmins > 0)
        {
        for (unsigned int i = 0; i < nClusAdmins; i++) {
            FREEUP (clusAdminNames[i]);
        }
        FREEUP (clusAdminIds);
        FREEUP (clusAdminGids);
        FREEUP (clusAdminNames);
        }
    nClusAdmins    = tempNAdmins;
    clusAdminIds   = tempAdminIds;
    clusAdminGids  = tempAdminGids;
    clusAdminNames = tempAdminNames;


    return 0;
}



int
doclparams (FILE * clfp, const char *lsfile, unsigned int *lineNum)
{
    char *linep = NULL;
    int warning = FALSE;

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
        EXINTERVAL = 0,
        ELIMARGS,
        PROBE_TIMEOUT,
        ELIM_POLL_INTERVAL,
        HOST_INACTIVITY_LIMIT,
        MASTER_INACTIVITY_LIMIT,
        RETRY_LIMIT,
        ADJUST_DURATION,
        LSF_ELIM_DEBUG,
        LSF_ELIM_BLOCKTIME,
        LSF_ELIM_RESTARTS
    };

    const char *keylist[ ] {
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
    }

    struct keymap keyList[] = {
        { EXINTERVAL,              "    ", keylist[ EXINTERVAL ]             , NULL },
        { ELIMARGS,                "    ", keylist[ ELIMARGS ]               , NULL },
        { PROBE_TIMEOUT,           "    ", keylist[ PROBE_TIMEOUT ]          , NULL },
        { ELIM_POLL_INTERVAL,      "    ", keylist[ ELIM_POLL_INTERVAL ]     , NULL },
        { HOST_INACTIVITY_LIMIT,   "    ", keylist[ HOST_INACTIVITY_LIMIT ]  , NULL },
        { MASTER_INACTIVITY_LIMIT, "    ", keylist[ MASTER_INACTIVITY_LIMIT ], NULL },
        { RETRY_LIMIT,             "    ", keylist[ RETRY_LIMIT ]            , NULL },
        { ADJUST_DURATION,         "    ", keylist[ ADJUST_DURATION ]        , NULL },
        { LSF_ELIM_DEBUG,          "    ", keylist[ LSF_ELIM_DEBUG ]         , NULL },
        { LSF_ELIM_BLOCKTIME,      "    ", keylist[ LSF_ELIM_BLOCKTIME ]     , NULL },
        { LSF_ELIM_RESTARTS,       "    ", keylist[ LSF_ELIM_RESTARTS ]      , NULL },
        { -1,                      "    ", NULL, NULL }
    };

    const char parameters[] = "parameters";

    linep = getNextLineC_ (clfp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, parameters ); /*catgets33 */
        return -1;
    }

    if (isSectionEnd (linep, lsfile, lineNum, parameters )) {
        return 0;
    }

    if (strchr (linep, '=') == NULL) {
        /* catgets 5298 */
        ls_syslog (LOG_ERR, "5298: %s: %s(%d): vertical section not supported, ignoring section", __func__, lsfile, *lineNum);
        doSkipSection (clfp, lineNum, lsfile, parameters);
        return -1;
    }
    else {
        if (readHvalues (keyList, linep, clfp, lsfile, lineNum, FALSE,  parameters ) < 0) {
            return -1;
        }
        if (keyList[EXINTERVAL].val)
            {
            if (!isanumber_ (keyList[EXINTERVAL].val) || atof (keyList[EXINTERVAL].val) < 0.001) { // FIXME FIXME possible rounding error here
                 /* catgets 5299 */
                ls_syslog (LOG_ERR, "5299: %s: %s(%d): Invalid exchange interval in section parameters: %s. Ignoring.", __func__, lsfile, *lineNum, keyList[EXINTERVAL].val);
                FREEUP (keyList[EXINTERVAL].val);
                warning = TRUE;
            }
            else {
                exchIntvl = atof (keyList[EXINTERVAL].val);
            }
            FREEUP (keyList[EXINTERVAL].val);

            if (exchIntvl < 15) {
                int temp = exchIntvl;
                resInactivityLimit = 180 / temp;
            }
        }

        if (keyList[ELIMARGS].val) {
            myClusterPtr->eLimArgs = putstr_ (keyList[ELIMARGS].val);

            if (!myClusterPtr->eLimArgs) {
                ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "malloc", keyList[ELIMARGS].val);
                return -1;
            }
            FREEUP (keyList[ELIMARGS].val);
        }

        if (keyList[PROBE_TIMEOUT].val) {
            if (!isint_ (keyList[PROBE_TIMEOUT].val) || atoi (keyList[PROBE_TIMEOUT].val) < 0)
                {
                /* catgets 5301 */
                ls_syslog (LOG_ERR, "5301: %s: %s(%d): Invalid probe timeout value in section parameters: %s. Ignoring.", __func__, lsfile, *lineNum, keyList[PROBE_TIMEOUT].val);
                warning = TRUE;
                FREEUP (keyList[PROBE_TIMEOUT].val);
                }
            else {
                probeTimeout = atoi (keyList[PROBE_TIMEOUT].val);
            }

            FREEUP (keyList[PROBE_TIMEOUT].val);
        }

        if (keyList[ELIM_POLL_INTERVAL].val)
            {
            if (!isanumber_ (keyList[ELIM_POLL_INTERVAL].val) || atof (keyList[ELIM_POLL_INTERVAL].val) < 0.001 || atof (keyList[ELIM_POLL_INTERVAL].val) > 5)
                {
                /* catgets 5302 */
                ls_syslog (LOG_ERR, "5302: %s: %s(%d): Invalid sample interval in section parameters: %s. Must be between 0.001 and 5. Ignoring.", __func__, lsfile, *lineNum, keyList[ELIM_POLL_INTERVAL].val);
                warning = TRUE;
                FREEUP (keyList[ELIM_POLL_INTERVAL].val);
                }
            else {
                sampleIntvl = (float) atof (keyList[ELIM_POLL_INTERVAL].val);
            }
            FREEUP (keyList[ELIM_POLL_INTERVAL].val);
        }

        if (keyList[HOST_INACTIVITY_LIMIT].val)
            {
            if (!isint_ (keyList[HOST_INACTIVITY_LIMIT].val) || atoi (keyList[HOST_INACTIVITY_LIMIT].val) < 2) {
                /* catgets 5303 */
                ls_syslog (LOG_ERR, "5303: %s: %s(%d): Invalid host inactivity limit in section parameters: %s. Ignoring.", __func__, lsfile, *lineNum, keyList[HOST_INACTIVITY_LIMIT].val);
                FREEUP (keyList[HOST_INACTIVITY_LIMIT].val);
                warning = TRUE;
            }
            else {
                assert( atoi (keyList[HOST_INACTIVITY_LIMIT].val) <= SHRT_MAX );
                hostInactivityLimit = (short) atoi (keyList[HOST_INACTIVITY_LIMIT].val);
            }
            FREEUP (keyList[HOST_INACTIVITY_LIMIT].val);
        }

        if (keyList[MASTER_INACTIVITY_LIMIT].val) {
            if (!isint_ (keyList[MASTER_INACTIVITY_LIMIT].val) || atoi (keyList[MASTER_INACTIVITY_LIMIT].val) < 0) {
                /* catgets 5304 */
                ls_syslog (LOG_ERR, "5304: %s: %s(%d): Invalid master inactivity limit in section parameters: %s. Ignoring.", __func__, lsfile, *lineNum, keyList[MASTER_INACTIVITY_LIMIT].val);
                FREEUP (keyList[MASTER_INACTIVITY_LIMIT].val);
                warning = TRUE;
            }
            else {
                assert( atoi (keyList[MASTER_INACTIVITY_LIMIT].val) <= SHRT_MAX );
                masterInactivityLimit = (short) atoi (keyList[MASTER_INACTIVITY_LIMIT].val);
            }
            FREEUP (keyList[MASTER_INACTIVITY_LIMIT].val);
        }

        if (keyList[RETRY_LIMIT].val)
            {
            if (!isint_ (keyList[RETRY_LIMIT].val) || atoi (keyList[RETRY_LIMIT].val) < 0)
                {
                ls_syslog (LOG_ERR, "5305: %s: %s(%d): Invalid host inactivity limit in section parameters: %s. Ignoring.", __func__, lsfile, *lineNum, keyList[RETRY_LIMIT].val);   /* catgets 5305 */
                FREEUP (keyList[RETRY_LIMIT].val);
                warning = TRUE;
                }
            else{
                assert( atoi (keyList[RETRY_LIMIT].val) <= SHRT_MAX );
                retryLimit = (short) atoi (keyList[RETRY_LIMIT].val);
            }
            FREEUP (keyList[RETRY_LIMIT].val);
            }

        if (keyList[ADJUST_DURATION].val)
            {
            if (!isint_ (keyList[ADJUST_DURATION].val) || atoi (keyList[ADJUST_DURATION].val) < 0)
                {
                /* catgets 5306 */
                ls_syslog (LOG_ERR, "5306: %s: %s(%d): Invalid load adjust duration in section parameters: %s. Ignoring.",  __func__, lsfile, *lineNum, keyList[ADJUST_DURATION].val);
                FREEUP (keyList[ADJUST_DURATION].val);
                warning = TRUE;
                }
            else {
                assert( atoi (keyList[ADJUST_DURATION].val) <= SHRT_MAX );
                keepTime = (short) atoi (keyList[ADJUST_DURATION].val);
            }
            FREEUP (keyList[ADJUST_DURATION].val);
        }


        if (keyList[LSF_ELIM_DEBUG].val)
            {

            if (strcasecmp (keyList[LSF_ELIM_DEBUG].val, "y") != 0 && strcasecmp (keyList[LSF_ELIM_DEBUG].val, "n") != 0) {

                /* catgets 5316 */
                ls_syslog (LOG_WARNING, "5316: LSF_ELIM_DEBUG invalid: %s, not debuging ELIM.", keyList[LSF_ELIM_DEBUG].val);
                warning = TRUE;
            }
            else {
                if (strcasecmp (keyList[LSF_ELIM_DEBUG].val, "y") == 0)
                    {
                    ELIMdebug = 1;
                    }
                }

            FREEUP (keyList[LSF_ELIM_DEBUG].val);
            }


        if (keyList[LSF_ELIM_BLOCKTIME].val) {

            ELIMblocktime = atoi (keyList[LSF_ELIM_BLOCKTIME].val);


            if (!isint_ (keyList[LSF_ELIM_BLOCKTIME].val) || ELIMblocktime < 0)
                {
                /* catgets 5318 */
                ls_syslog (LOG_WARNING, "5318: LSF_ELIM_BLOCKTIME invalid: %s, blocking communication with ELIM.", keyList[LSF_ELIM_BLOCKTIME].val);
                warning = TRUE;
                ELIMblocktime = -1;
            }

            FREEUP (keyList[LSF_ELIM_BLOCKTIME].val);
            }


        if (ELIMdebug && ELIMblocktime == -1)
            {
            /* catgets 5340 */
            ls_syslog (LOG_WARNING, "5340: LSF_ELIM_DEBUG=y but LSF_ELIM_BLOCKTIME is not set/valid; LSF_ELIM_BLOCKTIME will be set to 2 seconds ");
            warning = TRUE;
            ELIMblocktime = 2;
            }


        if (keyList[LSF_ELIM_RESTARTS].val)
            {
            ELIMrestarts = atoi (keyList[LSF_ELIM_RESTARTS].val);

            if (!isint_ (keyList[LSF_ELIM_RESTARTS].val) || ELIMrestarts < 0)
                {
                /* catgets 5366 */
                ls_syslog (LOG_WARNING, "5366: LSF_ELIM_RESTARTS invalid: %s, unlimited ELIM restarts.", keyList[LSF_ELIM_RESTARTS].val);
                warning = TRUE;
                ELIMrestarts = -1;
                }
            else
                {
                ELIMrestarts += 1;
                }
            FREEUP (keyList[LSF_ELIM_RESTARTS].val);
            }


        if (exchIntvl < sampleIntvl)
            {
            /* catgets 5308 */
            ls_syslog (LOG_ERR, "5308: %s: Exchange interval must be greater than or equal to sampling interval. Setting exchange and sample interval to %f.", __func__, sampleIntvl);
            exchIntvl = sampleIntvl;
            warning = TRUE;
            }

        if (warning == TRUE)
            return -1;

        return 0;
        }

    return 255;
}

// struct keymap *
// initDohostsKeyList (void)
// {
//  struct keymap keyList1[] = {
//      {  0, "    " , "HOSTNAME",                NULL },
//      {  1, "    " , "MODEL",                   NULL },
//      {  2, "    " , "TYPE",                    NULL },
//      {  3, "    " , "ND",                      NULL },
//      {  4, "    " , "RESOURCES",               NULL },
//      {  5, "    " , "RUNWINDOW",               NULL },
//      {  6, "    " , "REXPRI0",                 NULL },
//      {  7, "    " , "SERVER0",                 NULL },
//      {  8, "    " , "R",                       NULL },
//      {  9, "    " , "S",                       NULL },
//      {  0, "    " , NULL, NULL }
//  };


// #define  HOSTNAME    allInfo.numIndx     // FIXME FIXME FIXME FIXME FIXME find out if allInfo.numIndx was a global parapemeter list
// #define  MODEL        allInfo.numIndx+1
// #define  TYPE         allInfo.numIndx+2
// #define  ND           allInfo.numIndx+3
// #define  RESOURCES    allInfo.numIndx+4
// #define  RUNWINDOW    allInfo.numIndx+5
// #define  REXPRI_      allInfo.numIndx+6
// #define  SERVER_      allInfo.numIndx+7
// #define  R            allInfo.numIndx+8
// #define  S            allInfo.numIndx+9

    // enum {
    //  HOSTNAME = 0,
    //  MODEL    ,
    //  TYPE     ,
    //  ND       ,
    //  RESOURCES,
    //  RUNWINDOW,
    //  REXPRI0  ,
    //  SERVER0  ,
    //  R        ,
    //  S
    // };

    // if (hostKeyList == NULL)
    //  {
    //  hostKeyList = calloc (allInfo.numIndx + 11, sizeof (struct keymap));
    //  if (hostKeyList == NULL) {
    //      return NULL;
    //  }
    // }

    // for (unsigned int i = 0; i < S + 1; i++)
    //  {
    //  hostKeyList[i].key = "";
    //  hostKeyList[i].val = NULL;
    //  hostKeyList[i].position = 0;
    //  }

    // hostKeyList[HOSTNAME].key = "HOSTNAME";
    // hostKeyList[MODEL].key     = "MODEL";
    // hostKeyList[TYPE].key      = "TYPE";
    // hostKeyList[ND].key        = "ND";
    // hostKeyList[RESOURCES].key = "RESOURCES";
    // hostKeyList[RUNWINDOW].key = "RUNWINDOW";
    // hostKeyList[REXPRI_].key   = "REXPRI";
    // hostKeyList[SERVER_].key   = "SERVER";
    // hostKeyList[R].key         = "R";
    // hostKeyList[S].key         = "S";
    // hostKeyList[S + 1].key     = NULL;

//  for ( unsigned int i = 0; i < allInfo.numIndx; i++) {
//      hostKeyList[i].key = allInfo.resTable[i].name;
//  }

//  return hostKeyList;
// }

void
setMyClusterName (void)
{
    FILE *fp      = NULL;
    char found    = FALSE;
    char *lp      = NULL;
    char *cp      = NULL;
    char *hname   = NULL;
    char *cluster = NULL;
    unsigned int lineNum = 0;
    char clusterFile[MAX_FILENAME_LEN];

    struct keymap *keyList = NULL;

    const char lsf_cluster[] = "lsf.cluster";

    if ((hname = ls_getmyhostname ()) == NULL) {
        lim_Exit ("%s: ls_getmyhostname failed", __func__ );
    }

    ls_syslog (LOG_DEBUG, "%s: searching cluster files ...", __func__ );
    cluster = myClusterPtr->clName;
    sprintf (clusterFile, "%s/%s.%s", limParams[LSF_CONFDIR].paramValue, lsf_cluster, cluster);
    fp = confOpen (clusterFile, "r");

    if (!fp)
        {
        if (!found && !mcServersSet)
            {
            ls_syslog (LOG_ERR, I18N_CANNOT_OPEN, __func__, clusterFile);
            }
        goto endfile;
        }

    lineNum = 0;

    for (;;) // FIXME FIXME FIXME FIXME replace infinute loop with terminating condition
        {
        if ((lp = getBeginLine (fp, &lineNum)) == NULL)
            {
            if (!found)
                {
                ls_syslog (LOG_DEBUG, "%s: Local host %s not defined in cluster file %s", __func__, hname, clusterFile);
                }
            break;
            }

        cp = getNextWord_ (&lp);
        if (!cp)
            {
            if (!found)
                {
                ls_syslog (LOG_ERR, "5317: %s: %s(%d): Section name expected after Begin; section ignored.", __func__, clusterFile, lineNum);    /* catgets 5317 */
                lim_CheckError = WARNING_ERR;
                }
            continue;
            }

        if (strcasecmp (cp, "host") != 0)
            continue;

        lp = getNextLineC_ (fp, &lineNum, TRUE);
        if (!lp)
            {
            if (!found)
                {
                ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", /*catgets33 */
                           __func__, clusterFile, lineNum, "host");
                lim_CheckError = WARNING_ERR;
                }
            break;
            }
        if (isSectionEnd (lp, clusterFile, &lineNum, "Host"))
            {

            continue;
            }
        if (strchr (lp, '=') != NULL)
            {
            if (!found)
                {
                /* catgets 5319 */
                ls_syslog (LOG_ERR, "5319: %s: %s(%d): horizontal host section not implemented yet, use vertical format: section ignored", __func__, clusterFile, lineNum);
                lim_CheckError = WARNING_ERR;
                }
            continue;
            }

        keyList = initDohostsKeyList ();

        if (!keyMatch (keyList, lp, FALSE))
            {
            if (!found)
                {
                ls_syslog (LOG_ERR, "5320: %s: %s(%d): keyword line format error for section Host, section ignored", __func__, clusterFile, lineNum);    /* catgets 5320 */
                lim_CheckError = WARNING_ERR;
                }
            continue;
            }
        if (keyList[HOSTNAME].position == -1)
            {
            if (!found)
                {
                ls_syslog (LOG_ERR, "5321: %s: %s(%d): key HOSTNAME is missing in section host, section ignored", __func__, clusterFile, lineNum);  /* catgets 5321 */
                lim_CheckError = WARNING_ERR;
                }
            continue;
            }

        while ((lp = getNextLineC_ (fp, &lineNum, TRUE)) != NULL)
            {

            if (isSectionEnd (lp, clusterFile, &lineNum, "host"))
                break;

            if (mapValues (keyList, lp) < 0)
                {
                if (!found)
                    {
                    ls_syslog (LOG_ERR, "5322: %s: %s(%d): values do not match keys for section Host, record ignored", __func__, clusterFile, lineNum);  /* catgets 5322 */
                    lim_CheckError = WARNING_ERR;
                    }
                continue;
                }

            if (Gethostbyname_ (keyList[HOSTNAME].val) == NULL)
                {
                if (!found)
                    {
                    ls_syslog (LOG_ERR, "%s: %s(%d): Invalid hostname %s in section host, host ignored", __func__, clusterFile, lineNum, keyList[HOSTNAME].val);
                    lim_CheckError = WARNING_ERR;
                    }
                freeKeyList (keyList);
                continue;
                }

            if (strcasecmp (keyList[HOSTNAME].val, hname) == 0)
                {
                if (!found)
                    {
                    ls_syslog (LOG_DEBUG, "%s: local host %s belongs to cluster %s", __func__, hname, cluster);
                    found = TRUE;
                    strcpy (myClusterName, cluster);
                    freeKeyList (keyList);
                    break;
                    }
                else
                    {
                    ls_syslog (LOG_ERR, "5324: %s: %s(%d): local host %s defined in more than one cluster file. Previous definition was in lsf.cluster.%s, ignoring current definition", __func__, clusterFile, lineNum, hname, myClusterName);  /* catgets 5324 */
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                    }
                }
            freeKeyList (keyList);
            }
        }

    FCLOSEUP (&fp);

endfile:

    if (!found) {
        ls_syslog (LOG_ERR, " %s: unable to find the cluster file containing local host %s", __func__, hname);
        lim_Exit ( __func__ );
    }

    return;
}

void
freeKeyList (struct keymap *keyList)
{
    for ( unsigned int i = 0; keyList[i].key != NULL; i++) {
        if (keyList[i].position != -1) {
            FREEUP (keyList[i].val);
        }
    }
}


int
dohosts (FILE * clfp, struct clusterNode *clPtr, const char *lsfile, unsigned int *lineNum)
{
    char *sp     = NULL;
    char *cp     = NULL;
    char *word   = NULL;
    char *window = NULL;
    char *linep  = NULL;
    int ignoreR  = FALSE;
    int n = 0;
    struct hostEntry hostEntry;


    enum {
        HOSTNAME = 0,
        MODEL,
        TYPE,
        ND,
        RESOURCES,
        RUNWINDOW,
        REXPRI0,
        SERVER0,
        R,
        S
    };

    const char *keylist[ ] = {
        "HOSTNAME",
        "MODEL",
        "TYPE",
        "ND",
        "RESOURCES",
        "RUNWINDOW",
        "REXPRI0",
        "SERVER0",
        "R",
        "S",
        NULL
    };

    struct keymap keyList[] = {
        { HOSTNAME,  "    " , keylist[ HOSTNAME ],  NULL },
        { MODEL,     "    " , keylist[ MODEL ],     NULL },
        { TYPE,      "    " , keylist[ TYPE ],      NULL },
        { ND,        "    " , keylist[ ND ],        NULL },
        { RESOURCES, "    " , keylist[ RESOURCES ], NULL },
        { RUNWINDOW, "    " , keylist[ RUNWINDOW ], NULL },
        { REXPRI0,   "    " , keylist[ REXPRI0 ],   NULL },
        { SERVER0,   "    " , keylist[ SERVER0 ],   NULL },
        { R,         "    " , keylist[ R ],         NULL },
        { S,         "    " , keylist[ S ],         NULL },
        { -1,        "    " , NULL, NULL }
    };

    const char host[] = "host";


    hostEntry.busyThreshold = calloc (allInfo.numIndx, sizeof (float));
    if (hostEntry.busyThreshold == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
        return -1;
    }
    hostEntry.resList = calloc (allInfo.nRes, sizeof (char *));
    if (hostEntry.resList == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
        return -1;
    }
    hostEntry.nRes = 0;

    // for ( unsigned int i = 0; i < allInfo.numIndx; i++) { // FIXME FIXME FIXME FIXME not sure whatthis code is here for if we set everythign above
    //  hostKeyList[i].key = allInfo.resTable[i].name;
    // }


    /* Must be called after allInfo is initialiazed
     * here we have a dependency on lsf.shared
     */
    // keyList = initDohostsKeyList( );

    linep = getNextLineC_ (clfp, lineNum, TRUE);
    if (!linep)
    {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, host); /*catgets33 */
        return -1;
    }

    if (isSectionEnd (linep, lsfile, lineNum, host))
    {
        /* catgets 5329 */
        ls_syslog (LOG_ERR, "5329: %s: %s(%d): empty host section", __func__, lsfile, *lineNum);
        return -1;
    }

    if (strchr (linep, '=') == NULL)
    {
        if (!keyMatch (keyList, linep, FALSE))
        {
            ls_syslog (LOG_ERR, "5330: %s: %s(%d): keyword line format error for section host, ignoring section", __func__, lsfile, *lineNum);   /* catgets 5330 */
            doSkipSection (clfp, lineNum, lsfile, host);
            return -1;
        }

        for (unsigned int i = 0; keyList[i].key != NULL; i++)
        {

            if (keyList[i].position != -1) {
                continue;
            }

            if ((strcasecmp ("hostname", keyList[i].key) == 0) || (strcasecmp ("model", keyList[i].key) == 0) || (strcasecmp ("type", keyList[i].key) == 0) || (strcasecmp ("resources", keyList[i].key) == 0))
            {
                ls_syslog (LOG_ERR, "5331: %s: %s(%d): keyword line: key %s is missing in section host, ignoring section", __func__, lsfile, *lineNum, keyList[i].key);  /* catgets 5331 */
                doSkipSection (clfp, lineNum, lsfile, host);
                freeKeyList (keyList);
                return -1;
            }
        }

        if (keyList[R].position != -1 && keyList[SERVER_].position != -1)
        {
            /* catgets 5332 */
            ls_syslog (LOG_WARNING, "5332: %s: %s(%d): keyword line: conflicting keyword definition: you cannot define both 'R' and 'SERVER_'. 'R' ignored", __func__, lsfile, *lineNum);
            lim_CheckError = WARNING_ERR;
            ignoreR = TRUE;
        }

        while ((linep = getNextLineC_ (clfp, lineNum, TRUE)) != NULL)
        {

            freeKeyList (keyList);
            for ( int i = 0; i < hostEntry.nRes; i++) {
                FREEUP (hostEntry.resList[i]);
            }
            hostEntry.nRes = 0;

            if (isSectionEnd (linep, lsfile, lineNum, host ) )
            {

                struct hostNode *hPtr, *tPtr;
                for (hPtr = clPtr->hostList, clPtr->hostList = NULL; hPtr; hPtr = tPtr)
                {
                    tPtr = hPtr->nextPtr;
                    hPtr->nextPtr = clPtr->hostList;
                    clPtr->hostList = hPtr;
                }
                for (hPtr = clPtr->clientList, clPtr->clientList = NULL; hPtr; hPtr = tPtr)
                {
                    tPtr = hPtr->nextPtr;
                    hPtr->nextPtr = clPtr->clientList;
                    clPtr->clientList = hPtr;
                }
                return 0;
            }
            if (mapValues (keyList, linep) < 0)
            {
                ls_syslog (LOG_ERR, "5333: %s: %s(%d): values do not match keys for section host, ignoring line", __func__, lsfile, *lineNum);   /* catgets 5333 */
                lim_CheckError = WARNING_ERR;
                continue;
            }

            if (keyList[TYPE].val[0] == '!') {
                hostEntry.hostType[0] = '\0';
            }
            else {
                strcpy (hostEntry.hostType, keyList[TYPE].val);
            }

            if (keyList[MODEL].val[0] == '!') {
                hostEntry.hostModel[0] = '\0';
            }
            else {
                strcpy (hostEntry.hostModel, keyList[MODEL].val);
            }

            strcpy (hostEntry.hostName, keyList[HOSTNAME].val);
            if (keyList[ND].position != -1)
            {
                hostEntry.nDisks = atoi (keyList[ND].val);
            }
            else {
                hostEntry.nDisks = 0;
            }

            putThreshold (R15S, &hostEntry, keyList[R15S].position, keyList[R15S].val, INFINIT_LOAD);
            putThreshold (R1M,  &hostEntry, keyList[R1M].position,  keyList[R1M].val,  INFINIT_LOAD);
            putThreshold (R15M, &hostEntry, keyList[R15M].position, keyList[R15M].val, INFINIT_LOAD);
            if (keyList[UT].val && (cp = strchr (keyList[UT].val, '%')) != NULL) {
                *cp = '\0';
            }
            putThreshold (UT, &hostEntry, keyList[UT].position, keyList[UT].val, INFINIT_LOAD);
            if (hostEntry.busyThreshold[UT] > 1.0 && hostEntry.busyThreshold[UT] < INFINIT_LOAD)
            {
                /* catgets 5476 */
                ls_syslog (LOG_INFO, "5476: %s: %s(%d): value for threshold ut <%2.2f> is greater than 1, assumming <%5.1f%%>", __func__, lsfile, *lineNum, hostEntry.busyThreshold[UT], hostEntry.busyThreshold[UT]);
                hostEntry.busyThreshold[UT] /= 100.0;
            }
            putThreshold (PG,  &hostEntry,  keyList[PG].position,   keyList[PG].val,   INFINIT_LOAD);
            putThreshold (IO,  &hostEntry,  keyList[IO].position,   keyList[IO].val,   INFINIT_LOAD);
            putThreshold (LS,  &hostEntry,  keyList[LS].position,   keyList[LS].val,   INFINIT_LOAD);
            putThreshold (IT,  &hostEntry,  keyList[IT].position,   keyList[IT].val,  -INFINIT_LOAD);
            putThreshold (TMP, &hostEntry,  keyList[TMP].position,  keyList[TMP].val, -INFINIT_LOAD);
            putThreshold (SWP, &hostEntry,  keyList[SWP].position,  keyList[SWP].val, -INFINIT_LOAD);
            putThreshold (MEM, &hostEntry,  keyList[MEM].position,  keyList[MEM].val, -INFINIT_LOAD);

            for ( unsigned int i = NBUILTINDEX; i < allInfo.numIndx; i++)
            {
                if (keyList[i].key == NULL) {
                    continue;
                }

                if (allInfo.resTable[i].orderType == INCR) {
                    assert( i <= INT_MAX );
                    putThreshold ((int)i, &hostEntry, keyList[i].position, keyList[i].val, INFINIT_LOAD);
                }
                else {
                    assert( i <= INT_MAX );
                    putThreshold ((int)i, &hostEntry, keyList[i].position, keyList[i].val, -INFINIT_LOAD);
                }
            }

            for (unsigned int i = NBUILTINDEX + allInfo.numUsrIndx; i < allInfo.numIndx; i++) {
                hostEntry.busyThreshold[i] = INFINIT_LOAD;
            }

            for (unsigned int i = 0; i < allInfo.numIndx; i++) {
                if (keyList[i].position != -1) {
                    FREEUP (keyList[i].val);
                }
            }
            n = 0;
            sp = keyList[RESOURCES].val;
            while ((word = getNextWord_ (&sp)) != NULL) {
                hostEntry.resList[n] = putstr_ (word);
                n++;
            }
            hostEntry.resList[n] = NULL;
            hostEntry.nRes = n;

            hostEntry.rexPriority = DEF_REXPRIORITY;
            if (keyList[REXPRI_].position != -1) {
                hostEntry.rexPriority = atoi (keyList[REXPRI_].val);
            }

            hostEntry.rcv = 1;
            if (keyList[R].position != -1)
            {
                if (!ignoreR) {
                    hostEntry.rcv = atoi (keyList[R].val);
                }
            }

            if (keyList[SERVER_].position != -1)
            {
                hostEntry.rcv = atoi (keyList[SERVER_].val);
            }

            window = NULL;
            if (keyList[RUNWINDOW].position != -1)
            {
                if (strcmp (keyList[RUNWINDOW].val, "") == 0) {
                    window = NULL;
                }
                else {
                    window = keyList[RUNWINDOW].val;
                }
            }

            if (!addHost_ (clPtr, &hostEntry, window, lsfile, lineNum))
            {
                clPtr->checkSum += hostEntry.hostName[0];
                lim_CheckError = WARNING_ERR;
            }

        }
        freeKeyList (keyList);
    }
    else
    {
        ls_syslog (LOG_ERR, "5335: %s: %s(%d): horizontal host section not implemented yet, use vertical format: section ignored", __func__, lsfile, *lineNum);  /* catgets 5335 */
        doSkipSection (clfp, lineNum, lsfile, host);
        return -1;
    }

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNum, host); /*catgets33 */

    return -1;
}

void
putThreshold (int indx, struct hostEntry *hostEntryPtr, int position, char *val, float def)
{
    if (position != -1)
        {
        if (strcmp (val, "") == 0) {
            hostEntryPtr->busyThreshold[indx] = def;
        }
        else {
            hostEntryPtr->busyThreshold[indx] = (float)atof (val);
        }
    }
    else {
        hostEntryPtr->busyThreshold[indx] = def;
    }

}

int
typeNameToNo (const char *typeName)
{
    for ( unsigned int i = 0; i < allInfo.nTypes; i++) {
        if (strcmp (allInfo.hostTypes[i], typeName) == 0) {
            assert( i <= INT_MAX );
            return inti;
        }
    }
    return -1;
}

int
archNameToNo (const char *archName)
{
    int arch_speed = 0;
    int curr_speed = 0;
    int best_speed = 0;
    unsigned int best_pos  = 0;
    unsigned long len = 0;
    char *p = NULL;

    for (unsigned int i = 0; i < allInfo.nModels; ++i)
    {
        if (strcmp (allInfo.hostArchs[i], archName) == 0)
        {
            assert( i <= INT_MAX );
            return inti;
        }
    }
    if ((p = strchr (archName, '_')) != NULL)
    {
        len = (unsigned long)(p - archName);
        arch_speed = atoi (++p);
    }
    else
    {
        len = strlen (archName);
        arch_speed = 0;
    }
    best_pos = 0;
    best_speed = 0;
    for (unsigned int i = 0; i < allInfo.nModels; ++i)
    {
        if (strncmp (archName, allInfo.hostArchs[i], len)) {
            continue;
        }
        p = strchr (allInfo.hostArchs[i], '_');
        curr_speed = p ? atoi (++p) : 0;
        if (arch_speed)
        {
            if ((arch_speed - curr_speed) * (arch_speed - curr_speed) <=
                (arch_speed - best_speed) * (arch_speed - best_speed))
            {
                best_speed = curr_speed;
                assert( i >= 0);
                best_pos = (unsigned int)i;
            }
        }
        else
        {
            if (best_speed <= curr_speed)
            {
                best_speed = curr_speed;
                assert( i >= 0);
                best_pos = (unsigned int)i;
            }
        }
    }

    if (best_pos) {
        assert( best_pos <= INT_MAX );
        return intbest_pos;
    }

    return -1;
}

int
modelNameToNo ( const char *modelName)
{
    for ( unsigned int i = 0; i < allInfo.nModels; i++) {
        if (strcmp (allInfo.hostModels[i], modelName) == 0) {
            assert( i <= INT_MAX );
            return inti;        }
    }

    return -1;
}

struct hostNode *
addHost_ (struct clusterNode *clPtr, struct hostEntry *hEntPtr,  char *window, const char *fileName, unsigned int *lineNumPtr)
{
    struct hostNode *hPtr = NULL;
    struct hostent *hp    = NULL;
    char *word            = NULL;

    if ((hp = Gethostbyname_ (hEntPtr->hostName)) == NULL)
        {
        ls_syslog (LOG_ERR, "%s: Invalid hostname %s in section host. Ignoring host", __func__, hEntPtr->hostName);
        return NULL;
        }

    hPtr = findHostbyList (clPtr->hostList, hEntPtr->hostName);
    if (hPtr)
        {
        ls_syslog (LOG_WARNING, "%s: %s(%d): host %s redefined, using previous definition", __func__, fileName, *lineNumPtr, hEntPtr->hostName);
        return hPtr;
        }

    hPtr = findHostbyList (clPtr->clientList, hEntPtr->hostName);
    if (hPtr)
        {
        ls_syslog (LOG_WARNING, "%s: %s(%d): host %s redefined, using previous definition", __func__, fileName, *lineNumPtr, hEntPtr->hostName);
        return hPtr;
        }

    if ((hPtr = initHostNode ()) == NULL)
        {
        ls_syslog (LOG_ERR, "%s: initHostNode() failed %m", __func__);
        return NULL;
        }

    for ( int i = 0; i < hEntPtr->nRes; i++)
        {
        char *resStr = NULL;
        char dedicated = FALSE;
        int resNo_ = 0;

        if (hEntPtr->resList[i][0] == '!')
            {
            dedicated = TRUE;
            resStr = hEntPtr->resList[i] + 1;
            }
        else {
            resStr = hEntPtr->resList[i];
        }

        if ((resNo_ = validResource (resStr)) >= 0)
            {
            if (resNo_ < INTEGER_BITS) {
                hPtr->resClass |= (1 << resNo_);
            }
            SET_BIT (resNo_, hPtr->resBitMaps);
            if (dedicated)
                {
                if (resNo_ < INTEGER_BITS) {
                    hPtr->DResClass |= (1 << resNo_);
                }
                SET_BIT (resNo_, hPtr->DResBitMaps);
                }

            }
        else
            {
            lim_CheckError = WARNING_ERR;
            ls_syslog (LOG_ERR, "%s: %s(%d): Invalid resource name %s for host %s in section %s; ignored", __func__, fileName, *lineNumPtr, resStr, hEntPtr->hostName, "Host", resStr);
            }
        }

    if (!hEntPtr->hostModel[0])
        {
        hPtr->hModelNo = DETECTMODELTYPE;
        }
    else if ((hPtr->hModelNo = modelNameToNo (hEntPtr->hostModel)) < 0)
        {
        ls_syslog (LOG_ERR, "%s: %s(%d): Unknown host model %s. Ignoring host", __func__, fileName, *lineNumPtr, hEntPtr->hostModel);
        freeHostNodes (hPtr, FALSE);
        return NULL;
        }

    if (!hEntPtr->hostType[0])
        {
        hPtr->hTypeNo = DETECTMODELTYPE;
        }
    else if ((hPtr->hTypeNo = typeNameToNo (hEntPtr->hostType)) < 0)
        {
        ls_syslog (LOG_ERR, "%s: %s(%d): Unknown host type %s. Ignoring host", __func__, fileName, *lineNumPtr, hEntPtr->hostType);
        freeHostNodes (hPtr, FALSE);
        return NULL;
        }

    hPtr->hostName = putstr_ (hp->h_name);

    if (hEntPtr->rcv) {
        hPtr->hostNo = clPtr->hostList ? clPtr->hostList->hostNo + 1 : 0;
    }
    else {
        hPtr->hostNo = clPtr->clientList ? clPtr->clientList->hostNo + 1 : 0;
    }

    if (saveHostIPAddr (hPtr, hp) < 0)
        {
        ls_syslog (LOG_ERR, "%s Can not save internet address of host %s", __func__, hp->h_name);
        freeHostNodes (hPtr, FALSE);
        return NULL;
        }

    hPtr->statInfo.nDisks = hEntPtr->nDisks;
    hPtr->rexPriority = hEntPtr->rexPriority;

    for (unsigned int i = 0; i < allInfo.numIndx; i++) {
        hPtr->busyThreshold[i] = hEntPtr->busyThreshold[i];
    }

    for (unsigned int i = 0; i < 8; i++) {
        hPtr->week[i] = NULL;
    }

    if (window && hEntPtr->rcv)
        {
        hPtr->windows = putstr_ (window);
        while ((word = getNextWord_ (&window)) != NULL)
            {
            if (addWindow (word, hPtr->week, hPtr->hostName) < 0)
                {
                ls_syslog (LOG_ERR, "%s: %s(%d): Bad time expression %s; ignored", __func__, fileName, *lineNumPtr, word);
                lim_CheckError = WARNING_ERR;
                free (hPtr->windows);
                hPtr->windows = strdup ("-");
                hPtr->wind_edge = 0;
                break;
                }
            hPtr->wind_edge = time (0);
            }
        }
    else
        {
        /* dup() all strings so later
         * on we can free without fear.
         */
        hPtr->windows = strdup ("-");
        hPtr->wind_edge = 0;
        }

    if (hEntPtr->rcv)
        {
        hPtr->nextPtr = clPtr->hostList;
        clPtr->hostList = hPtr;
        hPtr->hostInactivityCount = 0;
        }
    else
        {
        hPtr->nextPtr = clPtr->clientList;
        clPtr->clientList = hPtr;
        hPtr->hostInactivityCount = -1;
        }

    if (hEntPtr->rcv) {
        for ( unsigned int resNo = 0; resNo < allInfo.nRes; resNo++)
            {
            int isSet;
            int j;
            char *value;
            char *name;;

            TEST_BIT (resNo, hPtr->resBitMaps, isSet);
            if (isSet == 0) {
                continue;
            }

            name = shortInfo.resName[resNo];
            j = resNameDefined (shortInfo.resName[resNo]);
            if (allInfo.resTable[j].valueType == LS_BOOLEAN) {
                value = "1";
            }
            else {
                value = "";
            }
        }
    }

    numofhosts++;

    return hPtr;
}

struct hostNode *
addFloatClientHost (struct hostent *hp)
{
    struct hostNode *hPtr = NULL;
    struct hostNode *lastHPtr = NULL;

    if (hp == NULL)
        {
        ls_syslog (LOG_ERR, "%s: Invalid hostAddr. Ignoring host", __func__);
        return NULL;
        }

    if (findHostInCluster (hp->h_name))
        {
        ls_syslog (LOG_ERR, "%s: %s already defined in this cluster", __func__, hp->h_name);
        return NULL;
        }

    hPtr = initHostNode ();
    if (hPtr == NULL)
        {
        return NULL;
        }

    hPtr->hTypeNo = DETECTMODELTYPE;
    hPtr->hModelNo = DETECTMODELTYPE;
    hPtr->hostName = putstr_ (hp->h_name);
    hPtr->hostNo = -1;

    for (hPtr->naddr = 0;
         hp->h_addr_list && hp->h_addr_list[hPtr->naddr] != NULL;
         hPtr->naddr++); // empty for body

    if (hPtr->naddr)
        {
        hPtr->addr = malloc (hPtr->naddr * sizeof (u_int));
        }
    else {
        hPtr->addr = 0;
    }

    for (i = 0; i < hPtr->naddr; i++) {
        memcpy ((char *) &hPtr->addr[i], hp->h_addr_list[i], hp->h_length); // FIXME FIXME FIXME try to get rid of cast
    }

    for (unsigned int i = 0; i < 8; i++) {  // FIXME FIXME FIXME FIXME 8 is awfully specific
        hPtr->week[i] = NULL;
    }

    hPtr->windows = putstr_ ("-");
    hPtr->wind_edge = 0;

    if (myClusterPtr->clientList == NULL)
        {

        myClusterPtr->clientList = hPtr;
        hPtr->nextPtr = NULL;
        }
    else
        {

        for (lastHPtr = myClusterPtr->clientList; lastHPtr->nextPtr != NULL;
             lastHPtr = lastHPtr->nextPtr); // empty for body
        lastHPtr->nextPtr = hPtr;
        hPtr->nextPtr = NULL;
        }

    hPtr->hostInactivityCount = -1;
    numofhosts++;

    return hPtr;
}

int
removeFloatClientHost (struct hostNode *hPtr)
{
    struct hostNode *tempPtr = NULL;

    if (logclass & LC_TRACE)
        {
        ls_syslog (LOG_DEBUG, "%s Entering ... ", __func__);
        }

    if (hPtr == NULL)
        {
        /* catgets 5408 */
        ls_syslog (LOG_ERR, "5408: %s: hostNode is invalid", __func__);
        return -1;
        }

    if (myClusterPtr->clientList == hPtr)
        {

        myClusterPtr->clientList = hPtr->nextPtr;
        }
    else
        {

        for (tempPtr = myClusterPtr->clientList; tempPtr && tempPtr->nextPtr != hPtr; tempPtr = tempPtr->nextPtr)
        { 
            ;
        }
        if (tempPtr == NULL)
        {
            /* catgets 5409 */                
            ls_syslog (LOG_ERR, "5409: %s: host <%s> not found in client list", __func__, hPtr->hostName);
            return -1;
        }
        tempPtr->nextPtr = hPtr->nextPtr;
    }
    hPtr->nextPtr = NULL;


    numofhosts--;
    freeHostNodes (hPtr, FALSE);

    return 0;
}


struct hostNode *
initHostNode (void)
{
    struct hostNode *hPtr = calloc (1, sizeof (struct hostNode));
    if (hPtr == NULL)
        {
        ls_syslog (LOG_ERR, "%s: malloc failed", __func__);
        return NULL;
        }

    hPtr->resBitMaps    = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));
    hPtr->DResBitMaps   = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));
    hPtr->status        = calloc ((1 + GET_INTNUM (allInfo.numIndx)), sizeof (int));
    hPtr->loadIndex     = calloc (allInfo.numIndx, sizeof (float));
    hPtr->uloadIndex    = calloc (allInfo.numIndx, sizeof (float));
    hPtr->busyThreshold = calloc (allInfo.numIndx, sizeof (float));

    for (unsigned int i = 0; i < allInfo.numIndx; i++)
        {
        hPtr->loadIndex[i] = INFINIT_LOAD;
        hPtr->uloadIndex[i] = INFINIT_LOAD;
        }

    for ( unsigned int i = NBUILTINDEX; i < allInfo.numIndx; i++) {
        hPtr->busyThreshold[i] = (allInfo.resTable[i].orderType == INCR) ? INFINIT_LOAD : -INFINIT_LOAD;
    }

    hPtr->use = -1;
    hPtr->expireTime = -1;
    hPtr->status[0] = LIM_UNAVAIL;

    return hPtr;
}

void
freeHostNodes (struct hostNode *hPtr, int allList)
{
    struct hostNode *next = NULL;

    while (hPtr)
        {

        FREEUP (hPtr->hostName);
        FREEUP (hPtr->addr);
        FREEUP (hPtr->windows);

        if (allList == FALSE)
            {
            for ( unsigned int i = 0; i < 8; i++) { // FIXME FIXME FIXM FIXME '8' is awfully particular
                FREEUP (hPtr->week[i]);
            }
        }
        FREEUP (hPtr->busyThreshold);
        FREEUP (hPtr->loadIndex);
        FREEUP (hPtr->uloadIndex);
        FREEUP (hPtr->resBitMaps);
        FREEUP (hPtr->DResBitMaps);
        FREEUP (hPtr->status);
        FREEUP (hPtr->instances);

        next = hPtr->nextPtr;
        FREEUP (hPtr);

        if (allList == TRUE) {
            hPtr = next;
        }
    }

    return;
}


struct sharedResource *
addResource ( const char *resName, unsigned int nHosts, char **hosts, char *value, const char *fileName, unsigned int lineNum, int resourceMap)
{

    struct sharedResource *temp   = NULL;
    struct sharedResource **temp1 = NULL;


    assert( *fileName );
    assert( lineNum );

    if (resName == NULL || hosts == NULL) {
        return NULL;
    }

    temp = (struct sharedResource *) malloc (sizeof (struct sharedResource));
    if (NULL == temp && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return NULL;
    }
    if ((temp->resourceName = putstr_ (resName)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return NULL;
    }
    temp->numInstances = 0;
    temp->instances = NULL;
    if (addHostInstance (temp, nHosts, hosts, value, resourceMap) < 0) {
        return NULL;
    }

    if (numHostResources == 0) {
        temp1 =(struct sharedResource **) malloc (sizeof (struct sharedResource *));
    }
    else {
        temp1 = (struct sharedResource **) realloc (hostResources, (numHostResources + 1) * sizeof (struct sharedResource *));
    }

    if (temp1 == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        freeSharedRes (temp);
        for ( unsigned int i = 0; i < numHostResources; i++) {
            freeSharedRes (hostResources[i]);
        }
        FREEUP (hostResources);
        return NULL;
    }
    hostResources = temp1;
    hostResources[numHostResources] = temp;
    numHostResources++;
    return temp;
}

void
freeSharedRes (struct sharedResource *sharedRes)
{
    if (sharedRes == NULL) {
        return;
    }
    FREEUP (sharedRes->resourceName);

    for ( unsigned int i = 0; i < sharedRes->numInstances; i++) {
        freeInstance (sharedRes->instances[i]);
    }
    FREEUP (sharedRes);

    return;
}

// addHostInstance: moved over to lib/liblsf/conf.c
// int
// addHostInstance (struct sharedResource *sharedResource, unsigned int nHosts, char **hostNames, char *value, int resourceMap)
// {
//  unsigned int numList  = 0;
//  unsigned int numHosts = 0;
//  char **temp           = NULL;
//  char **hostList       = NULL ;
//  struct resourceInstance *instance = NULL;

//  const char addInstance[] = "addInstance";
//  const char addHostList[] = "addHostList";

//  if (nHosts <= 0 || hostNames == NULL) {
//      return -1;
//  }

//  if (resourceMap == FALSE) {
//      if (sharedResource->numInstances > 1) {
//          /* catgets 5405 */
//          ls_syslog (LOG_ERR, "5405: %s: More than one instatnce defined for the resource <%s> on host <%s> in host section; ignoring", __func__, sharedResource->resourceName, hostNames[0]);
//          return -1;
//      }
//      if (sharedResource->numInstances == 0) {
//          if (addInstance (sharedResource, nHosts, hostNames, value) == NULL) {
//              ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, addInstance);
//              return -1;
//          }
//      }
//      else {
//          if (addHostList (sharedResource->instances[0], nHosts, hostNames) < 0) {
//              ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, addHostList);
//              return -1;
//           a}
//      }
//      instance = sharedResource->instances[0];

//      if (addHostNodeIns (instance, nHosts, hostNames) < 0) {
//          return -1;
//      }
//  }
//  else {
//      if (numHosts == 0 && temp == NULL) {
//          temp = malloc (numofhosts * sizeof (char *));
//          if(NULL == temp && ENOMEM == errno ) {
//              ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
//              return -1;
//          }
//      }
//      else {
//          for ( unsigned int i = 0; i < numHosts; i++) {
//              FREEUP (temp[i]);
//          }
//      }
//      numHosts = 0;
//      for (unsigned int i = 0; i < nHosts; i++) {

//          if ((hostList = getValidHosts (hostNames[i], &numList, sharedResource)) == NULL) {
//              continue;
//          }
//          for ( unsigned int j = 0; j < numList; j++) {

//              int duplicated = 0;
//              for ( unsigned int k = 0; k < numHosts; k++)  {
//                  if (!strcmp (temp[k], hostList[j])) {
//                      duplicated = 1;
//                      break;
//                  }
//              }
//              if (duplicated) {
//                  /* catgets 5478 */
//                  ls_syslog (LOG_WARNING, "5478: %s: Host %s is duplicated in resource %s mapping.", __func__, hostList[j], sharedResource->resourceName);
//                  continue;
//              }
//              temp[numHosts] = putstr_ (hostList[j]);
//              if (temp[numHosts] == NULL) {
//                  ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
//                  return -1;
//              }
//              numHosts++;
//          }
//      }
//      if ((instance = addInstance (sharedResource, numHosts, temp, value)) == NULL) {
//          const char addInstance[] = "addInstance";
//          ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, addInstance );
//          return -1;
//          }

//      if (addHostNodeIns (instance, numHosts, temp) < 0) {
//          return -1;
//      }

//      if (addSharedResourceInstance( numHosts, temp, sharedResource->resourceName) < 0) {
//          const char addSharedResourceInstance = "addSharedResourceInstance";
//          ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__,  addSharedResourceInstance);
//          return -1;
//      }
//  }
//  return 0;
// }

char **
getValidHosts ( const char *hostName, unsigned int *numHost, struct sharedResource *resource)
{
    int num               = 0;
    char **temp           = NULL;
    struct hostNode *hPtr = NULL;

    assert( *numHost );
    if (temp == NULL)
        {
        if ((temp = malloc (numofhosts * sizeof (char *))) == NULL)
            {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            return NULL;
            }
        }

    if (!strcmp (hostName, "all") || !strcmp (hostName, "others"))
        {
        if (resource->numInstances > 0 && !strcmp (hostName, "all"))
            {
            ls_syslog (LOG_ERR, "5349: %s: Shared resource <%s> has more than one instance, reserved word <all> can not be specified;ignoring", __func__, resource->resourceName);   /* catgets 5349 */
            return NULL;
            }
        for (hPtr = myClusterPtr->hostList; hPtr; hPtr = hPtr->nextPtr)
            {
            if (isInHostList (resource, hPtr->hostName) == NULL)
                {
                temp[*numHost] = hPtr->hostName;
                (*numHost)++;
                }
            }
        return temp;
        }

    if (Gethostbyname_ (hostName))
        {

        hPtr = findHostbyList (myClusterPtr->hostList, hostName);
        if (hPtr == NULL)
            {
            ls_syslog (LOG_ERR, "%s: Host %s is not used by cluster %s; ignored", __func__, hostName, myClusterName);
            return NULL;
            }
        if (isInHostList (resource, hostName) != NULL)
            {
            ls_syslog (LOG_ERR, "%s: Host %s is defined in more than one instance for resource %s; ignored", __func__, hostName, resource->resourceName);
            return NULL;
            }
        *numHost = 1;
        temp[0] = hPtr->hostName;
        return temp;
        }

    if ((num = typeNameToNo (hostName)) > 0)
        {
        for (hPtr = myClusterPtr->hostList; hPtr; hPtr = hPtr->nextPtr)
            {
            if (hPtr->hTypeNo == num
                && isInHostList (resource, hPtr->hostName) == NULL)
                {
                temp[*numHost] = hPtr->hostName;
                (*numHost)++;
                }
            }
        return temp;
        }

    if ((num = modelNameToNo (hostName)) > 0)
        {
        for (hPtr = myClusterPtr->hostList; hPtr; hPtr = hPtr->nextPtr)
            {
            if (hPtr->hModelNo == num
                && isInHostList (resource, hPtr->hostName) == NULL)
                {
                temp[*numHost] = hPtr->hostName;
                (*numHost)++;
                }
            }
        return temp;
        }
    return NULL;
}

int
addHostNodeIns (struct resourceInstance *instance, unsigned int nHosts, char **hostNames)
{
    int resNo                      = 0;
    struct hostNode *hPtr          = NULL;
    struct resourceInstance **temp = NULL;

    if ((resNo = resNameDefined (instance->resName)) < 0)
        {
        ls_syslog (LOG_ERR, "%s: Resource name <%s> is not defined in resource section in lsf.shared", __func__, instance->resName);
        return 0;
        }

    for ( unsigned int i = 0; i < nHosts; i++)
        {

        if (hostNames[i] == NULL) {
            continue;
        }

        if (!Gethostbyname_ (hostNames[i]))
            {
            ls_syslog (LOG_ERR, "%s: Invalid hostname %s ", __func__, hostNames[i]);
            continue;
            }

        hPtr = findHostbyList (myClusterPtr->hostList, hostNames[i]);
        if (hPtr == NULL)
            {
            ls_syslog (LOG_WARNING, "%s: Host %s is not defined in host sectionin lsf.cluster", __func__, hostNames[i]);
            continue;
            }

        if (hPtr->numInstances > 0 && isInHostNodeIns (instance->resName, hPtr->numInstances, hPtr->instances) != NULL) {
            continue;
        }

        if (hPtr->numInstances > 0) {
            temp = realloc (hPtr->instances, (hPtr->numInstances + 1) * sizeof (struct resourceInstance *));
        }
        else {
            temp = malloc (sizeof (struct resourceInstance *));
        }

        temp[hPtr->numInstances] = instance;
        hPtr->instances = temp;
        hPtr->numInstances++;
        }
    return 0;
}

struct resourceInstance *
isInHostNodeIns ( const char *resName, unsigned int numInstances, struct resourceInstance **instances)
{
    if (numInstances <= 0 || instances == NULL) {
        return NULL;
    }
    for (unsigned int i = 0; i < numInstances; i++) {
        if (!strcmp (resName, instances[i]->resName)) {
            return instances[i];
        }
    }
    return NULL;
}

int
addHostList (struct resourceInstance *resourceInstance, unsigned int nHosts, char **hostNames)
{
    struct hostNode *hostPtr = NULL;
    struct hostNode **temp   = NULL;

    if (resourceInstance == NULL || nHosts <= 0 || hostNames == NULL) {
        return -1;
    }

    if (resourceInstance->nHosts == 0) {
        temp = (struct hostNode **) malloc (nHosts * sizeof (struct hostNode *));
    }
    else{
        temp = (struct hostNode **) realloc (resourceInstance->hosts, (resourceInstance->nHosts + 1) * sizeof (struct hostNode *));
    }

    if (temp == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return -1;
    }
    resourceInstance->hosts = temp;

    for ( unsigned int i = 0; i < nHosts; i++) {
        if ((hostPtr = findHostbyList (myClusterPtr->hostList, hostNames[i])) == NULL) {
            ls_syslog (LOG_DEBUG3, "addHostList: Host <%s> is not used by cluster <%s> as a server:ignoring", hostNames[i], myClusterName);
            continue;
        }
        resourceInstance->hosts[resourceInstance->nHosts] = hostPtr;
        resourceInstance->nHosts++;
    }
    return 0;

}

struct resourceInstance *
addInstance (struct sharedResource *sharedResource, unsigned int nHosts, char **hostNames, char *value)
{
    int resNo = 0;
    struct hostNode *hPtr            = NULL;
    struct resourceInstance *temp    = NULL;
    struct resourceInstance **insPtr = NULL; 

    if (nHosts <= 0 || hostNames == NULL) {
        return NULL;
    }

    if ((temp = initInstance ()) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return NULL;
        }
    if ((temp->hosts =(struct hostNode **) malloc (nHosts * sizeof (struct hostNode *))) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return NULL;
        }
    temp->resName = sharedResource->resourceName;
    resNo = validResource (temp->resName);
    for ( unsigned int i = 0; i < nHosts; i++)
        {
        if (hostNames[i] == NULL) {
            continue;
        }

        if ((hPtr = findHostbyList (myClusterPtr->hostList, hostNames[i])) == NULL)
            {
            ls_syslog (LOG_DEBUG3, "addInstance: Host <%s> is not used by cluster <%s> as server;ignoring", hostNames[i], myClusterName);
            continue;
            }
        temp->hosts[temp->nHosts] = hPtr;
        temp->nHosts++;
        }
    if (value[0] == '\0') {
        strcpy (value, "-");
    }
    if ((temp->value = putstr_ (value)) == NULL || (temp->orignalValue = putstr_ (value)) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        freeInstance (temp);
        return NULL;
        }
    if ((insPtr = (struct resourceInstance **) myrealloc (sharedResource->instances, (sharedResource->numInstances + 1) * sizeof (char *))) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myrealloc");
        freeInstance (temp);
        return NULL;
        }
    sharedResource->instances = insPtr;
    sharedResource->instances[sharedResource->numInstances] = temp;
    sharedResource->numInstances++;

    return temp;
}


struct resourceInstance *
initInstance (void)
{
    struct resourceInstance *temp;

    if ((temp = (struct resourceInstance *) malloc (sizeof (struct resourceInstance))) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return NULL;
    }
    temp->nHosts       = 0;
    temp->resName      = NULL;
    temp->hosts        = NULL;
    temp->orignalValue = NULL;
    temp->value        = NULL;
    temp->updateTime   = 0;
    temp->updHost      = NULL;

    return temp;
}

void
freeInstance (struct resourceInstance *instance)
{
    if (instance == NULL) {
        return;
    }
    FREEUP (instance->hosts);
    FREEUP (instance->orignalValue);
    FREEUP (instance->value);
    FREEUP (instance)
}

struct resourceInstance *
isInHostList (struct sharedResource *sharedResource, const char *hostName)
{
    if (sharedResource->numInstances <= 0) {
        return NULL;
    }
    
    for ( unsigned int i = 0; i < sharedResource->numInstances; i++) {
        if (sharedResource->instances[i]->nHosts <= 0 || sharedResource->instances[i]->hosts == NULL) {
            continue;
        }
        for ( unsigned int j = 0; j < sharedResource->instances[i]->nHosts; j++) {
            if (strcmp(sharedResource->instances[i]->hosts[j]->hostName, hostName) == 0) {
                return sharedResource->instances[i];
            }
        }
    }
    return NULL;
}

// struct sharedResource *inHostResourcs ( const char *resName) used to live here
//      since there are no referneces for it in the limd source code, it was moved to 
//      its own file under lsf/lib/liblsf/inHostResourcess
int
validResource (const char *resName)
{
    for ( unsigned int i = 0; i < shortInfo.nRes; i++) {
        if (strcmp (shortInfo.resName[i], resName) == 0) {
            assert( i <= INT_MAX );
            return inti;
        }
    }

    return -1;
}

int
validLoadIndex (const char *resName)
{
    for ( unsigned int i = 0; i < allInfo.numIndx; i++) {
        if (strcmp (li[i].name, resName) == 0) {
            assert( i <= INT_MAX );
            return inti;
        }
    }

    return -1;
}


bool_t
validHostType (const char *hType)
{
    for ( unsigned int i = 0; i < shortInfo.nTypes; i++) {
        if (strcmp (shortInfo.hostTypes[i], hType) == 0) {
            assert( i <= INT_MAX );
            return inti;
        }
    }
    return -1;
}

int
validHostModel (const char *hModel)
{
    for ( unsigned int i = 0; i < allInfo.nModels; i++) {
        if (strcmp (allInfo.hostModels[i], hModel) == 0) {
            assert( i <= INT_MAX );
            return inti;
        }
    }
    return -1;
}



char
lim_addHostType ( const char *type) // duplicate function name from liblsf: ls_addHostType
{
    int cntofdefault = 0;

    if (allInfo.nTypes == MAX_TYPES)
        {
        /* catgets 5353 */
        ls_syslog (LOG_ERR, "5353: %s: Too many host types defined in section HostType. You can only define up to %d host types; host type %s ignored", __func__, MAX_TYPES, type);
        return FALSE;
        }

    for ( unsigned int i = 0; i < allInfo.nTypes; i++) {
        if (strcmp (allInfo.hostTypes[i], type) != 0) {
            continue;
        }
        if (strcmp (type, "DEFAULT") == 0) {
            cntofdefault++;
            if (cntofdefault <= 1) {
                break;
            }
        }
        /* catgets 5354 */
        ls_syslog (LOG_ERR, "5354: %s: host type %s multiply defined", __func__, type);
        return FALSE;
    }
    strcpy (allInfo.hostTypes[allInfo.nTypes], type);
    shortInfo.hostTypes[shortInfo.nTypes] = allInfo.hostTypes[allInfo.nTypes];
    allInfo.nTypes++;
    shortInfo.nTypes++;
    return TRUE;
}

char
lim_addHostModel ( const char *model, const char *arch, double factor) // duplicate function name from liblsf: ls_addHostModel
{

    if (allInfo.nModels == MAX_MODELS) { // FIXME FIXME FIXME remove this limitation
        /* catgets 5355 */
        ls_syslog (LOG_ERR, "5355: %s: Too many host models defined in section HostModel. You can only define up to %d host models; host model %s ignored", __func__, MAX_MODELS, model);
        return FALSE;
        }

    if (!strcmp (model, "DEFAULT")) {
        strcpy (allInfo.hostArchs[1], arch ? arch : "");
        allInfo.cpuFactor[1] = factor;
        return TRUE;
    }

    for ( unsigned int i = 0; i < allInfo.nModels; ++i) {

        if (!arch && strcmp (allInfo.hostModels[i], model) == 0) {
            /* catgets 5357 */
            ls_syslog (LOG_ERR, "5357: %s: host model %s multiply defined", __func__, model);
            return TRUE;
        }

        if (!arch || strcmp (allInfo.hostArchs[i], arch) != 0)  {
            continue;
        }

        /*catgets 5479 */
        ls_syslog (LOG_ERR, "5479: %s: host architecture %s defined multiple times", __func__, arch);
        
        return TRUE;
    }

    strcpy (allInfo.hostModels[allInfo.nModels], model);
    strcpy (allInfo.hostArchs[allInfo.nModels], arch ? arch : "\0" );
    allInfo.cpuFactor[allInfo.nModels] = factor;
    shortInfo.hostModels[shortInfo.nModels] = allInfo.hostModels[allInfo.nModels];
    allInfo.nModels++;
    shortInfo.nModels++;
    return TRUE;
}

struct clusterNode *
addCluster ( const char *clName, char *candlist)
{
    char *sp            = NULL;
    char *word          = NULL;
    struct hostent *hp  = NULL;
    unsigned int nextClNo = 0;
    int counter         = 0;
    
    if (myClusterPtr != NULL) {
        ls_syslog (LOG_ERR, "%s: Ignoring duplicate cluster %s", __func__, clName);
        return NULL;
    }
    
    myClusterPtr = calloc (1, sizeof (struct clusterNode));
    
    myClusterPtr->clName = putstr_ (clName);
    myClusterPtr->clusterNo = nextClNo++;
    
    if (!candlist || candlist[0] == '\0') {
        candlist = findClusterServers (clName);
    }
    
    counter = 0;
    sp = candlist;
    while ((word = getNextWord_ (&sp)) != NULL && counter < MAXCANDHOSTS) {
        
        hp = Gethostbyname_ (word);
        if (!hp) {
            ls_syslog (LOG_ERR, "%s: Invalid host %s for cluster %s, ignoring", __func__, word, clName);
            lim_CheckError = WARNING_ERR;
            continue;
        }
        myClusterPtr->candAddrList[counter] = *(in_addr_t *) hp->h_addr_list[0];
        counter++;
    }
    
    myClusterPtr->candAddrList[counter] = 0;
    myClusterPtr->status                = CLUST_ACTIVE | CLUST_STAT_UNAVAIL;
    myClusterPtr->masterKnown           = FALSE;
    myClusterPtr->masterInactivityCount = 0;
    myClusterPtr->masterPtr             = NULL;
    myClusterPtr->prevMasterPtr         = NULL;
    myClusterPtr->hostList              = NULL;
    myClusterPtr->clientList            = NULL;
    myClusterPtr->eLimArgs              = NULL;
    myClusterPtr->eLimArgv              = NULL;
    myClusterPtr->currentAddr           = 0;
    myClusterPtr->masterName            = NULL;
    myClusterPtr->managerName           = NULL;
    myClusterPtr->resClass              = 0;
    myClusterPtr->typeClass             = 0;
    myClusterPtr->modelClass            = 0;
    myClusterPtr->chanfd                = -1;
    myClusterPtr->numIndx               = 0;
    myClusterPtr->numUsrIndx            = 0;
    myClusterPtr->usrIndxClass          = 0;
    myClusterPtr->nAdmins               = 0;
    myClusterPtr->adminIds              = NULL;
    myClusterPtr->admins                = NULL;
    myClusterPtr->nRes                  = 0;
    myClusterPtr->resBitMaps            = NULL;
    myClusterPtr->hostTypeBitMaps       = NULL;
    myClusterPtr->hostModelBitMaps      = NULL;
    
    return myClusterPtr;
}

char *
findClusterServers ( const char *clName)
{
    FILE *clfp           = NULL;
    char *cp             = NULL ;
    char *word           = NULL;
    char *linep          = NULL;
    char *servers        = NULL;
    unsigned int lineNum = 0;

    char fileName[MAX_FILENAME_LEN];
    const char lsf_cluster[] = "lsf.cluster";
    const char ENOSERVER[]   = "ENOSERVER";
    const char readonly[]    = "r";
    
    servers = malloc( sizeof( char ) * MAX_LINE_LEN + 1 ); // FIXME FIXME FIXME FIXME memory management
    memset( servers, 0, sizeof( char ) * MAX_LINE_LEN + 1 );
    strcpy( servers, ENOSERVER );
    assert( strlen( limParams[LSF_CONFDIR].paramValue ) + strlen( lsf_cluster ) + strlen( clName ) =< MAX_FILENAME_LEN );
    sprintf (fileName, "%s/%s.%s", limParams[LSF_CONFDIR].paramValue, lsf_cluster ,clName); // FIXME FIXME FIXME FIXME const struct parameter; should be moved to configure.ac

    if ((clfp = confOpen (fileName, "r")) == NULL)
    {
        return servers;
    }
    
    for (;;) { // FIXME FIXME FIXME FIXME remove infinite loop

        const char host[] = "host"
        cp = getBeginLine (clfp, &lineNum);
        if (!cp)
        {
            FCLOSEUP (&clfp);
            return servers;
        }
        word = getNextWord_ (&cp);
        if (!word)
        {
            const char unknown[] = "unknown";
            /* catgets 5360 */
            ls_syslog (LOG_ERR, "5360: %s: %s(%d): Keyword expected after Begin, ignoring section", __func__, fileName, lineNum);
            lim_CheckError = WARNING_ERR;
            doSkipSection (clfp, &lineNum, fileName, unknown );
        }
        else if (strcasecmp (word, host) == 0)
        {
            char first = TRUE;
            int nServers = 0;
            
            
            while ((linep = getNextLineC_ (clfp, &lineNum, TRUE)) != NULL)
            {
                if (isSectionEnd (linep, fileName, &lineNum, host ) )
                {
                    FCLOSEUP (&clfp);
                    return servers;
                }
                
                if (first)
                {
                    first = FALSE;
                    continue;
                }
                cp = linep;
                word = getNextWord_ (&cp);
                if (word)
                {
                    nServers++;
                    strcat (servers, word);
                    strcat (servers, " ");
                }
                if (nServers > MAXCANDHOSTS)
                    break;
            }
            FCLOSEUP (&clfp);
            return servers;
        }
        else
        {
            doSkipSection (clfp, &lineNum, fileName, word);
        }
    }

    fprintf( stderr, "%s: you are not suppposed to be here.", __func__ );
    ls_syslog( LOG_ERR, "%s: you are not suppposed to be here.", __func__ );
    return NULL;
}


void
reCheckRes (void)
{
    unsigned int j           = 0;
    struct resItem *newTable = NULL;

    allInfo.numIndx = 0;
    newTable = calloc (allInfo.nRes, sizeof (struct resItem));
    if (newTable == NULL) {
        lim_Exit (__func__);
    }

    for (unsigned int i = 0; i < allInfo.nRes; i++) {
        if (allInfo.resTable[i].valueType == LS_NUMERIC && (allInfo.resTable[i].flags & RESF_DYNAMIC) && (allInfo.resTable[i].flags & RESF_GLOBAL)) {
            memcpy (&newTable[j], &allInfo.resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    for (unsigned int i = 0; i < allInfo.nRes; i++) {
        
        if (allInfo.resTable[i].valueType == LS_NUMERIC && (!(allInfo.resTable[i].flags & RESF_DYNAMIC) || !(allInfo.resTable[i].flags & RESF_GLOBAL))) {
            memcpy (&newTable[j], &allInfo.resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    for (unsigned int i = 0; i < allInfo.nRes; i++) {
        if (allInfo.resTable[i].valueType == LS_BOOLEAN) {
            memcpy (&newTable[j], &allInfo.resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    for (unsigned int i = 0; i < allInfo.nRes; i++) {
        if (allInfo.resTable[i].valueType == LS_STRING) {
            memcpy (&newTable[j], &allInfo.resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    free (allInfo.resTable);
    allInfo.resTable = newTable;

    shortInfo.nRes = 0;
    shortInfo.resName = calloc (allInfo.nRes, sizeof (char *));
    shortInfo.stringResBitMaps  = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));
    shortInfo.numericResBitMaps = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));

    if (shortInfo.resName == NULL || shortInfo.stringResBitMaps == NULL || shortInfo.numericResBitMaps == NULL) {
        lim_Exit (__func__);
    }

    for ( unsigned int resNo = 0; resNo < allInfo.nRes; resNo++) {

        if ((allInfo.resTable[resNo].flags & RESF_DYNAMIC) && (allInfo.resTable[resNo].valueType == LS_NUMERIC) && (allInfo.resTable[resNo].flags & RESF_GLOBAL)) {
            allInfo.numIndx++;
        }

        if ((allInfo.resTable[resNo].flags & RESF_BUILTIN) || (allInfo.resTable[resNo].flags & RESF_DYNAMIC && allInfo.resTable[resNo].valueType == LS_NUMERIC)  || (allInfo.resTable[resNo].valueType != LS_STRING && allInfo.resTable[resNo].valueType != LS_BOOLEAN)) {
            continue;
        }

        shortInfo.resName[shortInfo.nRes] = putstr_ (allInfo.resTable[resNo].name);
        if (shortInfo.resName[shortInfo.nRes] == NULL) {
            lim_Exit (__func__);
        }

        if (allInfo.resTable[resNo].valueType == LS_STRING) SET_BIT (shortInfo.nRes, shortInfo.stringResBitMaps) {
            ;
        }
        shortInfo.nRes++;
    }
    shortInfo.nModels = allInfo.nModels;
    for ( unsigned int i = 0; i < allInfo.nModels; i++) {
        shortInfo.hostModels[i] = allInfo.hostModels[i];
        shortInfo.cpuFactors[i] = allInfo.cpuFactor[i];
    }

    return;
}

int
reCheckClusterClass (struct clusterNode *clPtr)
{
    struct hostNode *hPtr = NULL;
    char malloc[] = "malloc";
    
    clPtr->resClass     = 0;
    clPtr->typeClass    = 0;
    clPtr->modelClass   = 0;
    clPtr->numHosts     = 0;
    clPtr->numClients   = 0;
    clPtr->nRes         = 0;

    ls_syslog (LOG_DEBUG1, "%s: cluster <%s>", __func__, clPtr->clName); // FIXME FIXME rename variables to clusterPtr and clusterName, respectively
    if (clPtr->resBitMaps == NULL) {

        clPtr->resBitMaps = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));
        if (clPtr->resBitMaps == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, malloc);
            return -1;
        }

        for (unsigned int i = 0; i < GET_INTNUM (allInfo.nRes); i++)
            clPtr->resBitMaps[i] = 0;
        }

    if (clPtr->hostTypeBitMaps == NULL) {
        clPtr->hostTypeBitMaps = calloc (GET_INTNUM (allInfo.nTypes), sizeof (int));
        if ( NULL == clPtr->hostTypeBitMaps && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, malloc);
            return -1;
        }
        for ( unsigned int i = 0; i < GET_INTNUM (allInfo.nTypes); i++) {
            clPtr->hostTypeBitMaps[i] = 0;
        }
    }

    if (clPtr->hostModelBitMaps == NULL) {
        clPtr->hostModelBitMaps =
        calloc (GET_INTNUM (allInfo.nModels), sizeof (int));
        if (clPtr->hostModelBitMaps == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, malloc );
            return -1;
        }

        for (unsigned int i = 0; i < GET_INTNUM (allInfo.nModels); i++)
            clPtr->hostModelBitMaps[i] = 0;
        }

    for (hPtr = clPtr->hostList; hPtr; hPtr = hPtr->nextPtr) {
        clPtr->numHosts++;
        clPtr->resClass |= hPtr->resClass;
        if (hPtr->hTypeNo >= 0) {
            clPtr->typeClass |= (1 << hPtr->hTypeNo);
            SET_BIT (hPtr->hTypeNo, clPtr->hostTypeBitMaps);
        }
        if (hPtr->hModelNo >= 0) {
            clPtr->modelClass |= (1 << hPtr->hModelNo);
            SET_BIT (hPtr->hModelNo, clPtr->hostModelBitMaps);
        }

        assert( allInfo.nRes <= INT_MAX );
        addMapBits ( (int)allInfo.nRes, clPtr->resBitMaps, hPtr->resBitMaps);
    }

    for (hPtr = clPtr->clientList; hPtr; hPtr = hPtr->nextPtr) {
        clPtr->numClients++;
        clPtr->resClass |= hPtr->resClass;
        if (hPtr->hTypeNo >= 0) {
            clPtr->typeClass |= (1 << hPtr->hTypeNo);
            SET_BIT (hPtr->hTypeNo, clPtr->hostTypeBitMaps);
        }

        if (hPtr->hModelNo >= 0) {
            clPtr->modelClass |= (1 << hPtr->hModelNo);
            SET_BIT (hPtr->hModelNo, clPtr->hostModelBitMaps);
        }
        assert( allInfo.nRes <= INT_MAX );
        addMapBits ((int)allInfo.nRes, clPtr->resBitMaps, hPtr->resBitMaps);
    }

    for ( unsigned int i = 0; i < GET_INTNUM (allInfo.nRes); i++) {
        for ( unsigned int j = 0; j < INTEGER_BITS; j++) {
            if (clPtr->resBitMaps[i] & (1 << j)) {
                clPtr->nRes++;
            }
        }
    }

    return 0;
}

void
addMapBits (int num, unsigned int *toBitMaps, unsigned int *fromMaps)
{
   
    for( unsigned int j = 0; j < GET_INTNUM( num ); j++ )
    {
        toBitMaps[j] = ( toBitMaps[j] | fromMaps[j] );
    }
}

int
reCheckClass (void)
{
    if (reCheckClusterClass (myClusterPtr) < 0) {
        return -1;
    }
    
    return 0;
}

int
configCheckSum ( const char *file, int *checkSum)
{
    FILE *fp   = NULL;
    char *line = NULL;
    unsigned int sum = 0;
    char readonly[] = "r";
    
    if ((fp = confOpen (file, readonly )) == NULL)
        {
        ls_syslog ( LOG_ERR, "%s: open() failed on %s", __func__, file );
        return -1;
        }
    
    sum = 0;
    while ((line = getNextLine_ (fp, TRUE)) != NULL)
        {
        int i = 0;
        int linesum = 0;
        
        while (line[i] != '\0')
            {
            if (line[i] == ' ' || line[i] == '\t' ||
                line[i] == '(' || line[i] == ')' ||
                line[i] == '[' || line[i] == ']')
                {
                i++;
                continue;
                }
            linesum += (int) line[i];
            i++;
            }
        
        for (i = 0; i < 4; i++)
            {
            if (sum & 01) {
                sum = (sum >> 1) + 0x8000;
            }
            else {
                sum >>= 1;
            }
            sum += linesum & 0xFF;
            sum &= 0xFFFF;
            linesum = linesum >> 8;
            }
        }
    FCLOSEUP (&fp);
    *checkSum = sum;
    
    return 0;
}

struct admins *
lim_getAdmins (const char *line, const char *lsfile, const unsigned int *lineNum, const char *secName)
{
    char *sp              = NULL;
    char *word            = NULL;
    char *forWhat         = "for LSF administrator";
    char malloc[]         = "malloc";
    int first             = TRUE;
    unsigned int numAds   = 0;
    struct passwd *pw     = NULL;
    struct group *unixGrp = NULL;
    struct admins *admins = malloc( sizeof ( *admins ) );
s
    assert( *lsfile );
    assert( *lineNum );
    assert( *secName );

    if (first == FALSE) {
        for (unsigned int i = 0; i < admins->nAdmins; i++) {
            FREEUP (admins->adminNames[i]);
        }
        FREEUP (admins->adminNames);
        FREEUP (admins->adminIds);
        FREEUP (admins->adminGIds);
    }
    first = FALSE;
    admins->nAdmins = 0;
    sp = strdup( line );

    while ((word = getNextWord_ (&sp)) != NULL) {
        numAds++;
    }
    if (numAds)
        {
        admins->adminIds   = malloc (numAds * sizeof (uid_t));
        admins->adminGIds  = malloc (numAds * sizeof (gid_t));
        admins->adminNames = malloc (numAds * sizeof (char *));
        if ( ( NULL == admins->adminIds && ENOMEM == errno) || (NULL == admins->adminGIds && ENOMEM == errno ) || (NULL == admins->adminNames && ENOMEM == errno ) ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, malloc);
            FREEUP (admins->adminIds);
            FREEUP (admins->adminGIds);
            FREEUP (admins->adminNames);
            admins->nAdmins = 0;
            lserrno = LSE_MALLOC;
            return admins;
        }
    }
    else {
        return admins;
    }

    sp = strdup( line );
    while ((word = getNextWord_ (&sp)) != NULL) {

        if ((pw = getpwlsfuser_ (word)) != NULL)
            {
            if (putInLists (word, admins, &numAds, forWhat) < 0) {
                return admins;
            }
        }
        else if ((unixGrp = getgrnam (word)) != NULL) {
            unsigned int i = 0;
            while (unixGrp->gr_mem[i] != NULL) {
                if (putInLists (unixGrp->gr_mem[i++], admins, &numAds, forWhat) < 0) {
                    return admins;
                }
            }
        }
        else {
            if (putInLists (word, admins, &numAds, forWhat) < 0) {
                return admins;
            }
        }
    }
    return admins;
}


int
doubleResTable ( const char *lsfile, const unsigned int lineNum) // FIXME FIXME FIXME function might need reconstruvtion: 
{
    struct resItem *tempTable = NULL;
    const char realloc[] = "realloc";

    assert( *lsfile );
    assert( lineNum );
    
    if (sizeOfResTable <= 0) {
        return -1;
    }
    
    tempTable = realloc (allInfo.resTable, 2 * sizeOfResTable * sizeof (struct resItem));
    if (tempTable == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, __func__, realloc, 2 * sizeOfResTable * sizeof (struct resItem));
        return -1;
    }
    allInfo.resTable = tempTable;
    sizeOfResTable *= 2;
    return 0;
}


FILE *
confOpen ( const char *filename, char *type)
{
    FILE *fp = NULL;
    int interval = 0;
    int max = 0;

    enum {
        DEFAULT_RETRY_MAX = 0,
        DEFAULT_RETRY_INT = 30
    };
    
    if (limParams[LSF_CONF_RETRY_MAX].paramValue) {
        max = atoi (limParams[LSF_CONF_RETRY_MAX].paramValue);
    }
    else {
        max = DEFAULT_RETRY_MAX;
    }
    
    if (limParams[LSF_CONF_RETRY_INT].paramValue) {
        interval = atoi (limParams[LSF_CONF_RETRY_INT].paramValue);
    }
    else {
        interval = DEFAULT_RETRY_INT;
    }
    
    for (;;) { // TODO TODO TODO TODO TODO remove inf

        fp = fopen (filename, type);
        if (fp != NULL) {
            break;
        }
        if (errno == ENOENT && max > 0) {
            float sleeptime = 0.0f;
            sleeptime = interval * mykey () * 1000;
            ls_syslog (LOG_ERR, "%s: still trying to open file %s", __func__, filename);
            assert( (int)sleeptime <= INT_MAX );
            millisleep_ ((int)sleeptime);
            max--;
            continue;
        }
        break;
    }

    return fp;
}

float
mykey (void)
{
    int sum          = 0;
    unsigned int i   = 0;
    float key        = 0.0f;
    char *myhostname = ls_getmyhostname ();
    
    for (i = 0; myhostname[i] != 0; i++) {
        sum += myhostname[i];
    }
    
    i = sum % 'z';
    if (i < 'A') {
        i += 'A';
    }
    
    key = (float) i / (float) 'Z';
    
    return key;
    
}

void
setExtResourcesDefDefault ( const char *resName)
{
    
    
    allInfo.resTable[allInfo.nRes].valueType = LS_STRING;
    
    
    allInfo.resTable[allInfo.nRes].flags |= RESF_EXTERNAL;
    
    
    allInfo.resTable[allInfo.nRes].interval = 60;
    allInfo.resTable[allInfo.nRes].flags |= RESF_DYNAMIC;
    
    
    strncpy (allInfo.resTable[allInfo.nRes].des, "Fail to get external defined value, set to default", MAXRESDESLEN);
    ls_syslog (LOG_INFO, "5370: %s: Fail to get external resource %s definition, set to default", __func__, resName );
    
    return;
    
}

int
setExtResourcesDef ( const char *resName)
{
    struct extResInfo *extResInfoPtr = NULL;
    int type = 0;
    
    if ((extResInfoPtr = getExtResourcesDef (resName)) == NULL) {
        setExtResourcesDefDefault (resName);
        return 0;
    }
    
    
    if ((type = validType (extResInfoPtr->type)) >= 0) {
        allInfo.resTable[allInfo.nRes].valueType =  (unsigned int) type;
    }
    else {
        /* catgets 5371 */
        ls_syslog (LOG_ERR, "5371: %s: type <%s> for external resource <%s> is not valid", __func__, extResInfoPtr->type, resName);
        setExtResourcesDefDefault (resName);
        return 0;
    }

    allInfo.resTable[allInfo.nRes].flags |= RESF_EXTERNAL;

    if (extResInfoPtr->interval != NULL && extResInfoPtr->interval[0] != '\0') {
        int interval;
        if ((interval = atoi (extResInfoPtr->interval)) > 0) {
            allInfo.resTable[allInfo.nRes].interval = interval;
            allInfo.resTable[allInfo.nRes].flags |= RESF_DYNAMIC;
        }
        else {
            /* catgets 5372 */
            ls_syslog (LOG_ERR, "5372: %s: interval <%s> for external resource <%s> should be a integer greater than 0", __func__, extResInfoPtr->interval, resName);
            setExtResourcesDefDefault (resName);
            return 0;
        }
    }
    
    
    if (extResInfoPtr->increasing != NULL && extResInfoPtr->increasing[0] != '\0') {
        if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC)  {
            if (!strcasecmp (extResInfoPtr->increasing, "N")) {
                allInfo.resTable[allInfo.nRes].orderType = DECR;
            }
            else
                {
                if (!strcasecmp (extResInfoPtr->increasing, "Y")) {
                    allInfo.resTable[allInfo.nRes].orderType = INCR;
                }
                else {
                    /* catgets 5373 */
                    ls_syslog (LOG_ERR, "5373: %s: increasing <%s> for resource <%s> is not valid", __func__, extResInfoPtr->increasing, resName);
                    setExtResourcesDefDefault (resName);
                    return 0;
                }
            }
        }
        else {
            /* catgets 5374 */
            ls_syslog (LOG_ERR, "5374: %s: increasing <%s> is not used by the resource <%s> with type <%s>; ignoring INCREASING", __func__, extResInfoPtr->increasing, resName, extResInfoPtr->type);
        }
    }
    else {
        if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC) {
            /* catgets 5375 */
            ls_syslog (LOG_ERR, "5375: %s: No increasing specified for a numeric resource <%s>", __func__, resName);
            setExtResourcesDefDefault (resName);
            return 0;
        }
    }

    strncpy (allInfo.resTable[allInfo.nRes].des, extResInfoPtr->des, MAXRESDESLEN);

    if (allInfo.resTable[allInfo.nRes].interval > 0 && (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC)) {

        if (allInfo.numUsrIndx + NBUILTINDEX >= li_len - 1) {
            li_len *= 2;
            if (!(li = realloc (li, li_len * sizeof (struct liStruct))))  {
                ls_syslog (LOG_ERR, "failed at %s during %s\n", __func__, "malloc");
                return -1;
            }
        }
        if ((li[NBUILTINDEX + allInfo.numUsrIndx].name = putstr_ (allInfo.resTable[allInfo.nRes].name)) == NULL) {
            ls_syslog (LOG_ERR, "failed at %s during %s\n", __func__, "malloc");
            return -1;
        }
        
        li[NBUILTINDEX + allInfo.numUsrIndx].increasing =
        (allInfo.resTable[allInfo.nRes].orderType == INCR) ? 1 : 0;
        allInfo.numUsrIndx++;
        allInfo.numIndx++;
    }
    return 0;
}

int setExtResourcesLoc ( const char *resName, int resNo)
{
    int isDefault           = 0;
    unsigned int lineNum    = 0;
    char *extResLocPtr      = NULL;
    char defaultExtResLoc[] = "[default]";
    char addResourceMap[]   = "addResourceMap";
    const char setExtResourcesLoc[] = "setExtResourcesLoc";

    extResLocPtr = getExtResourcesLoc (resName);
    
    if (extResLocPtr == NULL || extResLocPtr[0] == '\0') {
        /* catgets 5379 */
        ls_syslog (LOG_INFO, "5379: %s: Failed to get LOCATION specified for external resource <%s>; Set to %s", __func__, resName, defaultExtResLoc );
        extResLocPtr = defaultExtResLoc;
    }

    allInfo.resTable[resNo].flags |= RESF_EXTERNAL;

    if (addResourceMap(resName, extResLocPtr, setExtResourcesLoc, lineNum, &isDefault) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, setExtResourcesLoc, addResourceMap, resName);
        lim_CheckError = WARNING_ERR;
        return -1;
    }

    if (!(isDefault && (allInfo.resTable[resNo].flags & RESF_DYNAMIC) && (allInfo.resTable[resNo].valueType == LS_NUMERIC))) {
        allInfo.resTable[resNo].flags &= ~RESF_GLOBAL;
        allInfo.resTable[resNo].flags |= RESF_SHARED;
    }

    return 0;
}




struct extResInfo *
getExtResourcesDef ( const char *resName)
{
    assert( *resName );
    /* catgets 5453 */
    ls_syslog (LOG_ERR, "5453: %s: external resource object is current not support in this platform", __func__); // FIXME FIXME FIXME FIXME support external resource object in any platform
    return NULL;
}


char *
getExtResourcesLoc ( const char *resName)
{
    assert( *resName );
    /* catgets 5454 */
    ls_syslog (LOG_ERR, "5454: %s: external resource object is current not support in this platform", __func__); // FIXME FIXME FIXME FIXME support external resource object in any platform
    return NULL;
}


char *
getExtResourcesValDefault ( const char *resName)  // FIXME FIXME FIXME FIXME support external resource object in any platform
{
    char *defaultVal = NULL;
    assert( *resName );
    return defaultVal;
}


char *
getExtResourcesVal( const char *resName )
{
    
    assert( *resName );
    /* catgets 5455 */
    ls_syslog (LOG_ERR, "5455: %s: external resource object is current not support in this platform", __func__);  // FIXME FIXME FIXME FIXME support external resource object in any platform
    return getExtResourcesValDefault (resName);
}

int
initTypeModel (struct hostNode *me)
{
    
    if (me->hTypeNo == DETECTMODELTYPE)
        {
        me->hTypeNo = typeNameToNo (getHostType ());
        if (me->hTypeNo < 0)
        {
            /* catgets 5456 */
            ls_syslog (LOG_ERR, "5456: %s: Unknown host type <%s>, using <DEFAULT>", __func__, getHostType ());
            me->hTypeNo = 1;
        }
        
        myClusterPtr->typeClass |= (1 << me->hTypeNo);
        SET_BIT (me->hTypeNo, myClusterPtr->hostTypeBitMaps);
        
        }
    
    strcpy (me->statInfo.hostType, allInfo.hostTypes[me->hTypeNo]);
    
    if (me->hModelNo == DETECTMODELTYPE)
        {
        const char *arch = getHostModel ();
        
        me->hModelNo = archNameToNo (arch);
        if (me->hModelNo < 0) // FIXME FIXME FIXME FIXME should die on unknown architecture
        {
            /* catgets 5458 */ 
            ls_syslog( LOG_ERR, "5458: %s: Unknown host architecture <%s>, using <DEFAULT>", __func__, arch);
            me->hModelNo = 1; // FIXME FIXME FIXME FIXME should die on unknown architecture
        }
        else
        {
            if (strcmp (allInfo.hostArchs[me->hModelNo], arch) != 0)
                {
                if (logclass & LC_EXEC)
                {
                    /* catgets 5457 */
                    ls_syslog (LOG_WARNING, "5457: %s: Unknown host architecture <%s>, using best match <%s>, model <%s>", __func__, arch, allInfo.hostArchs[me->hModelNo], allInfo.hostModels[me->hModelNo]);
                }
            }
            
        }
        
        myClusterPtr->modelClass |= (1 << me->hModelNo);
        SET_BIT (me->hModelNo, myClusterPtr->hostModelBitMaps);
        
        }
    strcpy (me->statInfo.hostArch, allInfo.hostArchs[me->hModelNo]);
    
    
    ++allInfo.modelRefs[me->hModelNo];
    return 0;
}

char *
stripIllegalChars ( const char *str)
{
    char *c = str;
    char *p = str;
    
    while (*c)
        {
        if (isalnum ((int) *c)) {
            *p++ = *c++;
        }
        else {
            c++;
        }
    }
    *p = '\0';
    
    return str;
}

int
saveHostIPAddr (struct hostNode *hPtr, struct hostent *hp)
{
    int i;
    
    for (hPtr->naddr = 0;
         hp->h_addr_list && hp->h_addr_list[hPtr->naddr] != NULL;
         hPtr->naddr++); // empty if body
    
    hPtr->addr = NULL;
    if (hPtr->naddr)
        {
        hPtr->addr = calloc (hPtr->naddr, sizeof (in_addr_t));
        if (!hPtr->addr)
            {
            ls_syslog (LOG_ERR, "%s: call to calloc() fir %d bytes failed %m", __func__, hPtr->naddr * sizeof (in_addr_t));
            freeHostNodes (hPtr, FALSE);
            return -1;
            }
        }
    
    for (i = 0; i < hPtr->naddr; i++) {
        memcpy (&hPtr->addr[i], hp->h_addr_list[i], hp->h_length);
    }
    
    return 0;
}

/* addMigrantHost()
 */
void
addMigrantHost (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, unsigned int chan)
{
    char buf[MSGSIZE];
    struct LSFHeader hdr;
    struct hostEntry hPtr;
    struct hostNode *node = 0;
    unsigned int16_t opCode = 0;
    XDR xdrs2;
    unsigned int cc = 0;
    
    if (!masterMe) {
        wrongMaster (from, buf, reqHdr, -1);
        return;
    }
    
    memset (&hPtr, 0, sizeof (struct hostEntry));
    /* decode the hostInfo request
     */
    if (!xdr_hostEntry (xdrs, &hPtr, reqHdr)) {
        opCode = LIME_BAD_DATA;
        goto hosed;
    }
    
    node = findHostbyList (myClusterPtr->hostList, hPtr.hostName);
    if (node) {
        ls_syslog (LOG_WARNING, "%s: trying to add already configured host %s from %s", __func__, hPtr.hostName, sockAdd2Str_ (from));
        opCode = LIME_KWN_MIGRANT;
        goto hosed;
    }
    
    /* add the host
     */
    cc = 0;
    if ((node = addHost_ (myClusterPtr, &hPtr, hPtr.window, (char *) __func__, &cc)) == NULL) {
        ls_syslog (LOG_ERR, "%s: failed adding migrant host %s", __func__, hPtr.hostName);
        /* free the shit...
         */
        opCode = LIME_BAD_DATA;
        goto hosed;
    }
    
    /* log the lim event HOST_ADD
     */
    logAddHost (&hPtr);
    /* mark the node as migrant
     */
    node->migrant = 1;
    
    /* reply to the library
     */
    opCode = LIME_NO_ERR;
hosed:         // FIXME FIXME FIXME FIXME remove goto label
    initLSFHeader_ (&hdr);
    hdr.opCode = opCode;
    hdr.refCode = reqHdr->refCode;
    
    xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);
    
    if (!xdr_LSFHeader (&xdrs2, &hdr)) {
        ls_syslog (LOG_ERR, "%s: failed decoding header from %s", __func__, sockAdd2Str_ (from));
        xdr_destroy (&xdrs2);
        return;
    }

    assert( chan <= INT_MAX );
    if (chanWrite_ ((int)chan, buf, XDR_GETPOS (&xdrs2)) < 0) {
        ls_syslog (LOG_ERR, "%s: Failed replying to %s dbytes %m", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
        xdr_destroy (&xdrs2);
        return;
    }
    
    xdr_destroy (&xdrs2);
    
    /* At last announce my mastership to
     * the new host.
     */
    if (opCode == LIME_NO_ERR)
        announceMasterToHost (node, SEND_CONF_INFO);
}

/* addHostByTab()
 */
int
addHostByTab ( struct hTab * tab)
{
    struct hostEntry hPtr;
    struct hostEntryLog *hLog = NULL;
    struct hostNode *node     = NULL;
    struct hEnt *e            = NULL;
    struct sTab stab;
    
    for (e = h_firstEnt_ (tab, &stab); e != NULL; e = h_nextEnt_ (&stab)) {
        unsigned int cc = 0;
        
        hLog = e->hData;
        memcpy (&hPtr, hLog, sizeof (struct hostEntry));
        
        if ((node = addHost_ (myClusterPtr, &hPtr, hPtr.window, (char *) __func__, &cc)) == NULL)
            {
            ls_syslog (LOG_ERR, "%s: failed adding runtime host %s", __func__, hPtr.hostName);
            continue;
            }
        /* mark the node as migrant.
         */
        node->migrant = 1;
        
        ls_syslog (LOG_DEBUG, "%s: runtime host %s model %s type %s added all right", __func__, hPtr.hostName, hPtr.hostModel, hPtr.hostType);
        
        /* let the caller, the owner of the table
         * to deal with the entries...
         */
        }
    
    return 0;
}

/* rmMigrantHost()
 */
void
rmMigrantHost (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, unsigned int chan)
{
    char buf[MSGSIZE];
    struct LSFHeader hdr;
    char hostName[MAXHOSTNAMELEN];
    struct hostNode *hPtr = NULL;
    struct hostEntry hEnt;
    char *p = NULL;
    XDR xdrs2;
    unsigned int16_t opCode;
    
    if (!masterMe) {
        wrongMaster (from, buf, reqHdr, -1);
        return;
    }
    
    p = hostName;
    if (!xdr_hostName (xdrs, p, reqHdr)) {
        opCode = LIME_BAD_DATA;
        initLSFHeader_ (&hdr);
        hdr.opCode = opCode;
        hdr.refCode = reqHdr->refCode;
        
        xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);
        
        if (!xdr_LSFHeader (&xdrs2, &hdr)) {
            ls_syslog (LOG_ERR, "%s: Failed decoding header from %s.", __func__, sockAdd2Str_ (from));
            xdr_destroy (&xdrs2);
            return;
        }

        assert( chan <= INT_MAX );
        if (chanWrite_ ((int)chan, buf, XDR_GETPOS (&xdrs2)) < 0) {
            ls_syslog (LOG_ERR, " %s: Failed replying to %s dbytes %m.", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
            xdr_destroy (&xdrs2);
            return;
        }
        
        xdr_destroy (&xdrs2);

    }
    
    hPtr = findHostbyList (myClusterPtr->hostList, hostName);
    if (hPtr == NULL) {
        ls_syslog (LOG_WARNING, "%s: trying to remove unknown host %s from %s", __func__, hostName, sockAdd2Str_ (from));
        opCode = LIME_UNKWN_HOST;
        initLSFHeader_ (&hdr);
        hdr.opCode = opCode;
        hdr.refCode = reqHdr->refCode;
        
        xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);
        
        if (!xdr_LSFHeader (&xdrs2, &hdr)) {
            ls_syslog (LOG_ERR, "%s: Failed decoding header from %s.", __func__, sockAdd2Str_ (from));
            xdr_destroy (&xdrs2);
            return;
        }

        assert( chan <= INT_MAX );
        if (chanWrite_ ((int)chan, buf, XDR_GETPOS (&xdrs2)) < 0) {
            ls_syslog (LOG_ERR, " %s: Failed replying to %s dbytes %m.", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
            xdr_destroy (&xdrs2);
            return;
        }
        
        xdr_destroy (&xdrs2);

    }
    
    if (!hPtr->migrant) {
        /* Ho ho no migrant no remove...
         */
        opCode = LIME_BAD_DATA;
        initLSFHeader_ (&hdr);
        hdr.opCode = opCode;
        hdr.refCode = reqHdr->refCode;

        xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);
        if (!xdr_LSFHeader (&xdrs2, &hdr)) {
            ls_syslog (LOG_ERR, "%s: Failed decoding header from %s.", __func__, sockAdd2Str_ (from));
            xdr_destroy (&xdrs2);
            return;
        }

        assert( chan <= INT_MAX );
        if (chanWrite_ ((int)chan, buf, XDR_GETPOS (&xdrs2)) < 0) {
            ls_syslog (LOG_ERR, " %s: Failed replying to %s dbytes %m.", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
            xdr_destroy (&xdrs2);
            return;
        }
        
        xdr_destroy (&xdrs2);
    }
    
    rmHost (hPtr);
    
    memset (&hEnt, 0, sizeof (struct hostEntry));
    strcpy (hEnt.hostName, hPtr->hostName);
    
    logRmHost (&hEnt);
    freeHostNodes (hPtr, FALSE);
    
    opCode = LIME_NO_ERR;

    initLSFHeader_ (&hdr);
    hdr.opCode = opCode;
    hdr.refCode = reqHdr->refCode;
    
    xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);
    
    if (!xdr_LSFHeader (&xdrs2, &hdr)) {
        ls_syslog (LOG_ERR, "%s: Failed decoding header from %s.", __func__, sockAdd2Str_ (from));
        xdr_destroy (&xdrs2);
        return;
    }

    assert( chan <= INT_MAX );
    if (chanWrite_ ((int)chan, buf, XDR_GETPOS (&xdrs2)) < 0) {
        ls_syslog (LOG_ERR, " %s: Failed replying to %s dbytes %m.", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
        xdr_destroy (&xdrs2);
        return;
    }
    
    xdr_destroy (&xdrs2);

    return;
}
