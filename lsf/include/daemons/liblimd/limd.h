/*
 * Copyright (C) 2011 David Bigagli
 *
 * $Id: lim.h 397 2007-11-26 19:04:00Z mblack $
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
 
#include "libint/intlibout.h"
#include "daemons/liblimd/limout.h"
#include "lib/hdr.h"
#include "lib/lproto.h"
#include "lib/table.h"
#include "lib/xdr.h"
#include "lsf.h"

#define EXCHINTVL             15
#define SAMPLINTVL            5
#define HOSTINACTIVITYLIMIT   5
#define MASTERINACTIVITYLIMIT 2
#define RESINACTIVITYLIMIT    9
#define RETRYLIMIT            2

#define SBD_ACTIVE_TIME 60*5

#define KEEPTIME   2
#define MAXCANDHOSTS  10
#define MAXCLIENTS   64
#define WARNING_ERR   EXIT_WARNING_ERROR
#define MIN_FLOAT16  2.328306E-10
#define LIM_EVENT_MAXSIZE  (1024 * 1024)

#ifndef MIN
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define DEFAULT_AFTER_HOUR "19:00-7:00 5:19:00-1:7:00"

struct timewindow
{
  char *winName;
  windows_t *week[8];
  time_t wind_edge;
};

#define LIM_STARTUP    0
#define LIM_CHECK      1
#define LIM_RECONFIG   2

struct statInfo
{
  short hostNo;
  char padding1[2];
  int maxCpus;
  int maxMem;
  int maxSwap;
  int maxTmp;
  int nDisks;
  u_short portno;
  char *hostType;
  char *hostArch;
  char padding2[2];
};

#define HF_INFO_VALID   0x01
#define HF_REDUNDANT    0x02
#define HF_CONSTATUS    0x04

struct hostNode
{
    short hostNo;
    short availHigh;
    short availLow;
    short use;
    short hostInactivityCount;
    unsigned short naddr;
    unsigned int nRes;
    char padding1[2];
    int hModelNo;
    int hTypeNo;
    int infoMask;
    int loadMask;
    unsigned int numInstances;
    int callElim;
    int maxResIndex;
    int resClass;
    int DResClass;
    int rexPriority;
    unsigned int8_t migrant;
    char padding2[7];
    unsigned int *resBitMaps;
    unsigned int *DResBitMaps;
    int *resBitArray;
    int *status;
    in_addr_t *addr;
    unsigned int lastSeqNo;
    char infoValid;
    char conStatus;
    unsigned char protoVersion;
    char padding3[1];
    char *hostName;
    char *windows;
    windows_t *week[8];
    float *busyThreshold;
    float *loadIndex;
    float *uloadIndex;
    time_t wind_edge;
    time_t lastJackTime;
    time_t expireTime;
    struct hostNode *nextPtr;
    struct statInfo statInfo;
    char padding4[4];
    struct resourceInstance **instances;
};

#define CLUST_ACTIVE    0x00010000
#define CLUST_MASTKNWN      0x00020000
#define CLUST_CONNECT       0x00040000
#define CLUST_INFO_AVAIL    0x00080000
#define CLUST_HINFO_AVAIL   0x00100000
#define CLUST_ELIGIBLE      0x00200000
#define CLUST_ALL_ELIGIBLE  0x00400000


struct clusterNode
{
    unsigned short masterPort;
    unsigned short checkSum;
    char masterKnown;
    char padding1[3];
    int status;
    int currentAddr;
    int resClass;
    int typeClass;
    int modelClass;
    int masterInactivityCount;
    int chanfd;
    size_t numIndx;
    size_t numUsrIndx;
    int usrIndxClass;
    unsigned int *resBitMaps;
    unsigned int *hostTypeBitMaps;
    unsigned int *hostModelBitMaps;
    unsigned int clusterNo;
    unsigned int masterAddr;
    unsigned int numHosts;
    unsigned int numClients;
    unsigned int nAdmins;
    unsigned int nRes;
    unsigned int numSharedRes;
    char padding3[4];
    uid_t *adminIds;
    uid_t managerId;
    char padding2[4];
    char *masterName;
    char *clName;
    char *managerName;
    char *eLimArgs;
    char **eLimArgv;
    char **loadIndxNames;
    char **admins;
    char **sharedResource;
    in_addr_t candAddrList[MAXCANDHOSTS];
    struct hostNode *masterPtr;
    struct hostNode *prevMasterPtr;
    struct hostNode *hostList;
    struct hostNode *clientList;
    struct clusterNode *nextPtr;
    struct shortLsInfo *shortInfo;
};

struct clientNode
{
  char inprogress;
  char padding1[3];
  int clientMasks;
  int chanfd;
  enum limReqCode limReqCode;
  struct hostNode *fromHost;
  struct sockaddr_in from;
  struct Buffer *reqbuf;
};

struct liStruct
{
  char *name;
  char increasing;
  char padding1[3];
  float delta[2];
  float extraload[2];
  float valuesent;
  float exchthreshold;
  float sigdiff;
  float satvalue;
  float value;
};

// static unsigned int li_len;
// static struct liStruct *li;

#define  SEND_NO_INFO       0x00
#define  SEND_CONF_INFO     0x01
#define  SEND_LOAD_INFO     0x02
#define  SEND_MASTER_ANN    0x04
#define  SEND_ELIM_REQ      0x08
#define  SEND_MASTER_QUERY  0x10
#define  SLIM_XDR_DATA      0x20
#define  SEND_LIM_LOCKEDM   0x100

struct loadVectorStruct
{
  int hostNo;
  int *status;
  u_int seqNo;
  int checkSum;
  int flags;
  int numIndx;
  int numUsrIndx;
  float *li;
  int numResPairs;
  struct resPair *resPairs;
};

#define DETECTMODELTYPE 0

#define MAX_SRES_INDEX  2

struct masterReg
{
  char *clName;
  char *hostName;
  int flags;
  unsigned int seqNo;
  int checkSum;
  unsigned short portno;
  int licFlag;
  int maxResIndex;
  int *resBitArray;
};

struct resourceInstance
{
  char *resName;
  char *orignalValue;
  char *value;
  unsigned int nHosts;
  char padding1[4];
  time_t updateTime;
  struct hostNode *updHost;
  struct hostNode **hosts;
};
  


typedef struct sharedResourceInstance
{
  char *resName;
  unsigned int nHosts;
  char padding[4];
  struct hostNode **hosts;
  struct sharedResourceInstance *nextPtr;
} sharedResourceInstance;

struct minSLimConfData
{
  int defaultRunElim;
  unsigned int nClusAdmins;
  uid_t *clusAdminIds;
  char **clusAdminNames;
  float exchIntvl;
  float sampleIntvl;
  short hostInactivityLimit;
  short masterInactivityLimit;
  short retryLimit;
  short keepTime;
  struct resItem *allInfo_resTable;
  int allInfo_nRes;
  int allInfo_numIndx;
  int allInfo_numUsrIndx;
  u_short myCluster_checkSum;
  char *myCluster_eLimArgs;
  char *myHost_windows;
  int numMyhost_weekpair[8];
  windows_t *myHost_week[8];
  time_t myHost_wind_edge;
  float *myHost_busyThreshold;
  int myHost_rexPriority;
  int myHost_numInstances;
  struct resourceInstance **myHost_instances;
  struct sharedResourceInstance *sharedResHead;
};


struct sharedResourceInstance *sharedResourceHead;

#define  BYTE(byte)  (((int)byte)&0xff)
#define THRLDOK(inc,a,thrld)    (inc ? a <= thrld : a >= thrld)

int getpagesize (void);


int limSock = -1;
int limTcpSock = -1;
unsigned short lim_port;
unsigned short lim_tcp_port;
int probeTimeout = 2;
short resInactivityCount = 0;

struct clusterNode *myClusterPtr;
struct hostNode *myHostPtr;
int masterMe;
unsigned int nClusAdmins = 0;
uid_t *clusAdminIds = NULL;
gid_t *clusAdminGids = NULL;
char **clusAdminNames = NULL;

int kernelPerm;

struct limLock limLock;
char myClusterName[MAX_LSF_NAME_LEN];
u_int loadVecSeqNo = 0;
u_int masterAnnSeqNo = 0;
int lim_debug = 0;
int lim_CheckMode = 0;
int lim_CheckError = 0;
char *env_dir = NULL;
static int alarmed;
char ignDedicatedResource = FALSE;
u_short lsfSharedCkSum = 0;

pid_t pimPid = -1;
// static void startPIM (int, char **);

// static int initAndConfig (int, int *);
// static void term_handler (int);
// static void child_handler (int);
// static int processUDPMsg (void);
// static void doAcceptConn (void);
// static void initSignals (void);
// static void periodic (int);
// static struct tclLsInfo *getTclLsInfo (void);
// static void printTypeModel (void);
// static void initMiscLiStruct (void);
// static int getClusterConfig (void);

/* UDP message buffer.
 */
static char reqBuf[MSGSIZE];


#define LOOP_ADDR       0x7F000001


// struct config_param limParams[];
int lim_debug;
int lim_CheckMode;
int lim_CheckError;
int limSock;
int limTcpSock;
unsigned short lim_port;
unsigned short lim_tcp_port;
struct hostNode *myHostPtr;
char myClusterName[];
int masterMe;
float exchIntvl;
float sampleIntvl;
short hostInactivityLimit;
short masterInactivityLimit;
short resInactivityLimit;
short retryLimit;
short keepTime;
int probeTimeout;
short resInactivityCount;
char jobxfer;
short satper;
float *extraload;
uid_t *clusAdminIds;
gid_t *clusAdminGids;
char **clusAdminNames;
struct liStruct *li;
unsigned int li_len;
int defaultRunElim;
time_t lastSbdActiveTime;

char mustSendLoad;
hTab hostModelTbl;

char *env_dir;
struct hostNode **candidates;
u_int loadVecSeqNo;
u_int masterAnnSeqNo;
struct hostNode *fromHostPtr;
struct lsInfo allInfo;
struct shortLsInfo shortInfo;
int *clientHosts; // FIXME FIXME FIXME must manage dynamically
// struct floatClientInfo floatClientPool; // FIXME FIXME re-isntate when floating-client feature is implemented

pid_t elim_pid;
pid_t pimPid;

char ignDedicatedResource;
struct limLock limLock;

u_short lsfSharedCkSum;

int numMasterCandidates;
int isMasterCandidate;
int limConfReady;
int kernelPerm;

unsigned int numHostResources = 0;



int readShared (void);
int readCluster (int);
void reCheckRes (void);
int reCheckClass (void);
void setMyClusterName (void);
struct sharedResource *inHostResourcs (char *);
struct resourceInstance *isInHostList (struct sharedResource *, char *);
struct hostNode *initHostNode (void);
void freeHostNodes (struct hostNode *, int);
int validResource (const char *);
int validLoadIndex (const char *);
int validHostType (const char *);
int validHostModel (const char *);
char *stripIllegalChars (char *);
int initTypeModel (struct hostNode *);
char *getHostType (void);
struct hostNode *addFloatClientHost (struct hostent *);
int removeFloatClientHost (struct hostNode *);
void slaveOnlyInit (int checkMode, int *kernelPerm);
int slaveOnlyPreConf ();
int slaveOnlyPostConf (int checkMode, int *kernelPerm);
int typeNameToNo (const char *);
int archNameToNo (const char *);


void reconfigReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void shutdownReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void limDebugReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void lockReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
int limPortOk (struct sockaddr_in *);
void servAvailReq (XDR *, struct hostNode *, struct sockaddr_in *, struct LSFHeader *);

void pingReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void clusNameReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void masterInfoReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void cpufReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void clusInfoReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void masterRegister (XDR *, struct sockaddr_in *, struct LSFHeader *);
void jobxferReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void rcvConfInfo (XDR *, struct sockaddr_in *, struct LSFHeader *);
void tellMasterWho (XDR *, struct sockaddr_in *, struct LSFHeader *);
void whoIsMaster (struct clusterNode *);
void announceMaster (struct clusterNode *, char, char);
void wrongMaster (struct sockaddr_in *, char *, struct LSFHeader *, int);
void checkHostWd (void);
void announceMasterToHost (struct hostNode *, int);
int probeMasterTcp (struct clusterNode *);
void initNewMaster (void);
int callMasterTcp (enum limReqCode, struct hostNode *, void *, bool_t (*)(), void *, bool_t (*)(), int, int, struct LSFHeader *);
int validateHost (char *, int);
int validateHostbyAddr (struct sockaddr_in *, int);
void checkAfterHourWindow ();

void sendLoad (void);
void rcvLoad (XDR *, struct sockaddr_in *, struct LSFHeader *);
void copyIndices (float *, int, int, struct hostNode *);
float normalizeRq (float rawql, float cpuFactor, int nprocs);
struct resPair *getResPairs (struct hostNode *);
void satIndex (void);
void loadIndex (void);
void initReadLoad (int, int *);
void initConfInfo (void);
void readLoad (int);
const char *getHostModel (void);

void getLastActiveTime (void);
void putLastActiveTime (void);

void lim_Exit (const char *fname);
int equivHostAddr (struct hostNode *, u_int);
struct hostNode *findHost (char *);
struct hostNode *findHostbyAddr (struct sockaddr_in *, char *);
struct hostNode *findHostByAddr (in_addr_t);
struct hostNode *rmHost (struct hostNode *);
struct hostNode *findHostbyList (struct hostNode *, char *);
struct hostNode *findHostbyNo (struct hostNode *, int);
bool_t findHostInCluster (char *);
int definedSharedResource (struct hostNode *, struct lsInfo *);
struct shortLsInfo *shortLsInfoDup (struct shortLsInfo *);
void shortLsInfoDestroy (struct shortLsInfo *);
void errorBack (struct sockaddr_in *, struct LSFHeader *, enum limReplyCode, int);
int initSock (int);
void initLiStruct (void);
void placeReq (XDR *xdr, struct sockaddr_in *clientMap, struct LSFHeader *hdr, unsigned int chfd);
void loadadjReq (XDR *, struct sockaddr_in *, struct LSFHeader *, unsigned int s); // was int
void updExtraLoad (struct hostNode **, char *, int);
void loadReq (XDR *xdr, struct sockaddr_in *clientMap, struct LSFHeader *hdr, unsigned int chfd);
int getEligibleSites (register struct resVal *, struct decisionReq *, char, char *);
int validHosts (char **, int, char *, int);
int checkValues (struct resVal *, int);
void chkResReq (XDR *, struct sockaddr_in *, struct LSFHeader *);
void getTclHostData (struct tclHostData *, struct hostNode *, struct hostNode *, int);
void reconfig (void);
void shutdownLim (void);
int xdr_loadvector (XDR *, struct loadVectorStruct *, struct LSFHeader *);
int xdr_loadmatrix (XDR *, int, struct loadVectorStruct *, struct LSFHeader *);
int xdr_masterReg  (XDR *, struct masterReg *, struct LSFHeader *);
int xdr_statInfo   (XDR *, struct statInfo *, struct LSFHeader *);
void clientIO (struct Masks *);

/* openlava floating host management
 */
void addMigrantHost (XDR *, struct sockaddr_in *, struct LSFHeader *, unsigned int chan );
void rmMigrantHost (XDR *,  struct sockaddr_in *, struct LSFHeader *, unsigned int chan );
int logInit (void);
int logLIMStart (void);
int logLIMDown (void);
int logAddHost (struct hostEntry *);
int logRmHost (struct hostEntry *);
int addHostByTab (hTab *);
