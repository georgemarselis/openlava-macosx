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
 
#ifndef LSF_LSB_CONF_H
#define LSF_LSB_CONF_H

#define TYPE1  RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL
#define TYPE2  RESF_BUILTIN | RESF_GLOBAL

#define USEREQUIVALENT "userequivalent"
#define USERMAPPING    "usermap"

#define LSB_NULL_POINTER_STR     "%s: %s is Null"                           /* catgets 5050 */
#define LSB_FILE_PREMATUR_STR    "%s: File %s at line %d, premature EOF"	/* catgets 5051 */
#define LSB_EMPTY_SECTION_STR    "%s: File %s at line %d: Empty %s section"	/* catgets 5052 */
#define LSB_IN_QUEUE_ADMIN_STR   "in queue's administrator list"	        /* catgets 5053 */
#define LSB_IN_QUEUE_HOST_STR    "in queue's HOSTS list"	                /* catgets 5054 */
#define LSB_IN_SEND_RCV_STR      "in queue's SEND_TO or RCV_FROM list"	    /* catgets 5055 */

#define I18N_NULL_POINTER      (_i18n_msg_get(ls_catd, NL_SETN, 5050, LSB_NULL_POINTER_STR))
#define I18N_FILE_PREMATURE    (_i18n_msg_get(ls_catd, NL_SETN, 5051, LSB_FILE_PREMATUR_STR))
#define I18N_EMPTY_SECTION     (_i18n_msg_get(ls_catd, NL_SETN, 5052, LSB_EMPTY_SECTION_STR))
#define I18N_IN_QUEUE_ADMIN    (_i18n_msg_get(ls_catd, NL_SETN, 5053, LSB_IN_QUEUE_ADMIN_STR))
#define I18N_IN_QUEUE_HOST     (_i18n_msg_get(ls_catd, NL_SETN, 5054, LSB_IN_QUEUE_HOST_STR))
#define I18N_IN_QUEUE_SEND_RCV (_i18n_msg_get(ls_catd, NL_SETN, 5055, LSB_IN_SEND_RCV_STR))
#define MAX_SELECTED_LOADS 4

static struct paramConf *pConf      = NULL;
static struct userConf *uConf       = NULL;
static struct hostConf *hConf       = NULL;
static struct queueConf *qConf      = NULL;
static struct userInfoEnt **users   = NULL;
static struct hostInfoEnt **hosts   = NULL;
static struct queueInfoEnt **queues = NULL;
static struct hostInfoEnt **hosts   = NULL;
static struct groupInfoEnt *usergroups[MAX_GROUPS];
static struct groupInfoEnt *hostgroups[MAX_GROUPS];

static struct hTab unknownUsers;
static struct clusterConf cConf;
static struct sharedConf sConf;

static int numofugroups = 0;
static int numofhosts   = 0;
static int numofhgroups = 0;
static int numofqueues  = 0;
static int usersize     = 0;
static int hostsize     = 0;
static int queuesize    = 0;
float      maxFactor    = 0.0;
char      *maxHName     = NULL;

static int numofusers = 0;
static bool_t initUnknownUsers = FALSE;


char *getNextWord1_ (char **);

#define TYPE1  RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL
#define TYPE2  RESF_BUILTIN | RESF_GLOBAL

static char do_Param  (struct lsConf *, char *, int *);
static char do_Users  (struct lsConf *, char *, int *, int);
static char do_Hosts  (struct lsConf *, char *, int *, struct lsInfo *, int);
static char do_Queues (struct lsConf *, char *, int *, struct lsInfo *, int);
static char do_Groups (struct groupInfoEnt **, struct lsConf *, char *, int *, int *, int);
static int  isHostName (char *grpName);
static int  addHost   (struct hostInfoEnt *, struct hostInfo *, int);
static char addQueue  (struct queueInfoEnt *, char *, int);
static char addUser   (char *, int, float, char *, int);
static char addMember (struct groupInfoEnt *, char *, int, char *, int, char *, int, int);

static char isInGrp (char *, struct groupInfoEnt *, int, int);
static char **expandGrp (char *, int *, int);
static struct groupInfoEnt *addGroup (struct groupInfoEnt **, char *, int *, int);

static struct groupInfoEnt *addUnixGrp (struct group *,	char *, char *, int, char *, int);
static char *parseGroups (char *, char *, int *, char *, int, int);

static struct groupInfoEnt *getUGrpData (char *);
static struct groupInfoEnt *getHGrpData (char *);
static struct groupInfoEnt *getGrpData (struct groupInfoEnt **, char *, int);
static struct userInfoEnt  *getUserData (char *);
static struct hostInfoEnt  *getHostData (char *);
static struct queueInfoEnt *getQueueData (char *);

static void initParameterInfo (struct parameterInfo *);
static void freeParameterInfo (struct parameterInfo *);
static void initUserInfo  (struct userInfoEnt *);
static void freeUserInfo  (struct userInfoEnt *);
static void initGroupInfo (struct groupInfoEnt *);
static void freeGroupInfo (struct groupInfoEnt *);
static void initHostInfo  (struct hostInfoEnt *);
static void freeHostInfo  (struct hostInfoEnt *);
static void initQueueInfo (struct queueInfoEnt *);
static void freeQueueInfo (struct queueInfoEnt *);


static void initThresholds (struct lsInfo *, float *, float *);
static void getThresh (struct lsInfo *, struct keymap *, float *, float *, char *, int *, char *);

static char searchAll (char *);

static void checkCpuLimit (char **, float **, int, char *, int *, char *, struct lsInfo *, int);
static int parseDefAndMaxLimits (struct keymap, int *, int *, char *, int *, char *);
static int parseCpuAndRunLimit (struct keymap *, struct queueInfoEnt *, char *, int *, char *, struct lsInfo *, int);
static int parseProcLimit (char *, struct queueInfoEnt *, char *, int *, char *);
static int parseLimitAndSpec (char *, int *, char **, char *, char *, struct queueInfoEnt *, char *, int *, char *);
static int my_atoi (char *, int, int);
static float my_atof (char *, float, float);
static void resetUConf (struct userConf *);
static int copyHostInfo (struct hostInfoEnt *, struct hostInfoEnt *);
static void resetHConf (struct hostConf *hConf);
static float *getModelFactor (char *, struct lsInfo *);
static float *getHostFactor (char *);
static char *parseAdmins (char *, int, char *, int *);
static char *putIntoList (char **, int *, char *, char *);
static int isInList (char *, char *);
static int setDefaultHost (struct lsInfo *);
static int handleUserMem (void);
static int setDefaultUser (void);
static int handleHostMem (void);
static void freeSA (char **, int);
static int checkAllOthers (char *, int *);
static int resolveBatchNegHosts (char *, char **, int);
static int fillCell (struct inNames **, char *, char *);
static int expandWordAll (int *, int *, struct inNames **, char *);
static int readHvalues_conf (struct keymap *, char *, struct lsConf *, char *, int *, int, char *);
static char *a_getNextWord_ (char **);
static int getReserve (char *, struct queueInfoEnt *, char *, int);
static int isServerHost (char *);

static void addBinaryAttributes (char *, int *, struct queueInfoEnt *, struct keymap *, unsigned int, char *);
static int parseQFirstHost (char *, int *, char *, int *, char *, char *);

void freeUConf (struct userConf *, int);
void freeHConf (struct hostConf *, int);
void freeQConf (struct queueConf *, int);
void freeWorkUser (int);
void freeWorkHost (int);
void freeWorkQueue (int);
int chkFirstHost (char *, int *);
int checkSpoolDir (char *spoolDir);
int checkJobAttaDir (char *);


extern int sigNameToValue_ (char *);
extern int parseSigActCmd (struct queueInfoEnt *, char *, char *, int *, char *);
extern int terminateWhen (struct queueInfoEnt *, char *, char *, int *, char *);
extern char checkRequeEValues (struct queueInfoEnt *, char *, char *, int *);



#endif
