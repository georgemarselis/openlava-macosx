/*
 * Copyright (C) 2011-2012 David Bigagli 
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of version 2 of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#pragma once

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>      // FIXME FIXME FIXME FIXME openflava needs its own error.h
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <tirpc/rpc/types.h>
#include <tirpc/rpc/xdr.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#if !defined(__CYGWIN__)
#include <rpcsvc/ypclnt.h>
#endif

// FIXME FIXME FIXME FIXME these have to go
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// typedef long size_t; // FIXME FIXME FIXME FIXME FIXME  untypedef, leave to compiler to deal with
// typedef unsigned long LS_UNS_LONG_INT; // FIXME FIXME FIXME FIXME FIXME untypedef, leave to compiler to deal with

// #define LS_LONG_FORMAT ("%ld") 
// #define _OPENLAVA_PROJECT_ "openlava project 2.0" // FIXME FIXME FIXME FIXME FIXME move to configure.ac
const char _OPENLAVA_PROJECT_[] = "openlava project 2.0";


/*
 * This is our identifier printed out by all daemons and commands.
 */
#ifdef REL_DATE  // FIXME FIXME FIXME FIXME FIXME move to configure.ac
#define LS_VERSION_ (_OPENLAVA_PROJECT_", " REL_DATE"\n")
#else
#define LS_VERSION_ (_OPENLAVA_PROJECT_", " __DATE__"\n")
#endif

/*
 * This is our current version.
 */
// #define OPENLAVA_VERSION   20  // FIXME FIXME FIXME FIXME FIXME move to configure.ac
const unsigned short OPENLAVA_VERSION = 20;

// #define LSF_DEFAULT_SOCKS       15
// #define MAX_LINE_LEN              512
// #define MAX_LSF_NAME_LEN           128
// #define MAXSRES                 32
// #define MAXRESDESLEN            256
// #define NBUILTINDEX             11
// #define MAX_TYPES                128
// #define MAX_MODELS               128
// #define MAX_TYPES_31             25
// #define MAX_MODELS_31            30

int lsferrno = 0; // defined, not used? 
const int ENOLOCATION = 0x0FFF; // defined, not used

enum LINE_BUFSIZ {
    LINE_BUFSIZ = 4096
};

enum LSF_CONSTANTS {
    LSF_DEFAULT_SOCKS   = 15,
    MAX_CHARLEN         = 20,
    MAX_CMD_DESC_LEN    = 256,
    MAX_FILENAME_LEN    = 4096,
    MAX_GROUPS          = 150,
    MAX_HPART_USERS     = 100,
    MAX_HOSTNAME_LEN    = 2048,
    MAX_JOB_DESP_LEN    = 1024,
    MAX_LINE_LEN        = 512,
    MAX_LSB_NAME_LEN    = 60,
    MAX_LSF_NAME_LEN    = 128,
    MAX_MODELS          = 128,
    MAX_MODELS_31       = 30,
    MAX_NRLIMITS        = 512,
    MAX_QUEUENAME_LEN   = 512,
    MAX_RESDES_LEN      = 256,
    MAX_SRES            = 32,
    MAX_TYPES           = 128,
    MAX_TYPES_31        = 25,
    MAX_USER_EQUIVALENT = 128,
    MAX_VERSION_LEN     = 12,
    MAX_VERSIONLEN      = 128,
    NBUILTINDEX         = 11,
    MAX_PATH_LEN        = PATH_MAX // <limits.h>
};

// #define FIRST_RES_SOCK  20
const unsigned int FIRST_RES_SOCK = 20;

// FIXME FIXME FIXME FIXME HAVE_UNION_WAIT must be moved to configure.ac
#ifdef HAVE_UNION_WAIT
    #define LS_WAIT_T      union wait
    #define LS_STATUS(s)   ((s).w_status)
#else
    #define LS_WAIT_INT
    #define LS_WAIT_T      int
    #define LS_STATUS(s)   (s)
#endif

typedef enum {
        R15S,
        R1M,
        R15M,
        UT,
        PG,
        IO,
        LS,
        IT,
        TMP,
        SWP,
        MEM,
        USR1,
        USR2
} lsindx_t;

#ifndef MAXFLOAT
    #define MAXFLOAT        3.40282347e+38F // FIXME FIXME FIXME FIXME turn into constant
#endif

// #define INFINIT_LOAD    (float) (0x7fffffff) // FIXME FIXME FIXME FIXME turn into constant

const double INFINIT_LOAD = 0x7fffffff;
#define INFINIT_FLOAT   (float) (0x7fffffff) // FIXME FIXME FIXME FIXME turn into constant

#define INFINIT_INT         0x7fffffff // FIXME FIXME FIXME FIXME turn into constant
#define INFINIT_LONG_INT    0x7fffffff // FIXME FIXME FIXME FIXME turn into constant

#define INFINIT_SHORT  0x7fff // FIXME FIXME FIXME FIXME  turn into constant

// #define DEFAULT_RLIMIT     -1
const short DEFAULT_RLIMIT = -1;

// #define LSF_RLIMIT_CPU      0
// #define LSF_RLIMIT_FSIZE    1
// #define LSF_RLIMIT_DATA     2
// #define LSF_RLIMIT_STACK    3
// #define LSF_RLIMIT_CORE     4
// #define LSF_RLIMIT_RSS      5
// #define LSF_RLIMIT_NOFILE   6
// #define LSF_RLIMIT_OPEN_MAX 7
// #define LSF_RLIMIT_VMEM     8
// #define LSF_RLIMIT_SWAP     LSF_RLIMIT_VMEM
// #define LSF_RLIMIT_RUN      9
// #define LSF_RLIMIT_PROCESS  10
// #define LSF_RLIM_NLIMITS    11

enum LSF_RLIMIT {
    LSF_RLIMIT_CPU = 0,
    LSF_RLIMIT_FSIZE,
    LSF_RLIMIT_DATA,
    LSF_RLIMIT_STACK,
    LSF_RLIMIT_CORE,
    LSF_RLIMIT_RSS,
    LSF_RLIMIT_NOFILE,
    LSF_RLIMIT_OPEN_MAX,
    LSF_RLIMIT_VMEM,
    LSF_RLIMIT_SWAP = LSF_RLIMIT_VMEM,
    LSF_RLIMIT_RUN,
    LSF_RLIMIT_PROCESS,
    LSF_RLIM_NLIMITS
};

// #define LSF_NULL_MODE    0
// #define LSF_LOCAL_MODE   1
// #define LSF_REMOTE_MODE  2

unsigned int LSF_NULL_MODE   = 0;
unsigned int LSF_LOCAL_MODE  = 1;
unsigned int LSF_REMOTE_MODE = 2;

// #define STATUS_TIMEOUT        125
// #define STATUS_IOERR          124
// #define STATUS_EXCESS         123
// #define STATUS_REX_NOMEM      122
// #define STATUS_REX_FATAL      121
// #define STATUS_REX_CWD        120
// #define STATUS_REX_PTY        119
// #define STATUS_REX_SP         118
// #define STATUS_REX_FORK       117
// #define STATUS_REX_TOK        116
// #define STATUS_REX_UNKNOWN    115
// #define STATUS_REX_NOVCL      114
// #define STATUS_REX_NOSYM      113
// #define STATUS_REX_VCL_INIT   112
// #define STATUS_REX_VCL_SPAWN  111
// #define STATUS_REX_EXEC       110
// #define STATUS_REX_MLS_INVAL  109
// #define STATUS_REX_MLS_CLEAR  108
// #define STATUS_REX_MLS_RHOST  107
// #define STATUS_REX_MLS_DOMIN  106

enum STATUS_REX {
    STATUS_REX_MLS_DOMIN = 106,
    STATUS_REX_MLS_RHOST = 107,
    STATUS_REX_MLS_CLEAR = 108,
    STATUS_REX_MLS_INVAL = 109,
    STATUS_REX_EXEC      = 110,
    STATUS_REX_VCL_SPAWN = 111,
    STATUS_REX_VCL_INIT  = 112,
    STATUS_REX_NOSYM     = 113,
    STATUS_REX_NOVCL     = 114,
    STATUS_REX_UNKNOWN   = 115,
    STATUS_REX_TOK       = 116,
    STATUS_REX_FORK      = 117,
    STATUS_REX_SP        = 118,
    STATUS_REX_PTY       = 119,
    STATUS_REX_CWD       = 120,
    STATUS_REX_FATAL     = 121,
    STATUS_REX_NOMEM     = 122,
    STATUS_EXCESS        = 123,
    STATUS_IOERR         = 124,
    STATUS_TIMEOUT       = 125
};

#define REX_FATAL_ERROR(s)     (((s) == STATUS_REX_NOVCL)               \
                                || ((s) == STATUS_REX_NOSYM)            \
                                || ((s) == STATUS_REX_NOMEM)            \
                                || ((s) == STATUS_REX_FATAL)            \
                                || ((s) == STATUS_REX_CWD)              \
                                || ((s) == STATUS_REX_PTY)              \
                                || ((s) == STATUS_REX_VCL_INIT)         \
                                || ((s) == STATUS_REX_VCL_SPAWN)        \
                                || ((s) == STATUS_REX_MLS_INVAL)        \
                                || ((s) == STATUS_REX_MLS_CLEAR)        \
                                || ((s) == STATUS_REX_MLS_RHOST)        \
                                || ((s) == STATUS_REX_MLS_DOMIN))

enum  REXF {
    REXF_USEPTY   = 0x00000001,
    REXF_CLNTDIR  = 0x00000002,
    REXF_TASKPORT = 0x00000004,
    REXF_SHMODE   = 0x00000008,
    REXF_TASKINFO = 0x00000010,
    REXF_REQVCL   = 0x00000020,
    REXF_SYNCNIOS = 0x00000040,
    REXF_TTYASYNC = 0x00000080,
    REXF_STDERR   = 0x00000100
} REXF;

enum STATUSES {
    EXACT = 0x01,
    OK_ONLY = 0x02,
    NORMALIZE = 0x04,
    LOCALITY = 0x08,
    IGNORE_RES = 0x10,
    LOCAL_ONLY = 0x20,
    DFT_FROMTYPE = 0x40,
    ALL_CLUSTERS = 0x80,
    EFFECTIVE = 0x100,
    RECV_FROM_CLUSTERS = 0x200,
    NEED_MY_CLUSTER_NAME = 0x400
} STATUSES;

#define SEND_TO_CLUSTERS   0x400

#define FROM_MASTER   0x01

#define KEEPUID       0x01

enum RES_CMD {
    RES_CMD_REBOOT   = 1,
    RES_CMD_SHUTDOWN = 2,
    RES_CMD_LOGON    = 3,
    RES_CMD_LOGOFF   = 4
} RES_CMD;

enum LIM_CMD {
    LIM_CMD_REBOOT   = 1,
    LIM_CMD_SHUTDOWN = 2
} LIM_CMD;

#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif

#ifndef LSFRU_FIELD_ADD
#define LSFRU_FIELD_ADD(a,b) \
{ \
    if ((a) < 0 || (b) < 0) { \
        (a) = MAX((a), (b)); \
    } else { \
        (a) += (b); \
    } \
}
#endif

enum LSFERRORS { // FIXME FIXME FIXME FIXME FIXME global enum with all the lserrno values in it
    ENOERROR,
    ENEGATIVERESULT,
    KEYNULL
} LSFERRORS ;

#define INTEGER_BITS       32
#define GET_INTNUM(i) ((i)/INTEGER_BITS + 1)

// #define LIM_UNAVAIL  0x00010000
// #define LIM_LOCKEDU  0x00020000
// #define LIM_LOCKEDW  0x00040000
// #define LIM_BUSY     0x00080000
// #define LIM_RESDOWN  0x00100000
// #define LIM_LOCKEDM  0x00200000
// #define LIM_OK_MASK  0x00bf0000
// #define LIM_SBDDOWN  0x00400000
enum LIM_ENUM {
    LIM_UNAVAIL = 0x00010000,
    LIM_LOCKEDU = 0x00020000,
    LIM_LOCKEDW = 0x00040000,
    LIM_BUSY    = 0x00080000,
    LIM_RESDOWN = 0x00100000,
    LIM_LOCKEDM = 0x00200000,
    LIM_OK_MASK = 0x00bf0000,
    LIM_SBDDOWN = 0x00400000
} LIM_ENUM;

#define LS_ISUNAVAIL(status)     (((status[0]) & LIM_UNAVAIL) != 0)

#define LS_ISBUSYON(status, index) (((status[1 + (index)/INTEGER_BITS]) & (1 << (index)%INTEGER_BITS)) != 0)

#define LS_ISBUSY(status) (((status[0]) & LIM_BUSY) != 0)

#define LS_ISLOCKEDU(status)   (((status[0]) & LIM_LOCKEDU) != 0)

#define LS_ISLOCKEDW(status)   (((status[0]) & LIM_LOCKEDW) != 0)

#define LS_ISLOCKEDM(status)   (((status[0]) & LIM_LOCKEDM) != 0)

#define LS_ISLOCKED(status)    (((status[0]) & (LIM_LOCKEDU | LIM_LOCKEDW | LIM_LOCKEDM)) != 0)

#define LS_ISRESDOWN(status)   (((status[0]) & LIM_RESDOWN) != 0)

#define LS_ISSBDDOWN(status)   (((status[0]) & LIM_SBDDOWN) != 0)

#define LS_ISOK(status)  ((status[0] & LIM_OK_MASK) == 0)


#define LS_ISOKNRES(status) (((status[0]) & ~(LIM_RESDOWN | LIM_SBDDOWN)) == 0)

struct placeInfo
{
    int numtask;
    char padding[4];
    char *hostName;
};

struct hostLoad
{
    char *hostName;
    float *status;
    float *loadIndex;
};

enum valueType
{
    LS_BOOLEAN, LS_NUMERIC, LS_STRING, LS_EXTERNAL
};

// #define BOOLEAN  LS_BOOLEAN
// #define NUMERIC  LS_NUMERIC
// #define STRING   LS_STRING
// #define EXTERNAL LS_EXTERNAL

enum orderType
{
    INCR, DECR, NA
};

enum RESF {

// copied from https://www.ibm.com/support/knowledgecenter/en/SSWRJV_10.1.0/api_reference/group__defs__ls__info.html
    RESF_BUILTIN                = 0x01,
    RESF_DYNAMIC                = 0x02, // ability of this resource to be dynamic or not
    RESF_GLOBAL                 = 0x04,
    RESF_SHARED                 = 0x08,
    RESF_LIC                    = 0x10,
    RESF_EXTERNAL               = 0x20,
    RESF_RELEASE                = 0x40,
    RESF_DEFINED_IN_RESOURCEMAP = 0x80,
    RESF_NON_CONSUMABLE         = 0x100,
    RESF_REDEFINABLE            = 0x200,
    RESF_ESRES                  = 0x400,
    RESF_MEGA_CRITERIA          = 0x800,
    RESF_APPEARS_IN_RESOURCEMAP = 0x8000,

// original
    // RESF_BUILTIN                = 0x01,
    // RESF_DYNAMIC                = 0x02, // ability of this resource to be dynamic or not
    // RESF_GLOBAL                 = 0x04,
    // RESF_SHARED                 = 0x08,
    // RESF_EXTERNAL               = 0x10,
    // RESF_RELEASE                = 0x20,
    // RESF_DEFINED_IN_RESOURCEMAP = 0x40,
    RESF_KEYNULL                = 0xFFFF
};

struct resItem
{
    enum valueType valueType;
    enum orderType orderType;
    int flags;
    unsigned int interval;
    const char *name;
    const char *des;

};

struct lsInfo
{
    unsigned int nTypes;
    unsigned int nModels;
    unsigned int modelRefs[MAX_MODELS];         // FIXME FIXME FIXME FIXME fix sizes; add union that describes each type
    unsigned int nRes;
    unsigned int numIndx;
    unsigned int numUsrIndx;
    char hostTypes[MAX_TYPES][MAX_LSF_NAME_LEN];   // FIXME FIXME FIXME FIXME fix sizes; add union that describes each type
    char hostModels[MAX_MODELS][MAX_LSF_NAME_LEN]; // FIXME FIXME FIXME FIXME fix sizes; add union that describes each type
    char hostArchs[MAX_MODELS][MAX_LSF_NAME_LEN];  // FIXME FIXME FIXME FIXME fix sizes; add union that describes each type
    char paddin1[4];
    double cpuFactor[MAX_MODELS];               // FIXME FIXME FIXME FIXME fix sizes; add union that describes each type
    struct resItem **resTable;
};

#define CLUST_STAT_OK             0x01
#define CLUST_STAT_UNAVAIL        0x02

struct clusterInfo
{
    uid_t *adminIds;
    uid_t managerId;
    unsigned int status;
    unsigned int nTypes;
    unsigned int nModels;
    unsigned int numServers;
    unsigned int numClients;
    unsigned int nRes;
    unsigned int nAdmins;
    char **resources;
    char **hostTypes;
    char **hostModels;
    char **admins;
    char *clusterName;
    char *masterName;
    char *managerName;

};

struct hostInfo
{
    int rexPriority;
    unsigned int maxCpus;
    unsigned int maxMem;
    unsigned int maxSwap;
    unsigned int maxTmp;
    unsigned int nDisks;
    unsigned int nRes;
    char isServer;
    char padding1[3];
    char *hostModel;
    char *windows;
    char *hostType;
    double cpuFactor;
    double *busyThreshold;
    unsigned long numIndx;
    char **resources;
    // char hostName[MAXHOSTNAMELEN];
    char *hostName;
};

    

/*
 * This data structure is built when reading the lsf.cluster file and
 * building the host list, it is also used to add add host at runtime.
 */
struct hostEntry
{
    int rexPriority;
    int rcv;
    unsigned int numIndx;
    unsigned int nDisks;
    unsigned int nRes;
    char   padding1[4];
    double cpuFactor;
    double *busyThreshold;   
    const char *hostName; // [MAXHOSTNAMELEN];
    const char *hostModel; // [MAX_LSF_NAME_LEN];
    const char *hostType;  // [MAX_LSF_NAME_LEN];
    const char *window;
    char **resList;
};


// FIXME FIXME FIXME FIXME this struct needs to be profiled for issues
//      with time
struct lsfRusage
{
    struct timeval ru_utime; /* user CPU time used */
    struct timeval ru_stime; /* system CPU time used */
    // time_t ru_utime;
    // suseconds_t ru_stime;
    char   padding[8];
    long   ru_maxrss;
    time_t ru_ixrss;
    double ru_ismrss;
    long   ru_idrss;
    long   ru_isrss;
    long   ru_minflt;
    long   ru_majflt;
    long   ru_nswap;
    long   ru_inblock;
    long   ru_oublock;
    double ru_ioch;
    long   ru_msgsnd;
    long   ru_msgrcv;
    long   ru_nsignals;
    long   ru_nvcsw;
    long   ru_nivcsw;
    double ru_exutime;
};

struct lsfAcctRec
{
    int pid;
    int exitStatus;
    time_t dispTime;
    time_t termTime;
    char *username;
    char *fromHost;
    char *execHost;
    char *cwd;
    char *cmdln;
    struct lsfRusage lsfRu;
};


// #define NIO_STDIN_ON                        0x01
// #define NIO_STDIN_OFF                       0x02
// #define NIO_TAGSTDOUT_ON                    0x03
// #define NIO_TAGSTDOUT_OFF                   0x04
enum NIO_STREAMS {
    NIO_STDIN_ON      = 0x01,
    NIO_STDIN_OFF     = 0x02,
    NIO_TAGSTDOUT_ON  = 0x03,
    NIO_TAGSTDOUT_OFF = 0x04
};

// #define NIO_TASK_STDINON                    0x01
// #define NIO_TASK_STDINOFF                   0x02
// #define NIO_TASK_ALL                        0x03
// #define NIO_TASK_CONNECTED                  0x04
enum NIO_TASK {
    NIO_TASK_STDINON   = 0x01,
    NIO_TASK_STDINOFF  = 0x02,
    NIO_TASK_ALL       = 0x03,
    NIO_TASK_CONNECTED = 0x04,
};

enum nioType
{
    NIO_STATUS,
    NIO_STDOUT,
    NIO_EOF,
    NIO_IOERR,
    NIO_REQUEUE,
    NIO_STDERR
};

struct nioEvent
{
    int tid;
    enum nioType type;
    int status;
};

struct nioInfo
{
    int num;
    char padding[4];
    struct nioEvent *ioTask;
};

struct confNode
{
    char tag;
    char padding[7];
    char *cond;
    char **lines;
    unsigned long beginLineNum;
    unsigned long numLines;
    struct confNode *leftPtr;
    struct confNode *rightPtr;
    struct confNode *fwPtr;
};

struct pStack
{
    ssize_t top;
    size_t size;
    struct confNode **nodes;
};

struct confHandle
{
    char *fname;
    size_t lineCount;
    struct confNode *curNode;
    struct confNode *rootNode;
    struct pStack *ptrStack;
};

struct lsConf
{
    size_t numConds;
    int *values;
    char **conds;
    struct confHandle *confhandle;
 
};

struct sharedConf
{
    struct lsInfo *lsinfo;
    char *clusterName;
    char *servers;
};

// typedef 
struct lsSharedResourceInstance
{
    char **hostList;
    char *value;
    unsigned long nHosts;
};// LS_SHARED_RESOURCE_INST_T;

// typedef
struct lsSharedResourceInfo
{
    unsigned int nInstances;
    char padding[4];
    char *resourceName;
    struct lsSharedResourceInstance *instances;
}; // LS_SHARED_RESOURCE_INFO_T;

struct clusterConf {
    unsigned int numShareRes;
    unsigned int numHosts;
    struct lsSharedResourceInfo *shareRes;
    struct hostInfo *hosts;
    struct clusterInfo *clinfo;
};

/*
 * Maximum number of processes reported by PIM and read by the PIM library.
 */
// #define MAX_PROC_ENT (2 * 1024)           // FIXME FIXME FIXME FIXME FIXME set by configuration
enum MAX_PROC_ENT {
    MAX_PROC_ENT = 2 * 1024
};  // FIXME FIXME FIXME FIXME FIXME set by configuration

struct pidInfo
{
    size_t jobid;
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    char padding[4];
};

struct jRusage
{
    size_t mem;
    size_t swap;
    time_t utime;
    time_t stime;
    pid_t npids;
    pid_t npgids;
    pid_t *pgid; // FIXME FIXME FIXME FIXME maybe wrong
    struct pidInfo *pidInfo;
};

typedef enum
{
    EV_LIM_START,
    EV_LIM_SHUTDOWN,
    EV_ADD_HOST,
    EV_REMOVE_HOST,
    EV_EVENT_LAST
} event_t;

/*
 * openlava LIM events
 */
struct lsEventRec
{
    event_t event;
    uint16_t version; // FIXME um, why 16-bit int? 
    char padding[2];
    time_t etime;
    void *record;
};

/*
 * This is the log of the hostEntry structure. It is another structure
 * because it is used in the context of event logging and it can evolve in
 * different directions then the base configuration structure which is
 * hostEntry.
 */
struct hostEntryLog
{
    unsigned int nDisks;
    unsigned int numIndx;
    unsigned int nRes;
    int   rcv;
    int   rexPriority;
    char   padding[4];
    char  *hostName;  // [MAXHOSTNAMELEN];
    char  *hostModel; // [MAX_LSF_NAME_LEN];
    char  *hostType;  // [MAX_LSF_NAME_LEN];
    char  *window;
    double cpuFactor;
    double *busyThreshold;
    char  **resList;
};

/*
 * openlava error numbers
 */
enum OPENLAVA_ERROR_NUMBERS { // FIXME FIXME FIXME FIXME this needs to be compatible with LSF
    LSE_NO_ERR         = 0,
    LSE_BAD_XDR,
    LSE_MSG_SYS,
    LSE_BAD_ARGS,
    LSE_MASTR_UNKNW,
    LSE_LIM_DOWN,
    LSE_PROTOC_LIM,
    LSE_SOCK_SYS,
    LSE_ACCEPT_SYS,
    LSE_BAD_TASKF,
    LSE_NO_HOST,
    LSE_NO_ELHOST,
    LSE_TIME_OUT,
    LSE_NIOS_DOWN,
    LSE_LIM_DENIED,
    LSE_LIM_IGNORE,
    LSE_LIM_BADHOST,
    LSE_LIM_ALOCKED,
    LSE_LIM_NLOCKED,
    LSE_LIM_BADMOD,
    LSE_SIG_SYS,
    LSE_BAD_EXP,
    LSE_NORCHILD,
    LSE_MALLOC,
    LSE_LSFCONF,
    LSE_BAD_ENV,
    LSE_LIM_NREG,
    LSE_RES_NREG,
    LSE_RES_NOMORECONN,
    LSE_BADUSER,
    LSE_RES_ROOTSECURE,
    LSE_RES_DENIED,
    LSE_BAD_OPCODE,
    LSE_PROTOC_RES,
    LSE_RES_CALLBACK,
    LSE_RES_NOMEM,
    LSE_RES_FATAL,
    LSE_RES_PTY,
    LSE_RES_SOCK,
    LSE_RES_FORK,
    LSE_NOMORE_SOCK,
    LSE_WDIR,
    LSE_LOSTCON,
    LSE_RES_INVCHILD,
    LSE_RES_KILL,
    LSE_PTYMODE,
    LSE_BAD_HOST,
    LSE_PROTOC_NIOS,
    LSE_WAIT_SYS,
    LSE_SETPARAM,
    LSE_RPIDLISTLEN,
    LSE_BAD_CLUSTER,
    LSE_RES_VERSION,
    LSE_EXECV_SYS,
    LSE_RES_DIR,
    LSE_RES_DIRW,
    LSE_BAD_SERVID,
    LSE_NLSF_HOST,
    LSE_UNKWN_RESNAME,
    LSE_UNKWN_RESVALUE,
    LSE_TASKEXIST,
    LSE_BAD_TID,
    LSE_TOOMANYTASK,
    LSE_LIMIT_SYS,
    LSE_BAD_NAMELIST,
    LSE_LIM_NOMEM,
    LSE_NIO_INIT,
    LSE_CONF_SYNTAX,
    LSE_FILE_SYS,
    LSE_CONN_SYS,
    LSE_SELECT_SYS,
    LSE_EOF,
    LSE_ACCT_FORMAT,
    LSE_BAD_TIME,
    LSE_FORK,
    LSE_PIPE,
    LSE_ESUB,
    LSE_EAUTH,
    LSE_NO_FILE,
    LSE_NO_CHAN,
    LSE_BAD_CHAN,
    LSE_INTERNAL,
    LSE_PROTOCOL,
    LSE_MISC_SYS,
    LSE_RES_RUSAGE,
    LSE_NO_RESOURCE,
    LSE_BAD_RESOURCE,
    LSE_RES_PARENT,
    LSE_I18N_SETLC,
    LSE_I18N_CATOPEN,
    LSE_I18N_NOMEM,
    LSE_NO_MEM,
    LSE_FILE_CLOSE,
    LSE_MASTER_LIM_DOWN,
    LSE_MLS_INVALID,
    LSE_MLS_CLEARANCE,
    LSE_MLS_RHOST,
    LSE_MLS_DOMINATE,
    LSE_HOST_EXIST,
    LSE_NERR
};

#define LSE_ISBAD_RESREQ(s)     (((s) == LSE_BAD_EXP)                   \
                                 || ((s) == LSE_UNKWN_RESNAME)          \
                                 || ((s) == LSE_UNKWN_RESVALUE))

#define LSE_SYSCALL(s)          (((s) == LSE_SELECT_SYS)        \
                                 || ((s) == LSE_CONN_SYS)       \
                                 || ((s) == LSE_FILE_SYS)       \
                                 || ((s) == LSE_MSG_SYS)        \
                                 || ((s) == LSE_SOCK_SYS)       \
                                 || ((s) == LSE_ACCEPT_SYS)     \
                                 || ((s) == LSE_SIG_SYS)        \
                                 || ((s) == LSE_WAIT_SYS)       \
                                 || ((s) == LSE_EXECV_SYS)      \
                                 || ((s) == LSE_LIMIT_SYS)      \
                                 || ((s) == LSE_PIPE)           \
                                 || ((s) == LSE_ESUB)           \
                                 || ((s) == LSE_MISC_SYS))
/*
#define TIMEIT(level,func,name)                                         \
    { if (timinglevel > level) {                                        \
            struct timeval before, after;                               \
            struct timezone tz;                                         \
            gettimeofday(&before, &tz);                                 \
            func;                                                       \
            gettimeofday(&after, &tz);                                  \
            ls_syslog(LOG_INFO,"L%d %s %d ms",level,name,               \
                        (time_t)((after.tv_sec - before.tv_sec)*1000 +       \
                            (after.tv_usec-before.tv_usec)/1000));      \
        } else                                                          \
            func;                                                       \
    }

*/

/*
#define TIMEVAL(level,func,val)                                 \
    { if (timinglevel > level) {                                \
            struct timeval before, after;                       \
            struct timezone tz;                                 \
            gettimeofday(&before, &tz);                         \
            func;                                               \
            gettimeofday(&after, &tz);                          \
            val = (int)((after.tv_sec - before.tv_sec)*1000 +   \
                        (after.tv_usec-before.tv_usec)/1000);   \
        } else {                                                \
            func;                                               \
            val = 0;                                            \
        }                                                       \
    }

*/

// #define LC_SCHED    0x00000001
// #define LC_EXEC     0x00000002
// #define LC_TRACE    0x00000004
// #define LC_COMM     0x00000008
// #define LC_XDR      0x00000010
// #define LC_CHKPNT   0x00000020
// #define LC_FILE     0x00000080
// #define LC_AUTH     0x00000200
// #define LC_HANG     0x00000400
// #define LC_SIGNAL   0x00001000
// #define LC_PIM      0x00004000
// #define LC_SYS      0x00008000
// #define LC_JLIMIT   0x00010000
// #define LC_PEND     0x00080000
// #define LC_LOADINDX 0x00200000
// #define LC_JGRP     0x00400000
// #define LC_JARRAY   0x00800000
// #define LC_MPI      0x01000000
// #define LC_ELIM     0x02000000
// #define LC_M_LOG    0x04000000
// #define LC_PERFM    0x08000000

enum LC {
    LC_SCHED    = 0x00000001,
    LC_EXEC     = 0x00000002,
    LC_TRACE    = 0x00000004,
    LC_COMM     = 0x00000008,
    LC_XDR      = 0x00000010,
    LC_CHKPNT   = 0x00000020,
    LC_FILE     = 0x00000080,
    LC_AUTH     = 0x00000200,
    LC_HANG     = 0x00000400,
    LC_SIGNAL   = 0x00001000,
    LC_PIM      = 0x00004000,
    LC_SYS      = 0x00008000,
    LC_JLIMIT   = 0x00010000,
    LC_PEND     = 0x00080000,
    LC_LOADINDX = 0x00200000,
    LC_JGRP     = 0x00400000,
    LC_JARRAY   = 0x00800000,
    LC_MPI      = 0x01000000,
    LC_ELIM     = 0x02000000,
    LC_M_LOG    = 0x04000000,
    LC_PERFM    = 0x08000000,
    LC_KEYNULL  = 0xFFFFFFF
};

// #define LOG_DEBUG1  LOG_DEBUG + 1
// #define LOG_DEBUG2  LOG_DEBUG + 2
// #define LOG_DEBUG3  LOG_DEBUG + 3

enum LOGDEBUG {
    LOG_DEBUG1 = LOG_DEBUG + 1,
    LOG_DEBUG2,
    LOG_DEBUG3,
    LOG_DEBUG_KEYNULL
};

// #define LSF_NIOS_REQUEUE        127
enum LSF_NIOS_REQUEUE {
    LSF_NIOS_REQUEUE = 127
};

typedef void (*SIGFUNCTYPE) (int);

// #ifndef MSGSIZE
// #define MSGSIZE   8192
// #endif
enum MSGSIZE {
    MSGSIZE = 8192
};

#ifdef __CYGWIN__
#define NICE_LEAST -19
#else
#define NICE_LEAST -40
#endif
#define NICE_MIDDLE 20

#ifndef WCOREDUMP
#ifdef LS_WAIT_INT
#define WCOREDUMP(x)    ((x) & 0200)
#else
#define WCOREDUMP(x) (x).w_coredump
#endif
#endif

#define BSD_NICE                                        // FIXME FIXME FIXME FIXME FIXME set in configure.ac

// typedef struct stat LS_STAT_T;
#define LSTMPDIR        LSTMPDIR                       // FIXME FIXME FIXME FIXME FIXME set in configure.ac
// #define LSDEVNULL       "/dev/null"
const char LSDEVNULL[] = "/dev/null";                   // FIXME FIXME FIXME FIXME FIXME set in configure.ac
// #define LSETCDIR        "/fix/me/no/such/place"
const char LSETCDIR[] = "/fix/me/no/such/place";        // FIXME FIXME FIXME FIXME FIXME set in configure.ac

// #define LSETCDIR           SYSCONFDIR
#define SOCK_CALL_FAIL(c)  ((c) < 0 )
#define SOCK_INVALID(c)    ((c) < 0 )
// #define SOCK_READ_FIX      b_read_fix
// #define SOCK_WRITE_FIX     b_write_fix
// #define NB_SOCK_READ_FIX   nb_read_fix
// #define NB_SOCK_WRITE_FIX  nb_write_fix

// #define LSF_NSIG NSIG                                   // FIXME FIXME FIXME FIXME FIXME set in configure.ac
struct extResInfo
{
    char *name;
    char *type;
    char *interval;
    char *increasing;
    char *des; 
};

#ifdef __CYGWIN__
int optind;
char *optarg;
int opterr;
int optopt;
#endif

int lserrno;
int masterLimDown;
// int ls_nerr;           // FIXME FIXME FIXME FIXME FIXME too many different types of lserrno
// char *ls_errmsg[];  // FIXME FIXME FIXME FIXME FIXME put in specific header
int logclass;
int timinglevel;

// int lsf_lim_version; // FIXME FIXME FIXME FIXME FIXME set in configure.ac
// int lsf_res_version; // FIXME FIXME FIXME FIXME FIXME set in configure.ac
unsigned int LSF_LIM_VERSION; // FIXME FIXME FIXME FIXME FIXME set in configure.ac
unsigned int LSF_RES_VERSION; // FIXME FIXME FIXME FIXME FIXME set in configure.ac

// int ls_initrex (int, int);
// int ls_connect (char *);
// int ls_rexecv (char *, char **, int);
// int ls_rexecve (char *, char **, int, char **);
// int ls_rtask (char *, char **, int);
// int ls_rtaske (char *, char **, int, char **);
// int ls_rwait (LS_WAIT_T *, int, struct rusage *);
// int ls_rwaittid (int, LS_WAIT_T *, int, struct rusage *);
// int lls_rkill (unsigned int rtid, int sig);
// int ls_startserver (char *, char **, int);
// int ls_conntaskport (int);
// char **ls_placereq              (char *resreq, size_t *numhosts, int options, char *fromhost);
// char **ls_placeofhosts          (char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize);
// char **ls_placeoftype           (char *resreq, size_t *numhosts, int options, char *fromhost, char *hosttype);
// struct hostLoad *ls_load        (char *resreq, size_t *numhosts, int options, char *fromhost);
// struct hostLoad *ls_loadofhosts (char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize);
// struct hostLoad *ls_loadoftype  (char *resreq, size_t *numhosts, int options, char *fromhost, char *hosttype);
// struct hostLoad *ls_loadinfo    (char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize, char ***indxnamelist);
// int ls_loadadj  (char *resreq, struct placeInfo *hostlist, size_t listsize);

// char **ls_findmyconnections (void);
// int ls_lostconnection (void);
// char *ls_getclustername (void);
// struct clusterInfo *ls_clusterinfo (char *resReq, unsigned int *numclusters, char **clusterList, int listsize, int options);
// struct lsSharedResourceInfo *ls_sharedresourceinfo (char **resources, unsigned int *numResources, char *hostName, int options);
// char *ls_getmastername (void);
// char *ls_getmastername2 (void);
// struct hostInfo *ls_gethostinfo (char *resReq, size_t *numhosts, char **hostlist, size_t listsize, int options);
// char *ls_getISVmode (void);

// struct lsInfo *ls_info (void);

// char **ls_indexnames (struct lsInfo *);
// int ls_isclustername (char *);
// char *ls_gethosttype (char *hostname);
// float *ls_getmodelfactor (char *modelname);
// float *ls_gethostfactor (char *hostname);
// char *ls_gethostmodel (char *hostname);
// int ls_lockhost (time_t duration);
// int ls_unlockhost (void);
// int ls_limcontrol (char *hostname, int opCode);
// void ls_remtty (int ind, int enableIntSus);
// void ls_loctty (int ind);
// char *ls_sysmsg (void);
// void ls_perror (char *usrMsg);


// struct lsConf *ls_getconf (char *);
// void ls_freeconf (struct lsConf *);
// struct clusterConf *ls_readcluster (char *, struct lsInfo *);
// struct clusterConf *ls_readcluster_ex (char *, struct lsInfo *, int);

// int ls_initdebug (char *appName);
// void ls_syslog (int level, const char *fmt, ...);
// void ls_verrlog (FILE * fp, const char *fmt, va_list ap);

// int ls_rescontrol (char *host, int opcode, int options);
// int ls_stdinmode (int onoff);
// int ls_stoprex (void);
// int ls_donerex (void);
// int ls_rsetenv (char *host, char **env);
// int ls_rsetenv_async (char *host, char **env);
// int ls_setstdout (int on, char *format);
// int ls_niossync (long numTasks);
// int ls_setstdin (int on, int *rpidlist, size_t len);
// // int ls_chdir (char *, char *);
// char *ls_getmnthost (char *fn);
// // int ls_servavail (int, int);
// int ls_setpriority (int newPriority);

// int ls_ropen (char *host, char *fn, int flags, int mode);
// int ls_rclose (int rfd);
// int ls_rwrite (int rfd, char *buf, size_t len);
// int ls_rread (int rfd, char *buf, size_t len);
// off_t ls_rlseek (int rfd, off_t offset, int whence);
// int ls_runlink (char *host, char *fn);
// int ls_rfstat (int rfd, struct stat *buf);
// int ls_rstat (char *host, char *fn, struct stat *buf);
// char *ls_rgetmnthost (char *host, char *fn);
// int ls_rfcontrol (int command, int arg);
// int ls_rfterminate (char *host);

// void ls_ruunix2lsf (struct rusage *rusage, struct lsfRusage *lsfRusage);
// void ls_rulsf2unix (struct lsfRusage *lsfRusage, struct rusage *rusage);
// void cleanLsfRusage (struct lsfRusage *);
// void cleanRusage (struct rusage *);

// struct resLogRecord *ls_readrexlog (FILE *);
// int ls_nioinit (int sock);
// int ls_nioselect (int, fd_set *, fd_set *, fd_set *, struct nioInfo **, struct timeval *);
// int ls_nioctl (int, int);
// int ls_nionewtask (int, int);
// int ls_nioremovetask (int);
// int ls_niowrite (char *, int);
// int ls_nioclose (void);
// int ls_nioread (int, char *, int);
// int ls_niotasks (int, int *, int);
// int ls_niostatus (int, int *, struct rusage *);
// int ls_niokill (int);
// // int ls_niosetdebug (int);
// int ls_niodump (int, int, int, char *);
// struct lsfAcctRec *ls_getacctrec (FILE *, int *);
// int ls_putacctrec (FILE *, struct lsfAcctRec *);
// int getBEtime (char *, char, time_t *);


/*
 * openlava add host
 */
// int ls_addhost (struct hostEntry *);
// int ls_rmhost (const char *);

// end lsf_h_
