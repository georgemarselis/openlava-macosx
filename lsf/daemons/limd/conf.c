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
#include "lib/conf.h"
#include "lib/xdrlim.h"

#define NL_SETN 24


/* getHostType()
 */
char *
getHostType (void)
{

    return "MacOSX";
//    return HOST_TYPE_STRING;
}

int
readShared (void)
{
    static char fname[] = "readShared()";
    FILE *fp;
    char *cp = NULL;
    char lsfile[MAXFILENAMELEN] = { 'a' };
    char *word   = NULL;
    char modelok = 'a';
    char resok   = 'a';
    char clsok   = 'a';
    char indxok  = 'a';
    char typeok  = 'a';
    uint lineNum = 0;

    modelok = FALSE;
    resok = FALSE;
    clsok = FALSE;
    indxok = TRUE;
    typeok = FALSE;

    initResTable ();

    sprintf (lsfile, "%s/lsf.shared", limParams[LSF_CONFDIR].paramValue);

    if (configCheckSum (lsfile, &lsfSharedCkSum) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "configCheckSum");
        return (-1);
    }
    fp = confOpen (lsfile, "r");
    if (!fp) {
        /* catgets 5200 */
        ls_syslog (LOG_ERR, "catgets 5200: %s: Can't open configuration file <%s>: %m", fname, lsfile);
        return (-1);
    }

    for (;;) {
        if ((cp = getBeginLine (fp, &lineNum)) == NULL) {
            FCLOSEUP (&fp);
            if (!modelok) {
                /* catgets 5201 */
                ls_syslog (LOG_ERR, "catgets 5201: %s: HostModel section missing or invalid in %s", fname, lsfile);
            }
            if (!resok) {
                /* catgets 5202 */
                ls_syslog (LOG_ERR, "catgets 5202: %s: Warning: Resource section missing or invalid in %s", fname, lsfile);
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
                return (0);
            }
            else {
                return (-1);
            }
        }

        word = getNextWord_ (&cp);
        if (!word) {
            /* catgets 5206 */
            ls_syslog (LOG_ERR, "catgets 5206: %s: %s(%d): Section name expected after Begin; ignoring section", fname, lsfile, lineNum);
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
            ls_syslog (LOG_ERR, "catgets 5207: %s: %s(%d): Invalid section name %s; ignoring section", fname, lsfile, lineNum, word);
            lim_CheckError = WARNING_ERR;
            doSkipSection (fp, &lineNum, lsfile, word);
            }
        }
}

static char
doindex (FILE * fp, uint *lineNum, char *lsfile)
{
    static char fname[] = "doindex()";
    char *linep;
    struct keymap keyList[] = {
        {0, "    ", NULL, "INTERVAL"},
        {0, "    ", NULL, "INCREASING"},
        {0, "    ", NULL, "DESCRIPTION"},
        {0, "    ", NULL, "NAME"},
        {0, "    ", NULL, NULL}
    };

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
        {
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5208, "%s: %s(%d): Premature EOF"),    /* catgets 5208 */
                   fname, lsfile, *lineNum);
        lim_CheckError = WARNING_ERR;
        return (TRUE);
        }

    if (isSectionEnd (linep, lsfile, lineNum, "newindex"))
        {
        ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5209, "%s: %s(%d): empty section"),    /* catgets 5209 */
                   fname, lsfile, *lineNum);
        return (TRUE);
        }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, TRUE))
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5210, "%s: %s(%d): keyword line format error for section newindex; ignoring section"), /* catgets 5210 */
                       fname, lsfile, *lineNum);
            lim_CheckError = WARNING_ERR;
            doSkipSection (fp, lineNum, lsfile, "newindex");
            return (TRUE);
            }


        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, "newindex"))
                return (TRUE);
            if (mapValues (keyList, linep) < 0)
                {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5211, "%s: %s(%d): values do not match keys for section newindex; ignoring line"), fname, lsfile, *lineNum);   /* catgets 5211 */
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (!setIndex (keyList, lsfile, *lineNum))
                {
                doSkipSection (fp, lineNum, lsfile, "newindex");
                freeKeyList (keyList);
                return (FALSE);
                }
            freeKeyList (keyList);
            }
        }
    else {
        if (readHvalues (keyList, linep, fp, lsfile, lineNum, TRUE, "newindex") < 0) {
            return (TRUE);
        }
        if (!setIndex (keyList, lsfile, *lineNum)) {
            return (FALSE);
        }
        return (TRUE);
    }

    /* catgets 5212 */
    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5212, "%s: %s(%d): Premature EOF"), fname, lsfile, *lineNum);
    lim_CheckError = WARNING_ERR;
    return (TRUE);

}

static char
setIndex (struct keymap *keyList, char *lsfile, uint linenum)
{
    static char fname[] = "setIndex()";
    int resIdx = 0;


    if (strlen (keyList[3].val) >= MAXLSFNAMELEN)
        {
        /* catgets 5213 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5213, "%s: %s(%d): Name %s is too long (maximum is %d chars); ignoring index"), fname, lsfile, linenum, keyList[3].val, MAXLSFNAMELEN - 1);
        lim_CheckError = WARNING_ERR;
        return (TRUE);
        }

    if (strpbrk (keyList[3].val, ILLEGAL_CHARS) != NULL)
        {
        /* catgets 5214 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5214, "%s: %s(%d): illegal character (one of %s)"), fname, lsfile, linenum, ILLEGAL_CHARS);
        lim_CheckError = WARNING_ERR;
        return (TRUE);
        }
    if (IS_DIGIT (keyList[3].val[0]))
        {
        /* catgets 5215 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5215, "%s: %s(%d): Index name <%s> begun with a digit is illegal; ignored"), fname, lsfile, linenum, keyList[3].val);
        lim_CheckError = WARNING_ERR;
        return (TRUE);
        }

    if ((resIdx = resNameDefined (keyList[3].val)) >= 0)
        {
        if (!(allInfo.resTable[resIdx].flags & RESF_DYNAMIC))
            {
            /* catgets 5216 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5216, "%s: %s(%d): Name <%s> is not a dynamic resource; ignored"), fname, lsfile, linenum, keyList[3].val);
            return (TRUE);
            }
        if ((allInfo.resTable[resIdx].flags & RESF_BUILTIN))
            /* catgets 5217 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5217, "%s: %s(%d): Name <%s> reserved or previously defined;"), fname, lsfile, linenum, keyList[3].val);
        else
            {
            /* catgets 5219 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5219, "%s: %s(%d): Name <%s> reserved or previously defined;ignoring"), fname, lsfile, linenum, keyList[3].val);
            lim_CheckError = WARNING_ERR;
            return (TRUE);
            }
        }
    else {
        assert( allInfo.nRes <= INT_MAX );
        resIdx = (int)allInfo.nRes;
    }
    assert( sizeOfResTable <= INT_MAX );
    if (resIdx >= (int)sizeOfResTable && doubleResTable (lsfile, linenum) < 0) {
        return FALSE;
    }

    initResItem (&allInfo.resTable[resIdx]);
    allInfo.resTable[resIdx].interval = atoi (keyList[0].val);
    allInfo.resTable[resIdx].orderType =
    (strcasecmp (keyList[1].val, "y") == 0) ? INCR : DECR;

    strcpy (allInfo.resTable[resIdx].des, keyList[2].val);
    strcpy (allInfo.resTable[resIdx].name, keyList[3].val);
    allInfo.resTable[resIdx].valueType = LS_NUMERIC;
    allInfo.resTable[resIdx].flags = RESF_DYNAMIC | RESF_GLOBAL;

    if (allInfo.numUsrIndx + NBUILTINDEX >= li_len - 1) {
        li_len <<= 1;
        li = realloc (li, li_len * sizeof (struct liStruct));
        if ( NULL == li && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "malloc");
            return TRUE;
        }
    }

    assert( allInfo.nRes <= INT_MAX );
    if (resIdx == (int)allInfo.nRes) {

        li[NBUILTINDEX + allInfo.numUsrIndx].increasing = (strcasecmp (keyList[1].val, "y") == 0);
        if ((li[NBUILTINDEX + allInfo.numUsrIndx].name = putstr_ (keyList[3].val)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "malloc");
            return TRUE;
        }
        allInfo.numUsrIndx++;
        allInfo.numIndx++;
        allInfo.nRes++;
    }
    else {

        for (uint i = 0; i < NBUILTINDEX + allInfo.numUsrIndx; i++) {
            if (!strcasecmp (keyList[3].val, li[i].name)) {
                li[i].increasing = (strcasecmp (keyList[1].val, "y") == 0);
                break;
            }
        }
    }

    defaultRunElim = TRUE;

    return TRUE;
}

static char
doclist (FILE * fp, uint *lineNum, char *lsfile)
{
    static char fname[] = "doclist()";
    char *linep;
    struct keymap keyList[] = {
        {0, "    ", NULL, "CLUSTERNAME"},
        {0, "    ", NULL, "SERVERS"},
        {0, "    ", NULL, NULL}
    };
    char *servers;
    bool_t clusterAdded = FALSE;

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
        {
        /* catgets 5222 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5222, "%s: %s(%d): section cluster: Premature EOF"), fname, lsfile, *lineNum);
        return FALSE;
        }

    if (isSectionEnd (linep, lsfile, lineNum, "cluster")) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, FALSE))
            {
            /* catgets 5223 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5223, "%s: %s(%d): keyword line format error for section cluster; ignoring section"), fname, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, "cluster");
            return FALSE;
            }

        if (keyList[0].position == -1) {
            /* catgets 5224 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5224, "%s: %s(%d): keyword line: key %s is missing in section cluster; ignoring section"), fname, lsfile, *lineNum, keyList[0].key);
            doSkipSection (fp, lineNum, lsfile, "cluster");
            return FALSE;
        }


        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, "cluster")) {
                return TRUE;
            }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5225 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5225, "%s: %s(%d): values do not match keys for section cluster, ignoring line"),  fname, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (keyList[1].position != -1)
                {
                servers = keyList[1].val;
                mcServersSet = TRUE;
                }
            else
                servers = NULL;

            if (!clusterAdded && !addCluster (keyList[0].val, servers))
                {
                /* catgets 5226 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5226, "%s: Ignoring cluster %s"), fname, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                }
            else if (clusterAdded)
                {
                /* catgets 5226 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5226, "%s: Ignoring cluster %s"), fname, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                }

            FREEUP (keyList[0].val);
            if (keyList[1].position != -1) {
                FREEUP (keyList[1].val);
            }
            clusterAdded = TRUE;
        }
    }
    else
        {
        /* catgets 5227 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5227, "%s: %s(%d): horizontal cluster section not implemented yet, ignoring section"), fname, lsfile, *lineNum);
        doSkipSection (fp, lineNum, lsfile, "cluster");
        return FALSE;
        }

    ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "cluster");
    return FALSE;
}

static char
dotypelist (FILE * fp, uint *lineNum, char *lsfile)
{
    static char fname[] = "dotypelist()";
    struct keymap keyList[] = {
        {0, "    ", NULL, "TYPENAME"},
        {0, "    ", NULL, NULL}
    };
    char *linep;

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep)
        {
        ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "HostType");
        return FALSE;
        }

    if (isSectionEnd (linep, lsfile, lineNum, "HostType")) {
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

    strcpy (allInfo.hostTypes[0], "UNKNOWN_AUTO_DETECT");
    shortInfo.hostTypes[0] = allInfo.hostTypes[0];
    strcpy (allInfo.hostTypes[1], "DEFAULT");
    shortInfo.hostTypes[1] = allInfo.hostTypes[1];

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, TRUE))
            {
            /* catgets 5230 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5230, "%s: %s(%d): keyword line format error for section HostType, ignoring section"), fname, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, "HostType");
            return FALSE;
            }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, "HostType"))
                {
                return TRUE;
                }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5231 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5231, "%s: %s(%d): values do not match keys for section cluster, ignoring line"), fname, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (strpbrk (keyList[0].val, ILLEGAL_CHARS) != NULL)
                {
                /* catgets 5232 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5232, "%s: %s(%d): illegal character (one of %s), ignoring type %s"), fname, lsfile, *lineNum, ILLEGAL_CHARS, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                FREEUP (keyList[0].val);
                continue;
                }
            if (IS_DIGIT (keyList[0].val[0]))
                {
                /* catgets 5233 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5233, "%s: %s(%d): Type name <%s> begun with a digit is illegal; ignored"), fname, lsfile, *lineNum, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                FREEUP (keyList[0].val);
                continue;
                }
            if (!addHostType (keyList[0].val))
                lim_CheckError = WARNING_ERR;

            FREEUP (keyList[0].val);
            }
        }
    else
        {
        /* catgets 5234 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5234, "%s: %s(%d): horizontal HostType section not implemented yet, ignoring section"), fname, lsfile, *lineNum);
        doSkipSection (fp, lineNum, lsfile, "HostType");
        return FALSE;
        }

    ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum,
               "HostType");

    return FALSE;
}

static char
dohostmodel (FILE * fp, uint *lineNum, char *lsfile)
{
    static char fname[] = "dohostmodel()";
    static char first = TRUE;
    char *linep;
    int new;
    hEnt *hashEntPtr;
    float *floatp;
    struct keymap keyList[] = {
        {0, "    ", NULL, "MODELNAME"},
        {0, "    ", NULL, "CPUFACTOR"},
        {0, "    ", NULL, "ARCHITECTURE"},
        {0, "    ", NULL, NULL}
    };
    char *sp, *word;

    if (first) {
        for ( int i = 0; i < MAXMODELS; ++i) {
            allInfo.cpuFactor[i] = 1.0;
            allInfo.modelRefs[i] = 0;
        }

        h_initTab_ (&hostModelTbl, 11);
        first = FALSE;
    }

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "hostmodel");
        return FALSE;
    }

    if (isSectionEnd (linep, lsfile, lineNum, "hostmodel")) {
        return FALSE;
    }

    if (allInfo.nModels <= 0)
        {
        memset (allInfo.modelRefs, 0, sizeof (int) * MAXMODELS);
        allInfo.nModels = 2;
        }
    if (shortInfo.nModels <= 0)
        {
        shortInfo.nModels = 2;
        }

    strcpy (allInfo.hostModels[0], "UNKNOWN_AUTO_DETECT");
    strcpy (allInfo.hostArchs[0], "UNKNOWN_AUTO_DETECT");
    allInfo.cpuFactor[0] = 1;
    shortInfo.hostModels[0] = allInfo.hostModels[0];
    strcpy (allInfo.hostModels[1], "DEFAULT");
    strcpy (allInfo.hostArchs[1], "");
    allInfo.cpuFactor[1] = 1;
    shortInfo.hostModels[1] = allInfo.hostModels[1];

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, FALSE))
            {
            /* catgets 5237 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5237, "%s: %s(%d): keyword line format error for section hostmodel, ignoring section"), fname, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, "dohostmodel");
            return FALSE;
            }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, "hostmodel"))
                {
                return TRUE;
                }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5238 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5238, "%s: %s(%d): values do not match keys for section hostmodel, ignoring line"), fname, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if (!isanumber_ (keyList[1].val) || atof (keyList[1].val) <= 0)
                {
                /* catgets 5239 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5239, "%s: %s(%d): Bad cpuFactor for host model %s, ignoring line"), fname, lsfile, *lineNum, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                FREEUP (keyList[0].val);
                FREEUP (keyList[1].val);
                FREEUP (keyList[2].val);
                continue;
                }

            if (strpbrk (keyList[0].val, ILLEGAL_CHARS) != NULL)
                {
                /* catgets 5240 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5240, "%s: %s(%d): illegal character (one of %s), ignoring model %s"), fname, lsfile, *lineNum, ILLEGAL_CHARS, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                FREEUP (keyList[0].val);
                FREEUP (keyList[1].val);
                FREEUP (keyList[2].val);
                continue;
                }
            if (IS_DIGIT (keyList[0].val[0]))
                {
                /* catgets 5241 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5241, "%s: %s(%d): Model name <%s> begun with a digit is illegal; ignored"), fname, lsfile, *lineNum, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                FREEUP (keyList[0].val);
                FREEUP (keyList[1].val);
                FREEUP (keyList[2].val);
                continue;
                }

            sp = keyList[2].val;
            if (sp && sp[0])
                {
                while ((word = getNextWord_ (&sp)) != NULL)
                    {
                    if (!addHostModel (keyList[0].val, word, atof (keyList[1].val)))
                        {
                        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5242, "%s: %s(%d): Too many host models, ignoring model %s"), fname, lsfile, *lineNum, keyList[0].val);    /* catgets 5242 */
                        lim_CheckError = WARNING_ERR;
                        goto next_value;
                        }
                    }
                }
            else
                {
                if (!addHostModel (keyList[0].val, NULL, atof (keyList[1].val)))
                    {
                    /* catgets 5242 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5242, "%s: %s(%d): Too many host models, ignoring model %s"), fname, lsfile, *lineNum, keyList[0].val);
                    lim_CheckError = WARNING_ERR;
                    goto next_value;
                    }

                }

            hashEntPtr = h_addEnt_ (&hostModelTbl, keyList[0].val, &new);
            if (new)
                {
                floatp = malloc (sizeof (float));
                if (floatp == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M,
                               fname, "malloc", sizeof (float));
                    doSkipSection (fp, lineNum, lsfile, "HostModel");
                    return FALSE;
                    }
                *floatp = (float)atof (keyList[1].val);
                hashEntPtr->hData = floatp;
                }
            else
                {
                floatp = hashEntPtr->hData;
                *floatp = (float)atof (keyList[1].val);
                hashEntPtr->hData = floatp;
                }

            next_value:
            FREEUP (keyList[0].val);
            FREEUP (keyList[1].val);
            FREEUP (keyList[2].val);
            }
        }
    else
        {
        /* catgets 5244 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5244, "%s: %s(%d): horizontal HostModel section not implemented yet, ignoring section"), fname, lsfile, *lineNum);
        doSkipSection (fp, lineNum, lsfile, "HostModel");
        return FALSE;
        }

    ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "HostModel");
    return FALSE;
}

static void
initResTable (void)
{
    struct resItem *resTable;
    uint i = 0;

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

}

int
resNameDefined (char *name)
{

    for ( uint i = 0; i < allInfo.nRes; i++) {
        if (strcmp (name, allInfo.resTable[i].name) == 0) {
            assert( i <= INT_MAX );
            return ((int)i);
        }
    }
    return (-1);
}

#define RKEY_RESOURCENAME 0
#define RKEY_TYPE         1
#define RKEY_INTERVAL     2
#define RKEY_INCREASING   3
#define RKEY_RELEASE      4
#define RKEY_DESCRIPTION  5

static char
doresources (FILE * fp, uint *lineNum, char *lsfile)
{
    static char fname[] = "doresources()";
    char *linep = NULL;
    struct keymap keyList[] = {
            {0, "    ", NULL, "RESOURCENAME"},
            {0, "    ", NULL, "TYPE"},
            {0, "    ", NULL, "INTERVAL"},
            {0, "    ", NULL, "INCREASING"},
            {0, "    ", NULL, "RELEASE"},
            {0, "    ", NULL, "DESCRIPTION"},
            {0, "    ", NULL, NULL}
    };
    int nres = 0;
    int resIdx = 0;

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "resource");
        return FALSE;
    }

    if (isSectionEnd (linep, lsfile, lineNum, "resource")) {
        return FALSE;
    }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, FALSE)) {
            /* catgets 5248 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5248, "%s: %s(%d): keyword line format error for section resource, ignoring section"), fname, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, "resource");
            return FALSE;
        }

        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {

            if (isSectionEnd (linep, lsfile, lineNum, "resource")) {
                return TRUE;
            }

            if (mapValues (keyList, linep) < 0) {
                /* catgets 5249 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5249, "%s: %s(%d): values do not match keys for section resource, ignoring line"), fname, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
            }

            if (strlen (keyList[RKEY_RESOURCENAME].val) >= MAXLSFNAMELEN - 1) {
                /* catgets 5250 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5250, "%s: %s(%d): Resource name %s too long in section resource. Should be less than %d characters. Ignoring line"), fname, lsfile, *lineNum, keyList[0].val, MAXLSFNAMELEN - 1);
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
                        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5251, "%s: %s(%d): Built-in resource %s can't be overrided with different type or increasing. Ignoring line"), fname, lsfile, *lineNum, keyList[0].val);
                        lim_CheckError = WARNING_ERR;
                    }
                }
                else {
                    /* catgets 5251 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5251, "%s: %s(%d): Resource name %s reserved or previously defined. Ignoring line"), fname, lsfile, *lineNum, keyList[0].val);
                    lim_CheckError = WARNING_ERR;
                }
                freeKeyList (keyList);
                continue;
                }

            if (strpbrk (keyList[RKEY_RESOURCENAME].val, ILLEGAL_CHARS) != NULL) {
                /* catgets 5252 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5252, "%s: %s(%d): illegal character (one of %s): in resource name:%s, section resource, ignoring line"), fname, lsfile, *lineNum, ILLEGAL_CHARS, keyList[0].val);
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
            }
            if (IS_DIGIT (keyList[RKEY_RESOURCENAME].val[0])) {
                /* catgets 5253 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5253, "%s: %s(%d): Resource name <%s> begun with a digit is illegal; ignored"), fname, lsfile, *lineNum, keyList[0].val); 
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
            }
            if (allInfo.nRes >= sizeOfResTable  && doubleResTable (lsfile, *lineNum) < 0) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "doubleResTable");
                return FALSE;
            }
            initResItem (&allInfo.resTable[allInfo.nRes]);
            strcpy (allInfo.resTable[allInfo.nRes].name, keyList[RKEY_RESOURCENAME].val);


            if ( keyList[RKEY_TYPE].val != NULL && keyList[RKEY_TYPE].val[0] != '\0') {

                int type = 0;

                if (strcmp (keyList[RKEY_TYPE].val, LSF_LIM_ERES_TYPE) == 0)
                    {
                    if (setExtResourcesDef (keyList[RKEY_RESOURCENAME].val) != 0) {
                        /* catgets 5256 */
                        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5256, "%s: Ignoring the external resource <%s>(%d) in section resource of file %s"), fname, keyList[RKEY_RESOURCENAME].val, *lineNum, lsfile);
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
                    allInfo.resTable[allInfo.nRes].valueType = (uint)type;
                }
                else {
                    /* catgets 5257 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5257, "%s: %s(%d): resource type <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource"), fname, lsfile, *lineNum, keyList[RKEY_TYPE].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
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
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5258, "%s: %s(%d): INTERVAL <%s> for resource <%s> should be a integer greater than 0; ignoring resource <%s> in section resource"), fname, lsfile, *lineNum, keyList[RKEY_INTERVAL].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                    }
                }

            if (keyList[RKEY_INCREASING].val != NULL && keyList[RKEY_INCREASING].val[0] != '\0') {
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
                        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5259, "%s: %s(%d): INCREASING <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource"), fname, lsfile, *lineNum, keyList[RKEY_INCREASING].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                        lim_CheckError = WARNING_ERR;
                        freeKeyList (keyList);
                        continue;
                        }
                    }
                else
                    /* catgets 5260 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5260, "%s: %s(%d): INCREASING <%s> is not used by the resource <%s> with type <%s>; ignoring INCREASING"), fname, lsfile, *lineNum, keyList[RKEY_INCREASING].val, keyList[RKEY_RESOURCENAME].val, (allInfo.resTable[allInfo.nRes].orderType == LS_BOOLEAN) ? "BOOLEAN" : "STRING");
                }
            else
                {
                if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC) {
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5261, "%s: %s(%d): No INCREASING specified for a numeric resource <%s>; ignoring resource <%s> in section resource"), fname, lsfile, *lineNum, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                }
            }

            if (keyList[RKEY_RELEASE].val != NULL && keyList[RKEY_RELEASE].val[0] != '\0')
                {
                if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC)
                    {
                    if (!strcasecmp (keyList[RKEY_RELEASE].val, "Y"))
                        {
                        allInfo.resTable[allInfo.nRes].flags |= RESF_RELEASE;
                        }
                    else if (strcasecmp (keyList[RKEY_RELEASE].val, "N"))
                        {
                        ls_syslog (LOG_ERR, I18N (5474, "doresources:%s(%d): RELEASE defined for resource <%s> should be 'Y', 'y', 'N' or 'n' not <%s>; ignoring resource <%s> in section resource"),   /*catgets 5474 */
                                   lsfile, *lineNum,
                                   keyList[RKEY_RESOURCENAME].val,
                                   keyList[RKEY_RELEASE].val,
                                   keyList[RKEY_RESOURCENAME].val);
                        lim_CheckError = WARNING_ERR;
                        freeKeyList (keyList);
                        continue;
                        }
                    }
                else
                    {

                    ls_syslog (LOG_ERR, I18N (5475, "doresources:%s(%d): RELEASE cannot be defined for resource <%s> which isn't a numeric resource; ignoring resource <%s> in section resource"),  /*catgets 5475 */
                               lsfile, *lineNum, keyList[RKEY_RESOURCENAME].val,
                               keyList[RKEY_RESOURCENAME].val);
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
                        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "malloc",
                                   li_len * sizeof (struct liStruct));
                        return FALSE;
                        }
                    }
                if ((li[NBUILTINDEX + allInfo.numUsrIndx].name =
                     putstr_ (allInfo.resTable[allInfo.nRes].name)) == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "malloc",
                               sizeof (allInfo.resTable[allInfo.nRes].name));
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
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5264, "%s: %s(%d): horizontal resource section not implemented yet"), fname, lsfile, *lineNum);    /* catgets 5264 */
        return FALSE;
        }


    ls_syslog (LOG_ERR, I18N_PREMATURE_EOF,
               fname, lsfile, *lineNum, "resource");
    return FALSE;

}

static void
chkUIdxAndSetDfltRunElim (void)
{
    if (defaultRunElim == FALSE && allInfo.numUsrIndx > 0) {

        for ( uint i = NBUILTINDEX; i < allInfo.numIndx; i++) {
            if (allInfo.resTable[i].flags & (RESF_DYNAMIC | RESF_GLOBAL)) {
                if (allInfo.resTable[i].flags & RESF_DEFINED_IN_RESOURCEMAP) {
                    defaultRunElim = TRUE;
                    break;
                }
            }
        }
    }
}

static int
doresourcemap (FILE * fp, char *lsfile, uint *lineNum)
{
    static char fname[] = "doresourcemap()";
    enum {
        RKEY_RESOURCE_NAME,
        RKEY_LOCATION
    };
    int isDefault = 0;
    char *linep = NULL;
    int resNo = 0;
    uint cc = 0;
    struct keymap keyList[] = {
        {0, "    ", NULL, "RESOURCENAME"},
        {0, "    ", NULL, "LOCATION"},
        {0, "    ", NULL, NULL}
    };

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "resourceMap");
        return (-1);
    }

    if (isSectionEnd (linep, lsfile, lineNum, "resourceMap")) {
        /* catgets 5477 */
        ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5477, "%s: %s(%d): Empty resourceMap, no keywords or resources defined."), fname, lsfile, *lineNum);
        return (-1);
    }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, TRUE))
            {
            /* catgets 5267 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5267, "%s: %s(%d): keyword line format error for section resource, ignoring section"), fname, lsfile, *lineNum);
            doSkipSection (fp, lineNum, lsfile, "resourceMap");
            return (-1);
            }


        while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, "resourceMap"))
                {
                return (0);
                }

            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5268 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5268, "%s: %s(%d): values do not match keys for resourceMap section, ignoring line"), fname, lsfile, *lineNum);
                lim_CheckError = WARNING_ERR;
                continue;
                }

            if ((resNo = resNameDefined (keyList[RKEY_RESOURCE_NAME].val)) < 0)
                {
                /* catgets 5269 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5269, "%s: %s(%d): Resource name <%s> is  not defined; ignoring line"), fname, lsfile, *lineNum, keyList[RKEY_RESOURCE_NAME].val);
                lim_CheckError = WARNING_ERR;
                freeKeyList (keyList);
                continue;
                }
            else
                {

                if (resNo < NBUILTINDEX) {
                    /* catgets 5296 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5296, "%s: %s(%d): Built-in resource %s can't be redefined as shared resource here. Ignoring line"), fname, lsfile, *lineNum, keyList[0].val);
                    continue;
                }
            }

            if (keyList[RKEY_LOCATION].val != NULL
                && strcmp (keyList[RKEY_LOCATION].val, LSF_LIM_ERES_TYPE) == 0)
                {
                if (setExtResourcesLoc (keyList[RKEY_RESOURCENAME].val, resNo) != 0)
                    {
                    /* catgets 5270 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5270, "%s: Ignoring the external resource location <%s>(%d) in section resourceMap of file %s"), fname, keyList[RKEY_RESOURCENAME].val, *lineNum, lsfile);
                    lim_CheckError = WARNING_ERR;
                    freeKeyList (keyList);
                    continue;
                    }
                freeKeyList (keyList);
                continue;
                }

            if (keyList[RKEY_LOCATION].val != NULL && keyList[RKEY_LOCATION].val[0] != '\0')
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
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "doresourcemap", "malloc");
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
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "doresourcemap", "malloc");
                            return -1;
                        }
                        array.size++;
                    }

                    result = convertNegNotation_ (&(keyList[RKEY_LOCATION].val), &array);
                    if (result == 0)
                        {
                        /* catgets 5397 */
                        ls_syslog (LOG_WARNING, I18N (5397, "%s: %s(%d): convertNegNotation_: all the hosts are to be excluded %s !"), fname, lsfile, *lineNum, keyList[RKEY_LOCATION].val);
                        }
                    else if (result < 0)
                        {
                        ls_syslog (LOG_WARNING, I18N (5398, "%s: %s(%d): convertNegNotation_: Wrong syntax \'%s\'"), fname, lsfile, *lineNum, keyList[RKEY_LOCATION].val);
                        }
                    for (cc = 0; cc < array.size; cc++){
                        FREEUP (array.hosts[cc]);
                    }
                    FREEUP (array.hosts);
                    }

                if (addResourceMap (keyList[RKEY_RESOURCE_NAME].val, keyList[RKEY_LOCATION].val, lsfile, *lineNum, &isDefault) < 0)
                    {
                    /* catgets 5271 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5271, "%s: %s(%d): addResourceMap() failed for resource <%s>; ignoring line"), fname, lsfile, *lineNum, keyList[RKEY_RESOURCE_NAME].val);
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5272, "%s: %s(%d): No LOCATION specified for resource <%s>; ignoring the line"), fname, lsfile, *lineNum, keyList[RKEY_RESOURCE_NAME].val);
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
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5273, "%s: %s(%d): horizontal resource section not implemented yet"), fname, lsfile, *lineNum); 
        return (-1);
        }

    return (0);
}


static int
addSharedResourceInstance (uint nHosts, char **hosts, char *resName)
{
    char fname[] = "addSharedResourceInstance";
    struct sharedResourceInstance *tmp = NULL;
    struct hostNode *hPtr = NULL;
    static int firstFlag = 1;
    uint cnt = 0;
    int resNo = 0;

    if ((resNo = resNameDefined (resName)) < 0) {
         /* catgets 5274 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5274, "%s: Resource name <%s> not defined"), fname, resName);
        return (-1);
    }

    if (!(allInfo.resTable[resNo].flags & RESF_DYNAMIC)) {
        return 0;
    }

    tmp = (sharedResourceInstance *) malloc (sizeof (struct sharedResourceInstance));
    if (tmp == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, fname, "malloc", sizeof (struct sharedResourceInstance));
        return (-1);
    }
    else {
        tmp->nextPtr = NULL;
        tmp->resName = putstr_ (resName);
        tmp->hosts = (struct hostNode **) malloc (nHosts * sizeof (struct hostNode *));
        if (tmp->hosts == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, fname, "malloc", nHosts * sizeof (struct hostNode));
            return (-1);
        }
        else {
            cnt = 0;
            for ( uint i = 0; i < nHosts; i++) {
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
            for (uint i = 0; i < tmp->nHosts; i++) {
                sprintf (str, "%s %s ", str, tmp->hosts[i]->hostName);
            }
            ls_syslog (LOG_DEBUG, "%s", str);
        }
    }
    return 1;
}

static int
addResourceMap (char *resName, char *location, char *lsfile, uint lineNum, int *isDefault)
{
    static char fname[] = "addResourceMap";
    struct sharedResource *resource = NULL;
    struct hostNode *hPtr = NULL;
    uint numHosts   = 0;
    uint numCycle   = 0;
    int defaultWord = FALSE;
    int first       = TRUE;
    int error       = 0;
    int resNo       = 0;
    int dynamic     = 0;
    char *sp        = NULL;
    char *cp        = NULL;
    char ssp        = 'a';
    char *instance  = NULL;
    char *initValue = NULL;
    char **hosts    = NULL;

    int i = 0;

    *isDefault = FALSE;

    if (resName == NULL || location == NULL) {
        /* catgets 5382 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5382, "%s: %s(%d): Resource name <%s> location <%s>"), fname, lsfile, lineNum,resName ? resName : "NULL", (location ? location : "NULL"));
        return (-1);
    }

    if ((resNo = resNameDefined (resName)) < 0) {
        /* catgets 5275 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5275, "%s: %s(%d): Resource name <%s> not defined"), fname, lsfile, lineNum, resName);
        return (-1);
    }

    dynamic = (allInfo.resTable[resNo].flags & RESF_DYNAMIC);

    resource = inHostResourcs (resName);
    sp = location;


    i = 0;
    while (*sp != '\0') {
        if (*sp == '[') {
            i++;
        }
        else if (*sp == ']') {
            i--;
        }
        sp++;
    }
    if (i != 0) {
        /* catgets 5383 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5383, "%s: %s(%d): number of '[' is not match that of ']' in <%s> for resource <%s>; ignoring"), fname, lsfile, lineNum, location, resName);
        return (-1);
    }

    if ((initValue = (char *) malloc (4 * sizeof (char))) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "malloc");
        return (-1);
        }
    sp = location;

    while (sp != NULL && sp[0] != '\0') {
        for ( uint j = 0; j < numHosts; j++) {
            FREEUP (hosts[j]);
        }
        FREEUP (hosts);
        numHosts = 0;
        error = FALSE;
        instance = sp;
        initValue[0] = '\0';
        defaultWord = FALSE;
        while (*sp == ' ' && *sp != '\0') {
            sp++;
        }
        if (*sp == '\0') {
            FREEUP (initValue);
            if (first == TRUE) {
                return (-1);
            }
            else {
                return (0);
            }
        }
        cp = sp;
        if (*cp != '[' && *cp != '\0') {
            while (*cp && *cp != '@' && !(!iscntrl (*cp) && isspace (*cp))) {
                cp++;
            }
        }

        if (cp != sp) {
            unsigned long lsize = 0;
            ssp = cp[0];
            cp[0] = '\0';
            lsize = (strlen (sp) + 1) * sizeof (char);
            if ((initValue = (char *) realloc ((void *) initValue, lsize)) == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "realloc", lsize);
            }
            strcpy (initValue, sp);
            if (!isdigitstr_ (initValue)  && allInfo.resTable[resNo].valueType == LS_NUMERIC) {
                /* catgets 5386 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5386, "%s: %s(%d): Invalid characters (%s) used as NUMERIC resource value; ignoring"), fname, lsfile, lineNum, initValue);
                FREEUP (initValue);
                return (-1);
            }
            cp[0] = ssp;
            if (isspace (*cp)) {
                cp++;
            }
            if (*cp != '@') {
                error = TRUE;
            }
            sp = cp + 1;
        }
        if (isspace (*sp))
            sp++;

        if (*sp != '[' && *sp != '\0') {
            /* catgets 5384 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5384, "%s: %s(%d): Bad character <%c> in instance; ignoring"), fname, lsfile, lineNum, *sp);
            sp++;
        }
        if (isspace (*sp)) {
            sp++;
        }
        if (*sp == '[') {
            sp++;
            cp = sp;
            while (*sp != ']' && *sp != '\0') {
                sp++;
            }
            if (*sp == '\0') {
                /* catgets 5385 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5385, "%s: %s(%d): Bad format for instance <%s>; ignoring the instance"),  fname, lsfile, lineNum, instance);
                FREEUP (initValue);
                return (-1);
            }
            if (error == TRUE) {
                sp++;
                ssp = *sp;
                *sp = '\0';
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5385, "%s: %s(%d): Bad format for instance <%s>; ignoringthe instance"), fname, lsfile, lineNum, instance);
                *sp = ssp;
                continue;
            }
            *sp = '\0';
            sp++;

            if (initValue[0] == '\0' && !dynamic) {
                /* catgets 5276 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5276, "%s: %s(%d): Value must be defined for static resource; ignoring resource <%s>, instance <%s>"), fname, lsfile, lineNum, resName, instance);
                continue;
            }

            if (initValue[0] != '\0' && dynamic) {
                /* catgets 5277 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5277, "%s: %s(%d): Value <%s> ignored for dynamic resource <%s>, instance <%s>"), fname, lsfile, lineNum, initValue, resName, instance);
                initValue[0] = '\0';
            }

            numHosts = parseHostList (cp, lsfile, lineNum, &hosts,  &defaultWord);
            if ( 0 == numHosts ) {
                /* catgets 5387 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5387, "%s: %s(%d): getHostList(%s) failed; ignoring the instance <%s>"), fname, lsfile, lineNum, cp, instance);
                lim_CheckError = WARNING_ERR;
                continue;
            }
            if (defaultWord == TRUE) {
                *isDefault = TRUE;

                if (numHosts > 1) {
                    /* catgets 5388 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5388, "%s: %s(%d):  Other host is specified with reserved word <default> in the instance <%s> for resource <%s>;ignoring other hosts"), fname, lsfile, lineNum, instance, resName);
                }

                if (resource && resource->numInstances > 1) {
                    /* catgets 5389 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5389, "%s: %s(%d):  Other instances are specified with the instance <%s> for resource <%s>; ignoring the instance"), fname, lsfile, lineNum, instance, resName);
                    break;
                }
            }
            if (defaultWord == TRUE) {
                numCycle = numofhosts;
                FREEUP (hosts[0]);
            }
            else {
                numCycle = 1;
            }

            for (uint j = 0; j < numCycle; j++) {
                if (defaultWord == TRUE) {

                    if (dynamic) {
                        defaultRunElim = TRUE;
                    }

                    if (j == 0) {
                        hPtr = myClusterPtr->hostList;
                    }
                    else {
                        hPtr = hPtr->nextPtr;
                    }
                    if (hPtr == NULL) {
                        break;
                    }
                    FREEUP (hosts[0]);
                    hosts[0] = putstr_ (hPtr->hostName);
                    numHosts = 1;
                }
                if (resource == NULL) {

                    if (!(defaultWord && dynamic && allInfo.resTable[resNo].valueType == LS_NUMERIC) && (resource = addResource (resName, numHosts, hosts, initValue, lsfile, lineNum, TRUE)) == NULL) {
                        /* catgets 5390 */
                        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5390, "%s: %s(%d): addResource() failed; ignoring the instance <%s>"), fname, lsfile, lineNum, instance);
                    }
                }
                else {
                    if (addHostInstance (resource, numHosts, hosts, initValue, TRUE) < 0)
                        /* catgets 5391 */
                        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5391, "%s: %s(%d): addHostInstance() failed; ignoring the instance <%s>"), fname, lsfile, lineNum, instance);
                    }
                }
            defaultWord = FALSE;
            continue;
            }
        else {
            /* catgets 5392 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5392, "%s: %s(%d): No <[>  for instance in <%s>; ignoring"), fname, lsfile, lineNum, location);
            while (*sp != ']' && *sp != '\0') {
                sp++;
            }
            if (*sp == '\0') {
                FREEUP (initValue);
                return (-1);
            }
            sp++;
        }
    }
    for (uint j = 0; j < numHosts; j++) {
        FREEUP (hosts[j]);
    }
    FREEUP (hosts);
    FREEUP (initValue);
    return (0);
}


static uint
parseHostList (char *hostList, char *lsfile, uint lineNum, char ***hosts, int *hasDefault)
{
    static char fname[] = "parseHostList";
    char *host       = NULL;
    char *sp         = NULL;
    char *hostName   = NULL;
    char **hostTable = NULL;
    uint numHosts    = 0;

    if (hostList == NULL) {
        return (0);
    }

    sp = hostList;
    while ((host = getNextWord_ (&sp)) != NULL) {
        numHosts++;
    }
    hostTable = (char **) calloc (numHosts, sizeof (char *));
    if (NULL == hostTable && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "malloc");
        return (0);
    }
    sp = hostList;
    numHosts = 0;
    while ((host = getNextWord_ (&sp)) != NULL) {

        if ((hostName = validLocationHost (host)) == NULL) {
            /* catgets 5278 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5278, "%s: %s(%d): Invalid hostname <%s>;ignoring the host"), fname, lsfile, lineNum, host);
            lim_CheckError = WARNING_ERR;
            continue;
        }

        if ((hostTable[numHosts] = putstr_ (hostName)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "malloc");
            for (uint i = 0; i < numHosts; i++) {
                FREEUP (hostTable[i]);
            }
            FREEUP (hostTable);
            return (0);
        }

        if (!strcmp (hostName, "default")) {
            *hasDefault = TRUE;
        }
        numHosts++;
    }

    if (numHosts == 0) {
        FREEUP (hostTable);
        return (0);
    }

    *hosts = hostTable;
    return (numHosts);
}

static char *
validLocationHost (char *hostName)
{
    int num;

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

static void
initResItem (struct resItem *resTable)
{
    if (resTable == NULL) {
        return;
    }

    resTable->name[0] = '\0';
    resTable->des[0] = '\0';
    resTable->valueType = 0;
    resTable->orderType = NA;
    resTable->flags = RESF_GLOBAL;
    resTable->interval = 0;
}

static int
validType (char *type)
{
    if (type == NULL) {
        return (-1);
    }

    if (!strcasecmp (type, "Boolean")) {
        return (LS_BOOLEAN);
    }

    if (!strcasecmp (type, "String")) {
        return (LS_STRING);
    }

    if (!strcasecmp (type, "Numeric")) {
        return (LS_NUMERIC);
    }

    return (-1);
}

int
readCluster (int checkMode)
{
    static char fname[] = "readCluster()";
    char *hname = NULL;
    uint counter = 0;

    if (!myClusterPtr) {
        /* catgets 5279 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5279, "My cluster name %s is not configured in lsf.shared"), myClusterName);
        lim_Exit ("readCluster");
    }

    if (readCluster2 (myClusterPtr) < 0) {
        lim_Exit ("readCluster");
    }

    myClusterPtr->loadIndxNames = calloc (allInfo.numIndx, sizeof (char *));

    for (uint i = 0; i < allInfo.numIndx; i++) {
        myClusterPtr->loadIndxNames[i] = putstr_ (li[i].name);
    }

    if ((hname = ls_getmyhostname ()) == NULL) {
        lim_Exit ("readCluster/ls_getmyhostname");
    }

    myHostPtr = findHostbyList (myClusterPtr->hostList, hname);
    if (!myHostPtr) {
        myHostPtr = findHostbyList (myClusterPtr->clientList, hname);
        if (!myHostPtr) {
            /* catgets 5281 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5281, "%s: Local host %s not configured in Host section of file lsf.cluster.%s"), fname, hname, myClusterName);
            if (checkMode) {
                return -1;
            }
            else {
                lim_Exit ("readCluster");
            }
        }
        else
            {
            /* catgets 5282 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5282, "%s: Local host %s is configured as client-only in file lsf.cluster.%s; LIM will not run on a client-only host"), fname, hname, myClusterName);
            if (!checkMode) {
                lim_Exit ("readCluster");
            }
        }
    }

    for (uint i = 1; i < 8; i++) {
        if (myHostPtr->week[i] != NULL) {
            break;
        }
        counter = i;
    }

    if (counter == 8) {
        for (uint i = 1; i < 8; i++) {
            insertW (&(myHostPtr->week[i]), -1.0, 25.0);
        }
    }

    for ( uint i = 0; i < GET_INTNUM (allInfo.numIndx) + 1; i++) {
        myHostPtr->status[i] = 0;
    }
    checkHostWd ();

    if (nClusAdmins == 0) {
        char rootName[MAXLSFNAMELEN];

        /* catgets 5396 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5396, "%s: No LSF managers are specified in file lsf.cluster.%s, default cluster manager is root."), fname, myClusterName);

        clusAdminIds = malloc (sizeof (int));
        clusAdminIds[0] = 0;
        nClusAdmins = 1;
        clusAdminNames = malloc (sizeof (char *));
        getLSFUserByUid_ (0, rootName, sizeof (rootName));
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

static int
readCluster2 (struct clusterNode *clPtr)
{
    static char fname[] = "readCluster2()";
    char fileName[MAXFILENAMELEN];
    char *word;
    FILE *clfp;
    char *cp;
    uint lineNum = 0;
    int Error = FALSE;

    sprintf (fileName, "%s/lsf.cluster.%s",
             limParams[LSF_CONFDIR].paramValue, clPtr->clName);

    if (configCheckSum (fileName, &clPtr->checkSum) < 0)
        {
        return (-1);
        }
    if ((clfp = confOpen (fileName, "r")) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "confOpen", fileName);
        return (-1);
        }

    for (;;) {
        cp = getBeginLine (clfp, &lineNum);
        if (!cp)
            {
            FCLOSEUP (&clfp);
            if (clPtr->hostList)
                {
                if (Error)
                    return -1;
                else
                    {

                    adjIndx ();

                    chkUIdxAndSetDfltRunElim ();


                    return 0;
                    }
                }
            else if (!(clPtr->hostList))
                {
                /* catgets 5285 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5285, "%s: %s(%d): No hosts configured for cluster %s"), fname, fileName, lineNum, clPtr->clName);
                return -1;
                }
            }

        word = getNextWord_ (&cp);
        if (!word)
            {
            /* catgets 5286 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5286, "%s: %s(%d): Keyword expected after Begin. Ignoring section"), fname, fileName, lineNum);
            lim_CheckError = WARNING_ERR;
            doSkipSection (clfp, &lineNum, fileName, "unknown");
            }
        else if (strcasecmp (word, "clusteradmins") == 0)
            {
            if (clPtr != myClusterPtr)
                {
                doSkipSection (clfp, &lineNum, fileName, "clusteradmins");
                continue;
                }
            if (domanager (clfp, fileName, &lineNum, "clusteradmins") < 0)
                Error = TRUE;
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
            if (doresourcemap (clfp, fileName, &lineNum) < 0)
                Error = TRUE;
            continue;
            }
        else
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5287, "%s %s(%d): Invalid section name %s, ignoring section"), /* catgets 5287 */
                       fname, fileName, lineNum, word);
            lim_CheckError = WARNING_ERR;
            doSkipSection (clfp, &lineNum, fileName, word);
            }
        }

}

static void
adjIndx (void)
{
    static char fname[] = "adjIndx()";
    uint resNo = 0;
    int returnResult = 0;
    char **temp = NULL;
    struct resItem tmpTable;
    struct hostNode *hPtr = NULL;

    if (numHostResources <= 0) {
        return;
    }

    for ( uint i = 0; i < numHostResources; i++) {
        if ((returnResult = resNameDefined (hostResources[i]->resourceName)) < 0) {
            continue;
        }
        else {
            resNo = (uint)returnResult;
        }

        if ((allInfo.resTable[resNo].valueType != LS_NUMERIC) || !(allInfo.resTable[resNo].flags & RESF_SHARED)) {
            continue;
        }


        memcpy ((char *) &tmpTable, (char *) &allInfo.resTable[resNo], sizeof (struct resItem));
        for ( uint j = resNo; j < allInfo.nRes - 1; j++) {
            memcpy ((char *) &allInfo.resTable[j], (char *) &allInfo.resTable[j + 1], sizeof (struct resItem));
        }
        memcpy ((char *) &allInfo.resTable[allInfo.nRes - 1], (char *) &tmpTable, sizeof (struct resItem));


        if ((temp = realloc (shortInfo.resName, (shortInfo.nRes + 1) * sizeof (char *))) == NULL)
            {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            lim_Exit (fname);
            }
        shortInfo.resName = temp;
        if ((shortInfo.resName[shortInfo.nRes] = putstr_ (tmpTable.name)) == NULL)
            {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            lim_Exit (fname);
            }
        SET_BIT (shortInfo.nRes, shortInfo.numericResBitMaps);
        shortInfo.nRes++;

        if (tmpTable.flags & RESF_DYNAMIC)
            {

            for (uint k = NBUILTINDEX; k < allInfo.numIndx; k++)
                {
                if (strcasecmp (li[k].name, tmpTable.name) != 0) {
                    continue;
                }
                FREEUP (li[k].name);
                for (uint j = k; j < allInfo.numIndx - 1; j++)
                    {
                    memcpy ((char *) &li[j], (char *) &li[j + 1], sizeof (struct liStruct));
                    }
                break;
                }



            for (hPtr = myClusterPtr->hostList;
                 hPtr != NULL; hPtr = hPtr->nextPtr)
                {
                for (uint j =  resNo; j < allInfo.numIndx - 1; j++)
                    hPtr->busyThreshold[j] = hPtr->busyThreshold[j + 1];
                }
            allInfo.numUsrIndx--;
            allInfo.numIndx--;
            }
        }

}

static int
domanager (FILE * clfp, char *lsfile, uint *lineNum, char *secName)
{
    static char fname[] = "domanager()";
    char *linep;
    struct keymap keyList1[] = {
        {0, "    ", NULL, "MANAGER"},
        {0, "    ", NULL, NULL}
    };
    struct keymap keyList2[] = {
        {0, "    ", NULL, "ADMINISTRATORS"},
        {0, "    ", NULL, NULL}
    };
    struct keymap *keyList;

    if (lim_debug > 0 && lim_debug < 3)
        {
        char lsfUserName[MAXLSFNAMELEN];

        nClusAdmins = 1;
        clusAdminIds = (uid_t *) malloc (sizeof (uid_t));
        clusAdminGids = (uid_t *) malloc (sizeof (uid_t));
        if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) < 0)
            {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, fname, "getLSFUser_");
            return -1;
            }
        clusAdminIds[0] = getuid ();
        clusAdminGids[0] = getgid ();
        clusAdminNames = (char **) malloc (sizeof (char *));
        clusAdminNames[0] = putstr_ (lsfUserName);
        doSkipSection (clfp, lineNum, lsfile, secName);
        if (lim_CheckMode > 0) {
            /* catgets 5289 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5289, "%s: %s(%d): The cluster manager is the invoker <%s> in debug mode"), fname, lsfile, *lineNum, lsfUserName);
        }
        return 0;
        }

    linep = getNextLineC_ (clfp, lineNum, TRUE);
    if (!linep)
        {
        ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, secName);
        return -1;
        }

    if (isSectionEnd (linep, lsfile, lineNum, secName)) {
        return 0;
    }

    if (strcmp (secName, "clustermanager") == 0) {
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
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5291, "%s: %s(%d): keyword line format error for section %s, ignoring section"), fname, lsfile, *lineNum, secName);
            doSkipSection (clfp, lineNum, lsfile, secName);
            return -1;
            }

        if ((linep = getNextLineC_ (clfp, lineNum, TRUE)) != NULL)
            {
            if (isSectionEnd (linep, lsfile, lineNum, secName)) {
                return 0;
            }
            if (mapValues (keyList, linep) < 0)
                {
                /* catgets 5292 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5292, "%s: %s(%d): values do not match keys for section %s, ignoring section"), fname, lsfile, *lineNum, secName);
                doSkipSection (clfp, lineNum, lsfile, secName);
                return -1;
                }
            if (getClusAdmins (keyList[0].val, lsfile, lineNum, secName) < 0)
                {
                FREEUP (keyList[0].val);
                return -1;
                }
            else
                {
                FREEUP (keyList[0].val);
                return 0;
                }
            }
        }
    else
        {
        if (readHvalues (keyList, linep, clfp, lsfile, lineNum, TRUE, secName) <
            0)
            return -1;
        if (getClusAdmins (keyList[0].val, lsfile, lineNum, secName) < 0)
            {
            FREEUP (keyList[0].val);
            return -1;
            }
        else
            {
            FREEUP (keyList[0].val);
            return 0;
            }
        }
    return (0);

}

static int
getClusAdmins (char *line, char *lsfile, uint *lineNum, char *secName)
{
    static char fname[] = "getClusAdmins()";
    struct admins *admins;
    static char lastSecName[40];
    static int count = 0;

    if (count > 1)
        {
        /* catgets 5293 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5293, "%s: %s(%d): More than one %s section defined; ignored."), fname, lsfile, *lineNum, secName);
        return -1;
        }
    count++;
    if (strcmp (lastSecName, secName) == 0)
        {
        /* catgets 5294 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5294, "%s: %s(%d): section <%s> is multiply specified; ignoring the section"), fname, lsfile, *lineNum, secName);
        return (-1);
        }
    lserrno = LSE_NO_ERR;
    admins = getAdmins (line, lsfile, lineNum, secName);
    if (admins->nAdmins <= 0)
        {
        /* catgets 5295 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5295, "%s: %s(%d): No valid user for section %s: %s"), fname, lsfile, *lineNum, secName, line);
        return -1;
        }
    if (strcmp (secName, "clustermanager") == 0 && strcmp (lastSecName, "clusteradmins") == 0)
        {
        if (setAdmins (admins, A_THEN_M) < 0) {
            return (-1);
        }
    }
    else if (strcmp (lastSecName, "clustermanager") == 0 && strcmp (secName, "clusteradmins") == 0)
        {
        if (setAdmins (admins, M_THEN_A) < 0) {
            return (-1);
        }
    }
    else
        {
        if (setAdmins (admins, M_OR_A) < 0) {
            return (-1);
        }
    }
    strcpy (lastSecName, secName);
    return (0);

}

static int
setAdmins (struct admins *admins, int mOrA)
{
    static char fname[]   = "setAdmins()";
    uint workNAdmins      = 0;
    uint tempNAdmins      = 0;
    uid_t *tempAdminIds   = 0;
    gid_t *tempAdminGids  = 0;
    uint *workAdminIds    = 0;
    uint *workAdminGids   = 0;
    char **tempAdminNames = NULL;
    char **workAdminNames = NULL;

    tempNAdmins    = admins->nAdmins + nClusAdmins;
    tempAdminIds   = (uid_t *) malloc (tempNAdmins * sizeof (uid_t));
    tempAdminGids  = (gid_t *) malloc (tempNAdmins * sizeof (gid_t));
    tempAdminNames = (char **) malloc (tempNAdmins * sizeof (char *));
    if (!tempAdminIds || !tempAdminGids || !tempAdminNames) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        FREEUP (tempAdminIds);
        FREEUP (tempAdminGids);
        FREEUP (tempAdminNames);
        return (-1);
    }
    if (mOrA == M_THEN_A) {
        workNAdmins = nClusAdmins;
        workAdminIds = clusAdminIds;
        workAdminGids = clusAdminGids;
        workAdminNames = clusAdminNames;
    }
    else {
        workNAdmins = admins->nAdmins;
        workAdminIds = admins->adminIds;
        workAdminGids = admins->adminGIds;
        workAdminNames = admins->adminNames;
    }

    for ( uint i = 0; i < workNAdmins; i++) {
        tempAdminIds[i] = workAdminIds[i];
        tempAdminGids[i] = workAdminGids[i];
        tempAdminNames[i] = putstr_ (workAdminNames[i]);
    }
    tempNAdmins = workNAdmins;
    if (mOrA == M_THEN_A) {
        workNAdmins = admins->nAdmins;
        workAdminIds = admins->adminIds;
        workAdminGids = admins->adminGIds;
        workAdminNames = admins->adminNames;
    }
    else if (mOrA == A_THEN_M)
        {
        workNAdmins = nClusAdmins;
        workAdminIds = clusAdminIds;
        workAdminGids = clusAdminGids;
        workAdminNames = clusAdminNames;
        }
    else {
        workNAdmins = 0;
    }
    for ( uint i = 0; i < workNAdmins; i++)
        {
        if (isInlist (tempAdminNames, workAdminNames[i], tempNAdmins)) {
            continue;
        }
        tempAdminIds[tempNAdmins] = workAdminIds[i];
        tempAdminGids[tempNAdmins] = workAdminGids[i];
        tempAdminNames[tempNAdmins] = putstr_ (workAdminNames[i]);
        tempNAdmins++;
    }
    if (nClusAdmins > 0)
        {
        for (uint i = 0; i < nClusAdmins; i++) {
            FREEUP (clusAdminNames[i]);
        }
        FREEUP (clusAdminIds);
        FREEUP (clusAdminGids);
        FREEUP (clusAdminNames);
        }
    nClusAdmins = tempNAdmins;
    clusAdminIds = tempAdminIds;
    clusAdminGids = tempAdminGids;
    clusAdminNames = tempAdminNames;


    return (0);
}


#define EXINTERVAL              0
#define ELIMARGS                1
#define PROBE_TIMEOUT           2
#define ELIM_POLL_INTERVAL      3
#define HOST_INACTIVITY_LIMIT   4
#define MASTER_INACTIVITY_LIMIT 5
#define RETRY_LIMIT             6
#define ADJUST_DURATION         7
#define LSF_ELIM_DEBUG          8
#define LSF_ELIM_BLOCKTIME      9
#define LSF_ELIM_RESTARTS       10

static int
doclparams (FILE * clfp, char *lsfile, uint *lineNum)
{
    static char fname[] = "doclparams()";
    char *linep;
    int warning = FALSE;
    struct keymap keyList[] = {
        {0, "    ", NULL, "EXINTERVAL"},
        {0, "    ", NULL, "ELIMARGS"},
        {0, "    ", NULL, "PROBE_TIMEOUT"},
        {0, "    ", NULL, "ELIM_POLL_INTERVAL"},
        {0, "    ", NULL, "HOST_INACTIVITY_LIMIT"},
        {0, "    ", NULL, "MASTER_INACTIVITY_LIMIT"},
        {0, "    ", NULL, "RETRY_LIMIT"},
        {0, "    ", NULL, "ADJUST_DURATION"},
        {0, "    ", NULL, "LSF_ELIM_DEBUG"},
        {0, "    ", NULL, "LSF_ELIM_BLOCKTIME"},
        {0, "    ", NULL, "LSF_ELIM_RESTARTS"},
        {0, "    ", NULL, NULL}
    };

    linep = getNextLineC_ (clfp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "parameters");
        return -1;
    }

    if (isSectionEnd (linep, lsfile, lineNum, "parameters")) {
        return 0;
    }

    if (strchr (linep, '=') == NULL) {
        /* catgets 5298 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5298, "%s: %s(%d): vertical section not supported, ignoring section"), fname, lsfile, *lineNum);
        doSkipSection (clfp, lineNum, lsfile, "parameters");
        return -1;
    }
    else {
        if (readHvalues (keyList, linep, clfp, lsfile, lineNum, FALSE,  "parameters") < 0) {
            return -1;
        }
        if (keyList[EXINTERVAL].val)
            {
            if (!isanumber_ (keyList[0].val) || atof (keyList[0].val) < 0.001) {
                 /* catgets 5299 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5299, "%s: %s(%d): Invalid exchange interval in section parameters: %s. Ignoring."), fname, lsfile, *lineNum, keyList[EXINTERVAL].val);
                FREEUP (keyList[EXINTERVAL].val);
                warning = TRUE;
            }
            else {
                exchIntvl = (float)atof (keyList[EXINTERVAL].val);
            }
            FREEUP (keyList[EXINTERVAL].val);

            if (exchIntvl < 15) {
                int temp = (int) exchIntvl;
                resInactivityLimit = 180 / temp;
            }
        }

        if (keyList[ELIMARGS].val) {
            myClusterPtr->eLimArgs = putstr_ (keyList[1].val);

            if (!myClusterPtr->eLimArgs) {
                ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, fname, "malloc", keyList[ELIMARGS].val);
                return -1;
            }
            FREEUP (keyList[ELIMARGS].val);
        }

        if (keyList[PROBE_TIMEOUT].val) {
            if (!isint_ (keyList[PROBE_TIMEOUT].val) || atoi (keyList[PROBE_TIMEOUT].val) < 0)
                {
                /* catgets 5301 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5301, "%s: %s(%d): Invalid probe timeout value in section parameters: %s. Ignoring."), fname, lsfile, *lineNum, keyList[PROBE_TIMEOUT].val);
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5302, "%s: %s(%d): Invalid sample interval in section parameters: %s. Must be between 0.001 and 5. Ignoring."), fname, lsfile, *lineNum, keyList[ELIM_POLL_INTERVAL].val);
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5303, "%s: %s(%d): Invalid host inactivity limit in section parameters: %s. Ignoring."), fname, lsfile, *lineNum, keyList[HOST_INACTIVITY_LIMIT].val);
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5304, "%s: %s(%d): Invalid master inactivity limit in section parameters: %s. Ignoring."), fname, lsfile, *lineNum, keyList[MASTER_INACTIVITY_LIMIT].val);
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5305, "%s: %s(%d): Invalid host inactivity limit in section parameters: %s. Ignoring."), fname, lsfile, *lineNum, keyList[RETRY_LIMIT].val);   /* catgets 5305 */
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5306, "%s: %s(%d): Invalid load adjust duration in section parameters: %s. Ignoring."),  fname, lsfile, *lineNum, keyList[ADJUST_DURATION].val);
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
                ls_syslog (LOG_WARNING, I18N (5316, "LSF_ELIM_DEBUG invalid: %s, not debuging ELIM."), keyList[LSF_ELIM_DEBUG].val);
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
                ls_syslog (LOG_WARNING, I18N (5318, "LSF_ELIM_BLOCKTIME invalid: %s, blocking communication with ELIM."), keyList[LSF_ELIM_BLOCKTIME].val);
                warning = TRUE;
                ELIMblocktime = -1;
            }

            FREEUP (keyList[LSF_ELIM_BLOCKTIME].val);
            }


        if (ELIMdebug && ELIMblocktime == -1)
            {
            /* catgets 5340 */
            ls_syslog (LOG_WARNING, I18N (5340, "LSF_ELIM_DEBUG=y but LSF_ELIM_BLOCKTIME is not set/valid; LSF_ELIM_BLOCKTIME will be set to 2 seconds "));
            warning = TRUE;
            ELIMblocktime = 2;
            }


        if (keyList[LSF_ELIM_RESTARTS].val)
            {
            ELIMrestarts = atoi (keyList[LSF_ELIM_RESTARTS].val);

            if (!isint_ (keyList[LSF_ELIM_RESTARTS].val) || ELIMrestarts < 0)
                {
                /* catgets 5366 */
                ls_syslog (LOG_WARNING, I18N (5366, "LSF_ELIM_RESTARTS invalid: %s, unlimited ELIM restarts."), keyList[LSF_ELIM_RESTARTS].val);
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
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5308, "%s: Exchange interval must be greater than or equal to sampling interval. Setting exchange and sample interval to %f."), fname, sampleIntvl);
            exchIntvl = sampleIntvl;
            warning = TRUE;
            }

        if (warning == TRUE)
            return -1;

        return 0;
        }
}

static struct keymap *
initKeyList (void)
{
    static struct keymap *hostKeyList;

#define  HOSTNAME_    allInfo.numIndx
#define  MODEL        allInfo.numIndx+1
#define  TYPE         allInfo.numIndx+2
#define  ND           allInfo.numIndx+3
#define  RESOURCES    allInfo.numIndx+4
#define  RUNWINDOW    allInfo.numIndx+5
#define  REXPRI_      allInfo.numIndx+6
#define  SERVER_      allInfo.numIndx+7
#define  R            allInfo.numIndx+8
#define  S            allInfo.numIndx+9

    if (hostKeyList == NULL)
        {
        hostKeyList = calloc (allInfo.numIndx + 11, sizeof (struct keymap));
        if (hostKeyList == NULL) {
            return NULL;
        }
    }

    for (uint i = 0; i < S + 1; i++)
        {
        hostKeyList[i].key = "";
        hostKeyList[i].val = NULL;
        hostKeyList[i].position = 0;
        }

    hostKeyList[HOSTNAME_].key = "HOSTNAME";
    hostKeyList[MODEL].key     = "MODEL";
    hostKeyList[TYPE].key      = "TYPE";
    hostKeyList[ND].key        = "ND";
    hostKeyList[RESOURCES].key = "RESOURCES";
    hostKeyList[RUNWINDOW].key = "RUNWINDOW";
    hostKeyList[REXPRI_].key   = "REXPRI";
    hostKeyList[SERVER_].key   = "SERVER";
    hostKeyList[R].key         = "R";
    hostKeyList[S].key         = "S";
    hostKeyList[S + 1].key     = NULL;

    for ( uint i = 0; i < allInfo.numIndx; i++) {
        hostKeyList[i].key = allInfo.resTable[i].name;
    }

    return hostKeyList;
}

void
setMyClusterName (void)
{
    static char fname[] = "setMyClusterName()";
    struct keymap *keyList;
    FILE *fp;
    char clusterFile[MAXFILENAMELEN];
    char *cluster;
    char found = FALSE;
    char *lp;
    char *cp;
    char *hname;
    uint lineNum;

    if ((hname = ls_getmyhostname ()) == NULL)
        lim_Exit ("setMyClusterName/ls_getmyhostname failed");

    ls_syslog (LOG_DEBUG, "setMyClusterName: searching cluster files ...");
    cluster = myClusterPtr->clName;
    sprintf (clusterFile, "%s/lsf.cluster.%s", limParams[LSF_CONFDIR].paramValue, cluster);
    fp = confOpen (clusterFile, "r");

    if (!fp)
        {
        if (!found && !mcServersSet)
            {
            ls_syslog (LOG_ERR, I18N_CANNOT_OPEN, fname, clusterFile);
            }
        goto endfile;
        }

    lineNum = 0;

    for (;;)
        {
        if ((lp = getBeginLine (fp, &lineNum)) == NULL)
            {
            if (!found)
                {
                ls_syslog (LOG_DEBUG, "setMyClusterName: Local host %s not defined in cluster file %s", hname, clusterFile);
                }
            break;
            }

        cp = getNextWord_ (&lp);
        if (!cp)
            {
            if (!found)
                {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5317, "%s: %s(%d): Section name expected after Begin; section ignored."), fname, clusterFile, lineNum);    /* catgets 5317 */
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
                ls_syslog (LOG_ERR, I18N_PREMATURE_EOF,
                           fname, clusterFile, lineNum, "host");
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5319, "%s: %s(%d): horizontal host section not implemented yet, use vertical format: section ignored"),    /* catgets 5319 */
                           fname, clusterFile, lineNum);
                lim_CheckError = WARNING_ERR;
                }
            continue;
            }

        keyList = initKeyList ();

        if (!keyMatch (keyList, lp, FALSE))
            {
            if (!found)
                {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5320, "%s: %s(%d): keyword line format error for section Host, section ignored"), fname, clusterFile, lineNum);    /* catgets 5320 */
                lim_CheckError = WARNING_ERR;
                }
            continue;
            }
        if (keyList[HOSTNAME_].position == -1)
            {
            if (!found)
                {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5321, "%s: %s(%d): key HOSTNAME_ is missing in section host, section ignored"), fname, clusterFile, lineNum);  /* catgets 5321 */
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
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5322, "%s: %s(%d): values do not match keys for section Host, record ignored"), fname, clusterFile, lineNum);  /* catgets 5322 */
                    lim_CheckError = WARNING_ERR;
                    }
                continue;
                }

            if (Gethostbyname_ (keyList[HOSTNAME_].val) == NULL)
                {
                if (!found)
                    {
                    ls_syslog (LOG_ERR, "%\
                               s: %s(%d): Invalid hostname %s in section host, host ignored", fname, clusterFile, lineNum, keyList[HOSTNAME_].val);
                    lim_CheckError = WARNING_ERR;
                    }
                freeKeyList (keyList);
                continue;
                }

            if (strcasecmp (keyList[HOSTNAME_].val, hname) == 0)
                {
                if (!found)
                    {
                    ls_syslog (LOG_DEBUG, "\
                               setMyClusterName: local host %s belongs to cluster %s", hname, cluster);
                    found = TRUE;
                    strcpy (myClusterName, cluster);
                    freeKeyList (keyList);
                    break;
                    }
                else
                    {
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5324, "%s: %s(%d): local host %s defined in more than one cluster file. Previous definition was in lsf.cluster.%s, ignoring current definition"), fname, clusterFile, lineNum, hname, myClusterName);  /* catgets 5324 */
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

    if (!found)
        {
        ls_syslog (LOG_ERR, "\
                   %s: unable to find the cluster file containing local host %s", fname, hname);
        lim_Exit ("setMyClusterName");
        }
}

static void
freeKeyList (struct keymap *keyList)
{
    int i;

    for (i = 0; keyList[i].key != NULL; i++)
        if (keyList[i].position != -1)
            FREEUP (keyList[i].val);
}


static int
dohosts (FILE * clfp, struct clusterNode *clPtr, char *lsfile, uint *lineNum)
{
    static char fname[] = "dohosts()";
    static struct hostEntry hostEntry;
    char *sp     = NULL;
    char *cp     = NULL;
    char *word   = NULL;
    char *window = NULL;
    char *linep  = NULL;
    int n = 0;
    int ignoreR = FALSE;
    struct keymap *keyList;

    hostEntry.busyThreshold = calloc (allInfo.numIndx, sizeof (float));
    if (hostEntry.busyThreshold == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "malloc");
        return -1;
    }
    hostEntry.resList = calloc (allInfo.nRes, sizeof (char *));
    if (hostEntry.resList == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "malloc");
        return -1;
    }
    hostEntry.nRes = 0;

    /* Must be called after allInfo is initialiazed
     * here we have a dependency on lsf.shared
     */
    keyList = initKeyList ();

    linep = getNextLineC_ (clfp, lineNum, TRUE);
    if (!linep)
        {
        ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "host");
        return -1;
        }

    if (isSectionEnd (linep, lsfile, lineNum, "host"))
        {
        /* catgets 5329 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5329, "%s: %s(%d): empty host section"), fname, lsfile, *lineNum);
        return -1;
        }

    if (strchr (linep, '=') == NULL)
        {
        if (!keyMatch (keyList, linep, FALSE))
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5330, "%s: %s(%d): keyword line format error for section host, ignoring section"), fname, lsfile, *lineNum);   /* catgets 5330 */
            doSkipSection (clfp, lineNum, lsfile, "host");
            return -1;
            }

        for (uint i = 0; keyList[i].key != NULL; i++)
            {

            if (keyList[i].position != -1) {
                continue;
            }

            if ((strcasecmp ("hostname", keyList[i].key) == 0) || (strcasecmp ("model", keyList[i].key) == 0) || (strcasecmp ("type", keyList[i].key) == 0) || (strcasecmp ("resources", keyList[i].key) == 0))
                {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5331, "%s: %s(%d): keyword line: key %s is missing in section host, ignoring section"), fname, lsfile, *lineNum, keyList[i].key);  /* catgets 5331 */
                doSkipSection (clfp, lineNum, lsfile, "host");
                freeKeyList (keyList);
                return -1;
                }
            }

        if (keyList[R].position != -1 && keyList[SERVER_].position != -1)
            {
            /* catgets 5332 */
            ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5332, "%s: %s(%d): keyword line: conflicting keyword definition: you cannot define both 'R' and 'SERVER_'. 'R' ignored"), fname, lsfile, *lineNum);
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

            if (isSectionEnd (linep, lsfile, lineNum, "host"))
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
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5333, "%s: %s(%d): values do not match keys for section host, ignoring line"), fname, lsfile, *lineNum);   /* catgets 5333 */
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

            strcpy (hostEntry.hostName, keyList[HOSTNAME_].val);
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
                ls_syslog (LOG_INFO, (_i18n_msg_get(ls_catd, NL_SETN, 5476, "%s: %s(%d): value for threshold ut <%2.2f> is greater than 1, assumming <%5.1f%%>")), "dohosts", lsfile, *lineNum, hostEntry.busyThreshold[UT], hostEntry.busyThreshold[UT]);
                hostEntry.busyThreshold[UT] /= 100.0;
                }
            putThreshold (PG,  &hostEntry,  keyList[PG].position,   keyList[PG].val,   INFINIT_LOAD);
            putThreshold (IO,  &hostEntry,  keyList[IO].position,   keyList[IO].val,   INFINIT_LOAD);
            putThreshold (LS,  &hostEntry,  keyList[LS].position,   keyList[LS].val,   INFINIT_LOAD);
            putThreshold (IT,  &hostEntry,  keyList[IT].position,   keyList[IT].val,  -INFINIT_LOAD);
            putThreshold (TMP, &hostEntry,  keyList[TMP].position,  keyList[TMP].val, -INFINIT_LOAD);
            putThreshold (SWP, &hostEntry,  keyList[SWP].position,  keyList[SWP].val, -INFINIT_LOAD);
            putThreshold (MEM, &hostEntry,  keyList[MEM].position,  keyList[MEM].val, -INFINIT_LOAD);

            for ( uint i = NBUILTINDEX; i < allInfo.numIndx; i++)
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

            for (uint i = NBUILTINDEX + allInfo.numUsrIndx; i < allInfo.numIndx; i++) {
                hostEntry.busyThreshold[i] = INFINIT_LOAD;
            }

            for (uint i = 0; i < allInfo.numIndx; i++) {
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
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5335, "%s: %s(%d): horizontal host section not implemented yet, use vertical format: section ignored"), fname, lsfile, *lineNum);  /* catgets 5335 */
        doSkipSection (clfp, lineNum, lsfile, "host");
        return -1;
        }


    ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, fname, lsfile, *lineNum, "host");
    return -1;

}

static void
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
    for ( uint i = 0; i < allInfo.nTypes; i++) {
        if (strcmp (allInfo.hostTypes[i], typeName) == 0) {
            assert( i <= INT_MAX );
            return (int)(i);
        }
    }
    return (-1);
}

int
archNameToNo (const char *archName)
{
    int arch_speed = 0;
    int curr_speed = 0;
    int best_speed = 0;
    uint best_pos  = 0;
    unsigned long len = 0;
    char *p = NULL;

    for (uint i = 0; i < allInfo.nModels; ++i)
        {
        if (strcmp (allInfo.hostArchs[i], archName) == 0)
            {
            assert( i <= INT_MAX );
            return (int)(i);
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
    for (uint i = 0; i < allInfo.nModels; ++i)
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
                best_pos = (uint)i;
                }
            }
        else
            {
            if (best_speed <= curr_speed)
                {
                best_speed = curr_speed;
                assert( i >= 0);
                best_pos = (uint)i;
                }
            }
        }

    if (best_pos) {
        assert( best_pos <= INT_MAX );
        return (int)(best_pos);
    }

    return (-1);
}

static int
modelNameToNo (char *modelName)
{
    for ( uint i = 0; i < allInfo.nModels; i++) {
        if (strcmp (allInfo.hostModels[i], modelName) == 0) {
            assert( i <= INT_MAX );
            return (int)(i);        }
    }

    return -1;
}

static struct hostNode *
addHost_ (struct clusterNode *clPtr, struct hostEntry *hEntPtr,  char *window, char *fileName, uint *lineNumPtr)
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

    if (hEntPtr->rcv)
        hPtr->hostNo = clPtr->hostList ? clPtr->hostList->hostNo + 1 : 0;
    else
        hPtr->hostNo = clPtr->clientList ? clPtr->clientList->hostNo + 1 : 0;

    if (saveHostIPAddr (hPtr, hp) < 0)
        {
        ls_syslog (LOG_ERR, "%s Can not save internet address of host %s", __func__, hp->h_name);
        freeHostNodes (hPtr, FALSE);
        return NULL;
        }

    hPtr->statInfo.nDisks = hEntPtr->nDisks;
    hPtr->rexPriority = hEntPtr->rexPriority;

    for (uint i = 0; i < allInfo.numIndx; i++)
        hPtr->busyThreshold[i] = hEntPtr->busyThreshold[i];

    for (uint i = 0; i < 8; i++) {
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

    if (hEntPtr->rcv)
        for ( uint resNo = 0; resNo < allInfo.nRes; resNo++)
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

    numofhosts++;

    return hPtr;
}

struct hostNode *
addFloatClientHost (struct hostent *hp)
{
    struct hostNode *hPtr, *lastHPtr;
    int i;

    if (hp == NULL)
        {
        ls_syslog (LOG_ERR, "\
                   %s: Invalid hostAddr. Ignoring host", __func__);
        return NULL;
        }

    if (findHostInCluster (hp->h_name))
        {
        ls_syslog (LOG_ERR, "%\
                   s: %s already defined in this cluster", __func__, hp->h_name);
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
         hPtr->naddr++);

    if (hPtr->naddr)
        {
        hPtr->addr = malloc (hPtr->naddr * sizeof (u_int));
        }
    else
        hPtr->addr = 0;

    for (i = 0; i < hPtr->naddr; i++)
        memcpy ((char *) &hPtr->addr[i], hp->h_addr_list[i], hp->h_length);

    for (i = 0; i < 8; i++)
        hPtr->week[i] = NULL;

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
             lastHPtr = lastHPtr->nextPtr);
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
    static char fname[] = "removeFloatClientHost()";
    struct hostNode *tempPtr = NULL;

    if (logclass & LC_TRACE)
        {
        ls_syslog (LOG_DEBUG, "%s Entering ... ", fname);
        }

    if (hPtr == NULL)
        {
        ls_syslog (LOG_ERR, I18N (5408, "%s: hostNode is invalid"), /* catgets 5408 */
                   fname);
        return (-1);
        }

    if (myClusterPtr->clientList == hPtr)
        {

        myClusterPtr->clientList = hPtr->nextPtr;
        }
    else
        {

        for (tempPtr = myClusterPtr->clientList;
             tempPtr && tempPtr->nextPtr != hPtr; tempPtr = tempPtr->nextPtr);
        if (tempPtr == NULL)
            {
            ls_syslog (LOG_ERR, I18N (5409, "%s: host <%s> not found in client list"),  /* catgets 5409 */
                       fname, hPtr->hostName);
            return (-1);
            }
        tempPtr->nextPtr = hPtr->nextPtr;
        }
    hPtr->nextPtr = NULL;


    numofhosts--;
    freeHostNodes (hPtr, FALSE);

    return (0);
}


struct hostNode *
initHostNode (void)
{
    struct hostNode *hPtr;

    hPtr = calloc (1, sizeof (struct hostNode));
    if (hPtr == NULL)
        {
        ls_syslog (LOG_ERR, "%s: malloc failed %m", __func__);
        return NULL;
        }

    hPtr->resBitMaps = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));
    hPtr->DResBitMaps = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));
    hPtr->status = calloc ((1 + GET_INTNUM (allInfo.numIndx)), sizeof (int));
    hPtr->loadIndex = calloc (allInfo.numIndx, sizeof (float));
    hPtr->uloadIndex = calloc (allInfo.numIndx, sizeof (float));
    hPtr->busyThreshold = calloc (allInfo.numIndx, sizeof (float));

    for (uint i = 0; i < allInfo.numIndx; i++)
        {
        hPtr->loadIndex[i] = INFINIT_LOAD;
        hPtr->uloadIndex[i] = INFINIT_LOAD;
        }

    for ( uint i = NBUILTINDEX; i < allInfo.numIndx; i++)
        hPtr->busyThreshold[i] = (allInfo.resTable[i].orderType == INCR) ? INFINIT_LOAD : -INFINIT_LOAD;

    hPtr->use = -1;
    hPtr->expireTime = -1;
    hPtr->status[0] = LIM_UNAVAIL;

    return hPtr;
}

void
freeHostNodes (struct hostNode *hPtr, int allList)
{
    int i;
    struct hostNode *next;

    while (hPtr)
        {

        FREEUP (hPtr->hostName);
        FREEUP (hPtr->addr);
        FREEUP (hPtr->windows);

        if (allList == FALSE)
            {
            for (i = 0; i < 8; i++)
                FREEUP (hPtr->week[i]);
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

        if (allList == TRUE)
            hPtr = next;
        }
}


static struct sharedResource *
addResource (char *resName, uint nHosts, char **hosts, char *value, char *fileName, uint lineNum, int resourceMap)
{
    static char fname[] = "addResource()";

    struct sharedResource *temp = NULL;
    struct sharedResource **temp1 = NULL;


    assert( *fileName );
    assert( lineNum );

    if (resName == NULL || hosts == NULL) {
        return (NULL);
    }

    temp = (struct sharedResource *) malloc (sizeof (struct sharedResource));
    if (NULL == temp && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        return (NULL);
    }
    if ((temp->resourceName = putstr_ (resName)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        return (NULL);
    }
    temp->numInstances = 0;
    temp->instances = NULL;
    if (addHostInstance (temp, nHosts, hosts, value, resourceMap) < 0) {
        return (NULL);
    }

    if (numHostResources == 0) {
        temp1 =(struct sharedResource **) malloc (sizeof (struct sharedResource *));
    }
    else {
        temp1 = (struct sharedResource **) realloc (hostResources, (numHostResources + 1) * sizeof (struct sharedResource *));
    }

    if (temp1 == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        freeSharedRes (temp);
        for ( uint i = 0; i < numHostResources; i++) {
            freeSharedRes (hostResources[i]);
        }
        FREEUP (hostResources);
        return (NULL);
    }
    hostResources = temp1;
    hostResources[numHostResources] = temp;
    numHostResources++;
    return (temp);
}

static void
freeSharedRes (struct sharedResource *sharedRes)
{
    if (sharedRes == NULL) {
        return;
    }
    FREEUP (sharedRes->resourceName);

    for ( uint i = 0; i < sharedRes->numInstances; i++) {
        freeInstance (sharedRes->instances[i]);
    }
    FREEUP (sharedRes);

    return;
}

static int
addHostInstance (struct sharedResource *sharedResource, uint nHosts, char **hostNames, char *value, int resourceMap)
{

    static char fname[] = "addHostInstance()";
    uint numList = 0;
    static char **temp = NULL;
    static uint numHosts = 0;
    char **hostList = NULL ;
    struct resourceInstance *instance = NULL;

    if (nHosts <= 0 || hostNames == NULL) {
        return (-1);
    }

    if (resourceMap == FALSE) {
        if (sharedResource->numInstances > 1) {
            /* catgets 5405 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5405, "%s: More than one instatnce defined for the resource <%s> on host <%s> in host section; ignoring"), fname, sharedResource->resourceName, hostNames[0]);
            return (-1);
        }
        if (sharedResource->numInstances == 0) {
            if (addInstance (sharedResource, nHosts, hostNames, value) == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "addInstance");
                return (-1);
            }
        }
        else {
            if (addHostList (sharedResource->instances[0], nHosts, hostNames) < 0) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "addHostList");
                return (-1);
            }
        }
        instance = sharedResource->instances[0];

        if (addHostNodeIns (instance, nHosts, hostNames) < 0) {
            return (-1);
        }
    }
    else {
        if (numHosts == 0 && temp == NULL) {
            temp = (char **) malloc (numofhosts * sizeof (char *));
            if(NULL == temp && ENOMEM == errno ) {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
                return -1;
            }
        }
        else {
            for ( uint i = 0; i < numHosts; i++) {
                FREEUP (temp[i]);
            }
        }
        numHosts = 0;
        for (uint i = 0; i < nHosts; i++) {

            if ((hostList = getValidHosts (hostNames[i], &numList, sharedResource)) == NULL) {
                continue;
            }
            for ( uint j = 0; j < numList; j++) {

                int duplicated = 0;
                for ( uint k = 0; k < numHosts; k++)  {
                    if (!strcmp (temp[k], hostList[j])) {
                        duplicated = 1;
                        break;
                    }
                }
                if (duplicated) {
                    /* catgets 5478 */
                    ls_syslog (LOG_WARNING, I18N (5478, "%s: Host %s is duplicated in resource %s mapping."), fname, hostList[j], sharedResource->resourceName);
                    continue;
                }
                temp[numHosts] = putstr_ (hostList[j]);
                if (temp[numHosts] == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
                    return -1;
                }
                numHosts++;
            }
        }
        if ((instance = addInstance (sharedResource, numHosts, temp, value)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname, "addInstance");
            return (-1);
            }

        if (addHostNodeIns (instance, numHosts, temp) < 0) {
            return (-1);
        }

        if (addSharedResourceInstance( numHosts, temp, sharedResource->resourceName) < 0) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, fname,  "addSharedResourceInstance");
            return -1;
        }
    }
    return (0);
}

static char **
getValidHosts (char *hostName, uint *numHost, struct sharedResource *resource)
{
    static char fname[] = "getValidHosts";
    static char **temp = NULL;
    int num;
    struct hostNode *hPtr;

    *numHost = 0;
    if (temp == NULL)
        {
        if ((temp = malloc (numofhosts * sizeof (char *))) == NULL)
            {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            return NULL;
            }
        }

    if (!strcmp (hostName, "all") || !strcmp (hostName, "others"))
        {
        if (resource->numInstances > 0 && !strcmp (hostName, "all"))
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5349, "%s: Shared resource <%s> has more than one instance, reserved word <all> can not be specified;ignoring"), fname, resource->resourceName);   /* catgets 5349 */
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
            ls_syslog (LOG_ERR, "\
                       %s: Host %s is not used by cluster %s; ignored", fname, hostName, myClusterName);
            return NULL;
            }
        if (isInHostList (resource, hostName) != NULL)
            {
            ls_syslog (LOG_ERR, "\
                       %s: Host %s is defined in more than one instance for resource %s; ignored", __func__, hostName, resource->resourceName);
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

static int
addHostNodeIns (struct resourceInstance *instance, uint nHosts, char **hostNames)
{
    int resNo;
    struct hostNode *hPtr;
    struct resourceInstance **temp;

    if ((resNo = resNameDefined (instance->resName)) < 0)
        {
        ls_syslog (LOG_ERR, "%s: Resource name <%s> is not defined in resource section in lsf.shared", __func__, instance->resName);
        return 0;
        }

    for ( uint i = 0; i < nHosts; i++)
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
    return (0);
}

static struct resourceInstance *
isInHostNodeIns (char *resName, uint numInstances, struct resourceInstance **instances)
{
    if (numInstances <= 0 || instances == NULL) {
        return NULL;
    }
    for (uint i = 0; i < numInstances; i++) {
        if (!strcmp (resName, instances[i]->resName)) {
            return (instances[i]);
        }
    }
    return NULL;
}

static int
addHostList (struct resourceInstance *resourceInstance, uint nHosts, char **hostNames)
{
    static char fname[] = "addHostList()";
    struct hostNode **temp;
    struct hostNode *hostPtr;

    if (resourceInstance == NULL || nHosts <= 0 || hostNames == NULL) {
        return (-1);
    }

    if (resourceInstance->nHosts == 0) {
        temp = (struct hostNode **) malloc (nHosts * sizeof (struct hostNode *));
    }
    else{
        temp = (struct hostNode **) realloc (resourceInstance->hosts, (resourceInstance->nHosts + 1) * sizeof (struct hostNode *));
    }

    if (temp == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        return (-1);
    }
    resourceInstance->hosts = temp;

    for ( uint i = 0; i < nHosts; i++) {
        if ((hostPtr = findHostbyList (myClusterPtr->hostList, hostNames[i])) == NULL) {
            ls_syslog (LOG_DEBUG3, "addHostList: Host <%s> is not used by cluster <%s> as a server:ignoring", hostNames[i], myClusterName);
            continue;
        }
        resourceInstance->hosts[resourceInstance->nHosts] = hostPtr;
        resourceInstance->nHosts++;
    }
    return (0);

}

static struct resourceInstance *
addInstance (struct sharedResource *sharedResource, uint nHosts, char **hostNames, char *value)
{

    static char fname[] = "addInstance()";
    int resNo = 0;
    struct resourceInstance **insPtr, *temp;
    struct hostNode *hPtr;

    if (nHosts <= 0 || hostNames == NULL) {
        return (NULL);
    }

    if ((temp = initInstance ()) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        return (NULL);
        }
    if ((temp->hosts =(struct hostNode **) malloc (nHosts * sizeof (struct hostNode *))) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        return (NULL);
        }
    temp->resName = sharedResource->resourceName;
    resNo = validResource (temp->resName);
    for ( uint i = 0; i < nHosts; i++)
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
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        freeInstance (temp);
        return (NULL);
        }
    if ((insPtr = (struct resourceInstance **) myrealloc (sharedResource->instances, (sharedResource->numInstances + 1) * sizeof (char *))) == NULL)
        {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "myrealloc");
        freeInstance (temp);
        return (NULL);
        }
    sharedResource->instances = insPtr;
    sharedResource->instances[sharedResource->numInstances] = temp;
    sharedResource->numInstances++;

    return (temp);
}


static struct resourceInstance *
initInstance (void)
{

    static char fname[] = "initInstance()";
    struct resourceInstance *temp;

    if ((temp = (struct resourceInstance *) malloc (sizeof (struct resourceInstance))) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
        return (NULL);
    }
    temp->nHosts       = 0;
    temp->resName      = NULL;
    temp->hosts        = NULL;
    temp->orignalValue = NULL;
    temp->value        = NULL;
    temp->updateTime   = 0;
    temp->updHost      = NULL;

    return (temp);
}

static void
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
isInHostList (struct sharedResource *sharedResource, char *hostName)
{
    if (sharedResource->numInstances <= 0) {
        return (NULL);
    }
    
    for ( uint i = 0; i < sharedResource->numInstances; i++) {
        if (sharedResource->instances[i]->nHosts <= 0 || sharedResource->instances[i]->hosts == NULL) {
            continue;
        }
        for ( uint j = 0; j < sharedResource->instances[i]->nHosts; j++) {
            if (strcmp(sharedResource->instances[i]->hosts[j]->hostName, hostName) == 0) {
                return (sharedResource->instances[i]);
            }
        }
    }
    return (NULL);
}

struct sharedResource *
inHostResourcs (char *resName)
{
    if (numHostResources <= 0) {
        return NULL;
    }
    
    for ( uint i = 0; i < numHostResources; i++) {
        if (strcmp (hostResources[i]->resourceName, resName) == 0) {
            return hostResources[i];
        }
    }
    return NULL;
    
}

int
validResource (const char *resName)
{
    for ( uint i = 0; i < shortInfo.nRes; i++) {
        if (strcmp (shortInfo.resName[i], resName) == 0) {
            assert( i <= INT_MAX );
            return (int)(i);
        }
    }

    return -1;
}

int
validLoadIndex (const char *resName)
{
    for ( uint i = 0; i < allInfo.numIndx; i++) {
        if (strcmp (li[i].name, resName) == 0) {
            assert( i <= INT_MAX );
            return (int)(i);
        }
    }

    return (-1);
}


bool_t
validHostType (const char *hType)
{
    for ( uint i = 0; i < shortInfo.nTypes; i++) {
        if (strcmp (shortInfo.hostTypes[i], hType) == 0) {
            assert( i <= INT_MAX );
            return (int)(i);
        }
    }
    return (-1);
}

int
validHostModel (const char *hModel)
{
    for ( uint i = 0; i < allInfo.nModels; i++) {
        if (strcmp (allInfo.hostModels[i], hModel) == 0) {
            assert( i <= INT_MAX );
            return (int)(i);
        }
    }
    return (-1);
}


static int cntofdefault = 0;
static char
addHostType (char *type)
{
    static char fname[] = "addHostType()";

    if (allInfo.nTypes == MAXTYPES)
        {
        /* catgets 5353 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5353, "%s: Too many host types defined in section HostType. You can only define up to %d host types; host type %s ignored"), fname, MAXTYPES, type);
        return (FALSE);
        }

    for ( uint i = 0; i < allInfo.nTypes; i++) {
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
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5354, "%s: host type %s multiply defined"), fname, type);
        return (FALSE);
    }
    strcpy (allInfo.hostTypes[allInfo.nTypes], type);
    shortInfo.hostTypes[shortInfo.nTypes] = allInfo.hostTypes[allInfo.nTypes];
    allInfo.nTypes++;
    shortInfo.nTypes++;
    return (TRUE);
}

static char
addHostModel (char *model, char *arch, double factor)
{
    static char fname[] = "addHostModel()";

    if (allInfo.nModels == MAXMODELS) {
        /* catgets 5355 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5355, "%s: Too many host models defined in section HostModel. You can only define up to %d host models; host model %s ignored"), fname, MAXMODELS, model);
        return (FALSE);
        }

    if (!strcmp (model, "DEFAULT")) {
        strcpy (allInfo.hostArchs[1], arch ? arch : "");
        allInfo.cpuFactor[1] = factor;
        return (TRUE);
    }

    for ( uint i = 0; i < allInfo.nModels; ++i) {

        if (!arch && strcmp (allInfo.hostModels[i], model) == 0) {
            /* catgets 5357 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5357, "%s: host model %s multiply defined"), fname, model);
            return (TRUE);
        }

        if (!arch || strcmp (allInfo.hostArchs[i], arch) != 0)  {
            continue;
        }

        /*catgets 5479 */
        ls_syslog (LOG_ERR, I18N (5479, "%s: host architecture %s defined multiple times"), fname, arch);
        
        return (TRUE);
    }

    strcpy (allInfo.hostModels[allInfo.nModels], model);
    strcpy (allInfo.hostArchs[allInfo.nModels], arch ? arch : "");
    allInfo.cpuFactor[allInfo.nModels] = factor;
    shortInfo.hostModels[shortInfo.nModels] =
    allInfo.hostModels[allInfo.nModels];
    allInfo.nModels++;
    shortInfo.nModels++;
    return (TRUE);
}

static struct clusterNode *
addCluster (char *clName, char *candlist)
{
    char *sp            = NULL;
    char *word          = NULL;
    struct hostent *hp  = NULL;
    static uint nextClNo = 0;
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

static char *
findClusterServers (char *clName)
{
    static char fname[] = "findClusterServers";
    static char servers[MAXLINELEN];
    FILE *clfp;
    char *cp = NULL ;
    uint lineNum = 0;
    char fileName[MAXFILENAMELEN] = { 'a' };
    char *word = NULL;
    char *linep = NULL;
    
    sprintf (fileName, "%s/lsf.cluster.%s", limParams[LSF_CONFDIR].paramValue, clName);
    servers[0] = '\0';
    
    if ((clfp = confOpen (fileName, "r")) == NULL)
        {
        return (servers);
        }
    
    for (;;)
        {
        cp = getBeginLine (clfp, &lineNum);
        if (!cp)
            {
            FCLOSEUP (&clfp);
            return (servers);
            }
        word = getNextWord_ (&cp);
        if (!word)
            {
            /* catgets 5360 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5360, "%s: %s(%d): Keyword expected after Begin, ignoring section"), fname, fileName, lineNum);
            lim_CheckError = WARNING_ERR;
            doSkipSection (clfp, &lineNum, fileName, "unknown");
            }
        else if (strcasecmp (word, "host") == 0)
            {
            char first = TRUE;
            int nServers = 0;
            
            
            while ((linep = getNextLineC_ (clfp, &lineNum, TRUE)) != NULL)
                {
                if (isSectionEnd (linep, fileName, &lineNum, "host"))
                    {
                    FCLOSEUP (&clfp);
                    return (servers);
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
            return (servers);
            }
        else
            {
            doSkipSection (clfp, &lineNum, fileName, word);
            }
        }
}


void
reCheckRes (void)
{
    uint j    = 0;
    struct resItem *newTable = NULL;

    allInfo.numIndx = 0;
    newTable = calloc (allInfo.nRes, sizeof (struct resItem));
    if (newTable == NULL) {
        lim_Exit (__func__);
    }

    for (uint i = 0; i < allInfo.nRes; i++) {
        if (allInfo.resTable[i].valueType == LS_NUMERIC && (allInfo.resTable[i].flags & RESF_DYNAMIC) && (allInfo.resTable[i].flags & RESF_GLOBAL)) {
            memcpy (&newTable[j], &allInfo.resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    for (uint i = 0; i < allInfo.nRes; i++) {
        
        if (allInfo.resTable[i].valueType == LS_NUMERIC && (!(allInfo.resTable[i].flags & RESF_DYNAMIC) || !(allInfo.resTable[i].flags & RESF_GLOBAL))) {
            memcpy (&newTable[j], &allInfo.resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    for (uint i = 0; i < allInfo.nRes; i++) {
        if (allInfo.resTable[i].valueType == LS_BOOLEAN) {
            memcpy (&newTable[j], &allInfo.resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    for (uint i = 0; i < allInfo.nRes; i++) {
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

    for ( uint resNo = 0; resNo < allInfo.nRes; resNo++) {

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
    for ( uint i = 0; i < allInfo.nModels; i++) {
        shortInfo.hostModels[i] = allInfo.hostModels[i];
        shortInfo.cpuFactors[i] = allInfo.cpuFactor[i];
    }

    return;
}

static int
reCheckClusterClass (struct clusterNode *clPtr)
{
    static char fname[] = "reCheckClusterClass()";
    struct hostNode *hPtr = NULL;
    
    clPtr->resClass     = 0;
    clPtr->typeClass    = 0;
    clPtr->modelClass   = 0;
    clPtr->numHosts     = 0;
    clPtr->numClients   = 0;
    clPtr->nRes         = 0;

    ls_syslog (LOG_DEBUG1, "reCheckClusterClass: cluster <%s>", clPtr->clName);
    if (clPtr->resBitMaps == NULL) {

        clPtr->resBitMaps = calloc (GET_INTNUM (allInfo.nRes), sizeof (int));
        if (clPtr->resBitMaps == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            return (-1);
        }

        for (uint i = 0; i < GET_INTNUM (allInfo.nRes); i++)
            clPtr->resBitMaps[i] = 0;
        }

    if (clPtr->hostTypeBitMaps == NULL) {
        clPtr->hostTypeBitMaps = calloc (GET_INTNUM (allInfo.nTypes), sizeof (int));
        if ( NULL == clPtr->hostTypeBitMaps && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            return (-1);
        }
        for ( uint i = 0; i < GET_INTNUM (allInfo.nTypes); i++) {
            clPtr->hostTypeBitMaps[i] = 0;
        }
    }

    if (clPtr->hostModelBitMaps == NULL) {
        clPtr->hostModelBitMaps =
        calloc (GET_INTNUM (allInfo.nModels), sizeof (int));
        if (clPtr->hostModelBitMaps == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            return (-1);
        }

        for (uint i = 0; i < GET_INTNUM (allInfo.nModels); i++)
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

    for ( uint i = 0; i < GET_INTNUM (allInfo.nRes); i++) {
        for ( uint j = 0; j < INTEGER_BITS; j++) {
            if (clPtr->resBitMaps[i] & (1 << j)) {
                clPtr->nRes++;
            }
        }
    }

    return 0;
}

static void
addMapBits (int num, int *toBitMaps, int *fromMaps)
{
    int j;
    
    for (j = 0; j < GET_INTNUM (num); j++)
        {
        toBitMaps[j] = (toBitMaps[j] | fromMaps[j]);
        }
}

int
reCheckClass (void)
{
    if (reCheckClusterClass (myClusterPtr) < 0)
        return -1;
    
    return 0;
}

static int
configCheckSum (char *file, u_short * checkSum)
{
    unsigned int sum;
    int i, linesum;
    FILE *fp;
    char *line;
    
    if ((fp = confOpen (file, "r")) == NULL)
        {
        ls_syslog (LOG_ERR, "%s: open() failed %m", __func__);
        return -1;
        }
    
    sum = 0;
    while ((line = getNextLine_ (fp, TRUE)) != NULL)
        {
        i = 0;
        linesum = 0;
        
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
            if (sum & 01)
                sum = (sum >> 1) + 0x8000;
            else
                sum >>= 1;
            sum += linesum & 0xFF;
            sum &= 0xFFFF;
            linesum = linesum >> 8;
            }
        }
    FCLOSEUP (&fp);
    *checkSum = (u_short) sum;
    
    return 0;
}

static struct admins *
getAdmins (char *line, char *lsfile, uint *lineNum, char *secName)
{
    static char fname[] = "getAdmins()";
    static struct admins admins;
    static int first = TRUE;
    uint numAds = 0;
    char *sp   = NULL;
    char *word = NULL;
    char *forWhat = "for LSF administrator";
    struct passwd *pw = NULL;
    struct group *unixGrp = NULL;

    assert( *lsfile );
    assert( *lineNum );
    assert( *secName );

    if (first == FALSE) {
        for (uint i = 0; i < admins.nAdmins; i++) {
            FREEUP (admins.adminNames[i]);
        }
        FREEUP (admins.adminNames);
        FREEUP (admins.adminIds);
        FREEUP (admins.adminGIds);
    }
    first = FALSE;
    admins.nAdmins = 0;
    sp = line;

    while ((word = getNextWord_ (&sp)) != NULL) {
        numAds++;
    }
    if (numAds)
        {
        admins.adminIds = (uid_t *) malloc (numAds * sizeof (int));
        admins.adminGIds = (gid_t *) malloc (numAds * sizeof (int));
        admins.adminNames = (char **) malloc (numAds * sizeof (char *));
        if ( ( NULL == admins.adminIds && ENOMEM == errno) || (NULL == admins.adminGIds && ENOMEM == errno ) || (NULL == admins.adminNames && ENOMEM == errno ) ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            FREEUP (admins.adminIds);
            FREEUP (admins.adminGIds);
            FREEUP (admins.adminNames);
            admins.nAdmins = 0;
            lserrno = LSE_MALLOC;
            return (&admins);
        }
    }
    else {
        return (&admins);
    }

    sp = line;
    while ((word = getNextWord_ (&sp)) != NULL) {

        if ((pw = getpwlsfuser_ (word)) != NULL)
            {
            if (putInLists (word, &admins, &numAds, forWhat) < 0) {
                return (&admins);
            }
        }
        else if ((unixGrp = getgrnam (word)) != NULL) {
            uint i = 0;
            while (unixGrp->gr_mem[i] != NULL) {
                if (putInLists (unixGrp->gr_mem[i++], &admins, &numAds, forWhat) < 0) {
                    return (&admins);
                }
            }
        }
        else {
            if (putInLists (word, &admins, &numAds, forWhat) < 0) {
                return (&admins);
            }
        }
    }
    return (&admins);
}


static int
doubleResTable (char *lsfile, uint lineNum)
{
    static char fname[] = "doubleResTable()";
    struct resItem *tempTable = NULL;

    assert( *lsfile );
    assert( lineNum );
    
    if (sizeOfResTable <= 0) {
        return (-1);
    }
    
    tempTable = (struct resItem *) realloc (allInfo.resTable, 2 * sizeOfResTable * sizeof (struct resItem));
    if (tempTable == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL, fname, "realloc", 2 * sizeOfResTable * sizeof (struct resItem));
        return (-1);
    }
    allInfo.resTable = tempTable;
    sizeOfResTable *= 2;
    return (0);
}

#define DEFAULT_RETRY_MAX    0
#define DEFAULT_RETRY_INT    30
FILE *
confOpen (char *filename, char *type)
{
    FILE *fp;
    int max, interval;
    
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
    
    for (;;) {

        fp = fopen (filename, type);
        if (fp != NULL) {
            break;
        }
        if (errno == ENOENT && max > 0) {
            float sleeptime;
            sleeptime = interval * mykey () * 1000;
            ls_syslog (LOG_ERR, "%s: %m still trying ...", filename);
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
    int sum = 0;
    char *myhostname = ls_getmyhostname ();
    uint i = 0;
    float key;
    
    for (i = 0; myhostname[i] != 0; i++) {
        sum += myhostname[i];
    }
    
    i = sum % 'z';
    if (i < 'A')
        i += 'A';
    
    key = (float) i / (float) 'Z';
    
    return key;
    
}

static void
setExtResourcesDefDefault (char *resName)
{
    
    static char fname[] = "setExtResourcesDefDefault()";
    
    
    allInfo.resTable[allInfo.nRes].valueType = LS_STRING;
    
    
    allInfo.resTable[allInfo.nRes].flags |= RESF_EXTERNAL;
    
    
    allInfo.resTable[allInfo.nRes].interval = 60;
    allInfo.resTable[allInfo.nRes].flags |= RESF_DYNAMIC;
    
    
    strncpy (allInfo.resTable[allInfo.nRes].des,
             "Fail to get external defined value, set to default",
             MAXRESDESLEN);
    
    ls_syslog (LOG_INFO, I18N (5370,
                               "%s: Fail to get external resource %s definition, set to default"),
               fname, resName);
    
    return;
    
}

static int
setExtResourcesDef (char *resName)
{
    
    static char fname[] = "setExtResourcesDef()";
    struct extResInfo *extResInfoPtr = NULL;
    int type = 0;
    
    if ((extResInfoPtr = getExtResourcesDef (resName)) == NULL) {
        setExtResourcesDefDefault (resName);
        return (0);
    }
    
    
    if ((type = validType (extResInfoPtr->type)) >= 0) {
        allInfo.resTable[allInfo.nRes].valueType =  (uint) type;
    }
    else {
        /* catgets 5371 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5371, "%s: type <%s> for external resource <%s> is not valid"), fname, extResInfoPtr->type, resName);
        setExtResourcesDefDefault (resName);
        return (0);
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
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5372, "%s: interval <%s> for external resource <%s> should be a integer greater than 0"), fname, extResInfoPtr->interval, resName);
            setExtResourcesDefDefault (resName);
            return (0);
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
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5373, "%s: increasing <%s> for resource <%s> is not valid"), fname, extResInfoPtr->increasing, resName);
                    setExtResourcesDefDefault (resName);
                    return (0);
                }
            }
        }
        else {
            /* catgets 5374 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5374, "%s: increasing <%s> is not used by the resource <%s> with type <%s>; ignoring INCREASING"), fname, extResInfoPtr->increasing, resName, extResInfoPtr->type);
        }
    }
    else {
        if (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC) {
            /* catgets 5375 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5375, "%s: No increasing specified for a numeric resource <%s>"), fname, resName);
            setExtResourcesDefDefault (resName);
            return (0);
        }
    }

    strncpy (allInfo.resTable[allInfo.nRes].des, extResInfoPtr->des, MAXRESDESLEN);

    if (allInfo.resTable[allInfo.nRes].interval > 0 && (allInfo.resTable[allInfo.nRes].valueType == LS_NUMERIC)) {

        if (allInfo.numUsrIndx + NBUILTINDEX >= li_len - 1) {
            li_len *= 2;
            if (!(li = realloc (li, li_len * sizeof (struct liStruct))))  {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
                return (-1);
            }
        }
        if ((li[NBUILTINDEX + allInfo.numUsrIndx].name = putstr_ (allInfo.resTable[allInfo.nRes].name)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, fname, "malloc");
            return (-1);
        }
        
        li[NBUILTINDEX + allInfo.numUsrIndx].increasing =
        (allInfo.resTable[allInfo.nRes].orderType == INCR) ? 1 : 0;
        allInfo.numUsrIndx++;
        allInfo.numIndx++;
    }
    return (0);
}

static int
setExtResourcesLoc (char *resName, int resNo)
{
    
    static char fname[] = "setExtResourcesLoc()";
    char *extResLocPtr;
    static char defaultExtResLoc[] = "[default]";
    uint lineNum = 0;
    int isDefault = 0;

    extResLocPtr = getExtResourcesLoc (resName);
    
    if (extResLocPtr == NULL || extResLocPtr[0] == '\0') {
        /* catgets 5379 */
        ls_syslog (LOG_INFO, I18N (5379, "%s: Failed to get LOCATION specified for external resource <%s>; Set to default"), fname, resName);
        extResLocPtr = defaultExtResLoc;
    }

    allInfo.resTable[resNo].flags |= RESF_EXTERNAL;

    if (addResourceMap (resName, extResLocPtr, fname, lineNum, &isDefault) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, fname, "addResourceMap", resName);
        lim_CheckError = WARNING_ERR;
        return (-1);
    }

    if (!(isDefault && (allInfo.resTable[resNo].flags & RESF_DYNAMIC) && (allInfo.resTable[resNo].valueType == LS_NUMERIC))) {
        allInfo.resTable[resNo].flags &= ~RESF_GLOBAL;
        allInfo.resTable[resNo].flags |= RESF_SHARED;
    }

    return (0);
}




struct extResInfo *
getExtResourcesDef (char *resName)
{
    char fname[] = "getExtResourcesDef";
    assert( *resName );
    /* catgets 5453 */
    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5453, "%s: external resource object is current not support in this platform"), fname);
    return (NULL);
}


char *
getExtResourcesLoc (char *resName)
{
    char fname[] = "getExtResourcesLoc";
    assert( *resName );
    /* catgets 5454 */
    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5454, "%s: external resource object is current not support in this platform"), fname);
    return (NULL);
}


char *
getExtResourcesValDefault (char *resName)
{
    static char defaultVal[] = "-";
    assert( *resName );
    return (defaultVal);
}


char *
getExtResourcesVal (char *resName)
{
    
    char fname[] = "getExtResourcesVal";
    assert( *resName );
    /* catgets 5455 */
    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5455, "%s: external resource object is current not support in this platform"), fname);
    return (getExtResourcesValDefault (resName));
}

int
initTypeModel (struct hostNode *me)
{
    static char fname[] = "initTypeModel";
    
    if (me->hTypeNo == DETECTMODELTYPE)
        {
        me->hTypeNo = typeNameToNo (getHostType ());
        if (me->hTypeNo < 0)
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5456, "%s: Unknown host type <%s>, using <DEFAULT>"),  /* catgets 5456 */
                       fname, getHostType ());
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
        if (me->hModelNo < 0)
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5458,
                                               "%s: Unknown host architecture <%s>, using <DEFAULT>"),
                       /* catgets 5458 */ fname, arch);
            me->hModelNo = 1;
            }
        else
            {
            if (strcmp (allInfo.hostArchs[me->hModelNo], arch) != 0)
                {
                if (logclass & LC_EXEC)
                    {
                    ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5457, "%s: Unknown host architecture <%s>, using best match <%s>, model <%s>"),    /* catgets 5457 */
                               fname, arch, allInfo.hostArchs[me->hModelNo],
                               allInfo.hostModels[me->hModelNo]);
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
stripIllegalChars (char *str)
{
    char *c = str;
    char *p = str;
    
    while (*c)
        {
        if (isalnum ((int) *c))
            *p++ = *c++;
        else
            c++;
        }
    *p = '\0';
    
    return str;
}

static int
saveHostIPAddr (struct hostNode *hPtr, struct hostent *hp)
{
    int i;
    
    for (hPtr->naddr = 0;
         hp->h_addr_list && hp->h_addr_list[hPtr->naddr] != NULL;
         hPtr->naddr++);
    
    hPtr->addr = NULL;
    if (hPtr->naddr)
        {
        hPtr->addr = calloc (hPtr->naddr, sizeof (in_addr_t));
        if (!hPtr->addr)
            {
            ls_syslog (LOG_ERR, "\
                       %s: calloc() %dbytes failed %m", __func__, hPtr->naddr * sizeof (in_addr_t));
            freeHostNodes (hPtr, FALSE);
            return -1;
            }
        }
    
    for (i = 0; i < hPtr->naddr; i++)
        memcpy (&hPtr->addr[i], hp->h_addr_list[i], hp->h_length);
    
    return 0;
}

/* addMigrantHost()
 */
void
addMigrantHost (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint chan)
{
    static char buf[MSGSIZE];
    struct LSFHeader hdr;
    struct hostEntry hPtr;
    struct hostNode *node = 0;
    uint16_t opCode = 0;
    XDR xdrs2;
    uint cc = 0;
    
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
hosed:
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
addHostByTab (hTab * tab)
{
    struct hostEntry hPtr;
    struct hostEntryLog *hLog;
    struct hostNode *node;
    sTab stab;
    hEnt *e;
    
    for (e = h_firstEnt_ (tab, &stab); e != NULL; e = h_nextEnt_ (&stab)) {
        uint cc = 0;
        
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
rmMigrantHost (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint chan)
{
    static char buf[MSGSIZE];
    struct LSFHeader hdr;
    char hostName[MAXHOSTNAMELEN];
    struct hostNode *hPtr = NULL;
    struct hostEntry hEnt;
    char *p = NULL;
    XDR xdrs2;
    uint16_t opCode;
    
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
}
