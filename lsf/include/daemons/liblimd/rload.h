
#pragma once
// #define NL_SETN 24

// #define  EXP3   0.716531311
// #define  EXP4   0.77880078
// #define  EXP6   0.846481725
// #define  EXP12  0.920044415
// #define  EXP180 0.994459848

// #define MAXIDLETIME  15552000

// #ifndef L_SET
// # ifdef SEEK_SET
// #  define L_SET         SEEK_SET
// # else
// #  define L_SET         0
// # endif
// #endif

FILE *lim_popen (char **, char *);
int lim_pclose (FILE *);
time_t getXIdle (void);
char *getElimRes (void);
int saveSBValue (char *, char *);
int callElim (void);
int startElim (void);
void termElim (void);
int isResourceSharedInAllHosts (char *resName);
float initLicFactor (void);
void setUnkwnValues (void);
void unblockSigs_ (sigset_t *);

int defaultRunElim = FALSE;
int ELIMrestarts   = -1;
int ELIMdebug      = 0;
int ELIMblocktime  = -1;
pid_t elim_pid     = -1;
float overRide[NBUILTINDEX];
float k_hz         = 0.0;

