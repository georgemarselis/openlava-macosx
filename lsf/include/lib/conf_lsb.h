// Added by George Marselis <george@marsel.is> Sun April 7 2019

#pragma once



#include "lib/conf.h"
#include "lib/table.h"
// #include "lsb/lsbatch.h"
#include "libint/lsi18n.h"

// #define USEREQUIVALENT "userequivalent"
// #define USERMAPPING    "usermap"
const char userequivalent[ ] = "userequivalent";
const char usermapping[ ]    = "usermap";


// #define I18N_NULL_POINTER      (_i18n_msg_get(ls_catd, NL_SETN, 5050, LSB_NULL_POINTER_STR))
// #define I18N_FILE_PREMATURE    (_i18n_msg_get(ls_catd, NL_SETN, 5051, LSB_FILE_PREMATUR_STR))
// #define I18N_EMPTY_SECTION     (_i18n_msg_get(ls_catd, NL_SETN, 5052, LSB_EMPTY_SECTION_STR))
// #define I18N_IN_QUEUE_ADMIN    (_i18n_msg_get(ls_catd, NL_SETN, 5053, LSB_IN_QUEUE_ADMIN_STR))
// #define I18N_IN_QUEUE_HOST     (_i18n_msg_get(ls_catd, NL_SETN, 5054, LSB_IN_QUEUE_HOST_STR))
// #define I18N_IN_QUEUE_SEND_RCV (_i18n_msg_get(ls_catd, NL_SETN, 5055, LSB_IN_SEND_RCV_STR))

// #define MAX_SELECTED_LOADS 4
enum MAX_SELECTED {
	MAX_SELECTED_LOADS = 4
};

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
double maxFactor  = 0.0F;
char *maxHName    = NULL;

bool_t initUnknownUsers = FALSE;


// #define QKEY_NAME                   info->numIndx + 0
// #define QKEY_PRIORITY               info->numIndx + 1
// #define QKEY_NICE                   info->numIndx + 2
// #define QKEY_UJOB_LIMIT             info->numIndx + 3
// #define QKEY_PJOB_LIMIT             info->numIndx + 4
// #define QKEY_RUN_WINDOW             info->numIndx + 5
// #define QKEY_CPULIMIT               info->numIndx + 6
// #define QKEY_FILELIMIT              info->numIndx + 7
// #define QKEY_DATALIMIT              info->numIndx + 8
// #define QKEY_STACKLIMIT             info->numIndx + 9
// #define QKEY_CORELIMIT              info->numIndx + 10
// #define QKEY_MEMLIMIT               info->numIndx + 11
// #define QKEY_RUNLIMIT               info->numIndx + 12
// #define QKEY_USERS                  info->numIndx + 13
// #define QKEY_HOSTS                  info->numIndx + 14
// #define QKEY_EXCLUSIVE              info->numIndx + 15
// #define QKEY_DESCRIPTION            info->numIndx + 16
// #define QKEY_MIG                    info->numIndx + 17
// #define QKEY_QJOB_LIMIT             info->numIndx + 18
// #define QKEY_POLICIES               info->numIndx + 19
// #define QKEY_DISPATCH_WINDOW        info->numIndx + 20
// #define QKEY_USER_SHARES            info->numIndx + 21
// #define QKEY_DEFAULT_HOST_SPEC      info->numIndx + 22
// #define QKEY_PROCLIMIT              info->numIndx + 23
// #define QKEY_ADMINISTRATORS         info->numIndx + 24
// #define QKEY_PRE_EXEC               info->numIndx + 25
// #define QKEY_POST_EXEC              info->numIndx + 26
// #define QKEY_REQUEUE_EXIT_VALUES    info->numIndx + 27
// #define QKEY_HJOB_LIMIT             info->numIndx + 28
// #define QKEY_RES_REQ                info->numIndx + 29
// #define QKEY_SLOT_RESERVE           info->numIndx + 30
// #define QKEY_RESUME_COND            info->numIndx + 31
// #define QKEY_STOP_COND              info->numIndx + 32
// #define QKEY_JOB_STARTER            info->numIndx + 33
// #define QKEY_SWAPLIMIT              info->numIndx + 34
// #define QKEY_PROCESSLIMIT           info->numIndx + 35
// #define QKEY_JOB_CONTROLS           info->numIndx + 36
// #define QKEY_TERMINATE_WHEN         info->numIndx + 37
// #define QKEY_NEW_JOB_SCHED_DELAY    info->numIndx + 38
// #define QKEY_INTERACTIVE            info->numIndx + 39
// #define QKEY_JOB_ACCEPT_INTERVAL    info->numIndx + 40
// #define QKEY_BACKFILL               info->numIndx + 41
// #define QKEY_IGNORE_DEADLINE        info->numIndx + 42
// #define QKEY_CHKPNT                 info->numIndx + 43
// #define QKEY_RERUNNABLE             info->numIndx + 44
// #define QKEY_ENQUE_INTERACTIVE_AHEAD info->numIndx +45
// #define QKEY_ROUND_ROBIN_POLICY     info->numIndx + 46
// #define QKEY_PRE_POST_EXEC_USER     info->numIndx + 47
// #define KEYMAP_SIZE                 info->numIndx + 49


    // FREEUP (keyList);
    // assert( KEYMAP_SIZE >= 0 );
    // keyList = calloc( KEYMAP_SIZE, sizeof (struct keymap));
    // if( NULL == keyList && ENOMEM == errno ) {
    //  return false;
    // }

    // assert( QKEY_NAME + 1 <= INT_MAX );
    // assert( KEYMAP_SIZE <= INT_MAX );
    // initkeyList (keyList, (int)(QKEY_NAME + 1), (int)KEYMAP_SIZE, info);

    // keyList[QKEY_NAME].key                    = "QUEUE_NAME";
    // keyList[QKEY_PRIORITY].key                = "PRIORITY";
    // keyList[QKEY_NICE].key                    = "NICE";
    // keyList[QKEY_UJOB_LIMIT].key              = "UJOB_LIMIT";
    // keyList[QKEY_PJOB_LIMIT].key              = "PJOB_LIMIT";
    // keyList[QKEY_RUN_WINDOW].key              = "RUN_WINDOW";
    // keyList[QKEY_CPULIMIT].key                = "CPULIMIT";
    // keyList[QKEY_FILELIMIT].key               = "FILELIMIT";
    // keyList[QKEY_DATALIMIT].key               = "DATALIMIT";
    // keyList[QKEY_STACKLIMIT].key              = "STACKLIMIT";
    // keyList[QKEY_CORELIMIT].key               = "CORELIMIT";
    // keyList[QKEY_MEMLIMIT].key                = "MEMLIMIT";
    // keyList[QKEY_RUNLIMIT].key                = "RUNLIMIT";
    // keyList[QKEY_USERS].key                   = "USERS";
    // keyList[QKEY_HOSTS].key                   = "HOSTS";
    // keyList[QKEY_EXCLUSIVE].key               = "EXCLUSIVE";
    // keyList[QKEY_DESCRIPTION].key             = "DESCRIPTION";
    // keyList[QKEY_MIG].key                     = "MIG";
    // keyList[QKEY_QJOB_LIMIT].key              = "QJOB_LIMIT";
    // keyList[QKEY_POLICIES].key                = "POLICIES";
    // keyList[QKEY_DISPATCH_WINDOW].key         = "DISPATCH_WINDOW";
    // keyList[QKEY_USER_SHARES].key             = "USER_SHARES";
    // keyList[QKEY_DEFAULT_HOST_SPEC].key       = "DEFAULT_HOST_SPEC";
    // keyList[QKEY_PROCLIMIT].key               = "PROCLIMIT";
    // keyList[QKEY_ADMINISTRATORS].key          = "ADMINISTRATORS";
    // keyList[QKEY_PRE_EXEC].key                = "PRE_EXEC";
    // keyList[QKEY_POST_EXEC].key               = "POST_EXEC";
    // keyList[QKEY_REQUEUE_EXIT_VALUES].key     = "REQUEUE_EXIT_VALUES";
    // keyList[QKEY_HJOB_LIMIT].key              = "HJOB_LIMIT";
    // keyList[QKEY_RES_REQ].key                 = "RES_REQ";
    // keyList[QKEY_SLOT_RESERVE].key            = "SLOT_RESERVE";
    // keyList[QKEY_RESUME_COND].key             = "RESUME_COND";
    // keyList[QKEY_STOP_COND].key               = "STOP_COND";
    // keyList[QKEY_JOB_STARTER].key             = "JOB_STARTER";
    // keyList[QKEY_SWAPLIMIT].key               = "SWAPLIMIT";
    // keyList[QKEY_PROCESSLIMIT].key            = "PROCESSLIMIT";
    // keyList[QKEY_JOB_CONTROLS].key            = "JOB_CONTROLS";
    // keyList[QKEY_TERMINATE_WHEN].key          = "TERMINATE_WHEN";
    // keyList[QKEY_NEW_JOB_SCHED_DELAY].key     = "NEW_JOB_SCHED_DELAY";
    // keyList[QKEY_INTERACTIVE].key             = "INTERACTIVE";
    // keyList[QKEY_JOB_ACCEPT_INTERVAL].key     = "JOB_ACCEPT_INTERVAL";
    // keyList[QKEY_BACKFILL].key                = "BACKFILL";
    // keyList[QKEY_IGNORE_DEADLINE].key         = "IGNORE_DEADLINE";
    // keyList[QKEY_CHKPNT].key                  = "CHKPNT";
    // keyList[QKEY_RERUNNABLE].key              = "RERUNNABLE";
    // keyList[QKEY_ENQUE_INTERACTIVE_AHEAD].key = "ENQUE_INTERACTIVE_AHEAD";
    // keyList[QKEY_ROUND_ROBIN_POLICY].key      = "ROUND_ROBIN_POLICY";
    // keyList[QKEY_PRE_POST_EXEC_USER].key      = "PRE_POST_EXEC_USER";
    // keyList[KEYMAP_SIZE - 1].key              = NULL;

/* lib/liblsf/conf_lsb.c */
void freeSA( char **list, unsigned int num);
size_t fillCell_(struct inNames **table, const char *name, const char *level);
void initHostInfoEnt(struct hostInfoEnt *hp);
void freeHostInfoEnt(struct hostInfoEnt *hp);
unsigned int expandWordAll(size_t *size, unsigned int *num, struct inNames **inTable, const char *ptr_level);
int readHvalues_conf(struct keymap *keyList, const char *linep, struct lsConf *conf, const char *lsfile, size_t *lineNumber, int exact, const char *section);
struct paramConf *lsb_readparam(struct lsConf *conf);
char do_Param(struct lsConf *conf, const char *filename, size_t *lineNum);
unsigned int my_atoi( const char *arg, int upBound, int botBound);
float my_atof(char *arg, float upBound, float botBound);
void initParameterInfo(struct parameterInfo *param);
void freeParameterInfo(struct parameterInfo *param);
int checkSpoolDir(const char *pSpoolDir);
struct userConf *lsb_readuser(struct lsConf *conf, int options, struct clusterConf *clusterConf);
struct userConf *lsb_readuser_ex(struct lsConf *conf, int options, struct clusterConf *clusterConf, struct sharedConf *sharedConf);
char do_Users(struct lsConf *conf, const char *filename, size_t *lineNum, int options);
char do_Groups(struct groupInfoEnt **groups, struct lsConf *conf, const char *filename, size_t *lineNum, unsigned int *ngroups, int options);
int isHostName(const char *grpName);
struct groupInfoEnt *addGroup(struct groupInfoEnt **groups, const char *gname, unsigned int *ngroups, int type);
struct groupInfoEnt *addUnixGrp(struct group *unixGrp, const char *gname, const char *filename, unsigned int lineNum, const char *section, int type);
char addMember(struct groupInfoEnt *gp, const char *word, int grouptype, const char *filename, unsigned int lineNum, const char *section, int options, int checkAll);
struct groupInfoEnt *getUGrpData(const char *gname);
struct groupInfoEnt *getHGrpData(const char *gname);
struct groupInfoEnt *getGrpData(struct groupInfoEnt **groups, const char *name, unsigned int num);
struct userInfoEnt *getUserData(const char *name);
struct hostInfoEnt *getHostData(const char *name);
struct queueInfoEnt *getQueueData(const char *name);
char searchAll(char *const word);
void initUserInfo(struct userInfoEnt *up);
void freeUserInfo(struct userInfoEnt *up);
void initGroupInfo(struct groupInfoEnt *gp);
void freeGroupInfo(struct groupInfoEnt *gp);
char addUser(const char *username, size_t maxjobs, float pJobLimit, const char *filename, int override);
char isInGrp(char *word, struct groupInfoEnt *gp, int grouptype, int checkAll);
char **expandGrp(char *word, unsigned int *num, int grouptype);
struct hostConf *lsb_readhost(struct lsConf *conf, struct lsInfo *info, int options, struct clusterConf *clusterConf);
void getThresh(struct lsInfo *info, struct keymap *keylist, float loadSched[], float loadStop[], const char *filename, size_t *lineNum, const char *section);
int addHostEnt(struct hostInfoEnt *hp, struct hostInfo *hostInfo, size_t *override);
int copyHostInfo(struct hostInfoEnt *toHost, struct hostInfoEnt *fromHost);
void initThresholds(struct lsInfo *info, float loadSched[], float loadStop[]);
char *parseGroups(char *linep, const char *filename, size_t *lineNum, const char *section, int groupType, int options);
struct queueConf *lsb_readqueue(struct lsConf *conf, struct lsInfo *info, int options, struct sharedConf *sharedConf);
char do_Queues(struct lsConf *conf, char *filename, size_t *lineNum, struct lsInfo *info, int options);
void initQueueInfo(struct queueInfoEnt *qp);
void freeQueueInfo(struct queueInfoEnt *qp);
char checkRequeEValues(struct queueInfoEnt *qp, char *word, const char *filename, size_t *lineNum);
char addQueue(struct queueInfoEnt *qp, const char *filename, unsigned int lineNum);
void freeWorkUser(int freeAll);
void freeWorkHost(int freeAll);
void freeWorkQueue(int freeAll);
void freeUConf(struct userConf *uConf1, int freeAll);
void freeHConf(struct hostConf *hConf1, int freeAll);
void freeQConf(struct queueConf *qConf1, int freeAll);
void resetUConf(struct userConf *uConf1);
void resetHConf(struct hostConf *hConf1);
void checkCpuLimit(char **hostSpec, double **cpuFactor, int useSysDefault, const char *filename, size_t *lineNum, char *pname, struct lsInfo *info, int options);
int parseCpuAndRunLimit(struct keymap *keylist, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, char *pname, struct lsInfo *info, int options);
int parseProcLimit(char *word, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, char *pname);
int parseLimitAndSpec(char *word, int *limit, char **spec, char *hostSpec, char *param, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, const char *pname);
double *getModelFactor(char *hostModel, struct lsInfo *info);
double *getHostFactor(char *host);
char *parseAdmins(char *admins, int options, char *filename, size_t *lineNum);
char *putIntoList(char **list, unsigned int *len, const char *string, const char *listName);
int setDefaultHost(struct lsInfo *info);
int setDefaultUser(void);
int handleUserMem(void);
int handleHostMem(void);
int parseSigActCmd(struct queueInfoEnt *qp, char *linep, const char *filename, size_t *lineNum, const char *section);
int terminateWhen(struct queueInfoEnt *qp, char *linep, const char *filename, size_t *lineNum, const char *section);
int checkAllOthers(char *word, int *hasAllOthers);
int getReserve(char *reserve, struct queueInfoEnt *qp, const char *filename, unsigned int lineNum);
int isServerHost(const char *hostName);
struct group *mygetgrnam(const char *name);
void freeUnixGrp(struct group *unixGrp);
struct group *copyUnixGrp(struct group *unixGrp);
void addBinaryAttributes(const char *confFile, size_t *lineNum, struct queueInfoEnt *queue, struct keymap *keylist, unsigned int attrib, const char *attribName);
int resolveBatchNegHosts(const char *inHosts, char **outHosts, int isQueue);
int checkJobAttaDir(const char *path);
int parseDefAndMaxLimits(struct keymap *key, unsigned int *defaultVal, unsigned int *maxVal, const char *filename, size_t *lineNum, const char *pname);
bool parseQFirstHost(char *myWord, bool *haveFirst, const char *pname, size_t *lineNum, const char *filename, const char *section);
bool chkFirstHost(const char *host, bool *needChk);
void updateClusterConf(struct clusterConf *clusterConf);



// char do_Param   ( struct lsConf *conf,          char *fname, size_t *lineNumber );
// char do_Users   ( struct lsConf *conf,          char *fname, size_t *lineNumber, int option );
// char do_Hosts_  ( struct lsConf *conf,          char *fname, size_t *lineNumber, struct lsInfo *info, int options );
// char do_Queues  ( struct lsConf *conf,          char *fname, size_t *lineNumber, struct lsInfo *info, int options );
// char do_Groups  ( struct groupInfoEnt **groups, struct lsConf *conf, char *fname, size_t *lineNumber, unsigned int *ngroups, int options );
// int  isHostName ( char *grpName );
// int  addHostEnt ( struct hostInfoEnt *hp, struct hostInfo *hostInfo, size_t *override );
// char addQueue   ( struct queueInfoEnt *qp, char *fname, unsigned int lineNum );
// char addUser    ( char *, int, float, char *, int );
// char addMember  ( struct groupInfoEnt *gp, char *word, int grouptype, char *fname, unsigned int lineNumber, char *section, int options, int checkAll );

// char                  isInGrp   ( char *, struct groupInfoEnt *, int, int);
// char                **expandGrp ( char *, unsigned int *, int);
// struct groupInfoEnt  *addGroup  ( struct groupInfoEnt **, char *, unsigned int *, int);

// struct groupInfoEnt *addUnixGrp (struct group *, char *, char *, unsigned int, char *, int);
// char *parseGroups (char *linep, char *fname, size_t *lineNumber, char *section, int groupType, int options);

// struct groupInfoEnt *getUGrpData (char *);
// struct groupInfoEnt *getHGrpData (char *);
// struct groupInfoEnt *getGrpData (struct groupInfoEnt **groups, char *name, unsigned int num);
// struct userInfoEnt  *getUserData (char *);
// struct hostInfoEnt  *getHostData (char *);
// struct queueInfoEnt *getQueueData (char *);

// void initParameterInfo (struct parameterInfo *);
// void freeParameterInfo (struct parameterInfo *);
// void initUserInfo  (struct userInfoEnt *);
// void freeUserInfo  (struct userInfoEnt *);
// void initGroupInfo (struct groupInfoEnt *);
// void freeGroupInfo (struct groupInfoEnt *);
// void initHostInfoEnt (struct hostInfoEnt *);
// void freeHostInfoEnt  (struct hostInfoEnt *);
// void initQueueInfo (struct queueInfoEnt *);
// void freeQueueInfo (struct queueInfoEnt *);


// void initThresholds (struct lsInfo *, float *, float *);
// void getThresh (struct lsInfo *info, struct keymap *keyList, float *loadSched, float *loadStop, char *fname, size_t *lineNumber, char *description);

// char searchAll (char *);

// void checkCpuLimit (char **hostSpec, double **cpuFactor, int useSysDefault, char *fname, size_t *lineNumber, char *pname, struct lsInfo *info, int options);
// int parseDefAndMaxLimits (struct keymap key, int *defaultVal, int *maxVal, char *fname, size_t *lineNumber, char *pname);
// int parseCpuAndRunLimit (struct keymap *keylist, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname, struct lsInfo *info, int options);
// int parseProcLimit (char *word, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname);
// int parseLimitAndSpec (char *word, int *limit, char **spec, char *hostSpec, char *param, struct queueInfoEnt *qp, char *fname, size_t *lineNumber, char *pname);
// int my_atoi (char *, int, int);
// float my_atof (char *, float, float);
// void resetUConf (struct userConf *);
// int copyHostInfo (struct hostInfoEnt *, struct hostInfoEnt *);
// void resetHConf (struct hostConf *hConf);
// double *getModelFactor (char *, struct lsInfo *);
// double *getHostFactor (char *);
// char *parseAdmins (char *admins, int options, char *fname, size_t *lineNum);
// char *putIntoList (char **list, unsigned int *len, char *string, char *listName);
// int setDefaultHost (struct lsInfo *);
// int handleUserMem (void);
// int setDefaultUser (void);
// int handleHostMem (void);
// void freeSA (char **list, unsigned int num);
// int checkAllOthers (char *, int *);
// int resolveBatchNegHosts (char *, char **, int);
// unsigned int fillCell_ (struct inNames **table, char *name, char *level);
// unsigned int expandWordAll (unsigned int *size, unsigned int *num, struct inNames **inTable, char *ptr_level);
// int readHvalues_conf (struct keymap *keyList, char *linep, struct lsConf *conf, char *lsfile, size_t *lineNumber, int exact, char *section);
// char *a_getNextWord_ (char **);
// int getReserve (char *reserve, struct queueInfoEnt *qp, char *filename, unsigned int lineNum);
// int isServerHost (char *);

// void addBinaryAttributes (char *confFile, size_t *lineNumber, struct queueInfoEnt *queue, struct keymap *keylist, unsigned int attrib, char *attribName);
// int parseQFirstHost (char *myWord, int *haveFirst, char *pname, size_t *lineNumber, char *fname, char *section);

// void freeUConf (struct userConf *, int);
// void freeHConf (struct hostConf *, int);
// void freeQConf (struct queueConf *, int);
// void freeWorkUser (int);
// void freeWorkHost (int);
// void freeWorkQueue (int);
// int chkFirstHost (char *, int *);
// int checkSpoolDir (char *spoolDir);
// int checkJobAttaDir (char *);


// int sigNameToValue_ (char *);
// int parseSigActCmd (struct queueInfoEnt *queue, char *val, char *fname, size_t *lineNumber, char *section);
// int terminateWhen (struct queueInfoEnt *qp, char *linep, char *fname, size_t *lineNumber, char *section);
// char checkRequeEValues (struct queueInfoEnt *qp, char *word, char *fname, size_t *lineNum);
