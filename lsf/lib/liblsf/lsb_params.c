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
#include <stdbool.h>
#include <math.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#include <ctype.h>
#include <strings.h>

#include "lsb/lsbatch.h" // struct parameterInfo
// #include "lsb/sig.h"
#include "libint/intlibout.h"
#include "lsb/conf.h"
#include "lib/syslog.h"
#include "lib/do_hosts.h"
#include "lib/conf_lsb.h"
#include "lib/confmisc.h"    // putValue( )
#include "lib/words.h"       // getNextWord_( )
#include "lib/getnextline.h" // getNextLineC_conf( )
#include "lib/misc.h"        // putstr_( )
// #define  NL_SETN     13

void
freeSA ( char **list, unsigned int num)
{
    if (list == NULL || num <= 0) {
        return;
    }

    for( unsigned int i = 0; i < num; i++) {
        FREEUP (list[i]);
    }
    FREEUP (list);
}

size_t
fillCell_ (struct inNames **table, const char *name, const char *level)
{
    size_t size = 0;

    table[0] = malloc (sizeof (struct inNames)); // FIXME FIXME replace 0 with enum label
    if ( NULL == table[0] && ENOMEM == errno ) { // FIXME FIXME replace 0 with enum label
        const char malloc[] = "malloc";
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, malloc);
        lserrno = LSE_MALLOC;
        return 0;
    }

    assert( strlen( name + 1 ) <= UINT_MAX );
    size = strlen (name) + 1;
    if (level) {
        size += strlen (level) + 1;
    }
    table[0]->name = malloc( size ); // FIXME FIXME FIXME find what table[0] is and replace it with enum label
    if ( NULL == table[0]->name && ENOMEM == errno ) { // FIXME FIXME FIXME find what table[0] is and replace it with enum label
        FREEUP (table[0]);           // FIXME FIXME FIXME find what table[0] is and replace it with enum label
        const char malloc[] = "malloc";
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, malloc);
        lserrno = LSE_MALLOC;
        return 0;
    }

    strcpy(table[0]->name, name); // FIXME FIXME replace 0 with enum label
    if (level) {
        const char plus[ ] = "+";
        strcat (table[0]->name, plus ); // FIXME FIXME FIXME find what table[0] is and replace it with enum label
        strcat (table[0]->name, level); // FIXME FIXME FIXME find what table[0] is and replace it with enum label
    }

    if (level) {
        table[0]->prf_level = strchr (table[0]->name, '+') + 1; // FIXME FIXME FIXME find what table[0] is and replace it with enum label
        *(table[0]->prf_level - 1) = 0; // FIXME FIXME FIXME find what table[0] is and replace it with enum label
    }
    else {
        table[0]->prf_level = NULL; // FIXME FIXME FIXME find what table[0] is and replace it with enum label
    }

    return size;
}


void
initHostInfoEnt (struct hostInfoEnt *hp)
{
    if (hp != NULL) {
        hp->host         = NULL;
        hp->loadSched    = NULL;
        hp->loadStop     = NULL;
        hp->windows      = NULL;
        hp->load         = NULL;
        hp->hStatus      = INFINIT_INT;
        hp->busySched    = NULL;
        hp->busyStop     = NULL;
        hp->cpuFactor    = INFINIT_FLOAT;
        hp->nIdx         = 0;
        hp->userJobLimit = INFINIT_INT;
        hp->maxJobs      = INFINIT_INT;
        hp->numJobs      = INFINIT_INT;
        hp->numRUN       = INFINIT_INT;
        hp->numSSUSP     = INFINIT_INT;
        hp->numUSUSP     = INFINIT_INT;
        hp->mig          = INFINIT_INT;
        hp->attr         = INFINIT_INT;
        hp->chkSig       = INFINIT_INT;
    }
}

void
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


unsigned int
expandWordAll (size_t *size, unsigned int *num, struct inNames **inTable, const char *ptr_level)
{
    size_t cur_size = 0;

    if (numofhosts) {

        for ( unsigned int j = 0; j < numofhosts; j++) {
            cur_size = fillCell_ (&inTable[*num], hosts[j]->host, ptr_level);
            if (!cur_size) {
                return false;
            }
            *size += cur_size;
            (*num)++;
        }
    }
    else {
        for ( unsigned int j = 0; j < cConf->numHosts; j++)  {
            cur_size = fillCell_( &inTable[*num], cConf->hosts[j].hostName, ptr_level);
            if (!cur_size)  {
                return false;
            }
            *size += cur_size;
            (*num)++;
        }
    }
    
    return true;
}


int
readHvalues_conf (struct keymap *keyList, const char *linep, struct lsConf *conf, const char *lsfile, size_t *lineNumber, int exact, const char *section)
{
    char *key   = NULL;
    char *value = NULL;
    char *sp    = NULL;
    char *sp1   = NULL;
    char error  = false;

    if (linep == NULL || conf == NULL) {
        return -1;
    }

    sp = strdup( linep ); // FIXME FIXME FIXME FIXME this needs to go to the debugger, cuz it might just be a pointer, not a copy of the string.
    key = getNextWord_ (&linep);
    if ((sp1 = strchr (key, '=')) != NULL) {
        sp1 = NULL;
    }

    value = strchr (sp, '=');
    if (!value) {
        /* catgets 5414 */
        ls_syslog (LOG_ERR, "catgets 5414: %s: %s(%d): missing '=' after keyword %s, section %s ignoring the line", __func__, lsfile, *lineNumber, key, section);
        lsberrno = LSBE_CONF_WARNING;
    }
    else {
        value++;
        while (*value == ' ') {
            value++;
        }

        if (value == NULL ) {
            /* catgets 5415 */
            ls_syslog (LOG_ERR, "catgets 5415: %s: %s(%d): null value after keyword %s, section %s ignoring the line", __func__, lsfile, *lineNumber, key, section);
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
                ls_syslog (LOG_ERR, "catgets 5416: %s: %s(%d): bad keyword %s in section %s, ignoring the line", __func__, lsfile, *lineNumber, key, section);  
                lsberrno = LSBE_CONF_WARNING;
                }
            }
        }

    if ((linep = getNextLineC_conf (conf, lineNumber, true)) != NULL) {
        if (isSectionEnd (linep, lsfile, lineNumber, section)) {

            unsigned int i = 0;
            if (!exact) {
                return 0;
            }

            while (keyList[i].key != NULL) {
                if (keyList[i].val == NULL) {
                    /* catgets 5417 */
                    ls_syslog (LOG_ERR, "catgets 5417: %s: %s(%d): required keyword %s is missing in section %s, ignoring the section", __func__, lsfile, *lineNumber, keyList[i].key, section);
                    error = true;
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

    /* catgets 33 */
    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, lsfile, *lineNumber, section);
    return -1;
}


struct parameterInfo *
lsb_readparam (struct lsConf *conf)
{
    size_t lineNum = 0;
    char *filename = NULL;
    char *section  = NULL;
    char paramok   = 'a';

    const char sectionName[]    = "parameters";
    const char unknownSection[] = "unknown";


    lsberrno = LSBE_NO_ERROR;

    if (conf == NULL) {
        ls_syslog (LOG_ERR, "catgets 5050: %s(): struct lsConf *conf is NULL.\n", __func__ );
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (conf->confhandle == NULL) {
        ls_syslog (LOG_ERR, "catgets 5050: %s(): conf->confhandle is NULL.\n" , __func__);
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (pConf) {
        freeParameterInfo (pConf);
        FREEUP (pConf);
    }
    else {
        pConf = malloc (sizeof (struct parameterInfo));
        if ( NULL == pConf && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, sizeof (struct parameterInfo));
            lsberrno = LSBE_CONF_FATAL;
            return NULL;
        }
        // pConf = NULL;
    }

    // filename = conf->confhandle->fname;
    // // pConf = malloc(sizeof (struct parameterInfo));
    // if ( NULL == pConf->param && ENOMEM == errno ) {
    //     ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, sizeof (struct parameterInfo));
    //     lsberrno = LSBE_CONF_FATAL;
    //     return NULL;
    // }
    // initParameterInfo (pConf->param);
    initParameterInfo( pConf );


    conf->confhandle->curNode = conf->confhandle->rootNode;
    conf->confhandle->lineCount = 0;
    paramok = false;

    for (;;) { // FIXME FIXME FIXME FIXME replace infinite loop with a ccertain-to-terminate condition
        const char *cp = NULL;
        if ((cp = getBeginLine_conf (conf, &lineNum)) == NULL) {
            if (paramok == false) {
                /* catgets 5056 */
                ls_syslog (LOG_ERR, "catgets 5056: %s: File %s at line %d: No valid parameters are read", __func__, filename, lineNum);
                lsberrno = LSBE_CONF_WARNING;
            }
            return pConf;
        }

        section = getNextWord_ (&cp);
        if (!section) {
            /* catgets 5057 */
            ls_syslog (LOG_ERR, "catgets 5057: %s: File %s at line %d: Section name expected after Begin; ignoring section", __func__, filename, lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, unknownSection);
            continue;
        }
        else {
            if (strcasecmp (section, sectionName) == 0)  {
                if (do_Param (conf, filename, &lineNum)) {
                    paramok = true;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    return NULL;
                }

                continue;
            }
            /* catgets 5058 */
            ls_syslog (LOG_ERR, "catgets 5058: %s: File %s at line %d: Invalid section name <%s>; ignoring section", __func__, filename, lineNum, section);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, section);

            continue;
        }
    }

    return NULL;
}


char
do_Param (struct lsConf *conf, const char *filename, size_t *lineNum) // this function is basically split from the one above it, lsb_readparam (struct lsConf *conf)) because lsb_readparam (struct lsConf *conf() was fucking chaotic
{
    char *linep = NULL;

    enum state { // 35 items + NULL
        LSB_MANAGER                      =  0,
        DEFAULT_QUEUE                    =  1,
        DEFAULT_HOST_SPEC                =  2,
        DEFAULT_PROJECT                  =  3,
        JOB_ACCEPT_INTERVAL              =  4,
        PG_SUSP_IT                       =  5,
        MBD_SLEEP_TIME                   =  6,
        CLEAN_PERIOD                     =  7,
        MAX_RETRY                        =  8,
        SBD_SLEEP_TIME                   =  9,
        MAX_JOB_NUM                      = 10,
        RETRY_INTERVAL                   = 11,
        MAX_SBD_FAIL                     = 12,
        RUSAGE_UPDATE_RATE               = 13,
        RUSAGE_UPDATE_PERCENT            = 14,
        COND_CHECK_TIME                  = 15,
        MAX_SBD_CONNS                    = 16,
        MAX_SCHED_STAY                   = 17,
        FRESH_PERIOD                     = 18,
        MAX_JOB_ARRAY_SIZE               = 19,
        DISABLE_UACCT_MAP                = 20,
        JOB_TERMINATE_INTERVAL           = 21,
        JOB_RUN_TIMES                    = 22,
        JOB_DEP_LAST_SUB                 = 23,
        JOB_SPOOL_DIR                    = 24,
        MAX_USER_PRIORITY                = 25,
        JOB_PRIORITY_OVER_TIME           = 26,
        SHARED_RESOURCE_UPDATE_FACTOR    = 27,
        SCHE_RAW_LOAD                    = 28,
        PRE_EXEC_DELAY                   = 29,
        SLOT_RESOURCE_RESERVE            = 30,
        MAX_JOBID                        = 31,
        MAX_ACCT_ARCHIVE_FILE            = 32,
        ACCT_ARCHIVE_SIZE                = 33,
        ACCT_ARCHIVE_AGE                 = 34,
        STATE_NULL
    };

    const char *keylist[ ] = { 
        "LSB_MANAGER",                   //  0
        "DEFAULT_QUEUE",                 //  1
        "DEFAULT_HOST_SPEC",             //  2
        "DEFAULT_PROJECT",               //  3
        "JOB_ACCEPT_INTERVAL",           //  4
        "PG_SUSP_IT",                    //  5
        "MBD_SLEEP_TIME",                //  6
        "CLEAN_PERIOD",                  //  7
        "MAX_RETRY",                     //  8
        "SBD_SLEEP_TIME",                //  9
        "MAX_JOB_NUM",                   // 10
        "RETRY_INTERVAL",                // 11
        "MAX_SBD_FAIL",                  // 12
        "RUSAGE_UPDATE_RATE",            // 13
        "RUSAGE_UPDATE_PERCENT",         // 14
        "COND_CHECK_TIME",               // 15
        "MAX_SBD_CONNS",                 // 16
        "MAX_SCHED_STAY",                // 17
        "FRESH_PERIOD",                  // 18
        "MAX_JOB_ARRAY_SIZE",            // 19
        "DISABLE_UACCT_MAP",             // 20
        "JOB_TERMINATE_INTERVAL",        // 21
        "JOB_RUN_TIMES",                 // 22
        "JOB_DEP_LAST_SUB",              // 23
        "JOB_SPOOL_DIR",                 // 24
        "MAX_USER_PRIORITY",             // 25
        "JOB_PRIORITY_OVER_TIME",        // 26
        "SHARED_RESOURCE_UPDATE_FACTOR", // 27
        "SCHE_RAW_LOAD",                 // 28
        "PRE_EXEC_DELAY",                // 29
        "SLOT_RESOURCE_RESERVE",         // 30
        "MAX_JOBID",                     // 31
        "MAX_ACCT_ARCHIVE_FILE",         // 32
        "ACCT_ARCHIVE_SIZE",             // 33
        "ACCT_ARCHIVE_AGE",              // 34
        NULL
    };

    struct keymap keyList[36] = {
        { LSB_MANAGER,                   "    ", keylist[LSB_MANAGER],                   NULL },
        { DEFAULT_QUEUE,                 "    ", keylist[DEFAULT_QUEUE],                 NULL },
        { DEFAULT_HOST_SPEC,             "    ", keylist[DEFAULT_HOST_SPEC],             NULL },
        { DEFAULT_PROJECT,               "    ", keylist[DEFAULT_PROJECT],               NULL },
        { JOB_ACCEPT_INTERVAL,           "    ", keylist[JOB_ACCEPT_INTERVAL],           NULL },
        { PG_SUSP_IT,                    "    ", keylist[PG_SUSP_IT],                    NULL },
        { MBD_SLEEP_TIME,                "    ", keylist[MBD_SLEEP_TIME],                NULL },
        { CLEAN_PERIOD,                  "    ", keylist[CLEAN_PERIOD],                  NULL },
        { MAX_RETRY,                     "    ", keylist[MAX_RETRY],                     NULL },
        { SBD_SLEEP_TIME,                "    ", keylist[SBD_SLEEP_TIME],                NULL },
        { MAX_JOB_NUM,                   "    ", keylist[MAX_JOB_NUM],                   NULL },
        { RETRY_INTERVAL,                "    ", keylist[RETRY_INTERVAL],                NULL },
        { MAX_SBD_FAIL,                  "    ", keylist[MAX_SBD_FAIL],                  NULL },
        { RUSAGE_UPDATE_RATE,            "    ", keylist[RUSAGE_UPDATE_RATE],            NULL }, //* control how often sbatchd
        { RUSAGE_UPDATE_PERCENT,         "    ", keylist[RUSAGE_UPDATE_PERCENT],         NULL }, //* report job rusage to mbd
        { COND_CHECK_TIME,               "    ", keylist[COND_CHECK_TIME],               NULL }, //* time to check conditions
        { MAX_SBD_CONNS,                 "    ", keylist[MAX_SBD_CONNS],                 NULL }, //* Undocumented parameter for
                                                                                                 //* specifying how many sbd
                                                                                                 //* connections to keep around
        { MAX_SCHED_STAY,                "    ", keylist[MAX_SCHED_STAY],                NULL },
        { FRESH_PERIOD,                  "    ", keylist[FRESH_PERIOD],                  NULL },
        { MAX_JOB_ARRAY_SIZE,            "    ", keylist[MAX_JOB_ARRAY_SIZE],            NULL },
        { DISABLE_UACCT_MAP,             "    ", keylist[DISABLE_UACCT_MAP],             NULL },
        { JOB_TERMINATE_INTERVAL,        "    ", keylist[JOB_TERMINATE_INTERVAL],        NULL },
        { JOB_RUN_TIMES,                 "    ", keylist[JOB_RUN_TIMES],                 NULL },
        { JOB_DEP_LAST_SUB,              "    ", keylist[JOB_DEP_LAST_SUB],              NULL },
        { JOB_SPOOL_DIR,                 "    ", keylist[JOB_SPOOL_DIR],                 NULL },
        { MAX_USER_PRIORITY,             "    ", keylist[MAX_USER_PRIORITY],             NULL },
        { JOB_PRIORITY_OVER_TIME,        "    ", keylist[JOB_PRIORITY_OVER_TIME],        NULL },
        { SHARED_RESOURCE_UPDATE_FACTOR, "    ", keylist[SHARED_RESOURCE_UPDATE_FACTOR], NULL },
        { SCHE_RAW_LOAD,                 "    ", keylist[SCHE_RAW_LOAD],                 NULL },
        { PRE_EXEC_DELAY,                "    ", keylist[PRE_EXEC_DELAY],                NULL },
        { SLOT_RESOURCE_RESERVE,         "    ", keylist[SLOT_RESOURCE_RESERVE],         NULL },
        { MAX_JOBID,                     "    ", keylist[MAX_JOBID],                     NULL },
        { MAX_ACCT_ARCHIVE_FILE,         "    ", keylist[MAX_ACCT_ARCHIVE_FILE],         NULL },
        { ACCT_ARCHIVE_SIZE,             "    ", keylist[ACCT_ARCHIVE_SIZE],             NULL },
        { ACCT_ARCHIVE_AGE,              "    ", keylist[ACCT_ARCHIVE_AGE],              NULL },
        { UINT_MAX,                      "    ", NULL,                                   NULL }
    };

    const char sectionName[] = "parameters";

    if (conf == NULL) {
        return false;
    }

    //
    // set up the local buffer with the current line
    //
    linep = getNextLineC_conf (conf, lineNum, true);
    if (logclass & LC_EXEC)  {
        ls_syslog (LOG_DEBUG, "%s: file %s: the linep is %s, and %d \n", __func__, filename, linep, strlen (linep));
    }

    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 5050: %s(): filename %s, line number %l: input line is empty.\n", __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (isSectionEnd (linep, filename, lineNum, sectionName )) {
        ls_syslog (LOG_WARNING, "%s: File %s at line %d: Empty %s section\n", __func__, filename, *lineNum, sectionName );
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (strchr (linep, '=') == NULL) {
        /* catgets 5059 */
        ls_syslog (LOG_ERR, "catgets 5059: %s: File %s at line %d: Vertical parameters section is not implemented yet; use horizontal format; ignoring section", __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, sectionName );
        return false;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 
    //  MAIN BODY OF FUNCTION

    //
    // Extract the (somewhat generic) key values from current line FIXME FIXME FIXME FIXME why do we pass the damn file name?
    //
    if (readHvalues_conf (keyList, linep, conf, filename, lineNum, false, sectionName ) < 0) {
        /* catgets 5060 */
        ls_syslog (LOG_ERR, "catgets 5060: %s: File %s at line %d: Incorrect section; ignored", __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        freekeyval (keyList);
        return false;
    }


    //
    // assign the (somewhat generic) key values to specific values to pConf->param
    //
    //      not quite sure if this is the most efficient way, but i will go along for now.
    //
    for ( unsigned int i = 0; keyList[i].key != NULL; i++) {

        // unsigned int value   = 0;

        if (keyList[i].val != NULL && strcmp (keyList[i].val, "")) {
            if (i == LSB_MANAGER) { // i == 0 // IGNORRED
                /* catgets 5061 */
                ls_syslog (LOG_WARNING, "catgets 5061: %s: Ignore LSB_MANAGER value <%s>; use MANAGERS  defined in cluster file instead", filename, keyList[i].val ); 
                lsberrno = LSBE_CONF_WARNING;
            }
            else if (i == DEFAULT_QUEUE) { // i == 1 
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.default_project.5.html
                //
                // DEFAULT_PROJECT=project_name (string)
                //
                // Description; The name of the default project. Project names can be up to 59 characters long. When you submit a job without specifying any project name, and the environment variable LSB_DEFAULTPROJECT is not set, LSF automatically assigns the job to this project.
                pConf->defaultQueues = putstr_ (keyList[i].val); // struct paramConf *pConf in lib/conf_lsb.h 
                if (pConf->defaultQueues == NULL) { //
                    const char malloc[] = "malloc";
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen (keyList[i].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    return false;
                }
            }
            else if (i == DEFAULT_HOST_SPEC ) { // i == 2 // char *defaultHostSpec;
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.default_host_spec.5.html
                //
                // DEFAULT_HOST_SPEC=host_name | host_model
                //
                // Description: The default CPU time normalization host for the cluster.
                //
                // The CPU factor of the specified host or host model will be used to normalize the CPU time limit of all jobs in the cluster, unless the CPU time normalization host is specified at the queue or job level.
                //
                pConf->defaultHostSpec = putstr_ (keyList[i].val);  // struct paramConf *pConf in lib/conf_lsb.h
                if (pConf->defaultHostSpec == NULL) {
                    const char malloc[] = "malloc";
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen (keyList[i].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    return false;
                }
            }
            else if (i == JOB_SPOOL_DIR ) { // i == 24 // char *pjobSpoolDir;
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.job_spool_dir.5.html
                // 
                // JOB_SPOOL_DIR=dir (string)
                //
                // Description: Specifies the directory for buffering batch standard output and standard error for a job. When JOB_SPOOL_DIR is defined, the standard output and standard error for the job is buffered in the specified directory.
                //      The entire path including JOB_SPOOL_DIR can up to 4094 characters on UNIX and Linux or up to 255 characters for Windows.
                //      Batch job output (standard output and standard error) is sent to the .lsbatch directory on the execution host:
                //          * On UNIX: $HOME/.lsbatch
                //          * On Windows: %windir%\lsbtmp$<user_id>\.lsbatch
                //  
                if (checkSpoolDir (keyList[i].val) == 0) {
                    // this is is already in string format
                    pConf->pjobSpoolDir = strdup(keyList[i].val);

                    if (pConf->pjobSpoolDir == NULL) {
                        ls_syslog (LOG_ERR, "%s: ENOMEM in malloc for JOB_SPOOL_DIR: strlen (keyList[i].val): %u", __func__, strlen (keyList[i].val) + 1);
                        lsberrno = LSBE_NO_MEM;
                        freekeyval (keyList);
                        return false;
                    }
                }
                else {
                    // 
                    // Throw an error if it does not
                    // 
                    pConf->pjobSpoolDir = NULL; // struct paramConf *pConf in lib/conf_lsb.h
                    /* catgets 5095 */
                    ls_syslog (LOG_ERR, "catgets 5095: %s: Invalid JOB_SPOOL_DIR!", __func__);
                    lsberrno = LSBE_CONF_WARNING;
                }
            }
            else if (i == MAX_ACCT_ARCHIVE_FILE ) { // i == 32
                //
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.max_acct_archive_file.5.html
                //
                // MAX_ACCT_ARCHIVE_FILE=integer  // WARNING COMPATIBILITY
                //
                // Description: Enables automatic deletion of archived LSF accounting log files and specifies the archive limit.
                //
                unsigned long  maxAcctArchiveNum = strtoul( keyList[i].val, NULL , 10 ); // FIXME FIXME FIXME might want to include char **endptr, as it might end up making the code smaller. Also, what happens when someone enters a string that starts with a '+' or a '-' // 10 is for decimal base.
                if (maxAcctArchiveNum == ULONG_MAX && errno ==  ERANGE ) {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, "catgets 5459: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored" , __func__, filename, *lineNum, keyList[i].val, keyList[i].key, INFINIT_INT - 1);
                    // pConf->maxAcctArchiveNum = ULONG_MAX; // struct paramConf *pConf in lib/conf_lsb.h
                    lsberrno = LSBE_CONF_WARNING;
                }
                else {
                    pConf->maxAcctArchiveNum = maxAcctArchiveNum;
                }
            }
            else if (i == ACCT_ARCHIVE_SIZE ) { // i == 33
                //
                //  https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.acct_archive_size.5.html
                //
                // ACCT_ARCHIVE_SIZE=kilobytes // WARNING Get rid of this: LSF should not manage logs, that's what syslog is for.
                //
                // Description: Enables automatic archiving of LSF accounting log files, and specifies the archive threshold. LSF archives the current log file if its size exceeds the specified number of kilobytes.
                //
                // WARNING Must go, syslog can take care of these things better than LSF.
                //
                unsigned long acctArchiveInSize = strtoul( keyList[i].val, NULL, 10 );
                if (acctArchiveInSize == ULONG_MAX && errno ==  ERANGE ) {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, "catgets 5459: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key, INFINIT_INT - 1);
                    pConf->acctArchiveInSize = 0; // 0 will be a special value meaning "none"
                    lsberrno = LSBE_CONF_WARNING;
                }
                else {
                    pConf->acctArchiveInSize = acctArchiveInSize;
                }
            }
            else if (i == ACCT_ARCHIVE_AGE) { // i == 34
                //
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.acct_archive_age.5.html
                // 
                // ACCT_ARCHIVE_AGE=non-negative_integer (representing days)
                //
                // Description: Enables automatic archiving of LSF accounting log files, and specifies the archive interval. LSF archives the current log file if the length of time from its creation date exceeds the specified number of days.
                //
                // WARNING Must go, syslog can take of these things better than LSF.
                //
                unsigned long acctArchiveInDays = strtoul( keyList[i].val, NULL , 10 );
                if( acctArchiveInDays == ULONG_MAX && errno ==  ERANGE ) {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, "catgets 5459: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key, INFINIT_INT - 1);
                    pConf->acctArchiveInDays = 0; // 0 will be a special value meaning "inactive"
                    lsberrno = LSBE_CONF_WARNING;
                }
                else {
                    pConf->acctArchiveInDays = acctArchiveInDays;
                }
            }
            else if (i == DEFAULT_PROJECT ) { // i == 3
                //
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.default_project.5.html
                //
                // DEFAULT_PROJECT=project_name (string)
                //
                // Description: The name of the default project. Specify any string up to 59 chars long. When you submit a job without specifying any project name, and the environment variable LSB_DEFAULTPROJECT is not set, LSF automatically assigns the job to this project.
                //
                pConf->defaultProject = putstr_ (keyList[i].val); // struct paramConf *pConf in lib/conf_lsb.h
                if (pConf->defaultProject == NULL) {
                    const char malloc[] = "malloc";
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen (keyList[i].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    return false;
                }
            }
            else if (i == JOB_ACCEPT_INTERVAL ) { // || i == PG_SUSP_IT ) { // i == 4 || i == 5
                //
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.job_accept_interval.5.html
                //
                // JOB_ACCEPT_INTERVAL=non-negative_integer
                //
                // Description: The number you specify is multiplied by the value of lsb.params MBD_SLEEP_TIME (60 seconds by default). The result of the calculation is the number of seconds to wait after dispatching a job to a host, before dispatching a second job to the same host. If 0 (zero), a host may accept more than one job. By default, there is no limit to the total number of jobs that can run on a host, so if this parameter is set to 0, a very large number of jobs might be dispatched to a host all at once. // FIXME FIXME FIXME FIXME automate the calculation of this parameter according to load and ability of the host
                //
                // Default: Set to 0 at time of installation. If otherwise undefined, then set to 1.
                //
                unsigned long jobAcceptInterval = strtoul( keyList[i].val, NULL, -10 );
                if ( jobAcceptInterval == ULONG_MAX && errno ==  ERANGE ) {
                    /* catgets 5067 */
                    ls_syslog (LOG_ERR, "catgets 5067: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a non-negative integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key, ULONG_MAX);
                    lsberrno = LSBE_CONF_WARNING;
                }
                else if (i == JOB_ACCEPT_INTERVAL) { // i == 4
                    pConf->jobAcceptInterval = jobAcceptInterval; // struct paramConf *pConf in lib/conf_lsb.h
                }
            }
            else if ( i == PG_SUSP_IT ) { // PG_SUSP_IT , i == 5
                // 
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.3/lsf_config_ref/lsb.params.pg_susp_it.5.html
                //
                // PG_SUSP_IT=seconds (time_t/non-integer)
                //
                // The time interval that a host should be interactively idle (it > 0) before jobs suspended because of a threshold on the pg load index can be resumed.This parameter is used to prevent the case in which a batch job is suspended and resumed too often as it raises the paging rate while running and lowers it while suspended. If you are not concerned with the interference with interactive jobs caused by paging, the value of this parameter may be set to 0.
                //
                unsigned long pgSuspendIt = strtoul( keyList[i].val, NULL, -10 );
                if ( pgSuspendIt == ULONG_MAX && errno ==  ERANGE ) {
                    /* catgets 5067 */
                    ls_syslog (LOG_ERR, "catgets 5067: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a non-negative integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key, ULONG_MAX);
                    lsberrno = LSBE_CONF_WARNING;
                }
                pConf->pgSuspendIt = pgSuspendIt; // struct paramConf *pConf in lib/conf_lsb.h
            }
            else if (i == DISABLE_UACCT_MAP ) { // i == 20
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.disable_uacct_map.5.html
                //
                // DISABLE_UACCT_MAP=y | Y
                //
                // Description: Specify y or Y to disable user-level account mapping.
                //
                // Default: N
                const char Y[] = "Y";
                const char N[] = "N";

                if (strcasecmp (keyList[i].val, Y) == 0) {
                    pConf->disableUAcctMap = true; // struct paramConf *pConf in lib/conf_lsb.h
                }
                else if (strcasecmp (keyList[i].val, N ) == 0) {
                    pConf->disableUAcctMap = false; // struct paramConf *pConf in lib/conf_lsb.h
                }
                else {
                    /* catgets 5068 */
                    ls_syslog (LOG_ERR, "catgets 5068: %s: File %s in section Parameters ending at line %d: unrecognizable value <%s> for the keyword DISABLE_UACCT_MAP; assume user level account mapping is allowed", __func__, filename, *lineNum, keyList[i].val);
                    pConf->disableUAcctMap = false; // struct paramConf *pConf in lib/conf_lsb.h
                }
            }
            else if (i == JOB_PRIORITY_OVER_TIME ) { // i == 26
                // https://www.ibm.com/support/knowledgecenter/en/SSETD4_9.1.2/lsf_config_ref/lsb.params.job_priority_over_time.5.html
                //
                // JOB_PRIORITY_OVER_TIME=increment/interval
                //
                // JOB_PRIORITY_OVER_TIME enables automatic job priority escalation when MAX_USER_PRIORITY is also defined.
                //
                // Valid values: 1. increment: Specifies the value used to increase job priority every interval minutes. Valid values are positive integers.
                //               2. interval:  Specifies the frequency, in minutes, to increment job priority. Valid values are positive integers.
                //
                // Example:JOB_PRIORITY_OVER_TIME=3/20 Specifies that every 20 minute interval increment to job priority of pending jobs by 3.

                unsigned long jobPriorityValue = 0;
                unsigned long jobPriorityTime  = 0;
                char *token[100]; // FIXME FIXME FIXME FIXME if we can find somether better than just a random number

                // tokenize just twice. // FIXME FIXME FIXME FIXME add test to check what happens when you got a string 
                // strcpy (str, keyList[i].val);
                token[0] = strtok( keyList[i].val, "/"); // jobPriorityValue
                token[1] = strtok( NULL,           "/"); // jobPriorityTime
                jobPriorityValue = strtoul( token[0], NULL, 10 );
                jobPriorityTime  = strtoul( token[1], NULL, 10 );

                if( token[0] == NULL || token[1] == NULL || ( jobPriorityValue == ULONG_MAX && errno ==  ERANGE ) || ( jobPriorityTime == ULONG_MAX  && errno ==  ERANGE ) ) {
                    /* catgets 5451 */
                    ls_syslog (LOG_ERR, "catgets 5451: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't in format value/time (time in minutes) or value is not positive; ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key);
                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }

                pConf->jobPriorityValue = jobPriorityValue; // struct paramConf *pConf in lib/conf_lsb.h
                pConf->jobPriorityTime  = jobPriorityTime; // struct paramConf *pConf in lib/conf_lsb.h

            }
            else if (i == SHARED_RESOURCE_UPDATE_FACTOR ) { // i == 27
                value = my_atoi (keyList[i].val, INFINIT_INT, 0);
                if (value == INFINIT_INT) {
                    /* catgets 5459 */
                    ls_syslog (LOG_ERR, "catgets 5459: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key, INFINIT_INT - 1); 
                    lsberrno = LSBE_CONF_WARNING;
                    pConf->sharedResourceUpdFactor = INFINIT_INT; // struct paramConf *pConf in lib/conf_lsb.h
                }
                else {
                    pConf->sharedResourceUpdFactor = value; // struct paramConf *pConf in lib/conf_lsb.h
                }
            }
            else if (i == MAX_JOBID ) { // i == 31
                unsigned int value_ = my_atoi (keyList[i].val, INFINIT_INT, 0);

                if ((value_ < MAX_JOBID_LOW) || (value_ > MAX_JOBID_HIGH)) {
                    /*catgets 5062 */
                    ls_syslog (LOG_ERR, "catgets 5062: %s: File %s in section Parameters ending at line %d: maxJobId value %s not in [%d, %d], use default value %d;", __func__, filename, *lineNum, keyList[i].key, MAX_JOBID_LOW, MAX_JOBID_HIGH, DEF_MAX_JOBID);
                    lsberrno = LSBE_CONF_WARNING;
                    pConf->maxJobId = DEF_MAX_JOBID; // struct paramConf *pConf in lib/conf_lsb.h
                }
                else {
                    pConf->maxJobId = value_;
                }
            }
            else if (i == SCHE_RAW_LOAD )  { // i == 28
                const char Y[] = "Y";
                if (strcasecmp (keyList[i].val, Y ) == 0) {
                    pConf->scheRawLoad = true;
                }
                else {
                    pConf->scheRawLoad = false;
                }
            }
            else if (i == SLOT_RESOURCE_RESERVE ) { // i == 30
                const char Y[] = "Y";
                if (strcasecmp (keyList[i].val, Y ) == 0)  {
                    pConf->slotResourceReserve = true;
                }
                else {
                    pConf->slotResourceReserve = false;
                }
            }
            else if (i > PG_SUSP_IT ) { // i > 5
                if (i < JOB_DEP_LAST_SUB ) { // i < 23 
                    value = my_atoi (keyList[i].val, INFINIT_INT, 0);
                }
                else {
                    value = atoi (keyList[i].val);
                }

                if (value == INFINIT_INT) {
                    /* catgets 5071 */
                    ls_syslog (LOG_ERR, "catgets 5071: %s: File %s in section Parameters ending at line %d: Value <%s> of %s isn't a positive integer between 1 and %d; ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key, INFINIT_INT - 1);
                    lsberrno = LSBE_CONF_WARNING;
                }
                else {
                    switch( i ) {
                        case MBD_SLEEP_TIME: // i == 6
                            pConf->mbatchdInterval = value;
                        break;
                        case CLEAN_PERIOD: // i == 7
                            pConf->cleanPeriod = value;
                        break;
                        case MAX_RETRY: // i == 8 // u
                            pConf->maxDispRetries = value;
                        break;
                        case SBD_SLEEP_TIME: // i == 9
                            pConf->sbatchdInterval = value;
                        break;
                        case MAX_JOB_NUM: // i == 10
                            pConf->maxNumJobs = value;
                        break;
                        case RETRY_INTERVAL: // i == 11
                            pConf->retryIntvl = value;
                        break;
                        case MAX_SBD_FAIL: // i == 12
                            pConf->maxSbdRetries = value;
                        break;
                        case RUSAGE_UPDATE_RATE: // i == 13
                            pConf->rusageUpdateRate = value;
                        break;
                        case RUSAGE_UPDATE_PERCENT: // i == 14
                            pConf->rusageUpdatePercent = value;
                        break;
                        case COND_CHECK_TIME: // i == 15
                            pConf->condCheckTime = value;
                        break;
                        case MAX_SBD_CONNS: // i == 16
                            pConf->maxSbdConnections = value;
                        break;
                        case MAX_SCHED_STAY: // i == 17
                            pConf->maxSchedStay = value;
                        break;
                        case FRESH_PERIOD: // i == 18
                            pConf->freshPeriod = value;
                        break;
                        case MAX_JOB_ARRAY_SIZE: // i == 19
                            if (value < 1 || value >= LSB_MAX_ARRAY_IDX) {
                                /* catgets 5073 */
                                ls_syslog (LOG_ERR, "catgets 5073: %s: File %s in section Parameters ending at line %d: Value <%s> of %s is out of range[1-65534]); ignored", __func__, filename, *lineNum, keyList[i].val, keyList[i].key);
                                lsberrno = LSBE_CONF_WARNING;
                            }
                            else { 
                                pConf->maxJobArraySize = value;
                            }
                        break;
                        case JOB_TERMINATE_INTERVAL: // i == 21
                            pConf->jobTerminateInterval = value;
                        break;
                        case JOB_RUN_TIMES: // i == 22
                            assert( value >= 0);
                            pConf->jobRunTimes = (unsigned int )value;  // FIXME FIXME FIXME see the cast can be done away
                        break;
                        case JOB_DEP_LAST_SUB: // i == 23
                            assert( value >= 0);
                            pConf->jobDepLastSub = (unsigned int) value; // FIXME FIXME FIXME see the cast can be done away
                        break;
                        case MAX_USER_PRIORITY: // i == 25
                            value = my_atoi (keyList[i].val, INFINIT_INT, -1);
                            if (value != INFINIT_INT) {
                                if (value > 0) {
                                    pConf->maxUserPriority = value;
                                }
                            }
                            else {
                                /* catgets 5452 */
                                ls_syslog (LOG_ERR, "catgets 5452: %s: File %s in section Parameters ending at line %d: invalid value <%s> of <%s> : ignored;", __func__, filename, *lineNum, keyList[i].val, keyList[i].key);
                            }
                        break;
                        case PRE_EXEC_DELAY: // i == 29
                            pConf->preExecDelay = value;
                        break;
                        default:
                            /* catgets 5074 */
                            ls_syslog (LOG_ERR, "catgets 5074: %s: File %s in section Parameters ending at line %d: Impossible cases <%d>.", __func__, filename, *lineNum, i);
                            lsberrno = LSBE_CONF_WARNING;
                        break;
                    }
                }
            }
        }
    }


    if (pConf->maxUserPriority <= 0 && pConf->jobPriorityValue > 0 && pConf->jobPriorityTime > 0) {
        /* catgets 5453 */
        ls_syslog (LOG_ERR, "catgets 5453: %s: File %s in section Parameters : MAX_USER_PRIORITY should be defined first so that JOB_PRIORITY_OVER_TIME can be used: job priority control disabled", __func__, filename);
        pConf->jobPriorityValue = -1;
        pConf->jobPriorityTime = -1;
        lsberrno = LSBE_CONF_WARNING;
    }
    freekeyval (keyList);
    return true;
}

unsigned int
my_atoi (const char *arg, int upBound, int botBound)
{
    unsigned int num = 0;
    if( !isint_ (arg) ) {
        return INFINIT_INT;
    }
    num = atoi (arg);
    if (num >= upBound || num <= botBound) {
        return INFINIT_INT;
    }

    return num;
}

float
my_atof ( const char *arg, float upBound, float botBound) 
{
    float num = 0;

    // FIXME FIXME FIXME FIXME check to see if there is a better implementation of atof

    if (!isanumber_ (arg)) {
        return INFINIT_FLOAT;
    }
    num = (float) atof (arg); // FIXME probably intentional, but the if statement bellow may need some fixup
    if (num >= upBound || num <= botBound) {
        return INFINIT_FLOAT;
    }

    return num;
}

void
initParameterInfo (struct parameterInfo *param)
{
    if (param != NULL) {
        defaultQueues     = NULL;
        defaultHostSpec   = NULL;
        mbatchdInterval   = INFINIT_INT;
        sbatchdInterval   = INFINIT_INT;
        jobAcceptInterval = INFINIT_INT;
        maxDispRetries    = INFINIT_INT;
        maxSbdRetries     = INFINIT_INT;
        cleanPeriod       = INFINIT_INT;
        maxNumJobs        = INFINIT_INT;
        pgSuspendIt       = INFINIT_INT;
        defaultProject    = NULL;

        retryIntvl              = INFINIT_INT;
        rusageUpdateRate        = INFINIT_INT;
        rusageUpdatePercent     = INFINIT_INT;
        condCheckTime           = INFINIT_INT;
        maxSbdConnections       = INFINIT_INT;
        maxSchedStay            = INFINIT_INT;
        freshPeriod             = INFINIT_INT;
        maxJobArraySize         = INFINIT_INT;
        jobTerminateInterval    = INFINIT_INT;
        disableUAcctMap         = false;
        jobRunTimes             = INFINIT_INT;
        jobDepLastSub           = INFINIT_INT;
        pjobSpoolDir            = NULL;
        maxUserPriority         = INFINIT_INT;
        jobPriorityValue        = INFINIT_INT;
        jobPriorityTime         = INFINIT_INT;
        sharedResourceUpdFactor = INFINIT_INT;
        scheRawLoad             = 0;
        preExecDelay            = INFINIT_INT;
        slotResourceReserve     = false;
        maxJobId                = INFINIT_INT;
        maxAcctArchiveNum       = INFINIT_INT;
        acctArchiveInDays       = INFINIT_INT;
        acctArchiveInSize       = INFINIT_INT;
    }

    return;
}

void
freeParameterInfo (struct parameterInfo *param)
{
    if (param != NULL) {
        FREEUP (defaultQueues);
        FREEUP (defaultHostSpec);
        FREEUP (defaultProject);
        FREEUP (pjobSpoolDir);
    }

    return;
}

int
checkSpoolDir ( const char *pSpoolDir)
{
    char *pTemp = NULL;
    char TempPath[MAX_PATH_LEN];
    char TempNT[MAX_PATH_LEN];
    char TempUnix[MAX_PATH_LEN];

    memset( TempPath, '\0', strlen( TempPath ) );
    memset( TempNT,   '\0', strlen( TempNT ) );
    memset( TempUnix, '\0', strlen( TempUnix ) );

    if (logclass & LC_EXEC) {
        ls_syslog (LOG_DEBUG, "%s: JOB_SPOOL_DIR is set to %s, and  is of length %d \n", __func__, pSpoolDir, strlen (pSpoolDir));
    }
    if (strlen (pSpoolDir) >= MAX_PATH_LEN) {
        return -1; // FIXME FIXME FIXME return an appropriate *non-negative* n umber
    }

    if (strlen (pSpoolDir) < 1) {
        return 0;
    }

    if (strchr (pSpoolDir, '|') != strrchr (pSpoolDir, '|')) {
        return -1; // FIXME FIXME FIXME return an appropriate *non-negative* n umber
    }

    strcpy (TempPath, pSpoolDir);
    strcpy (TempUnix, pSpoolDir);
    if ((pTemp = strchr (TempUnix, '|')) != NULL) {

        pTemp = NULL
        if (logclass & LC_EXEC) {
            ls_syslog (LOG_DEBUG, "%s: JOB_SPOOL_DIR in UNIX and GNU/Linux is %s \n", __func__, TempUnix);
        }

        if (strlen (TempUnix) > 1) {

            if (TempUnix[0] != '/') {
                return -1; // FIXME FIXME FIXME return an appropriate *non-negative* n umber
            }
        }

        pTemp = strchr (TempPath, '|');
        pTemp++;
        while (isspace (*pTemp)) {
            pTemp++;
        }

        strcpy (TempNT, pTemp);
        if (logclass & LC_EXEC) {
            ls_syslog (LOG_DEBUG, "%s:JOB_SPOOL_DIR in Windows NT[tm] is %s \n", __func__, TempNT);
        }

        if (strlen (TempNT) > 1) {
            for ( unsigned int i = 0; TempNT[i] != '\0'; i++) {
                if (TempNT[i] == '\\' && TempNT[i + 1] == '\\') {
                    ls_syslog (LOG_DEBUG, "%s: NT's JOB_SPOOL_DIR[%d] is ## \n", __func__, i);
                }
                else if (TempNT[i] == '\\') {
                    ls_syslog (LOG_DEBUG, "%s: NT's JOB_SPOOL_DIR[%d] is # \n", __func__, i);
                }
                else {
                    ls_syslog (LOG_DEBUG, "%s: NT's JOB_SPOOL_DIR[%d] is %c\n", __func__, i, TempNT[i]);
                }
            }

            if (TempNT[0] != '\\' || TempNT[1] != '\\') {

                if (TempNT[1] != ':') {
                    return -1; // FIXME FIXME FIXME return an appropriate *non-negative* n umber
                }
                else {
                    if (!isalpha (TempNT[0])) {
                        return -1; // FIXME FIXME FIXME return an appropriate *non-negative* n umber
                    }
                }
            }
        }
    }
    else {

        pTemp = pSpoolDir;

        while (isspace (*pTemp)) {
            pTemp++;
        }


        strcpy (pSpoolDir, pTemp);
        if (pSpoolDir[0] != '/') { // figure out if the path starts with '/' or '\' (distinguish between *nix and Windows)

            if ((pSpoolDir[0] != '\\') || (pSpoolDir[1] != '\\')) {

                if (!(isalpha (pSpoolDir[0]) && (pSpoolDir[1] == ':'))) {
                    return -1; // FIXME FIXME FIXME return an appropriate *non-negative* n umber
                }
            }
        }
    }

    return 0;
}


// NOFIX wrapper function
// struct userConf *
// lsb_readuser (struct lsConf *conf, int options, struct clusterConf *clusterConf)
// {
//     return lsb_readuser_ex (conf, options, clusterConf, NULL);

// }

struct userConf *
lsb_readuser_ex (struct lsConf *conf, int options, struct clusterConf *clusterConf, struct sharedConf *sharedConf)
{
    char *filename  = NULL;
    char *cp        = NULL;
    char *section   = NULL;
    size_t lineNum  = 0;

    lsberrno = LSBE_NO_ERROR;

    if (conf == NULL) {
        const char conf[] = "conf";
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, conf);
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }

    if (sharedConf != NULL) {
        sConf = sharedConf;
    }

    if (conf && conf->confhandle == NULL) {
        const char conf[] = "confhandle";
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, __func__, confhandle );
        lsberrno = LSBE_CONF_FATAL;
        return NULL;
    }
    if (handleUserMem ()) {
        const char handleUserMem[] = "handleUserMem";
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, handleUserMem);
        return NULL;
    }

    filename = conf->confhandle->fname;


    conf->confhandle->curNode = conf->confhandle->rootNode;
    conf->confhandle->lineCount = 0;

    cConf = clusterConf;
    for (;;) { // FIXME FIXME FIXME FIXME replace infinite loop with a ccertain-to-terminate condition
        if ((cp = getBeginLine_conf (conf, &lineNum)) == NULL) {

            if (numofugroups) {
                uConf->ugroups = calloc (numofugroups, sizeof (struct groupInfoEnt));
                if( NULL == uConf->ugroups && ENOMEM == errno ) {
                    const char malloc[] = "malloc";
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, numofugroups * sizeof (struct groupInfoEnt));
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkUser (true);
                    freeUConf (uConf, false);
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
                    const char malloc[] = "malloc";
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, numofusers * sizeof (struct userInfoEnt));
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkUser (true);
                    freeUConf (uConf, false);
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
            const char unknown[ ] = "unknown";
            /* catgets 5082 */
            ls_syslog (LOG_ERR, "catgets 5082: %s: File %s at line %d: Section name expected after Begin; ignoring section", __func__, filename, lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, unknown );
            continue;
        }
        else {
            const char user[]    = "user";
            const char usergroup = "usergroup";

            if (strcasecmp (section, user) == 0) {
                if (do_Users (conf, filename, &lineNum, options) == false) {
                    if (lsberrno == LSBE_NO_MEM) {
                        lsberrno = LSBE_CONF_FATAL;
                        freeWorkUser (true);
                        freeUConf (uConf, false);
                        return NULL;
                    }
                }
                continue;
            }
            else if (strcasecmp (section, usergroup ) == 0) {

                if (do_Groups (usergroups, conf, filename, &lineNum, &numofugroups, options) == false) {
                    if (lsberrno == LSBE_NO_MEM) {
                        lsberrno = LSBE_CONF_FATAL;
                        freeWorkUser (true);
                        freeUConf (uConf, false);
                        return NULL;
                    }
                }
                continue;
            }
            /* catgets 5083 */
            ls_syslog (LOG_ERR, "catgets 5083: %s: File %s at line %d: Invalid section name %s; ignoring section", __func__, filename, lineNum, section);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, &lineNum, filename, section);
            continue;
        }
    }

    return NULL;
}

bool
do_Users (struct lsConf *conf, const char *filename, size_t *lineNum, int options)
{
    int new                          = 0;
    int isGroupAt                    = false;
    size_t maxjobs                   = 0;
    float pJobLimit                  = 0.0f;
    char *linep                      = NULL;
    char *grpSl                      = NULL;
    struct passwd *pw                = NULL;
    struct hTab *tmpUsers            = NULL;
    struct hTab *nonOverridableUsers = NULL;
    
    enum state {
        USER_NAME,
        MAX_JOBS,
        JL_P
    };

    const char keylist[ ] = {
        "USER_NAME",
        "MAX_JOBS",
        "JL_P",
        NULL
    };

    struct keymap keyList[] = {
        { USER_NAME, "    ", keylist[ USER_NAME ], NULL },
        { MAX_JOBS,  "    ", keylist[ MAX_JOBS  ], NULL },
        { JL_P,      "    ", keylist[ JL_P ],      NULL },
        { UINT_MAX,  "    ", NULL,                 NULL }
    };

    const char user[ ] = "user";

    if (conf == NULL) {
        return false;
    }

    linep = getNextLineC_conf (conf, lineNum, true);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (isSectionEnd (linep, filename, lineNum, user)) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum, user );
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (strchr (linep, '=') == NULL) {
        if (!keyMatch (keyList, linep, false)) {
            /* catgets 5086 */
            ls_syslog (LOG_ERR, "catgets 5086: %s: File %s at line %d: Keyword line format error for User section; ignoring section", __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, user );
            return false;
        }

        if (keyList[USER_NAME].position < 0) {
            /* catgets 5087 */
            ls_syslog (LOG_ERR, "catgets 5087: %s: File %s at line %d: User name required for User section; ignoring section", __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, user );
            return false;
        }

        tmpUsers = malloc (sizeof (struct hTab));
        nonOverridableUsers = malloc (sizeof (struct hTab));
        if ( ( NULL == tmpUsers && ENOMEM == errno ) || ( NULL == nonOverridableUsers && ENOMEM == errno ) ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct hTab));
            lsberrno = LSBE_NO_MEM;
            return false;
        }
        h_initTab_ (tmpUsers, 32);             // FIXME FIXME FIXME 32 is awfully particular
        h_initTab_ (nonOverridableUsers, 16);  // FIXME FIXME FIXME 16 is awfully particular

        while ((linep = getNextLineC_conf (conf, lineNum, true)) != NULL) {
            int hasAt               = 0;
            size_t lastChar         = 0;
            struct groupInfoEnt *gp = NULL;
            struct group *unixGrp   = NULL;

            isGroupAt = false;
            freekeyval (keyList);
            if (isSectionEnd (linep, filename, lineNum, user )) {
                h_delTab_ (tmpUsers);
                FREEUP (tmpUsers);
                h_delTab_ (nonOverridableUsers);
                FREEUP (nonOverridableUsers);
                return true;
            }
            if (mapValues (keyList, linep) < 0) {
                /* catgets 5089 */
                ls_syslog (LOG_ERR, "catgets 5089: %s: File %s at line %d: Values do not match keys in User section; ignoring line", __func__, filename, *lineNum); 
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }
            h_addEnt_ (tmpUsers, keylist[USER_NAME].val, &new);
            if (!new) {
                /* catgets 5090 */
                ls_syslog (LOG_ERR, "catgets 5090: %s: File %s at line %d: User name <%s> multiply specified; ignoring line", __func__, filename, *lineNum, keylist[USER_NAME].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }
            hasAt = false;
            lastChar = strlen (keylist[USER_NAME].val) - 1;
            if (lastChar > 0 && keylist[USER_NAME].val[lastChar] == '@') {
                hasAt = true;
                keylist[USER_NAME].val[lastChar] = '\0';
            }

            lastChar = strlen (keylist[USER_NAME].val) - 1;
            if (lastChar > 0 && keylist[USER_NAME].val[lastChar] == '/') {
                grpSl = putstr_ (keylist[USER_NAME].val);
                if (grpSl == NULL) {
                    lsberrno = LSBE_NO_MEM;
                    h_delTab_ (tmpUsers);
                    FREEUP (tmpUsers);
                    FREEUP (grpSl);
                    h_delTab_ (nonOverridableUsers);
                    FREEUP (nonOverridableUsers);
                    return false;
                }

                grpSl[lastChar] = '\0';
            }
            gp = getUGrpData (keylist[USER_NAME].val);
            pw = getpwlsfuser_ (keylist[USER_NAME].val);

            if ((options != CONF_NO_CHECK) && !gp && (grpSl || (strcmp (keylist[USER_NAME].val, defaultLabel) && !pw))) {
                if (grpSl) {
                    unixGrp = mygetgrnam (grpSl);
                    grpSl[lastChar] = '/';
                }
                else {
                    unixGrp = mygetgrnam (keylist[USER_NAME].val);
                }

                if (unixGrp != NULL) {
                    if (options & (CONF_EXPAND | CONF_NO_EXPAND | CONF_CHECK)) {

                        gp = addUnixGrp (unixGrp, keylist[USER_NAME].val, filename, *lineNum, " in User section", 0);

                        if (gp == NULL)
                            {
                            if (lsberrno == LSBE_NO_MEM) {
                                return false;
                            }
                            /* catgets 5091 */
                            ls_syslog (LOG_WARNING, "catgets 5091: %s: File %s at line %d: No valid users defined in Unix group <%s>; ignored", __func__, filename, *lineNum, keylist[USER_NAME].val);
                            lsberrno = LSBE_CONF_WARNING;
                            continue;
                            }
                        }
                    }
                else
                    {
                    /* catgets 5092 */
                    ls_syslog (LOG_WARNING, "catgets 5092: %s: File %s at line %d: Unknown user <%s>; Maybe a windows user or of another domain.", __func__, filename, *lineNum, keylist[USER_NAME].val);  
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            if (hasAt && gp) {
                isGroupAt = true;
            }
            if (hasAt && (!(options & (CONF_EXPAND | CONF_NO_EXPAND)) ||  options == CONF_NO_CHECK)) {
                keylist[USER_NAME].val[lastChar + 1] = '@';
            }

            maxjobs = INFINIT_INT;
            if (keyList[MAX_JOBS].position >= 0 && keylist[MAX_JOBS].val != NULL && strcmp (keylist[MAX_JOBS].val, ""))
                {


                if ((maxjobs = my_atoi (keylist[MAX_JOBS].val, INFINIT_INT, -1)) == INFINIT_INT)
                    {
                    /* catgets 5093 */
                    ls_syslog (LOG_ERR, "catgets 5093: %s: File %s at line %d: Invalid value <%s> for key <%s>; %d is assumed", __func__, filename, *lineNum, keylist[MAX_JOBS].val, keyList[MAX_JOBS].key, INFINIT_INT);
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            pJobLimit = INFINIT_FLOAT;
            if (keyList[JL_P].position >= 0 && keylist[JL_P].val != NULL
                && strcmp (keylist[JL_P].val, ""))
                {

                pJobLimit = my_atof( keylist[JL_P].val, INFINIT_FLOAT, -1.0 );
                if ( fabs( INFINIT_FLOAT - pJobLimit) < 0.00001 ) {
                    /* catgets 5094 */
                    ls_syslog (LOG_ERR, "catgets 5094: %s: File %s at line %d: Invalid value <%s> for key %s; %f is assumed", __func__, filename, *lineNum, keylist[JL_P].val, keyList[JL_P].key, INFINIT_FLOAT);
                    lsberrno = LSBE_CONF_WARNING;
                }
            }
            if (!isGroupAt && (!(options & (CONF_EXPAND | CONF_NO_EXPAND)) || options == CONF_NO_CHECK)) {
                h_addEnt_ (nonOverridableUsers, keylist[USER_NAME].val, 0);
                if (!addUser (keylist[USER_NAME].val, maxjobs, pJobLimit, filename, true) && lsberrno == LSBE_NO_MEM) {
                    FREEUP (grpSl);
                    lsberrno = LSBE_NO_MEM;
                    h_delTab_ (tmpUsers);
                    FREEUP (tmpUsers);
                    FREEUP (grpSl);
                    h_delTab_ (nonOverridableUsers);
                    FREEUP (nonOverridableUsers);
                    return false;

                }
            }
            else {
                char **groupMembers = NULL;
                unsigned int *numMembers = 0;
                char gr__func__[MAX_LSB_NAME_LEN];
                STRNCPY (gr__func__, keylist[USER_NAME].val, MAX_LSB_NAME_LEN);


                if (isGroupAt)
                    {
                    if (gr__func__[strlen (keylist[USER_NAME].val) - 1] == '@')
                        gr__func__[strlen (keylist[USER_NAME].val) - 1] = '\0';
                    }

                if ((groupMembers = expandGrp (gr__func__, numMembers, USER_GRP)) == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, __func__);
                    // goto Error;
                    h_delTab_ (tmpUsers);
                    FREEUP (tmpUsers);
                    FREEUP (grpSl);
                    h_delTab_ (nonOverridableUsers);
                    FREEUP (nonOverridableUsers);
                    return false;
                }

                if (strcmp (groupMembers[0], "all") == 0) {
                    ls_syslog (LOG_ERR, "catgets 5096: %s: File %s at line %d: user group <%s> with no members is ignored", __func__, filename, *lineNum, keylist[USER_NAME].val); /* catgets 5096 */
                }
                else if (!(options & CONF_NO_EXPAND)) {
                    for ( unsigned int i = 0; i < *numMembers; i++) {
                        if (h_getEnt_ (nonOverridableUsers, groupMembers[i])) {
                            continue;
                        }
                        addUser (groupMembers[i], maxjobs, pJobLimit, filename, true);
                    }
                }
                else {
                    if (isGroupAt) {
                        gr__func__[lastChar + 1] = '@';
                    }

                    addUser (gr__func__, maxjobs, pJobLimit, filename, true);
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
        return false;

    }
    else {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "user");
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, "user");
        return false;
    }

// Error:     // FIXME FIXME FIXME goto label has to go
//     h_delTab_ (tmpUsers);
//     FREEUP (tmpUsers);
//     FREEUP (grpSl);
//     h_delTab_ (nonOverridableUsers);
//     FREEUP (nonOverridableUsers);
//     return false;
    return false;
}


bool
do_Groups (struct groupInfoEnt **groups, struct lsConf *conf, const char *filename, size_t *lineNum, unsigned int *ngroups, int options)
{
    enum state { 
        GROUP_NAME, 
        GROUP_MEMBER 
    };

    const char *keylist[] {
        "GROUP_NAME",
        "GROUP_MEMBER",
        NULL
    };

    struct keymap keyList[] = {
        {GROUP_NAME,   "    ", keylist[GROUP_NAME],   NULL },
        {GROUP_MEMBER, "    ", keylist[GROUP_MEMBER], NULL },
        {UINT_MAX,     "    ", NULL,                  NULL }
    };

    char *wp                = NULL;
    char *sp                = NULL;
    char *linep             = NULL;
    char HUgroups[20];      // FIXME FIXME FIXME 20 chars is very generic
    int type                = 0;
    int allFlag             = false;
    int nGrpOverFlow        = 0;
    struct passwd *pw       = NULL;
    struct groupInfoEnt *gp = NULL;

    const char ALL[] = "all";
    const cahr OTHERS[] = "others";

    if (groups == NULL || conf == NULL || ngroups == NULL) {
        return false;
    }

    if (groups == usergroups) {
        const char usergroup[] = "usergroup";
        type = USER_GRP;
        strcpy( HUgroups, usergroup ) ;
    }
    else if (groups == hostgroups) {
        const char hostgroup[] = "hostgroup";
        type = HOST_GRP;
        strcpy( HUgroups, hostgroup );
    }

    linep = getNextLineC_conf (conf, lineNum, true);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (isSectionEnd (linep, filename, lineNum, HUgroups)) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum, HUgroups);
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (strchr (linep, '=') == NULL) {

        if (!keyMatch (keyList, linep, false)) {
            /* catgets 5101 */
            ls_syslog (LOG_ERR, "catgets 5101: %s: File %s at line %d: Keyword line format error for section <%s>; ignoring section", __func__, filename, *lineNum, HUgroups);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, HUgroups);
            return false;
        }
        while ((linep = getNextLineC_conf (conf, lineNum, true)) != NULL) {

            freekeyval (keyList);

            if (isSectionEnd (linep, filename, lineNum, HUgroups)) {
                return true;
            }

            if (mapValues (keyList, linep) < 0) {
                /* catgets 5102 */
                ls_syslog (LOG_ERR, "catgets 5102: %s: File %s at line %d: Values do not match keys in %s section; ignoring line", __func__, filename, *lineNum, HUgroups);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (    strcmp (keyList[GROUP_NAME].val, ALL)          == 0 ||  // ALL is defined at top
                    strcmp (keyList[GROUP_NAME].val, defaultLabel) == 0 || 
                    strcmp (keyList[GROUP_NAME].val, OTHERS)       == 0     // OTHERS is defined on top
                )
                {
                /* catgets 5103 */
                ls_syslog (LOG_WARNING, "catgets 5103: %s: File %s at line %d: Group name <%s> conflicts with reserved word <all/default/others>; ignoring line", __func__, filename, *lineNum, keyList[GROUP_NAME].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }


            if ((type == USER_GRP || type == HOST_GRP) && (keyList[GROUP_NAME].val != NULL) && (strcmp (keyList[keyList[GROUP_MEMBER].val].val, "!") == 0)) {
                char *members;

                if ((members = runEGroup_ (type == USER_GRP ? "-u" : "-m", keyList[GROUP_NAME].val)) != NULL) {
                    FREEUP (keyList[GROUP_MEMBER].val);
                    keyList[GROUP_MEMBER].val = members;
                }
                else {
                    memset( keyList[GROUP_MEMBER].val, 0, strlen( keyList[GROUP_MEMBER].val ) );
                    keyList[GROUP_MEMBER].val = NULL
                }
            }

            if (options != CONF_NO_CHECK && type == USER_GRP) {

                pw = getpwlsfuser_ (keyList[GROUP_NAME].val);
                if (!initUnknownUsers) {
                    initTab (&unknownUsers);
                    initUnknownUsers = true;
                }

                if ((pw != NULL) || (chekMembStr (&unknownUsers, keyList[GROUP_NAME].val) != NULL)) {
                    /* catgets 5104 */
                    ls_syslog (LOG_WARNING, "catgets 5104: %s: File %s at line %d: Group name <%s> conflicts with user login name; ignoring line", __func__, filename, *lineNum, keyList[GROUP_NAME].val);
                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }
            }

            if (options != CONF_NO_CHECK && type == HOST_GRP && isHostName (keyList[GROUP_NAME].val)) {
                /* catgets 5105 */
                ls_syslog (LOG_WARNING, "catgets 5105: %s: File %s at line %d: Group name <%s> conflicts with host name; ignoring line", __func__, filename, *lineNum, keyList[GROUP_NAME].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (options != CONF_NO_CHECK && getGrpData (groups, keyList[GROUP_NAME].val, *ngroups)) {
                /* catgets 5106 */
                ls_syslog (LOG_WARNING, "catgets 5106: %s: File %s at line %d: Group <%s> is multiply defined; ignoring line", __func__, filename, *lineNum, keyList[GROUP_NAME].val);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (*ngroups >= MAX_GROUPS) {

                if (nGrpOverFlow++ == 0) {
                    /* catgets 5107 */
                    ls_syslog (LOG_WARNING, "catgets 5107: %s: File %s at line %d: The number of configured groups reaches the limit <%d>; ignoring the rest of groups defined", __func__, filename, *lineNum, MAX_GROUPS);
                }

                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if ((type == HOST_GRP) && *ngroups >= MAX_GROUPS) {
                /* catgets 5113 */
                ls_syslog (LOG_ERR, "catgets 5113: %s: File %s at line %d: The number of configured host groups reaches the limit <%d>; ignoring the rest of groups defined", __func__, filename, *lineNum, MAX_GROUPS);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }
            else if ((type == USER_GRP) && (*ngroups >= MAX_GROUPS)) {
                /* catgets 5107 */
                ls_syslog (LOG_ERR, "catgets 5107: %s: File %s at line %d: The number of configured user groups reaches the limit <%d>; ignoring the rest of groups defined", __func__, filename, *lineNum, MAX_GROUPS);
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }

            if (type == HOST_GRP && (strchr (keyList[GROUP_MEMBER].val, '~') != NULL && (options & CONF_NO_EXPAND) == 0)) {
                char *outHosts = NULL;
                int numHosts = 0;

                ls_syslog (LOG_DEBUG, "%s: resolveBatchNegHosts( ): \'%s\': the string is \'%s\'", __func__, keyList[GROUP_NAME].val, keyList[GROUP_MEMBER].val);
                numHosts = resolveBatchNegHosts (keyList[GROUP_MEMBER].val, &outHosts, false);
                if (numHosts > 0) {
                    ls_syslog (LOG_DEBUG, "%s: resolveBatchNegHosts( ): \'%s\' the string is replaced with \'%s\'", __func__, keyList[GROUP_NAME].val, outHosts);
                }
                else if (numHosts == 0) { // FIXME FIXME FIXME non-positive values
                    /* catgets 5460 */
                    ls_syslog (LOG_WARNING, "catgets 5460: %s: File %s at line %d: there are no hosts found to exclude, replaced with \'%s\'", __func__, filename, *lineNum, outHosts);
                }
                else if (numHosts == -1) { // FIXME FIXME FIXME non-positive values
                    lsberrno = LSBE_NO_MEM;
                    return false;
                }
                else {
                    if (numHosts == -3) { // FIXME FIXME FIXME non-positive values
                        /* catgets 5461 */
                        ls_syslog (LOG_WARNING, "catgets 5461: %s: \'%s\': \'%s\' The result is that all the hosts are to be excluded.", __func__, keyList[GROUP_NAME].val, keyList[GROUP_MEMBER].val);
                    }
                    /* catgets 5108 */
                    ls_syslog (LOG_WARNING, "catgets 5108: %s: File %s at line %d: No valid member in group <%s>; ignoring line", __func__, filename, *lineNum, keyList[GROUP_NAME].val);

                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }

                memset( keyList[GROUP_MEMBER].val, 0, strlen( keyList[GROUP_MEMBER].val ));
                assume( strlen( keyList[GROUP_MEMBER].val ) >= strlen( outHosts ) + 1 );
                strcpy( keyList[GROUP_MEMBER].val, outHosts );
            }

            gp = addGroup (groups, keyList[GROUP_MEMBER].val, ngroups, 0);
            if (gp == NULL && lsberrno == LSBE_NO_MEM) {
                return false;
            }

            sp = keyList[GROUP_MEMBER].val;
            allFlag = false;
            if (searchAll (keyList[GROUP_MEMBER].val) && (strchr (sp, '~') == NULL)) {
                allFlag = true;
            }

            while (!allFlag && (wp = getNextWord_ (&sp)) != NULL) {
                if (!addMember(gp, wp, type, filename, *lineNum, "", options, true) && lsberrno == LSBE_NO_MEM) {
                    return false;
                }
            }

            if (allFlag) {
                if (!addMember (gp, "all", type, filename, *lineNum, "", options, true) && lsberrno == LSBE_NO_MEM) {
                    return false;
                }
            }

            if (gp->memberList == NULL) {
                /* catgets 5108 */
                ls_syslog (LOG_WARNING, "catgets 5108: %s: File %s at line %d: No valid member in group <%s>; ignoring line", __func__, filename, *lineNum, gp->group);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (gp->group);
                FREEUP (gp);
                *ngroups -= 1;
            }

        }

        ls_syslog (LOG_WARNING, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;

        return true;

    }
    else {
        ls_syslog (LOG_WARNING, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, HUgroups);
        lsberrno = LSBE_CONF_WARNING;

        doSkipSection_conf (conf, lineNum, filename, HUgroups);

        return false;
    }

    fprintf(   stdout,  "%s: you are not supposed to be here!\n", __func__ );
    ls_syslog( LOG_ERR, "%s: you are not supposed to be here", __func__ );
    return false;
}


bool
isHostName ( const char *grpName)
{
    if (Gethostbyname_ (grpName) != NULL) {
        return true;
    }

    for ( unsigned int i = 0; i < numofhosts; i++) {
        if (hosts && hosts[i]->host && equalHost_ (grpName, hosts[i]->host)) {
            return true;
        }
    }

    return false;
}

struct groupInfoEnt *
addGroup (struct groupInfoEnt **groups, const char *gname, unsigned int *ngroups, int type)
{
    if (groups == NULL || ngroups == NULL) {
        return NULL;
    }

    if (type == 0) {

        groups[*ngroups] = malloc(sizeof (struct groupInfoEnt));
        if ( NULL == groups[*ngroups] && ENOMEM == errno ) {
            const char malloc[ ] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, sizeof (struct groupInfoEnt));
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
            const char malloc[ ] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, sizeof (struct groupInfoEnt));
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

    return NULL;
}

struct groupInfoEnt *
addUnixGrp (struct group *unixGrp, const char *gname, const char *filename, unsigned int lineNum, const char *section, int type)
{
    // int i                   = -1;
    unsigned int            = 0;
    struct groupInfoEnt *gp = NULL;
    struct passwd *pw       = NULL;

    if (unixGrp == NULL) {
        return NULL;
    }

    if (numofugroups >= MAX_GROUPS) { // unsigned int numofugroups in lib/conf_lsb.h
        /* catgets 5131 */
        ls_syslog (LOG_ERR, "catgets 5131: %s: File %s at line %d: numofugroups <%d> is equal to or greater than MAX_GROUPS <%d>; ignoring the group <%s>", __func__, filename, lineNum, numofugroups, MAX_GROUPS, unixGrp->gr_name);
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
            if (!addMember (gp, unixGrp->gr_mem[i], USER_GRP, filename, lineNum, section, CONF_EXPAND, true)  && lsberrno == LSBE_NO_MEM) {
                return NULL;
            }
            if (logclass & LC_TRACE) {
                ls_syslog (LOG_DEBUG, "%s: addMember() at i = %ld was successful: %s", __func__, i, unixGrp->gr_mem[i]);
            }
        }
        else {
            /* catgets 5132 */
            ls_syslog (LOG_ERR, "catgets 5132: %s: File %s%s at line %d: Bad user name <%s> in group <%s>; ignored", __func__, filename, section, lineNum, unixGrp->gr_mem[i], gname);
            lsberrno = LSBE_CONF_WARNING;
            continue;
        }
    }

    if (gp->memberList == NULL) {
        freeGroupInfo (gp);
        assert( numofugroups > 0 ); // CHECK for overflow
        numofugroups -= 1;
        return NULL;
    }

    return gp;
}

bool
addMember (struct groupInfoEnt *gp, const char *word, int grouptype, const char *filename, unsigned int lineNum, const char *section, int options, int checkAll)
{
    // 
    size_t len                     = 0; 
    int isHp                       = false;
    char isgrp                     = false;
    char *cp                       = NULL;
    char *myWord                   = NULL;
    char returnVal                 = false;
    struct passwd *pw              = NULL;
    struct hostent *hp             = NULL;
    struct groupInfoEnt *subgrpPtr = NULL;
    char name[MAXHOSTNAMELEN];
    char cpWord[MAXHOSTNAMELEN];

    memset( name,   '\0', strlen( name ) );
    memset( cpWord, '\0', strlen( cpWord ) );

    if (gp == NULL || word == NULL) {
        return false;
    }

    cp = word;
    if (cp[0] == '~') {
        cp++;
    }

    if( options & CONF_NO_EXPAND ) {
        if (cp == NULL) {
            return false;
        }
        myWord = putstr_ (cp);
        strcpy (cpWord, word);
    }
    else {
        myWord = putstr_ (word);
        strcpy (cpWord, "");
    }

    if (myWord == NULL) {
        const char malloc[ ] = "malloc";
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen (cp) + 1);
        lsberrno = LSBE_NO_MEM;
        return false;
    }

    if (grouptype == USER_GRP && strcmp (myWord, "all") && options != CONF_NO_CHECK) {
        subgrpPtr = getUGrpData (myWord);

        if (gp == subgrpPtr) {
            /* catgets 5134 */
            ls_syslog (LOG_ERR, "catgets 5134: %s: File %s at line %d: Recursive member in Unix group <%s>; ignored", __func__, filename, lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return false;
        }

        if (!subgrpPtr) {
            pw = getpwlsfuser_ (myWord);
        }

        if (!initUnknownUsers) {
            initTab (&unknownUsers);
            initUnknownUsers = true;
        }

        isgrp = true;
        if (pw != NULL) {
            strcpy (name, pw->pw_name);
            isgrp = false;
        }
        else if (chekMembStr (&unknownUsers, myWord) != NULL) {
            strcpy (name, myWord);
            isgrp = false;
        }
        else if (!subgrpPtr) {
            char *grpSl = NULL;
            struct group *unixGrp = NULL;
            size_t lastChar = strlen (myWord) - 1;

            if (lastChar > 0 && myWord[lastChar] == '/') {
                grpSl = putstr_ (myWord);
                if (grpSl == NULL) {
                    const char malloc[ ] = "malloc";
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen (myWord) + 1);
                    lsberrno = LSBE_NO_MEM;
                    FREEUP (myWord);
                    return false;
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
                        ls_syslog (LOG_WARNING, "catgets 5136: %s: File %s at line %d: No valid users defined in Unix group <%s>; ignored", __func__, filename, lineNum, myWord);
                        lsberrno = LSBE_CONF_WARNING;
                    }
                    FREEUP (myWord);
                    return false;
                }
            }
            else {
                /* catgets 5137 */
                ls_syslog (LOG_WARNING, "catgets 5137: %s: File %s%s at line %d: Unknown user <%s> in group <%s>; Maybe a windows user or of another domain.", __func__, filename, section, lineNum, myWord, gp->group);
                lsberrno = LSBE_CONF_WARNING;
                if (!addMembStr (&unknownUsers, myWord)) {
                    FREEUP (myWord);
                    return false;
                }
            }

        }
    }

    if (grouptype == HOST_GRP && strcmp (myWord, "all") && options != CONF_NO_CHECK) {

        subgrpPtr = getHGrpData (myWord);
        if (gp == subgrpPtr) {
            /* catgets 5138 */
            ls_syslog (LOG_ERR, "catgets 5138: %s: File %s at line %d: Recursive member in Unix group <%s>; ignored", __func__, filename, lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return false;
        }

        if (subgrpPtr != NULL) {
            isgrp = true;
        }
        else {
            if ((hp = Gethostbyname_ (myWord)) == NULL) {
                /* catgets 5139 */
                ls_syslog (LOG_ERR, "catgets 5139: %s: File %s%s at line %d: Bad host/group name <%s> in group <%s>; ignored", __func__, filename, section, lineNum, myWord, gp->group);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return false;
            }
            strcpy (name, hp->h_name);
            if (getHostData (name) == NULL && numofhosts != 0) {
                /* catgets 5140 */
                ls_syslog (LOG_ERR, "catgets 5140: %s: File %s%s at line %d: Host <%s> is not used by the batch system; ignored", __func__, filename, section, lineNum, name);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return false;
            }
            if (isServerHost (name) == false) {
                /* catgets 5141 */
                ls_syslog (LOG_ERR, "catgets 5141: %s: File %s%s at line %d: Host <%s> is not a server; ignored", __func__, filename, section, lineNum, name);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return false;
            }
            isgrp = false;
        }
    }

    if (isHp == false)
        {
        if ((options & CONF_NO_EXPAND) == 0) {
            returnVal = isInGrp (myWord, gp, grouptype, checkAll);
        }
        else {
            returnVal = isInGrp (cpWord, gp, grouptype, checkAll);
        }
    }
    else {
        returnVal = false;
    }

    if (returnVal) {
        if (isgrp){
            /* catgets 5142 */
            ls_syslog (LOG_ERR, "catgets 5142: %s: File %s%s at line %d: Group <%s> is multiply defined in group <%s>; ignored", __func__, filename, section, lineNum, myWord, gp->group);
        }
        else {
            /* catgets 5143 */
            ls_syslog (LOG_ERR, "catgets 5143: %s: File %s%s at line %d: Member <%s> is multiply defined in group <%s>; ignored", __func__, filename, section, lineNum, myWord, gp->group);
        }
        lsberrno = LSBE_CONF_WARNING;
        FREEUP (myWord);
        return false;
    }

    if (gp->memberList == NULL) {
        if (options & CONF_NO_EXPAND) {
            gp->memberList = putstr_ (cpWord);
        }
        else {
            gp->memberList = putstr_ (myWord);
        }
        if (gp->memberList == NULL) {
            const char malloc[ ] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen (myWord) + 1);
            lsberrno = LSBE_NO_MEM;
            FREEUP (myWord);
            return false;
        }
    }
    else {
        if (options & CONF_NO_EXPAND) {
            len = strlen (gp->memberList) + strlen (cpWord) + 2;
        }
        else {
            len = strlen (gp->memberList) + strlen (myWord) + 2;
        }
        gp->memberList = myrealloc( gp->memberList, len * sizeof (char));
        if ( NULL == gp->memberList && ENOMEM == errno ) {
            const char malloc[ ] = "myrealloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, myrealloc, len * sizeof (char));
            lsberrno = LSBE_NO_MEM;
            FREEUP (myWord);
            return false;
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
    return true;
}


// struct groupInfoEnt *
// getUGrpData ( const char *gname)
// {
//     return getGrpData (usergroups, gname, numofugroups);
// }

// struct groupInfoEnt *
// getHGrpData ( const char *gname)
// {
//     return getGrpData (hostgroups, gname, numofhgroups);
// }

struct groupInfoEnt *
getGrpData (struct groupInfoEnt **groups, const char *groupname, unsigned int num)
{
    if (groupname == NULL || groups == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < num; i++) {
        if (groups[i] && groups[i]->group && (strcmp (groupname, groups[i]->group) == 0) ) {
            return groups[i];
        }
    }

    return NULL;
}

struct userInfoEnt *
getUserData ( const char *username)
{
    if (username == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < numofusers; i++) {
        if (strcmp (username, users[i]->user) == 0) {
            return users[i];
        }
    }

    return NULL;
}


struct hostInfoEnt *
getHostData ( const char *hostname)
{
    if (hostname == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < numofhosts; i++) {
        if (equalHost_ (hostname, hosts[i]->host)) {
            return hosts[i];
        }
    }

    return NULL;
}

struct queueInfoEnt *
getQueueData (const char *queuename)
{
    if (queuename == NULL) {
        return NULL;
    }

    for ( unsigned int i = 0; i < numofqueues; i++) {
        if (strcmp (queuename, queues[i]->queue) == 0) {
            return queues[i];
        }
    }

    return NULL;
}

bool
searchAll (char *const word)
{
    char *sp = NULL;
    char *cp = NULL;

    if (word == NULL) {
        return false;
    }
    cp = word;
    while ((sp = getNextWord_ (&cp))) {
        const char all[] = "all"
        if (strcmp (sp, all ) == 0) {
            return true;
        }
    }
    return false;

}


void
initUserInfo (struct userInfoEnt *up)
{
    if (up != NULL) {
        up->user         = NULL;
        up->procJobLimit = INFINIT_FLOAT;
        up->maxJobs      = INFINIT_INT;
        up->numStartJobs = INFINIT_INT;
        up->numJobs      = INFINIT_INT;
        up->numPEND      = INFINIT_INT;
        up->numRUN       = INFINIT_INT;
        up->numSSUSP     = INFINIT_INT;
        up->numUSUSP     = INFINIT_INT;
        up->numRESERVE   = INFINIT_INT;
    }

    return;
}

void
freeUserInfo (struct userInfoEnt *up)
{
    if (up != NULL) {
        FREEUP (up->user);
    }

    return;
}

void
initGroupInfo (struct groupInfoEnt *gp)
{
    if (gp != NULL) {
        gp->group      = NULL;
        gp->memberList = NULL;
    }

    return;
}

void
freeGroupInfo (struct groupInfoEnt *gp)
{
    if (gp != NULL) {
        FREEUP (gp->group);
        FREEUP (gp->memberList);
    }

    return;
}

bool
addUser ( const char *username, size_t maxjobs, float pJobLimit, const char *filename, int override)
{
    struct userInfoEnt *up        = NULL;
    struct userInfoEnt **tmpUsers = NULL;

    if (username == NULL) {
        return false;
    }

    if ((up = getUserData (username)) != NULL) {
        if (override)  {
            freeUserInfo (up);
            initUserInfo (up);
            up->user = putstr_ (username);
            if (up->user == NULL) {
                const char malloc[ ] = "malloc";
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "%s", malloc, __func__, strlen (username));
                lsberrno = LSBE_NO_MEM;
                return false;
            }
            up->procJobLimit = pJobLimit;
            up->maxJobs = maxjobs;
            return true;
        }
        else {
            if (filename) {
                /* catgets 5147 */
                ls_syslog (LOG_ERR, "catgets 5147: %s: %s: User <%s> is multiply defined; retaining old definition", __func__, filename, username);
                lsberrno = LSBE_CONF_WARNING;
            }
            return false;
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
                const char malloc[ ] = "myrealloc";
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, myrealloc, usersize * sizeof (struct userInfoEnt *));
                lsberrno = LSBE_NO_MEM;
                return false;
            }
            else {
                users = tmpUsers;
            }
        }

        users[numofusers] = malloc (sizeof (struct userInfoEnt));
        if ( NULL == users[numofusers] ) {
            const char malloc[ ] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, sizeof (struct userInfoEnt));
            lsberrno = LSBE_NO_MEM;
            return false;
        }

        initUserInfo (users[numofusers]);
        users[numofusers]->user = putstr_ (username);
        if (users[numofusers]->user == NULL) {
            FREEUP (users[numofusers]);
            lsberrno = LSBE_NO_MEM;
            return false;
        }

        users[numofusers]->procJobLimit = pJobLimit;
        users[numofusers]->maxJobs = maxjobs;
        numofusers++;
        return true;
    }

    fprintf(   stdout,  "%s: you are not supposed to be here, esse!\n", __func__ );
    ls_syslog( LOG_ERR, "%s: you are not supposed to be here, esse!", __func__ );
    return NULL;
}

bool
isInGrp (char *word, struct groupInfoEnt *gp, int grouptype, int checkAll)
{
    char *tmp = NULL;
    char *str = NULL;
    struct groupInfoEnt *sub_gp = NULL;

    if (word == NULL || gp == NULL || gp->memberList == NULL) {
        return false;
    }

    if (word[0] == '~') {
        return false;
    }

    tmp = gp->memberList;

    if (!strcmp (tmp, "all") && checkAll == true) {
        return true;
    }

    while ((str = getNextWord_ (&tmp)) != NULL) {
        if (grouptype == USER_GRP) {
            if (!strcmp (str, word))
                return true;
            }
        else {
            if (equalHost_ (str, word))
                return true;
        }

        if (grouptype == USER_GRP) {
            sub_gp = getUGrpData (str);
        }
        else {
            sub_gp = getHGrpData (str);
        }

        if (isInGrp (word, sub_gp, grouptype, false)) {
            return true;
        }
    }

    return false;
}

char **
expandGrp (char *word, unsigned int *num, int grouptype)
{
    unsigned int sub_num        = 0;
    unsigned int n              = 0;
    char *tmp                   = NULL;
    char *str                   = NULL;
    char **sub_list             = NULL;
    char **list                 = NULL;
    char **tempList             = NULL;
    struct groupInfoEnt *gp     = NULL;
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
            const char malloc[ ] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, sizeof (char *));
            lsberrno = LSBE_NO_MEM;
            return NULL;
        }

        if ((list[0] = putstr_ (word)) == NULL) { // FIXME FIXME FIXME FIXME find out what list[0] is supposed to be
            const char malloc[ ] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen (word) + 1);
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
                const char myrealloc[ ] = "myrealloc";
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, myrealloc, sizeof (char *));
                lsberrno = LSBE_NO_MEM;
                return NULL;
            }
            list = tempList;
            if ((list[0] = putstr_ ("all")) == NULL) { // FIXME FIXME FIXME FIXME find out what list[0] is supposed to be
                const char malloc[ ] = "malloc";
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, strlen ("all") + 1);
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
    char hostok     = false;
    char hgroupok   = false;
    char hpartok    = false;
    size_t *lineNum = 0;
    char *filename  = NULL;
    char *cp        = NULL;
    char *section   = NULL;
    struct lsInfo myinfo;

    // assert( hostok );
    // assert( hpartok );
    // assert( hgroupok );

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
        ls_syslog (LOG_INFO, ("catgets 5446: %s: default user will be used."), __func__);  // FIXME FIXME __func__ here is wrong. it should be filename, but it is not init'ed yet
        if (setDefaultUser ()) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "setDefaultUser");
            return NULL;
        }
    }
    myinfo = *info;
    cConf = clusterConf; // cConf: struct clusterConf *cConf in lib/conf.h

    assert( info->nRes >= 0 );
    myinfo.resTable = malloc( info->nRes * sizeof (struct resItem));
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

    for (;;) { // FIXME FIXME FIXME FIXME replace infinite loop with a ccertain-to-terminate condition
        if ((cp = getBeginLine_conf (conf, lineNum)) == NULL) {
            if (!hostok) {
                /* catgets 5165 */
                ls_syslog (LOG_ERR, "catgets 5165: %s: Host section missing or invalid.", filename);
                lsberrno = LSBE_CONF_WARNING;
            }

            if( numofhosts ) {
                hConf->hosts = malloc(numofhosts * sizeof (struct hostInfoEnt));
                if ( NULL == hConf->hosts && ENOMEM == errno) { // FIXME FIXME FIXME FIXME this may be wrong and need some adjustment
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", numofhosts * sizeof (struct hostInfoEnt));
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkHost (true);
                    freeHConf (hConf, false);
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
                    freeWorkHost (true);
                    freeHConf (hConf, false);
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
            ls_syslog (LOG_ERR, "catgets 5169: %s: File %s at line %d: Section name expected after Begin; ignoring section", __func__, filename, &lineNum);
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, "unknown");
            continue;
        }
        else {
            if (strcasecmp (section, "host") == 0) {
                if (do_Hosts_lsb(conf, filename, lineNum, &myinfo, options)) {
                    hostok = true;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkHost (true);
                    freeHConf (hConf, false);
                    FREEUP (myinfo.resTable);
                    return NULL;
                }
                continue;
            }
            else if (strcasecmp (section, "hostgroup") == 0) {

                if (do_Groups(hostgroups, conf, filename, lineNum, &numofhgroups, options)) {
                    hgroupok = true;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkHost (true);
                    FREEUP (myinfo.resTable);
                    freeHConf (hConf, false);
                    return NULL;
                }
                continue;
            }
            else {
                /* catgets 5170 */
                ls_syslog (LOG_ERR, "catgets 5170: %s: File %s at line %d: Invalid section name <%s>; ignoring section", __func__, filename, lineNum, section);
                lsberrno = LSBE_CONF_WARNING;
                doSkipSection_conf (conf, lineNum, filename, section);
            }
        }
    }

    fprintf(   stdout,  "%s: you are not supposed to be here, esse!\n", __func__ );
    ls_syslog( LOG_ERR, "%s: you are not supposed to be here, esse!", __func__ );
    return NULL;
}


void
getThresh (struct lsInfo *info, struct keymap *keylist, float loadSched[], float loadStop[], const char *filename, size_t *lineNum, const char *section)
{

    char *stop = NULL;
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
            ls_syslog (LOG_ERR, "catgets 5192: %s: File %s%s at line %d: Value <%s> of loadSched <%s> isn't a float number between %1.1f and %1.1f; ignored", __func__, filename, section, *lineNum, keylist[i].val, keylist[i].key, -INFINIT_LOAD, INFINIT_LOAD);
            lsberrno = LSBE_CONF_WARNING;
            if (info->resTable[i].orderType == DECR)
                loadSched[i] = -INFINIT_LOAD;
            }
        if (*keylist[i].val != '\0' && loadSched[i] < 0 && loadSched[i] > -INFINIT_LOAD) {
            /* catgets 5193 */
            ls_syslog (LOG_WARNING, "catgets 5193: %s: File %s%s at line %d: Warning: Value <%s> of loadSched <%s> is not a non-negative number", __func__, filename, section, *lineNum, keylist[i].val, keylist[i].key);
            lsberrno = LSBE_CONF_WARNING;
        }

        if (i == UT && loadSched[i] > 1.0 && loadSched[i] < INFINIT_LOAD ) {
            /* catgets 5447 */
            ls_syslog (LOG_INFO, ("catgets 5447: %s: File %s %s at line %d: For load index <%s>, loadSched <%2.2f> is greater than 1; assumming <%5.1f%%>"), __func__, filename, section, *lineNum, keylist[i].key, loadSched[i], loadSched[i]);
            lsberrno = LSBE_CONF_WARNING;
            loadSched[i] /= 100.0;
        }

        if (!stop) {
            continue;
        }

        loadStop[i] = my_atof (stop, INFINIT_LOAD, -INFINIT_LOAD);
        if ( fabs(INFINIT_LOAD - loadStop[i]) < 0.000001 ) {
            /* catgets 5194 */
            ls_syslog (LOG_ERR, "catgets 5194: %s: File %s%s at line %d: Value <%s> of loadStop <%s> isn't a float number between %1.1f and %1.1f; ignored", __func__, filename, section, *lineNum, stop, keylist[i].key, -INFINIT_LOAD, INFINIT_LOAD);
            lsberrno = LSBE_CONF_WARNING;
            if (info->resTable[i].orderType == DECR) {
                loadStop[i] = -INFINIT_LOAD;
            }
            continue;
        }

        if (loadStop[i] < 0 && loadStop[i] > -INFINIT_LOAD) {
            /* catgets 5195 */
            ls_syslog (LOG_WARNING, "catgets 5195: %s: File %s%s at line %d: Warning: Value <%s> of loadStop <%s> is not a non-negative number", __func__, filename, section, *lineNum, stop, keylist[i].key);
            lsberrno = LSBE_CONF_WARNING;
        }

        if (i == UT && loadStop[i] > 1.0 && loadSched[i] < INFINIT_LOAD) {
            /* catgets 5440 */
            ls_syslog (LOG_INFO, ("catgets 5440: %s: File %s%s at line %d: For load index <%s>, loadStop <%2.2f> is greater than 1; assumming <%5.1f%%>"), __func__, filename, section, *lineNum, keylist[i].key, loadStop[i], loadStop[i]);
            lsberrno = LSBE_CONF_WARNING;
            loadStop[i] /= 100.0;
        }

        if ((loadSched[i] > loadStop[i]) && info->resTable[i].orderType == INCR) {
            /* catgets 5196 */
            ls_syslog (LOG_ERR, "catgets 5196: %s: File %s%s at line %d: For load index <%s>, loadStop <%2.2f> is lower than loadSched <%2.2f>; swapped", __func__, filename, section, *lineNum, keylist[i].key, loadStop[i], loadSched[i]);
            lsberrno = LSBE_CONF_WARNING;
            swap = loadSched[i];
            loadSched[i] = loadStop[i];
            loadStop[i] = swap;
        }

        if ((loadStop[i] > loadSched[i]) && info->resTable[i].orderType == DECR) {
            /* catgets 5197 */
            ls_syslog (LOG_ERR, "catgets 5197: %s: File %s%s at line %d: For load index <%s>, loadStop <%2.2f> is higher than loadSched <%2.2f>; swapped", __func__, filename, section, *lineNum, keylist[i].key, loadStop[i], loadSched[i]);
            lsberrno = LSBE_CONF_WARNING;
            swap = loadSched[i];
            loadSched[i] = loadStop[i];
            loadStop[i] = swap;
        }
    }

}

int
addHostEnt (struct hostInfoEnt *hp, struct hostInfo *hostInfo, size_t *override)
{
    unsigned int ihost       = 0;
    bool bExists             = false;
    struct hostInfoEnt *host = NULL;

    if (hp == NULL) {
        return false;
    }

    if (numofhosts == hostsize) {
        struct hostInfoEnt **tmpHosts;
        if (hostsize == 0) {
            hostsize = 5;
        }
        else {
            hostsize *= 2;
        }
        tmpHosts = myrealloc(hosts, hostsize * sizeof (struct hostInfoEnt *));
        if ( NULL == tmpHosts && ENOMEM == errno ) {
            const char realloc[] = "realloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, realloc, hostsize * sizeof (struct hostInfoEnt *));
            lsberrno = LSBE_NO_MEM;
            freeHostInfoEnt( hp );
            return false;
        }
        else {
            hosts = tmpHosts;
        }
    }

    if ( (hp->host == NULL && (host = getHostData (hostInfo->hostName)) != NULL) || 
         (hp->host != NULL && (host = getHostData (hp->host)) != NULL) )
    {
        bExists = true;
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
            return true;
        }
    }
    else {
        host = malloc( sizeof (struct hostInfoEnt));
        if ( NULL == host && ENOMEM == errno ) {
            const char malloc[ ] = "malloc";
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, malloc, sizeof (struct hostInfoEnt));
            lsberrno = LSBE_NO_MEM;
            return false;
        }
    }
    initHostInfoEnt( host );
    if (copyHostInfo (host, hp) < 0) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, hp->host, hp->hostName );
        FREEUP (host);
        return false;
    }

    if (bExists) {
        hosts[ihost] = host;
    }
    else {
        hosts[numofhosts] = host;
        numofhosts++;
    }

    return true;
}

int
copyHostInfo (struct hostInfoEnt *toHost, struct hostInfoEnt *fromHost)
{
    memcpy (toHost, fromHost, sizeof (struct hostInfoEnt));
    if ((toHost->host = putstr_ (fromHost->host)) == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (fromHost->host) + 1);
        lsberrno = LSBE_NO_MEM;
        return -1; // FIXME FIXME FIXME replace -1 with relative, non-negative integer.
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
            return -1; // FIXME FIXME FIXME replace -1 with relative, non-negative integer.
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

void
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

    return;
}


char *
parseGroups (char *linep, const char *filename, size_t *lineNum, const char *section, int groupType, int options)
{
    
    char *str                    = NULL;
    char *word                   = NULL;
    char *myWord                 = NULL;
    char *groupName              = NULL;
    char *grpSl                  = NULL;
    char *hostGroup              = NULL;
    struct group        *unixGrp = NULL;
    struct groupInfoEnt *gp      = NULL;
    struct groupInfoEnt *mygp    = NULL;
    struct passwd       *pw      = NULL;

    unsigned int len = 0;
    int hasAllOthers = false;
    int checkAll     = true;
    int haveFirst    = false;
    bool_t hasNone   = false;
    char returnVal   = false;
    char hostName[MAXHOSTNAMELEN];

// #define failReturn(mygp, size)  {                                       \
// ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", size); \
// freeGroupInfo (mygp);                                           \
// FREEUP (mygp);                                                  \
// FREEUP (hostGroup);                                             \
// lsberrno = LSBE_NO_MEM;                                         \
// return NULL;}

    if (groupType == USER_GRP) {
        groupName = "User/User";
    }
    else {
        groupName = "Host/Host";
    }

    if (groupType == USER_GRP && numofugroups >= MAX_GROUPS) {
        /* catgets 5245 */
        ls_syslog (LOG_ERR, "catgets 5245: %s: File %s%s at line %d: number of %s group <%d> is equal to or greater than MAX_GROUPS <%d>; ignored the group <%s>", __func__, filename, section, *lineNum, (groupType == USER_GRP) ? "user" : "host", (groupType == USER_GRP) ? numofugroups : numofhgroups, MAX_GROUPS, linep); 
        return NULL;
    }

    mygp = malloc(sizeof (struct groupInfoEnt));
    if( NULL == mygp && ENOMEM == errno ) {
        // failReturn (mygp, sizeof (struct groupInfoEnt));
        const char malloc[] = "malloc";
        ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct groupInfoEnt));
        freeGroupInfo (mygp);
        FREEUP (mygp);
        FREEUP (hostGroup);
        lsberrno = LSBE_NO_MEM;

        return NULL;
    }

    initGroupInfo (mygp);
    if (groupType == HOST_GRP) {
        hostGroup = malloc (MAX_LINE_LEN);
        if (NULL == hostGroup && ENOMEM == errno ) {
            // failReturn (mygp, MAX_LINE_LEN);
            const char malloc[] = "malloc";
            ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", MAX_LINE_LEN );
            freeGroupInfo (mygp);
            FREEUP (mygp);
            FREEUP (hostGroup);
            lsberrno = LSBE_NO_MEM;

            return NULL;
        }
        len = MAX_LINE_LEN;
        checkAll = false;
        hostGroup[0] = '\0';
    }

    while ((word = getNextWord_ (&linep)) != NULL) {
        char *cp = NULL;
        char cpWord[MAXHOSTNAMELEN]
        memset( cpWord, 0, MAXHOSTNAMELEN );

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
            //failReturn (mygp, strlen (word) + 1);
            const char malloc[] = "malloc";
            ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (word) + 1 );
            freeGroupInfo (mygp);
            FREEUP (mygp);
            FREEUP (hostGroup);
            lsberrno = LSBE_NO_MEM;

            return NULL;
        }

        if (groupType == HOST_GRP && ((options & CONF_NO_EXPAND) == 0)) {
            returnVal = isInGrp (myWord, mygp, groupType, checkAll);
        }
        else {
            returnVal = isInGrp (cpWord, mygp, groupType, checkAll);
        }

        if (returnVal) {
            /* catgets 5246 */
            ls_syslog (LOG_ERR, "catgets 5246: %s: File %s%s at line %d: %s group <%s> is multiply defined; ignored", __func__, filename, section, *lineNum, groupName, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            continue;
        }

        if (groupType == USER_GRP) {
            size_t lastChar = 0;

            if (options == CONF_NO_CHECK) {
                if (!addMember (mygp, myWord, USER_GRP, filename, *lineNum, section, options, checkAll) && lsberrno == LSBE_NO_MEM) {

                    // failReturn (mygp, strlen (myWord) + 1);
                    const char malloc[] = "malloc";
                    ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (myWord) + 1 );
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (hostGroup);
                    lsberrno = LSBE_NO_MEM;

                    return NULL;
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
                    // failReturn (mygp, strlen (myWord) + 1);
                    const char malloc[] = "malloc";
                    ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (myWord) + 1 );
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (hostGroup);
                    lsberrno = LSBE_NO_MEM;

                    return NULL;
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
                        ls_syslog (LOG_WARNING, "catgets 5247: %s: File %s at line %d: No valid users defined in Unix group <%s>; ignoring", __func__, filename, *lineNum, myWord);
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
                ls_syslog (LOG_WARNING, "catgets 5248: %s: File %s, section %s at line %d: Unknown user or user group <%s>; Maybe a windows user or of another domain", __func__, filename, section, *lineNum, myWord);
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
                int badPref = false;
                char *sp;

                length = strlen (myWord);
                strcpy (hostName, myWord);


                if (length > 2 && myWord[length - 1] >= '0' && myWord[length - 1] <= '9' && myWord[length - 2] == '+') {
                    if (myWord[length - 3] == '+') {
                        /* catgets 5251 */
                        ls_syslog (LOG_ERR, "catgets 5251: %s: File %s section %s at line %d: Host <%s> is specified with bad host preference expression; ignored", __func__, filename, section, *lineNum, hostName);   
                        FREEUP (myWord);
                        continue;
                    }
                    myWord[length - 2] = '\0';
                }
                else if ((sp = strchr (myWord, '+')) != NULL && sp > myWord) {

                    char *cp_      = NULL;
                    bool is_number = false;
                    badPref        = false;
                    cp_            = sp;

                    while (*cp_ != '\0') {
                        if (*cp_ == '+' && !number) {
                            cp_++;
                            continue;
                        }
                        else if ((*cp_ >= '0') && (*cp_ <= '9')) {
                            cp_++;
                            number = true;
                            continue;
                        }
                        else {
                            /* catgets 5252 */
                            ls_syslog (LOG_ERR, "catgets 5252: %s: File %s%s at line %d: Host <%s> is specified with bad host preference expression; ignored", __func__, filename, section, *lineNum, hostName);   
                            FREEUP (myWord);
                            badPref = true;
                            break;
                        }
                    }
                    if (badPref == true) {
                        continue;
                    }
                    sp = NULL;
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
                        cpWord[length - 2] = '\0'; // FIXME FIXME FIXME such ascii gymnastics bullshit is bad for one's health.
                    }
                    else if ((sp = strchr (cpWord, '+')) != NULL && sp > cpWord) {
 
                        char *cp_  = NULL;
                        int number = false;
                        badPref    = false;
                        cp_        = sp;

                        while (*cp_ != '\0') {
                            if (*cp_ == '+' && !number) {
                                cp_++;
                                continue;
                            }
                            else if ((*cp_ >= '0') && (*cp_ <= '9')) {
                                cp_++;
                                number = true;
                                continue;
                            }
                            else {
                                badPref = true;
                                break;
                            }
                        }
                        if (badPref == true) {
                            continue;
                        }
                        *sp = '\0';
                    }
                }

                if (strcmp (myWord, "none") == 0) {
                    hasNone = true;
                    FREEUP (myWord);
                    continue;
                }
                if (!strcmp (myWord, "others")) {
                    if (hasAllOthers == true) {
                        /* catgets 5253 */
                        ls_syslog (LOG_ERR, "catgets 5253: %s: File %s section %s at line %d: More than one <others> or host group with <all> as its member specified; <%s> is ignored", __func__, filename, section, *lineNum, hostName);  
                        FREEUP (myWord);
                        continue;
                    }

                    if (putIntoList (&hostGroup, &len, hostName, I18N_IN_QUEUE_HOST) == NULL) {
                        // failReturn (mygp, strlen (myWord) + 1);
                        const char malloc[] = "malloc";
                        ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (myWord) + 1 );
                        freeGroupInfo (mygp);
                        FREEUP (mygp);
                        FREEUP (hostGroup);
                        lsberrno = LSBE_NO_MEM;

                        return NULL;
                    }
                    hasAllOthers = true;
                    FREEUP (myWord);
                    continue;
                }
            }

            if (isHostName (myWord) == false) {

                gp = getHGrpData (myWord);
                if (groupType == HOST_GRP && strcmp (myWord, "all") != 0 && checkAllOthers (myWord, &hasAllOthers)) {
                    /* catgets 5255 */
                    ls_syslog (LOG_ERR, "catgets 5255: %s: File %s section %s at line %d: More than one host group with <all> as its members or <others> specified; <%s> is ignored", __func__, filename, section, *lineNum, hostName); 
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
                    // failReturn (mygp, strlen (hostName) + 1);
                    const char malloc[] = "malloc";
                    ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (myWord) + 1 );
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (hostGroup);
                    lsberrno = LSBE_NO_MEM;

                    return NULL;
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
                    ls_syslog (LOG_ERR, "catgets 5257: %s: File %s section %s at line %d: Host <%s> is not used by the batch system; ignored", __func__, filename, section, *lineNum, myWord);  
                    lsberrno = LSBE_CONF_WARNING;
                    FREEUP (myWord);
                    continue;
                }

                if (isServerHost (myWord) == false) {
                    /* catgets 5258 */
                    ls_syslog (LOG_ERR, "catgets 5258: %s: File %s section %s at line %d: Host <%s> is not a server; ignored", __func__, filename, section, *lineNum, myWord);  
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
                    ls_syslog (LOG_ERR, "catgets 5259: %s: File %s section %s at line %d: Host name <%s> is multiply defined; ignored", __func__, filename, section, *lineNum, myWord); /* catgets 5259 */
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
                    // failReturn (mygp, strlen (myWord) + 1);
                    const char malloc[] = "malloc";
                    ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (myWord) + 1 );
                    freeGroupInfo (mygp);
                    FREEUP (mygp);
                    FREEUP (hostGroup);
                    lsberrno = LSBE_NO_MEM;

                    return NULL;
                }
                FREEUP (myWord);
                continue;
            }
        }
    }
    FREEUP (grpSl);

    if (hasNone == false || groupType != HOST_GRP)
        if (mygp->memberList == NULL)
            {
            freeGroupInfo (mygp);
            FREEUP (mygp);
            FREEUP (hostGroup);
            return NULL;
            }

    if (groupType != HOST_GRP) {
        if ((str = putstr_ (mygp->memberList)) == NULL) {
            // failReturn (mygp, strlen (mygp->memberList) + 1);
            const char malloc[] = "malloc";
            ls_syslog(LOG_ERR,  I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (mygp->memberList) + 1 );
            freeGroupInfo (mygp);
            FREEUP (mygp);
            FREEUP (hostGroup);
            lsberrno = LSBE_NO_MEM;

            return NULL;
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
                ls_syslog (LOG_ERR, "catgets 5260: %s: file <%s> line <%d> host names <%s> should not be specified together with \"none\", ignored", __func__, filename, *lineNum, hostGroup);
                lsberrno = LSBE_CONF_WARNING;
            }
            strcpy (hostGroup, "none");
        }

        return hostGroup;
    }

    fprintf( stderr, "%s: you are not supposed to be here!\n", __func__ );
    ls_syslog( LOG_ERR, "%s: you are not supposed to be here", __func__ );
    lsberrno = EROCKBOTTOM;
    return NULL;
}

struct queueConf *
lsb_readqueue (struct lsConf *conf, struct lsInfo *info, int options, struct sharedConf *sharedConf)
{
    
    char *cp        = NULL;
    char *section   = NULL;
    char *filename  = NULL;
    size_t *lineNum = 0;
    int j           = 0;
    char queueok    = 'a';
    struct lsInfo myinfo;

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

    // j = 0;
    for ( unsigned int i = 0, unsigned int j = 0; i < info->nRes; i++, j++) {

        if (info->resTable[i].flags & RESF_DYNAMIC) {
            memcpy (&myinfo.resTable[j], &info->resTable[i], sizeof (struct resItem));
            j++;
        }
    }

    // for ( unsigned int i = 0; i < info->nRes; i++) {
    for ( unsigned int i = 0, unsigned int j = 0; i < info->nRes; i++, j++) {
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
        freeQConf (qConf, true);
    }
    else {
        if ((qConf = malloc (sizeof (struct queueConf))) == NULL) {
            lsberrno = LSBE_CONF_FATAL;
            return NULL;
        }
        qConf->numQueues = 0;
        qConf->queues = NULL;
    }

    freeWorkQueue (false);
    queuesize = 0;
    maxFactor = 0.0;
    maxHName = NULL;
    filename = conf->confhandle->fname;
    conf->confhandle->curNode = conf->confhandle->rootNode;
    conf->confhandle->lineCount = 0;
    queueok = false;

    for (;;) { // FIXME FIXME FIXME FIXME replace infinite loop with a ccertain-to-terminate condition

        if ((cp = getBeginLine_conf (conf, lineNum)) == NULL) {
            if (!queueok) {
                ls_syslog (LOG_ERR, "catgets 5271: %s: Queue section missing or invalid.", filename);    /* catgets 5271 */
                lsberrno = LSBE_CONF_WARNING;
            }

            if (numofqueues){

                qConf->queues = calloc (numofqueues, sizeof (struct queueInfoEnt));
                if ( NULL == qConf->queues && ENOMEM == errno ) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkQueue (true);
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
            ls_syslog (LOG_ERR, "catgets 5273: %s: File %s at line %d: Section name expected after Begin; ignoring section", __func__, filename, lineNum);  /* catgets 5273 */
            lsberrno = LSBE_CONF_WARNING;
            doSkipSection_conf (conf, lineNum, filename, "unknown");
        }
        else {

            if (strcasecmp (section, "Queue") == 0) {
                if (do_Queues (conf, filename, lineNum, &myinfo, options)) {
                    queueok = true;
                }
                else if (lsberrno == LSBE_NO_MEM) {
                    lsberrno = LSBE_CONF_FATAL;
                    freeWorkQueue (true);
                    freeQConf (qConf, false);
                    FREEUP (myinfo.resTable);
                    return NULL;
                }
                else {
                    lsberrno = LSBE_CONF_WARNING;
                }
                continue;
            }
            else {
                ls_syslog (LOG_ERR, "catgets 5274: %s: File %s at line %d: Invalid section name <%s>; ignoring section", __func__, filename, lineNum, section);    /* catgets 5274 */
                lsberrno = LSBE_CONF_WARNING;
                doSkipSection_conf (conf, lineNum, filename, section);
            }
        }
    }

    return NULL;
}

// char
bool 
do_Queues (struct lsConf *conf, const char *filename, size_t *lineNum, struct lsInfo *info, int options)
{
    
    char *linep = NULL;
    char *sp    = NULL;
    char *word  = NULL;
    int retval  = 0;
    char *originalString = NULL;
    char *subString      = NULL;

    enum {
        QKEY_NAME = info->numIndx,
        QKEY_PRIORITY,
        QKEY_NICE,
        QKEY_UJOB_LIMIT,
        QKEY_PJOB_LIMIT,
        QKEY_RUN_WINDOW,
        QKEY_CPULIMIT,
        QKEY_FILELIMIT,
        QKEY_DATALIMIT,
        QKEY_STACKLIMIT,
        QKEY_CORELIMIT,
        QKEY_MEMLIMIT,
        QKEY_RUNLIMIT,
        QKEY_USERS,
        QKEY_HOSTS,
        QKEY_EXCLUSIVE,
        QKEY_DESCRIPTION,
        QKEY_MIG,
        QKEY_QJOB_LIMIT,
        QKEY_POLICIES,
        QKEY_DISPATCH_WINDOW,
        QKEY_USER_SHARES,
        QKEY_DEFAULT_HOST_SPEC,
        QKEY_PROCLIMIT,
        QKEY_ADMINISTRATORS,
        QKEY_PRE_EXEC,
        QKEY_POST_EXEC,
        QKEY_REQUEUE_EXIT_VALUES,
        QKEY_HJOB_LIMIT,
        QKEY_RES_REQ,
        QKEY_SLOT_RESERVE,
        QKEY_RESUME_COND,
        QKEY_STOP_COND,
        QKEY_JOB_STARTER,
        QKEY_SWAPLIMIT,
        QKEY_PROCESSLIMIT,
        QKEY_JOB_CONTROLS,
        QKEY_TERMINATE_WHEN,
        QKEY_NEW_JOB_SCHED_DELAY,
        QKEY_INTERACTIVE,
        QKEY_JOB_ACCEPT_INTERVAL,
        QKEY_BACKFILL,
        QKEY_IGNORE_DEADLINE,
        QKEY_CHKPNT,
        QKEY_RERUNNABLE,
        QKEY_ENQUE_INTERACTIVE_AHEAD,
        QKEY_ROUND_ROBIN_POLICY,
        QKEY_PRE_POST_EXEC_USER
    };

    const char *keylist[ ] = {
        "QKEY_NAME",
        "QKEY_PRIORITY",
        "QKEY_NICE",
        "QKEY_UJOB_LIMIT",
        "QKEY_PJOB_LIMIT",
        "QKEY_RUN_WINDOW",
        "QKEY_CPULIMIT",
        "QKEY_FILELIMIT",
        "QKEY_DATALIMIT",
        "QKEY_STACKLIMIT",
        "QKEY_CORELIMIT",
        "QKEY_MEMLIMIT",
        "QKEY_RUNLIMIT",
        "QKEY_USERS",
        "QKEY_HOSTS",
        "QKEY_EXCLUSIVE",
        "QKEY_DESCRIPTION",
        "QKEY_MIG",
        "QKEY_QJOB_LIMIT",
        "QKEY_POLICIES",
        "QKEY_DISPATCH_WINDOW",
        "QKEY_USER_SHARES",
        "QKEY_DEFAULT_HOST_SPEC",
        "QKEY_PROCLIMIT",
        "QKEY_ADMINISTRATORS",
        "QKEY_PRE_EXEC",
        "QKEY_POST_EXEC",
        "QKEY_REQUEUE_EXIT_VALUES",
        "QKEY_HJOB_LIMIT",
        "QKEY_RES_REQ",
        "QKEY_SLOT_RESERVE",
        "QKEY_RESUME_COND",
        "QKEY_STOP_COND",
        "QKEY_JOB_STARTER",
        "QKEY_SWAPLIMIT",
        "QKEY_PROCESSLIMIT",
        "QKEY_JOB_CONTROLS",
        "QKEY_TERMINATE_WHEN",
        "QKEY_NEW_JOB_SCHED_DELAY",
        "QKEY_INTERACTIVE",
        "QKEY_JOB_ACCEPT_INTERVAL",
        "QKEY_BACKFILL",
        "QKEY_IGNORE_DEADLINE",
        "QKEY_CHKPNT",
        "QKEY_RERUNNABLE",
        "QKEY_ENQUE_INTERACTIVE_AHEAD",
        "QKEY_ROUND_ROBIN_POLICY",
        "QKEY_PRE_POST_EXEC_USER",
        NULL
    };

    struct keymap *keyList = {
        { QKEY_NAME,                    "    ", keylist[QKEY_NAME],                    NULL },
        { QKEY_PRIORITY,                "    ", keylist[QKEY_PRIORITY],                NULL },
        { QKEY_NICE,                    "    ", keylist[QKEY_NICE],                    NULL },
        { QKEY_UJOB_LIMIT,              "    ", keylist[QKEY_UJOB_LIMIT],              NULL },
        { QKEY_PJOB_LIMIT,              "    ", keylist[QKEY_PJOB_LIMIT],              NULL },
        { QKEY_RUN_WINDOW,              "    ", keylist[QKEY_RUN_WINDOW],              NULL },
        { QKEY_CPULIMIT,                "    ", keylist[QKEY_CPULIMIT],                NULL },
        { QKEY_FILELIMIT,               "    ", keylist[QKEY_FILELIMIT],               NULL },
        { QKEY_DATALIMIT,               "    ", keylist[QKEY_DATALIMIT],               NULL },
        { QKEY_STACKLIMIT,              "    ", keylist[QKEY_STACKLIMIT],              NULL },
        { QKEY_CORELIMIT,               "    ", keylist[QKEY_CORELIMIT],               NULL },
        { QKEY_MEMLIMIT,                "    ", keylist[QKEY_MEMLIMIT],                NULL },
        { QKEY_RUNLIMIT,                "    ", keylist[QKEY_RUNLIMIT],                NULL },
        { QKEY_USERS,                   "    ", keylist[QKEY_USERS],                   NULL },
        { QKEY_HOSTS,                   "    ", keylist[QKEY_HOSTS],                   NULL },
        { QKEY_EXCLUSIVE,               "    ", keylist[QKEY_EXCLUSIVE],               NULL },
        { QKEY_DESCRIPTION,             "    ", keylist[QKEY_DESCRIPTION],             NULL },
        { QKEY_MIG,                     "    ", keylist[QKEY_MIG],                     NULL },
        { QKEY_QJOB_LIMIT,              "    ", keylist[QKEY_QJOB_LIMIT],              NULL },
        { QKEY_POLICIES,                "    ", keylist[QKEY_POLICIES],                NULL },
        { QKEY_DISPATCH_WINDOW,         "    ", keylist[QKEY_DISPATCH_WINDOW],         NULL },
        { QKEY_USER_SHARES,             "    ", keylist[QKEY_USER_SHARES],             NULL },
        { QKEY_DEFAULT_HOST_SPEC,       "    ", keylist[QKEY_DEFAULT_HOST_SPEC],       NULL },
        { QKEY_PROCLIMIT,               "    ", keylist[QKEY_PROCLIMIT],               NULL },
        { QKEY_ADMINISTRATORS,          "    ", keylist[QKEY_ADMINISTRATORS],          NULL },
        { QKEY_PRE_EXEC,                "    ", keylist[QKEY_PRE_EXEC],                NULL },
        { QKEY_POST_EXEC,               "    ", keylist[QKEY_POST_EXEC],               NULL },
        { QKEY_REQUEUE_EXIT_VALUES,     "    ", keylist[QKEY_REQUEUE_EXIT_VALUES],     NULL },
        { QKEY_HJOB_LIMIT,              "    ", keylist[QKEY_HJOB_LIMIT],              NULL },
        { QKEY_RES_REQ,                 "    ", keylist[QKEY_RES_REQ],                 NULL },
        { QKEY_SLOT_RESERVE,            "    ", keylist[QKEY_SLOT_RESERVE],            NULL },
        { QKEY_RESUME_COND,             "    ", keylist[QKEY_RESUME_COND],             NULL },
        { QKEY_STOP_COND,               "    ", keylist[QKEY_STOP_COND],               NULL },
        { QKEY_JOB_STARTER,             "    ", keylist[QKEY_JOB_STARTER],             NULL },
        { QKEY_SWAPLIMIT,               "    ", keylist[QKEY_SWAPLIMIT],               NULL },
        { QKEY_PROCESSLIMIT,            "    ", keylist[QKEY_PROCESSLIMIT],            NULL },
        { QKEY_JOB_CONTROLS,            "    ", keylist[QKEY_JOB_CONTROLS],            NULL },
        { QKEY_TERMINATE_WHEN,          "    ", keylist[QKEY_TERMINATE_WHEN],          NULL },
        { QKEY_NEW_JOB_SCHED_DELAY,     "    ", keylist[QKEY_NEW_JOB_SCHED_DELAY],     NULL },
        { QKEY_INTERACTIVE,             "    ", keylist[QKEY_INTERACTIVE],             NULL },
        { QKEY_JOB_ACCEPT_INTERVAL,     "    ", keylist[QKEY_JOB_ACCEPT_INTERVAL],     NULL },
        { QKEY_BACKFILL,                "    ", keylist[QKEY_BACKFILL],                NULL },
        { QKEY_IGNORE_DEADLINE,         "    ", keylist[QKEY_IGNORE_DEADLINE],         NULL },
        { QKEY_CHKPNT,                  "    ", keylist[QKEY_CHKPNT],                  NULL },
        { QKEY_RERUNNABLE,              "    ", keylist[QKEY_RERUNNABLE],              NULL },
        { QKEY_ENQUE_INTERACTIVE_AHEAD, "    ", keylist[QKEY_ENQUE_INTERACTIVE_AHEAD], NULL },
        { QKEY_ROUND_ROBIN_POLICY,      "    ", keylist[QKEY_ROUND_ROBIN_POLICY],      NULL },
        { QKEY_PRE_POST_EXEC_USER,      "    ", keylist[QKEY_PRE_POST_EXEC_USER],      NULL },
        { -1,                           "    ", NULL, NULL }
    };

    const char queueLabel[]   = "Queue";
    const char defaultLabel[] = "default";

    struct queueInfoEnt queue;

    if (conf == NULL) {
        return false;
    }

    initQueueInfo (&queue); // important!

    linep = getNextLineC_conf (conf, lineNum, true);
    if (!linep) {
        ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (isSectionEnd (linep, filename, lineNum, queueLabel)) {
        ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum, queueLabel);
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (strchr (linep, '=') == NULL) {
        /* catgets 5277 */
        ls_syslog (LOG_ERR, "catgets 5277: %s: File %s at line %d: Vertical Queue section not implented yet; use horizontal format; ignoring section", __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, queueLabel);
        return false;
    }
    else {
        char *function_name = malloc( strlen(filename )  + 1 );
        strcpy( function_name, filename );
        retval = readHvalues_conf (keyList, linep, conf, filename, lineNum, false, queueLabel);
        if (retval < 0) { // FIXME FIXME FIXME FIXME replace with meaningful, non-negative number
            if (retval == -2) { // FIXME FIXME FIXME FIXME replace with meaningful, non-negative number
                lsberrno = LSBE_CONF_WARNING;
                /* catgets 5463 */
                ls_syslog (LOG_ERR, "catgets 5463: %s: Parameter error in %s(%d); remaining parameters in this section will be either ignored or set to default values.", __func__, filename, *lineNum);
            }
            else {
                /* catgets 5278 */
                ls_syslog (LOG_ERR, "catgets 5278: %s: File %s at line %d: Incorrect section; ignoring this Queue section", __func__, filename, *lineNum);
                lsberrno = LSBE_CONF_WARNING;
                freekeyval (keyList);
                return false;
            }
        }

        if (keyList[QKEY_NAME].val == NULL) {
            /* catgets 5279 */
            ls_syslog (LOG_ERR, "catgets 5279: %s: File %s in section Queue ending at line %d: Queue name is not given; ignoring section", __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keyList);
            return false;
        }

        if (strcmp (keyList[QKEY_NAME].val, defaultLabel) == 0) {
            /* catgets 5280 */
            ls_syslog (LOG_ERR, "catgets 5280: %s: File %s in section Queue ending at line %d: Queue name <%s> is a reserved word; ignoring the queue section", __func__, filename, *lineNum, keyList[QKEY_NAME].val);
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keyList);
            return false;
        }

        if (getQueueData (keyList[QKEY_NAME].val)) {
            /* catgets 5281 */
            ls_syslog (LOG_ERR, "catgets 5281: %s: File %s in section Queue ending at line %d: Duplicate queue name <%s>; ignoring section", __func__, filename, *lineNum, keyList[QKEY_NAME].val);
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keyList);
            return false;
        }

        queue.queue = putstr_ (keyList[QKEY_NAME].val);
        if (queue.queue == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_NAME].val) + 1);
            lsberrno = LSBE_NO_MEM;
            freekeyval (keyList);
            return false;
        }

        if (keyList[QKEY_PRIORITY].val != NULL && strcmp (keyList[QKEY_PRIORITY].val, "")) {
            queue.priority = my_atoi (keyList[QKEY_PRIORITY].val, INFINIT_INT, 0);
            if (INFINIT_INT == queue.priority ) {
                /* catgets 5284 */
                ls_syslog (LOG_ERR, "catgets 5284: %s: File %s in section Queue ending at line %d: Priority value <%s> isn't a positive integer between 1 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_PRIORITY].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
            }
        }
        else {
            queue.priority = 1;
        }

        if (keyList[QKEY_NICE].val != NULL && strcmp (keyList[QKEY_NICE].val, "")) {
            if (my_atoi (keyList[QKEY_NICE].val, INFINIT_SHORT, -INFINIT_SHORT) == INFINIT_INT) {
                /* catgets 5285 */
                ls_syslog (LOG_ERR, "catgets 5285: %s: File %s in section Queue ending at line %d: Nice value <%s> must be an integer; ignored", __func__, filename, *lineNum, keyList[QKEY_NICE].val);
                lsberrno = LSBE_CONF_WARNING;
            }
            else
                queue.nice = my_atoi (keyList[QKEY_NICE].val, INFINIT_SHORT, -INFINIT_SHORT);
            }


        if (keyList[QKEY_UJOB_LIMIT].val != NULL && strcmp (keyList[QKEY_UJOB_LIMIT].val, "")) {
            queue.userJobLimit = my_atoi (keyList[QKEY_UJOB_LIMIT].val, INFINIT_INT, -1);
            if ( INFINIT_INT == queue.userJobLimit ) {
                /* catgets 5286 */
                ls_syslog (LOG_ERR, "catgets 5286: %s: File %s in section Queue ending at line %d: UJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_UJOB_LIMIT].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
                }
            }


        if (keyList[QKEY_PJOB_LIMIT].val != NULL && strcmp (keyList[QKEY_PJOB_LIMIT].val, "")) {
            queue.procJobLimit = my_atof (keyList[QKEY_PJOB_LIMIT].val, INFINIT_FLOAT, -1.0);
            if( fabs (INFINIT_FLOAT - queue.procJobLimit ) < 0.0000001 ) {
                /* catgets 5287 */
                ls_syslog (LOG_ERR, "catgets 5287: %s: File %s in section Queue ending at line %d: PJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %f; ignored", __func__, filename, *lineNum, keyList[QKEY_PJOB_LIMIT].val, INFINIT_FLOAT - 1);
                lsberrno = LSBE_CONF_WARNING;
            }
        }


        if (keyList[QKEY_QJOB_LIMIT].val != NULL
            && strcmp (keyList[QKEY_QJOB_LIMIT].val, ""))
            {
            if ((queue.maxJobs = my_atoi (keyList[QKEY_QJOB_LIMIT].val, INFINIT_INT, -1)) == INFINIT_INT)
                {
                /* catgets 5289 */
                ls_syslog (LOG_ERR, "catgets 5289: %s: File %s in section Queue ending at line %d: QJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_QJOB_LIMIT].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
                }
            }

        if (keyList[QKEY_HJOB_LIMIT].val != NULL && strcmp (keyList[QKEY_HJOB_LIMIT].val, "")) {
            if ((queue.hostJobLimit = my_atoi (keyList[QKEY_HJOB_LIMIT].val, INFINIT_INT, -1)) == INFINIT_INT) {
                /* catgets 5290 */
                ls_syslog (LOG_ERR, "catgets 5290: %s: File %s in section Queue ending at line %d: HJOB_LIMIT value <%s> isn't a non-negative integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_HJOB_LIMIT].val, INFINIT_INT - 1);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_RUN_WINDOW].val != NULL && strcmp (keyList[QKEY_RUN_WINDOW].val, "")) {
            queue.windows = parsewindow (keyList[QKEY_RUN_WINDOW].val, filename, lineNum, queueLabel);

            if (lserrno == LSE_CONF_SYNTAX) {
                lserrno = LSE_NO_ERR;
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_DISPATCH_WINDOW].val != NULL && strcmp (keyList[QKEY_DISPATCH_WINDOW].val, "")) {
            queue.windowsD = parsewindow (keyList[QKEY_DISPATCH_WINDOW].val, filename, lineNum, queueLabel);

            if (lserrno == LSE_CONF_SYNTAX) {
                lserrno = LSE_NO_ERR;
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_DEFAULT_HOST_SPEC].val != NULL
            && strcmp (keyList[QKEY_DEFAULT_HOST_SPEC].val, ""))
            {
            double *cpuFactor = NULL;

            if (options != CONF_NO_CHECK)
                {
                if ((cpuFactor = getModelFactor
                     (keyList[QKEY_DEFAULT_HOST_SPEC].val, info)) == NULL &&
                    (cpuFactor = getHostFactor
                     (keyList[QKEY_DEFAULT_HOST_SPEC].val)) == NULL)
                    {
                    /* catgets 5292 */
                    ls_syslog (LOG_ERR, "catgets 5292: %s: File %s in section Queue ending at line %d: Invalid value <%s> for %s; ignored", __func__, filename, *lineNum, keyList[QKEY_DEFAULT_HOST_SPEC].val, keyList[QKEY_DEFAULT_HOST_SPEC].key);
                    lsberrno = LSBE_CONF_WARNING;
                    }
                }
            if (cpuFactor != NULL || options == CONF_NO_CHECK)
                {
                queue.defaultHostSpec =
                putstr_ (keyList[QKEY_DEFAULT_HOST_SPEC].val);
                if (queue.defaultHostSpec == NULL)
                    {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_DEFAULT_HOST_SPEC].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                    }
                }
            }

        if (parseCpuAndRunLimit (keyList, &queue, filename, lineNum, function_name, info, options) == false && lsberrno == LSBE_NO_MEM) {
            freekeyval (keyList);
            freeQueueInfo (&queue);
            return false;
        }

        if (keyList[QKEY_FILELIMIT].val != NULL && strcmp (keyList[QKEY_FILELIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_FSIZE] = my_atoi (keyList[QKEY_FILELIMIT].val, INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5295 */
                ls_syslog (LOG_ERR, "catgets 5295: %s: File %s in section Queue ending at line %d: FILELIMIT value <%s> isn't a positive integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_FILELIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_DATALIMIT].val != NULL && strcmp (keyList[QKEY_DATALIMIT].val, "")) {
            parseDefAndMaxLimits (keyList[QKEY_DATALIMIT], &queue.defLimits[LSF_RLIMIT_DATA], &queue.rLimits[LSF_RLIMIT_DATA], filename, lineNum, function_name );
        }

        if (keyList[QKEY_STACKLIMIT].val != NULL && strcmp (keyList[QKEY_STACKLIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_STACK] = my_atoi (keyList[QKEY_STACKLIMIT].val,INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5297 */
                ls_syslog (LOG_ERR, "catgets 5297: %s: File %s in section Queue ending at line %d: STACKLIMIT value <%s> isn't a positive integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_STACKLIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_CORELIMIT].val != NULL && strcmp (keyList[QKEY_CORELIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_CORE] = my_atoi (keyList[QKEY_CORELIMIT].val, INFINIT_INT, -1)) == INFINIT_INT) {
                /* catgets 5298 */
                ls_syslog (LOG_ERR, "catgets 5298: %s: File %s in section Queue ending at line %d: CORELIMIT value <%s> isn't a non-negative integer between -1 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_CORELIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_MEMLIMIT].val != NULL && strcmp (keyList[QKEY_MEMLIMIT].val, "")) {
            parseDefAndMaxLimits (keyList[QKEY_MEMLIMIT], &queue.defLimits[LSF_RLIMIT_RSS], &queue.rLimits[LSF_RLIMIT_RSS], filename, lineNum, function_name);
        }


        if (keyList[QKEY_SWAPLIMIT].val != NULL && strcmp (keyList[QKEY_SWAPLIMIT].val, "")) {
            if ((queue.rLimits[LSF_RLIMIT_SWAP] = my_atoi (keyList[QKEY_SWAPLIMIT].val, INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5300 */
                ls_syslog (LOG_ERR, "catgets 5300: %s: File %s in section Queue ending at line %d: SWAPLIMIT value <%s> isn't a positive integer between 0 and %d; ignored", __func__, filename, *lineNum, keyList[QKEY_SWAPLIMIT].val, INFINIT_INT);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_PROCESSLIMIT].val != NULL && strcmp (keyList[QKEY_PROCESSLIMIT].val, "")) {
            parseDefAndMaxLimits (keyList[QKEY_PROCESSLIMIT], &queue.defLimits[LSF_RLIMIT_PROCESS], &queue.rLimits[LSF_RLIMIT_PROCESS], filename, lineNum, function_name);
        }


        if (keyList[QKEY_PROCLIMIT].val != NULL && strcmp (keyList[QKEY_PROCLIMIT].val, "")) {
            if (parseProcLimit(keyList[QKEY_PROCLIMIT].val, &queue, filename, lineNum, function_name) == false) {
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_USERS].val != NULL && !searchAll (keyList[QKEY_USERS].val)) {

            assert( *lineNum <= UINT_MAX );
            queue.userList = parseGroups (keyList[QKEY_USERS].val, filename, lineNum, " in section  Queue ending", USER_GRP, options);
            if (queue.userList == NULL) {
                if (lsberrno == LSBE_NO_MEM) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, filename, "parseGroups");
                }
                else if (numofugroups >= MAX_GROUPS) {
                    /* catgets 5304 */
                    ls_syslog (LOG_ERR, "catgets 5304: %s: File %s in section Queue ending at line %d:  Number of user group <%d> is equal to or greater than MAX_GROUPS <%d>; ignoring the queue for <%s>; ignoring the queue", __func__, filename, *lineNum, numofugroups, MAX_GROUPS, queue.queue);
                }
                else {
                    /* catgets 5305 */
                    ls_syslog (LOG_ERR, "catgets 5305: %s: File %s in section Queue ending at line %d: No valid user or user group specified in USERS for <%s>; ignoring the queue", __func__, filename, *lineNum, queue.queue);   
                }
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
        }

        if (keyList[QKEY_HOSTS].val != NULL) {

            originalString = keyList[QKEY_HOSTS].val;
            subString = getNextWord_ (&originalString);
            while (subString != NULL) {
                if (strcmp (keyList[QKEY_HOSTS].val, "none") == 0) {
                    /* catgets 5307 */
                    ls_syslog (LOG_ERR, "catgets 5307: %s: File %s in section Queue at line %d: \"none\" specified , queue ignored", __func__, filename, *lineNum);
                    lsberrno = LSBE_CONF_WARNING;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                }
                subString = getNextWord_ (&originalString);
            }

            if (strchr (keyList[QKEY_HOSTS].val, '~') != NULL  && ((options & CONF_NO_EXPAND) == 0)) {
                char *outHosts = NULL;
                int numHosts = 0;

                ls_syslog (LOG_DEBUG, "resolveBatchNegHosts: for %s the string is \"%s\"", __func__, keyList[QKEY_HOSTS].val);
                numHosts = resolveBatchNegHosts (keyList[QKEY_HOSTS].val, &outHosts, true);
                if (numHosts > 0) {
                    ls_syslog (LOG_DEBUG, "resolveBatchNegHosts: for %s the string is replaced with \'%s\'", __func__, outHosts);
                }
                else if (numHosts == 0) {
                    /* catgets 5460 */
                    ls_syslog (LOG_WARNING, "catgets 5460: %s: File %s at line %d: there are no hosts found to exclude, replaced with \'%s\'", __func__, filename, *lineNum, outHosts);
                }
                else {
                    if (numHosts == -3) {
                        /* catgets 5461 */
                        ls_syslog (LOG_WARNING, "catgets 5461: %s: \'%s\' The result is that all the hosts are to be excluded.", filename, keyList[QKEY_HOSTS].val);
                    }
                    ls_syslog (LOG_ERR, "catgets 5310: %s: File %s in section Queue ending at line %d: No valid hosts or host group specified in HOSTS for <%s>; ignoring the queue", __func__, filename, *lineNum, queue.queue);   /* catgets 5310 */

                    lsberrno = LSBE_CONF_WARNING;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                }
                free (keyList[QKEY_HOSTS].val);
                keyList[QKEY_HOSTS].val = outHosts;
            }

            assert( *lineNum < UINT_MAX );
            queue.hostList = parseGroups (keyList[QKEY_HOSTS].val, filename, lineNum, " in section Queue ending", HOST_GRP, options);
            if (queue.hostList == NULL) {
                if (lsberrno == LSBE_NO_MEM) {
                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, filename, "parseGroups");
                }
                else {
                    /* catgets 5310 */
                    ls_syslog (LOG_ERR, "catgets 5310: %s: File %s in section Queue ending at line %d: No valid hosts or host group specified in HOSTS for <%s>; ignoring the queue", __func__, filename, *lineNum, queue.queue);
                }
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
        }

        if (keyList[QKEY_CHKPNT].val != NULL && strcmp (keyList[QKEY_CHKPNT].val, "")) {
            if (strlen (keyList[QKEY_CHKPNT].val) >= MAX_LINE_LEN) {
                /* catgets 5439 */
                ls_syslog (LOG_ERR, "catgets 5439: %s: File %s in section Queue ending at line %d: CHKPNT of the queue <%s> is too long <%s>; ignoring", __func__, filename, *lineNum, queue.queue, keyList[QKEY_CHKPNT].val);
                lsberrno = LSBE_CONF_WARNING;
            }
            else {

                int chkpntPrd = 0;
                char dir[MAX_CMD_DESC_LEN];
                char prdstr[10];            // FIXME FIXME FIXME FIXME 10 is awfully particular

                memset (dir,    '\0', strlen( dir ) );
                memset (prdstr, '\0', strlen( prdstr ) );

                sscanf (keyList[QKEY_CHKPNT].val, "%s %s", dir, prdstr);
                queue.chkpntDir = putstr_ (dir);
                chkpntPrd = atoi (prdstr);

                if (queue.chkpntDir == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", strlen (keyList[QKEY_CHKPNT].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                }

                if (chkpntPrd < 0) { // FIXME FIXME FIXME FIXME negative value

                    ls_syslog (LOG_ERR, "catgets 5441: %s: File %s in section Queue ending at line %d:  options for CHKPNT of the queue <%s> is invalid ; ignoring", __func__, filename, *lineNum, queue.queue);    /* catgets 5441 */
                    ls_syslog (LOG_ERR, "catgets 5442: %s: invalid checkpoint period", __func__);    /* catgets 5442 */
                    lsberrno = LSBE_CONF_WARNING;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                }
                queue.qAttrib |= QUEUE_ATTRIB_CHKPNT;
                queue.chkpntPeriod = chkpntPrd * 60;
            }
        }

        if (keyList[QKEY_RERUNNABLE].val != NULL) {
            if (strcasecmp (keyList[QKEY_RERUNNABLE].val, "y") == 0 || strcasecmp (keyList[QKEY_RERUNNABLE].val, "yes") == 0) {
                queue.qAttrib |= QUEUE_ATTRIB_RERUNNABLE;
            }
            else {
                if (strcasecmp (keyList[QKEY_RERUNNABLE].val, "n") != 0 &&
                    strcasecmp (keyList[QKEY_RERUNNABLE].val, "no") != 0) {
                    /* catgets 5445 */
                    ls_syslog (LOG_ERR, "catgets 5445: %s: File %s in section Queue ending at line %d:  options for RERUNNABLE of the queue <%s> is not y|yes|n|no; ignoring", __func__, filename, *lineNum, queue.queue, keyList[QKEY_RERUNNABLE].val);
                    lsberrno = LSBE_CONF_WARNING;
                }
                queue.qAttrib &= ~QUEUE_ATTRIB_RERUNNABLE;
            }
        }

        addBinaryAttributes (filename, lineNum, &queue, &keyList[QKEY_EXCLUSIVE],               QUEUE_ATTRIB_EXCLUSIVE, "EXCLUSIVE");
        addBinaryAttributes (filename, lineNum, &queue, &keyList[QKEY_BACKFILL],                QUEUE_ATTRIB_BACKFILL, "BACKFILL");
        addBinaryAttributes (filename, lineNum, &queue, &keyList[QKEY_IGNORE_DEADLINE],         QUEUE_ATTRIB_IGNORE_DEADLINE, "IGNORE_DEADLINE");
        addBinaryAttributes (filename, lineNum, &queue, &keyList[QKEY_ENQUE_INTERACTIVE_AHEAD], QUEUE_ATTRIB_ENQUE_INTERACTIVE_AHEAD, "ENQUE_INTERACTIVE_AHEAD");
        addBinaryAttributes (filename, lineNum, &queue, &keyList[QKEY_ROUND_ROBIN_POLICY],      QUEUE_ATTRIB_ROUND_ROBIN, "ROUND_ROBIN_POLICY");

        if (keyList[QKEY_INTERACTIVE].val != NULL) {
            if (strcasecmp (keyList[QKEY_INTERACTIVE].val, "n") == 0 ||
                strcasecmp (keyList[QKEY_INTERACTIVE].val, "no") == 0) {
                queue.qAttrib |= QUEUE_ATTRIB_NO_INTERACTIVE;
            }
            else if (strcasecmp (keyList[QKEY_INTERACTIVE].val, "only") == 0) {
                queue.qAttrib |= QUEUE_ATTRIB_ONLY_INTERACTIVE;
            }
            else if( ( strcasecmp (keyList[QKEY_INTERACTIVE].val, "yes") != 0 ) && ( strcasecmp (keyList[QKEY_INTERACTIVE].val, "y") != 0 ) ) {
                /* catgets 5311 */
                ls_syslog (LOG_ERR, "catgets 5311:  %s: File %s in section Queue ending at line %d: INTERACTIVE value <%s> isn't one of 'Y', 'y', 'N', 'n' or 'ONLY'; ignored", __func__, filename, *lineNum, keyList[QKEY_INTERACTIVE].val);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_JOB_ACCEPT_INTERVAL].val != NULL && keyList[QKEY_JOB_ACCEPT_INTERVAL].position >= 0 && strcmp (keyList[QKEY_JOB_ACCEPT_INTERVAL].val, "")) {
            if ((queue.acceptIntvl = my_atoi (keyList[QKEY_JOB_ACCEPT_INTERVAL].val, INFINIT_INT, -1)) == INFINIT_INT) {
                /* catgets 5313 */
                ls_syslog (LOG_ERR, "catgets 5313: %s: File %s in section Queue ending at line %d: JOB_ACCEPT_INTERVAL value <%s> isn't an integer greater than -1; ignored", __func__, filename, *lineNum, keyList[QKEY_JOB_ACCEPT_INTERVAL].val);    
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_NEW_JOB_SCHED_DELAY].val != NULL && keyList[QKEY_NEW_JOB_SCHED_DELAY].position >= 0 && strcmp (keyList[QKEY_NEW_JOB_SCHED_DELAY].val, "")) {
            if ((queue.schedDelay = my_atoi (keyList[QKEY_NEW_JOB_SCHED_DELAY].val, INFINIT_INT, -1)) == INFINIT_INT) {
                /* catgets 5315 */
                ls_syslog (LOG_ERR, "catgets 5315: %s: File %s in section Queue ending at line %d: NEW_JOB_SCHED_DELAY value <%s> isn't an integer greater than -1; ignored", __func__, filename, *lineNum, keyList[QKEY_NEW_JOB_SCHED_DELAY].val);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_POLICIES].val != NULL) {
            sp = keyList[QKEY_POLICIES].val;
            while ((word = getNextWord_ (&sp)) != NULL) {
                if (strcasecmp (word, "EXCLUSIVE") == 0) {
                    queue.qAttrib |= QUEUE_ATTRIB_EXCLUSIVE;
                }
                else {
                    ls_syslog (LOG_ERR, "catgets 5317: %s: File %s in section Queue ending at line %d: POLICIES value <%s> unrecognizable; ignored", __func__, filename, *lineNum, word);  /* catgets 5317 */
                    lsberrno = LSBE_CONF_WARNING;
                }
            }
        }

        if (keyList[QKEY_DESCRIPTION].val != NULL) {
            if (strlen (keyList[QKEY_DESCRIPTION].val) > 10 * MAX_LINE_LEN) { // FIXME FIXME why 10 times MAX_LINE_LEN?
                ls_syslog (LOG_ERR, "catgets 5338: %s: File %s in section Queue ending at line %d: Too many characters in DESCRIPTION of the queue; truncated", __func__, filename, *lineNum); /* catgets 5338 */
                lsberrno = LSBE_CONF_WARNING;
                keyList[QKEY_DESCRIPTION].val[10 * MAX_LINE_LEN - 1] = '\0';
            }
            queue.description = putstr_ (keyList[QKEY_DESCRIPTION].val);
            if (queue.description == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_DESCRIPTION].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
        }

        if (keyList[QKEY_MIG].val != NULL && strcmp (keyList[QKEY_MIG].val, "")) {

            if ((queue.mig = my_atoi (keyList[QKEY_MIG].val, INFINIT_INT / 60, -1)) == INFINIT_INT) {
                /* catgets 5340 */
                ls_syslog (LOG_ERR, "catgets 5340: %s: File %s in section Queue ending at line %d: Invalid value <%s> for MIG; no MIG threshold is assumed", __func__, filename, *lineNum, keyList[QKEY_MIG].val);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        if (keyList[QKEY_ADMINISTRATORS].val != NULL && strcmp (keyList[QKEY_ADMINISTRATORS].val, "")) {
            if (options & CONF_NO_CHECK) { 
                queue.admins = parseAdmins (keyList[QKEY_ADMINISTRATORS].val, options, filename, lineNum); // FIXME fix above line
            }
            else {
                queue.admins = putstr_ (keyList[QKEY_ADMINISTRATORS].val);
            }
            if (queue.admins == NULL)  {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", strlen (keyList[QKEY_ADMINISTRATORS].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
            if (queue.admins[0] == '\0') {
                ls_syslog (LOG_ERR, "catgets 5343: %s: File %s in section Queue ending at line %d: No valid administrators <%s> specified for queue <%s>;ignoring", __func__, filename, *lineNum, keyList[QKEY_ADMINISTRATORS].val, queue.queue);  /* catgets 5343 */
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (queue.admins);
            }
        }

        if (keyList[QKEY_PRE_EXEC].val != NULL && strcmp (keyList[QKEY_PRE_EXEC].val, "")) {
            if (strlen (keyList[QKEY_PRE_EXEC].val) >= MAX_LINE_LEN) {
                ls_syslog (LOG_ERR, "catgets 5344: %s: File %s in section Queue ending at line %d: PRE_EXEC of the queue <%s> is too long <%s>; ignoring", __func__, filename, *lineNum, queue.queue, keyList[QKEY_PRE_EXEC].val); /* catgets 5344 */
                lsberrno = LSBE_CONF_WARNING;
            }
            else {
                queue.preCmd = putstr_ (keyList[QKEY_PRE_EXEC].val);
                if (queue.preCmd == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_PRE_EXEC].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                }
            }
        }

        if (keyList[QKEY_PRE_POST_EXEC_USER].val != NULL && strcmp (keyList[QKEY_PRE_POST_EXEC_USER].val, "")) {
            if (strlen (keyList[QKEY_PRE_POST_EXEC_USER].val) >= MAX_LINE_LEN) {
                ls_syslog (LOG_ERR, "catgets 5352: %s: User name %s in section Queue ending at line %d: PRE_POST_EXEC_USER of the queue <%s> is too long <%s>; ignoring", __func__, filename, *lineNum, queue.queue, keyList[QKEY_PRE_POST_EXEC_USER].val);    /* catgets 5352 */
                lsberrno = LSBE_CONF_WARNING;
            }
            else {
                queue.prepostUsername = putstr_ (keyList[QKEY_PRE_POST_EXEC_USER].val);
                if (queue.prepostUsername == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_PRE_POST_EXEC_USER].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                }
            }
        }

        if (keyList[QKEY_POST_EXEC].val != NULL && strcmp (keyList[QKEY_POST_EXEC].val, "")) {
            if (strlen (keyList[QKEY_POST_EXEC].val) >= MAX_LINE_LEN) {
                ls_syslog (LOG_ERR, "catgets 5347: %s: File %s in section Queue ending at line %d: POST_EXEC of the queue <%s> is too long <%s>; ignoring", __func__, filename, *lineNum, queue.queue, keyList[QKEY_POST_EXEC].val);   /* catgets 5347 */
                lsberrno = LSBE_CONF_WARNING;
            }
            else {
                queue.postCmd = putstr_ (keyList[QKEY_POST_EXEC].val);
                if (queue.postCmd == NULL) {
                    ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_POST_EXEC].val) + 1);
                    lsberrno = LSBE_NO_MEM;
                    freekeyval (keyList);
                    freeQueueInfo (&queue);
                    return false;
                }
            }
        }

        if (keyList[QKEY_REQUEUE_EXIT_VALUES].val != NULL && strcmp (keyList[QKEY_REQUEUE_EXIT_VALUES].val, "")) {
            if (strlen (keyList[QKEY_REQUEUE_EXIT_VALUES].val) >= MAX_LINE_LEN) {
                ls_syslog (LOG_ERR, "catgets 5350: %s: File %s in section Queue ending at line %d: REQUEUE_EXIT_VALUES  of the queue <%s> is too long <%s>; ignoring", __func__, filename, *lineNum, queue.queue, keyList[QKEY_REQUEUE_EXIT_VALUES].val);  /* catgets 5350 */
                lsberrno = LSBE_CONF_WARNING;
            }
            else if (!checkRequeEValues (&queue, keyList[QKEY_REQUEUE_EXIT_VALUES].val, filename, lineNum) && lsberrno == LSBE_NO_MEM) {
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
            else {
                printf( "we done goofed up at %s", __func__ );
                ls_syslog( LOG_ERR, "we done goofed up at %s", __func__ );
            }
        }

        if (keyList[QKEY_RES_REQ].val != NULL && strcmp (keyList[QKEY_RES_REQ].val, "")) {
            queue.resReq = putstr_ (keyList[QKEY_RES_REQ].val);
            if (queue.resReq == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_RES_REQ].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
        }

        if (keyList[QKEY_SLOT_RESERVE].val != NULL && strcmp (keyList[QKEY_SLOT_RESERVE].val, "")) {
            getReserve (keyList[QKEY_SLOT_RESERVE].val, &queue, filename, *lineNum);
        }

        if (keyList[QKEY_RESUME_COND].val != NULL && strcmp (keyList[QKEY_RESUME_COND].val, "")) {
            queue.resumeCond = putstr_ (keyList[QKEY_RESUME_COND].val);
            if (queue.resumeCond == NULL) {

                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_RESUME_COND].val));
                lsberrno = LSBE_NO_MEM;
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
        }

        if (keyList[QKEY_STOP_COND].val != NULL && strcmp (keyList[QKEY_STOP_COND].val, "")) {
            queue.stopCond = putstr_ (keyList[QKEY_STOP_COND].val);
            if (queue.stopCond == NULL) {

                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_STOP_COND].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
        }

        if (keyList[QKEY_JOB_STARTER].val != NULL && strcmp (keyList[QKEY_JOB_STARTER].val, "")) {
            queue.jobStarter = putstr_ (keyList[QKEY_JOB_STARTER].val);
            if (queue.jobStarter == NULL) {
                ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (keyList[QKEY_JOB_STARTER].val) + 1);
                lsberrno = LSBE_NO_MEM;
                freekeyval (keyList);
                freeQueueInfo (&queue);
                return false;
            }
        }

        if (keyList[QKEY_JOB_CONTROLS].val != NULL && strcmp (keyList[QKEY_JOB_CONTROLS].val, "") && 
                parseSigActCmd (&queue, keyList[QKEY_JOB_CONTROLS].val, filename, lineNum, "in section Queue ending") < 0 ) {
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keyList);
            freeQueueInfo (&queue);
            return false;
        }

        if (keyList[QKEY_TERMINATE_WHEN].val != NULL && strcmp (keyList[QKEY_TERMINATE_WHEN].val, "") && 
                terminateWhen (&queue, keyList[QKEY_TERMINATE_WHEN].val, filename, lineNum, "in section Queue ending") < 0 ) {
            lsberrno = LSBE_CONF_WARNING;
            freekeyval (keyList);
            freeQueueInfo (&queue);
            return false;
        }

        assert( info->numIndx >= 0 );
        queue.loadSched = calloc( info->numIndx, sizeof (float *));
        if (info->numIndx && NULL == queue.loadSched && ENOMEM == errno) {
            lsberrno = LSBE_NO_MEM;
            freekeyval (keyList);
            freeQueueInfo (&queue);
            return false;
        }
        assert( info->numIndx >=0 );
        queue.loadStop = calloc( info->numIndx, sizeof (float *));
        if (info->numIndx && NULL == queue.loadStop && ENOMEM == errno ) {
            lsberrno = LSBE_NO_MEM;
            freekeyval (keyList);
            freeQueueInfo (&queue);
            return false;
        }

        getThresh (info, keyList, queue.loadSched, queue.loadStop, filename, lineNum, " in section Queue ending");
        queue.nIdx = info->numIndx;
        if (!addQueue (&queue, filename, *lineNum) && lsberrno == LSBE_NO_MEM) {
            freekeyval (keyList);
            freeQueueInfo (&queue);
            return false;
        }

        freekeyval (keyList);
        return true;
    }

    return false;
}


void
initQueueInfo (struct queueInfoEnt *qp)
{
    qp->queue           = NULL;
    qp->description     = NULL;
    qp->userList        = NULL;
    qp->hostList        = NULL;
    qp->loadSched       = NULL;
    qp->loadStop        = NULL;
    qp->windows         = NULL;
    qp->hostSpec        = NULL;
    qp->windowsD        = NULL;
    qp->defaultHostSpec = NULL;
    qp->admins          = NULL;
    qp->preCmd          = NULL;
    qp->postCmd         = NULL;
    qp->prepostUsername = NULL;
    qp->requeueEValues  = NULL;
    qp->resReq          = NULL;
    qp->priority        = INFINIT_INT;
    qp->nice            = INFINIT_SHORT;
    qp->nIdx            = 0;
    qp->userJobLimit    = INFINIT_INT;
    qp->procJobLimit    = INFINIT_FLOAT;
    qp->qAttrib         = 0;
    qp->qStatus         = INFINIT_INT;
    qp->maxJobs         = INFINIT_INT;
    qp->numJobs         = INFINIT_INT;
    qp->numPEND         = INFINIT_INT;
    qp->numRUN          = INFINIT_INT;
    qp->numSSUSP        = INFINIT_INT;
    qp->numUSUSP        = INFINIT_INT;
    qp->mig             = INFINIT_INT;
    qp->schedDelay      = INFINIT_INT;
    qp->acceptIntvl     = INFINIT_INT;
    qp->procLimit       = INFINIT_INT;
    qp->minProcLimit    = INFINIT_INT;
    qp->defProcLimit    = INFINIT_INT;
    qp->hostJobLimit    = INFINIT_INT;

    for ( unsigned int i = 0; i < LSF_RLIM_NLIMITS; i++) {
        qp->rLimits[i]   = INFINIT_INT;
        qp->defLimits[i] = INFINIT_INT;
    }

    qp->numRESERVE      = INFINIT_INT;
    qp->slotHoldTime    = INFINIT_INT;
    qp->resumeCond      = NULL;
    qp->stopCond        = NULL;
    qp->jobStarter      = NULL;

    qp->suspendActCmd   = NULL;
    qp->resumeActCmd    = NULL;
    qp->terminateActCmd = NULL;

    for ( unsigned int i = 0; i < LSB_SIG_NUM; i++) {
        qp->sigMap[i] = 0;
    }

    qp->chkpntPeriod = -1;
    qp->chkpntDir    = NULL;

    return;
}

void
freeQueueInfo (struct queueInfoEnt *qp)
{
    if (qp == NULL) {
        return;
    }

    FREEUP( qp->queue );
    FREEUP( qp->description );
    FREEUP( qp->userList );
    FREEUP( qp->hostList );
    FREEUP( qp->loadSched );
    FREEUP( qp->loadStop );
    FREEUP( qp->windows );
    FREEUP( qp->hostSpec );
    FREEUP( qp->windowsD );
    FREEUP( qp->defaultHostSpec );
    FREEUP( qp->admins );
    FREEUP( qp->preCmd );
    FREEUP( qp->postCmd );
    FREEUP( qp->prepostUsername );
    FREEUP( qp->requeueEValues );
    FREEUP( qp->resReq );
    FREEUP( qp->jobStarter );
    FREEUP( qp->stopCond );
    FREEUP( qp->resumeCond );
    FREEUP( qp->chkpntDir );
    FREEUP( qp->suspendActCmd );
    FREEUP( qp->resumeActCmd );
    FREEUP( qp->terminateActCmd );

    return;
}

bool
checkRequeEValues (struct queueInfoEnt *qp, char *word, const char *filename, size_t *lineNum)
{
// #define NORMAL_EXIT 0
// #define EXCLUDE_EXIT 1
    enum exit_status {
        NORMAL_EXIT,
        EXCLUDE_EXIT
    };
    
    int numEValues = 0;
    int exitV      = 0;
    // int i          = 0;
    int found      = 0; // false
    int mode       = NORMAL_EXIT;
    char *sp       = NULL;
    char *cp       = NULL;
    int exitInts[400];  // FIXME FIXME IFME FIXME wtf why 400 exit statuses?
    char exitValues[MAX_LINE_LEN];

    memset( exitInts,   0, sizeof( exitInts ) );
    memset( exitValues, '\0', strlen( exitValues ) );

    cp = word;

    while ((sp = a_getNextWord_ (&word)) != NULL) {
        if (isint_ (sp) && (exitV = my_atoi (sp, 256, -1)) != INFINIT_INT) {
            found = false;
            for ( unsigned int i = 0; i < numEValues; i++) {
                if (exitInts[i] == exitV) {
                    found = true;
                    break;
                }
            }
            if (found == true) {
                if (filename) {      // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
                    ls_syslog (LOG_ERR, "catgets 5376: %s: File %s in section Queue ending at line %d: requeue exit value <%s> for queue <%s> is repeated; ignoring", __func__, filename, *lineNum, sp, qp->queue);    /* catgets 5376 */
                }
                lsberrno = LSBE_CONF_WARNING;
                continue;
            }
            strcat (exitValues, sp);
            strcat (exitValues, " ");
            exitInts[numEValues] = exitV;
            numEValues++;
        }
        else if (strncasecmp (sp, "EXCLUDE(", 8) == 0) {
            strcat (exitValues, sp);
            strcat (exitValues, " ");
            mode = EXCLUDE_EXIT;
        }
        else if (*sp == ')' && mode == EXCLUDE_EXIT) {
            strcat (exitValues, sp);
            strcat (exitValues, " ");
            mode = NORMAL_EXIT;
        }
        else {
            if (filename) {     // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
                ls_syslog (LOG_ERR, "catgets 5377: %s: File %s in section Queue ending at line %d: requeue exit values <%s> for queue <%s> is not an interger between 0-255; ignored", __func__, filename, *lineNum, sp, qp->queue);   /* catgets 5377 */
            }
            lsberrno = LSBE_CONF_WARNING;
        }
    }

    if (numEValues == 0) {
        if (!filename) {       // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
            ls_syslog (LOG_ERR, "catgets 5378: %s: File %s in section Queue ending at line %d: No valid requeue exit values <%s> for queue <%s>; ignoring", __func__, filename, *lineNum, cp, qp->queue);   /* catgets 5378 */
        }
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }
    qp->requeueEValues = putstr_ (exitValues);
    if (qp->requeueEValues == NULL) {
        if (!filename) {       // FIXME FIXME FIXME FIXME this section is suspcet; coudl be a !filename
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", strlen (exitValues) + 1);
        }
        lsberrno = LSBE_NO_MEM;
        return false;
    }

    return true;
}

bool
addQueue (struct queueInfoEnt *qp, const char *filename, unsigned int lineNum)
{
    
    struct queueInfoEnt **tmpQueues = NULL;

    assert( *filename );
    assert( lineNum );

    if (qp == NULL) {
        return true;
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
            return false;
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
        return false;
    }

    initQueueInfo (queues[numofqueues]);
    *queues[numofqueues] = *qp;
    numofqueues++;
    return true;
}


void
freeWorkUser (int freeAll)
{
    for ( unsigned int i = 0; i < numofugroups; i++) {
        if (freeAll == true) {
            freeGroupInfo (usergroups[i]);
        }
        FREEUP (usergroups[i]);
    }
    numofugroups = 0;
 
    for ( unsigned int i = 0; i < numofusers; i++) {
        if (freeAll == true) {
            freeUserInfo (users[i]);
        }
        FREEUP (users[i]);
    }
    FREEUP (users);
    numofusers = 0;

    return;
}

void
freeWorkHost (int freeAll)
{
    for ( unsigned int i = 0; i < numofhosts; i++) {
        if (freeAll == true) {
            freeHostInfoEnt( hosts[i] );
        }
        FREEUP (hosts[i]);
    }
    FREEUP (hosts);
    numofhosts = 0;

    for ( unsigned int i = 0; i < numofhgroups; i++) {
        if (freeAll == true) {
            freeGroupInfo (hostgroups[i]);
        }
        FREEUP (hostgroups[i]);
    }
    numofhgroups = 0;

    return;
}

void
freeWorkQueue (int freeAll)
{
    for ( unsigned int i = 0; i < numofqueues; i++) {
        if (freeAll == true) {
            freeQueueInfo (queues[i]);
        }
        FREEUP (queues[i]);
    }
    FREEUP (queues);
    numofqueues = 0;

    return;
}

void
freeUConf (struct userConf *uConf1, int freeAll)
{
    for ( unsigned int i = 0; i < uConf1->numUgroups; i++) {
        if (freeAll == true) {
            freeGroupInfo (&uConf1->ugroups[i]);
        }
    }

    FREEUP (uConf1->ugroups);
    uConf1->numUgroups = 0;

    for ( unsigned int i = 0; i < uConf1->numUsers; i++) {
        if (freeAll == true) {
            freeUserInfo (&uConf1->users[i]);
        }
    }

    FREEUP (uConf1->users);
    uConf1->numUsers = 0;

    return;
}

void
freeHConf (struct hostConf *hConf1, int freeAll)
{
    for ( unsigned int i = 0; i < hConf1->numHosts; i++) {
        if (freeAll == true) {
            freeHostInfoEnt ( &hConf1->hosts[i]);
        }
    }

    FREEUP (hConf1->hosts);
    hConf1->numHosts = 0;
    
    for ( unsigned int i = 0; i < hConf1->numHgroups; i++)  {
        if (freeAll == true) {
            freeGroupInfo (&hConf1->hgroups[i]);
        }
    }
    FREEUP (hConf1->hgroups);
    hConf1->numHgroups = 0;

    return;
}

void
freeQConf (struct queueConf *qConf1, int freeAll)
{
    for ( unsigned int i = 0; i < qConf1->numQueues; i++) {
        if (freeAll == true) {
            freeQueueInfo (&qConf1->queues[i]);
        }
    }
    FREEUP (qConf1->queues);
    qConf1->numQueues = 0;

    return;
}

void
resetUConf (struct userConf *uConf1)
{
    uConf1->numUgroups = 0;
    uConf1->numUsers = 0;
    uConf1->ugroups = NULL;
    uConf1->users = NULL;

    return;
}

void
resetHConf (struct hostConf *hConf1)
{
    hConf1->numHosts   = 0;
    hConf1->numHgroups = 0;
    hConf1->hosts      = NULL;
    hConf1->hgroups    = NULL;

    return;   
}

void
checkCpuLimit (char **hostSpec, double **cpuFactor, int useSysDefault, const char *filename, size_t *lineNum, const char *pname, struct lsInfo *info, int options)
{
    if( ( *hostSpec ) && ( *cpuFactor == NULL ) && ( options != CONF_NO_CHECK ) && ( (*cpuFactor = getModelFactor (*hostSpec, info)) == NULL) && ( (*cpuFactor = getHostFactor (*hostSpec)) == NULL) ) {
        if (useSysDefault == true) {
            /* catgets 5383 */
            ls_syslog (LOG_ERR, "catgets 5383: [%s] %s: File %s in section Queue end at line %ld: Invalid DEFAULT_HOST_SPEC <%s>; ignored", __func__, pname, filename, *lineNum, pConf->defaultHostSpec);

            lsberrno = LSBE_CONF_WARNING;
            FREEUP (pConf->defaultHostSpec);
            FREEUP (*hostSpec);
        }
        else {
            /* catgets 5384 */
            ls_syslog (LOG_ERR, "catgets 5384: [%s] %s: File %s in section Queue end at line %ld: Invalid host_spec <%s>; ignored", __func__, pname, filename, *lineNum, *hostSpec);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (*hostSpec);
        }
    }

    return;
}

bool
parseCpuAndRunLimit (struct keymap *keylist, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, const char *pname, struct lsInfo *info, int options)
{
    int limit            = 0; 
    bool retValue        = false;
    int useSysDefault    = false;
    double *cpuFactor    = NULL;
    char   *spec         = NULL;
    char   *sp           = NULL;
    char   *hostSpec     = NULL;
    char   *defaultLimit = NULL;
    char   *maxLimit     = NULL;

    struct keymap key = keylist[QKEY_CPULIMIT]; // FIXME FIXME FIXME FIXME wtf is this reference from?

    defaultLimit = NULL;
    maxLimit = NULL;

    sp = key.val;
    if (sp != NULL) {
        defaultLimit = putstr_ (getNextWord_ (&sp));
        maxLimit = getNextWord_ (&sp);
    }

    if (maxLimit != NULL) {
        
        retValue = parseLimitAndSpec (defaultLimit, &limit, &spec, hostSpec, key.key, qp, filename, lineNum, pname);  
        if (retValue == true) {
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
        ls_syslog (LOG_ERR, "catgets 5464: %s: File %s in section Queue ending at line %d: CPULIMIT for queue has extra parameters: %s; These parameters will be ignored.", pname, filename, *lineNum, sp); 
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
        if (retValue == true) {
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
    if (retValue == true) {
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
        ls_syslog (LOG_ERR, "catgets 5464: %s: File %s in section Queue ending at line %d: RUNLIMIT for queue has extra parameters: %s; These parameters will be ignored.", pname, filename, *lineNum, sp); 
        lsberrno = LSBE_CONF_WARNING;
    }

    if (qp->defLimits[LSF_RLIMIT_CPU] != INFINIT_INT && qp->rLimits[LSF_RLIMIT_CPU] != INFINIT_INT && qp->defLimits[LSF_RLIMIT_CPU] > qp->rLimits[LSF_RLIMIT_CPU]) {
        /* catgets 5111 */
        ls_syslog (LOG_ERR, "catgets 5111: %s: File %s in section Queue at line %d: The default CPULIMIT %d should not be greater than the max CPULIMIT %d; ignoring the default CPULIMIT and using max CPULIMIT also as default CPULIMIT", pname, filename, *lineNum, qp->defLimits[LSF_RLIMIT_CPU], qp->rLimits[LSF_RLIMIT_CPU]);
        qp->defLimits[LSF_RLIMIT_CPU] = qp->rLimits[LSF_RLIMIT_CPU];
    }

    if (qp->defLimits[LSF_RLIMIT_RUN] != INFINIT_INT && qp->rLimits[LSF_RLIMIT_RUN] != INFINIT_INT && qp->defLimits[LSF_RLIMIT_RUN] > qp->rLimits[LSF_RLIMIT_RUN]) {
        /* catgets 5110 */
        ls_syslog (LOG_ERR, "catgets 5110: %s: File %s in section Queue at line %d: The default RUNLIMIT %d should not be greater than the max RUNLIMIT %d; ignoring the default RUNLIMIT and using max RUNLIMIT also as default RUNLIMIT", pname, filename, *lineNum, qp->defLimits[LSF_RLIMIT_RUN], qp->rLimits[LSF_RLIMIT_RUN]);
        qp->defLimits[LSF_RLIMIT_RUN] = qp->rLimits[LSF_RLIMIT_RUN];
    }

    if (hostSpec != NULL) {
        checkCpuLimit (&hostSpec, &cpuFactor, useSysDefault, filename, lineNum, pname, info, options);
    }

    if (cpuFactor == NULL && options & CONF_RETURN_HOSTSPEC) {
        if (qp->defaultHostSpec != NULL && qp->defaultHostSpec[0]) {
            
            hostSpec = putstr_ (qp->defaultHostSpec);
            if (hostSpec && hostSpec[0]) { // FIXME FIXME FIXME label [0]
                checkCpuLimit (&hostSpec, &cpuFactor, useSysDefault, filename, lineNum, pname, info, options);
            }
        }
    }

    if (cpuFactor == NULL && options & CONF_RETURN_HOSTSPEC) {
        if (pConf && pConf->param && pConf->defaultHostSpec != NULL && pConf->defaultHostSpec[0]) {
            
            hostSpec = putstr_ (pConf->defaultHostSpec);
            useSysDefault = true;
            if (hostSpec && hostSpec[0]) { // FIXME FIXME FIXME label [0]
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
                qp->rLimits[LSF_RLIMIT_CPU] = limit_;
            }

            if (qp->defLimits[LSF_RLIMIT_CPU] > 0 && qp->defLimits[LSF_RLIMIT_CPU] != INFINIT_INT && (options & CONF_RETURN_HOSTSPEC)) {
                limit_ = qp->defLimits[LSF_RLIMIT_CPU] * (*cpuFactor);
                limit_ = limit_ < 1 ? 1 : limit_ + 0.5;
                qp->defLimits[LSF_RLIMIT_CPU] = limit_;
            }

            if (qp->rLimits[LSF_RLIMIT_RUN] > 0 && qp->rLimits[LSF_RLIMIT_RUN] != INFINIT_INT && (options & CONF_RETURN_HOSTSPEC)) {
                limit_ = qp->rLimits[LSF_RLIMIT_RUN] * (*cpuFactor);
                limit_ = limit_ < 1 ? 1 : limit_ + 0.5;
                qp->rLimits[LSF_RLIMIT_RUN] = limit_;
            }

            if (qp->defLimits[LSF_RLIMIT_RUN] > 0 && qp->defLimits[LSF_RLIMIT_RUN] != INFINIT_INT && (options & CONF_RETURN_HOSTSPEC)) {
                limit_ = qp->defLimits[LSF_RLIMIT_RUN] * (*cpuFactor);
                limit_ = limit_ < 1 ? 1 : limit_ + 0.5;
                qp->defLimits[LSF_RLIMIT_RUN] = limit_;
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
    return true;
}

bool
parseProcLimit (char *word, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, const char *pname )
{
    int values[ ] = { 0, 0, 0 }; // FIXME FIXME FIXME label each value
    char *sp      = NULL;
    char *curWord = NULL;

    assert( pname ); // FIXME FIXME where was this supposed to go?

    enum {
        MINPROCLIMIT,
        DEFAULTPROCLIMIT,
        PROCLIMIT
    };

    sp = word;
    if (sp == NULL) {
        // FIXME FIXME FIXME FIXME set lsferr or something here to notify caller
        return false;
    }
    else {

        unsigned int i = 0;
        while (*sp != '\0' && *sp != '#') {
            sp++;
        }

        if (*sp == '#') {
            *sp = '\0';
        }

        sp = word;
        for ( i = 0; i < ( sizeof( values) / sizeof( values[MINPROCLIMIT] - 1) ); i++) {
            curWord = getNextWord_ (&sp);
            if (curWord == NULL) {
                break;
            }
            if ((values[i] = my_atoi (curWord, INFINIT_INT, 0)) == INFINIT_INT) {
                /* catgets 5302 */
                ls_syslog (LOG_ERR, "catgets 5302: %s: File %s in section Queue ending at line %d: PROCLIMIT value <%s> isn't a positive integer; ignored", pname, filename, *lineNum, curWord);
                return false;
            }
        }
        if (getNextWord_ (&sp) != NULL) {
            /* catgets 5371 */
            ls_syslog (LOG_ERR, "catgets 5371: %s: File %s in section Queue ending at line %d: PROCLIMIT has too many parameters; ignored. PROCLIMIT=[minimum [default]] maximum", pname, filename, *lineNum);
            return false;
        }
        
        switch (i) {
            case 1:
                qp->procLimit = values[MINPROCLIMIT];
                qp->minProcLimit = 1;
                qp->defProcLimit = 1;
            break;
            case 2:
                if (values[MINPROCLIMIT] > values[DEFAULTPROCLIMIT]) {
                    /* catgets 5370 */
                    ls_syslog (LOG_ERR, "catgets 5370: %s: File %s in section Queue ending at line %d: PROCLIMIT values <%d %d> are not valid; ignored. PROCLIMIT values must satisfy the following condition: 1 <= minimum <= maximum", pname, filename, *lineNum, values[MINPROCLIMIT], values[DEFAULTPROCLIMIT]);
                    return false;
                }
                else {
                    qp->minProcLimit = values[MINPROCLIMIT];
                    qp->defProcLimit = values[MINPROCLIMIT];
                    qp->procLimit    = values[DEFAULTPROCLIMIT];
                }
            break;
            case 3:
                if (!(values[MINPROCLIMIT] <= values[DEFAULTPROCLIMIT] && values[DEFAULTPROCLIMIT] <= values[PROCLIMIT])) {
                    /* catgets 5374 */
                    ls_syslog (LOG_ERR, "catgets 5374: %s: File %s in section Queue ending at line %d: PROCLIMIT value <%d %d %d> is not valid; ignored. PROCLIMIT values must satisfy the following condition: 1 <= minimum <= default <= maximum", filename, filename, *lineNum, values[MINPROCLIMIT], values[DEFAULTPROCLIMIT], values[PROCLIMIT]);
                    return false;
                }
                else {
                    qp->minProcLimit = values[MINPROCLIMIT];
                    qp->defProcLimit = values[DEFAULTPROCLIMIT];
                    qp->procLimit    = values[PROCLIMIT];
                }
            break;
            default: 
                printf( "we done goofed up at parseProcLimit()\n");
            break;
            }
        }

    return true;
}

bool
parseLimitAndSpec (char *word, int *limit, char **spec, char *hostSpec, char *param, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, const char *pname )
{
    int limitVal = -1;
    char *sp     = NULL;

    assert( pname ); // FIXME FIXME where was this supposed to go?

    // *limit = -1; // FIXME FIXME FIXME FIXME nani? why set an incoming variable?
    FREEUP (*spec);
    assert( qp->nice );
    if (word == NULL) {
        return false;
    }

    if (word && (sp = strchr (word, '/')) != NULL) {
        *sp = '\0';
        *spec = putstr_ (sp + 1);
    }

    if (*spec && hostSpec && strcmp (*spec, hostSpec) != 0) {
        /* catgets 5382 */
        ls_syslog (LOG_ERR, "catgets 5382: %s: File %s in section Queue at line %d: host_spec for %s is multiply defined; ignoring <%s> and retaining last host_spec <%s>", __func__, filename, *lineNum, param, *spec, hostSpec);
        lsberrno = LSBE_CONF_WARNING;
        FREEUP (*spec);
    }

    if ((sp = strchr (word, ':')) != NULL) {
        sp = NULL;
    }

    limitVal = my_atoi (word, INFINIT_INT / 60 + 1, -1);
    if (limitVal == INFINIT_INT) {
        /* catgets 5386 */
        ls_syslog (LOG_ERR, "catgets 5386: %s: File %s in section Queue at line %d: Value <%s> of %s isn't a positive integer between 0 and %d; ignored.", __func__, filename, *lineNum, word, param, INFINIT_INT / 60);
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
            ls_syslog (LOG_ERR, "catgets 5386: %s: File %s in section Queue at line %d: Value <%s> of %s isn't a positive integer between 0 and %d; ignored.", __func__, filename, *lineNum, word, param, INFINIT_INT / 60);
            lsberrno = LSBE_CONF_WARNING;
            *limit = -1;
        }
        else {
            *limit += limitVal;
            *limit *= 60;
        }
    }

    return true;    
}

double *
getModelFactor ( const char *hostModel, struct lsInfo *info)
{
    double *cpuFactor = 0.0;
    
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

double *
getHostFactor ( const char *hostname)
{
    double *cpuFactor  = 0;
    struct hostent *hp = NULL;

    if (NULL == hostname) {
        return NULL;
    }
    if (hConf == NULL || hConf->numHosts == 0 || hConf->hosts == NULL) {
        return NULL;
    }

    hp = Gethostbyname_ (hostname);
    if (NULL == hp ) {
        return NULL;
    }

    for ( unsigned int i = 0; i < hConf->numHosts; i++) {
        if (equalHost_ (hp->h_name, hConf->hosts[i].hostname)) {
            *cpuFactor = hConf->hosts[i].cpuFactor;
            return cpuFactor;
        }
    }

    return NULL;
}

char *
parseAdmins ( const char *admins, int options, const char *filename, size_t *lineNum)
{
    
    char *sp                  = NULL;
    char *word                = NULL;
    char *expandAds           = NULL;
    struct passwd *pw         = NULL;
    struct group *unixGrp     = NULL;
    struct groupInfoEnt *uGrp = NULL;
    unsigned int len          = 0;

    if (admins == NULL) {
        ls_syslog (LOG_ERR, I18N_NULL_POINTER, filename, "admins");
        return NULL;
    }

    expandAds = malloc( MAX_LINE_LEN );
    if ( NULL == expandAds && ENOMEM == errno ) {
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, filename, "malloc", MAX_LINE_LEN);
        lsberrno = LSBE_NO_MEM;
        return NULL;
    }

    expandAds = "";
    len = MAX_LINE_LEN;
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
            ls_syslog (LOG_WARNING, "catgets 5390: %s: File %s at line %d: Unknown user or user group name <%s>; Maybe a windows user or of another domain.", __func__, filename, *lineNum, word); 
            
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

char *
putIntoList (char **list, unsigned int *len, const char *string, const char *listName)
{
    char *sp             = NULL;
    unsigned long length = *len;

    if ( NULL == string) {
        return *list;
    }
    if ( (char **) NULL == list ) {
        return NULL;
    }

    sp = putstr_ (listName);
    if (isInList (*list, string) == true) {
        /* catgets 5392 */
        ls_syslog (LOG_ERR, "catgets 5392: %s: %s is repeatedly specified %s; ignoring", __func__, string, sp);  
        FREEUP (sp);
        return *list;
    }

    if ( length <= strlen (*list) + strlen (string) + 2) {
        char *temp = myrealloc (*list, 2 * length);
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


int
setDefaultHost (struct lsInfo *info)
{
    size_t *override         = NULL;
    struct hostInfoEnt *host = NULL;

    *override = true;
    
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

        if (addHostEnt( host, &cConf->hosts[i], override) == false) {
            const char addHostEnt[] = "addHostEnt";
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, addHostEnt );
            return -1;
        }
    }
    return 0;
}

unsigned int
setDefaultUser (void)
{
    char *functionName = malloc( strlen(  __func__ ) + 1);

    strcpy( functionName, __func__ ); 

    if (handleUserMem ()) {
        return UINT_MAX;
    }

    if (!addUser( defaultLabel, INFINIT_INT, INFINIT_FLOAT, functionName, true)) { // FIXME FIXME FIXME the fuck is functionName present here?
        return 0;
    }

    assert( numofusers > 0 );
    uConf->users = malloc( numofusers * sizeof( struct userInfoEnt ) );
    if(  0 == numofusers || ( NULL == uConf->users  && ENOMEM == errno ) ) {
        //assert( numofusers >= 0 );
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", numofusers * sizeof (struct userInfoEnt));
        lsberrno = LSBE_CONF_FATAL;
        freeWorkUser (true);
        freeUConf (uConf, false);
        return -1;
    }

    for ( unsigned int i = 0; i < numofusers; i++) {
        initUserInfo (&uConf->users[i]);
        uConf->users[i] = *users[i];
    }

    uConf->numUsers = numofusers;
    return 0;
}

bool
handleUserMem (void)
{
    if (uConf == NULL) {
        uConf = malloc (sizeof (struct userConf));
        if (NULL == uConf && ENOMEM == errno ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", sizeof (struct userConf));
            lsberrno = LSBE_CONF_FATAL;
            return false;
        }
        resetUConf (uConf);
    }
    else {
        freeUConf (uConf, true);
    }
    freeWorkUser (false);
    for( unsigned int i = 0; i < MAX_GROUPS; i++) { // defined lsf/include/lsb/lsbatch.h // FIXME FIXME FIXME why 150? software slow after that? too much mem? find out relationship of #of chosts and mem usage; move to configure.ac
        usergroups[i] = NULL;
    }
    usersize = 0;
    
    return true;
}

int
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
        freeHConf (hConf, true);
    }

    freeWorkHost (false);
    for ( unsigned int i = 0; i < MAX_GROUPS; i++) {
        hostgroups[i] = NULL;
    }

    hostsize = 0;
    return 0;
}

int
parseSigActCmd (struct queueInfoEnt *qp, char *linep, const char *filename, size_t *lineNum, const char *section)
{
    
    char *actClass    = NULL;
    char *sigActCmd   = NULL;
    int actClassValue = 0;

    while (isspace (*linep)) {
        linep++;
    }

    while (linep != NULL && linep[0] != '\0') {
        if ((actClass = getNextWord1_ (&linep)) == NULL) {
            if (filename) {
                /* catgets 5408 */
                ls_syslog (LOG_ERR, "catgets 5408: %s:File %s %s at line %d: SUSPEND, RESUME or TERMINATE is missing", __func__, filename, section, *lineNum);
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
                ls_syslog (LOG_ERR, "catgets 5409: %s:File %s %s at line %d: wrong KEYWORD", __func__, filename, section, *lineNum);    /* catgets 5409 */
            }
            lsberrno = LSBE_CONF_WARNING;
            return -2;
        }

        while (isspace (*linep)) {
            linep++;
        }

        if (*linep != '[') {
            if (filename) {
                ls_syslog (LOG_ERR, "catgets 5410: %s:File %s %s at line %d: '[' is missing", __func__, filename, section, *lineNum);   /* catgets 5410 */
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
                ls_syslog (LOG_ERR, "catgets 5411: %s:File %s %s at line %d: ']' is missing", __func__, filename, section, *lineNum);
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
                    ls_syslog (LOG_ERR, "catgets 5412: %s: File %s %s at line %d: 'CHKPNT' is not valid in RESUME", __func__, filename, section, *lineNum);
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
terminateWhen (struct queueInfoEnt *qp, char *linep, const char *filename, size_t *lineNum, const char *section)
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
                    ls_syslog (LOG_ERR, "catgets 5413: %s:File %s %s at line %d: LOAD or WINDOW is missing", __func__, filename, section, *lineNum);
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

int
checkAllOthers ( const char *word, int *hasAllOthers)
{
    int returnCode        = 0;
    unsigned int numHosts = 0;
    char **grpHosts       = NULL;
   
    grpHosts = expandGrp (word, &numHosts, HOST_GRP);
    if (grpHosts == NULL && lsberrno == LSBE_NO_MEM) {
        return returnCode;
    }
 
    if (numHosts && !strcmp (grpHosts[0], "all")) { // FIXME FIXME FIXME label [0]
        if (*hasAllOthers == true) {
            returnCode = -1;
        }
        else {
            *hasAllOthers = true;
        }
    }
    
    freeSA (grpHosts, numHosts);
    return returnCode;
}

bool
getReserve ( const char *reserve, struct queueInfoEnt *qp, const char *filename, unsigned int lineNum)
{
    char *sp = NULL;
    char *cp = NULL;
    
    if ((sp = strstr (reserve, "MAX_RESERVE_TIME")) != NULL) {
        sp += strlen ("MAX_RESERVE_TIME");
        while (isspace (*sp)) {
            sp++;
        }

        if ( sp == NULL ) {
            ls_syslog (LOG_ERR, "catgets 5431: %s: File %s in section Queue ending at line %d: MAX_RESERVE_TIME is specified without period time; ignoring SLOT_RESERVE", __func__, filename, lineNum);  /* catgets 5431 */
            lsberrno = LSBE_CONF_WARNING;
            return false;
        }
        if (*sp != '[') {
            ls_syslog (LOG_ERR, "catgets 5432: %s: File %s in section Queue ending at line %d: MAX_RESERVE_TIME <%s> is specified without '['; ignoring SLOT_RESERVE", __func__, filename, lineNum, reserve);    /* catgets 5432 */
            lsberrno = LSBE_CONF_WARNING;
            return false;
        }
        cp = ++sp;
        while (*sp != ']' && *sp != '\0' && isdigit (*sp) && *sp != ' ') {
            sp++;
        }
        if (*sp == '\0' || (*sp != ']' && *sp != ' ')) {
            ls_syslog (LOG_ERR, "catgets 5433: %s: File %s in section Queue ending at line %d: MAX_RESERVE_TIME is specified without ']'; ignoring SLOT_RESERVE", __func__, filename, lineNum);  /* catgets 5433 */
            lsberrno = LSBE_CONF_WARNING;
            return false;
        }
        if (*sp == ' ') {
            while (*sp == ' ') {
                sp++;
            }
            if (*sp != ']') {
                ls_syslog (LOG_ERR, "catgets 5434: %s: File %s in section Queue ending at line %d: MAX_RESERV_TIME is specified without ']';ignoring SLOT_RESERVE", __func__, filename, lineNum);    /* catgets 5434 */
                lsberrno = LSBE_CONF_WARNING;
                return false;
            }
        }
        sp = NULL;
        qp->slotHoldTime = my_atoi (cp, INFINIT_INT, 0);
        if (qp->slotHoldTime == INFINIT_INT) {
            ls_syslog (LOG_ERR, "catgets 5435: %s: File %s in section Queue ending at line %d: Value <%s> of MAX_RESERV_TIME for queue <%s> isn't an integer between 1 and %d; ignored.", __func__, filename, lineNum, cp, qp->queue, INFINIT_INT);  /* catgets 5435 */
            lsberrno = LSBE_CONF_WARNING;
            *sp = ']';
            return false;
        }
        *sp = ']';
    }
    if (strstr (reserve, "BACKFILL") != NULL) {
        if (qp->slotHoldTime == INFINIT_INT) {
            ls_syslog (LOG_ERR, "catgets 5436: %s: File %s in section Queue ending at line %d: BACKFILL is specified without MAX_RESERV_TIME for SLOT_RESERVE; ignoring", __func__, filename, lineNum);  /* catgets 5436 */
            lsberrno = LSBE_CONF_WARNING;
            return false;
        }
        qp->qAttrib |= QUEUE_ATTRIB_BACKFILL;
    }

    return true;
}

bool
isServerHost ( const char *hostName)
{
    if( hostName == NULL ) {
        return false;
    }

    for ( unsigned int i = 0; i < cConf->numHosts; i++ ) {
        if (equalHost_ (cConf->hosts[i].hostName, hostName) && cConf->hosts[i].isServer == true) {
            return true;
        }
    }

    return false;
}

struct group *
mygetgrnam (const char *name)
{
    int first_entry   = 1;
    int num           = 0;
    int total_entries = 0;
    // int count         = 0;
    struct group *grentry    = NULL;
    struct group *grretentry = NULL;
    struct mygroup {
        struct group gr_entry;
        struct mygroup *gr_next;
    } *mygrhead = NULL, *mygrentry = NULL, *tmpgrentry = NULL;
    
    
    freeUnixGrp (grretentry);
    grretentry = NULL;
    
    
    setgrent ();
    
    while ((grentry = getgrent ()) != NULL) {
        
        if (strcmp (name, grentry->gr_name) == 0) {
            
            mygrentry = malloc (sizeof (struct mygroup));
            
            if (!mygrentry) {
                // goto errorCleanup;
                while (mygrhead) {
                    mygrentry = mygrhead->gr_next;
                    FREEUP (mygrhead->gr_entry.gr_name);
                    FREEUP (mygrhead->gr_entry.gr_passwd);
                    for ( unsigned int count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
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
            
            mygrentry->gr_entry.gr_mem = calloc (sizeof (char *), num + 1 );
            if ( NULL == mygrentry->gr_entry.gr_mem && ENOMEM == errno ) {
                // goto errorCleanup;
                while (mygrhead) {
                    mygrentry = mygrhead->gr_next;
                    FREEUP (mygrhead->gr_entry.gr_name);
                    FREEUP (mygrhead->gr_entry.gr_passwd);
                    for (unsigned int count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
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
            
            for (unsigned int count = 0; grentry->gr_mem[count]; count++) {
                mygrentry->gr_entry.gr_mem[count] = putstr_ (grentry->gr_mem[count]);
            }
            
            mygrentry->gr_next = NULL;
            tmpgrentry = mygrentry;
            mygrentry = mygrentry->gr_next;
        }
    }

    endgrent ();
    if (!mygrhead) {
        // goto errorCleanup;
        while (mygrhead) {
            mygrentry = mygrhead->gr_next;
            FREEUP (mygrhead->gr_entry.gr_name);
            FREEUP (mygrhead->gr_entry.gr_passwd);
            for (unsigned int count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
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

    mygrentry = mygrhead;
    while (mygrentry) {
        for (num = 0; mygrentry->gr_entry.gr_mem[num]; num++) {
            ;
        }
        total_entries += num;
        mygrentry = mygrentry->gr_next;
    }
    
    
    if (!(grretentry = malloc (sizeof (struct group)))) {
        // goto errorCleanup;
        while (mygrhead) {
            mygrentry = mygrhead->gr_next;
            FREEUP (mygrhead->gr_entry.gr_name);
            FREEUP (mygrhead->gr_entry.gr_passwd);
            for( unsigned int count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
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
    
    // assert( total_entries + 1 >= 0 );
    grretentry->gr_mem = calloc( sizeof (char *), total_entries + 1 );
    if ( NULL == grretentry->gr_mem && ENOMEM == errno ) {
        // goto errorCleanup;
        while (mygrhead) {
            mygrentry = mygrhead->gr_next;
            FREEUP (mygrhead->gr_entry.gr_name);
            FREEUP (mygrhead->gr_entry.gr_passwd);
            for( unsigned int count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
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
    
    
    grretentry->gr_name   = putstr_ (mygrhead->gr_entry.gr_name);
    grretentry->gr_passwd = putstr_ (mygrhead->gr_entry.gr_passwd);
    grretentry->gr_gid    = mygrhead->gr_entry.gr_gid;
    
    
    mygrentry = mygrhead;
    // count = 0;
    { // cheating ... kinda
        unsigned int count = 0;
        unsigned int maxcount = 0;

        while (mygrentry) {
            for (unsigned int num = 0; mygrentry->gr_entry.gr_mem[num]; num++) {

                grretentry->gr_mem[count] = putstr_ (mygrentry->gr_entry.gr_mem[num]);

                if (!grretentry->gr_mem[count]) {
                    // goto errorCleanup;
                    while (mygrhead) {
                        mygrentry = mygrhead->gr_next;
                        FREEUP (mygrhead->gr_entry.gr_name);
                        FREEUP (mygrhead->gr_entry.gr_passwd);
                        for (unsigned int count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
                            FREEUP (mygrhead->gr_entry.gr_mem[count]);
                            maxcount = count;
                        }
                        count = maxcount; // FIXME FIXME FIXME see code around herre, don't think it is corrrect
                        FREEUP (mygrhead->gr_entry.gr_mem);
                        FREEUP (mygrhead);
                        mygrhead = mygrentry;
                    }
                    
                    
                    freeUnixGrp (grretentry);
                    grretentry = NULL;
                    
                    return NULL;
                }
            }
            mygrentry = mygrentry->gr_next;
        }
        
        grretentry->gr_mem[count] = NULL;
    }

    while (mygrhead) {

        mygrentry = mygrhead->gr_next;
        FREEUP (mygrhead->gr_entry.gr_name);
        FREEUP (mygrhead->gr_entry.gr_passwd);
        for ( unsigned int count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
            FREEUP (mygrhead->gr_entry.gr_mem[count]);
        }
        FREEUP (mygrhead->gr_entry.gr_mem);
        FREEUP (mygrhead);
        mygrhead = mygrentry;
    }
    
    
    mygrhead = mygrentry = tmpgrentry = NULL;
    
    
    return grretentry;
    
// errorCleanup: // FIXME FIXME FIXME FIXME remove goto label
    // while (mygrhead) {
    //  mygrentry = mygrhead->gr_next;
    //  FREEUP (mygrhead->gr_entry.gr_name);
    //  FREEUP (mygrhead->gr_entry.gr_passwd);
    //  for (count = 0; mygrhead->gr_entry.gr_mem[count]; count++) {
    //      FREEUP (mygrhead->gr_entry.gr_mem[count]);
    //  }
    //  FREEUP (mygrhead->gr_entry.gr_mem);
    //  FREEUP (mygrhead);
    //  mygrhead = mygrentry;
    // }
    
    
    // freeUnixGrp (grretentry);
    // grretentry = NULL;
    
    // return NULL;
}


void
freeUnixGrp (struct group *unixGrp)
{
    if (unixGrp)
        {
        FREEUP (unixGrp->gr_name);
        FREEUP (unixGrp->gr_passwd);
        for ( unsigned int count = 0; unixGrp->gr_mem[count]; count++) {
            FREEUP (unixGrp->gr_mem[count]);
        }
        FREEUP (unixGrp->gr_mem);
        FREEUP (unixGrp);
        unixGrp = NULL;
    }
    
    return;
}



struct group *
copyUnixGrp (struct group *unixGrp)
{
    int count                 = 0;
    struct group *unixGrpCopy = NULL;
        
    unixGrpCopy = malloc( sizeof (struct group) );
    if ( NULL == unixGrpCopy && ENOMEM == errno ) {
        // goto errorCleanup;
        freeUnixGrp (unixGrpCopy);
        unixGrpCopy = NULL;
        return NULL;

    }
    
    unixGrpCopy->gr_name = putstr_ (unixGrp->gr_name);
    unixGrpCopy->gr_passwd = putstr_ (unixGrp->gr_passwd);
    unixGrpCopy->gr_gid = unixGrp->gr_gid;
    
    for (count = 0; unixGrp->gr_mem[count]; count++){
        ;
    }
    
    // assert( count + 1 >= 0 );
    unixGrpCopy->gr_mem = calloc( sizeof (char *), (count + 1) );
    if ( NULL == unixGrpCopy->gr_mem && ENOMEM == errno ) {
        // goto errorCleanup;
        freeUnixGrp (unixGrpCopy);
        unixGrpCopy = NULL;
        return NULL;
    }
    
    for (count = 0; unixGrp->gr_mem[count]; count++) {
        unixGrpCopy->gr_mem[count] = putstr_ (unixGrp->gr_mem[count]);
    }
    unixGrpCopy->gr_mem[count] = NULL;
    
    return unixGrpCopy;
    
// errorCleanup: // FIXME FIXME FIXME remove goto label
//     freeUnixGrp (unixGrpCopy);
//     unixGrpCopy = NULL;

//     return NULL;
}

void
addBinaryAttributes ( const char *confFile, size_t *lineNum, struct queueInfoEnt *queue, struct keymap *keylist, unsigned int attrib, const char *attribName)
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

    return;
}

int
resolveBatchNegHosts ( const char *inHosts, char **outHosts, int isQueue)
{
    struct inNames **inTable = NULL;
    char **outTable  = NULL;
    char *buffer     = strdup (inHosts);
    char *save_buf   = buffer;
    char *word       = NULL;
    char *ptr_level  = NULL;
    int isAll        = false;
    int result       = 0;
    unsigned int neg_num      = 0;
    unsigned int in_num       = 0;
    unsigned int outTableSize = 0;
    unsigned int inTableSize  = 0;
    size_t size = 0;

    
    assert( isAll );
    assert( outTableSize );
    inTable = calloc ( cConf->numHosts, sizeof (struct inNames *));
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
        fprintf( stderr, "we done goodfedup in %s\n", __func__); // FIXME FIXME FIXME FIXME ?
    }

    // assert( cConf->numHosts >= 0 );
    outTable = calloc(  cConf->numHosts, sizeof (char *));
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
        FREEUP (outHosts[0]); // FIXME FIXME FIXME label [0]
        FREEUP (save_buf);
        FREEUP (ptr_level);

        return result;
    }
    else {
        outTableSize = cConf->numHosts;
        fprintf( stderr, "we done goofedup again in %s\n", __func__ );
    }
    
    if (!buffer || !inTable || !outTable) {
        if (result > -2) { // FIXME FIXME FIXME change to non-negative return value
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
            result = -1; // FIXME FIXME FIXME change to non-negative return value
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
        if (word[0] == '~') { // FIXME FIXME FIXME label [0]
            if (word[1] == '\0') { // FIXME FIXME FIXME label [1]
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
            
            if (isHostName (word) == false) {
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
                
                if (    ( strcmp( word, grpMembers[0]) == 0 )
                     && ( strcmp( word, "all")         != 0 )
                     && ( strcmp( word, "others")      != 0 )
                     && ( strcmp( word, "none")        != 0 )
                    )
                {
                    
                    word--;
                    freeSA (grpMembers, num);
                    /* catgets 5905 */
                    ls_syslog (LOG_ERR, "catgets 5905: %s: host/group name \"%s\" is ignored.", __func__, word);
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

            if (isQueue == true) {
                ptr_level = strchr (word, '+');
            }
            
            if (ptr_level) {
                ptr_level[0] = 0; // FIXME FIXME FIXME label [0]
                ptr_level++;
                
                if (strlen (ptr_level) && !isdigit (ptr_level[0])) { // FIXME FIXME FIXME label [0]
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

                isAll = true;
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

                if (expandWordAll (&size, &in_num, inTable, ptr_level) == false) {
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
            else if (isHostName (word) == false) {
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
                
                if (!strcmp (grpMembers[0], "all")) { // FIXME FIXME FIXME label [0]

                    unsigned int miniTableSize = 0;
                    isAll = true;
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
                    if (expandWordAll (&size, &in_num, inTable, ptr_level) == false) {
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
                        size_t cur_size = fillCell_ (&inTable[in_num], grpMembers[j], ptr_level);
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
                            // inTable =  realloc (inTable, (unsigned long)(cConf->numHosts + in_num) * sizeof (struct inNames *));
                            inTable =  realloc (inTable, (cConf->numHosts + in_num) * sizeof (struct inNames *));
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
                size_t cur_size = fillCell_ (&inTable[in_num], word, ptr_level);
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
                    inTable = realloc (inTable, (cConf->numHosts +  in_num) * sizeof (struct inNames *));
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
                size -= strlen (inTable[k]->name) + 1;
                FREEUP (inTable[k]->name);
                FREEUP (inTable[k]);
                result++;
            }
            else if ((nameLen > 1) && (inTable[k]->name[nameLen - 1] == '!')
                     && (!inTable[k]->prf_level)
                     && (isHostName (inTable[k]->name) == false))
            {
                
                inTable[k]->name[nameLen - 1] = '\0';
                if (equalHost_ (inTable[k]->name, outTable[j])) {
                    size -= strlen (inTable[k]->name) + 1;
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
        if (neg_num == 0) { // FIXME FIXME FIXME FIXME what is outHosts[0]? turn into enum
            FREEUP (outTable);
        }
        FREEUP (outHosts[0]); // FIXME FIXME FIXME FIXME what is outHosts[0]? turn into enum
        FREEUP (save_buf);
        FREEUP (ptr_level);

        return result;

    }

    outHosts[0] = malloc ( size + in_num ); // FIXME FIXME FIXME FIXME what is outHosts[0]? turn into enum
    if ( NULL == outHosts[0] && ENOMEM == errno ) {  // FIXME FIXME FIXME FIXME what is outHosts[0]? turn into enum
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
    outHosts[0][0] = 0; // FIXME FIXME FIXME figure out and label [0][0]
    
    
    if (!result) {
        buffer = save_buf;
        while ((word = getNextWord_ (&buffer)) != NULL) {
            if (word[0] != '~') { // FIXME FIXME FIXME figure out and label [0]
                strcat (outHosts[0], word); // FIXME FIXME FIXME figure out and label [0]
                strcat (outHosts[0], " "); // FIXME FIXME FIXME figure out and label [0]
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
                
                strcat (outHosts[0], (const char *) inTable[j]->name); // FIXME FIXME FIXME figure out and label [0]
                FREEUP (inTable[j]->name);
                FREEUP (inTable[j]);
                strcat (outHosts[0], " "); // FIXME FIXME FIXME figure out and label [0]
                k++;
            }
        }
    }

    if (outHosts[0][0]) { // FIXME FIXME FIXME figure out and label [0][0]
        outHosts[0][strlen (outHosts[0]) - 1] = '\0'; // FIXME FIXME FIXME figure out and label [0]
    }

    free (inTable);
    free (outTable);
    free (save_buf);

    return result;
}


bool
checkJobAttaDir ( const char *path)
{
    struct stat statBuf;
    unsigned long length = strlen (path);

    if (checkSpoolDir (path) == -1) {
        return false;
    }

    if (length > 0 && (path[length - 1] == '/' || path[length - 1] == '\\')) { // FIXME FIXME FIXME this code might turn out to be useless, once all string termination issues have been resolved.
        // path[length - 1] = '\0';
        assert( path[length - 1] == '\0' );
    }

    if ( 0 == stat (path, &statBuf) && S_ISDIR (statBuf.st_mode) ) {
            return true;
    }

    return false;
}



int
parseDefAndMaxLimits (struct keymap *key, unsigned int *defaultVal, unsigned int *maxVal, const char *filename, size_t *lineNum, const char *pname)
{
    
    const char *sp     = strdup(key->val);
    char *defaultLimit = NULL;
    char *maxLimit     = NULL;
    
    if (sp != NULL) {
        defaultLimit = putstr_ (getNextWord_ (&sp));
        maxLimit = getNextWord_ (&sp);
    }
    
    if (maxLimit != NULL) {

        *defaultVal = my_atoi (defaultLimit, INFINIT_INT, 0);
        if (INFINIT_INT == *defaultVal ) {
            /* catgets 5387 */
            ls_syslog (LOG_ERR, "catgets 5387: [%s] %s: File %s in section Queue at line %d: Default value <%s> of %s isn't a positive integer between 0 and %d; ignored.", __func__, pname, filename, *lineNum, defaultLimit, key->key, INFINIT_INT);
            lsberrno = LSBE_CONF_WARNING;
        }
        
    }
    else if (!maxLimit && defaultLimit) {
        maxLimit = defaultLimit;
    }
    
    if ((*maxVal = my_atoi (maxLimit, INFINIT_INT, 0)) == INFINIT_INT) {
        /* catgets 5388 */
        ls_syslog (LOG_ERR, "catgets 5388: [%s] %s: File %s in section Queue at line %d: Maximum value <%s> of %s isn't a positive integer between 0 and %d; ignored.", __func__, pname, filename, *lineNum, maxLimit, key->key, INFINIT_INT);
        lsberrno = LSBE_CONF_WARNING;
    }
    
    FREEUP (defaultLimit);
    
    if ((*defaultVal != INFINIT_INT) && (*maxVal != INFINIT_INT) && (*defaultVal > *maxVal)) {
        /* catgets 5112 */
        ls_syslog (LOG_ERR, "catgets 5112: [%s] %s: File %s in section Queue at line %d: The default %s %d should not be greater than the max %d; ignoring the default and using max value also as default.", __func__, pname, filename, *lineNum, key->key, *defaultVal, *maxVal );
        *defaultVal = *maxVal;
    }

    return 0; // FIXME FIXME FIXME no error condition on this function
}


bool
parseQFirstHost ( const char *myWord, bool *haveFirst, const char *pname, size_t *lineNum, const char *filename, const char *section)
{
    bool needCheck = true;
    struct groupInfoEnt *gp = NULL;
    
    if (chkFirstHost (myWord, &needCheck)) {
        
        if (*haveFirst) {
            /* catgets 5439 */
            ls_syslog (LOG_ERR, "catgets 5439: [%s] %s: File %s%s ending at line %d : Multiple first execution hosts specified:<%s>; ignored.", __func__, pname, filename, section, *lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return true;
        }

        if (needCheck) {
            if (!strcmp (myWord, "others")) {
                /* catgets 5900 */
                ls_syslog (LOG_ERR, "catgets 5900: [%s] %s: File %s%s ending at line %d : \"others\" specified as first execution host; ignored.", __func__, pname, filename, section, *lineNum);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return true;
            }
            if (!strcmp (myWord, "all")) {
                /* catgets 5901 */
                ls_syslog (LOG_ERR, "catgets 5901: [%s] %s: File %s%s ending at line %d : \"all\" specified as first execution host; ignored.", __func__, pname, filename, section, *lineNum);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return true;
            }
            
            gp = getHGrpData (myWord);
            if (gp != NULL) {
                /* catgets 5902 */
                ls_syslog (LOG_ERR, "catgets 5902: [%s] %s: File %s%s ending at line %d : host group <%s> specified as first execution host; ignored.",__func__, pname, filename, section, *lineNum, myWord);
                lsberrno = LSBE_CONF_WARNING;
                FREEUP (myWord);
                return true;
            }
            /* catgets 5904 */
            ls_syslog (LOG_ERR, "catgets 5904: [%s] %s: File %s%s ending at line %d : Invalid first execution host <%s>, not a valid host name; ignored.", __func__, pname, filename, section, *lineNum, myWord);
            lsberrno = LSBE_CONF_WARNING;
            FREEUP (myWord);
            return true;
        }
        *haveFirst = true;
    }

    return false;
}

bool
chkFirstHost ( const char *host, bool *needChk)
{
// #define FIRST_HOST_TOKEN '!'
    char FIRST_HOST_TOKEN = '!';
    unsigned long length = strlen (host);
    
    assert( length > 0 );
    if (host[length - 1] == FIRST_HOST_TOKEN) {
        
        if (isHostName (host) && getHostData (host)) {
            return false;
        }
        else {
            assert( host[length - 1] == '\0' );
            if (isHostName (host) && getHostData (host)) {
                *needChk = false;
            }
            return true;
        }
    }
    else {
        return false;
    }
}

void
updateClusterConf (struct clusterConf *clusterConf) // FIXME FIXME FIXME fix mutator function here
{
    if( clusterConf ) {
        cConf = clusterConf;
    }

    return;
}