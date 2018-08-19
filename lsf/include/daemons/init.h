// added by George Marselis <george@marsel.is>

#pragma once

#define setString(string, specString) {         \
        FREEUP(string);                         \
        if(specString != NULL)                  \
            string = safeSave(specString);}

#define setValue(value, specValue) {                                    \
        if(specValue != INFINIT_INT && specValue != INFINIT_FLOAT)      \
            value = specValue;                                          \
    }

// #define NL_SETN         10

// extern void resetStaticSchedVariables (void);
// extern void cleanSbdNode (struct jData *);
// extern void uDataPtrTbInitialize ();

// extern struct mbd_func_type mbd_func; // no decleration of struct anywhere

// extern int FY_MONTH;

// extern void freeUConf (struct userConf *, int);
// extern void freeHConf (struct hostConf *, int);
// extern void freeQConf (struct queueConf *, int);
// extern void freeWorkUser (int);
// extern void freeWorkHost (int);
// extern void freeWorkQueue (int);

// extern void uDataTableFree (UDATA_TABLE_T * uTab);
// extern void cleanCandHosts (struct jData *);


// static char defUser = FALSE;
// static int numofclusters = 0;
// static struct clusterInfo *clusterInfo = NULL;
// static struct clusterConf clusterConf;
// static struct lsConf *paramFileConf = NULL;
// static struct lsConf *userFileConf = NULL;
// static struct lsConf *hostFileConf = NULL;
// static struct lsConf *queueFileConf = NULL;
// static struct hostConf *hostConf = NULL;
// static struct queueConf *queueConf = NULL;
// static struct paramConf *paramConf = NULL;
// static struct gData *tempUGData[MAX_GROUPS];
// static struct gData *tempHGData[MAX_GROUPS];
// static int nTempUGroups;
// static int nTempHGroups;

char defUser = FALSE;
int numofclusters = 0;
struct clusterInfo *clusterInfo = NULL;
struct clusterConf clusterConf;
struct lsConf *paramFileConf = NULL;
struct lsConf *userFileConf = NULL;
struct lsConf *hostFileConf = NULL;
struct lsConf *queueFileConf = NULL;
struct hostConf *hostConf = NULL;
struct queueConf *queueConf = NULL;
struct paramConf *paramConf = NULL;
struct gData *tempUGData[MAX_GROUPS];
struct gData *tempHGData[MAX_GROUPS];
int nTempUGroups = 0;
int nTempHGroups = 0;


struct tclLsInfo *tclLsInfo = NULL;
struct userConf *userConf = NULL;



const char batchName[MAX_LSB_NAME_LEN] = "root";


// #define PARAM_FILE    0x01
// #define USER_FILE     0x02
// #define HOST_FILE     0x04
// #define QUEUE_FILE    0x08

enum PUHQ_FILES {
	PARAM_FILE = 0x01,
	USER_FILE  = 0x02,
	HOST_FILE  = 0x04,
	QUEUE_FILE = 0x08,
};

// #define HDATA         1
// #define UDATA         2
enum HUDATA {
	HDATA = 1,
	UDATA = 2
};

// extern int rusageUpdateRate;
// extern int rusageUpdatePercent;

// extern void initTab (struct hTab *tabPtr);
void readParamConf (int);
int readHostConf (int);
void readUserConf (int);
void readQueueConf (int);

int isHostAlias (const char *grpName);
int searchAll ( const char *);
void initThresholds (float *, float *);
void parseGroups (int, struct gData **, char *, char *);
void addMember (struct gData *, char *, int, char *, struct gData *group[], int *);
struct gData *addGroup (struct gData **, char *, int *);
struct qData *initQData (void);
int isInGrp (char *word, struct gData *);
struct gData *addUnixGrp (struct group *, char *, char *, struct gData *group[], int *);
void parseAUids (struct qData *, char *);

void getClusterData (void);
void setManagers (struct clusterInfo);
void setAllusers (struct qData *, struct admins *);

void createTmpGData (struct groupInfoEnt *, int, int, struct gData *tempHGData[], int *);
void addHostData (int, struct hostInfoEnt *);
void setParams (struct paramConf *);
void addUData (struct userConf *);
void setDefaultParams (void);
void addQData (struct queueConf *, int);
int updCondData (struct lsConf *, int);
struct condData *initConfData (void);
void createCondNodes (int, char **, char *, int);
struct lsConf *getFileConf (char *, int);
void copyQData (struct qData *, struct qData *);
void copyGroups (int);
void addDefaultHost (void);
void removeFlags (struct hTab *, int, int);
int needPollQHost (struct qData *, struct qData *);
void updQueueList (void);
void updUserList (int);

void addUGDataValues (struct uData *, struct gData *);
void addUGDataValues1 (struct uData *, struct uData *);
int parseQHosts (struct qData *, char *);
void fillClusterConf (struct clusterConf *);
void fillSharedConf (struct sharedConf *);
void createDefQueue (void);
void freeGrp (struct gData *);
int validHostSpec (char *);
void getMaxCpufactor (void);
int parseFirstHostErr (int, char *, char *, struct qData *, struct askedHost *, int);

struct hData *mkLostAndFoundHost (void);