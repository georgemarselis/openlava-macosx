
#pragma once

#include "daemons/liblimd/common.h"
#include "lsf.h"

// #ifndef L_SET
// # ifdef SEEK_SET
// #  define L_SET         SEEK_SET
// # else
// #  define L_SET         0
// # endif
// #endif

// #define NL_SETN 24

// #define  EXP3   0.716531311
// #define  EXP4   0.77880078
// #define  EXP6   0.846481725
// #define  EXP12  0.920044415
// #define  EXP180 0.994459848

// #define MAXIDLETIME  15552000
// #define IDLE_INTVL      30
// #define GUESS_NUM       30

// #define ENV_LAST_ACTIVE_TIME "LSF_LAST_ACTIVE_TIME"

// #define ELIMNAME "elim"
// #define MAXEXTRESLEN 4096

///////////////////////////////////////////////////
//
// defines
// 
#define nonuser(ut) ((ut).ut_type != USER_PROCESS)
// #define timersub(a,b,result)                                \  // fixme fixme this was defined for some reson
//     do {                                                    \
//         (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;       \
//         (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;    \
//         if ((result)->tv_usec < 0) {                        \
//             --(result)->tv_sec;                             \
//             (result)->tv_usec += 1000000;                   \
//         }                                                   \
//     } while (0)



#define  SWP_INTVL_CNT   45/exchIntvl  // FIXME FIXME FIXME FIXME these outghta be turned into constants
#define  TMP_INTVL_CNT 	120/exchIntvl
#define  PAGE_INTVL_CNT 120/exchIntvl

///////////////////////////////////////////////////
//
// constants
//
      char ENV_LAST_ACTIVE_TIME[] = "LSF_LAST_ACTIVE_TIME";
const char LINUX_LDAV_FILE[] = "/proc/loadavg";
const char ELIMNAME[] = "ELIM";
const time_t MAXIDLETIME    = 15552000;
const u_long MAXEXTRESLEN   = 4096;
const size_t GUESS_NUM      = 30;
const size_t IDLE_INTVL     = 30;
const float  EXP3           = 0.71653131;
const float  EXP4           = 0.77880078;
const float  EXP6           = 0.846481725;
const float  EXP12          = 0.920044415;
const float  EXP180         = 0.994459848;




///////////////////////////////////////////////////
//
// Function prototypes
// 
FILE   *lim_popen(char **, char *);
int     lim_pclose(FILE *);
time_t  getXIdle(void);
char   *getElimRes(void);
int     saveSBValue(char *, char *);
int     callElim(void);
int     startElim(void);
void    termElim(void);
int     isResourceSharedInAllHosts (char *resName);
float   initLicFactor(void);
void    setUnkwnValues(void);
void    unblockSigs_(sigset_t *);
void    getusr(void);
int     queueLengthEx (float *r15s, float *r1m, float *r15m);
void    cpuTime(time_t *itime, time_t *etime);
float   queueLength(void);
float   getpaging(float etime);
float   getIoRate (float etime);
float   getswap(void);
int     readMeminfo(void);
int     getPage (double *page_in, double *page_out, bool_t isPaging);
float   tmpspace (void);
int     realMem (float extrafactor);
int     numCpus (void);




///////////////////////////////////////////////////
//
// Globals
// 
char *buffer; // [MSGSIZE];
u_int64_t main_mem   = 0;
u_int64_t free_mem   = 0;
u_int64_t shared_mem = 0;
u_int64_t buf_mem    = 0;
u_int64_t cashed_mem = 0;
u_int64_t swap_mem   = 0;
u_int64_t free_swap  = 0;
u_long prevRQ             = 0; 

uint maxnLbHost = 0;
uint ncpus = 1;


time_t prev_time = 0;
time_t prev_idle = 0;
time_t prev_cpu_user_time = 0.0; // FIXME FIXME FIXME FIXME prev_cpu_user or prev_cpu_user_time ?
time_t prev_cpu_nice_time = 0.0;
time_t prev_cpu_sys_time  = 0.0;
time_t prev_cpu_idle_time = 0.0;
time_t lastActiveTime = 0;

int defaultRunElim    = FALSE;
int ELIMrestarts      = -1;
int ELIMdebug         = 0;
int ELIMblocktime     = -1;
pid_t elim_pid        = -1;

float k_hz            = 0.0;
float cpu_usage       = 0.0;
float overRide[NBUILTINDEX];  // FIXME FIXME FIXME FIXME enumerate each individual element




int pipefd[2];
struct limLock limLock;

