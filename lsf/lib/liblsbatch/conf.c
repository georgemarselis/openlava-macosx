/* $Id: lsb.conf.c 397 2007-11-26 19:04:00Z mblack $
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

#include <stdlib.h>
#include <math.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#include <ctype.h>
#include <strings.h>

#include "lsb/lsb.h"
#include "lsb/sig.h"
#include "libint/intlibout.h"
#include "lsb/conf.h"

#define  NL_SETN     13

static void
freeSA (char **list, unsigned int num)
{
    if (list == NULL || num <= 0) {
        return;
    }

    for( unsigned int i = 0; i < num; i++) {
        FREEUP (list[i]);
    }
    FREEUP (list);
}

static unsigned int
fillCell_ (struct inNames **table, char *name, char *level)
{
    unsigned int size = 0;

    table[0] = malloc (sizeof (struct inNames));    // FIXME FIXME replace 0 with enum label
    if ( NULL == table[0] && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        lserrno = LSE_MALLOC;
        return 0;
    }

    assert( strlen( name + 1 ) <= UINT_MAX );
    size = strlen (name) + 1;
    if (level) {
        size += strlen (level) + 1;
    }
    table[0]->name = malloc( size );
    if ( NULL == table[0]->name && ENOMEM == errno ) {
        FREEUP (table[0]);
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        lserrno = LSE_MALLOC;
        return 0;
    }

    strcpy(table[0]->name, name);
    if (level) {
        strcat (table[0]->name, "+");
        strcat (table[0]->name, level);
    }

    if (level) {
        table[0]->prf_level = strchr (table[0]->name, '+') + 1;
        *(table[0]->prf_level - 1) = 0;
    }
    else {
        table[0]->prf_level = NULL;
    }

    return size;
}


static void
initHostInfoEnt (struct hostInfoEnt *hp)
{
    if (hp != NULL) {
        hp->host        = NULL;
        hp->loadSched   = NULL;
        hp->loadStop    = NULL;
        hp->windows     = NULL;
        hp->load        = NULL;
        hp->hStatus     = INFINIT_INT;
        hp->busySched   = NULL;
        hp->busyStop    = NULL;
        hp->cpuFactor   = INFINIT_FLOAT;
        hp->nIdx        = 0;
        hp->userJobLimit = INFINIT_INT;
        hp->maxJobs     = INFINIT_INT;
        hp->numJobs     = INFINIT_INT;
        hp->numRUN      = INFINIT_INT;
        hp->numSSUSP    = INFINIT_INT;
        hp->numUSUSP    = INFINIT_INT;
        hp->mig         = INFINIT_INT;
        hp->attr        = INFINIT_INT;
        hp->chkSig      = INFINIT_INT;
    }
}

static void
freeHostInfoEnt (struct hostInfoEnt *hp)
{
    if (hp != NULL) {
        FREEUP (hp->host);
        FREEUP (hp->loadSched);
        FREEUP (hp->loadStop);
        FREEUP (hp->windows);
        FREEUP (hp->load);
    }
}


static unsigned int
expandWordAll (unsigned int *size, unsigned int *num, struct inNames **inTable, char *ptr_level)
{
    unsigned int cur_size = 0;

    if (numofhosts) {

        for ( unsigned int j = 0; j < numofhosts; j++) {
            cur_size = fillCell_ (&inTable[*num], hosts[j]->host, ptr_level);
            if (!cur_size) {
                return FALSE;
            }
            *size += cur_size;
            (*num)++;
        }
    }
    else {
        for ( unsigned int j = 0; j < cConf->numHosts; j++)  {
            cur_size = fillCell_( &inTable[*num], cConf->hosts[j].hostName, ptr_level);
            if (!cur_size)  {
                return FALSE;
            }
            *size += cur_size;
            (*num)++;
        }
    }
    
    return TRUE;
}


static int
readHvalues_conf (struct keymap *keyList, char *linep, struct lsConf *conf, char *lsfile, size_t *lineNumber, int exact, char *section)
{
    char *key   = NULL;
    char *value = NULL;
    char *sp    = NULL;
    char *sp1   = NULL;
    char error = FALSE;

    if (linep == NULL || conf == NULL) {
        return -1;
    }

    sp = linep;
    key = getNextWord_ (&linep);
    if ((sp1 = strchr (key, '=')) != NULL) {
        *sp1 = '\0';
    }

    value = strchr (sp, '=');
    if (!value) {
        /* catgets 5414 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5414, "%s: %s(%d): missing '=' after keyword %s, section %s ignoring the line"), __func__, lsfile, *lineNumber, key, section);
        lsberrno = LSBE_CONF_WARNING;
    }
    else {
        value++;
        while (*value == ' ') {
            value++;
        }

        if (value[0] == '\0') {
            /* catgets 5415 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5415, "%s: %s(%d): null value after keyword %s, section %s ignoring the line"), __func__, lsfile, *lineNumber, key, section);
            lsberrno = LSBE_CONF_WARNING;
        }
        else {
            if (value[0] == '(') {
                value++;
                if ((sp1 = strrchr (value, ')')) != NULL) {
                    *sp1 = '\0';
                }
            }
            if (putValue (keyList, key, value) < 0) {
                /* catgets 5416 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5416, "%s: %s(%d): bad keyword %s in section %s, ignoring the line"), __func__, lsfile, *lineNumber, key, section);  
                lsberrno = LSBE_CONF_WARNING;
                }
            }
        }

    if ((linep = getNextLineC_conf (conf, lineNumber, TRUE)) != NULL) {
        if (isSectionEnd (linep, lsfile, lineNumber, section)) {

            unsigned int i = 0;
            if (!exact) {
                return 0;
            }

            while (keyList[i].key != NULL) {
                if (keyList[i].val == NULL) {
                    /* catgets 5417 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5417, "%s: %s(%d): required keyword %s is missing in section %s, ignoring the section"), __func__, lsfile, *lineNumber, keyList[i].key, section);
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

        return readHvalues_conf (keyList, linep, conf, lsfile, lineNumber, exact, section);
    }

    ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, lsfile, *lineNumber, section);
    return -1;
}


struct paramConf *
lsb_readparam (struct lsConf *conf)
{
    // 
    char *filename = NULL;
    char *cp       = NULL;
    char *section  = NULL;
    char paramok   = 'a';
    size_t lineNum = 0;

    lsberrno = LSBE_NO_ERROR;

    if (conf == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "conf");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (conf->confhandle == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "confhandle");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (pConf) {
        freeParameterInfo (pConf->param);
        FREEUP (pConf->param);
    }
    else {
        pConf = malloc (sizeof (struct paramConf));
        if ( NULL == pConf && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct paramConf));
            lsberrno = LSBE_CONF_FATAL;
            return NULL;
        }
        pConf->param = NULL;
    }

    filename = conf->confhandle->fname;
    pConf->param = malloc(sizeof (struct parameterInfo));
    if ( NULL == pConf->param && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct parameterInfo));
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }
    initParameterInfo (pConf->param);


    conf->confhandle->curNode = conf->confhandle->rootNode;
    conf->confhandle->lineCount = 0;
    paramok = FALSE;

    for (;;) {
        if ((cp = getBeginLine_conf (conf, &lineNum)) == NULL) {
            if (paramok == FALSE) {
                /* catgets 5056 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5056, "%s: File %s at line %d: No valid parameters are read"), __func__, filename, lineNum);
                lsberrno = LSBE_CONF_WARNING;
            }
            return pConf;
        }

        section = getNextWord_ (&cp);
        if (!section) {
            /* catgets 5057 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5057, "%s: File %s at line %d: Section name expected after Begin; ignoring section"), __func__, filename, lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, "unknown");
            continue;
        }
        else {
            if (strcasecmp (section, "parameters") == 0)  {
                if (do_Param (conf, filename, &lineNum)) {
                    paramok = TRUE;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    return NULL;
                }

                continue;
            }
            /* catgets 5058 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5058, "%s: File %s at line %d: Invalid section name <%s>; ignoring section"), __func__, filename, lineNum, section);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, section);

            continue;
        }
    }
}

static char
do_Param (struct lsConf *conf, char *filename, size_t *lineNum)
{
    // 
    char *linep;
    int value = 0;

    enum state {
        LSB_MANAGER, DEFAULT_QUEUE, DEFAULT_HOST_SPEC, DEFAULT_PROJECT, JOB_ACCEPT_INTERVAL,
        PG_SUSP_IT, MBD_SLEEP_TIME, CLEAN_PERIOD, MAX_RETRY, SBD_SLEEP_TIME, MAX_JOB_NUM,
        RETRY_INTERVAL, MAX_SBD_FAIL, RUSAGE_UPDATE_RATE, RUSAGE_UPDATE_PERCENT, COND_CHECK_TIME,
        MAX_SBD_CONNS, MAX_SCHED_STAY, FRESH_PERIOD, MAX_JOB_ARRAY_SIZE, DISABLE_UACCT_MAP,
        JOB_TERMINATE_INTERVAL, JOB_RUN_TIMES, JOB_DEP_LAST_SUB, JOB_SPOOL_DIR, MAX_USER_PRIORITY,
        JOB_PRIORITY_OVER_TIME, SHARED_RESOURCE_UPDATE_FACTOR, SCHE_RAW_LOAD, PRE_EXEC_DELAY,
        SLOT_RESOURCE_RESERVE, MAX_JOBID, MAX_ACCT_ARCHIVE_FILE, ACCT_ARCHIVE_SIZE, ACCT_ARCHIVE_AGE 
    };

    struct keymap keylist[] = {
        { LSB_MANAGER,           "    ", NULL, 0 },
        { DEFAULT_QUEUE,         "    ", NULL, 0 },
        { DEFAULT_HOST_SPEC,     "    ", NULL, 0 },
        { DEFAULT_PROJECT,       "    ", NULL, 0 },
        { JOB_ACCEPT_INTERVAL,   "    ", NULL, 0 },
        { PG_SUSP_IT,            "    ", NULL, 0 },
        { MBD_SLEEP_TIME,        "    ", NULL, 0 },
        { CLEAN_PERIOD,          "    ", NULL, 0 },
        { MAX_RETRY,             "    ", NULL, 0 },
        { SBD_SLEEP_TIME,        "    ", NULL, 0 },
        { MAX_JOB_NUM,           "    ", NULL, 0 },
        { RETRY_INTERVAL,        "    ", NULL, 0 },
        { MAX_SBD_FAIL,          "    ", NULL, 0 },
        { RUSAGE_UPDATE_RATE,    "    ", NULL, 0 },     //* control how often sbatchd
        { RUSAGE_UPDATE_PERCENT, "    ", NULL, 0 },     //* report job rusage to mbd
        { COND_CHECK_TIME,       "    ", NULL, 0 },     //* time to check conditions
        { MAX_SBD_CONNS,         "    ", NULL, 0 },     //* Undocumented parameter for
                                                        //* specifying how many sbd
                                                        //* connections to keep around
        { MAX_SCHED_STAY,         "    ", NULL, 0 },
        { FRESH_PERIOD,           "    ", NULL, 0 },
        { MAX_JOB_ARRAY_SIZE,     "    ", NULL, 0 },
        { DISABLE_UACCT_MAP,      "    ", NULL, 0 },
        { JOB_TERMINATE_INTERVAL, "    ", NULL, 0 },
        { JOB_RUN_TIMES,          "    ", NULL, 0 },
        { JOB_DEP_LAST_SUB,       "    ", NULL, 0 },
        { JOB_SPOOL_DIR,          "    ", NULL, 0 },
        { MAX_USER_PRIORITY,      "    ", NULL, 0 },
        { JOB_PRIORITY_OVER_TIME, "    ", NULL, 0 },
        { SHARED_RESOURCE_UPDATE_FACTOR, "    ", NULL, 0 },
        { SCHE_RAW_LOAD,          "    ", NULL, 0 },
        { PRE_EXEC_DELAY,         "    ", NULL, 0 },
        { SLOT_RESOURCE_RESERVE,  "    ", NULL, 0 },
        { MAX_JOBID,              "    ", NULL, 0 },
        { MAX_ACCT_ARCHIVE_FILE,  "    ", NULL, 0 },
        { ACCT_ARCHIVE_SIZE,      "    ", NULL, 0 },
        { ACCT_ARCHIVE_AGE,       "    ", NULL, 0 },
        { 0,                      "    ", NULL, 0 }

    };

    if (conf == NULL) {
        return FALSE;
    }

    linep = getNextLineC_conf (conf, lineNum, TRUE);
    if (logclass & LC_EXEC)  {
        ls_syslog (LOG_DEBUG, "%s: file %s: the linep is %s, and %d \n", __func__, filename, linep, strlen (linep));
    }

    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, "parameters")) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum,  "parameters");
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (strchr (linep, '=') == NULL) {
        /* catgets 5059 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5059, "%s: File %s at line %d: Vertical parameters section is not implemented yet; use horizontal format; ignoring section"), __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, "parameters");
        return FALSE;
    }

    if (readHvalues_conf (keylist, linep, conf, filename, lineNum, FALSE,  "parameters") < 0) {
        /* catgets 5060 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5060, "%s: File %s at line %d: Incorrect section; ignored"), __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        freekeyval (keylist);
        return FALSE;
    }

    for ( unsigned int i = 0; keylist[i].key != NULL; i++) {

       if (keylist[i].val != NULL && strcmp (keylist[i].val, "")) {
/// FIXME FIXME FIXME discover what the values mean and replace them with constant unsigned integers
            if (i == 0) {
                /* catgets 5061 */
                ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5061, "%s: Ignore LSB_MANAGER value <%s>; use MANAGERS  defined in cluster file instead"), filename, keylist[i].val); 
                lsberrno = LSBE_CONF_WARNING;
            }
            else if (i == 1) {
                pConf->param->defaultQueues = putstr_ (keylist[i].val);
                if (pConf->param->defaultQueues == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__,  "malloc", strlen (keylist[i].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    return FALSE;
                }
            }
            else if (i == 2) {
                pConf->param->defaultHostSpec = putstr_ (keylist[i].val);
                if (pConf->param->defaultHostSpec == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[i].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    return FALSE;
                }
            }
            else if (i == 24)
                {

                if (checkSpoolDir (keylist[i].val) == 0)
                    {
                    pConf->param->pjobSpoolDir = putstr_ (keylist[i].val);
                    if (pConf->param->pjobSpoolDir == NULL)
                        {
                        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__,"malloc", strlen (keylist[i].val) + 1);
                        lsberrno = LSBE_NO_MEM;
                        freekeyval (keylist);
                        return FALSE;
                        }
                    }
                else
                    {
                    pConf->param->pjobSpoolDir = NULL;
                    /* catgets 5095 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5095, "%s: Invalid JOB_SPOOL_DIR!"), __func__);
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            else if (i == 32)
                {

                value = my_atoi (keylist[i].val, INFINIT_INT, 0);
                if (value == INFINIT_INT)
                    {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, I18N (5459, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key, INFINIT_INT - 1);
                    pConf->param->maxAcctArchiveNum = -1;
                    lsberrno = LSBE_CONF_WARNING;
                    }
                else
                    {
                    pConf->param->maxAcctArchiveNum = value;
                    }
                }
            else if (i == 33)
                {

                value = my_atoi (keylist[i].val, INFINIT_INT, 0);
                if (value == INFINIT_INT)
                    {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, I18N (5459, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key, INFINIT_INT - 1);
                    pConf->param->acctArchiveInSize = -1;
                    lsberrno = LSBE_CONF_WARNING;
                    }
                else
                    {
                    pConf->param->acctArchiveInSize = value;
                    }
                }
            else if (i == 34)
                {

                value = my_atoi (keylist[i].val, INFINIT_INT, 0);
                if (value == INFINIT_INT)
                    {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, I18N (5459, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key, INFINIT_INT - 1);
                    pConf->param->acctArchiveInDays = -1;
                    lsberrno = LSBE_CONF_WARNING;
                    }
                else
                    {
                    pConf->param->acctArchiveInDays = value;
                    }
                }

            else if (i == 3)
                {
                pConf->param->defaultProject = putstr_ (keylist[i].val);
                if (pConf->param->defaultProject == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[i].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    return FALSE;
                    }
                }
            else if (i == 4 || i == 5)
                {
                if ((value = my_atoi (keylist[i].val, INFINIT_INT, -1)) == INFINIT_INT)
                    {
                    /* catgets 5067 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5067, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a non-negative integer between 0 and %d; ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key, INFINIT_INT - 1);
                    lsberrno = LSBE_CONF_WARNING;
                    }
                else if (i == 4) {
                    pConf->param->jobAcceptInterval = value;
                }
                else {
                    pConf->param->pgSuspendIt = value;
                }
            }
            else if (i == 20)
                {
                if (strcasecmp (keylist[i].val, "Y") == 0)
                    {
                    pConf->param->disableUAcctMap = TRUE;
                    }
                else if (strcasecmp (keylist[i].val, "N") == 0)
                    {
                    pConf->param->disableUAcctMap = FALSE;
                    }
                else
                    {
                    /* catgets 5068 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5068, "%s: File %s in section Parameters ending at line %d: unrecognizable value <%s> for the keyword DISABLE_UACCT_MAP; assume user level account mapping is allowed"), __func__, filename, *lineNum, keylist[i].val);
                    pConf->param->disableUAcctMap = FALSE;
                    }
                }
            else if (i == 26) { // FIXME FIXME 26 is awfully particular, maybe we should translate this to enum labels?

                int value_ = 0;
                int mytime = 0;
                char str[100]; // FIXME FIXME FIXME 100 is awfully particular 
                char *ptr = NULL;

                strcpy (str, keylist[i].val);
                ptr = strchr (str, '/');
                if (ptr != NULL)  {
                    *ptr = 0x0;
                    ptr++;
                    value_ = my_atoi (str, INFINIT_INT, 0);
                    mytime = my_atoi (ptr, INFINIT_INT, 0);
                }

                if (ptr == NULL || value_ == INFINIT_INT || mytime == INFINIT_INT) {
                    /* catgets 5451 */
                    ls_syslog (LOG_ERR, I18N (5451, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't in format value/time (time in minutes) or value is not positive; ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key);
                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }

                pConf->param->jobPriorityValue = value_;
                pConf->param->jobPriorityTime = mytime;

            }
            else if (i == 27) {
                value = my_atoi (keylist[i].val, INFINIT_INT, 0);
                if (value == INFINIT_INT) {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, I18N (5459, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key, INFINIT_INT - 1); 
                    lsberrno = LSBE_CONF_WARNING;
                    pConf->param->sharedResourceUpdFactor = INFINIT_INT;
                }
                else {
                    pConf->param->sharedResourceUpdFactor = value;
                }
            }
            else if (i == 31) {
                int foo = 0;
                unsigned int value_ = 0;

                foo = my_atoi (keylist[i].val, INFINIT_INT, 0);
                assert( foo >= 0 );
                value_ = (unsigned int) foo;
                if ((value_ < MAX_JOBID_LOW) || (value_ > MAX_JOBID_HIGH)) {
                    /*catgets 5062 */
                    ls_syslog (LOG_ERR, I18N (5062, "%s: File %s in section Parameters ending at line %d: maxJobId value %s not in [%d, %d], use default value %d;"), __func__, filename, *lineNum, keylist[i].key, MAX_JOBID_LOW, MAX_JOBID_HIGH, DEF_MAX_JOBID);
                    lsberrno = LSBE_CONF_WARNING;
                    pConf->param->maxJobId = DEF_MAX_JOBID;
                }
                else {
                    pConf->param->maxJobId = value_;
                }
            }
            else if (i == 28)  {
                if (strcasecmp (keylist[i].val, "Y") == 0) {
                    pConf->param->scheRawLoad = TRUE;
                }
                else {
                    pConf->param->scheRawLoad = FALSE;
                }
            }
            else if (i == 30) {
                if (strcasecmp (keylist[i].val, "Y") == 0)  {
                    pConf->param->slotResourceReserve = TRUE;
                }
                else {
                    pConf->param->slotResourceReserve = FALSE;
                }
            }
            else if (i > 5) {
                if (i < 23) {
                    value = my_atoi (keylist[i].val, INFINIT_INT, 0);
                }
                else {
                    value = atoi (keylist[i].val);
                }

                if (value == INFINIT_INT) {
                    /* catgets 5071 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5071, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key, INFINIT_INT - 1);
                    lsberrno = LSBE_CONF_WARNING;
                }
                else {
                    switch (i) {
                        case 6:
                            pConf->param->mbatchdInterval = value;
                        break;
                        case 7:
                            pConf->param->cleanPeriod = value;
                        break;
                        case 8:
                            pConf->param->maxDispRetries = value;
                        break;
                        case 9:
                            pConf->param->sbatchdInterval = value;
                        break;
                        case 10:
                            pConf->param->maxNumJobs = value;
                        break;
                        case 11:
                            pConf->param->retryIntvl = value;
                        break;
                        case 12:
                            pConf->param->maxSbdRetries = value;
                        break;
                        case 13:
                            pConf->param->rusageUpdateRate = value;
                        break;
                        case 14:
                            pConf->param->rusageUpdatePercent = value;
                        break;
                        case 15:
                            pConf->param->condCheckTime = value;
                        break;
                        case 16:
                            pConf->param->maxSbdConnections = value;
                        break;
                        case 17:
                            pConf->param->maxSchedStay = value;
                        break;
                        case 18:
                            pConf->param->freshPeriod = value;
                        break;
                        case 19:
                            if (value < 1 || value >= LSB_MAX_ARRAY_IDX) {
                                /* catgets 5073 */
                                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5073, "%s: File %s in section Parameters ending at line %d: Value <%s> of %s is out of range[1-65534]); ignored"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key);
                                lsberrno = LSBE_CONF_WARNING;
                            }
                            else { 
                                pConf->param->maxJobArraySize = value;
                            }
                        break;
                        case 21:
                            pConf->param->jobTerminateInterval = value;
                        break;
                        case 22:
                            assert( value >= 0);
                            pConf->param->jobRunTimes = (unsigned int )value;
                        break;
                        case 23:
                            assert( value >= 0);
                            pConf->param->jobDepLastSub = (unsigned int) value;
                        break;
                        case 25:
                            value = my_atoi (keylist[i].val, INFINIT_INT, -1);
                            if (value != INFINIT_INT) {
                                if (value > 0) {
                                    pConf->param->maxUserPriority = value;
                                }
                            }
                            else {
                                /* catgets 5452 */
                                ls_syslog (LOG_ERR, I18N (5452, "%s: File %s in section Parameters ending at line %d: invalid value <%s> of <%s> : ignored;"), __func__, filename, *lineNum, keylist[i].val, keylist[i].key);
                            }
                        break;
                        case 29:
                            pConf->param->preExecDelay = value;
                        break;
                        default:
                            /* catgets 5074 */
                            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5074, "%s: File %s in section Parameters ending at line %d: Impossible cases <%d>."), __func__, filename, *lineNum, i);
                            lsberrno = LSBE_CONF_WARNING;
                        break;
                    }
                }
            }
        }
    }


    if (pConf->param->maxUserPriority <= 0
        && pConf->param->jobPriorityValue > 0
        && pConf->param->jobPriorityTime > 0)
        {
        /* catgets 5453 */
        ls_syslog (LOG_ERR, I18N (5453, "%s: File %s in section Parameters : MAX_USER_PRIORITY should be defined first so that JOB_PRIORITY_OVER_TIME can be used: job priority control disabled"), __func__, filename);
        pConf->param->jobPriorityValue = -1;
        pConf->param->jobPriorityTime = -1;
        lsberrno = LSBE_CONF_WARNING;
        }
    freekeyval (keylist);
    return TRUE;
}

static int
my_atoi (char *arg, int upBound, int botBound)
{
    int num = 0;
    if (!isint_ (arg)) {
        return INFINIT_INT;
    }
    num = atoi (arg);
    if (num >= upBound || num <= botBound) {
        return INFINIT_INT;
    }

    return num;
}

static float
my_atof (char *arg, float upBound, float botBound)
{
    float num = 0;

    if (!isanumber_ (arg)) {
        return INFINIT_FLOAT;
    }
    num = (float) atof (arg); // FIXME probably intentional, but the if statement bellow may need some fixup
    if (num >= upBound || num <= botBound) {
        return INFINIT_FLOAT;
    }

    return num;
}

static void
initParameterInfo (struct parameterInfo *param)
{
    if (param != NULL) {
        param->defaultQueues = NULL;
        param->defaultHostSpec = NULL;
        param->mbatchdInterval = INFINIT_INT;
        param->sbatchdInterval = INFINIT_INT;
        param->jobAcceptInterval = INFINIT_INT;
        param->maxDispRetries = INFINIT_INT;
        param->maxSbdRetries = INFINIT_INT;
        param->cleanPeriod = INFINIT_INT;
        param->maxNumJobs = INFINIT_INT;
        param->pgSuspendIt = INFINIT_INT;
        param->defaultProject = NULL;

        param->retryIntvl = INFINIT_INT;
        param->rusageUpdateRate = INFINIT_INT;
        param->rusageUpdatePercent = INFINIT_INT;
        param->condCheckTime = INFINIT_INT;
        param->maxSbdConnections = INFINIT_INT;
        param->maxSchedStay = INFINIT_INT;
        param->freshPeriod = INFINIT_INT;
        param->maxJobArraySize = INFINIT_INT;
        param->jobTerminateInterval = INFINIT_INT;
        param->disableUAcctMap = FALSE;
        param->jobRunTimes = INFINIT_INT;
        param->jobDepLastSub = 0;
        param->pjobSpoolDir = NULL;
        param->maxUserPriority = -1;
        param->jobPriorityValue = -1;
        param->jobPriorityTime = -1;
        param->sharedResourceUpdFactor = INFINIT_INT;
        param->scheRawLoad = 0;
        param->preExecDelay = INFINIT_INT;
        param->slotResourceReserve = FALSE;
        param->maxJobId = INFINIT_INT;
        param->maxAcctArchiveNum = -1;
        param->acctArchiveInDays = -1;
        param->acctArchiveInSize = -1;
    }
}

static void
freeParameterInfo (struct parameterInfo *param)
{
    if (param != NULL) {
        FREEUP (param->defaultQueues);
        FREEUP (param->defaultHostSpec);
        FREEUP (param->defaultProject);
        FREEUP (param->pjobSpoolDir);
    }
}

int
checkSpoolDir (char *pSpoolDir)
{
    char *pTemp = NULL;
    char TempPath[MAXPATHLEN];
    char TempNT[MAXPATHLEN];
    char TempUnix[MAXPATHLEN];

    if (logclass & LC_EXEC) {
        ls_syslog (LOG_DEBUG, "%s: the JOB_SPOOL_DIR is %s, and %d \n", __func__, pSpoolDir, strlen (pSpoolDir));
    }
    if (strlen (pSpoolDir) >= MAXPATHLEN) {
        return -1;
        }

    if (strlen (pSpoolDir) < 1)
        {
        return 0;
        }

    if (strchr (pSpoolDir, '|') != strrchr (pSpoolDir, '|'))
        {
        return -1;
        }


    strcpy (TempPath, pSpoolDir);
    strcpy (TempUnix, pSpoolDir);
    if ((pTemp = strchr (TempUnix, '|')) != NULL)
        {

        *pTemp = '\0';
        if (logclass & LC_EXEC)
            {
            ls_syslog (LOG_DEBUG, "%s: the UNIX's JOB_SPOOL_DIR is %s \n", __func__, TempUnix);
            }

        if (strlen (TempUnix) > 1)
            {

            if (TempUnix[0] != '/')
                {
                return -1;
                }
            }

        pTemp = strchr (TempPath, '|');
        pTemp++;
        while (isspace (*pTemp)) {
            pTemp++;
        }

        strcpy (TempNT, pTemp);
        if (logclass & LC_EXEC)
            {
            ls_syslog (LOG_DEBUG, "%s: the NT's JOB_SPOOL_DIR is %s \n", __func__, TempNT);
            }

        if (strlen (TempNT) > 1)
            {

            int i;
            for (i = 0; TempNT[i] != '\0'; i++)
                {
                if (TempNT[i] == '\\' && TempNT[i + 1] == '\\')
                    {
                    ls_syslog (LOG_DEBUG, "%s: NT's JOB_SPOOL_DIR[%d] is ## \n", __func__, i);
                    }
                else if (TempNT[i] == '\\')
                    {
                    ls_syslog (LOG_DEBUG, "%s: NT's JOB_SPOOL_DIR[%d] is # \n", __func__, i);
                    }
                else
                    {
                    ls_syslog (LOG_DEBUG, "%s: NT's JOB_SPOOL_DIR[%d] is %c\n", __func__, i, TempNT[i]);
                    }
                }

            if (TempNT[0] != '\\' || TempNT[1] != '\\')
                {

                if (TempNT[1] != ':')
                    {
                    return -1;
                    }
                else
                    {

                    if (!isalpha (TempNT[0]))
                        {
                        return -1;
                        }
                    }
                }
            }
        }
    else
        {

        pTemp = pSpoolDir;

        while (isspace (*pTemp)) {
            pTemp++;
        }


        strcpy (pSpoolDir, pTemp);
        if (pSpoolDir[0] != '/')
            {

            if ((pSpoolDir[0] != '\\') || (pSpoolDir[1] != '\\'))
                {

                if (!(isalpha (pSpoolDir[0]) && (pSpoolDir[1] == ':')))
                    return -1;
                }
            }
        }
    return 0;
}

struct userConf *
lsb_readuser (struct lsConf *conf, int options, struct clusterConf *clusterConf)
{
    return lsb_readuser_ex (conf, options, clusterConf, NULL);

}

struct userConf *
lsb_readuser_ex (struct lsConf *conf, int options, struct clusterConf *clusterConf, struct sharedConf *sharedConf)
{
    char *filename  = NULL;
    char *cp        = NULL;
    char *section   = NULL;
    size_t lineNum  = 0;

    lsberrno = LSBE_NO_ERROR;

    if (conf == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "conf");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (sharedConf != NULL) {
        sConf = sharedConf;
    }

    if (conf && conf->confhandle == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "confhandle");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }
    if (handleUserMem ()) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "handleUserMem");
        return NULL;
    }

    filename = conf->confhandle->fname;


    conf->confhandle->curNode = conf->confhandle->rootNode;
    conf->confhandle->lineCount = 0;

    cConf = clusterConf;
    for (;;) {
        if ((cp = getBeginLine_conf (conf, &lineNum)) == NULL) {

            if (numofugroups) {
                uConf->ugroups = calloc (numofugroups, sizeof (struct groupInfoEnt));
                if( NULL == uConf->ugroups && ENOMEM == errno ) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", numofugroups * sizeof (struct groupInfoEnt));
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkUser (TRUE);
                    freeUConf (uConf, FALSE);
                    return NULL;
                }

                for ( unsigned int i = 0; i < numofugroups; i++) {
                    initGroupInfo (&uConf->ugroups[i]);
                    uConf->ugroups[i] = *usergroups[i];
                }
                uConf->numUgroups = numofugroups;
            }

            if (numofusers) {
                uConf->users = malloc(numofusers * sizeof (struct userInfoEnt));
                if ( NULL == uConf->users && ENOMEM == errno ) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", numofusers * sizeof (struct userInfoEnt));
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkUser (TRUE);
                    freeUConf (uConf, FALSE);
                    return NULL;
                }

                for( unsigned int i = 0; i < numofusers; i++) {
                    initUserInfo (&uConf->users[i]);
                    uConf->users[i] = *users[i];
                }
                uConf->numUsers = numofusers;
            }

            return uConf;
        }

        section = getNextWord_ (&cp);
        if (!section)  {
            /* catgets 5082 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5082, "%s: File %s at line %d: Section name expected after Begin; ignoring section"), __func__, filename, lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, "unknown");
            continue;
        }
        else {
            if (strcasecmp (section, "user") == 0) {
                if (do_Users (conf, filename, &lineNum, options) == FALSE) {
                    if (lsberrno == LSBE_NO_MEM) {
                        lsberrno = LSBE_CONF_FATAL;
                        freeWorkUser (TRUE);
                        freeUConf (uConf, FALSE);
                        return NULL;
                    }
                }
                continue;
            }
            else if (strcasecmp (section, "usergroup") == 0) {

                if (do_Groups (usergroups, conf, filename, &lineNum, &numofugroups, options) == FALSE) {
                    if (lsberrno == LSBE_NO_MEM) {
                        lsberrno = LSBE_CONF_FATAL;
                        freeWorkUser (TRUE);
                        freeUConf (uConf, FALSE);
                        return NULL;
                    }
                }
                continue;
            }
            /* catgets 5083 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5083, "%s: File %s at line %d: Invalid section name %s; ignoring section"), __func__, filename, lineNum, section);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, section);
            continue;
        }
    }
}

static char
do_Users (struct lsConf *conf, char *filename, size_t *lineNum, int options)
{
    // 
    char *linep = NULL;
    char *grpSl = NULL;
    size_t maxjobs = 0;
    int new = 0;
    int isGroupAt = FALSE;
    float pJobLimit = 0.0;
    struct passwd *pw                = NULL;
    struct hTab *tmpUsers            = NULL;
    struct hTab *nonOverridableUsers = NULL;
    
    enum state {
        USER_NAME, MAX_JOBS, JL_P
    };

    struct keymap keylist[] = {
        {USER_NAME, "    ", NULL, 0},
        {MAX_JOBS,  "    ", NULL, 0},
        {JL_P,      "    ", NULL, 0},
        {0,         "    ", NULL, 0}
    };

    if (conf == NULL) {
        return FALSE;
    }

    linep = getNextLineC_conf (conf, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, "user")) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum, "user");
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (strchr (linep, '=') == NULL) {
        if (!keyMatch (keylist, linep, FALSE)) {
            /* catgets 5086 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5086, "%s: File %s at line %d: Keyword line format error for User section; ignoring section"), __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, "user");
            return FALSE;
        }

        if (keylist[0].position < 0) {
            /* catgets 5087 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5087, "%s: File %s at line %d: User name required for User section; ignoring section"), __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, "user");
            return FALSE;
        }

        tmpUsers = malloc (sizeof (struct hTab));
        nonOverridableUsers = malloc (sizeof (struct hTab));
        if ( ( NULL == tmpUsers && ENOMEM == errno ) || ( NULL == nonOverridableUsers && ENOMEM == errno ) ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct hTab));
            lsberrno = LSBE_NO_MEM;
            return FALSE;
        }
        h_initTab_ (tmpUsers, 32);
        h_initTab_ (nonOverridableUsers, 16);

        while ((linep = getNextLineC_conf (conf, lineNum, TRUE)) != NULL) {
            size_t lastChar = 0;
            int hasAt = 0;
            struct groupInfoEnt *gp;
            struct group *unixGrp = NULL;

            isGroupAt = FALSE;
            freekeyval (keylist);
            if (isSectionEnd (linep, filename, lineNum, "user")) {
                h_delTab_ (tmpUsers);
                FREEUP (tmpUsers);
                h_delTab_ (nonOverridableUsers);
                FREEUP (nonOverridableUsers);
                return TRUE;
            }
            if (mapValues (keylist, linep) < 0) {
                /* catgets 5089 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5089, "%s: File %s at line %d: Values do not match keys in User section; ignoring line"), __func__, filename, *lineNum); 
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }
            h_addEnt_ (tmpUsers, keylist[0].val, &new);
            if (!new) {
                /* catgets 5090 */
                ls_syslog (LOG_ERR, I18N (5090, "%s: File %s at line %d: User name <%s> multiply specified; ignoring line"), __func__, filename, *lineNum, keylist[0].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }
            hasAt = FALSE;
            lastChar = strlen (keylist[0].val) - 1;
            if (lastChar > 0 && keylist[0].val[lastChar] == '@') {
                hasAt = TRUE;
                keylist[0].val[lastChar] = '\0';
            }

            lastChar = strlen (keylist[0].val) - 1;
            if (lastChar > 0 && keylist[0].val[lastChar] == '/') {
                grpSl = putstr_ (keylist[0].val);
                if (grpSl == NULL) {
                    lsberrno = LSBE_NO_MEM;
                    h_delTab_ (tmpUsers);
                    FREEUP (tmpUsers);
                    FREEUP (grpSl);
                    h_delTab_ (nonOverridableUsers);
                    FREEUP (nonOverridableUsers);
                    return FALSE;
                }

                grpSl[lastChar] = '\0';
            }
            gp = getUGrpData (keylist[0].val);
            pw = getpwlsfuser_ (keylist[0].val);

            if ((options != CONF_NO_CHECK) && !gp && (grpSl || (strcmp (keylist[0].val, "default") && !pw))) {
                if (grpSl) {
                    unixGrp = mygetgrnam (grpSl);
                    grpSl[lastChar] = '/';
                }
                else {
                    unixGrp = mygetgrnam (keylist[0].val);
                }

                if (unixGrp != NULL) {
                    if (options & (CONF_EXPAND | CONF_NO_EXPAND | CONF_CHECK)) {

                        gp = addUnixGrp (unixGrp, keylist[0].val, filename, *lineNum, " in User section", 0);

                        if (gp == NULL)
                            {
                            if (lsberrno == LSBE_NO_MEM) {
                                return FALSE;
                            }
                            /* catgets 5091 */
                            ls_syslog (LOG_WARNING, I18N (5091, "%s: File %s at line %d: No valid users defined in Unix group <%s>; ignored"), __func__, filename, *lineNum, keylist[0].val);
                            lsberrno = LSBE_CONF_WARNING;
                            continue;
                            }
                        }
                    }
                else
                    {
                    /* catgets 5092 */
                    ls_syslog (LOG_WARNING, I18N (5092, "%s: File %s at line %d: Unknown user <%s>; Maybe a windows user or of another domain."), __func__, filename, *lineNum, keylist[0].val);  
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            if (hasAt && gp) {
                isGroupAt = TRUE;
            }
            if (hasAt && (!(options & (CONF_EXPAND | CONF_NO_EXPAND)) ||  options == CONF_NO_CHECK)) {
                keylist[0].val[lastChar + 1] = '@';
            }

            maxjobs = INFINIT_INT;
            if (keylist[1].position >= 0 && keylist[1].val != NULL && strcmp (keylist[1].val, ""))
                {


                if ((maxjobs = my_atoi (keylist[1].val, INFINIT_INT, -1)) == INFINIT_INT)
                    {
                    /* catgets 5093 */
                    ls_syslog (LOG_ERR, I18N (5093, "%s: File %s at line %d: Invalid value <%s> for key <%s>; %d is assumed"), __func__, filename, *lineNum, keylist[1].val, keylist[1].key, INFINIT_INT);
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            pJobLimit = INFINIT_FLOAT;
            if (keylist[2].position >= 0 && keylist[2].val != NULL
                && strcmp (keylist[2].val, ""))
                {

                pJobLimit = my_atof( keylist[2].val, INFINIT_FLOAT, -1.0 );
                if ( fabs( INFINIT_FLOAT - pJobLimit) < 0.00001 ) {
                    /* catgets 5094 */
                    ls_syslog (LOG_ERR, I18N (5094, "%s: File %s at line %d: Invalid value <%s> for key %s; %f is assumed"), __func__, filename, *lineNum, keylist[2].val, keylist[2].key, INFINIT_FLOAT);
                    lsberrno = LSBE_CONF_WARNING;
                }
            }
            if (!isGroupAt && (!(options & (CONF_EXPAND | CONF_NO_EXPAND)) || options == CONF_NO_CHECK)) {
                h_addEnt_ (nonOverridableUsers, keylist[0].val, 0);
                if (!addUser (keylist[0].val, maxjobs, pJobLimit, filename, TRUE) && lsberrno == LSBE_NO_MEM) {
                    FREEUP (grpSl);
                    lsberrno = LSBE_NO_MEM;
                    h_delTab_ (tmpUsers);
                    FREEUP (tmpUsers);
                    FREEUP (grpSl);
                    h_delTab_ (nonOverridableUsers);
                    FREEUP (nonOverridableUsers);
                    return FALSE;

                }
            }
            else {
                char **groupMembers;
                unsigned int *numMembers = 0;
                char gr__func__[MAX_LSB_NAME_LEN];
                STRNCPY (gr__func__, keylist[0].val, MAX_LSB_NAME_LEN);


                if (isGroupAt)
                    {
                    if (gr__func__[strlen (keylist[0].val) - 1] == '@')
                        gr__func__[strlen (keylist[0].val) - 1] = '\0';
                    }

                if ((groupMembers = expandGrp (gr__func__, numMembers, USER_GRP)) == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, __func__);
                    goto Error;
                }

                if (strcmp (groupMembers[0], "all") == 0) {
                    ls_syslog (LOG_ERR, I18N (5096, "%s: File %s at line %d: user group <%s> with no members is ignored"), __func__, filename, *lineNum, keylist[0].val); /* catgets 5096 */
                }
                else if (!(options & CONF_NO_EXPAND)) {
                    for ( unsigned int i = 0; i < *numMembers; i++) {
                        if (h_getEnt_ (nonOverridableUsers, groupMembers[i])) {
                            continue;
                        }
                        addUser (groupMembers[i], maxjobs, pJobLimit, filename, TRUE);
                    }
                }
                else {
                    if (isGroupAt) {
                        gr__func__[lastChar + 1] = '@';
                    }

                    addUser (gr__func__, maxjobs, pJobLimit, filename, TRUE);
                }
                freeSA (groupMembers, *numMembers);
            }
            FREEUP (grpSl);
        }

        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        FREEUP (grpSl);
        h_delTab_ (tmpUsers);
        FREEUP (tmpUsers);
        FREEUP (grpSl);
        h_delTab_ (nonOverridableUsers);
        FREEUP (nonOverridableUsers);
        return FALSE;

    }
    else {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "user");
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, "user");
        return FALSE;
    }

Error:     // FIXME FIXME FIXME goto label has to go
    h_delTab_ (tmpUsers);
    FREEUP (tmpUsers);
    FREEUP (grpSl);
    h_delTab_ (nonOverridableUsers);
    FREEUP (nonOverridableUsers);
    return FALSE;

}



static char
do_Groups (struct groupInfoEnt **groups, struct lsConf *conf, char *filename, size_t *lineNum, unsigned int *ngroups, int options)
{
    // 

    enum state { GROUP_NAME, GROUP_MEMBER };

    struct keymap keylist[] = {
        {GROUP_NAME,   "    ", NULL, 0},
        {GROUP_MEMBER, "    ", NULL, 0},
        {0,            "    ", NULL, 0}
    };
    char *linep;
    char *wp;
    char *sp;
    char *HUgroups = NULL;
    int type = 0;
    int allFlag = FALSE;
    int nGrpOverFlow = 0;
    struct passwd *pw;
    struct groupInfoEnt *gp;

    if (groups == NULL || conf == NULL || ngroups == NULL) {
        return FALSE;
    }

    if (groups == usergroups) {
        type = USER_GRP;
        HUgroups = "usergroup";
    }
    else if (groups == hostgroups) {
        type = HOST_GRP;
        HUgroups = "hostgroup";
    }

    linep = getNextLineC_conf (conf, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, HUgroups)) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum, HUgroups);
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (strchr (linep, '=') == NULL) {

        if (!keyMatch (keylist, linep, FALSE)) {
            /* catgets 5101 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5101, "%s: File %s at line %d: Keyword line format error for section <%s>; ignoring section"), __func__, filename, *lineNum, HUgroups);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, HUgroups);
            return FALSE;
        }
        while ((linep = getNextLineC_conf (conf, lineNum, TRUE)) != NULL) {
            freekeyval (keylist);
            if (isSectionEnd (linep, filename, lineNum, HUgroups)) {
                return TRUE;
            }

            if (mapValues (keylist, linep) < 0) {
                /* catgets 5102 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5102, "%s: File %s at line %d: Values do not match keys in %s section; ignoring line"), __func__, filename, *lineNum, HUgroups);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (    strcmp (keylist[0].val, "all")     == 0 || 
                    strcmp (keylist[0].val, "default") == 0 || 
                    strcmp (keylist[0].val, "others")  == 0
                )
                {
                /* catgets 5103 */
                ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5103, "%s: File %s at line %d: Group name <%s> conflicts with reserved word <all/default/others>; ignoring line"), __func__, filename, *lineNum, keylist[0].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }


            if ((type == USER_GRP || type == HOST_GRP) && (keylist[0].val != NULL) && (strcmp (keylist[1].val, "!") == 0)) {
                char *members;

                if ((members = runEGroup_ (type == USER_GRP ? "-u" : "-m", keylist[0].val)) != NULL) {
                    FREEUP (keylist[1].val);
                    keylist[1].val = members;
                }
                else {
                    keylist[1].val[0] = '\0';
                }
            }

            if (options != CONF_NO_CHECK && type == USER_GRP) {

                pw = getpwlsfuser_ (keylist[0].val);
                if (!initUnknownUsers) {
                    initTab (&unknownUsers);
                    initUnknownUsers = TRUE;
                }

                if ((pw != NULL) || (chekMembStr (&unknownUsers, keylist[0].val) != NULL)) {
                    /* catgets 5104 */
                    ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5104, "%s: File %s at line %d: Group name <%s> conflicts with user login name; ignoring line"), __func__, filename, *lineNum, keylist[0].val);
                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }
            }

            if (options != CONF_NO_CHECK && type == HOST_GRP && isHostName (keylist[0].val)) {
                /* catgets 5105 */
                ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5105, "%s: File %s at line %d: Group name <%s> conflicts with host name; ignoring line"), __func__, filename, *lineNum, keylist[0].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (options != CONF_NO_CHECK && getGrpData (groups, keylist[0].val, *ngroups)) {
                /* catgets 5106 */
                ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5106, "%s: File %s at line %d: Group <%s> is multiply defined; ignoring line"), __func__, filename, *lineNum, keylist[0].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (*ngroups >= MAX_GROUPS) {

                if (nGrpOverFlow++ == 0) {
                    /* catgets 5107 */
                    ls_syslog (LOG_WARNING, I18N (5107, "%s: File %s at line %d: The number of configured groups reaches the limit <%d>; ignoring the rest of groups defined"), __func__, filename, *lineNum, MAX_GROUPS);
                }

                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if ((type == HOST_GRP) && *ngroups >= MAX_GROUPS) {
                /* catgets 5113 */
                ls_syslog (LOG_ERR, I18N (5113, "%s: File %s at line %d: The number of configured host groups reaches the limit <%d>; ignoring the rest of groups defined"), __func__, filename, *lineNum, MAX_GROUPS);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }
            else if ((type == USER_GRP) && (*ngroups >= MAX_GROUPS)) {
                /* catgets 5107 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5107, "%s: File %s at line %d: The number of configured user groups reaches the limit <%d>; ignoring the rest of groups defined"), __func__, filename, *lineNum, MAX_GROUPS);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (type == HOST_GRP && (strchr (keylist[1].val, '~') != NULL && (options & CONF_NO_EXPAND) == 0)) {
                char *outHosts = NULL;
                int numHosts = 0;

                ls_syslog (LOG_DEBUG, "resolveBatchNegHosts in do_Groups \'%s\': the string is \'%s\'", keylist[0].val, keylist[1].val);
                numHosts = resolveBatchNegHosts (keylist[1].val, &outHosts, FALSE);
                if (numHosts > 0) {
                    ls_syslog (LOG_DEBUG, "resolveBatchNegHosts in do_Groups \'%s\' the string is replaced with \'%s\'", keylist[0].val, outHosts);
                }
                else if (numHosts == 0) {
                    /* catgets 5460 */
                    ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5460, "%s: File %s at line %d: there are no hosts found to exclude, replaced with \'%s\'"), __func__, filename, *lineNum, outHosts);
                }
                else if (numHosts == -1) {
                    lsberrno = LSBE_NO_MEM;
                    return FALSE;
                }
                else {
                    if (numHosts == -3) {
                        /* catgets 5461 */
                        ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5461, "%s: \'%s\': \'%s\' The result is that all the hosts are to be excluded."), __func__, keylist[0].val, keylist[1].val);
                    }
                    /* catgets 5108 */
                    ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5108, "%s: File %s at line %d: No valid member in group <%s>; ignoring line"), __func__, filename, *lineNum, keylist[0].val);

                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }

                free (keylist[1].val);
                keylist[1].val = outHosts;
            }

            gp = addGroup (groups, keylist[0].val, ngroups, 0);
            if (gp == NULL && lsberrno == LSBE_NO_MEM) {
                return FALSE;
            }

            sp = keylist[1].val;
            allFlag = FALSE;
            if (searchAll (keylist[1].val) && (strchr (sp, '~') == NULL)) {
                allFlag = TRUE;
            }

            while (!allFlag && (wp = getNextWord_ (&sp)) != NULL) {
                if (!addMember(gp, wp, type, filename, *lineNum, "", options, TRUE) && lsberrno == LSBE_NO_MEM) {
                    return FALSE;
                }
            }

            if (allFlag) {
                if (!addMember (gp, "all", type, filename, *lineNum, "", options, TRUE) && lsberrno == LSBE_NO_MEM) {
                    return FALSE;
                }
            }

            if (gp->memberList == NULL) {
                /* catgets 5108 */
                ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5108, "%s: File %s at line %d: No valid member in group <%s>; ignoring line"), __func__, filename, *lineNum, gp->group);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (gp->group);
                FREEUP (gp);
                *ngroups -= 1;
            }

        }

        ls_syslog (LOG_WARNING, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;

        return TRUE;

    }
    else {
        ls_syslog (LOG_WARNING, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, HUgroups);
        lsberrno = LSBE_CONF_WARNING;

        doSkipSection_conf (conf, lineNum, filename, HUgroups);

        return FALSE;
    }
}

static int
isHostName (char *grpName)
{
    if (Gethostbyname_ (grpName) != NULL) {
        return TRUE;
    }

    for ( unsigned int i = 0; i < numofhosts; i++) {
        if (hosts && hosts[i]->host && equalHost_ (grpName, hosts[i]->host)) {
            return TRUE;
        }
    }

    return FALSE;
}

static struct groupInfoEnt *
addGroup (struct groupInfoEnt **groups, char *gname, unsigned int *ngroups, int type)
{
    if (groups == NULL || ngroups == NULL) {
        return NULL;
    }

    if (type == 0) {

        groups[*ngroups] = malloc(sizeof (struct groupInfoEnt));
        if ( NULL == groups[*ngroups] && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "addGroup", "malloc", sizeof (struct groupInfoEnt));
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }

        initGroupInfo (groups[*ngroups]);
        groups[*ngroups]->group = putstr_ (gname);
        if (NULL == groups[*ngroups]->group ) {
            FREEUP (groups[*ngroups]);
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }
        *ngroups += 1;
        return groups[*ngroups - 1];
    }
    else {

        groups[*ngroups] = groups[*ngroups - 1];
        groups[*ngroups - 1] = malloc(sizeof (struct groupInfoEnt));
        if ( NULL == groups[*ngroups - 1] && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "addGroup", "malloc", sizeof (struct groupInfoEnt));
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }

        initGroupInfo (groups[*ngroups - 1]);
        groups[*ngroups - 1]->group = putstr_ (gname);
        if (groups[*ngroups - 1]->group == NULL) {
            FREEUP (groups[*ngroups - 1]);
            groups[*ngroups - 1] = groups[*ngroups];
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }
        *ngroups += 1;
        return groups[*ngroups - 2];
    }
}

static struct groupInfoEnt *
addUnixGrp (struct group *unixGrp, char *gname, char *filename, unsigned int lineNum, char *section, int type)
{
    int i = -1;
    struct groupInfoEnt *gp;
    struct passwd *pw = NULL;

    if (unixGrp == NULL) {
        return NULL;
    }

    if (numofugroups >= MAX_GROUPS) {
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5131, "addUnixGrp: File %s at line %d: numofugroups <%d> is equal to or greater than MAX_GROUPS <%d>; ignoring the group <%s>"), __func__, lineNum, numofugroups, MAX_GROUPS, unixGrp->gr_name);  /* catgets 5131 */
        return NULL;
    }
    if (gname == NULL) {
        gp = addGroup (usergroups, unixGrp->gr_name, &numofugroups, type);
    }
    else {
        gp = addGroup (usergroups, gname, &numofugroups, type);
    }

    if (gp == NULL && lsberrno == LSBE_NO_MEM) {
        return NULL;
    }

    while (unixGrp->gr_mem[++i] != NULL){
        if ((pw = getpwlsfuser_ (unixGrp->gr_mem[i]))) {
            if (!addMember (gp, unixGrp->gr_mem[i], USER_GRP, filename, lineNum, section, CONF_EXPAND, TRUE)  && lsberrno == LSBE_NO_MEM) {
                return NULL;
            }
            if (logclass & LC_TRACE) {
                ls_syslog (LOG_DEBUG, "addUnixGrp: addMember %s successful", unixGrp->gr_mem[i]);
            }
        }
        else {
            /* catgets 5132 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5132, "addUnixGrp: File %s%s at line %d: Bad user name <%s> in group <%s>; ignored"), __func__, section, lineNum, unixGrp->gr_mem[i], gname);
            lsberrno = LSBE_CONF_WARNING;
            continue;
        }
    }

    if (gp->memberList == NULL) {
        freeGroupInfo (gp);
        numofugroups -= 1;
        return NULL;
    }

    return gp;
}

static char
addMember (struct groupInfoEnt *gp, char *word, int grouptype, char *filename, unsigned int lineNum, char *section, int options, int checkAll)
{
    // 
    int isHp = FALSE;
    size_t len = 0; 
    char isgrp = FALSE;
    char name[MAXHOSTNAMELEN];
    char *cp;
    char *myWord;
    char cpWord[MAXHOSTNAMELEN];
    char returnVal = FALSE;
    struct passwd *pw = NULL;
    struct hostent *hp;
    struct groupInfoEnt *subgrpPtr = NULL;

    if (gp == NULL || word == NULL) {
        return FALSE;
    }

    cp = word;
    if (cp[0] == '~') {
        cp++;
    }

    if( options & CONF_NO_EXPAND ) {
        if (cp == NULL) {
            return FALSE;
        }
        myWord = putstr_ (cp);
        strcpy (cpWord, word);
    }
    else {
        myWord = putstr_ (word);
        strcpy (cpWord, "");
    }

    if (myWord == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (cp) + 1);
        lsberrno = LSBE_NO_MEM;
        return FALSE;
    }

    if (grouptype == USER_GRP && strcmp (myWord, "all") && options != CONF_NO_CHECK) {
        subgrpPtr = getUGrpData (myWord);

        if (gp == subgrpPtr) {
            /* catgets 5134 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5134, "%s: File %s at line %d: Recursive member in Unix group <%s>; ignored"), __func__, filename, lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return FALSE;
        }

        if (!subgrpPtr) {
            pw = getpwlsfuser_ (myWord);
        }

        if (!initUnknownUsers) {
            initTab (&unknownUsers);
            initUnknownUsers = TRUE;
        }

        isgrp = TRUE;
        if (pw != NULL) {
            strcpy (name, pw->pw_name);
            isgrp = FALSE;
        }
        else if (chekMembStr (&unknownUsers, myWord) != NULL) {
            strcpy (name, myWord);
            isgrp = FALSE;
        }
        else if (!subgrpPtr) {
            char *grpSl = NULL;
            struct group *unixGrp = NULL;
            size_t lastChar = strlen (myWord) - 1;

            if (lastChar > 0 && myWord[lastChar] == '/') {
                grpSl = putstr_ (myWord);
                if (grpSl == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (myWord) + 1);
                    lsberrno = LSBE_NO_MEM;
                    FREEUP (myWord);
                    return FALSE;
                }
                grpSl[lastChar] = '\0';
                unixGrp = mygetgrnam (grpSl);
                FREEUP (grpSl);
            }
            else {
                unixGrp = mygetgrnam (myWord);
            }

            if (unixGrp && (options & CONF_EXPAND) ) {
                subgrpPtr = addUnixGrp (unixGrp, myWord, filename, lineNum, section, 1);

                if (!subgrpPtr) {
                    if (lsberrno != LSBE_NO_MEM) {
                        /* catgets 5136 */
                        ls_syslog (LOG_WARNING, I18N (5136, "%s: File %s at line %d: No valid users defined in Unix group <%s>; ignored"), __func__, filename, lineNum, myWord);
                        lsberrno = LSBE_CONF_WARNING;
                    }
                    FREEUP (myWord);
                    return FALSE;
                }
            }
            else {
                /* catgets 5137 */
                ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5137, "%s: File %s%s at line %d: Unknown user <%s> in group <%s>; Maybe a windows user or of another domain."), __func__, filename, section, lineNum, myWord, gp->group);
                lsberrno = LSBE_CONF_WARNING;
                if (!addMembStr (&unknownUsers, myWord)) {
                    FREEUP (myWord);
                    return FALSE;
                }
            }

        }
    }

    if (grouptype == HOST_GRP && strcmp (myWord, "all") && options != CONF_NO_CHECK) {

        subgrpPtr = getHGrpData (myWord);
        if (gp == subgrpPtr) {
            /* catgets 5138 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5138, "%s: File %s at line %d: Recursive member in Unix group <%s>; ignored"), __func__, filename, lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return FALSE;
        }

        if (subgrpPtr != NULL) {
            isgrp = TRUE;
        }
        else {
            if ((hp = Gethostbyname_ (myWord)) == NULL) {
                /* catgets 5139 */
                ls_syslog (LOG_ERR, I18N (5139, "%s: File %s%s at line %d: Bad host/group name <%s> in group <%s>; ignored"), __func__, filename, section, lineNum, myWord, gp->group);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return FALSE;
            }
            strcpy (name, hp->h_name);
            if (getHostData (name) == NULL && numofhosts != 0) {
                /* catgets 5140 */
                ls_syslog (LOG_ERR, I18N (5140, "%s: File %s%s at line %d: Host <%s> is not used by the batch system; ignored"), __func__, filename, section, lineNum, name);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return FALSE;
            }
            if (isServerHost (name) == FALSE) {
                /* catgets 5141 */
                ls_syslog (LOG_ERR, I18N (5141, "%s: File %s%s at line %d: Host <%s> is not a server; ignored"), __func__, filename, section, lineNum, name);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return FALSE;
            }
            isgrp = FALSE;
        }
    }

    if (isHp == FALSE)
        {
        if ((options & CONF_NO_EXPAND) == 0) {
            returnVal = isInGrp (myWord, gp, grouptype, checkAll);
        }
        else {
            returnVal = isInGrp (cpWord, gp, grouptype, checkAll);
        }
    }
    else {
        returnVal = FALSE;
    }

    if (returnVal) {
        if (isgrp){
            /* catgets 5142 */
            ls_syslog (LOG_ERR, I18N (5142, "%s: File %s%s at line %d: Group <%s> is multiply defined in group <%s>; ignored"), __func__, filename, section, lineNum, myWord, gp->group);
        }
        else {
            /* catgets 5143 */
            ls_syslog (LOG_ERR, I18N (5143, "%s: File %s%s at line %d: Member <%s> is multiply defined in group <%s>; ignored"), __func__, filename, section, lineNum, myWord, gp->group);
        }
        lsberrno = LSBE_CONF_WARNING;
        FREEUP (myWord);
        return FALSE;
    }

    if (gp->memberList == NULL) {
        if (options & CONF_NO_EXPAND) {
            gp->memberList = putstr_ (cpWord);
        }
        else {
            gp->memberList = putstr_ (myWord);
        }
        if (gp->memberList == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (myWord) + 1);
            lsberrno = LSBE_NO_MEM;
            FREEUP (myWord);
            return FALSE;
        }
    }
    else {
        if (options & CONF_NO_EXPAND) {
            len = strlen (gp->memberList) + strlen (cpWord) + 2;
        }
        else {
            len = strlen (gp->memberList) + strlen (myWord) + 2;
        }
        gp->memberList = (char *)myrealloc( gp->memberList, len * sizeof (char));
        if ( NULL == gp->memberList && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "myrealloc", len * sizeof (char));
            lsberrno = LSBE_NO_MEM;
            FREEUP (myWord);
            return FALSE;
        }
        strcat (gp->memberList, " ");
        if (options & CONF_NO_EXPAND) {
            strcat (gp->memberList, cpWord);
        }
        else {
            strcat (gp->memberList, myWord);
        }
    }

    FREEUP (myWord);
    return TRUE;
}


static struct groupInfoEnt *
getUGrpData (char *gname)
{
    return getGrpData (usergroups, gname, numofugroups);
}

static struct groupInfoEnt *
getHGrpData (char *gname)
{
    return getGrpData (hostgroups, gname, numofhgroups);
}

static struct groupInfoEnt *
getGrpData (struct groupInfoEnt **groups, char *name, unsigned int num)
{
    if (name == NULL || groups == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < num; i++) {
        if (groups[i] && groups[i]->group && (strcmp (name, groups[i]->group) == 0) ) {
            return groups[i];
        }
    }

    return NULL;
}

static struct userInfoEnt *
getUserData (char *name)
{
    if (name == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < numofusers; i++) {
        if (strcmp (name, users[i]->user) == 0) {
            return users[i];
        }
    }

    return NULL;
}


static struct hostInfoEnt *
getHostData (char *name)
{
    if (name == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < numofhosts; i++) {
        if (equalHost_ (name, hosts[i]->host)) {
            return hosts[i];
        }
    }

    return NULL;
}

static struct queueInfoEnt *
getQueueData (char *name)
{
    if (name == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < numofqueues; i++) {
        if (strcmp (name, queues[i]->queue) == 0) {
            return queues[i];
        }
    }

    return NULL;
}

static char
searchAll (char *word)
{
    char *sp, *cp;

    if (word == NULL) {
        return FALSE;
    }
    cp = word;
    while ((sp = getNextWord_ (&cp))) {
        if (strcmp (sp, "all") == 0) {
            return TRUE;
        }
    }
    return FALSE;

}


static void
initUserInfo (struct userInfoEnt *up)
{
    if (up != NULL) {
        up->user = NULL;
        up->procJobLimit = INFINIT_FLOAT;
        up->maxJobs = INFINIT_INT;
        up->numStartJobs = INFINIT_INT;
        up->numJobs = INFINIT_INT;
        up->numPEND = INFINIT_INT;
        up->numRUN = INFINIT_INT;
        up->numSSUSP = INFINIT_INT;
        up->numUSUSP = INFINIT_INT;
        up->numRESERVE = INFINIT_INT;
    }
}

static void
freeUserInfo (struct userInfoEnt *up)
{
    if (up != NULL) {
        FREEUP (up->user);
    }
}

static void
initGroupInfo (struct groupInfoEnt *gp)
{
    if (gp != NULL) {
        gp->group = NULL;
        gp->memberList = NULL;
    }
}

static void
freeGroupInfo (struct groupInfoEnt *gp)
{
    if (gp != NULL) {
        FREEUP (gp->group);
        FREEUP (gp->memberList);
    }
}

static char
addUser (char *username, int maxjobs, float pJobLimit, char *filename, int override)
{
    struct userInfoEnt *up        = NULL;
    struct userInfoEnt **tmpUsers = NULL;

    if (username == NULL) {
        return FALSE;
    }

    if ((up = getUserData (username)) != NULL) {
        if (override)  {
            freeUserInfo (up);
            initUserInfo (up);
            up->user = putstr_ (username);
            if (up->user == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "%s", "malloc", __func__, strlen (username));
                lsberrno = LSBE_NO_MEM;
                return FALSE;
            }
            up->procJobLimit = pJobLimit;
            up->maxJobs = maxjobs;
            return TRUE;
        }
        else {
            if (filename) {
                /* catgets 5147 */
                ls_syslog (LOG_ERR, I18N (5147, "%s: %s: User <%s> is multiply defined; retaining old definition"), __func__, filename, username);
                lsberrno = LSBE_CONF_WARNING;
            }
            return FALSE;
        }
    }
    else {
        if (numofusers == usersize) {
            if (usersize == 0) {
                usersize = 5;
            }
            else {
                usersize *= 2;
            }
            tmpUsers = myrealloc(users, usersize * sizeof (struct userInfoEnt *));
            if ( NULL == tmpUsers && ENOMEM == errno ) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "myrealloc", usersize * sizeof (struct userInfoEnt *));
                lsberrno = LSBE_NO_MEM;
                return FALSE;
            }
            else {
                users = tmpUsers;
            }
        }

        users[numofusers] = malloc (sizeof (struct userInfoEnt));
        if ( NULL == users[numofusers] ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct userInfoEnt));
            lsberrno = LSBE_NO_MEM;
            return FALSE;
        }

        initUserInfo (users[numofusers]);
        users[numofusers]->user = putstr_ (username);
        if (users[numofusers]->user == NULL) {
            FREEUP (users[numofusers]);
            lsberrno = LSBE_NO_MEM;
            return FALSE;
        }

        users[numofusers]->procJobLimit = pJobLimit;
        users[numofusers]->maxJobs = maxjobs;
        numofusers++;
        return TRUE;
    }
}

static char
isInGrp (char *word, struct groupInfoEnt *gp, int grouptype, int checkAll)
{
    char *tmp = NULL;
    char *str = NULL;
    struct groupInfoEnt *sub_gp = NULL;

    if (word == NULL || gp == NULL || gp->memberList == NULL) {
        return FALSE;
    }

    if (word[0] == '~')
        {
        return FALSE;
        }

    tmp = gp->memberList;

    if (!strcmp (tmp, "all") && checkAll == TRUE) {
        return TRUE;
    }

    while ((str = getNextWord_ (&tmp)) != NULL)
        {
        if (grouptype == USER_GRP)
            {
            if (!strcmp (str, word))
                return TRUE;
            }
        else
            {
            if (equalHost_ (str, word))
                return TRUE;
            }

        if (grouptype == USER_GRP) {
            sub_gp = getUGrpData (str);
        }
        else {
            sub_gp = getHGrpData (str);
        }

        if (isInGrp (word, sub_gp, grouptype, FALSE))

            return TRUE;
        }

    return FALSE;

}

static char **
expandGrp (char *word, unsigned int *num, int grouptype)
{
    char *tmp = NULL;
    char *str =  NULL;
    unsigned int sub_num = 0;
    unsigned int n = 0;
    char **sub_list = NULL;
    char **list     = NULL;
    char **tempList = NULL;
    struct groupInfoEnt *gp = NULL;
    struct groupInfoEnt *sub_gp = NULL;

    if (word == NULL || num == NULL) {
        return NULL;
    }

    if (grouptype == USER_GRP) {
        gp = getUGrpData (word);
    }
    else {
        gp = getHGrpData (word);
    }

    if (gp == NULL) {
        list = malloc (sizeof (char *));
        if ( NULL == list && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (char *));
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }

        if ((list[0] = putstr_ (word)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (word) + 1);
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }
        n = 1;
    }
    else {
        tmp = gp->memberList;
        if (!strcmp (tmp, "all")){
            tempList = myrealloc (list, sizeof (char *));
            if ( NULL == tempList && ENOMEM == errno ) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "myrealloc", sizeof (char *));
                lsberrno = LSBE_NO_MEM;
                return NULL;
            }
            list = tempList;
            if ((list[0] = putstr_ ("all")) == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen ("all") + 1);
                lsberrno = LSBE_NO_MEM;
                return NULL;
            }

            n = 1;
            *num = n;
            return list;
        }

        while ((str = getNextWord_ (&tmp)) != NULL) {

            if (grouptype == USER_GRP) {
                sub_gp = getUGrpData (str);
            }
            else {
                sub_gp = getHGrpData (str);
            }

            if (sub_gp == NULL) {
                tempList = myrealloc(list, (n + 1) * sizeof (char *));
                if ( NULL == tempList && ENOMEM == errno ) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "myrealloc", (n + 1) * sizeof (char *));
                    lsberrno = LSBE_NO_MEM;
                    return NULL;
                }

                list = tempList;
                if ((list[n] = putstr_ (str)) == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (str) + 1);
                    lsberrno = LSBE_NO_MEM;
                    return NULL;
                }

                n++;
            }
            else {
                sub_num = 0;
                sub_list = expandGrp (str, &sub_num, grouptype);
                if (sub_list == NULL && lsberrno == LSBE_NO_MEM) {
                    return NULL;
                }

                if (sub_num) {
                    if (!strcmp (sub_list[0], "all")) {

                        tempList = myrealloc(list, sizeof (char *));
                        if ( NULL == tempList && ENOMEM == errno ) {
                            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "myrealloc", sizeof (char *));
                            lsberrno = LSBE_NO_MEM;
                            return NULL;
                        }
                        list = tempList;
                        if ((list[0] = putstr_ ("all")) == NULL) {
                            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen ("all") + 1);
                            lsberrno = LSBE_NO_MEM;
                            return NULL;
                        }

                        n = 1;
                        *num = n;
                        return list;
                    }

                    list = (char **) myrealloc(list, (n + sub_num) * sizeof (char *));
                    if ( NULL == list && ENOMEM == errno ) {
                        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M,  __func__, "realloc", (n + sub_num) * sizeof (char *));
                        lsberrno = LSBE_NO_MEM;
                        return NULL;
                    }
                    for ( unsigned int i = 0; i < sub_num; i++) {
                        list[n + i] = sub_list[i];
                    }
                    n += sub_num;
                }
                FREEUP (sub_list);
            }
        }
    }

    *num = n;
    return list;

}

struct hostConf *
lsb_readhost (struct lsConf *conf, struct lsInfo *info, int options, struct clusterConf *clusterConf)
{
    // 
    struct lsInfo myinfo;
    char *filename = NULL;
    char *cp       = NULL;
    char *section  = NULL;
    char hostok;
    char hgroupok = 'a';
    char hpartok  = 'a';
    size_t *lineNum = 0;

    hostok = FALSE;
    hpartok = FALSE;
    hgroupok = FALSE;

    assert( hostok );
    assert( hpartok );
    assert( hgroupok );

    lsberrno = LSBE_NO_ERROR;

    if (info == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "lsinfo");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if ((options != CONF_NO_CHECK) && clusterConf == NULL)  {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "clusterConf");
        return NULL;
    }

    if ((options != CONF_NO_CHECK) && uConf == NULL)  {
        /* catgets 5446 */
        ls_syslog (LOG_INFO, (_i18n_msg_get (ls_catd, NL_SETN, 5446, "%s: default user will be used.")), __func__);  // FIXME FIXME __func__ here is wrong. it should be filename, but it is not init'ed yet
        if (setDefaultUser ()) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "setDefaultUser");
            return NULL;
        }
    }
    myinfo = *info;
    cConf = (struct clusterConf *)clusterConf;

    assert( info->nRes >= 0 );
    myinfo.resTable = malloc( (size_t)info->nRes * sizeof (struct resItem));
    if (info->nRes && NULL == myinfo.resTable && ENOMEM == errno ) {
        assert( info->nRes );
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", info->nRes * sizeof (struct resItem));
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }
    for ( unsigned int k = 0, l = 0; k < info->nRes; k++) {
        if (info->resTable[k].flags & RESF_DYNAMIC) {
            memcpy (&myinfo.resTable[l], &info->resTable[k], sizeof (struct resItem));
            l++;
        }
    }
    for ( unsigned int i = 0, j = 0; i < info->nRes; i++) {
        if (!(info->resTable[i].flags & RESF_DYNAMIC)) {
            memcpy (&myinfo.resTable[j], &info->resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    if (!conf) {
        FREEUP (myinfo.resTable);
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "conf");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (!conf->confhandle) {
        FREEUP (myinfo.resTable);
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "confhandle");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }
    if (handleHostMem ()) {
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    filename = conf->confhandle->fname;
    conf->confhandle->curNode = conf->confhandle->rootNode;
    conf->confhandle->lineCount = 0;

    for (;;) {
        if ((cp = getBeginLine_conf (conf, lineNum)) == NULL) {
            if (!hostok) {
                /* catgets 5165 */
                ls_syslog (LOG_ERR, I18N (5165, "%s: Host section missing or invalid."), filename);
                lsberrno = LSBE_CONF_WARNING;
            }

            if( numofhosts ) {
                hConf->hosts = malloc(numofhosts * sizeof (struct hostInfoEnt));
                if ( NULL == hConf->hosts && ENOMEM == errno) { // FIXME FIXME FIXME FIXME this may be wrong and need some adjustment
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", numofhosts * sizeof (struct hostInfoEnt));
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkHost (TRUE);
                    freeHConf (hConf, FALSE);
                    FREEUP (myinfo.resTable);
                    return NULL;
                }

                for ( unsigned int i = 0; i < numofhosts; i++) {
                    initHostInfoEnt ((struct hostInfoEnt *)&hConf->hosts[i]);
                    hConf->hosts[i] = *hosts[i];
                }
                hConf->numHosts = numofhosts;
            }

            if (numofhgroups) {
                hConf->hgroups = malloc (numofhgroups * sizeof (struct groupInfoEnt));
                if ( NULL == hConf->hgroups && ENOMEM == errno ) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", numofhgroups * sizeof (struct groupInfoEnt));
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkHost (TRUE);
                    freeHConf (hConf, FALSE);
                    FREEUP (myinfo.resTable);
                    return NULL;
                }
                for ( unsigned int i = 0; i < numofhgroups; i++) {
                    initGroupInfo (&hConf->hgroups[i]);
                    hConf->hgroups[i] = *hostgroups[i];
                }
                hConf->numHgroups = numofhgroups;
            }

            FREEUP (myinfo.resTable);
            return hConf;
        }
        section = getNextWord_ (&cp);
        if (!section) {
            /* catgets 5169 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5169, "%s: File %s at line %d: Section name expected after Begin; ignoring section"), __func__, filename, &lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, "unknown");
            continue;
        }
        else {
            if (strcasecmp (section, "host") == 0) {
                if (do_Hosts_(conf, filename, lineNum, &myinfo, options)) {
                    hostok = TRUE;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkHost (TRUE);
                    freeHConf (hConf, FALSE);
                    FREEUP (myinfo.resTable);
                    return NULL;
                }
                continue;
            }
            else if (strcasecmp (section, "hostgroup") == 0) {

                if (do_Groups(hostgroups, conf, filename, lineNum, &numofhgroups, options)) {
                    hgroupok = TRUE;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkHost (TRUE);
                    FREEUP (myinfo.resTable);
                    freeHConf (hConf, FALSE);
                    return NULL;
                }
                continue;
            }
            else {
                /* catgets 5170 */
                ls_syslog (LOG_ERR, I18N (5170, "%s: File %s at line %d: Invalid section name <%s>; ignoring section"), __func__, filename, lineNum, section);
                lsberrno = LSBE_CONF_WARNING;
                doSkipSection_conf (conf, lineNum, filename, section);
            }
        }
    }
}

static char
do_Hosts_ (struct lsConf *conf, char *filename, size_t *lineNum, struct lsInfo *info, int options)
{

#define HKEY_HNAME info->numIndx
#define HKEY_MXJ info->numIndx+1
#define HKEY_RUN_WINDOW info->numIndx+2
#define HKEY_MIG info->numIndx+3
#define HKEY_UJOB_LIMIT info->numIndx+4
#define HKEY_DISPATCH_WINDOW info->numIndx+5

    static struct keymap *keylist = NULL;
//   
    char *linep;
    char hostname[MAXHOSTNAMELEN];
    int num                 = 0;
    int new                 = 0;
    int numSelectedHosts    = 0;
    int isTypeOrModel       = FALSE;
    int returnCode          = FALSE;
    int copyCPUFactor       = FALSE;
    size_t *override        = 0;
    struct hostent *hp      = NULL;
    struct hTab *tmpHosts;
    struct hostInfoEnt host;
    struct hostInfo *hostList;
    struct htab *nonOverridableHosts = NULL;
    struct hostInfo *hostInfo;


    if (conf == NULL) {
        return FALSE;
    }

    FREEUP (keylist);
    keylist = malloc ((unsigned long)(HKEY_DISPATCH_WINDOW + 2) * sizeof (struct keymap));
    if ( NULL == keylist && ENOMEM == errno ){
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        return FALSE;
    }

    assert( HKEY_HNAME + 1 <= INT_MAX );
    assert( HKEY_DISPATCH_WINDOW + 2 <= INT_MAX );
    initkeylist (keylist, (int)HKEY_HNAME + 1, (int)(HKEY_DISPATCH_WINDOW + 2), info);
    keylist[HKEY_HNAME].key               = "HOST_NAME";
    keylist[HKEY_MXJ].key                 = "MXJ";
    keylist[HKEY_RUN_WINDOW].key          = "RUN_WINDOW";
    keylist[HKEY_MIG].key                 = "MIG";
    keylist[HKEY_UJOB_LIMIT].key          = "JL/U";
    keylist[HKEY_DISPATCH_WINDOW].key     = "DISPATCH_WINDOW";
    keylist[HKEY_DISPATCH_WINDOW + 1].key = NULL;

    initHostInfoEnt ((struct hostInfoEnt *)&host);
    linep = getNextLineC_conf (conf, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, "host")) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum, "host");
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (strchr (linep, '=') != NULL) {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "host");
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, "host");
        return FALSE;
    }

    if (!keyMatch (keylist, linep, FALSE)) {
        /* catgets 5174 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5174, "%s: File %s at line %d: Keyword line format error for Host section; ignoring section"), __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, "host");
        return FALSE;
    }

    if (keylist[HKEY_HNAME].position < 0) {
        /* catgets 5175 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5175, "%s: File %s at line %d: Hostname required for Host section; ignoring section"), __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, "host");
        return FALSE;
    }

    nonOverridableHosts = malloc( sizeof( struct hTab ) );
    tmpHosts            = malloc( sizeof( struct hTab ) );
    hostList            = malloc( cConf->numHosts *  sizeof( struct hostInfo ) );
    if ( ( NULL == nonOverridableHosts && ENOMEM == errno ) || 
         ( NULL == tmpHosts            && ENOMEM == errno ) ||
         ( NULL == hostList            && ENOMEM == errno )
        )
    {

        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc",  cConf->numHosts * sizeof (struct hostInfo));
        lsberrno = LSBE_NO_MEM;
        FREEUP (nonOverridableHosts);
        FREEUP (tmpHosts);
        FREEUP (hostList);
        return FALSE;
    }
    h_initTab_ (tmpHosts, 32);                      // FIXME FIXME 32 seems awfully specific
    h_initTab_ ((hTab *)nonOverridableHosts, 32);   // FIXME FIXME 32 seems awfully specific
    while ((linep = getNextLineC_conf (conf, lineNum, TRUE)) != NULL) {

        freekeyval (keylist);
        initHostInfoEnt( (struct hostInfoEnt *)&host );
        isTypeOrModel    = FALSE;
        numSelectedHosts = 0;

        if (isSectionEnd (linep, filename, lineNum, "host")) {
            FREEUP (hostList);
            h_delTab_ ((hTab *)nonOverridableHosts);
            FREEUP (nonOverridableHosts);
            h_delTab_ (tmpHosts);
            FREEUP (tmpHosts);
            freeHostInfoEnt ((struct hostInfoEnt *)&host);
            assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
            return (char)returnCode;
        }

        if (mapValues (keylist, linep) < 0) {
            /* catgets 5177 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5177, "%s: File %s at line %d: Values do not match keys in Host section; ignoring line"), __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            continue;
        }

        if (strcmp (keylist[HKEY_HNAME].val, "default") != 0) {

            hp = Gethostbyname_ (keylist[HKEY_HNAME].val);
            if (!hp && options != CONF_NO_CHECK) {

                unsigned int total1 = 0;
                for ( unsigned int i = 0; i < info->nModels; i++) {
                    total1 = i;
                    if (strcmp (keylist[HKEY_HNAME].val, info->hostModels[i]) == 0) {
                        break;
                    }
                }

                if ( total1 == (unsigned int) info->nModels) {

                    unsigned int total2 = 0;
                    for ( unsigned int i = 0; i < info->nTypes; i++) {

                        total2 = i;
                        if( strcmp( keylist[HKEY_HNAME].val, info->hostTypes[i]) == 0 ) {
                            break;
                        }
                    }

                    if( total2 == (unsigned int) info->nTypes ) {
                        /* catgets 5178 */
                        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5178, "%s: File %s at line %d: Invalid host/type/model name <%s>; ignoring line"), __func__, filename, *lineNum, keylist[HKEY_HNAME].val);
                        lsberrno = LSBE_CONF_WARNING;
                        continue;
                    }
                }

                numSelectedHosts = 0;
                for ( unsigned int i = 0; i < cConf->numHosts; i++) {
                    hostInfo = &(cConf->hosts[i]);
                    if( ( strcmp (hostInfo->hostType,  keylist[HKEY_HNAME].val) == 0 || 
                          strcmp (hostInfo->hostModel, keylist[HKEY_HNAME].val) == 0
                        ) && hostInfo->isServer == TRUE) 
                    {
                        hostList[numSelectedHosts] = cConf->hosts[i];
                        numSelectedHosts++;
                    }
                }

                if (!numSelectedHosts) {
                    /* catgets 5179 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5179, "%s: File %s at line %d: no server hosts of type/model <%s> are known by LSF; ignoring line"), __func__, filename, *lineNum, keylist[HKEY_HNAME].val);
                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }
                strcpy (hostname, keylist[HKEY_HNAME].val);
                isTypeOrModel = TRUE;
            }
            else {
                if (hp) {
                    strcpy (hostname, hp->h_name);
                }
                else if (options == CONF_NO_CHECK) {
                    strcpy (hostname, keylist[HKEY_HNAME].val);
                }
            }
        }
        else {
            strcpy (hostname, "default");
        }
        h_addEnt_ (tmpHosts, hostname, &new);
        if (!new) {
            /* catgets 5180 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5180, "%s: File %s at line %d: Host name <%s> multiply specified; ignoring line"), __func__, filename, *lineNum, keylist[HKEY_HNAME].val );
            lsberrno = LSBE_CONF_WARNING;
            continue;
        }

        if (keylist[HKEY_MXJ].val != NULL && strcmp (keylist[HKEY_MXJ].val, "!") == 0) {
            host.maxJobs = 0;
        }
        else if( keylist[HKEY_MXJ].position >= 0 && 
                 keylist[HKEY_MXJ].val != NULL   && 
                 strcmp (keylist[HKEY_MXJ].val, "") ) 
        {
            assert( my_atoi (keylist[HKEY_MXJ].val, INFINIT_INT, -1) >= 0);
            host.maxJobs = my_atoi (keylist[HKEY_MXJ].val, INFINIT_INT, -1);
            // if ( fabs( INFINIT_INT - host.maxJobs) < 0.00001 ) {
            if ( ( INFINIT_INT - host.maxJobs) < 0.00001 ) {
                /* catgets 5183 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5183, "%s: File %s at line %d: Invalid value <%s> for key <%s>; %d is assumed"), __func__, filename, *lineNum, keylist[HKEY_MXJ].val, keylist[HKEY_MXJ].key, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if( keylist[HKEY_UJOB_LIMIT].position >= 0 && keylist[HKEY_UJOB_LIMIT].val != NULL && strcmp (keylist[HKEY_UJOB_LIMIT].val, "")) {

            assert( my_atoi( keylist[HKEY_UJOB_LIMIT].val, INFINIT_INT, -1 ) >=0 );
            host.userJobLimit = my_atoi( keylist[HKEY_UJOB_LIMIT].val, INFINIT_INT, -1 );
            // if ( fabs( INFINIT_INT - host.userJobLimit ) < 0.00001 ) {
            if ( ( INFINIT_INT - host.userJobLimit ) < 0.00001 ) {
                /* catgets 5183 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5183, "%s: File %s at line %d: Invalid value <%s> for key <%s>; %d is assumed"), __func__, filename, *lineNum, keylist[HKEY_UJOB_LIMIT].val, keylist[HKEY_UJOB_LIMIT].key, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[HKEY_RUN_WINDOW].position >= 0) {

            if (strcmp (keylist[HKEY_RUN_WINDOW].val, "")) {
                host.windows = parsewindow (keylist[HKEY_RUN_WINDOW].val, filename, lineNum, "Host");

                if(  lserrno == LSE_CONF_SYNTAX) {
                     lserrno  = LSE_NO_ERR;
                     lsberrno = LSBE_CONF_WARNING;
                }
            }
        }

        if (keylist[HKEY_DISPATCH_WINDOW].position >= 0) {

            if (strcmp (keylist[HKEY_DISPATCH_WINDOW].val, "")) {
                FREEUP (host.windows);
                host.windows = parsewindow (keylist[HKEY_DISPATCH_WINDOW].val, filename, lineNum, "Host");

                if (lserrno == LSE_CONF_SYNTAX) {
                    lserrno = LSE_NO_ERR;
                    lsberrno = LSBE_CONF_WARNING;
                }
            }
        }

        if (keylist[HKEY_MIG].position >= 0 && keylist[HKEY_MIG].val != NULL && strcmp (keylist[HKEY_MIG].val, "")) {

            host.mig = my_atoi (keylist[HKEY_MIG].val, INFINIT_INT / 60, -1);
            // if ( fabs( INFINIT_INT - host.mig ) < 0.00001 ) {
            if ( ( INFINIT_INT - host.mig ) < 0.00001 ) {
                /* catgets 5186 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5186, "%s: File %s at line %d: Invalid value <%s> for key <%s>; no MIG threshold is assumed"), __func__, filename, *lineNum, keylist[HKEY_MIG].val, keylist[HKEY_MIG].key);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        assert( info->numIndx >= 0 );
        host.loadSched = (float *)malloc( (unsigned long)info->numIndx * sizeof (float *));
        if (info->numIndx && ( NULL == host.loadSched && ENOMEM == errno ) ) {
            assert( info->numIndx >= 0 );
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", info->numIndx * sizeof (float *));
            lsberrno = LSBE_NO_MEM;
            freekeyval (keylist);
            FREEUP (hostList);
            h_delTab_ ( (hTab *)nonOverridableHosts);
            FREEUP (nonOverridableHosts);
            h_delTab_ (tmpHosts);
            FREEUP (tmpHosts);
            FREEUP (host.windows);
            FREEUP (host.loadSched);
            FREEUP (host.loadStop);
            assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
            return (char)returnCode;
        }

        assert( info->numIndx >= 0 );
        host.loadStop = malloc( (unsigned long)info->numIndx * sizeof (float *));
        if (info->numIndx && ( NULL == host.loadStop && ENOMEM == errno ) ) {
            assert( info->numIndx >= 0 );
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", info->numIndx * sizeof (float *));
            lsberrno = LSBE_NO_MEM;
            freekeyval (keylist);
            FREEUP (hostList);
            h_delTab_ ( (hTab *)nonOverridableHosts);
            FREEUP (nonOverridableHosts);
            h_delTab_ (tmpHosts);
            FREEUP (tmpHosts);
            FREEUP (host.windows);
            FREEUP (host.loadSched);
            FREEUP (host.loadStop);
            assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
            return (char)returnCode;
        }

        getThresh (info, keylist, host.loadSched, host.loadStop, filename, lineNum, " in section Host ending");
        host.nIdx = info->numIndx;
        if (options == CONF_NO_CHECK) {
            host.host = hostname;
            num = 1;
            *override = (size_t)TRUE;
            copyCPUFactor = FALSE;
            h_addEnt_ ( (hTab *)nonOverridableHosts, hostname, &new);
        }
        else if (strcmp (hostname, "default") == 0) {
            num = 0;
            for ( unsigned int i = 0; i < cConf->numHosts; i++) {
                if (cConf->hosts[i].isServer != TRUE) {
                    continue;
                }
                hostList[num++] = cConf->hosts[i];
            }

            override = FALSE;
            copyCPUFactor = TRUE;
        }
        else if (isTypeOrModel) {
            num           = numSelectedHosts;
            *override     = (size_t) TRUE;
            copyCPUFactor = TRUE;
        }
        else {
            unsigned int total = 0;
            for ( unsigned int i = 0; i < cConf->numHosts; i++) {
                if (equalHost_ (hostname, cConf->hosts[i].hostName)) {
                    hostList[0] = cConf->hosts[i];
                    break;
                }
                total = i;
            }
            if (total == cConf->numHosts) {
                num = 0;
                /* catgets 5189 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5189, "%s: File %s at line %d: Can't find the information of host <%s>"), __func__, filename, *lineNum, hostname);
                lsberrno = LSBE_CONF_WARNING;
                freeHostInfoEnt (&host);
            }
            else if (cConf->hosts[total].isServer != TRUE) {
                num = 0;
                /* catgets 5190 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5190, "%s: File %s at line %d: Host <%s> is not a server;ignoring"), __func__, filename, *lineNum, hostname);
                lsberrno = LSBE_CONF_WARNING;
                freeHostInfoEnt ( &host);
            }
            else {
                num = 1;
                *override = (size_t) TRUE;
                h_addEnt_( (hTab *)nonOverridableHosts, hostList[0].hostName, &new);
                copyCPUFactor = TRUE;
            }
        }
        for( int i = 0; i < num; i++) {

            if (isTypeOrModel && h_getEnt_ ((hTab *)nonOverridableHosts, hostList[i].hostName)) {
                continue;
            }

            if (copyCPUFactor == TRUE) {
                host.host = hostList[i].hostName;

                for( unsigned int j = 0; j < info->nModels; j++) {
                    if (strcmp (hostList[i].hostModel, info->hostModels[j]) == 0) {
                        host.cpuFactor = info->cpuFactor[j];
                        break;
                    }
                }
            }

            if (addHostEnt (&host, &hostList[i], override) == FALSE) {
                    freekeyval (keylist);
                    FREEUP (hostList);
                    h_delTab_ ((hTab *)nonOverridableHosts);
                    FREEUP (nonOverridableHosts);
                    h_delTab_ (tmpHosts);
                    FREEUP (tmpHosts);
                    FREEUP (host.windows);
                    FREEUP (host.loadSched);
                    FREEUP (host.loadStop);
                    assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
                    return (char)returnCode;

            }
        }
        FREEUP (host.windows);
        FREEUP (host.loadSched);
        FREEUP (host.loadStop);

    }

    ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
    lsberrno = LSBE_CONF_WARNING;
    returnCode = TRUE;

    assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
    return (char)returnCode;
}


static void
getThresh (struct lsInfo *info, struct keymap *keylist, float loadSched[], float loadStop[], char *filename, size_t *lineNum, char *section)
{

    char *stop;
    float swap = 0;

    initThresholds (info, loadSched, loadStop);

    for ( unsigned int i = 0; i < info->numIndx; i++) {
        if (keylist[i].position < 0) {
            continue;
        }
        if (keylist[i].val == NULL) {
            continue;
        }
        if (strcmp (keylist[i].val, "") == 0) {
            continue;
        }

        if ((stop = strchr (keylist[i].val, '/')) != NULL) {
            *stop = '\0';
            stop++;
            if (stop[0] == '\0'){
                stop = NULL;
            }
        }
        if (*keylist[i].val != '\0' && (loadSched[i] = my_atof (keylist[i].val, INFINIT_LOAD, -INFINIT_LOAD)) >= INFINIT_LOAD) {
            /* catgets 5192 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5192, "%s: File %s%s at line %d: Value <%s> of loadSched <%s> isn't a float number between %1.1f and %1.1f; ignored"), __func__, filename, section, *lineNum, keylist[i].val, keylist[i].key, -INFINIT_LOAD, INFINIT_LOAD);
            lsberrno = LSBE_CONF_WARNING;
            if (info->resTable[i].orderType == DECR)
                loadSched[i] = -INFINIT_LOAD;
            }
        if (*keylist[i].val != '\0' && loadSched[i] < 0 && loadSched[i] > -INFINIT_LOAD) {
            /* catgets 5193 */
            ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5193, "%s: File %s%s at line %d: Warning: Value <%s> of loadSched <%s> is not a non-negative number"), __func__, filename, section, *lineNum, keylist[i].val, keylist[i].key);
            lsberrno = LSBE_CONF_WARNING;
        }

        if (i == UT && loadSched[i] > 1.0 && loadSched[i] < INFINIT_LOAD ) {
            /* catgets 5447 */
            ls_syslog (LOG_INFO, (_i18n_msg_get (ls_catd, NL_SETN, 5447, "%s: File %s %s at line %d: For load index <%s>, loadSched <%2.2f> is greater than 1; assumming <%5.1f%%>")), __func__, filename, section, *lineNum, keylist[i].key, loadSched[i], loadSched[i]);
            lsberrno = LSBE_CONF_WARNING;
            loadSched[i] /= 100.0;
        }

        if (!stop) {
            continue;
        }

        loadStop[i] = my_atof (stop, INFINIT_LOAD, -INFINIT_LOAD);
        if ( fabs(INFINIT_LOAD - loadStop[i]) < 0.000001 ) {
            /* catgets 5194 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5194, "%s: File %s%s at line %d: Value <%s> of loadStop <%s> isn't a float number between %1.1f and %1.1f; ignored"), __func__, filename, section, *lineNum, stop, keylist[i].key, -INFINIT_LOAD, INFINIT_LOAD);
            lsberrno = LSBE_CONF_WARNING;
            if (info->resTable[i].orderType == DECR) {
                loadStop[i] = -INFINIT_LOAD;
            }
            continue;
        }

        if (loadStop[i] < 0 && loadStop[i] > -INFINIT_LOAD) {
            /* catgets 5195 */
            ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5195, "%s: File %s%s at line %d: Warning: Value <%s> of loadStop <%s> is not a non-negative number"), __func__, filename, section, *lineNum, stop, keylist[i].key);
            lsberrno = LSBE_CONF_WARNING;
        }

        if (i == UT && loadStop[i] > 1.0 && loadSched[i] < INFINIT_LOAD) {
            /* catgets 5440 */
            ls_syslog (LOG_INFO, (_i18n_msg_get (ls_catd, NL_SETN, 5440, "%s: File %s%s at line %d: For load index <%s>, loadStop <%2.2f> is greater than 1; assumming <%5.1f%%>")), __func__, filename, section, *lineNum, keylist[i].key, loadStop[i], loadStop[i]);
            lsberrno = LSBE_CONF_WARNING;
            loadStop[i] /= 100.0;
        }

        if ((loadSched[i] > loadStop[i]) && info->resTable[i].orderType == INCR) {
            /* catgets 5196 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5196, "%s: File %s%s at line %d: For load index <%s>, loadStop <%2.2f> is lower than loadSched <%2.2f>; swapped"), __func__, filename, section, *lineNum, keylist[i].key, loadStop[i], loadSched[i]);
            lsberrno = LSBE_CONF_WARNING;
            swap = loadSched[i];
            loadSched[i] = loadStop[i];
            loadStop[i] = swap;
        }

        if ((loadStop[i] > loadSched[i]) && info->resTable[i].orderType == DECR) {
            /* catgets 5197 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5197, "%s: File %s%s at line %d: For load index <%s>, loadStop <%2.2f> is higher than loadSched <%2.2f>; swapped"), __func__, filename, section, *lineNum, keylist[i].key, loadStop[i], loadSched[i]);
            lsberrno = LSBE_CONF_WARNING;
            swap = loadSched[i];
            loadSched[i] = loadStop[i];
            loadStop[i] = swap;
        }
    }

}

static int
addHostEnt (struct hostInfoEnt *hp, struct hostInfo *hostInfo, size_t *override)
{
    struct hostInfoEnt *host;
    bool_t bExists = FALSE;
    unsigned int ihost = 0;

    if (hp == NULL) {
        return FALSE;
    }

    if (numofhosts == hostsize) {
        struct hostInfoEnt **tmpHosts;
        if (hostsize == 0) {
            hostsize = 5;
        }
        else {
            hostsize *= 2;
        }
        tmpHosts = (struct hostInfoEnt **) myrealloc(hosts, hostsize * sizeof (struct hostInfoEnt *));
        if ( NULL == tmpHosts && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "realloc", hostsize * sizeof (struct hostInfoEnt *));
            lsberrno = LSBE_NO_MEM;
            freeHostInfoEnt( hp );
            return FALSE;
        }
        else {
            hosts = tmpHosts;
        }
    }

    if ( (hp->host == NULL && (host = getHostData (hostInfo->hostName)) != NULL) || 
         (hp->host != NULL && (host = getHostData (hp->host)) != NULL) )
    {
        bExists = TRUE;
        if (override) {
            for ( unsigned int i = 0; i < numofhosts; i++) {
                if (equalHost_ (host->host, hosts[i]->host)) {
                    ihost = i;
                    break;
                }
            }
            freeHostInfoEnt( host );
        }
        else {
            return TRUE;
        }
    }
    else {
        host = malloc( sizeof (struct hostInfoEnt));
        if ( NULL == host && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct hostInfoEnt));
            lsberrno = LSBE_NO_MEM;
            return FALSE;
        }
    }
    initHostInfoEnt( host );
    if (copyHostInfo (host, hp) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, __func__);
        FREEUP (host);
        return FALSE;
    }

    if (bExists) {
        hosts[ihost] = host;
    }
    else {
        hosts[numofhosts] = host;
        numofhosts++;
    }

    return TRUE;
}

static int
copyHostInfo (struct hostInfoEnt *toHost, struct hostInfoEnt *fromHost)
{
    memcpy (toHost, fromHost, sizeof (struct hostInfoEnt));
    if ((toHost->host = putstr_ (fromHost->host)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (fromHost->host) + 1);
        lsberrno = LSBE_NO_MEM;
        return -1;
    }

    if( fromHost->nIdx ) {
        assert( fromHost->nIdx >= 0 );
        toHost->loadSched = malloc( (unsigned long)(fromHost->nIdx) * sizeof (float *) );
        toHost->loadStop  = malloc( (unsigned long)(fromHost->nIdx) * sizeof (float *) );
        if(  ( NULL == toHost->loadSched && ENOMEM == errno )  ||
             ( NULL == toHost->loadStop  && ENOMEM == errno ) ||
             ( fromHost->windows && (toHost->windows = putstr_ (fromHost->windows)) == NULL)
        ) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            lsberrno = LSBE_NO_MEM;
            FREEUP (toHost->host);
            FREEUP (toHost->loadSched);
            FREEUP (toHost->loadStop);
            FREEUP (toHost->windows);
            return -1;
        }
    }
    if (fromHost->loadSched != NULL) {
        for ( unsigned int i = 0; i < fromHost->nIdx; i++) {
            toHost->loadSched[i] = fromHost->loadSched[i];
        }
    }
    if (fromHost->loadStop != NULL) {
        for ( unsigned int i = 0; i < fromHost->nIdx; i++) {
            toHost->loadStop[i] = fromHost->loadStop[i];
        }
    }

    return 0;
}

static void
initThresholds (struct lsInfo *info, float loadSched[], float loadStop[])
{
    if (info == NULL) {
        return;
    }

    for ( unsigned int i = 0; i < info->numIndx; i++) {
        if (info->resTable[i].orderType == INCR) {
            loadSched[i] = INFINIT_LOAD;
            loadStop[i] = INFINIT_LOAD;
        }
        else {
            loadSched[i] = -INFINIT_LOAD;
            loadStop[i] = -INFINIT_LOAD;
        }
    }
}


static char *
parseGroups (char *linep, char *filename, size_t *lineNum, char *section, int groupType, int options)
{
    
    char *str       = NULL;
    char *word      = NULL;
    char *myWord    = NULL;
    char *groupName = NULL;
    char *grpSl     = NULL;
    struct group *unixGrp;
    struct groupInfoEnt *gp, *mygp = NULL;

    struct passwd *pw;
    unsigned int len = 0;
    int hasAllOthers = FALSE;
    int checkAll = TRUE;
    char hostName[MAXHOSTNAMELEN];
    char *hostGroup = NULL;
    bool_t hasNone = FALSE;
    int haveFirst = FALSE;
    char returnVal = FALSE;

#define failReturn(mygp, size)  {                                       \
ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", size); \
freeGroupInfo (mygp);                                           \
FREEUP (mygp);                                                  \
FREEUP (hostGroup);                                             \
lsberrno = LSBE_NO_MEM;                                         \
return NULL;}

    if (groupType == USER_GRP) {
        groupName = "User/User";
    }
    else {
        groupName = "Host/Host";
    }

    if (groupType == USER_GRP && numofugroups >= MAX_GROUPS) {
        /* catgets 5245 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5245, "%s: File %s%s at line %d: number of %s group <%d> is equal to or greater than MAX_GROUPS <%d>; ignored the group <%s>"), __func__, filename, section, *lineNum, (groupType == USER_GRP) ? "user" : "host", (groupType == USER_GRP) ? numofugroups : numofhgroups, MAX_GROUPS, linep); 
        return NULL;
    }

    mygp = malloc(sizeof (struct groupInfoEnt));
    if( NULL == mygp && ENOMEM == errno ) {
        failReturn (mygp, sizeof (struct groupInfoEnt));
    }

    initGroupInfo (mygp);
    if (groupType == HOST_GRP) {
        hostGroup = malloc (MAXLINELEN);
        if (NULL == hostGroup && ENOMEM == errno ) {
            failReturn (mygp, MAXLINELEN);
        }
        len = MAXLINELEN;
        checkAll = FALSE;
        hostGroup[0] = '\0';
    }

    while ((word = getNextWord_ (&linep)) != NULL) {
        char *cp;
        char cpWord[MAXHOSTNAMELEN];

        cp = word;
        if (cp[0] == '~') {
            cp++;
        }

        if (options & CONF_NO_EXPAND) {
            myWord = putstr_ (cp);
            strcpy (cpWord, word);
        }
        else {
            myWord = putstr_ (word);
            strcpy (cpWord, "");
        }

        if (myWord == NULL) {
            failReturn (mygp, strlen (word) + 1);
        }

        if (groupType == HOST_GRP && ((options & CONF_NO_EXPAND) == 0)) {
            returnVal = isInGrp (myWord, mygp, groupType, checkAll);
        }
        else {
            returnVal = isInGrp (cpWord, mygp, groupType, checkAll);
        }

        if (returnVal) {
            /* catgets 5246 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5246, "%s: File %s%s at line %d: %s group <%s> is multiply defined; ignored"), __func__, filename, section, *lineNum, groupName, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            continue;
        }

        if (groupType == USER_GRP) {
            size_t lastChar = 0;

            if (options == CONF_NO_CHECK) {
                if (!addMember (mygp, myWord, USER_GRP, filename, *lineNum, section, options, checkAll) && lsberrno == LSBE_NO_MEM) {

                    failReturn (mygp, strlen (myWord) + 1);
                }
                FREEUP (myWord);
                continue;
            }
            pw = getpwlsfuser_ (myWord);
            if (pw != NULL) {
                if (!addMember (mygp, myWord, USER_GRP, filename, *lineNum, section, options, checkAll) && lsberrno == LSBE_NO_MEM) {
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (myWord);
                    return NULL;
                }
                FREEUP (myWord);
                continue;
            }
            FREEUP (grpSl);
            lastChar = strlen (myWord) - 1;
            if (lastChar > 0 && myWord[lastChar] == '/') {
                grpSl = putstr_ (myWord);
                if (grpSl == NULL) {
                    failReturn (mygp, strlen (myWord) + 1);
                }
                grpSl[lastChar] = '\0';
            }

            gp = getUGrpData (myWord);
            if (gp != NULL) {
                if (!addMember (mygp, myWord, USER_GRP, filename, *lineNum, section, options, checkAll) && lsberrno == LSBE_NO_MEM) {
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (myWord);
                    return NULL;
                }
                FREEUP (myWord);
                continue;
            }

            if (grpSl) {
                unixGrp = mygetgrnam (grpSl);

                grpSl[lastChar] = '/';
            }
            else {
                unixGrp = mygetgrnam (myWord);
            }
            if (unixGrp != NULL) {
                if (options & CONF_EXPAND) {
                    assert( *lineNum <= INT_MAX );
                    gp = addUnixGrp (unixGrp, grpSl, filename, *lineNum, section, 0);
                    if (gp == NULL) {
                        /* catgets 5247 */
                        ls_syslog (LOG_WARNING, I18N (5247, "%s: File %s at line %d: No valid users defined in Unix group <%s>; ignoring"), __func__, filename, *lineNum, myWord);
                        lsberrno = LSBE_CONF_WARNING;
                        FREEUP (myWord);
                        continue;
                    }
                }

                if (!addMember (mygp, myWord, USER_GRP, filename, *lineNum, section, options, checkAll) && lsberrno == LSBE_NO_MEM) {
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (myWord);
                    return NULL;
                }
            }
            else if (strcmp (myWord, "all") == 0) {
                if (!addMember (mygp, myWord, USER_GRP, filename, *lineNum, section, options, checkAll) && lsberrno == LSBE_NO_MEM) {
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (myWord);
                    return NULL;
                }
            }
            else {
                /* catgets 5248 */
                ls_syslog (LOG_WARNING, I18N (5248, "%s: File %s, section %s at line %d: Unknown user or user group <%s>; Maybe a windows user or of another domain"), __func__, filename, section, *lineNum, myWord);
                lsberrno = LSBE_CONF_WARNING;
                if (!addMember (mygp, myWord, USER_GRP, filename, *lineNum, section, options, checkAll) && lsberrno == LSBE_NO_MEM) {
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (myWord);
                    return NULL;
                }
            }

            FREEUP (myWord);
            continue;
        }
        else {
            if (groupType == HOST_GRP) {
                size_t length = 0;
                int badPref = FALSE;
                char *sp;

                length = strlen (myWord);
                strcpy (hostName, myWord);


                if (length > 2 && myWord[length - 1] >= '0' && myWord[length - 1] <= '9' && myWord[length - 2] == '+') {
                    if (myWord[length - 3] == '+') {
                        /* catgets 5251 */
                        ls_syslog (LOG_ERR, I18N (5251, "%s: File %s section %s at line %d: Host <%s> is specified with bad host preference expression; ignored"), __func__, filename, section, *lineNum, hostName);   
                        FREEUP (myWord);
                        continue;
                    }
                    myWord[length - 2] = '\0';
                }
                else if ((sp = strchr (myWord, '+')) != NULL && sp > myWord) {
                    char *cp_;
                    int number = FALSE;
                    badPref = FALSE;
                    cp_ = sp;
                    while (*cp_ != '\0') {
                        if (*cp_ == '+' && !number) {
                            cp_++;
                            continue;
                        }
                        else if ((*cp_ >= '0') && (*cp_ <= '9')) {
                            cp_++;
                            number = TRUE;
                            continue;
                        }
                        else {
                            /* catgets 5252 */
                            ls_syslog (LOG_ERR, I18N (5252, "%s: File %s%s at line %d: Host <%s> is specified with bad host preference expression; ignored"), __func__, filename, section, *lineNum, hostName);   
                            FREEUP (myWord);
                            badPref = TRUE;
                            break;
                        }
                    }
                    if (badPref == TRUE) {
                        continue;
                    }
                    *sp = '\0';
                }
                else {
                    char *function_name = malloc( strlen(filename)  + 1);
                    strcpy( function_name, filename );
                    if (parseQFirstHost(myWord, &haveFirst, function_name, lineNum, filename, section)) {
                        free( function_name );
                        continue;
                    }
                    free( function_name );
                }

                if ((options & CONF_NO_EXPAND) != 0) {
                    length = strlen (cpWord);
                    strcpy (hostName, cpWord);

                    if (length > 2 && cpWord[length - 1] >= '0' && cpWord[length - 1] <= '9' && cpWord[length - 2] == '+') {
                        if (cpWord[length - 3] == '+') {
                            continue;
                        }
                        cpWord[length - 2] = '\0';
                    }
                    else if ((sp = strchr (cpWord, '+')) != NULL && sp > cpWord) {
                        char *cp_;
                        int number = FALSE;
                        badPref = FALSE;
                        cp_ = sp;
                        while (*cp_ != '\0') {
                            if (*cp_ == '+' && !number) {
                                cp_++;
                                continue;
                            }
                            else if ((*cp_ >= '0') && (*cp_ <= '9')) {
                                cp_++;
                                number = TRUE;
                                continue;
                            }
                            else {
                                badPref = TRUE;
                                break;
                            }
                        }
                        if (badPref == TRUE) {
                            continue;
                        }
                        *sp = '\0';
                    }
                }

                if (strcmp (myWord, "none") == 0) {
                    hasNone = TRUE;
                    FREEUP (myWord);
                    continue;
                }
                if (!strcmp (myWord, "others")) {
                    if (hasAllOthers == TRUE) {
                        /* catgets 5253 */
                        ls_syslog (LOG_ERR, I18N (5253, "%s: File %s section %s at line %d: More than one <others> or host group with <all> as its member specified; <%s> is ignored"), __func__, filename, section, *lineNum, hostName);  
                        FREEUP (myWord);
                        continue;
                    }

                    if (putIntoList (&hostGroup, &len, hostName, I18N_IN_QUEUE_HOST) == NULL) {
                        failReturn (mygp, strlen (myWord) + 1);
                    }
                    hasAllOthers = TRUE;
                    FREEUP (myWord);
                    continue;
                }
            }

            if (isHostName (myWord) == FALSE) {

                gp = getHGrpData (myWord);
                if (groupType == HOST_GRP && strcmp (myWord, "all") != 0 && checkAllOthers (myWord, &hasAllOthers)) {
                    /* catgets 5255 */
                    ls_syslog (LOG_ERR, I18N (5255, "%s: File %s section %s at line %d: More than one host group with <all> as its members or <others> specified; <%s> is ignored"), __func__, filename, section, *lineNum, hostName); 
                    lsberrno = LSBE_CONF_WARNING;
                    FREEUP (myWord);
                    continue;
                }
                if ((options & CONF_NO_EXPAND) == 0) {
                    returnVal = addMember (mygp, myWord, HOST_GRP, filename, *lineNum, section, options, checkAll);
                }
                else {
                    returnVal = addMember (mygp, cpWord, HOST_GRP, filename, *lineNum, section, options, checkAll);
                }
                if (!returnVal) {
                    FREEUP (myWord);
                    if (lsberrno == LSBE_NO_MEM) {
                        freeGroupInfo (mygp);
                        FREEUP (mygp);
                        FREEUP (hostGroup);
                        return NULL;
                    }
                    continue;
                }

                if (groupType == HOST_GRP && putIntoList (&hostGroup, &len, hostName, I18N_IN_QUEUE_HOST) == NULL) {
                    failReturn (mygp, strlen (hostName) + 1);
                }
                FREEUP (myWord);
                continue;
            }
            else {
                if ((Gethostbyname_ (myWord)) == NULL) {
                    ls_syslog (LOG_ERR, "%s: File %s section %s at line %d: Host name <%s> cannot be found; ignored", __func__, filename, section, *lineNum, myWord);
                    lsberrno = LSBE_CONF_WARNING;
                    FREEUP (myWord);
                    continue;
                }
                if (getHostData (myWord) == NULL && numofhosts != 0) {
                    /* catgets 5257 */
                    ls_syslog (LOG_ERR, I18N (5257, "%s: File %s section %s at line %d: Host <%s> is not used by the batch system; ignored"), __func__, filename, section, *lineNum, myWord);  
                    lsberrno = LSBE_CONF_WARNING;
                    FREEUP (myWord);
                    continue;
                }

                if (isServerHost (myWord) == FALSE) {
                    /* catgets 5258 */
                    ls_syslog (LOG_ERR, I18N (5258, "%s: File %s section %s at line %d: Host <%s> is not a server; ignored"), __func__, filename, section, *lineNum, myWord);  
                    lsberrno = LSBE_CONF_WARNING;
                    FREEUP (myWord);
                    continue;
                }

                if ((options & CONF_NO_EXPAND) == 0) {
                    returnVal = isInGrp (myWord, mygp, groupType, checkAll);
                }
                else {
                    returnVal = isInGrp (cpWord, mygp, groupType, checkAll);
                }
                if (returnVal) {
                    ls_syslog (LOG_ERR, I18N (5259, "%s: File %s section %s at line %d: Host name <%s> is multiply defined; ignored"), __func__, filename, section, *lineNum, myWord); /* catgets 5259 */
                    lsberrno = LSBE_CONF_WARNING;
                    FREEUP (myWord);
                    continue;
                }
                if ((options & CONF_NO_EXPAND) == 0) {
                    returnVal = addMember (mygp, myWord, HOST_GRP, filename, *lineNum, section, options, checkAll);
                }
                else {
                    if( strchr (cpWord, '+')) {
                        returnVal = addMember (mygp, myWord, HOST_GRP, filename, *lineNum, section, options, checkAll);
                    }
                    else {
                        returnVal = addMember (mygp, cpWord, HOST_GRP, filename, *lineNum, section, options, checkAll);
                    }
                }

                if (!returnVal) {
                    FREEUP (myWord);
                    if (lsberrno == LSBE_NO_MEM) {
                        freeGroupInfo (mygp);
                        FREEUP (mygp);
                        return NULL;
                    }
                    continue;
                }

                if (groupType == HOST_GRP && putIntoList (&hostGroup, &len, hostName, I18N_IN_QUEUE_HOST) == NULL) {
                    failReturn (mygp, strlen (myWord) + 1);
                }
                FREEUP (myWord);
                continue;
            }
        }
    }
    FREEUP (grpSl);

    if (hasNone == FALSE || groupType != HOST_GRP)
        if (mygp->memberList == NULL)
            {
            freeGroupInfo (mygp);
            FREEUP (mygp);
            FREEUP (hostGroup);
            return NULL;
            }

    if (groupType != HOST_GRP) {
        if ((str = putstr_ (mygp->memberList)) == NULL) {
            failReturn (mygp, strlen (mygp->memberList) + 1);
        }
        freeGroupInfo (mygp);
        FREEUP (mygp);
        return str;
    }
    else {
        freeGroupInfo (mygp);
        FREEUP (mygp);
        if (hasNone) {
            if (hostGroup[0] != '\000') {
                /* catgets 5260 */
                ls_syslog (LOG_ERR, I18N (5260, "%s: file <%s> line <%d> host names <%s> should not be specified together with \"none\", ignored"), __func__, filename, *lineNum, hostGroup);
                lsberrno = LSBE_CONF_WARNING;
                }
            strcpy (hostGroup, "none");
            }

        return hostGroup;
    }
}

struct queueConf *
lsb_readqueue (struct lsConf *conf, struct lsInfo *info, int options, struct sharedConf *sharedConf)
{
    
    struct lsInfo myinfo;
    char *filename = NULL;
    char *cp = NULL;
    char *section = NULL;
    char queueok;
    size_t *lineNum = 0;
    int j = 0;

    lsberrno = LSBE_NO_ERROR;

    if (info == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "lsinfo");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if ((options != CONF_NO_CHECK) && sharedConf == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "sharedConf");
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if ((options != CONF_NO_CHECK) && uConf == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "uConf");
        if (setDefaultUser ()) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "setDefaultUser");
            lsberrno = LSBE_CONF_FATAL;
            return NULL;
        }
    }

    if ((options != CONF_NO_CHECK) && hConf == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, "hConf");
        if (setDefaultHost (info)) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "setDefaultHost");
            lsberrno = LSBE_CONF_FATAL;
            return NULL;
        }
    }

    myinfo = *info;
    sConf = sharedConf;
    assert( info->nRes >= 0);
    myinfo.resTable = calloc( info->nRes, sizeof (struct resItem) );
    if (info->nRes && NULL == myinfo.resTable && ENOMEM == errno ) {
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    j = 0;
    for ( unsigned int i = 0; i < info->nRes; i++) {

        if (info->resTable[i].flags & RESF_DYNAMIC) {
            memcpy (&myinfo.resTable[j], &info->resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    for ( unsigned int i = 0; i < info->nRes; i++) {
        if (!(info->resTable[i].flags & RESF_DYNAMIC)) {
            memcpy (&myinfo.resTable[j], &info->resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    if (!conf) {
        FREEUP (myinfo.resTable);
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (!conf->confhandle) {
        FREEUP (myinfo.resTable);
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (qConf != NULL) {
        freeQConf (qConf, TRUE);
    }
    else {
        if ((qConf = malloc (sizeof (struct queueConf))) == NULL) {
            lsberrno = LSBE_CONF_FATAL;
            return NULL;
        }
        qConf->numQueues = 0;
        qConf->queues = NULL;
    }

    freeWorkQueue (FALSE);
    queuesize = 0;
    maxFactor = 0.0;
    maxHName = NULL;
    filename = conf->confhandle->fname;
    conf->confhandle->curNode = conf->confhandle->rootNode;
    conf->confhandle->lineCount = 0;
    queueok = FALSE;

    for (;;) {

        if ((cp = getBeginLine_conf (conf, lineNum)) == NULL) {
            if (!queueok) {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5271, "%s: Queue section missing or invalid."), filename);    /* catgets 5271 */
                lsberrno = LSBE_CONF_WARNING;
            }

            if (numofqueues){

                qConf->queues = calloc (numofqueues, sizeof (struct queueInfoEnt));
                if ( NULL == qConf->queues && ENOMEM == errno ) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkQueue (TRUE);
                    qConf->numQueues = 0;
                    FREEUP (myinfo.resTable);
                    return NULL;
                }

                for( unsigned int i = 0; i < numofqueues; i++) {
                    initQueueInfo (&qConf->queues[i]);
                    qConf->queues[i] = *queues[i];
                }
                qConf->numQueues = numofqueues;
            }

            FREEUP (myinfo.resTable);
            return qConf;
        }

        section = getNextWord_ (&cp);
        if (!section) {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5273, "%s: File %s at line %d: Section name expected after Begin; ignoring section"), __func__, filename, lineNum);  /* catgets 5273 */
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, "unknown");
        }
        else {

            if (strcasecmp (section, "Queue") == 0) {
                if (do_Queues (conf, filename, lineNum, &myinfo, options)) {
                    queueok = TRUE;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkQueue (TRUE);
                    freeQConf (qConf, FALSE);
                    FREEUP (myinfo.resTable);
                    return NULL;
                }
                else {
                    lsberrno = LSBE_CONF_WARNING;
                }
                continue;
            }
            else {
                ls_syslog (LOG_ERR, I18N (5274, "%s: File %s at line %d: Invalid section name <%s>; ignoring section"), __func__, filename, lineNum, section);    /* catgets 5274 */
                lsberrno = LSBE_CONF_WARNING;
                doSkipSection_conf (conf, lineNum, filename, section);
            }
        }
    }
}

static char
do_Queues (struct lsConf *conf, char *filename, size_t *lineNum, struct lsInfo *info, int options)
{

#define QKEY_NAME                   info->numIndx + 0
#define QKEY_PRIORITY               info->numIndx + 1
#define QKEY_NICE                   info->numIndx + 2
#define QKEY_UJOB_LIMIT             info->numIndx + 3
#define QKEY_PJOB_LIMIT             info->numIndx + 4
#define QKEY_RUN_WINDOW             info->numIndx + 5
#define QKEY_CPULIMIT               info->numIndx + 6
#define QKEY_FILELIMIT              info->numIndx + 7
#define QKEY_DATALIMIT              info->numIndx + 8
#define QKEY_STACKLIMIT             info->numIndx + 9
#define QKEY_CORELIMIT              info->numIndx + 10
#define QKEY_MEMLIMIT               info->numIndx + 11
#define QKEY_RUNLIMIT               info->numIndx + 12
#define QKEY_USERS                  info->numIndx + 13
#define QKEY_HOSTS                  info->numIndx + 14
#define QKEY_EXCLUSIVE              info->numIndx + 15
#define QKEY_DESCRIPTION            info->numIndx + 16
#define QKEY_MIG                    info->numIndx + 17
#define QKEY_QJOB_LIMIT             info->numIndx + 18
#define QKEY_POLICIES               info->numIndx + 19
#define QKEY_DISPATCH_WINDOW        info->numIndx + 20
#define QKEY_USER_SHARES            info->numIndx + 21
#define QKEY_DEFAULT_HOST_SPEC      info->numIndx + 22
#define QKEY_PROCLIMIT              info->numIndx + 23
#define QKEY_ADMINISTRATORS         info->numIndx + 24
#define QKEY_PRE_EXEC               info->numIndx + 25
#define QKEY_POST_EXEC              info->numIndx + 26
#define QKEY_REQUEUE_EXIT_VALUES    info->numIndx + 27
#define QKEY_HJOB_LIMIT             info->numIndx + 28
#define QKEY_RES_REQ                info->numIndx + 29
#define QKEY_SLOT_RESERVE           info->numIndx + 30
#define QKEY_RESUME_COND            info->numIndx + 31
#define QKEY_STOP_COND              info->numIndx + 32
#define QKEY_JOB_STARTER            info->numIndx + 33
#define QKEY_SWAPLIMIT              info->numIndx + 34
#define QKEY_PROCESSLIMIT           info->numIndx + 35
#define QKEY_JOB_CONTROLS           info->numIndx + 36
#define QKEY_TERMINATE_WHEN         info->numIndx + 37
#define QKEY_NEW_JOB_SCHED_DELAY    info->numIndx + 38
#define QKEY_INTERACTIVE            info->numIndx + 39
#define QKEY_JOB_ACCEPT_INTERVAL    info->numIndx + 40
#define QKEY_BACKFILL               info->numIndx + 41
#define QKEY_IGNORE_DEADLINE        info->numIndx + 42
#define QKEY_CHKPNT                 info->numIndx + 43
#define QKEY_RERUNNABLE             info->numIndx + 44
#define QKEY_ENQUE_INTERACTIVE_AHEAD info->numIndx +45
#define QKEY_ROUND_ROBIN_POLICY     info->numIndx + 46
#define QKEY_PRE_POST_EXEC_USER     info->numIndx + 47
#define KEYMAP_SIZE                 info->numIndx + 49

    
    static struct keymap *keylist;
    struct queueInfoEnt queue;
    char *linep = NULL;
    char *sp    = NULL;
    char *word  = NULL;
    int retval  = 0;
    char *originalString = NULL;
    char *subString      = NULL;

    if (conf == NULL) {
        return FALSE;
    }

    FREEUP (keylist);
    assert( KEYMAP_SIZE >= 0 );
    keylist = calloc( KEYMAP_SIZE, sizeof (struct keymap));
    if( NULL == keylist && ENOMEM == errno ) {
        return FALSE;
    }

    assert( QKEY_NAME + 1 <= INT_MAX );
    assert( KEYMAP_SIZE <= INT_MAX );
    initkeylist (keylist, (int)(QKEY_NAME + 1), (int)KEYMAP_SIZE, info);

    keylist[QKEY_NAME].key                    = "QUEUE_NAME";
    keylist[QKEY_PRIORITY].key                = "PRIORITY";
    keylist[QKEY_NICE].key                    = "NICE";
    keylist[QKEY_UJOB_LIMIT].key              = "UJOB_LIMIT";
    keylist[QKEY_PJOB_LIMIT].key              = "PJOB_LIMIT";
    keylist[QKEY_RUN_WINDOW].key              = "RUN_WINDOW";
    keylist[QKEY_CPULIMIT].key                = "CPULIMIT";
    keylist[QKEY_FILELIMIT].key               = "FILELIMIT";
    keylist[QKEY_DATALIMIT].key               = "DATALIMIT";
    keylist[QKEY_STACKLIMIT].key              = "STACKLIMIT";
    keylist[QKEY_CORELIMIT].key               = "CORELIMIT";
    keylist[QKEY_MEMLIMIT].key                = "MEMLIMIT";
    keylist[QKEY_RUNLIMIT].key                = "RUNLIMIT";
    keylist[QKEY_USERS].key                   = "USERS";
    keylist[QKEY_HOSTS].key                   = "HOSTS";
    keylist[QKEY_EXCLUSIVE].key               = "EXCLUSIVE";
    keylist[QKEY_DESCRIPTION].key             = "DESCRIPTION";
    keylist[QKEY_MIG].key                     = "MIG";
    keylist[QKEY_QJOB_LIMIT].key              = "QJOB_LIMIT";
    keylist[QKEY_POLICIES].key                = "POLICIES";
    keylist[QKEY_DISPATCH_WINDOW].key         = "DISPATCH_WINDOW";
    keylist[QKEY_USER_SHARES].key             = "USER_SHARES";
    keylist[QKEY_DEFAULT_HOST_SPEC].key       = "DEFAULT_HOST_SPEC";
    keylist[QKEY_PROCLIMIT].key               = "PROCLIMIT";
    keylist[QKEY_ADMINISTRATORS].key          = "ADMINISTRATORS";
    keylist[QKEY_PRE_EXEC].key                = "PRE_EXEC";
    keylist[QKEY_POST_EXEC].key               = "POST_EXEC";
    keylist[QKEY_REQUEUE_EXIT_VALUES].key     = "REQUEUE_EXIT_VALUES";
    keylist[QKEY_HJOB_LIMIT].key              = "HJOB_LIMIT";
    keylist[QKEY_RES_REQ].key                 = "RES_REQ";
    keylist[QKEY_SLOT_RESERVE].key            = "SLOT_RESERVE";
    keylist[QKEY_RESUME_COND].key             = "RESUME_COND";
    keylist[QKEY_STOP_COND].key               = "STOP_COND";
    keylist[QKEY_JOB_STARTER].key             = "JOB_STARTER";
    keylist[QKEY_SWAPLIMIT].key               = "SWAPLIMIT";
    keylist[QKEY_PROCESSLIMIT].key            = "PROCESSLIMIT";
    keylist[QKEY_JOB_CONTROLS].key            = "JOB_CONTROLS";
    keylist[QKEY_TERMINATE_WHEN].key          = "TERMINATE_WHEN";
    keylist[QKEY_NEW_JOB_SCHED_DELAY].key     = "NEW_JOB_SCHED_DELAY";
    keylist[QKEY_INTERACTIVE].key             = "INTERACTIVE";
    keylist[QKEY_JOB_ACCEPT_INTERVAL].key     = "JOB_ACCEPT_INTERVAL";
    keylist[QKEY_BACKFILL].key                = "BACKFILL";
    keylist[QKEY_IGNORE_DEADLINE].key         = "IGNORE_DEADLINE";
    keylist[QKEY_CHKPNT].key                  = "CHKPNT";
    keylist[QKEY_RERUNNABLE].key              = "RERUNNABLE";
    keylist[QKEY_ENQUE_INTERACTIVE_AHEAD].key = "ENQUE_INTERACTIVE_AHEAD";
    keylist[QKEY_ROUND_ROBIN_POLICY].key      = "ROUND_ROBIN_POLICY";
    keylist[QKEY_PRE_POST_EXEC_USER].key      = "PRE_POST_EXEC_USER";
    keylist[KEYMAP_SIZE - 1].key              = NULL;

    initQueueInfo (&queue);

    linep = getNextLineC_conf (conf, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, "Queue")) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, (size_t) *lineNum,  "queue");
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
    }

    if (strchr (linep, '=') == NULL) {
        /* catgets 5277 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5277, "%s: File %s at line %d: Vertical Queue section not implented yet; use horizontal format; ignoring section"), __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, "Queue");
        return FALSE;
    }
    else {
        char *function_name = malloc( strlen(filename )  + 1 );
        strcpy( function_name, filename );
        retval = readHvalues_conf (keylist, linep, conf, filename, lineNum, FALSE, "Queue");
        if (retval < 0) {
            if (retval == -2) {
                lsberrno = LSBE_CONF_WARNING;
                /* catgets 5463 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5463, "%s: Parameter error in %s(%d); remaining parameters in this section will be either ignored or set to default values."), __func__, filename, *lineNum);
            }
            else {
                /* catgets 5278 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5278, "%s: File %s at line %d: Incorrect section; ignoring this Queue section"), __func__, filename, *lineNum);
                lsberrno = LSBE_CONF_WARNING;
                freekeyval (keylist);
                return FALSE;
            }
        }

        if (keylist[QKEY_NAME].val == NULL) {
            /* catgets 5279 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5279, "%s: File %s in section Queue ending at line %d: Queue name is not given; ignoring section"), __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keylist);
            return FALSE;
        }

        if (strcmp (keylist[QKEY_NAME].val, "default") == 0) {
            /* catgets 5280 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5280, "%s: File %s in section Queue ending at line %d: Queue name <%s> is a reserved word; ignoring the queue section"), __func__, filename, *lineNum, keylist[QKEY_NAME].val);
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keylist);
            return FALSE;
        }

        if (getQueueData (keylist[QKEY_NAME].val)) {
            /* catgets 5281 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5281, "%s: File %s in section Queue ending at line %d: Duplicate queue name <%s>; ignoring section"), __func__, filename, *lineNum, keylist[QKEY_NAME].val);
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keylist);
            return FALSE;
        }

        queue.queue = putstr_ (keylist[QKEY_NAME].val);
        if (queue.queue == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_NAME].val) + 1);
            lsberrno = LSBE_NO_MEM;
            freekeyval (keylist);
            return FALSE;
        }

        if (keylist[QKEY_PRIORITY].val != NULL && strcmp (keylist[QKEY_PRIORITY].val, "")) {
            queue.priority = my_atoi (keylist[QKEY_PRIORITY].val, INFINIT_INT, 0);
            if (INFINIT_INT == queue.priority ) {
                /* catgets 5284 */
                ls_syslog (LOG_ERR, I18N (5284, "%s: File %s in section Queue ending at line %d: Priority value <%s> isn't a positive integer between 1 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_PRIORITY].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
            }
        }
        else {
            queue.priority = 1;
        }

        if (keylist[QKEY_NICE].val != NULL && strcmp (keylist[QKEY_NICE].val, "")) {
            if (my_atoi (keylist[QKEY_NICE].val, INFINIT_SHORT, -INFINIT_SHORT) == INFINIT_INT) {
                /* catgets 5285 */
                ls_syslog (LOG_ERR, I18N (5285, "%s: File %s in section Queue ending at line %d: Nice value <%s> must be an integer; ignored"), __func__, filename, *lineNum, keylist[QKEY_NICE].val);
                lsberrno = LSBE_CONF_WARNING;
            }
            else
                queue.nice = my_atoi (keylist[QKEY_NICE].val, INFINIT_SHORT, -INFINIT_SHORT);
            }


        if (keylist[QKEY_UJOB_LIMIT].val != NULL && strcmp (keylist[QKEY_UJOB_LIMIT].val, "")) {
            queue.userJobLimit = my_atoi (keylist[QKEY_UJOB_LIMIT].val, INFINIT_INT, -1);
            if ( INFINIT_INT == queue.userJobLimit ) {
                /* catgets 5286 */
                ls_syslog (LOG_ERR, I18N (5286, "%s: File %s in section Queue ending at line %d: UJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_UJOB_LIMIT].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
                }
            }


        if (keylist[QKEY_PJOB_LIMIT].val != NULL && strcmp (keylist[QKEY_PJOB_LIMIT].val, "")) {
            queue.procJobLimit = my_atof (keylist[QKEY_PJOB_LIMIT].val, INFINIT_FLOAT, -1.0);
            if( fabs (INFINIT_FLOAT - queue.procJobLimit ) < 0.0000001 ) {
                /* catgets 5287 */
                ls_syslog (LOG_ERR, I18N (5287, "%s: File %s in section Queue ending at line %d: PJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %f; ignored"), __func__, filename, *lineNum, keylist[QKEY_PJOB_LIMIT].val, INFINIT_FLOAT - 1);
                lsberrno = LSBE_CONF_WARNING;
            }
        }


        if (keylist[QKEY_QJOB_LIMIT].val != NULL
            && strcmp (keylist[QKEY_QJOB_LIMIT].val, ""))
            {
            if ((queue.maxJobs = my_atoi (keylist[QKEY_QJOB_LIMIT].val, INFINIT_INT, -1)) == INFINIT_INT)
                {
                /* catgets 5289 */
                ls_syslog (LOG_ERR, I18N (5289, "%s: File %s in section Queue ending at line %d: QJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_QJOB_LIMIT].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
                }
            }

        if (keylist[QKEY_HJOB_LIMIT].val != NULL && strcmp (keylist[QKEY_HJOB_LIMIT].val, "")) {
            if ((queue.hostJobLimit = my_atoi (keylist[QKEY_HJOB_LIMIT].val, INFINIT_INT, -1)) == INFINIT_INT) {
                /* catgets 5290 */
                ls_syslog (LOG_ERR, I18N (5290, "%s: File %s in section Queue ending at line %d: HJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_HJOB_LIMIT].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_RUN_WINDOW].val != NULL && strcmp (keylist[QKEY_RUN_WINDOW].val, "")) {
            queue.windows = parsewindow (keylist[QKEY_RUN_WINDOW].val, filename, lineNum, "Queue");

            if (lserrno == LSE_CONF_SYNTAX) {
                lserrno = LSE_NO_ERR;
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_DISPATCH_WINDOW].val != NULL && strcmp (keylist[QKEY_DISPATCH_WINDOW].val, "")) {
            queue.windowsD = parsewindow (keylist[QKEY_DISPATCH_WINDOW].val, filename, lineNum, "Queue");

            if (lserrno == LSE_CONF_SYNTAX) {
                lserrno = LSE_NO_ERR;
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_DEFAULT_HOST_SPEC].val != NULL
            && strcmp (keylist[QKEY_DEFAULT_HOST_SPEC].val, ""))
            {
            double *cpuFactor = NULL;

            if (options != CONF_NO_CHECK)
                {
                if ((cpuFactor = getModelFactor
                     (keylist[QKEY_DEFAULT_HOST_SPEC].val, info)) == NULL &&
                    (cpuFactor = getHostFactor
                     (keylist[QKEY_DEFAULT_HOST_SPEC].val)) == NULL)
                    {
                    /* catgets 5292 */
                    ls_syslog (LOG_ERR, I18N (5292, "%s: File %s in section Queue ending at line %d: Invalid value <%s> for %s; ignored"), __func__, filename, *lineNum, keylist[QKEY_DEFAULT_HOST_SPEC].val, keylist[QKEY_DEFAULT_HOST_SPEC].key);
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            if (cpuFactor != NULL || options == CONF_NO_CHECK)
                {
                queue.defaultHostSpec =
                putstr_ (keylist[QKEY_DEFAULT_HOST_SPEC].val);
                if (queue.defaultHostSpec == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_DEFAULT_HOST_SPEC].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                    }
                }
            }

        if (parseCpuAndRunLimit (keylist, &queue, filename, lineNum, function_name, info, options) == FALSE && lsberrno == LSBE_NO_MEM) {
            freekeyval (keylist);
            freeQueueInfo (&queue);
            return FALSE;
        }

        if (keylist[QKEY_FILELIMIT].val != NULL && strcmp (keylist[QKEY_FILELIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_FSIZE] = my_atoi (keylist[QKEY_FILELIMIT].val, INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5295 */
                ls_syslog (LOG_ERR, I18N (5295, "%s: File %s in section Queue ending at line %d: FILELIMIT value <%s> isn't a positive integer between 0 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_FILELIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_DATALIMIT].val != NULL && strcmp (keylist[QKEY_DATALIMIT].val, "")) {
            parseDefAndMaxLimits (keylist[QKEY_DATALIMIT], &queue.defLimits[LSF_RLIMIT_DATA], &queue.rLimits[LSF_RLIMIT_DATA], filename, lineNum, function_name );
        }

        if (keylist[QKEY_STACKLIMIT].val != NULL && strcmp (keylist[QKEY_STACKLIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_STACK] = my_atoi (keylist[QKEY_STACKLIMIT].val,INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5297 */
                ls_syslog (LOG_ERR, I18N (5297, "%s: File %s in section Queue ending at line %d: STACKLIMIT value <%s> isn't a positive integer between 0 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_STACKLIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_CORELIMIT].val != NULL && strcmp (keylist[QKEY_CORELIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_CORE] = my_atoi (keylist[QKEY_CORELIMIT].val, INFINIT_INT, -1)) == INFINIT_INT) {
                /* catgets 5298 */
                ls_syslog (LOG_ERR, I18N (5298, "%s: File %s in section Queue ending at line %d: CORELIMIT value <%s> isn't a non-negative integer between -1 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_CORELIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_MEMLIMIT].val != NULL && strcmp (keylist[QKEY_MEMLIMIT].val, "")) {
            parseDefAndMaxLimits (keylist[QKEY_MEMLIMIT], &queue.defLimits[LSF_RLIMIT_RSS], &queue.rLimits[LSF_RLIMIT_RSS], filename, lineNum, function_name);
        }


        if (keylist[QKEY_SWAPLIMIT].val != NULL && strcmp (keylist[QKEY_SWAPLIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_SWAP] = my_atoi (keylist[QKEY_SWAPLIMIT].val, INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5300 */
                ls_syslog (LOG_ERR, I18N (5300, "%s: File %s in section Queue ending at line %d: SWAPLIMIT value <%s> isn't a positive integer between 0 and %d; ignored"), __func__, filename, *lineNum, keylist[QKEY_SWAPLIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_PROCESSLIMIT].val != NULL && strcmp (keylist[QKEY_PROCESSLIMIT].val, "")) {
            parseDefAndMaxLimits (keylist[QKEY_PROCESSLIMIT], &queue.defLimits[LSF_RLIMIT_PROCESS], &queue.rLimits[LSF_RLIMIT_PROCESS], filename, lineNum, function_name);
        }


        if (keylist[QKEY_PROCLIMIT].val != NULL && strcmp (keylist[QKEY_PROCLIMIT].val, "")) {
            if (parseProcLimit(keylist[QKEY_PROCLIMIT].val, &queue, filename, lineNum, function_name) == FALSE) {
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keylist[QKEY_USERS].val != NULL && !searchAll (keylist[QKEY_USERS].val)) {

            assert( *lineNum <= UINT_MAX );
            queue.userList = parseGroups (keylist[QKEY_USERS].val, filename, lineNum, " in section  Queue ending", USER_GRP, options);
            if (queue.userList == NULL) {
                if (lsberrno == LSBE_NO_MEM) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, filename, "parseGroups");
                }
                else if (numofugroups >= MAX_GROUPS) {
                    /* catgets 5304 */
                    ls_syslog (LOG_ERR, I18N (5304, "%s: File %s in section Queue ending at line %d:  Number of user group <%d> is equal to or greater than MAX_GROUPS <%d>; ignoring the queue for <%s>; ignoring the queue"), __func__, filename, *lineNum, numofugroups, MAX_GROUPS, queue.queue);
                }
                else {
                    /* catgets 5305 */
                    ls_syslog (LOG_ERR, I18N (5305, "%s: File %s in section Queue ending at line %d: No valid user or user group specified in USERS for <%s>; ignoring the queue"), __func__, filename, *lineNum, queue.queue);   
                }
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
        }

        if (keylist[QKEY_HOSTS].val != NULL) {

            originalString = keylist[QKEY_HOSTS].val;
            subString = getNextWord_ (&originalString);
            while (subString != NULL) {
                if (strcmp (keylist[QKEY_HOSTS].val, "none") == 0) {
                    /* catgets 5307 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5307, "%s: File %s in section Queue at line %d: \"none\" specified , queue ignored"), __func__, filename, *lineNum);
                    lsberrno = LSBE_CONF_WARNING;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                }
                subString = getNextWord_ (&originalString);
            }

            if (strchr (keylist[QKEY_HOSTS].val, '~') != NULL  && ((options & CONF_NO_EXPAND) == 0)) {
                char *outHosts = NULL;
                int numHosts = 0;

                ls_syslog (LOG_DEBUG, "resolveBatchNegHosts: for do_Queues the string is \'%s\'", keylist[QKEY_HOSTS].val);
                numHosts = resolveBatchNegHosts (keylist[QKEY_HOSTS].val, &outHosts, TRUE);
                if (numHosts > 0) {
                    ls_syslog (LOG_DEBUG, "resolveBatchNegHosts: for do_Queues the string is replaced with \'%s\'", outHosts);
                }
                else if (numHosts == 0) {
                    ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5460, "%s: File %s at line %d: there are no hosts found to exclude, replaced with \'%s\'"), __func__, filename, *lineNum, outHosts); /* catgets 5460 */
                }
                else {
                    if (numHosts == -3) {
                        ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5461, "%s: \'%s\' The result is that all the hosts are to be excluded."), filename, keylist[QKEY_HOSTS].val); /* catgets 5461 */
                    }
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5310, "%s: File %s in section Queue ending at line %d: No valid hosts or host group specified in HOSTS for <%s>; ignoring the queue"), __func__, filename, *lineNum, queue.queue);   /* catgets 5310 */

                    lsberrno = LSBE_CONF_WARNING;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                }
                free (keylist[QKEY_HOSTS].val);
                keylist[QKEY_HOSTS].val = outHosts;
            }

            assert( *lineNum < UINT_MAX );
            queue.hostList = parseGroups (keylist[QKEY_HOSTS].val, filename, lineNum, " in section  Queue ending", HOST_GRP, options);
            if (queue.hostList == NULL) {
                if (lsberrno == LSBE_NO_MEM) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, filename, "parseGroups");
                }
                else {
                    /* catgets 5310 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5310, "%s: File %s in section Queue ending at line %d: No valid hosts or host group specified in HOSTS for <%s>; ignoring the queue"), __func__, filename, *lineNum, queue.queue);
                }
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
        }

        if (keylist[QKEY_CHKPNT].val != NULL
            && strcmp (keylist[QKEY_CHKPNT].val, ""))
            {
            if (strlen (keylist[QKEY_CHKPNT].val) >= MAXLINELEN)
                {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5439, "%s: File %s in section Queue ending at line %d: CHKPNT of the queue <%s> is too long <%s>; ignoring"), __func__, filename, *lineNum, queue.queue, keylist[QKEY_CHKPNT].val);  /* catgets 5439 */
                lsberrno = LSBE_CONF_WARNING;
                }
            else
                {

                int chkpntPrd = 0;
                char dir[MAX_CMD_DESC_LEN];
                char prdstr[10];

                memset (prdstr, 0, 10);
                memset (dir, 0, MAX_CMD_DESC_LEN);

                sscanf (keylist[QKEY_CHKPNT].val, "%s %s", dir, prdstr);
                queue.chkpntDir = putstr_ (dir);
                chkpntPrd = atoi (prdstr);

                if (queue.chkpntDir == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", strlen (keylist[QKEY_CHKPNT].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                    }

                if (chkpntPrd < 0)
                    {

                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5441, "%s: File %s in section Queue ending at line %d:  options for CHKPNT of the queue <%s> is invalid ; ignoring"), __func__, filename, *lineNum, queue.queue);    /* catgets 5441 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5442, "%s: invalid checkpoint period"), __func__);    /* catgets 5442 */
                    lsberrno = LSBE_CONF_WARNING;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                    }
                queue.qAttrib |= QUEUE_ATTRIB_CHKPNT;
                queue.chkpntPeriod = chkpntPrd * 60;
                }
            }

        if (keylist[QKEY_RERUNNABLE].val != NULL)
            {
            if (strcasecmp (keylist[QKEY_RERUNNABLE].val, "y") == 0
                || strcasecmp (keylist[QKEY_RERUNNABLE].val, "yes") == 0)
                {
                queue.qAttrib |= QUEUE_ATTRIB_RERUNNABLE;
                }
            else
                {
                if (strcasecmp (keylist[QKEY_RERUNNABLE].val, "n") != 0 &&
                    strcasecmp (keylist[QKEY_RERUNNABLE].val, "no") != 0)
                    {
                    /* catgets 5445 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5445, "%s: File %s in section Queue ending at line %d:  options for RERUNNABLE of the queue <%s> is not y|yes|n|no; ignoring"), __func__, filename, *lineNum, queue.queue, keylist[QKEY_RERUNNABLE].val);
                    lsberrno = LSBE_CONF_WARNING;
                    }
                queue.qAttrib &= ~QUEUE_ATTRIB_RERUNNABLE;
                }
            }

        addBinaryAttributes (filename, lineNum, &queue, &keylist[QKEY_EXCLUSIVE],               QUEUE_ATTRIB_EXCLUSIVE, "EXCLUSIVE");
        addBinaryAttributes (filename, lineNum, &queue, &keylist[QKEY_BACKFILL],                QUEUE_ATTRIB_BACKFILL, "BACKFILL");
        addBinaryAttributes (filename, lineNum, &queue, &keylist[QKEY_IGNORE_DEADLINE],         QUEUE_ATTRIB_IGNORE_DEADLINE, "IGNORE_DEADLINE");
        addBinaryAttributes (filename, lineNum, &queue, &keylist[QKEY_ENQUE_INTERACTIVE_AHEAD], QUEUE_ATTRIB_ENQUE_INTERACTIVE_AHEAD, "ENQUE_INTERACTIVE_AHEAD");
        addBinaryAttributes (filename, lineNum, &queue, &keylist[QKEY_ROUND_ROBIN_POLICY],      QUEUE_ATTRIB_ROUND_ROBIN, "ROUND_ROBIN_POLICY");

        if (keylist[QKEY_INTERACTIVE].val != NULL)
            {
            if (strcasecmp (keylist[QKEY_INTERACTIVE].val, "n") == 0 ||
                strcasecmp (keylist[QKEY_INTERACTIVE].val, "no") == 0)
                {
                queue.qAttrib |= QUEUE_ATTRIB_NO_INTERACTIVE;
                }
            else if (strcasecmp (keylist[QKEY_INTERACTIVE].val, "only") == 0)
                {
                queue.qAttrib |= QUEUE_ATTRIB_ONLY_INTERACTIVE;
                }
            else if ((strcasecmp (keylist[QKEY_INTERACTIVE].val, "yes") != 0) &&
                     (strcasecmp (keylist[QKEY_INTERACTIVE].val, "y") != 0))
                {
                /* catgets 5311 */
                ls_syslog (LOG_ERR, I18N (5311, " %s: File %s in section Queue ending at line %d: INTERACTIVE value <%s> isn't one of 'Y', 'y', 'N', 'n' or 'ONLY'; ignored"), __func__, filename, *lineNum, keylist[QKEY_INTERACTIVE].val);
                lsberrno = LSBE_CONF_WARNING;
                }
            }

        if (keylist[QKEY_JOB_ACCEPT_INTERVAL].val != NULL && keylist[QKEY_JOB_ACCEPT_INTERVAL].position >= 0 && strcmp (keylist[QKEY_JOB_ACCEPT_INTERVAL].val, ""))
            {
            if ((queue.acceptIntvl = my_atoi (keylist[QKEY_JOB_ACCEPT_INTERVAL].val, INFINIT_INT, -1)) == INFINIT_INT)
                {
                /* catgets 5313 */
                ls_syslog (LOG_ERR, I18N (5313, "%s: File %s in section Queue ending at line %d: JOB_ACCEPT_INTERVAL value <%s> isn't an integer greater than -1; ignored"), __func__, filename, *lineNum, keylist[QKEY_JOB_ACCEPT_INTERVAL].val);    
                lsberrno = LSBE_CONF_WARNING;
                }
            }

        if (keylist[QKEY_NEW_JOB_SCHED_DELAY].val != NULL && keylist[QKEY_NEW_JOB_SCHED_DELAY].position >= 0 && strcmp (keylist[QKEY_NEW_JOB_SCHED_DELAY].val, ""))
            {
            if ((queue.schedDelay = my_atoi (keylist[QKEY_NEW_JOB_SCHED_DELAY].val, INFINIT_INT, -1)) == INFINIT_INT)
                {
                /* catgets 5315 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5315, "%s: File %s in section Queue ending at line %d: NEW_JOB_SCHED_DELAY value <%s> isn't an integer greater than -1; ignored"), __func__, filename, *lineNum, keylist[QKEY_NEW_JOB_SCHED_DELAY].val);
                lsberrno = LSBE_CONF_WARNING;
                }
            }

        if (keylist[QKEY_POLICIES].val != NULL)
            {
            sp = keylist[QKEY_POLICIES].val;
            while ((word = getNextWord_ (&sp)) != NULL)
                {
                if (strcasecmp (word, "EXCLUSIVE") == 0)
                    queue.qAttrib |= QUEUE_ATTRIB_EXCLUSIVE;
                else
                    {
                    ls_syslog (LOG_ERR, I18N (5317, "%s: File %s in section Queue ending at line %d: POLICIES value <%s> unrecognizable; ignored"), __func__, filename, *lineNum, word);  /* catgets 5317 */
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            }

        if (keylist[QKEY_DESCRIPTION].val != NULL)
            {
            if (strlen (keylist[QKEY_DESCRIPTION].val) > 10 * MAXLINELEN)
                {
                ls_syslog (LOG_ERR, I18N (5338, "%s: File %s in section Queue ending at line %d: Too many characters in DESCRIPTION of the queue; truncated"), __func__, filename, *lineNum); /* catgets 5338 */
                lsberrno = LSBE_CONF_WARNING;
                keylist[QKEY_DESCRIPTION].val[10 * MAXLINELEN - 1] = '\0';
                }
            queue.description = putstr_ (keylist[QKEY_DESCRIPTION].val);
            if (queue.description == NULL)
                {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_DESCRIPTION].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
                }
            }

        if (keylist[QKEY_MIG].val != NULL && strcmp (keylist[QKEY_MIG].val, ""))
            {

            if ((queue.mig = my_atoi (keylist[QKEY_MIG].val, INFINIT_INT / 60, -1)) == INFINIT_INT)
                {
                /* catgets 5340 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5340, "%s: File %s in section Queue ending at line %d: Invalid value <%s> for MIG; no MIG threshold is assumed"), __func__, filename, *lineNum, keylist[QKEY_MIG].val);
                lsberrno = LSBE_CONF_WARNING;
                }
            }

        if (keylist[QKEY_ADMINISTRATORS].val != NULL && strcmp (keylist[QKEY_ADMINISTRATORS].val, "")) {
            if (options & CONF_NO_CHECK) { 
                queue.admins = parseAdmins (keylist[QKEY_ADMINISTRATORS].val, options, filename, lineNum); // FIXME fix above line
            }
            else {
                queue.admins = putstr_ (keylist[QKEY_ADMINISTRATORS].val);
            }
            if (queue.admins == NULL)  {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", strlen (keylist[QKEY_ADMINISTRATORS].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
            if (queue.admins[0] == '\0') {
                ls_syslog (LOG_ERR, I18N (5343, "%s: File %s in section Queue ending at line %d: No valid administrators <%s> specified for queue <%s>;ignoring"), __func__, filename, *lineNum, keylist[QKEY_ADMINISTRATORS].val, queue.queue);  /* catgets 5343 */
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (queue.admins);
            }
        }

        if (keylist[QKEY_PRE_EXEC].val != NULL
            && strcmp (keylist[QKEY_PRE_EXEC].val, ""))
            {
            if (strlen (keylist[QKEY_PRE_EXEC].val) >= MAXLINELEN)
                {
                ls_syslog (LOG_ERR, I18N (5344, "%s: File %s in section Queue ending at line %d: PRE_EXEC of the queue <%s> is too long <%s>; ignoring"), __func__, filename, *lineNum, queue.queue, keylist[QKEY_PRE_EXEC].val); /* catgets 5344 */
                lsberrno = LSBE_CONF_WARNING;
                }
            else {
                queue.preCmd = putstr_ (keylist[QKEY_PRE_EXEC].val);
                if (queue.preCmd == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_PRE_EXEC].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                }
            }
        }

        if (keylist[QKEY_PRE_POST_EXEC_USER].val != NULL && strcmp (keylist[QKEY_PRE_POST_EXEC_USER].val, "")) {
            if (strlen (keylist[QKEY_PRE_POST_EXEC_USER].val) >= MAXLINELEN) {
                ls_syslog (LOG_ERR, I18N (5352, "%s: User name %s in section Queue ending at line %d: PRE_POST_EXEC_USER of the queue <%s> is too long <%s>; ignoring"), __func__, filename, *lineNum, queue.queue, keylist[QKEY_PRE_POST_EXEC_USER].val);    /* catgets 5352 */
                lsberrno = LSBE_CONF_WARNING;
            }
            else {
                queue.prepostUsername = putstr_ (keylist[QKEY_PRE_POST_EXEC_USER].val);
                if (queue.prepostUsername == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_PRE_POST_EXEC_USER].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                }
            }
        }

        if (keylist[QKEY_POST_EXEC].val != NULL && strcmp (keylist[QKEY_POST_EXEC].val, "")) {
            if (strlen (keylist[QKEY_POST_EXEC].val) >= MAXLINELEN) {
                ls_syslog (LOG_ERR, I18N (5347, "%s: File %s in section Queue ending at line %d: POST_EXEC of the queue <%s> is too long <%s>; ignoring"), __func__, filename, *lineNum, queue.queue, keylist[QKEY_POST_EXEC].val);   /* catgets 5347 */
                lsberrno = LSBE_CONF_WARNING;
            }
            else {
                queue.postCmd = putstr_ (keylist[QKEY_POST_EXEC].val);
                if (queue.postCmd == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_POST_EXEC].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keylist);
                    freeQueueInfo (&queue);
                    return FALSE;
                }
            }
        }

        if (keylist[QKEY_REQUEUE_EXIT_VALUES].val != NULL && strcmp (keylist[QKEY_REQUEUE_EXIT_VALUES].val, "")) {
            if (strlen (keylist[QKEY_REQUEUE_EXIT_VALUES].val) >= MAXLINELEN) {
                ls_syslog (LOG_ERR, I18N (5350, "%s: File %s in section Queue ending at line %d: REQUEUE_EXIT_VALUES  of the queue <%s> is too long <%s>; ignoring"), __func__, filename, *lineNum, queue.queue, keylist[QKEY_REQUEUE_EXIT_VALUES].val);  /* catgets 5350 */
                lsberrno = LSBE_CONF_WARNING;
            }
            else if (!checkRequeEValues (&queue, keylist[QKEY_REQUEUE_EXIT_VALUES].val, filename, lineNum) && lsberrno == LSBE_NO_MEM) {
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
            else {
                printf( "we done goofed up at do_Queues()");
            }
        }

        if (keylist[QKEY_RES_REQ].val != NULL && strcmp (keylist[QKEY_RES_REQ].val, "")) {
            queue.resReq = putstr_ (keylist[QKEY_RES_REQ].val);
            if (queue.resReq == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_RES_REQ].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
        }

        if (keylist[QKEY_SLOT_RESERVE].val != NULL && strcmp (keylist[QKEY_SLOT_RESERVE].val, "")) {
            getReserve (keylist[QKEY_SLOT_RESERVE].val, &queue, filename, *lineNum);
        }

        if (keylist[QKEY_RESUME_COND].val != NULL && strcmp (keylist[QKEY_RESUME_COND].val, "")) {
            queue.resumeCond = putstr_ (keylist[QKEY_RESUME_COND].val);
            if (queue.resumeCond == NULL) {

                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_RESUME_COND].val));
                lsberrno = LSBE_NO_MEM;
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
        }

        if (keylist[QKEY_STOP_COND].val != NULL && strcmp (keylist[QKEY_STOP_COND].val, "")) {
            queue.stopCond = putstr_ (keylist[QKEY_STOP_COND].val);
            if (queue.stopCond == NULL) {

                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_STOP_COND].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
        }

        if (keylist[QKEY_JOB_STARTER].val != NULL && strcmp (keylist[QKEY_JOB_STARTER].val, "")) {
            queue.jobStarter = putstr_ (keylist[QKEY_JOB_STARTER].val);
            if (queue.jobStarter == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keylist[QKEY_JOB_STARTER].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keylist);
                freeQueueInfo (&queue);
                return FALSE;
            }
        }

        if (keylist[QKEY_JOB_CONTROLS].val != NULL && strcmp (keylist[QKEY_JOB_CONTROLS].val, "") && 
                parseSigActCmd (&queue, keylist[QKEY_JOB_CONTROLS].val, filename, lineNum, "in section Queue ending") < 0 ) {
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keylist);
            freeQueueInfo (&queue);
            return FALSE;
        }

        if (keylist[QKEY_TERMINATE_WHEN].val != NULL && strcmp (keylist[QKEY_TERMINATE_WHEN].val, "") && 
                terminateWhen (&queue, keylist[QKEY_TERMINATE_WHEN].val, filename, lineNum, "in section Queue ending") < 0 ) {
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keylist);
            freeQueueInfo (&queue);
            return FALSE;
        }

        assert( info->numIndx >= 0 );
        queue.loadSched = calloc( info->numIndx, sizeof (float *));
        if (info->numIndx && NULL == queue.loadSched && ENOMEM == errno) {
            lsberrno = LSBE_NO_MEM;
            freekeyval (keylist);
            freeQueueInfo (&queue);
            return FALSE;
        }
        assert( info->numIndx >=0 );
        queue.loadStop = calloc( info->numIndx, sizeof (float *));
        if (info->numIndx && NULL == queue.loadStop && ENOMEM == errno ) {
            lsberrno = LSBE_NO_MEM;
            freekeyval (keylist);
            freeQueueInfo (&queue);
            return FALSE;
        }

        getThresh (info, keylist, queue.loadSched, queue.loadStop, filename, lineNum, " in section Queue ending");
        queue.nIdx = info->numIndx;
        if (!addQueue (&queue, filename, *lineNum) && lsberrno == LSBE_NO_MEM) {
            freekeyval (keylist);
            freeQueueInfo (&queue);
            return FALSE;
        }

        freekeyval (keylist);
        return TRUE;
    }
}

static void
initQueueInfo (struct queueInfoEnt *qp)
{
    int i;

    qp->queue = NULL;
    qp->description = NULL;
    qp->userList = NULL;
    qp->hostList = NULL;
    qp->loadSched = NULL;
    qp->loadStop = NULL;
    qp->windows = NULL;
    qp->hostSpec = NULL;
    qp->windowsD = NULL;
    qp->defaultHostSpec = NULL;
    qp->admins = NULL;
    qp->preCmd = NULL;
    qp->postCmd = NULL;
    qp->prepostUsername = NULL;
    qp->requeueEValues = NULL;
    qp->resReq = NULL;
    qp->priority = INFINIT_INT;
    qp->nice = INFINIT_SHORT;
    qp->nIdx = 0;
    qp->userJobLimit = INFINIT_INT;
    qp->procJobLimit = INFINIT_FLOAT;
    qp->qAttrib = 0;
    qp->qStatus = INFINIT_INT;
    qp->maxJobs = INFINIT_INT;
    qp->numJobs = INFINIT_INT;
    qp->numPEND = INFINIT_INT;
    qp->numRUN = INFINIT_INT;
    qp->numSSUSP = INFINIT_INT;
    qp->numUSUSP = INFINIT_INT;
    qp->mig = INFINIT_INT;
    qp->schedDelay = INFINIT_INT;
    qp->acceptIntvl = INFINIT_INT;
    qp->procLimit = INFINIT_INT;
    qp->minProcLimit = INFINIT_INT;
    qp->defProcLimit = INFINIT_INT;
    qp->hostJobLimit = INFINIT_INT;

    for (i = 0; i < LSF_RLIM_NLIMITS; i++)
        {
        qp->rLimits[i] = INFINIT_INT;
        qp->defLimits[i] = INFINIT_INT;
        }

    qp->numRESERVE = INFINIT_INT;
    qp->slotHoldTime = INFINIT_INT;
    qp->resumeCond = NULL;
    qp->stopCond = NULL;
    qp->jobStarter = NULL;

    qp->suspendActCmd = NULL;
    qp->resumeActCmd = NULL;
    qp->terminateActCmd = NULL;
    for (i = 0; i < LSB_SIG_NUM; i++)
        qp->sigMap[i] = 0;

    qp->chkpntPeriod = -1;
    qp->chkpntDir = NULL;
}

static void
freeQueueInfo (struct queueInfoEnt *qp)
{
    if (qp == NULL)
        return;

    FREEUP (qp->queue);
    FREEUP (qp->description);
    FREEUP (qp->userList);
    FREEUP (qp->hostList);
    FREEUP (qp->loadSched);
    FREEUP (qp->loadStop);
    FREEUP (qp->windows);
    FREEUP (qp->hostSpec);
    FREEUP (qp->windowsD);
    FREEUP (qp->defaultHostSpec);
    FREEUP (qp->admins);
    FREEUP (qp->preCmd);
    FREEUP (qp->postCmd);
    FREEUP (qp->prepostUsername);
    FREEUP (qp->requeueEValues);
    FREEUP (qp->resReq);
    FREEUP (qp->jobStarter);
    FREEUP (qp->stopCond);
    FREEUP (qp->resumeCond);
    FREEUP (qp->chkpntDir);
    FREEUP (qp->suspendActCmd);
    FREEUP (qp->resumeActCmd);
    FREEUP (qp->terminateActCmd);
}

char
checkRequeEValues (struct queueInfoEnt *qp, char *word, char *filename, size_t *lineNum)
{
#define NORMAL_EXIT 0
#define EXCLUDE_EXIT 1
    
    char *sp, *cp, exitValues[MAXLINELEN];
    int numEValues = 0, exitV, i, found, mode = NORMAL_EXIT;
    int exitInts[400];

    exitValues[0] = '\0';
    cp = word;

    while ((sp = a_getNextWord_ (&word)) != NULL)
        {
        if (isint_ (sp) && (exitV = my_atoi (sp, 256, -1)) != INFINIT_INT)
            {
            found = FALSE;
            for (i = 0; i < numEValues; i++)
                {
                if (exitInts[i] == exitV)
                    {
                    found = TRUE;
                    break;
                    }
                }
            if (found == TRUE)
                {
                if (filename) {      // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
                    ls_syslog (LOG_ERR, I18N (5376, "%s: File %s in section Queue ending at line %d: requeue exit value <%s> for queue <%s> is repeated; ignoring"), __func__, filename, *lineNum, sp, qp->queue);    /* catgets 5376 */
                }
                lsberrno = LSBE_CONF_WARNING;
                continue;
                }
            strcat (exitValues, sp);
            strcat (exitValues, " ");
            exitInts[numEValues] = exitV;
            numEValues++;
            }
        else if (strncasecmp (sp, "EXCLUDE(", 8) == 0)
            {
            strcat (exitValues, sp);
            strcat (exitValues, " ");
            mode = EXCLUDE_EXIT;
            }
        else if (*sp == ')' && mode == EXCLUDE_EXIT)
            {
            strcat (exitValues, sp);
            strcat (exitValues, " ");
            mode = NORMAL_EXIT;
            }
        else
            {
            if (filename) {     // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
                ls_syslog (LOG_ERR, I18N (5377, "%s: File %s in section Queue ending at line %d: requeue exit values <%s> for queue <%s> is not an interger between 0-255; ignored"), __func__, filename, *lineNum, sp, qp->queue);   /* catgets 5377 */
            }
            lsberrno = LSBE_CONF_WARNING;
            }
        }
    if (numEValues == 0)
        {
        if (!filename) {       // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5378, "%s: File %s in section Queue ending at line %d: No valid requeue exit values <%s> for queue <%s>; ignoring"), __func__, filename, *lineNum, cp, qp->queue);   /* catgets 5378 */
        }
        lsberrno = LSBE_CONF_WARNING;
        return FALSE;
        }
    qp->requeueEValues = putstr_ (exitValues);
    if (qp->requeueEValues == NULL)
        {
        if (!filename) {       // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (exitValues) + 1);
        }
        lsberrno = LSBE_NO_MEM;
        return FALSE;
        }
    return TRUE;

}

static char *
a_getNextWord_ (char **line)
{
    char *wordp, *word;
    word = getNextWord_ (line);
    if (word && (wordp = strchr (word, '('))) {
        *(wordp + 1) = '\0';
        while (**line != '(') {
            (*line)--;
        }
        (*line)++;
        }
    else if (word && (wordp = strchr (word, ')')))
        {
        if (wordp != word)
            {
            wordp--;
            while (isspace (*wordp)) {
                wordp--;
            }
            *(wordp + 1) = '\0';
            while (**line != ')')
                (*line)--;
            }
        }
    return word;
}

static char
addQueue (struct queueInfoEnt *qp, char *filename, unsigned int lineNum)
{
    
    struct queueInfoEnt **tmpQueues = NULL;

    assert( *filename );
    assert( lineNum );

    if (qp == NULL) {
        return TRUE;
    }

    if (numofqueues == queuesize) {
        if (queuesize == 0) {
            queuesize = 5;
        }
        else {
            queuesize *= 2;
        }
        //assert( queuesize >= 0 );
        tmpQueues = myrealloc(queues, queuesize * sizeof (struct queueInfoEnt *));
        if( NULL == tmpQueues && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "myrealloc", queuesize * sizeof (struct queueInfoEnt *));
            lsberrno = LSBE_NO_MEM;
            freeQueueInfo (qp);
            return FALSE;
        }
        else {
            queues = tmpQueues;
        }
    }
    
    queues[numofqueues] = malloc(sizeof (struct queueInfoEnt));
    if ( NULL == queues[numofqueues] ) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct queueInfoEnt));
        lsberrno = LSBE_NO_MEM;
        freeQueueInfo (qp);
        return FALSE;
    }

    initQueueInfo (queues[numofqueues]);
    *queues[numofqueues] = *qp;
    numofqueues++;
    return TRUE;
}


void
freeWorkUser (int freeAll)
{
    for ( unsigned int i = 0; i < numofugroups; i++) {
        if (freeAll == TRUE) {
            freeGroupInfo (usergroups[i]);
        }
        FREEUP (usergroups[i]);
    }
    numofugroups = 0;
 
    for ( unsigned int i = 0; i < numofusers; i++) {
        if (freeAll == TRUE) {
            freeUserInfo (users[i]);
        }
        FREEUP (users[i]);
    }
    FREEUP (users);
    numofusers = 0;
}

void
freeWorkHost (int freeAll)
{
    for ( unsigned int i = 0; i < numofhosts; i++) {
        if (freeAll == TRUE) {
            freeHostInfoEnt( hosts[i] );
        }
        FREEUP (hosts[i]);
    }
    FREEUP (hosts);
    numofhosts = 0;

    for ( unsigned int i = 0; i < numofhgroups; i++) {
        if (freeAll == TRUE) {
            freeGroupInfo (hostgroups[i]);
        }
        FREEUP (hostgroups[i]);
    }
    numofhgroups = 0;
}

void
freeWorkQueue (int freeAll)
{
    for ( unsigned int i = 0; i < numofqueues; i++) {
        if (freeAll == TRUE) {
            freeQueueInfo (queues[i]);
        }
        FREEUP (queues[i]);
    }
    FREEUP (queues);
    numofqueues = 0;
}

void
freeUConf (struct userConf *uConf1, int freeAll)
{
    for ( unsigned int i = 0; i < uConf1->numUgroups; i++) {
        if (freeAll == TRUE) {
            freeGroupInfo (&uConf1->ugroups[i]);
        }
    }

    FREEUP (uConf1->ugroups);
    uConf1->numUgroups = 0;

    for ( unsigned int i = 0; i < uConf1->numUsers; i++) {
        if (freeAll == TRUE) {
            freeUserInfo (&uConf1->users[i]);
        }
    }

    FREEUP (uConf1->users);
    uConf1->numUsers = 0;
}

void
freeHConf (struct hostConf *hConf1, int freeAll)
{
    for ( unsigned int i = 0; i < hConf1->numHosts; i++) {
        if (freeAll == TRUE) {
            freeHostInfoEnt ( &hConf1->hosts[i]);
        }
    }

    FREEUP (hConf1->hosts);
    hConf1->numHosts = 0;
    
    for ( unsigned int i = 0; i < hConf1->numHgroups; i++)  {
        if (freeAll == TRUE) {
            freeGroupInfo (&hConf1->hgroups[i]);
        }
    }
    FREEUP (hConf1->hgroups);
    hConf1->numHgroups = 0;
}

void
freeQConf (struct queueConf *qConf1, int freeAll)
{
    for ( unsigned int i = 0; i < qConf1->numQueues; i++) {
        if (freeAll == TRUE) {
            freeQueueInfo (&qConf1->queues[i]);
        }
    }
    FREEUP (qConf1->queues);
    qConf1->numQueues = 0;
}

static void
resetUConf (struct userConf *uConf1)
{
    uConf1->numUgroups = 0;
    uConf1->numUsers = 0;
    uConf1->ugroups = NULL;
    uConf1->users = NULL;
}

static void
resetHConf (struct hostConf *hConf1)
{
    hConf1->numHosts = 0;
    hConf1->numHgroups = 0;
    hConf1->hosts = NULL;
    hConf1->hgroups = NULL;   
}

static void
checkCpuLimit (char **hostSpec, double **cpuFactor, int useSysDefault, char *filename, size_t *lineNum, char *pname, struct lsInfo *info, int options)
{
    if( ( *hostSpec ) && ( *cpuFactor == NULL ) && ( options != CONF_NO_CHECK ) && ( (*cpuFactor = getModelFactor (*hostSpec, info)) == NULL) && ( (*cpuFactor = getHostFactor (*hostSpec)) == NULL) ) {
        if (useSysDefault == TRUE) {
            /* catgets 5383 */
            ls_syslog (LOG_ERR, I18N (5383, "[%s] %s: File %s in section Queue end at line %ld: Invalid DEFAULT_HOST_SPEC <%s>; ignored"), __func__, pname, filename, *lineNum, pConf->param->defaultHostSpec);

            lsberrno = LSBE_CONF_WARNING;
            FREEUP (pConf->param->defaultHostSpec);
            FREEUP (*hostSpec);
        }
        else {
            /* catgets 5384 */
            ls_syslog (LOG_ERR, I18N (5384, "[%s] %s: File %s in section Queue end at line %ld: Invalid host_spec <%s>; ignored"), __func__, pname, filename, *lineNum, *hostSpec);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (*hostSpec);
        }
    }
}

static int
parseCpuAndRunLimit (struct keymap *keylist, struct queueInfoEnt *qp, char *filename, size_t *lineNum, char *pname, struct lsInfo *info, int options)
{
    struct keymap key;
    int limit = 0; 
    int retValue = 0;
    int useSysDefault   = FALSE;
    double *cpuFactor    = NULL;
    char   *spec         = NULL;
    char   *sp           = NULL;
    char   *hostSpec     = NULL;
    char   *defaultLimit = NULL;
    char   *maxLimit     = NULL;

    key = keylist[QKEY_CPULIMIT];

    defaultLimit = NULL;
    maxLimit = NULL;
    sp = key.val;
    if (sp != NULL) {
        defaultLimit = putstr_ (getNextWord_ (&sp));
        maxLimit = getNextWord_ (&sp);
    }

    if (maxLimit != NULL) {
        
        retValue = parseLimitAndSpec (defaultLimit, &limit, &spec, hostSpec, key.key, qp, filename, lineNum, pname);
        
        if (retValue == 0) {
            if (limit >= 0) {
                qp->defLimits[LSF_RLIMIT_CPU] = limit;
            }
            if ((spec) && !(hostSpec)) {
                hostSpec = putstr_ (spec);
            }
        }
    }
    else if (!maxLimit && defaultLimit) {
        maxLimit = defaultLimit;
    }

    retValue = parseLimitAndSpec (maxLimit, &limit, &spec, hostSpec, key.key, qp, filename, lineNum, pname);
    if (retValue == 0) {
        if (limit >= 0) {
            qp->rLimits[LSF_RLIMIT_CPU] = limit;
        }
        if ((spec) && !(hostSpec)) {
            hostSpec = putstr_ (spec);
        }
    }

    if (sp && strlen (sp) != 0) {
        lsberrno = LSBE_CONF_WARNING;
        /* catgets 5464 */
        ls_syslog (LOG_ERR, I18N (5464, "%s: File %s in section Queue ending at line %d: CPULIMIT for queue has extra parameters: %s; These parameters will be ignored."), pname, filename, *lineNum, sp); 
        lsberrno = LSBE_CONF_WARNING;
    }

    key = keylist[QKEY_RUNLIMIT];

    FREEUP (defaultLimit);
    maxLimit = NULL;
    sp = key.val;
    if (sp != NULL) {
        defaultLimit = putstr_ (getNextWord_ (&sp));
        maxLimit = getNextWord_ (&sp);
    }

    if (maxLimit != NULL) {

        retValue = parseLimitAndSpec (defaultLimit, &limit, &spec, hostSpec, key.key, qp, filename, lineNum, pname);
        if (retValue == 0) {
            if (limit >= 0) {
                qp->defLimits[LSF_RLIMIT_RUN] = limit;
            }
            if ((spec) && !(hostSpec)) {
                hostSpec = putstr_ (spec);
            }
        }
    }
    else if (!maxLimit && defaultLimit) {
        maxLimit = defaultLimit;
    }

    retValue = parseLimitAndSpec (maxLimit, &limit, &spec, hostSpec, key.key, qp, filename, lineNum, pname);
    if (retValue == 0) {
        if (limit >= 0) {
            qp->rLimits[LSF_RLIMIT_RUN] = limit;
        }
        if ((spec) && !(hostSpec)) {
            hostSpec = putstr_ (spec);
        }
    }

    if (sp && strlen (sp) != 0) {
        lsberrno = LSBE_CONF_WARNING;
        /* catgets 5464 */
        ls_syslog (LOG_ERR, I18N (5464, "%s: File %s in section Queue ending at line %d: RUNLIMIT for queue has extra parameters: %s; These parameters will be ignored."), pname, filename, *lineNum, sp); 
        lsberrno = LSBE_CONF_WARNING;
    }

    if (qp->defLimits[LSF_RLIMIT_CPU] != INFINIT_INT && qp->rLimits[LSF_RLIMIT_CPU] != INFINIT_INT && qp->defLimits[LSF_RLIMIT_CPU] > qp->rLimits[LSF_RLIMIT_CPU]) {
        /* catgets 5111 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5111, "%s: File %s in section Queue at line %d: The default CPULIMIT %d should not be greater than the max CPULIMIT %d; ignoring the default CPULIMIT and using max CPULIMIT also as default CPULIMIT"), pname, filename, *lineNum, qp->defLimits[LSF_RLIMIT_CPU], qp->rLimits[LSF_RLIMIT_CPU]);
        qp->defLimits[LSF_RLIMIT_CPU] = qp->rLimits[LSF_RLIMIT_CPU];
    }

    if (qp->defLimits[LSF_RLIMIT_RUN] != INFINIT_INT && qp->rLimits[LSF_RLIMIT_RUN] != INFINIT_INT && qp->defLimits[LSF_RLIMIT_RUN] > qp->rLimits[LSF_RLIMIT_RUN]) {
        /* catgets 5110 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5110, "%s: File %s in section Queue at line %d: The default RUNLIMIT %d should not be greater than the max RUNLIMIT %d; ignoring the default RUNLIMIT and using max RUNLIMIT also as default RUNLIMIT"), pname, filename, *lineNum, qp->defLimits[LSF_RLIMIT_RUN], qp->rLimits[LSF_RLIMIT_RUN]);
        qp->defLimits[LSF_RLIMIT_RUN] = qp->rLimits[LSF_RLIMIT_RUN];
    }

    if (hostSpec != NULL) {
        checkCpuLimit (&hostSpec, &cpuFactor, useSysDefault, filename, lineNum, pname, info, options);
    }

    if (cpuFactor == NULL && options & CONF_RETURN_HOSTSPEC) {
        if (qp->defaultHostSpec != NULL && qp->defaultHostSpec[0]) {
            
            hostSpec = putstr_ (qp->defaultHostSpec);
            if (hostSpec && hostSpec[0]) {
                checkCpuLimit (&hostSpec, &cpuFactor, useSysDefault, filename, lineNum, pname, info, options);
                }
            }
        }

    if (cpuFactor == NULL && options & CONF_RETURN_HOSTSPEC) {
        if (pConf && pConf->param && pConf->param->defaultHostSpec != NULL && pConf->param->defaultHostSpec[0]) {
            
            hostSpec = putstr_ (pConf->param->defaultHostSpec);
            useSysDefault = TRUE;
            if (hostSpec && hostSpec[0]) {
                checkCpuLimit (&hostSpec, &cpuFactor, useSysDefault, filename, lineNum, pname, info, options);
                }
            }
        }

    if (hostSpec == NULL && (options & CONF_RETURN_HOSTSPEC) && (options & CONF_CHECK)) {
        if (maxHName == NULL && maxFactor <= 0.0) {
            struct hostInfoEnt *hostPtr;
            for ( unsigned int i = 0; i < hConf->numHosts; i++) {
                hostPtr = &(hConf->hosts[i]);
                if (maxFactor < hostPtr->cpuFactor) {
                    maxFactor = hostPtr->cpuFactor;
                    maxHName = hostPtr->host;
                }
            }
        }
        cpuFactor = &maxFactor;
        hostSpec = putstr_ (maxHName);
    }

    if (hostSpec && ((qp->hostSpec = putstr_ (hostSpec)) == NULL)) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (hostSpec) + 1);
        lsberrno = LSBE_NO_MEM;
        FREEUP (hostSpec);
        cpuFactor = NULL;
    }

    if ((options & CONF_NO_EXPAND) == 0) {

        if (cpuFactor != NULL) {

            double limit_ = 0.0;
            if (qp->rLimits[LSF_RLIMIT_CPU] > 0 && qp->rLimits[LSF_RLIMIT_CPU] != INFINIT_INT && (options & CONF_RETURN_HOSTSPEC)) {
                limit_ = qp->rLimits[LSF_RLIMIT_CPU] * (*cpuFactor);
                limit_ = limit_ < 1 ? 1 : limit_ + 0.5;
                qp->rLimits[LSF_RLIMIT_CPU] = (int) limit_;
            }

            if (qp->defLimits[LSF_RLIMIT_CPU] > 0 && qp->defLimits[LSF_RLIMIT_CPU] != INFINIT_INT && (options & CONF_RETURN_HOSTSPEC)) {
                limit_ = qp->defLimits[LSF_RLIMIT_CPU] * (*cpuFactor);
                limit_ = limit_ < 1 ? 1 : limit_ + 0.5;
                qp->defLimits[LSF_RLIMIT_CPU] = (int) limit_;
            }

            if (qp->rLimits[LSF_RLIMIT_RUN] > 0 && qp->rLimits[LSF_RLIMIT_RUN] != INFINIT_INT && (options & CONF_RETURN_HOSTSPEC)) {
                limit_ = qp->rLimits[LSF_RLIMIT_RUN] * (*cpuFactor);
                limit_ = limit_ < 1 ? 1 : limit_ + 0.5;
                qp->rLimits[LSF_RLIMIT_RUN] = (int) limit_;
            }

            if (qp->defLimits[LSF_RLIMIT_RUN] > 0 && qp->defLimits[LSF_RLIMIT_RUN] != INFINIT_INT && (options & CONF_RETURN_HOSTSPEC)) {
                limit_ = qp->defLimits[LSF_RLIMIT_RUN] * (*cpuFactor);
                limit_ = limit_ < 1 ? 1 : limit_ + 0.5;
                qp->defLimits[LSF_RLIMIT_RUN] = (int) limit_;
            }
        }
        else {
            qp->rLimits[LSF_RLIMIT_CPU] = INFINIT_INT;
            qp->rLimits[LSF_RLIMIT_RUN] = INFINIT_INT;
            qp->defLimits[LSF_RLIMIT_CPU] = INFINIT_INT;
            qp->defLimits[LSF_RLIMIT_RUN] = INFINIT_INT;
        }
    }
    
    FREEUP (hostSpec);
    FREEUP (spec);
    FREEUP (defaultLimit);
    return TRUE;
}

static int
parseProcLimit (char *word, struct queueInfoEnt *qp, char *filename, size_t *lineNum, char *pname )
{
    char *sp = NULL;
    char *curWord = NULL;
    int values[3] = { 0, 0, 0 };
    int i = 0;

    assert( pname ); // FIXME FIXME where was this supposed to go?

    sp = word;
    if (sp == NULL) {
        return -1;
    }
    else {
        while (*sp != '\0' && *sp != '#') {
            sp++;
        }

        if (*sp == '#') {
            *sp = '\0';
        }

        sp = word;
        for ( i = 0; i < 3; i++) {
            curWord = getNextWord_ (&sp);
            if (curWord == NULL) {
                break;
            }
            if ((values[i] = my_atoi (curWord, INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5302 */
                ls_syslog (LOG_ERR, I18N (5302, "%s: File %s in section Queue ending at line %d: PROCLIMIT value <%s> isn't a positive integer; ignored"), pname, filename, *lineNum, curWord);
                return FALSE;
            }
        }
        if (getNextWord_ (&sp) != NULL) {
            /* catgets 5371 */
            ls_syslog (LOG_ERR, I18N (5371, "%s: File %s in section Queue ending at line %d: PROCLIMIT has too many parameters; ignored. PROCLIMIT=[minimum [default]] maximum"), pname, filename, *lineNum);
            return FALSE;
        }
        
        switch (i) {
            case 1:
                qp->procLimit = values[0];
                qp->minProcLimit = 1;
                qp->defProcLimit = 1;
            break;
            case 2:
                if (values[0] > values[1]) {
                    /* catgets 5370 */
                    ls_syslog (LOG_ERR, I18N (5370, "%s: File %s in section Queue ending at line %d: PROCLIMIT values <%d %d> are not valid; ignored. PROCLIMIT values must satisfy the following condition: 1 <= minimum <= maximum"), pname, filename, *lineNum, values[0], values[1]);
                    return FALSE;
                }
                else {
                    qp->minProcLimit = values[0];
                    qp->defProcLimit = values[0];
                    qp->procLimit = values[1];
                }
            break;
            case 3:
                if (!(values[0] <= values[1] && values[1] <= values[2])) {
                    /* catgets 5374 */
                    ls_syslog (LOG_ERR, I18N (5374, "%s: File %s in section Queue ending at line %d: PROCLIMIT value <%d %d %d> is not valid; ignored. PROCLIMIT values must satisfy the following condition: 1 <= minimum <= default <= maximum"), filename, filename, *lineNum, values[0], values[1], values[2]);
                    return FALSE;
                }
                else {
                    qp->minProcLimit = values[0];
                    qp->defProcLimit = values[1];
                    qp->procLimit = values[2];
                }
            break;
            default: 
                printf( "we done goofed up at parseProcLimit()\n");
            break;
            }
        }
    return TRUE;
}

static int
parseLimitAndSpec (char *word, int *limit, char **spec, char *hostSpec, char *param, struct queueInfoEnt *qp, char *filename, size_t *lineNum, char *pname )
{
    int limitVal = -1;
    char *sp = NULL;

    assert( pname ); // FIXME FIXME where was this supposed to go?

    *limit = -1;
    FREEUP (*spec);
    assert( qp->nice );
    if (word == NULL) {
        return -1;
    }

    if (word && (sp = strchr (word, '/')) != NULL) {
        *sp = '\0';
        *spec = putstr_ (sp + 1);
    }

    if (*spec && hostSpec && strcmp (*spec, hostSpec) != 0) {
        /* catgets 5382 */
        ls_syslog (LOG_ERR, I18N (5382, "%s: File %s in section Queue at line %d: host_spec for %s is multiply defined; ignoring <%s> and retaining last host_spec <%s>"), __func__, filename, *lineNum, param, *spec, hostSpec);
        lsberrno = LSBE_CONF_WARNING;
        FREEUP (*spec);
    }

    if ((sp = strchr (word, ':')) != NULL) {
        *sp = '\0';
    }

    limitVal = my_atoi (word, INFINIT_INT / 60 + 1, -1);
    if (limitVal == INFINIT_INT) {
        /* catgets 5386 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5386, "%s: File %s in section Queue at line %d: Value <%s> of %s isn't a positive integer between 0 and %d; ignored."), __func__, filename, *lineNum, word, param, INFINIT_INT / 60);
        lsberrno = LSBE_CONF_WARNING;
        sp = NULL;
    }
    else {
        *limit = limitVal * 60;
    }

    if (sp != NULL) {
        word = sp + 1;
        limitVal = my_atoi (word, INFINIT_INT / 60 + 1, -1);

        if (limitVal == INFINIT_INT) {
            /* catgets 5386 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5386, "%s: File %s in section Queue at line %d: Value <%s> of %s isn't a positive integer between 0 and %d; ignored."), __func__, filename, *lineNum, word, param, INFINIT_INT / 60);
            lsberrno = LSBE_CONF_WARNING;
            *limit = -1;
        }
        else {
            *limit += limitVal;
            *limit *= 60;
        }
    }

    return 0;    
}

static double *
getModelFactor (char *hostModel, struct lsInfo *info)
{
    static double *cpuFactor = 0;
    
    if ( NULL == hostModel ) {
        return NULL;
    }
    
    for ( unsigned int i = 0; i < info->nModels; i++) {
        if (strcmp (hostModel, info->hostModels[i]) == 0) {
            *cpuFactor = info->cpuFactor[i];
            return cpuFactor;
        }
    }
    return NULL;
}

static double *
getHostFactor (char *host)
{
    static double *cpuFactor = 0;
    struct hostent *hp;

    if (NULL == host) {
        return NULL;
    }
    if (hConf == NULL || hConf->numHosts == 0 || hConf->hosts == NULL) {
        return NULL;
    }

    hp = Gethostbyname_ (host);
    if (NULL == hp ) {
        return NULL;
    }

    for ( unsigned int i = 0; i < hConf->numHosts; i++) {
        if (equalHost_ (hp->h_name, hConf->hosts[i].host)) {
            *cpuFactor = hConf->hosts[i].cpuFactor;
            return cpuFactor;
        }
    }

    return NULL;
}

static char *
parseAdmins (char *admins, int options, char *filename, size_t *lineNum)
{
    
    char *expandAds;
    char *sp;
    char *word;
    struct group *unixGrp;
    struct passwd *pw;
    struct groupInfoEnt *uGrp;
    unsigned int len = 0;

    if (admins == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, filename, "admins");
        return NULL;
    }

    expandAds = (char *)malloc( MAXLINELEN );
    if ( NULL == expandAds && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", MAXLINELEN);
        lsberrno = LSBE_NO_MEM;
        return NULL;
    }

    expandAds = "";
    len = MAXLINELEN;
    sp = admins;
    while ((word = getNextWord_ (&sp)) != NULL) {
        if (strcmp (word, "all") == 0) {

            if (options & CONF_EXPAND) {
                expandAds = "";
                putIntoList (&expandAds, &len, "all", I18N_IN_QUEUE_ADMIN);
                return expandAds;
            }
            else {
                if ((putIntoList (&expandAds, &len, "all", I18N_IN_QUEUE_ADMIN)) == NULL) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                        FREEUP (expandAds);
                        lsberrno = LSBE_NO_MEM;
                        return NULL;
                }
                continue;
            }
        }
        else if ((pw = getpwlsfuser_ (word))) {
            if ((putIntoList(&expandAds, &len, pw->pw_name, I18N_IN_QUEUE_ADMIN)) == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                    FREEUP (expandAds);
                    lsberrno = LSBE_NO_MEM;
                    return NULL;
            }
            continue;
        }
        else if ((uGrp = getUGrpData (word)) == NULL) {
            if (options & CONF_EXPAND) {
                char **groupMembers = NULL;
                unsigned int numMembers = 0;
                if ((groupMembers = expandGrp (uGrp->group, &numMembers, USER_GRP)) == NULL) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                        FREEUP (expandAds);
                        lsberrno = LSBE_NO_MEM;
                        return NULL;
                }
                else {
                    for ( unsigned int i = 0; i < numMembers; i++) {
                        if (putIntoList (&expandAds, &len, groupMembers[i], I18N_IN_QUEUE_ADMIN) == NULL) {
                            FREEUP (groupMembers);
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                            FREEUP (expandAds);
                            lsberrno = LSBE_NO_MEM;
                            return NULL;
                        }
                    }
                }
                freeSA (groupMembers, numMembers);
                continue;
            }
            else {
                if (putIntoList(&expandAds, &len, uGrp->group, I18N_IN_QUEUE_ADMIN) == NULL) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                        FREEUP (expandAds);
                        lsberrno = LSBE_NO_MEM;
                        return NULL;
                }
                continue;
            }
        }
        else if ((unixGrp = mygetgrnam (word)) != NULL) {

            if (options & CONF_EXPAND) {
                int i = 0;
                while (unixGrp->gr_mem[i] != NULL) {
                    if (putIntoList (&expandAds, &len, unixGrp->gr_mem[i++], I18N_IN_QUEUE_ADMIN) == NULL) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                            FREEUP (expandAds);
                            lsberrno = LSBE_NO_MEM;
                            return NULL;
                    }
                    continue;
                }
            }
            else {
                if (putIntoList (&expandAds, &len, word, I18N_IN_QUEUE_ADMIN) == NULL) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                        FREEUP (expandAds);
                        lsberrno = LSBE_NO_MEM;
                        return NULL;
                }
                continue;
            }
        }
        else {
            /* catgets 5390 */
            ls_syslog (LOG_WARNING, I18N (5390, "%s: File %s at line %d: Unknown user or user group name <%s>; Maybe a windows user or of another domain."), __func__, filename, *lineNum, word); 
            
            if ((putIntoList (&expandAds, &len, word, I18N_IN_QUEUE_ADMIN)) == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, filename);
                    FREEUP (expandAds);
                    lsberrno = LSBE_NO_MEM;
                    return NULL;
            }
            continue;
        }
    }
    return expandAds;
}

static char *
putIntoList (char **list, unsigned int *len, char *string, char *listName)
{
    char *sp;
    unsigned long length = *len;

    if ( NULL == string) {
        return *list;
    }
    if ( (char **) NULL == list ) {
        return NULL;
    }

    sp = putstr_ (listName);
    if (isInList (*list, string) == TRUE) {
        /* catgets 5392 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5392, "%s: %s is repeatedly specified %s; ignoring"), __func__, string, sp);  
        FREEUP (sp);
        return *list;
    }

    if (length <= strlen (*list) + strlen (string) + 2) {
        char *temp = (char *) myrealloc (*list, 2 * length);
        if ( NULL == temp && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "myrealloc", 2 * length);
            FREEUP (*list);
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }
        *len *= 2;
        length *= 2;
        *list = temp;
    }

    FREEUP (sp);
    strcat (*list, string);
    strcat (*list, " ");

    return *list;
}


static int
isInList (char *list, char *string)
{
    char *sp, *word;
    
    if (list == NULL || string == NULL || list[0] == '\0' || string[0] == '\0') {
        return FALSE;
    }
    
    sp = list;
    while ((word = getNextWord_ (&sp)) != NULL) {
        if (strcmp (string, word) == 0) {
            return TRUE;
        }
    }
    return FALSE;
    
}

static int
setDefaultHost (struct lsInfo *info)
{
    size_t *override = NULL;
    struct hostInfoEnt *host = NULL;

    *override = TRUE;
    
    if (handleHostMem ()) {
        return -1;
    }
    
    for( unsigned int i = 0; i < cConf->numHosts; i++) {
        initHostInfoEnt(host);
        host->host = cConf->hosts[i].hostName;
        
        for( unsigned int j = 0; j < info->nModels; j++) {
            if (strcmp (cConf->hosts[i].hostModel, info->hostModels[j]) == 0) {
                host->cpuFactor = info->cpuFactor[i];
                break;
            }
        }

        if (addHostEnt( host, &cConf->hosts[i], override) == FALSE) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "addHostEnt" );
            return -1;
        }
    }
    return 0;
}

static int
setDefaultUser (void)
{
    char *functionName = malloc( strlen(  __func__ ) + 1);

    strcpy( functionName, __func__ ); 

    if (handleUserMem ()) {
        return -1;
    }

    if (!addUser( "default", INFINIT_INT, INFINIT_FLOAT, functionName, TRUE)) {
        return FALSE;
    }

    assert( numofusers > 0 );
    uConf->users = malloc( numofusers * sizeof( struct userInfoEnt ) );
    if(  0 == numofusers || ( NULL == uConf->users  && ENOMEM == errno ) ) {
        //assert( numofusers >= 0 );
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", numofusers * sizeof (struct userInfoEnt));
        lsberrno = LSBE_CONF_FATAL;
        freeWorkUser (TRUE);
        freeUConf (uConf, FALSE);
        return -1;
    }

    for ( unsigned int i = 0; i < numofusers; i++) {
        initUserInfo (&uConf->users[i]);
        uConf->users[i] = *users[i];
    }

    uConf->numUsers = numofusers;
    return 0;
}

static int
handleUserMem (void)
{
    if (uConf == NULL) {
        uConf = malloc (sizeof (struct userConf));
        if (NULL == uConf && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct userConf));
            lsberrno = LSBE_CONF_FATAL;
            return -1;
        }
        resetUConf (uConf);
    }
    else {
        freeUConf (uConf, TRUE);
    }
    freeWorkUser (FALSE);
    for( unsigned int i = 0; i < MAX_GROUPS; i++) {
        usergroups[i] = NULL;
    }
    usersize = 0;
    
    return 0;
    
}

static int
handleHostMem (void)
{
    if (hConf == NULL) {
        hConf = malloc (sizeof (struct hostConf));
        if( NULL == hConf && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct hostConf));
            lsberrno = LSBE_CONF_FATAL;
            return -1;
        }
        resetHConf (hConf);
    }
    else {        
        freeHConf (hConf, TRUE);
    }

    freeWorkHost (FALSE);
    for ( unsigned int i = 0; i < MAX_GROUPS; i++) {
        hostgroups[i] = NULL;
    }
    hostsize = 0;
    return 0;
}

int
parseSigActCmd (struct queueInfoEnt *qp, char *linep, char *filename, size_t *lineNum, char *section)
{
    
    char *actClass  = NULL;
    char *sigActCmd = NULL;
    int actClassValue = 0;

    while (isspace (*linep)) {
        linep++;
    }

    while (linep != NULL && linep[0] != '\0') {
        if ((actClass = getNextWord1_ (&linep)) == NULL) {
            if (filename) {
                /* catgets 5408 */
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5408, "%s:File %s %s at line %d: SUSPEND, RESUME or TERMINATE is missing"), __func__, filename, section, *lineNum);
            }
            lsberrno = LSBE_CONF_WARNING;
            return -1;
        }
        if (strcmp (actClass, "SUSPEND") == 0) {
            actClassValue = 0;
        }
        else if (strcmp (actClass, "RESUME") == 0) {
            actClassValue = 1;
        }
        else if (strcmp (actClass, "TERMINATE") == 0) {
            actClassValue = 2;
        }
        else {
            if (filename) {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5409, "%s:File %s %s at line %d: wrong KEYWORD"), __func__, filename, section, *lineNum);    /* catgets 5409 */
            }
            lsberrno = LSBE_CONF_WARNING;
            return -2;
        }

        while (isspace (*linep)) {
            linep++;
        }

        if (*linep != '[') {
            if (filename) {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5410, "%s:File %s %s at line %d: '[' is missing"), __func__, filename, section, *lineNum);   /* catgets 5410 */
            }
            lsberrno = LSBE_CONF_WARNING;
            return -3;
        }
        linep++;

        while (isspace (*linep)) {
            linep++;
        }
        sigActCmd = linep;

        while ((linep == NULL || linep[0] == '\0') || (*linep != ']')) {
            linep++;
        }
        if ((linep == NULL || linep[0] == '\0') || (*linep != ']')) {
            if (filename) {
                /* catgets 5411 */
                ls_syslog (LOG_ERR, I18N (5411, "%s:File %s %s at line %d: ']' is missing"), __func__, filename, section, *lineNum);
            }
            lsberrno = LSBE_CONF_WARNING;
            return -5;
        }
        *linep++ = '\0';

        if (actClassValue == 0) {
            if (strcmp (sigActCmd, "CHKPNT") == 0) {
                qp->suspendActCmd = putstr_ ("SIG_CHKPNT");
            }
            else {
                qp->suspendActCmd = putstr_ (sigActCmd);
            }
        }
        else if (actClassValue == 1) {
            if (strcmp (sigActCmd, "CHKPNT") == 0) {
                if (filename) {
                    /* catgets 5412 */
                    ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5412, "%s: File %s %s at line %d: 'CHKPNT' is not valid in RESUME"), __func__, filename, section, *lineNum);
                }
                return -7;
            }
            else {
                qp->resumeActCmd = putstr_ (sigActCmd);
            }
        }
        else if (actClassValue == 2) {
            if (strcmp (sigActCmd, "CHKPNT") == 0) {
                qp->terminateActCmd = putstr_ ("SIG_CHKPNT");
            }
            else {
                qp->terminateActCmd = putstr_ (sigActCmd);
            }
        }

        while (isspace (*linep)) {
            linep++;
        }

    } // end -- while (linep != NULL && linep[0] != '\0') 

    return 0;
}



int
terminateWhen (struct queueInfoEnt *qp, char *linep, char *filename, size_t *lineNum, char *section)
{
    

    char *sigName = NULL;
    while (isspace (*linep)) {
        linep++;
    }

    while (linep != NULL && linep[0] != '\0') {
        if ((sigName = getNextWord_ (&linep)) != NULL)
            {
            if (strcmp (sigName, "USER") == 0) {
                qp->sigMap[-sigNameToValue_ ("SIG_SUSP_USER")] = -10;
            }
            else if (strcmp (sigName, "LOAD") == 0) {
                qp->sigMap[-sigNameToValue_ ("SIG_SUSP_LOAD")] = -10;
            }
            else if (strcmp (sigName, "WINDOW") == 0) {
                qp->sigMap[-sigNameToValue_ ("SIG_SUSP_WINDOW")] = -10;
            }
            else {
                if (filename) {
                    /* catgets 5413 */
                    ls_syslog (LOG_ERR, I18N (5413, "%s:File %s %s at line %d: LOAD or WINDOW is missing"), __func__, filename, section, *lineNum);
                }
                lsberrno = LSBE_CONF_WARNING;
                return -1;
            }
        }
        while (isspace (*linep)) {
            linep++;
        }
    }

    return 0;
}

static int
checkAllOthers (char *word, int *hasAllOthers)
{
    char **grpHosts;
    unsigned int numHosts = 0;
    int returnCode = 0;
   
    grpHosts = expandGrp (word, &numHosts, HOST_GRP);
    if (grpHosts == NULL && lsberrno == LSBE_NO_MEM) {
        return returnCode;
    }
 
    if (numHosts && !strcmp (grpHosts[0], "all")) {
        if (*hasAllOthers == TRUE) {
            returnCode = -1;
        }
        else {
            *hasAllOthers = TRUE;
        }
    }
    
    freeSA (grpHosts, numHosts);
    return returnCode;
}

static int
getReserve (char *reserve, struct queueInfoEnt *qp, char *filename, unsigned int lineNum)
{
    char *sp = NULL;
    char *cp = NULL;
    
    if ((sp = strstr (reserve, "MAX_RESERVE_TIME")) != NULL)
        {
        sp += strlen ("MAX_RESERVE_TIME");
        while (isspace (*sp))
            sp++;
        if (*sp == '\0')
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5431, "%s: File %s in section Queue ending at line %d: MAX_RESERVE_TIME is specified without period time; ignoring SLOT_RESERVE"), __func__, filename, lineNum);  /* catgets 5431 */
            lsberrno = LSBE_CONF_WARNING;
            return -1;
            }
        if (*sp != '[')
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5432, "%s: File %s in section Queue ending at line %d: MAX_RESERVE_TIME <%s> is specified without '['; ignoring SLOT_RESERVE"), __func__, filename, lineNum, reserve);    /* catgets 5432 */
            lsberrno = LSBE_CONF_WARNING;
            return -1;
            }
        cp = ++sp;
        while (*sp != ']' && *sp != '\0' && isdigit (*sp) && *sp != ' ')
            sp++;
        if (*sp == '\0' || (*sp != ']' && *sp != ' '))
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5433, "%s: File %s in section Queue ending at line %d: MAX_RESERVE_TIME is specified without ']'; ignoring SLOT_RESERVE"), __func__, filename, lineNum);  /* catgets 5433 */
            lsberrno = LSBE_CONF_WARNING;
            return -1;
            }
        if (*sp == ' ')
            {
            while (*sp == ' ')
                sp++;
            if (*sp != ']')
                {
                ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5434, "%s: File %s in section Queue ending at line %d: MAX_RESERV_TIME is specified without ']';ignoring SLOT_RESERVE"), __func__, filename, lineNum);    /* catgets 5434 */
                lsberrno = LSBE_CONF_WARNING;
                return -1;
                }
            }
        *sp = '\0';
        qp->slotHoldTime = my_atoi (cp, INFINIT_INT, 0);
        if (qp->slotHoldTime == INFINIT_INT)
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5435, "%s: File %s in section Queue ending at line %d: Value <%s> of MAX_RESERV_TIME for queue <%s> isn't an integer between 1 and %d; ignored."), __func__, filename, lineNum, cp, qp->queue, INFINIT_INT);  /* catgets 5435 */
            lsberrno = LSBE_CONF_WARNING;
            *sp = ']';
            return -1;
            }
        *sp = ']';
        }
    if (strstr (reserve, "BACKFILL") != NULL)
        {
        if (qp->slotHoldTime == INFINIT_INT)
            {
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5436, "%s: File %s in section Queue ending at line %d: BACKFILL is specified without MAX_RESERV_TIME for SLOT_RESERVE; ignoring"), __func__, filename, lineNum);  /* catgets 5436 */
            lsberrno = LSBE_CONF_WARNING;
            return -1;
            }
        qp->qAttrib |= QUEUE_ATTRIB_BACKFILL;
        }
    return 0;
    
}

static int
isServerHost (char *hostName)
{
    if( hostName == NULL ) {
        return FALSE;
    }

    for ( unsigned int i = 0; i < cConf->numHosts; i++ ) {
        if (equalHost_ (cConf->hosts[i].hostName, hostName) && cConf->hosts[i].isServer == TRUE) {
            return TRUE;
        }
    }
    return FALSE;
    
}

struct group *
mygetgrnam (const char *name)
{
    int first_entry = 1;
    int num;
    int total_entries = 0;
    int count;
    struct group *grentry;
    static struct group *grretentry;
    static struct mygroup {
        struct group gr_entry;
        struct mygroup *gr_next;
    } *mygrhead, *mygrentry, *tmpgrentry;
    
    
    freeUnixGrp (grretentry);
    grretentry = NULL;
    
    
    setgrent ();
    
    while ((grentry = getgrent ()) != NULL) {
        
        if (strcmp (name, grentry->gr_name) == 0) {
            
            mygrentry = malloc (sizeof (struct mygroup));
            
            if (!mygrentry) {
                goto errorCleanup;
            }
            
            if (first_entry) {
                mygrhead = mygrentry;
                first_entry = 0;
            }
            else {
                tmpgrentry->gr_next = mygrentry;
            }
            
            
            mygrentry->gr_entry.gr_name = putstr_ (grentry->gr_name);
            mygrentry->gr_entry.gr_passwd = putstr_ (grentry->gr_passwd);
            mygrentry->gr_entry.gr_gid = grentry->gr_gid;
            
            
            for (num = 0; grentry->gr_mem[num]; num++) {
                ;
            }
            
            assert( num + 1 >= 0 );
            (mygrentry->gr_entry.gr_mem) = (char **) calloc (sizeof (char *), (unsigned long)(num + 1) );
            if ( NULL == mygrentry->gr_entry.gr_mem && ENOMEM == errno ) {
                goto errorCleanup;
            }
            
            for (count = 0; grentry->gr_mem[count]; count++) {
                mygrentry->gr_entry.gr_mem[count] = putstr_ (grentry->gr_mem[count]);
            }
            
            mygrentry->gr_next = NULL;
            tmpgrentry = mygrentry;
            mygrentry = mygrentry->gr_next;
        }
    }

    endgrent ();
    if (!mygrhead) {
        goto errorCleanup;
    }

    mygrentry = mygrhead;
    while (mygrentry) {
        for (num = 0; mygrentry->gr_entry.gr_mem[num]; num++) {
            ;
        }
        total_entries += num;
        mygrentry = mygrentry->gr_next;
    }
    
    
    if (!(grretentry = malloc (sizeof (struct group)))) {
        goto errorCleanup;
    }
    
    assert( total_entries + 1 >= 0 );
    (grretentry->gr_mem) = (char **) calloc (sizeof (char *), (unsigned long)(total_entries + 1) );
    if ( NULL == grretentry->gr_mem && ENOMEM == errno ) {
        goto errorCleanup;
    }
    
    
    grretentry->gr_name   = putstr_ (mygrhead->gr_entry.gr_name);
    grretentry->gr_passwd = putstr_ (mygrhead->gr_entry.gr_passwd);
    grretentry->gr_gid    = mygrhead->gr_entry.gr_gid;
    
    
    mygrentry = mygrhead;
    count = 0;
    while (mygrentry) {
        for (num = 0; mygrentry->gr_entry.gr_mem[num]; num++) {
            grretentry->gr_mem[count] = putstr_ (mygrentry->gr_entry.gr_mem[num]);
            if (!grretentry->gr_mem[count]) {
                goto errorCleanup;
            }
            count++;
        }
        mygrentry = mygrentry->gr_next;
    }
    
    grretentry->gr_mem[count] = NULL;
    
    
    while (mygrhead) {

        mygrentry = mygrhead->gr_next;
        FREEUP (mygrhead->gr_entry.gr_name);
        FREEUP (mygrhead->gr_entry.gr_passwd);
        for (count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
            FREEUP (mygrhead->gr_entry.gr_mem[count]);
        }
        FREEUP (mygrhead->gr_entry.gr_mem);
        FREEUP (mygrhead);
        mygrhead = mygrentry;
    }
    
    
    mygrhead = mygrentry = tmpgrentry = NULL;
    
    
    return grretentry;
    
errorCleanup:
    
    
    while (mygrhead) {
        mygrentry = mygrhead->gr_next;
        FREEUP (mygrhead->gr_entry.gr_name);
        FREEUP (mygrhead->gr_entry.gr_passwd);
        for (count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
            FREEUP (mygrhead->gr_entry.gr_mem[count]);
        }
        FREEUP (mygrhead->gr_entry.gr_mem);
        FREEUP (mygrhead);
        mygrhead = mygrentry;
    }
    
    
    freeUnixGrp (grretentry);
    grretentry = NULL;
    
    return NULL;
}


void
freeUnixGrp (struct group *unixGrp)
{
    int count;
    
    if (unixGrp)
        {
        FREEUP (unixGrp->gr_name);
        FREEUP (unixGrp->gr_passwd);
        for (count = 0; unixGrp->gr_mem[count]; count++) {
            FREEUP (unixGrp->gr_mem[count]);
        }
        FREEUP (unixGrp->gr_mem);
        FREEUP (unixGrp);
        unixGrp = NULL;
        }
    
    
}



struct group *
copyUnixGrp (struct group *unixGrp)
{
    struct group *unixGrpCopy;
    int count;
        
    unixGrpCopy = malloc( sizeof (struct group) );
    if ( NULL == unixGrpCopy && ENOMEM == errno ) {
        goto errorCleanup;
    }
    
    unixGrpCopy->gr_name = putstr_ (unixGrp->gr_name);
    unixGrpCopy->gr_passwd = putstr_ (unixGrp->gr_passwd);
    unixGrpCopy->gr_gid = unixGrp->gr_gid;
    
    for (count = 0; unixGrp->gr_mem[count]; count++){
        ;
    }
    
    assert( count + 1 >= 0 );
    unixGrpCopy->gr_mem = calloc( sizeof (char *), (count + 1) );
    if ( NULL == unixGrpCopy->gr_mem && ENOMEM == errno ) {
        goto errorCleanup;
    }
    
    for (count = 0; unixGrp->gr_mem[count]; count++) {
        unixGrpCopy->gr_mem[count] = putstr_ (unixGrp->gr_mem[count]);
    }
    unixGrpCopy->gr_mem[count] = NULL;
    
    return unixGrpCopy;
    
errorCleanup: // FIXME FIXME FIXME remove goto label
    freeUnixGrp (unixGrpCopy);
    unixGrpCopy = NULL;
    return NULL;
    
}

static void
addBinaryAttributes (char *confFile, size_t *lineNum, struct queueInfoEnt *queue, struct keymap *keylist, unsigned int attrib, char *attribName)
{
    if (keylist->val != NULL) {
        
        queue->qAttrib &= ~attrib;
        if (strcasecmp (keylist->val, "y") == 0 || strcasecmp (keylist->val, "yes") == 0) {
            queue->qAttrib |= attrib;
        }
        else {
            if (strcasecmp (keylist->val, "n") != 0 && strcasecmp (keylist->val, "no") != 0) {
                ls_syslog (LOG_ERR, "%s: File %s in section Queue ending at line %d: %s value <%s> isn't one of 'Y', 'y', 'n' or 'N'; ignored", __func__, confFile, *lineNum, attribName, keylist->val);
                lsberrno = LSBE_CONF_WARNING;
            }
            
        }
    }
}

static int
resolveBatchNegHosts (char *inHosts, char **outHosts, int isQueue)
{
    struct inNames **inTable = NULL;
    char **outTable  = NULL;
    char *buffer     = strdup (inHosts);
    char *save_buf   = buffer;
    char *word       = NULL;
    char *ptr_level  = NULL;
    int isAll        = FALSE;
    int result       = 0;
    unsigned int neg_num     = 0;
    unsigned int size        = 0;
    unsigned int in_num      = 0;
    unsigned int outTableSize = 0;
    unsigned int inTableSize  = 0;
    
    assert( isAll );
    assert( outTableSize );
    inTable = (struct inNames **)calloc ( (unsigned long)cConf->numHosts, sizeof (struct inNames *));
    if( NULL == inTable && ENOMEM == errno ) {
        if (result > -2) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            result = -1;
        }

        for ( unsigned int j = 0; j < in_num; j++) {
            if (inTable[j]) {
                if (inTable[j]->name) {
                    free (inTable[j]->name);
                }
                free (inTable[j]);
            }
        }
        free (inTable);
        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
        if (neg_num == 0) {
            FREEUP (outTable);
        }
        FREEUP (outHosts[0]);
        FREEUP (save_buf);
        FREEUP (ptr_level);

        return result;
    }
    else {
        inTableSize  = cConf->numHosts;
        fprintf( stderr, "we done goodfedup in %s\n", __func__);
    }

    // assert( cConf->numHosts >= 0 );
    outTable = (char **)calloc(  cConf->numHosts, sizeof (char *));
    if( NULL == outTable && ENOMEM == errno ) {
        if (result > -2) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            result = -1;
        }

        for ( unsigned int j = 0; j < in_num; j++) {
            if (inTable[j]) {
                if (inTable[j]->name) {
                    free (inTable[j]->name);
                }
                free (inTable[j]);
            }
        }
        free (inTable);
        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
        if (neg_num == 0) {
            FREEUP (outTable);
        }
        FREEUP (outHosts[0]);
        FREEUP (save_buf);
        FREEUP (ptr_level);

        return result;
    }
    else {
        outTableSize = cConf->numHosts;
        fprintf( stderr, "we done goofedup again in %s\n", __func__ );
    }
    
    if (!buffer || !inTable || !outTable) {
        if (result > -2) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            result = -1;
        }

        for ( unsigned int j = 0; j < in_num; j++) {
            if (inTable[j]) {
                if (inTable[j]->name) {
                    free (inTable[j]->name);
                }
                free (inTable[j]);
            }
        }
        free (inTable);
        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
        if (neg_num == 0) {
            FREEUP (outTable);
        }
        FREEUP (outHosts[0]);
        FREEUP (save_buf);
        FREEUP (ptr_level);

        return result;
    }
    
    while ((word = getNextWord_ (&buffer))) {
        if (word[0] == '~') {
            if (word[1] == '\0') {
                result = -2;
                if (result > -2) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                    result = -1;
                }

                for ( unsigned int j = 0; j < in_num; j++) {
                    if (inTable[j]) {
                        if (inTable[j]->name) {
                            free (inTable[j]->name);
                        }
                        free (inTable[j]);
                    }
                }
                free (inTable);
                freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                if (neg_num == 0) {
                    FREEUP (outTable);
                }
                FREEUP (outHosts[0]);
                FREEUP (save_buf);
                FREEUP (ptr_level);

                return result;
            }
            word++;
            
            if (isHostName (word) == FALSE) {
                unsigned int num = 0;
                char **grpMembers = expandGrp (word, &num, HOST_GRP);
                if (!grpMembers) {
                    if (result > -2) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                        result = -1;
                    }

                    for ( unsigned int j = 0; j < in_num; j++) {
                        if (inTable[j]) {
                            if (inTable[j]->name) {
                                free (inTable[j]->name);
                            }
                            free (inTable[j]);
                        }
                    }
                    free (inTable);
                    freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                    if (neg_num == 0) {
                        FREEUP (outTable);
                    }
                    FREEUP (outHosts[0]);
                    FREEUP (save_buf);
                    FREEUP (ptr_level);

                    return result;
                }
                
                if ((strcmp (word, grpMembers[0]) == 0)
                    && (strcmp (word, "all") != 0)
                    && (strcmp (word, "others") != 0)
                    && (strcmp (word, "none") != 0))
                {
                    
                    word--;
                    freeSA (grpMembers, num);
                    /* catgets 5905 */
                    ls_syslog (LOG_ERR, I18N (5905, "%s: host/group name \"%s\" is ignored."), __func__, word);
                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }
                
                for ( unsigned int j = 0; j < num; j++) {
                    if (!strcmp (grpMembers[j], "all")) {
                        
                        freeSA (grpMembers, num);
                        result = -3;
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int l = 0; l < in_num; l++) {
                            if (inTable[l]) {
                                if (inTable[j]->name) {
                                    free (inTable[l]->name);
                                }
                                free (inTable[l]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                    }
                    
                    outTable[neg_num] = strdup (grpMembers[j]);
                    if (!outTable[neg_num]) {
                        freeSA (grpMembers, num);
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int l = 0; l < in_num; l++) {
                            if (inTable[l]) {
                                if (inTable[l]->name) {
                                    free (inTable[l]->name);
                                }
                                free (inTable[l]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                    }
                    neg_num++;
                    
                    if (((neg_num - cConf->numHosts) % cConf->numHosts) == 0) {
                        //assert( cConf->numHosts +  neg_num >= 0 );
                        outTable = realloc (outTable, (unsigned long)(cConf->numHosts + neg_num) * sizeof (char *));
                        if ( NULL == outTable && ENOMEM == errno ) {
                                if (result > -2) {
                                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                    result = -1;
                                }

                                for ( unsigned int l = 0; l < in_num; l++) {
                                    if (inTable[l]) {
                                        if (inTable[l]->name) {
                                            free (inTable[l]->name);
                                        }
                                        free (inTable[l]);
                                    }
                                }
                                free (inTable);
                                freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                                if (neg_num == 0) {
                                    FREEUP (outTable);
                                }
                                FREEUP (outHosts[0]);
                                FREEUP (save_buf);
                                FREEUP (ptr_level);

                                return result;
                        }
                    }
                }
                freeSA (grpMembers, num);
            }
            else {
                outTable[neg_num] = strdup (word);
                if (!outTable[neg_num]) {
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int j = 0; j < in_num; j++) {
                            if (inTable[j]) {
                                if (inTable[j]->name) {
                                    free (inTable[j]->name);
                                }
                                free (inTable[j]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                }
                neg_num++;
                
                if (((neg_num - cConf->numHosts) % cConf->numHosts) == 0) {
                    //assert( cConf->numHosts +  neg_num >= 0 );
                    outTable = realloc (outTable, (unsigned long)(cConf->numHosts + neg_num) * sizeof (char *));
                    if ( NULL == outTable && ENOMEM == errno ) {
                            if (result > -2) {
                                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                result = -1;
                            }

                            for ( unsigned int j = 0; j < in_num; j++) {
                                if (inTable[j]) {
                                    if (inTable[j]->name) {
                                        free (inTable[j]->name);
                                    }
                                    free (inTable[j]);
                                }
                            }
                            free (inTable);
                            freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                            if (neg_num == 0) {
                                FREEUP (outTable);
                            }
                            FREEUP (outHosts[0]);
                            FREEUP (save_buf);
                            FREEUP (ptr_level);

                            return result;
                    }
                }
            }
        }
        else {
            unsigned int cur_size = 0;
            if (isQueue == TRUE) {
                ptr_level = strchr (word, '+');
            }
            
            if (ptr_level) {
                ptr_level[0] = 0;
                ptr_level++;
                
                if (strlen (ptr_level) && !isdigit (ptr_level[0])) {
                    result = -2;
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int j = 0; j < in_num; j++) {
                            if (inTable[j]) {
                                if (inTable[j]->name) {
                                    free (inTable[j]->name);
                                }
                                free (inTable[j]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                                    }

                ptr_level = strdup (ptr_level);
                if (!ptr_level) {
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int j = 0; j < in_num; j++) {
                            if (inTable[j]) {
                                if (inTable[j]->name) {
                                    free (inTable[j]->name);
                                }
                                free (inTable[j]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                }
            }
            
            if (!strcmp (word, "all")) {
                
                unsigned int miniTableSize = 0;
                isAll = TRUE;
                miniTableSize = in_num + numofhosts;

                // if (miniTableSize - inTableSize >= 0) {
                    // assert( cConf->numHosts +  miniTableSize >= 0 );
                    inTable = realloc (inTable, (unsigned long)(cConf->numHosts + miniTableSize) * sizeof (struct inNames *));
                    if ( NULL == inTable && ENOMEM == errno ) {
                            if (result > -2) {
                                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                result = -1;
                            }

                            for ( unsigned int j = 0; j < in_num; j++) {
                                if (inTable[j]) {
                                    if (inTable[j]->name) {
                                        free (inTable[j]->name);
                                    }
                                    free (inTable[j]);
                                }
                            }
                            free (inTable);
                            freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                            if (neg_num == 0) {
                                FREEUP (outTable);
                            }
                            FREEUP (outHosts[0]);
                            FREEUP (save_buf);
                            FREEUP (ptr_level);

                            return result;
                    }
                    else {
                        inTableSize = cConf->numHosts + miniTableSize;
                    }
                //}

                if (expandWordAll (&size, &in_num, inTable, ptr_level) == FALSE) {
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int j = 0; j < in_num; j++) {
                            if (inTable[j]) {
                                if (inTable[j]->name) {
                                    free (inTable[j]->name);
                                }
                                free (inTable[j]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                }
            }
            else if (isHostName (word) == FALSE) {
                unsigned int num = 0;
                char **grpMembers = expandGrp (word, &num, HOST_GRP);
                if (!grpMembers) {
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int j = 0; j < in_num; j++) {
                            if (inTable[j]) {
                                if (inTable[j]->name) {
                                    free (inTable[j]->name);
                                }
                                free (inTable[j]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                }
                
                if (!strcmp (grpMembers[0], "all")) {

                    unsigned int miniTableSize = 0;
                    isAll = TRUE;
                    miniTableSize = in_num + numofhosts;

                    //if (miniTableSize - inTableSize >= 0) {
                        // assert( cConf->numHosts +  miniTableSize >= 0 );
                        inTable = realloc (inTable, (unsigned long)(cConf->numHosts +  miniTableSize) * sizeof (struct inNames *));
                        if ( NULL == inTable && ENOMEM == errno ) {
                                if (result > -2) {
                                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                    result = -1;
                                }

                                for ( unsigned int j = 0; j < in_num; j++) {
                                    if (inTable[j]) {
                                        if (inTable[j]->name) {
                                            free (inTable[j]->name);
                                        }
                                        free (inTable[j]);
                                    }
                                }
                                free (inTable);
                                freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                                if (neg_num == 0) {
                                    FREEUP (outTable);
                                }
                                FREEUP (outHosts[0]);
                                FREEUP (save_buf);
                                FREEUP (ptr_level);

                                return result;
                        }
                        else {
                            inTableSize = cConf->numHosts + miniTableSize;
                        }
                    //}
                    if (expandWordAll (&size, &in_num, inTable, ptr_level) == FALSE) {
                            if (result > -2) {
                                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                result = -1;
                            }

                            for ( unsigned int j = 0; j < in_num; j++) {
                                if (inTable[j]) {
                                    if (inTable[j]->name) {
                                        free (inTable[j]->name);
                                    }
                                    free (inTable[j]);
                                }
                            }
                            free (inTable);
                            freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                            if (neg_num == 0) {
                                FREEUP (outTable);
                            }
                            FREEUP (outHosts[0]);
                            FREEUP (save_buf);
                            FREEUP (ptr_level);

                            return result;
                    }
                }
                else {
                    for ( unsigned int j = 0; j < num; j++) {
                        cur_size = fillCell_ (&inTable[in_num], grpMembers[j], ptr_level);
                        if (!cur_size) {
                                if (result > -2) {
                                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                    result = -1;
                                }

                                for ( unsigned int l = 0; l < in_num; l++) {
                                    if (inTable[l]) {
                                        if (inTable[l]->name) {
                                            free (inTable[l]->name);
                                        }
                                        free (inTable[l]);
                                    }
                                }
                                free (inTable);
                                freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                                if (neg_num == 0) {
                                    FREEUP (outTable);
                                }
                                FREEUP (outHosts[0]);
                                FREEUP (save_buf);
                                FREEUP (ptr_level);

                                return result;
                        }
                        size += cur_size;
                        in_num++;
                        
                        if ( 0 == in_num - inTableSize) {
                            // assert( cConf->numHosts + in_num >= 0 );
                            inTable =  realloc (inTable, (unsigned long)(cConf->numHosts + in_num) * sizeof (struct inNames *));
                            if ( NULL == inTable && ENOMEM == errno ) {
                                    if (result > -2) {
                                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                        result = -1;
                                    }

                                    for ( unsigned int l = 0; l < in_num; l++) {
                                        if (inTable[l]) {
                                            if (inTable[l]->name) {
                                                free (inTable[l]->name);
                                            }
                                            free (inTable[l]);
                                        }
                                    }
                                    free (inTable);
                                    freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                                    if (neg_num == 0) {
                                        FREEUP (outTable);
                                    }
                                    FREEUP (outHosts[0]);
                                    FREEUP (save_buf);
                                    FREEUP (ptr_level);

                                    return result;
                            }
                            else {
                                inTableSize = cConf->numHosts + in_num;
                            }
                        }
                    }
                }
                freeSA (grpMembers, num);
            }
            else {
                cur_size = fillCell_ (&inTable[in_num], word, ptr_level);
                if (!cur_size) {
                        if (result > -2) {
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                            result = -1;
                        }

                        for ( unsigned int j = 0; j < in_num; j++) {
                            if (inTable[j]) {
                                if (inTable[j]->name) {
                                    free (inTable[j]->name);
                                }
                                free (inTable[j]);
                            }
                        }
                        free (inTable);
                        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                        if (neg_num == 0) {
                            FREEUP (outTable);
                        }
                        FREEUP (outHosts[0]);
                        FREEUP (save_buf);
                        FREEUP (ptr_level);

                        return result;
                }
                size += cur_size;
                in_num++;
                
                if ( 0 == in_num - inTableSize ) {
                    // assert( cConf->numHosts +  in_num >= 0 );
                    inTable = realloc (inTable, (unsigned long)(cConf->numHosts +  in_num) * sizeof (struct inNames *));
                    if ( NULL == inTable && ENOMEM == errno ) {
                            if (result > -2) {
                                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                                result = -1;
                            }

                            for ( unsigned int j = 0; j < in_num; j++) {
                                if (inTable[j]) {
                                    if (inTable[j]->name) {
                                        free (inTable[j]->name);
                                    }
                                    free (inTable[j]);
                                }
                            }
                            free (inTable);
                            freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
                            if (neg_num == 0) {
                                FREEUP (outTable);
                            }
                            FREEUP (outHosts[0]);
                            FREEUP (save_buf);
                            FREEUP (ptr_level);

                            return result;
                    }
                    else {
                        inTableSize = cConf->numHosts + in_num;
                    }
                }
            }
            FREEUP (ptr_level);
        }
    }
    
    
    for ( unsigned int j = 0; j < neg_num; j++) {
        for ( unsigned int k = 0; k < in_num; k++) {

            unsigned long nameLen = 0;
            if (inTable[k] && inTable[k]->name) {
                nameLen = strlen (inTable[k]->name);
            }
            if (inTable[k] && inTable[k]->name && equalHost_ (inTable[k]->name, outTable[j])) {
                if (inTable[k]->prf_level) {
                    *(inTable[k]->prf_level - 1) = '+';
                }
                size -= (strlen (inTable[k]->name) + 1);
                FREEUP (inTable[k]->name);
                FREEUP (inTable[k]);
                result++;
            }
            else if ((nameLen > 1) && (inTable[k]->name[nameLen - 1] == '!')
                     && (!inTable[k]->prf_level)
                     && (isHostName (inTable[k]->name) == FALSE))
            {
                
                inTable[k]->name[nameLen - 1] = '\0';
                if (equalHost_ (inTable[k]->name, outTable[j])) {
                    size -= (strlen (inTable[k]->name) + 1);
                    FREEUP (inTable[k]->name);
                    FREEUP (inTable[k]);
                    result++;
                }
                else {
                    inTable[k]->name[nameLen - 1] = '!';
                }
            }
        }
        FREEUP (outTable[j]);
    }
    
    if (size <= 0) {
        
        result = -3;
        if (result > -2) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            result = -1;
        }
        for ( unsigned int j = 0; j < in_num; j++) {
            if (inTable[j]) {
                if (inTable[j]->name) {
                    free (inTable[j]->name);
                }
                free (inTable[j]);
            }
        }
        free (inTable);
        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
        if (neg_num == 0) {
            FREEUP (outTable);
        }
        FREEUP (outHosts[0]);
        FREEUP (save_buf);
        FREEUP (ptr_level);

        return result;

    }

    outHosts[0] = malloc ( (unsigned long)(size + in_num ) );
    if ( NULL == outHosts[0] && ENOMEM == errno ) {
        if (result > -2) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            result = -1;
        }

        for ( unsigned int j = 0; j < in_num; j++) {
            if (inTable[j]) {
                if (inTable[j]->name) {
                    free (inTable[j]->name);
                }
                free (inTable[j]);
            }
        }
        free (inTable);
        freeSA (outTable, neg_num); // FIXME FIXME FIXME FIXME FIXME Which freeSA function is called here?
        if (neg_num == 0) {
            FREEUP (outTable);
        }
        FREEUP (outHosts[0]);
        FREEUP (save_buf);
        FREEUP (ptr_level);
        return result;
    }
    outHosts[0][0] = 0;
    
    
    if (!result) {
        buffer = save_buf;
        while ((word = getNextWord_ (&buffer)) != NULL) {
            if (word[0] != '~') {
                strcat (outHosts[0], word);
                strcat (outHosts[0], " ");
            }
        }
        for ( unsigned int j = 0; j < in_num; j++) {
            if (inTable[j]) {
                if (inTable[j]->name) {
                    free (inTable[j]->name);
                }
                free (inTable[j]);
            }
        }
    }
    else {
        for ( unsigned int j = 0, k = 0; j < in_num; j++) {
            if (inTable[j] && inTable[j]->name) {
                if (inTable[j]->prf_level) {
                    *(inTable[j]->prf_level - 1) = '+';
                }
                
                strcat (outHosts[0], (const char *) inTable[j]->name);
                FREEUP (inTable[j]->name);
                FREEUP (inTable[j]);
                strcat (outHosts[0], " ");
                k++;
            }
        }
    }

    if (outHosts[0][0]) {
        outHosts[0][strlen (outHosts[0]) - 1] = '\0';
    }

    free (inTable);
    free (outTable);
    free (save_buf);

    return result;
}


int
checkJobAttaDir (char *path)
{
    struct stat statBuf;
    unsigned long len = strlen (path);

    if (checkSpoolDir (path) == -1) {
        return FALSE;
    }

    if (len > 0 && (path[len - 1] == '/' || path[len - 1] == '\\')) {
        path[len - 1] = '\0';
    }

    if ( 0 == stat (path, &statBuf) && S_ISDIR (statBuf.st_mode) ) {
            return TRUE;
    }

    return FALSE;
}



static int
parseDefAndMaxLimits (struct keymap key, int *defaultVal, int *maxVal, char *filename, size_t *lineNum, char *pname)
{
    
    char *sp;
    char *defaultLimit = NULL;
    char *maxLimit = NULL;
    
    sp = key.val;
    
    if (sp != NULL) {
        defaultLimit = putstr_ (getNextWord_ (&sp));
        maxLimit = getNextWord_ (&sp);
    }
    
    if (maxLimit != NULL) {

        *defaultVal = my_atoi (defaultLimit, INFINIT_INT, 0);
        if (INFINIT_INT == *defaultVal ) {
            /* catgets 5387 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5387, "[%s] %s: File %s in section Queue at line %d: Default value <%s> of %s isn't a positive integer between 0 and %d; ignored."), __func__, pname, filename, *lineNum, defaultLimit, key.key, INFINIT_INT);
            lsberrno = LSBE_CONF_WARNING;
        }
        
    }
    else if (!maxLimit && defaultLimit) {
        maxLimit = defaultLimit;
    }
    
    if ((*maxVal = my_atoi (maxLimit, INFINIT_INT, 0)) == INFINIT_INT) {
        /* catgets 5388 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5388, "[%s] %s: File %s in section Queue at line %d: Maximum value <%s> of %s isn't a positive integer between 0 and %d; ignored."), __func__, pname, filename, *lineNum, maxLimit, key.key, INFINIT_INT);
        lsberrno = LSBE_CONF_WARNING;
    }
    
    FREEUP (defaultLimit);
    
    if ((*defaultVal != INFINIT_INT) && (*maxVal != INFINIT_INT) && (*defaultVal > *maxVal)) {
        /* catgets 5112 */
        ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5112, "[%s] %s: File %s in section Queue at line %d: The default %s %d should not be greater than the max %d; ignoring the default and using max value also as default %s"), __func__, pname, filename, *lineNum, key.key, *defaultVal, *maxVal, key.key);
        *defaultVal = *maxVal;
    }
    return 0;
    
}


static int
parseQFirstHost (char *myWord, int *haveFirst, char *pname, size_t *lineNum, char *filename, char *section)
{
    struct groupInfoEnt *gp;
    int needCheck = TRUE;
    
    if (chkFirstHost (myWord, &needCheck))
        {
        
        if (*haveFirst) {
            /* catgets 5439 */
            ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5439, "[%s] %s: File %s%s ending at line %d : Multiple first execution hosts specified:<%s>; ignored."), __func__, pname, filename, section, *lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return 1;
        }
        if (needCheck)
            {
            if (!strcmp (myWord, "others"))
                {
                /* catgets 5900 */
                ls_syslog (LOG_ERR, I18N (5900, "[%s] %s: File %s%s ending at line %d : \"others\" specified as first execution host; ignored."), __func__, pname, filename, section, *lineNum);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return 1;
                }
            if (!strcmp (myWord, "all"))
                {
                /* catgets 5901 */
                ls_syslog (LOG_ERR, I18N (5901, "[%s] %s: File %s%s ending at line %d : \"all\" specified as first execution host; ignored."), __func__, pname, filename, section, *lineNum);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return 1;
                }
            
            gp = getHGrpData (myWord);
            if (gp != NULL)
                {
                /* catgets 5902 */
                ls_syslog (LOG_ERR, I18N (5902, "[%s] %s: File %s%s ending at line %d : host group <%s> specified as first execution host; ignored."),__func__, pname, filename, section, *lineNum, myWord);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return 1;
            }
            /* catgets 5904 */
            ls_syslog (LOG_ERR, I18N (5904, "[%s] %s: File %s%s ending at line %d : Invalid first execution host <%s>, not a valid host name; ignored."), __func__, pname, filename, section, *lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return 1;
        }
        *haveFirst = TRUE;
    }

    return 0;
}

int
chkFirstHost (char *host, int *needChk)
{
#define FIRST_HOST_TOKEN '!'
    unsigned long len = 0;
    
    len = strlen (host);
    if (host[len - 1] == FIRST_HOST_TOKEN) {
        
        if (isHostName (host) && getHostData (host)) {
            
            return FALSE;
        }
        else {
            host[len - 1] = '\0';
            if (isHostName (host) && getHostData (host)) {
                *needChk = FALSE;
            }
            return TRUE;
        }
    }
    else {
        return FALSE;
    }
}

void
updateClusterConf (struct clusterConf *clusterConf)
{
    cConf = clusterConf;
}
