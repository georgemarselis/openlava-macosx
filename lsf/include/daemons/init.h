// added by George Marselis <george@marsel.is>

#pragma once

// #define NL_SETN         10

extern void resetStaticSchedVariables (void);
extern void cleanSbdNode (struct jData *);
extern void uDataPtrTbInitialize ();

// extern struct mbd_func_type mbd_func; // no decleration of struct anywhere

extern int FY_MONTH;

extern void freeUConf (struct userConf *, int);
extern void freeHConf (struct hostConf *, int);
extern void freeQConf (struct queueConf *, int);
extern void freeWorkUser (int);
extern void freeWorkHost (int);
extern void freeWorkQueue (int);

extern void uDataTableFree (UDATA_TABLE_T * uTab);
extern void cleanCandHosts (struct jData *);

#define setString(string, specString) {         \
        FREEUP(string);                         \
        if(specString != NULL)                  \
            string = safeSave(specString);}

#define setValue(value, specValue) {                                    \
        if(specValue != INFINIT_INT && specValue != INFINIT_FLOAT)      \
            value = specValue;                                          \
    }

static char defUser = FALSE;
static int numofclusters = 0;
static struct clusterInfo *clusterInfo = NULL;
struct tclLsInfo *tclLsInfo = NULL;
static struct clusterConf clusterConf;
static struct lsConf *paramFileConf = NULL;
static struct lsConf *userFileConf = NULL;
static struct lsConf *hostFileConf = NULL;
static struct lsConf *queueFileConf = NULL;
struct userConf *userConf = NULL;
static struct hostConf *hostConf = NULL;
static struct queueConf *queueConf = NULL;
static struct paramConf *paramConf = NULL;
static struct gData *tempUGData[MAX_GROUPS];
static struct gData *tempHGData[MAX_GROUPS];
static int nTempUGroups;
static int nTempHGroups;

static char batchName[MAX_LSB_NAME_LEN] = "root";


#define PARAM_FILE    0x01
#define USER_FILE     0x02
#define HOST_FILE     0x04
#define QUEUE_FILE    0x08

#define HDATA         1
#define UDATA         2

extern int rusageUpdateRate;
extern int rusageUpdatePercent;

extern void initTab (struct hTab *tabPtr);
static void readParamConf (int);
static int readHostConf (int);
static void readUserConf (int);
static void readQueueConf (int);

static int isHostAlias (char *grpName);
static int searchAll (char *);
static void initThresholds (float *, float *);
static void parseGroups (int, struct gData **, char *, char *);
static void addMember (struct gData *, char *, int, char *,
		       struct gData *group[], int *);
static struct gData *addGroup (struct gData **, char *, int *);
static struct qData *initQData (void);
static int isInGrp (char *word, struct gData *);
static struct gData *addUnixGrp (struct group *, char *, char *,
				 struct gData *group[], int *);
static void parseAUids (struct qData *, char *);

static void getClusterData (void);
static void setManagers (struct clusterInfo);
static void setAllusers (struct qData *, struct admins *);

static void createTmpGData (struct groupInfoEnt *, int, int,
			    struct gData *tempHGData[], int *);
static void addHostData (int, struct hostInfoEnt *);
static void setParams (struct paramConf *);
static void addUData (struct userConf *);
static void setDefaultParams (void);
static void addQData (struct queueConf *, int);
static int updCondData (struct lsConf *, int);
static struct condData *initConfData (void);
static void createCondNodes (int, char **, char *, int);
static struct lsConf *getFileConf (char *, int);
static void copyQData (struct qData *, struct qData *);
static void copyGroups (int);
static void addDefaultHost (void);
static void removeFlags (struct hTab *, int, int);
static int needPollQHost (struct qData *, struct qData *);
static void updQueueList (void);
static void updUserList (int);

static void addUGDataValues (struct uData *, struct gData *);
static void addUGDataValues1 (struct uData *, struct uData *);
static int parseQHosts (struct qData *, char *);
static void fillClusterConf (struct clusterConf *);
static void fillSharedConf (struct sharedConf *);
static void createDefQueue (void);
static void freeGrp (struct gData *);
static int validHostSpec (char *);
static void getMaxCpufactor (void);
static int parseFirstHostErr (int, char *, char *, struct qData *,
			      struct askedHost *, int);

static struct hData *mkLostAndFoundHost (void);