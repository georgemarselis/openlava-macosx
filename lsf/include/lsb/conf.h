/* $Id: lsb.conf.h 397 2007-11-26 19:04:00Z mblack $
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

#pragma once

#include "lib/conf.h"
#include "lib/table.h"
#include "lsb/lsbatch.h"

#define TYPE1  RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL
#define TYPE2  RESF_BUILTIN | RESF_GLOBAL

#define USEREQUIVALENT "userequivalent"
#define USERMAPPING    "usermap"

#define LSB_NULL_POINTER_STR     "%s: %s is Null"                           /* catgets 5050 */
#define LSB_FILE_PREMATUR_STR    "%s: File %s at line %d, premature EOF"    /* catgets 5051 */
#define LSB_EMPTY_SECTION_STR    "%s: File %s at line %d: Empty %s section" /* catgets 5052 */
#define LSB_IN_QUEUE_ADMIN_STR   "in queue's administrator list"            /* catgets 5053 */
#define LSB_IN_QUEUE_HOST_STR    "in queue's HOSTS list"                    /* catgets 5054 */
#define LSB_IN_SEND_RCV_STR      "in queue's SEND_TO or RCV_FROM list"      /* catgets 5055 */

#define I18N_NULL_POINTER      (_i18n_msg_get(ls_catd, NL_SETN, 5050, LSB_NULL_POINTER_STR))
#define I18N_FILE_PREMATURE    (_i18n_msg_get(ls_catd, NL_SETN, 5051, LSB_FILE_PREMATUR_STR))
#define I18N_EMPTY_SECTION     (_i18n_msg_get(ls_catd, NL_SETN, 5052, LSB_EMPTY_SECTION_STR))
#define I18N_IN_QUEUE_ADMIN    (_i18n_msg_get(ls_catd, NL_SETN, 5053, LSB_IN_QUEUE_ADMIN_STR))
#define I18N_IN_QUEUE_HOST     (_i18n_msg_get(ls_catd, NL_SETN, 5054, LSB_IN_QUEUE_HOST_STR))
#define I18N_IN_QUEUE_SEND_RCV (_i18n_msg_get(ls_catd, NL_SETN, 5055, LSB_IN_SEND_RCV_STR))
#define MAX_SELECTED_LOADS 4

 struct inNames {
    char *name;
    char *prf_level;
};

static struct paramConf *pConf      = NULL;
static struct userConf *uConf       = NULL;
static struct hostConf *hConf       = NULL;
static struct queueConf *qConf      = NULL;
static struct userInfoEnt **users   = NULL;
static struct hostInfoEnt **hosts   = NULL;
static struct queueInfoEnt **queues = NULL;

static struct groupInfoEnt *usergroups[MAX_GROUPS];
static struct groupInfoEnt *hostgroups[MAX_GROUPS];

static struct hTab unknownUsers = { };
// static struct clusterConf cConf_;
// static struct sharedConf sConf_;

static unsigned int numofugroups = 0;
static unsigned int numofhosts   = 0;
static unsigned int numofhgroups = 0;
static unsigned int numofqueues  = 0;
static unsigned int usersize     = 0;
static unsigned int hostsize     = 0;
static unsigned int queuesize    = 0;
static unsigned int numofusers   = 0;
static double maxFactor  = 0.0;
static char *maxHName    = NULL;

static bool_t initUnknownUsers = FALSE;


char *getNextWord1_ (char **);

#define TYPE1  RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL
#define TYPE2  RESF_BUILTIN | RESF_GLOBAL

static char do_Param  (struct lsConf *conf, char *fname, size_t *lineNumber);
static char do_Users  (struct lsConf *conf, char *fname, size_t *lineNumber, int option);
static char do_Hosts_ (struct lsConf *conf, char *fname, size_t *lineNumber, struct lsInfo *info, int options);
static char do_Queues (struct lsConf *conf, char *fname, size_t *lineNumber, struct lsInfo *info, int options);
static char do_Groups (struct groupInfoEnt **groups, struct lsConf *conf, char *fname, size_t *lineNumber, unsigned int *ngroups, int options);
static int  isHostName (char *grpName);
static int  addHostEnt (struct hostInfoEnt *hp, struct hostInfo *hostInfo, size_t *override);
static char addQueue  (struct queueInfoEnt *qp, char *fname, unsigned int lineNum);
static char addUser   (char *, int, float, char *, int);
static char addMember (struct groupInfoEnt *gp, char *word, int grouptype, char *fname, unsigned int lineNumber, char *section, int options, int checkAll);

static char isInGrp (char *, struct groupInfoEnt *, int, int);
static char **expandGrp (char *, unsigned int *, int);
static struct groupInfoEnt *addGroup (struct groupInfoEnt **, char *, unsigned int *, int);

static struct groupInfoEnt *addUnixGrp (struct group *, char *, char *, unsigned int, char *, int);
static char *parseGroups (char *linep, char *fname, size_t *lineNumber, char *section, int groupType, int options);

static struct groupInfoEnt *getUGrpData (char *);
static struct groupInfoEnt *getHGrpData (char *);
static struct groupInfoEnt *getGrpData (struct groupInfoEnt **groups, char *name, unsigned int num);
static struct userInfoEnt  *getUserData (char *);
static struct hostInfoEnt  *getHostData (char *);
static struct queueInfoEnt *getQueueData (char *);

static void initParameterInfo (struct parameterInfo *);
static void freeParameterInfo (struct parameterInfo *);
static void initUserInfo  (struct userInfoEnt *);
static void freeUserInfo  (struct userInfoEnt *);
static void initGroupInfo (struct groupInfoEnt *);
static void freeGroupInfo (struct groupInfoEnt *);
static void initHostInfoEnt (struct hostInfoEnt *);
static void freeHostInfoEnt  (struct hostInfoEnt *);
static void initQueueInfo (struct queueInfoEnt *);
static void freeQueueInfo (struct queueInfoEnt *);


static void initThresholds (struct lsInfo *, float *, float *);
static void getThresh (struct lsInfo *info, struct keymap *keyList, float *loadSched, float *loadStop, char *fname, size_t *lineNumber, char *description);

static char searchAll (char *);

static void checkCpuLimit (char **hostSpec, double **cpuFactor, int useSysDefault, char *fname, size_t *lineNumber, char *pname, struct lsInfo *info, int options);
static int parseDefAndMaxLimits (struct keymap key, int *defaultVal, int *maxVal, char *fname, size_t *lineNumber, char *pname);
static int parseCpuAndRunLimit (struct keymap *keylist, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname, struct lsInfo *info, int options);
static int parseProcLimit (char *word, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname);
static int parseLimitAndSpec (char *word, int *limit, char **spec, char *hostSpec, char *param, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname);
static int my_atoi (char *, int, int);
static float my_atof (char *, float, float);
static void resetUConf (struct userConf *);
static int copyHostInfo (struct hostInfoEnt *, struct hostInfoEnt *);
static void resetHConf (struct hostConf *hConf);
static double *getModelFactor (char *, struct lsInfo *);
static double *getHostFactor (char *);
static char *parseAdmins (char *admins, int options, char *fname, size_t *lineNum);
static char *putIntoList (char **list, unsigned int *len, char *string, char *listName);
static int isInList (char *, char *);
static int setDefaultHost (struct lsInfo *);
static int handleUserMem (void);
static int setDefaultUser (void);
static int handleHostMem (void);
static void freeSA (char **list, unsigned int num);
static int checkAllOthers (char *, int *);
static int resolveBatchNegHosts (char *, char **, int);
static unsigned int fillCell_ (struct inNames **table, char *name, char *level);
static unsigned int expandWordAll (unsigned int *size, unsigned int *num, struct inNames **inTable, char *ptr_level);
static int readHvalues_conf (struct keymap *keyList, char *linep, struct lsConf *conf, char *lsfile, size_t *lineNumber, int exact, char *section);
static char *a_getNextWord_ (char **);
static int getReserve (char *reserve, struct queueInfoEnt *qp, char *filename, unsigned int lineNum);
static int isServerHost (char *);

static void addBinaryAttributes (char *confFile, size_t *lineNumber, struct queueInfoEnt *queue, struct keymap *keylist, unsigned int attrib, char *attribName);
static int parseQFirstHost (char *myWord, int *haveFirst, char *pname, size_t *lineNumber, char *fname, char *section);

void freeUConf (struct userConf *, int);
void freeHConf (struct hostConf *, int);
void freeQConf (struct queueConf *, int);
void freeWorkUser (int);
void freeWorkHost (int);
void freeWorkQueue (int);
int chkFirstHost (char *, int *);
int checkSpoolDir (char *spoolDir);
int checkJobAttaDir (char *);


int sigNameToValue_ (char *);
int parseSigActCmd (struct queueInfoEnt *queue, char *val, char *fname, size_t *lineNumber, char *section);
int terminateWhen (struct queueInfoEnt *qp, char *linep, char *fname, size_t *lineNumber, char *section);
char checkRequeEValues (struct queueInfoEnt *qp, char *word, char *fname, size_t *lineNum);

