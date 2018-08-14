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

// #define TYPE1  RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL
// #define TYPE2  RESF_BUILTIN | RESF_GLOBAL

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

struct paramConf *pConf      = NULL;
struct userConf *uConf       = NULL;
struct hostConf *hConf       = NULL;
struct queueConf *qConf      = NULL;
struct userInfoEnt **users   = NULL;
struct hostInfoEnt **hosts   = NULL;
struct queueInfoEnt **queues = NULL;

struct groupInfoEnt *usergroups[MAX_GROUPS];
struct groupInfoEnt *hostgroups[MAX_GROUPS];

struct hTab unknownUsers;
// static struct clusterConf cConf_;
// static struct sharedConf sConf_;

unsigned int numofugroups = 0;
unsigned int numofhosts   = 0;
unsigned int numofhgroups = 0;
unsigned int numofqueues  = 0;
unsigned int usersize     = 0;
unsigned int hostsize     = 0;
unsigned int queuesize    = 0;
unsigned int numofusers   = 0;
double maxFactor  = 0.0;
char *maxHName    = NULL;

bool_t initUnknownUsers = FALSE;


char *getNextWord1_ (char **);

char do_Param  (struct lsConf *conf, char *fname, size_t *lineNumber);
char do_Users  (struct lsConf *conf, char *fname, size_t *lineNumber, int option);
char do_Hosts_ (struct lsConf *conf, char *fname, size_t *lineNumber, struct lsInfo *info, int options);
char do_Queues (struct lsConf *conf, char *fname, size_t *lineNumber, struct lsInfo *info, int options);
char do_Groups (struct groupInfoEnt **groups, struct lsConf *conf, char *fname, size_t *lineNumber, unsigned int *ngroups, int options);
int  isHostName (char *grpName);
int  addHostEnt (struct hostInfoEnt *hp, struct hostInfo *hostInfo, size_t *override);
char addQueue  (struct queueInfoEnt *qp, char *fname, unsigned int lineNum);
char addUser   (char *, int, float, char *, int);
char addMember (struct groupInfoEnt *gp, char *word, int grouptype, char *fname, unsigned int lineNumber, char *section, int options, int checkAll);

char isInGrp (char *, struct groupInfoEnt *, int, int);
char **expandGrp (char *, unsigned int *, int);
struct groupInfoEnt *addGroup (struct groupInfoEnt **, char *, unsigned int *, int);

struct groupInfoEnt *addUnixGrp (struct group *, char *, char *, unsigned int, char *, int);
char *parseGroups (char *linep, char *fname, size_t *lineNumber, char *section, int groupType, int options);

struct groupInfoEnt *getUGrpData (char *);
struct groupInfoEnt *getHGrpData (char *);
struct groupInfoEnt *getGrpData (struct groupInfoEnt **groups, char *name, unsigned int num);
struct userInfoEnt  *getUserData (char *);
struct hostInfoEnt  *getHostData (char *);
struct queueInfoEnt *getQueueData (char *);

void initParameterInfo (struct parameterInfo *);
void freeParameterInfo (struct parameterInfo *);
void initUserInfo  (struct userInfoEnt *);
void freeUserInfo  (struct userInfoEnt *);
void initGroupInfo (struct groupInfoEnt *);
void freeGroupInfo (struct groupInfoEnt *);
void initHostInfoEnt (struct hostInfoEnt *);
void freeHostInfoEnt  (struct hostInfoEnt *);
void initQueueInfo (struct queueInfoEnt *);
void freeQueueInfo (struct queueInfoEnt *);


void initThresholds (struct lsInfo *, float *, float *);
void getThresh (struct lsInfo *info, struct keymap *keyList, float *loadSched, float *loadStop, char *fname, size_t *lineNumber, char *description);

char searchAll (char *);

void checkCpuLimit (char **hostSpec, double **cpuFactor, int useSysDefault, char *fname, size_t *lineNumber, char *pname, struct lsInfo *info, int options);
int parseDefAndMaxLimits (struct keymap key, int *defaultVal, int *maxVal, char *fname, size_t *lineNumber, char *pname);
int parseCpuAndRunLimit (struct keymap *keylist, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname, struct lsInfo *info, int options);
int parseProcLimit (char *word, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname);
int parseLimitAndSpec (char *word, int *limit, char **spec, char *hostSpec, char *param, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname);
int my_atoi (char *, int, int);
float my_atof (char *, float, float);
void resetUConf (struct userConf *);
int copyHostInfo (struct hostInfoEnt *, struct hostInfoEnt *);
void resetHConf (struct hostConf *hConf);
double *getModelFactor (char *, struct lsInfo *);
double *getHostFactor (char *);
char *parseAdmins (char *admins, int options, char *fname, size_t *lineNum);
char *putIntoList (char **list, unsigned int *len, char *string, char *listName);
int isInList (char *, char *);
int setDefaultHost (struct lsInfo *);
int handleUserMem (void);
int setDefaultUser (void);
int handleHostMem (void);
void freeSA (char **list, unsigned int num);
int checkAllOthers (char *, int *);
int resolveBatchNegHosts (char *, char **, int);
unsigned int fillCell_ (struct inNames **table, char *name, char *level);
unsigned int expandWordAll (unsigned int *size, unsigned int *num, struct inNames **inTable, char *ptr_level);
int readHvalues_conf (struct keymap *keyList, char *linep, struct lsConf *conf, char *lsfile, size_t *lineNumber, int exact, char *section);
char *a_getNextWord_ (char **);
int getReserve (char *reserve, struct queueInfoEnt *qp, char *filename, unsigned int lineNum);
int isServerHost (char *);

void addBinaryAttributes (char *confFile, size_t *lineNumber, struct queueInfoEnt *queue, struct keymap *keylist, unsigned int attrib, char *attribName);
int parseQFirstHost (char *myWord, int *haveFirst, char *pname, size_t *lineNumber, char *fname, char *section);

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

