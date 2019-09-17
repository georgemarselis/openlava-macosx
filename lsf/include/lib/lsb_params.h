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

enum MAX_ARCHIVE {
	ACCT_CHECK            = 0,
	MAX_ACCT_ARCHIVE_FILE = 1,
	ACCT_ARCHIVE_SIZE     = 2,
	ACCT_ARCHIVE_AGE      = 3,
	ACCT_NONE_SET         = 4,
	ACC_ALL_SET           = 5
};

 struct inNames {
    char *name;      // FIXME FIXME FIXME learn how to init and set double indirection
    char *prf_level; // FIXME FIXME FIXME learn how to init and set double indirection
};

struct parameterInfo *pConf  = NULL;
struct userConf      *uConf  = NULL;
struct hostConf      *hConf  = NULL;
struct queueConf     *qConf  = NULL;
struct userInfoEnt  **users  = NULL;
struct hostInfoEnt  **hosts  = NULL;
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
double maxFactor          = 0.0F;
char *maxHostName         = NULL;

bool_t initUnknownUsers = FALSE;


/* lib/liblsf/conf_lsb.c */
void freeSA( char **list, unsigned int num);
size_t fillCell_(struct inNames **table, const char *name, const char *level);
void initHostInfoEnt(struct hostInfoEnt *hp);
void freeHostInfoEnt(struct hostInfoEnt *hp);
unsigned int expandWordAll(size_t *size, unsigned int *num, struct inNames **inTable, const char *ptr_level);
int readHvalues_conf(struct keymap *keyList, const char *linep, struct lsConf *conf, const char *lsfile, size_t *lineNumber, int exact, const char *section);
struct parameterInfo *lsb_readparam(struct lsConf *conf);
char do_Param(struct lsConf *conf, const char *filename, size_t *lineNum);
unsigned int my_atoi( const char *arg, int upBound, int botBound);
float my_atof(char *arg, float upBound, float botBound);
void initParameterInfo(struct parameterInfo *param);
void freeParameterInfo(struct parameterInfo *param);
size_t trimwhitespace(char *out, const char *str);
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
char isInGrp( const char *word, struct groupInfoEnt *gp, int grouptype, int checkAll);
char **expandGrp( const char *word, unsigned int *num, int grouptype);
struct hostConf *lsb_readhost(struct lsConf *conf, struct lsInfo *info, int options, struct clusterConf *clusterConf);
void getThresh(struct lsInfo *info, struct keymap *keylist, float loadSched[], float loadStop[], const char *filename, size_t *lineNum, const char *section);
int addHostEnt(struct hostInfoEnt *hp, struct hostInfo *hostInfo, size_t *override);
int copyHostInfo(struct hostInfoEnt *toHost, struct hostInfoEnt *fromHost);
void initThresholds(struct lsInfo *info, float loadSched[], float loadStop[]);
char *parseGroups( const char *linep, const char *filename, size_t *lineNum, const char *section, int groupType, int options);
struct queueConf *lsb_readqueue(struct lsConf *conf, struct lsInfo *info, int options, struct sharedConf *sharedConf);
char do_Queues(struct lsConf *conf, const char *filename, size_t *lineNum, struct lsInfo *info, int options);
void initQueueInfo(struct queueInfoEnt *qp);
void freeQueueInfo(struct queueInfoEnt *qp);
char checkRequeEValues(struct queueInfoEnt *qp, const char *word, const char *filename, size_t *lineNum);
char addQueue(struct queueInfoEnt *qp, const char *filename, unsigned int lineNum);
void freeWorkUser(int freeAll);
void freeWorkHost(int freeAll);
void freeWorkQueue(int freeAll);
void freeUConf(struct userConf *uConf1, int freeAll);
void freeHConf(struct hostConf *hConf1, int freeAll);
void freeQConf(struct queueConf *qConf1, int freeAll);
void resetUConf(struct userConf *uConf1);
void resetHConf(struct hostConf *hConf1);
void checkCpuLimit(char **hostSpec, double **cpuFactor, int useSysDefault, const char *filename, size_t *lineNum, const char *pname, struct lsInfo *info, int options);
int parseCpuAndRunLimit(struct keymap *keylist, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, const char *pname, struct lsInfo *info, int options);
int parseProcLimit( const char *word, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, const char *pname);
int parseLimitAndSpec( const char *word, int *limit, char **spec, const char *hostSpec, const char *param, struct queueInfoEnt *qp, const char *filename, size_t *lineNum, const char *pname);
double *getModelFactor( const char *hostModel, struct lsInfo *info);
double *getHostFactor( const char *host);
char *parseAdmins( const char *admins, int options, const char *filename, size_t *lineNum);
char *putIntoList(const char **list, unsigned int *length, const char *string, const char *listName);
int setDefaultHost(struct lsInfo *info);
int setDefaultUser(void);
int handleUserMem(void);
int handleHostMem(void);
int parseSigActCmd(struct queueInfoEnt *qp, char *linep, const char *filename, size_t *lineNum, const char *section);
int terminateWhen(struct queueInfoEnt *qp, char *linep, const char *filename, size_t *lineNum, const char *section);
int checkAllOthers( const char *word, int *hasAllOthers);
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
bool enableAccounting( enum MAX_ARCHIVE max_archive_var  );
