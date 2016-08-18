/*
 * Copyright (C) 2011-2012 David Bigagli Copyright (C) 2007 Platform
 * Computing Inc
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
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
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

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef long LS_LONG_INT; // FIXME FIXME FIXME FIXME FIXME  untypedef, leave to compiler to deal with
typedef unsigned long LS_UNS_LONG_INT; // FIXME FIXME FIXME FIXME FIXME untypedef, leave to compiler to deal with

// #define LS_LONG_FORMAT ("%ld") // FIXME FIXME FIXME FIXME 
#define _OPENLAVA_PROJECT_ "openlava project 2.0" // FIXME FIXME FIXME FIXME FIXME move to configure.ac

/*
 * This is our identifier printed out by all daemons and commands.
 */
#ifdef REL_DATE  // FIXME FIXME FIXME FIXME FIXME move to configure.ac
#define _LS_VERSION_ (_OPENLAVA_PROJECT_", " REL_DATE"\n")
#else
#define _LS_VERSION_ (_OPENLAVA_PROJECT_", " __DATE__"\n")
#endif

/*
 * This is our current version.
 */
#define OPENLAVA_VERSION   20  // FIXME FIXME FIXME FIXME FIXME move to configure.ac

#define LSF_DEFAULT_SOCKS       15
#define MAXLINELEN              512
#define MAXLSFNAMELEN           128
#define MAXSRES                 32
#define MAXRESDESLEN            256
#define NBUILTINDEX             11
#define MAXTYPES                128
#define MAXMODELS               128
#define MAXTYPES_31             25
#define MAXMODELS_31            30
//#define MAXFILENAMELEN          4096
const uint MAXFILENAMELEN = 4096;   // FIXME FIXME FIXME FIXME value of MAXFILENAMELEN must be set by configure script

// #define FIRST_RES_SOCK  20
static const uint FIRST_RES_SOCK = 20;

#ifdef HAVE_UNION_WAIT
  #define LS_WAIT_T      union wait
  #define LS_STATUS(s)   ((s).w_status)
#else
  #define LS_WAIT_INT
  #define LS_WAIT_T      int
  #define LS_STATUS(s)   (s)
#endif

typedef enum
{
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
  #define MAXFLOAT        3.40282347e+38F
#endif

#define INFINIT_LOAD    (float) (0x7fffffff)
#define INFINIT_FLOAT   (float) (0x7fffffff)

#define INFINIT_INT    0x7fffffff
#define INFINIT_LONG_INT    0x7fffffff

#define INFINIT_SHORT  0x7fff

#define DEFAULT_RLIMIT     -1

#define LSF_RLIMIT_CPU      0
#define LSF_RLIMIT_FSIZE    1
#define LSF_RLIMIT_DATA     2
#define LSF_RLIMIT_STACK    3
#define LSF_RLIMIT_CORE     4
#define LSF_RLIMIT_RSS      5
#define LSF_RLIMIT_NOFILE   6
#define LSF_RLIMIT_OPEN_MAX 7
#define LSF_RLIMIT_VMEM     8
#define LSF_RLIMIT_SWAP     LSF_RLIMIT_VMEM
#define LSF_RLIMIT_RUN      9
#define LSF_RLIMIT_PROCESS  10
#define LSF_RLIM_NLIMITS    11


#define LSF_NULL_MODE    0
#define LSF_LOCAL_MODE   1
#define LSF_REMOTE_MODE  2

#define RF_MAXHOSTS 5

#define RF_CMD_MAXHOSTS 0
#define RF_CMD_RXFLAGS 2


#define STATUS_TIMEOUT        125
#define STATUS_IOERR          124
#define STATUS_EXCESS         123
#define STATUS_REX_NOMEM      122
#define STATUS_REX_FATAL      121
#define STATUS_REX_CWD        120
#define STATUS_REX_PTY        119
#define STATUS_REX_SP         118
#define STATUS_REX_FORK       117
#define STATUS_REX_TOK        116
#define STATUS_REX_UNKNOWN    115
#define STATUS_REX_NOVCL      114
#define STATUS_REX_NOSYM      113
#define STATUS_REX_VCL_INIT   112
#define STATUS_REX_VCL_SPAWN  111
#define STATUS_REX_EXEC       110
#define STATUS_REX_MLS_INVAL  109
#define STATUS_REX_MLS_CLEAR  108
#define STATUS_REX_MLS_RHOST  107
#define STATUS_REX_MLS_DOMIN  106

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

#define   REXF_USEPTY   0x00000001
#define   REXF_CLNTDIR  0x00000002
#define   REXF_TASKPORT 0x00000004
#define   REXF_SHMODE   0x00000008
#define   REXF_TASKINFO 0x00000010
#define   REXF_REQVCL   0x00000020
#define   REXF_SYNCNIOS 0x00000040
#define   REXF_TTYASYNC 0x00000080
#define   REXF_STDERR   0x00000100

#define EXACT         0x01
#define OK_ONLY       0x02
#define NORMALIZE     0x04
#define LOCALITY      0x08
#define IGNORE_RES    0x10
#define LOCAL_ONLY    0x20
#define DFT_FROMTYPE  0x40
#define ALL_CLUSTERS  0x80
#define EFFECTIVE     0x100
#define RECV_FROM_CLUSTERS 0x200
#define NEED_MY_CLUSTER_NAME 0x400

#define SEND_TO_CLUSTERS   0x400

#define FROM_MASTER   0x01

#define KEEPUID       0x01

#define RES_CMD_REBOOT          1
#define RES_CMD_SHUTDOWN        2
#define RES_CMD_LOGON           3
#define RES_CMD_LOGOFF          4

#define LIM_CMD_REBOOT          1
#define LIM_CMD_SHUTDOWN        2

#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif

struct connectEnt
{
  char *hostname;
  int csock[2];
};

#define INTEGER_BITS       32
#define GET_INTNUM(i) ((i)/INTEGER_BITS + 1)

#define LIM_UNAVAIL  0x00010000
#define LIM_LOCKEDU  0x00020000
#define LIM_LOCKEDW  0x00040000
#define LIM_BUSY     0x00080000
#define LIM_RESDOWN  0x00100000
#define LIM_LOCKEDM  0x00200000
#define LIM_OK_MASK  0x00bf0000
#define LIM_SBDDOWN  0x00400000

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
  char hostName[MAXHOSTNAMELEN];
  int numtask;
};

struct hostLoad
{
    char hostName[MAXHOSTNAMELEN];
    int *status;
    float *loadIndex;
};

enum valueType
{
  LS_BOOLEAN, LS_NUMERIC, LS_STRING, LS_EXTERNAL
};

#define BOOLEAN  LS_BOOLEAN
#define NUMERIC  LS_NUMERIC
#define STRING   LS_STRING
#define EXTERNAL LS_EXTERNAL

enum orderType
{
  INCR, DECR, NA
};

#define RESF_BUILTIN     0x01
#define RESF_DYNAMIC     0x02
#define RESF_GLOBAL      0x04
#define RESF_SHARED      0x08
#define RESF_EXTERNAL    0x10
#define RESF_RELEASE     0x20
#define RESF_DEFINED_IN_RESOURCEMAP  0x40

struct resItem
{
  enum valueType valueType;
  enum orderType orderType;
  int flags;
  int interval;
  char name[MAXLSFNAMELEN];
  char des[MAXRESDESLEN];

};

struct lsInfo
{
    uint nTypes;
    uint nModels;
    uint modelRefs[MAXMODELS];
    uint nRes;
    uint numIndx;
    uint numUsrIndx;
    char hostTypes[MAXTYPES][MAXLSFNAMELEN];
    char hostModels[MAXMODELS][MAXLSFNAMELEN];
    char hostArchs[MAXMODELS][MAXLSFNAMELEN];
    char paddin1[4];
    double cpuFactor[MAXMODELS];
    struct resItem *resTable;
};

#define CLUST_STAT_OK             0x01
#define CLUST_STAT_UNAVAIL        0x02

struct clusterInfo
{
  uid_t *adminIds;
  uid_t managerId;
  uint status;
  uint nTypes;
  uint nModels;
  uint numServers;
  uint numClients;
  uint nRes;
  uint nAdmins;
  char **resources;
  char **hostTypes;
  char **hostModels;
  char **admins;
  char clusterName[MAXLSFNAMELEN];
  char masterName[MAXHOSTNAMELEN];
  char managerName[MAXLSFNAMELEN];

};

struct hostInfo
{
    int rexPriority;
    uint maxCpus;
    uint maxMem;
    uint maxSwap;
    uint maxTmp;
    uint nDisks;
    uint nRes;
    char isServer;
    char padding1[3];
    char *hostModel;
    char *windows;
    char *hostType;
    float cpuFactor;
    char padding2[4];
    float *busyThreshold;
    unsigned long numIndx;
    char **resources;
    // char hostName[MAXHOSTNAMELEN]; /* FIXME FIXME FIXME this should be converted to char *hostName */
    char *hostName; // FIXME FIXME FIXME this should be converted to char *hostName 

};

  

/*
 * This data structure is built when reading the lsf.cluster file and
 * building the host list, it is also used to add add host at runtime.
 */
struct hostEntry
{
  int numIndx;
  int rcv;
  int nDisks;
  int nRes;
  int rexPriority;
  char hostName[MAXHOSTNAMELEN];
  char hostModel[MAXLSFNAMELEN];
  char hostType[MAXLSFNAMELEN];
  float cpuFactor;
  float *busyThreshold;
  char *window;
  char **resList;
  
};

struct config_param
{
  char *paramName;
  char *paramValue;
};

// FIXME FIXME FIXME FIXME this struct needs to be profiled for issues
//      with time
struct lsfRusage
{
//  struct timeval ru_utime; /* user CPU time used */
//  struct timeval ru_stime; /* system CPU time used */
  time_t ru_utime;
  suseconds_t ru_stime;
  char   padding[4];
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


#define NIO_STDIN_ON                        0x01
#define NIO_STDIN_OFF                       0x02
#define NIO_TAGSTDOUT_ON                    0x03
#define NIO_TAGSTDOUT_OFF                   0x04

#define NIO_TASK_STDINON                    0x01
#define NIO_TASK_STDINOFF                   0x02
#define NIO_TASK_ALL                        0x03
#define NIO_TASK_CONNECTED                  0x04

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

typedef struct lsSharedResourceInstance
{
  int nHosts;
  char padding[4];
  char *value;
  char **hostList;

} LS_SHARED_RESOURCE_INST_T;

typedef struct lsSharedResourceInfo
{
  int nInstances;
  char padding[4];
  char *resourceName;
  LS_SHARED_RESOURCE_INST_T *instances;
} LS_SHARED_RESOURCE_INFO_T;

struct clusterConf
{
  int numShareRes;
  uint numHosts;
  LS_SHARED_RESOURCE_INFO_T *shareRes;
  struct hostInfo *hosts;
  struct clusterInfo *clinfo;
};

/*
 * Maximum number of processes reported by PIM and read by the PIM library.
 */
#define MAX_PROC_ENT (2 * 1024)

struct pidInfo
{
  int pid;
  int ppid;
  int pgid;
  int jobid;
};

struct jRusage
{
  int mem;
  int swap;
  int utime;
  int stime;
  int npids;
  int npgids;
  int *pgid;
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
  uint16_t version;
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
  int   rcv;
  int   nDisks;
  int   numIndx;
  int   nRes;
  int   rexPriority;
  char  hostName[MAXHOSTNAMELEN];
  char  hostModel[MAXLSFNAMELEN];
  char  hostType[MAXLSFNAMELEN];
  float cpuFactor;
  float *busyThreshold;
  char  **resList;
  char  *window;
};

/*
 * openlava error numbers
 */
#define LSE_NO_ERR              0
#define LSE_BAD_XDR             1
#define LSE_MSG_SYS             2
#define LSE_BAD_ARGS            3
#define LSE_MASTR_UNKNW         4
#define LSE_LIM_DOWN            5
#define LSE_PROTOC_LIM          6
#define LSE_SOCK_SYS            7
#define LSE_ACCEPT_SYS          8
#define LSE_BAD_TASKF           9
#define LSE_NO_HOST             10
#define LSE_NO_ELHOST           11
#define LSE_TIME_OUT            12
#define LSE_NIOS_DOWN           13
#define LSE_LIM_DENIED          14
#define LSE_LIM_IGNORE          15
#define LSE_LIM_BADHOST         16
#define LSE_LIM_ALOCKED         17
#define LSE_LIM_NLOCKED         18
#define LSE_LIM_BADMOD          19
#define LSE_SIG_SYS             20
#define LSE_BAD_EXP             21
#define LSE_NORCHILD            22
#define LSE_MALLOC              23
#define LSE_LSFCONF             24
#define LSE_BAD_ENV             25
#define LSE_LIM_NREG            26
#define LSE_RES_NREG            27
#define LSE_RES_NOMORECONN      28
#define LSE_BADUSER             29
#define LSE_RES_ROOTSECURE      30
#define LSE_RES_DENIED          31
#define LSE_BAD_OPCODE          32
#define LSE_PROTOC_RES          33
#define LSE_RES_CALLBACK        34
#define LSE_RES_NOMEM           35
#define LSE_RES_FATAL           36
#define LSE_RES_PTY             37
#define LSE_RES_SOCK            38
#define LSE_RES_FORK            39
#define LSE_NOMORE_SOCK         40
#define LSE_WDIR                41
#define LSE_LOSTCON             42
#define LSE_RES_INVCHILD        43
#define LSE_RES_KILL            44
#define LSE_PTYMODE             45
#define LSE_BAD_HOST            46
#define LSE_PROTOC_NIOS         47
#define LSE_WAIT_SYS            48
#define LSE_SETPARAM            49
#define LSE_RPIDLISTLEN         50
#define LSE_BAD_CLUSTER         51
#define LSE_RES_VERSION         52
#define LSE_EXECV_SYS           53
#define LSE_RES_DIR             54
#define LSE_RES_DIRW            55
#define LSE_BAD_SERVID          56
#define LSE_NLSF_HOST           57
#define LSE_UNKWN_RESNAME       58
#define LSE_UNKWN_RESVALUE      59
#define LSE_TASKEXIST           60
#define LSE_BAD_TID             61
#define LSE_TOOMANYTASK         62
#define LSE_LIMIT_SYS           63
#define LSE_BAD_NAMELIST        64
#define LSE_LIM_NOMEM           65
#define LSE_NIO_INIT            66
#define LSE_CONF_SYNTAX         67
#define LSE_FILE_SYS            68
#define LSE_CONN_SYS            69
#define LSE_SELECT_SYS          70
#define LSE_EOF                 71
#define LSE_ACCT_FORMAT         72
#define LSE_BAD_TIME            73
#define LSE_FORK                74
#define LSE_PIPE                75
#define LSE_ESUB                76
#define LSE_EAUTH               77
#define LSE_NO_FILE             78
#define LSE_NO_CHAN             79
#define LSE_BAD_CHAN            80
#define LSE_INTERNAL            81
#define LSE_PROTOCOL            82
#define LSE_MISC_SYS            83
#define LSE_RES_RUSAGE          84
#define LSE_NO_RESOURCE         85
#define LSE_BAD_RESOURCE        86
#define LSE_RES_PARENT          87
#define LSE_I18N_SETLC          88
#define LSE_I18N_CATOPEN        89
#define LSE_I18N_NOMEM          90
#define LSE_NO_MEM              91
#define LSE_FILE_CLOSE          92
#define LSE_MASTER_LIM_DOWN     93
#define LSE_MLS_INVALID         94
#define LSE_MLS_CLEARANCE       95
#define LSE_MLS_RHOST           96
#define LSE_MLS_DOMINATE        97
#define LSE_HOST_EXIST          98
#define LSE_NERR                99

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

#define TIMEIT(level,func,name)                                         \
  { if (timinglevel > level) {                                        \
      struct timeval before, after;                               \
      struct timezone tz;                                         \
      gettimeofday(&before, &tz);                                 \
      func;                                                       \
      gettimeofday(&after, &tz);                                  \
      ls_syslog(LOG_INFO,"L%d %s %d ms",level,name,               \
            (int)((after.tv_sec - before.tv_sec)*1000 +       \
              (after.tv_usec-before.tv_usec)/1000));      \
    } else                                                          \
      func;                                                       \
  }


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

#define LC_SCHED    0x00000001
#define LC_EXEC     0x00000002
#define LC_TRACE    0x00000004
#define LC_COMM     0x00000008
#define LC_XDR      0x00000010
#define LC_CHKPNT   0x00000020
#define LC_FILE     0x00000080
#define LC_AUTH     0x00000200
#define LC_HANG     0x00000400
#define LC_SIGNAL   0x00001000
#define LC_PIM      0x00004000
#define LC_SYS      0x00008000
#define LC_JLIMIT   0x00010000
#define LC_PEND     0x00080000
#define LC_LOADINDX 0x00200000
#define LC_JGRP     0x00400000
#define LC_JARRAY   0x00800000
#define LC_MPI      0x01000000
#define LC_ELIM     0x02000000
#define LC_M_LOG    0x04000000
#define LC_PERFM    0x08000000

#define LOG_DEBUG1  LOG_DEBUG + 1
#define LOG_DEBUG2  LOG_DEBUG + 2
#define LOG_DEBUG3  LOG_DEBUG + 3

#define LSF_NIOS_REQUEUE        127

typedef void (*SIGFUNCTYPE) (int);

#ifndef MSGSIZE
#define MSGSIZE   8192
#endif

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

#define BSD_NICE

typedef struct stat LS_STAT_T;
#define LSTMPDIR        lsTmpDir_
#define LSDEVNULL       "/dev/null"
#define LSETCDIR        "/fix/me/no/such/place"
/* #define LSETCDIR        SYSCONFDIR */
#define closesocket close
#define CLOSESOCKET(s) close((s))
#define SOCK_CALL_FAIL(c) ((c) < 0 )
#define SOCK_INVALID(c) ((c) < 0 )
#define CLOSEHANDLE close
#define SOCK_READ_FIX  b_read_fix
#define SOCK_WRITE_FIX b_write_fix
#define NB_SOCK_READ_FIX   nb_read_fix
#define NB_SOCK_WRITE_FIX  nb_write_fix

#define LSF_NSIG NSIG

int lserrno;
int masterLimDown;
int ls_nerr;
extern char *ls_errmsg[];  // FIXME FIXME FIXME FIXME FIXME put in specific header
int logclass;
int timinglevel;

int lsf_lim_version;
int lsf_res_version;

int ls_initrex (int, int);
int ls_readconfenv (struct config_param *, char *);
int ls_connect (char *);
int ls_rexecv (char *, char **, int);
int ls_rexecve (char *, char **, int, char **);
int ls_rtask (char *, char **, int);
int ls_rtaske (char *, char **, int, char **);
int ls_rwait (LS_WAIT_T *, int, struct rusage *);
int ls_rwaittid (int, LS_WAIT_T *, int, struct rusage *);
int lls_rkill (uint rtid, int sig);
int ls_startserver (char *, char **, int);
int ls_conntaskport (int);
char **ls_placereq              (char *resreq, size_t *numhosts, int options, char *fromhost);
char **ls_placeofhosts          (char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize);
char **ls_placeoftype           (char *resreq, size_t *numhosts, int options, char *fromhost, char *hosttype);
struct hostLoad *ls_load        (char *resreq, size_t *numhosts, int options, char *fromhost);
struct hostLoad *ls_loadofhosts (char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize);
struct hostLoad *ls_loadoftype  (char *resreq, size_t *numhosts, int options, char *fromhost, char *hosttype);
struct hostLoad *ls_loadinfo    (char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize, char ***indxnamelist);
int ls_loadadj  (char *resreq, struct placeInfo *hostlist, size_t listsize);
int ls_eligible (char *task, char *resreqstr, char mode);
char *ls_resreq (char *task);
int ls_insertrtask (char *task);
int ls_insertltask (char *task);
int ls_deletertask (char *task);
int ls_deleteltask (char *task);
long ls_listrtask (char ***taskList, int sortflag);
long ls_listltask (char ***taskList, int sortflag);
char **ls_findmyconnections (void);
int ls_isconnected (char *hostName);
int ls_lostconnection (void);
char *ls_getclustername (void);
struct clusterInfo *ls_clusterinfo (char *resReq, unsigned int *numclusters, char **clusterList, int listsize, int options);
struct lsSharedResourceInfo *ls_sharedresourceinfo (char **resources, uint *numResources, char *hostName, int options);
char *ls_getmastername (void);
char *ls_getmastername2 (void);
char *ls_getmyhostname (void);
struct hostInfo *ls_gethostinfo (char *resReq, size_t *numhosts, char **hostlist, size_t listsize, int options);
char *ls_getISVmode (void);

struct lsInfo *ls_info (void);

char **ls_indexnames (struct lsInfo *);
int ls_isclustername (char *);
char *ls_gethosttype (char *hostname);
float *ls_getmodelfactor (char *modelname);
float *ls_gethostfactor (char *hostname);
char *ls_gethostmodel (char *hostname);
int ls_lockhost (time_t duration);
int ls_unlockhost (void);
int ls_limcontrol (char *hostname, int opCode);
void ls_remtty (int ind, int enableIntSus);
void ls_loctty (int ind);
char *ls_sysmsg (void);
void ls_perror (char *usrMsg);


struct lsConf *ls_getconf (char *);
void ls_freeconf (struct lsConf *);
struct sharedConf *ls_readshared (char *);
struct clusterConf *ls_readcluster (char *, struct lsInfo *);
struct clusterConf *ls_readcluster_ex (char *, struct lsInfo *, int);

int ls_initdebug (char *appName);
void ls_syslog (int level, const char *fmt, ...);
void ls_verrlog (FILE * fp, const char *fmt, va_list ap);

int ls_rescontrol (char *host, int opcode, int options);
int ls_stdinmode (int onoff);
int ls_stoprex (void);
int ls_donerex (void);
int ls_rsetenv (char *host, char **env);
int ls_rsetenv_async (char *host, char **env);
int ls_setstdout (int on, char *format);
int ls_niossync (long numTasks);
int ls_setstdin (int on, int *rpidlist, size_t len);
int ls_chdir (char *, char *);
int ls_fdbusy (uint fd);
char *ls_getmnthost (char *fn);
int ls_servavail (int, int);
int ls_setpriority (int newPriority);

int ls_ropen (char *host, char *fn, int flags, int mode);
int ls_rclose (int rfd);
int ls_rwrite (int rfd, char *buf, size_t len);
int ls_rread (int rfd, char *buf, size_t len);
off_t ls_rlseek (int rfd, off_t offset, int whence);
int ls_runlink (char *host, char *fn);
int ls_rfstat (int rfd, struct stat *buf);
int ls_rstat (char *host, char *fn, struct stat *buf);
char *ls_rgetmnthost (char *host, char *fn);
int ls_rfcontrol (int command, int arg);
int ls_rfterminate (char *host);

void ls_ruunix2lsf (struct rusage *rusage, struct lsfRusage *lsfRusage);
void ls_rulsf2unix (struct lsfRusage *lsfRusage, struct rusage *rusage);
void cleanLsfRusage (struct lsfRusage *);
void cleanRusage (struct rusage *);

struct resLogRecord *ls_readrexlog (FILE *);
int ls_nioinit (int sock);
int ls_nioselect (int, fd_set *, fd_set *, fd_set *, struct nioInfo **, struct timeval *);
int ls_nioctl (int, int);
int ls_nionewtask (int, int);
int ls_nioremovetask (int);
int ls_niowrite (char *, int);
int ls_nioclose (void);
int ls_nioread (int, char *, int);
int ls_niotasks (int, int *, int);
int ls_niostatus (int, int *, struct rusage *);
int ls_niokill (int);
int ls_niosetdebug (int);
int ls_niodump (int, int, int, char *);
struct lsfAcctRec *ls_getacctrec (FILE *, int *);
int ls_putacctrec (FILE *, struct lsfAcctRec *);
int getBEtime (char *, char, time_t *);

/*
 * openlava add host
 */
int ls_addhost (struct hostEntry *);
int ls_rmhost (const char *);

/*
 * openlava LIM log functions
 */
struct lsEventRec *ls_readeventrec (FILE *);
int ls_writeeventrec (FILE *, struct lsEventRec *);
int freeHostEntryLog (struct hostEntryLog **);

struct extResInfo
{
  char *name;
  char *type;
  char *interval;
  char *increasing;
  char *des;
};

#ifndef __CYGWIN__
int optind;
char *optarg;
int opterr;
int optopt;
#endif

// end lsf_h_
