/* $Id: lib.conf.h 397 2007-11-26 19:04:00Z mblack $
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
 
#include "lsf.h"

#define ILLEGAL_CHARS     ".!-=+*/[]@:&|{}'`\""
#define M_THEN_A  1
#define A_THEN_M  2
#define M_OR_A    3


// #define DEF_REXPRIORITY 0
const unsigned int DEF_REXPRIORITY = 0;


// #define NL_SETN 42

// static 
struct lsInfo lsinfo;
// static 
struct clusterInfo clinfo;
// static 
struct sharedConf sConf_;
// static 
struct clusterConf cConf_;
// static 
struct sharedConf *sConf = NULL;
// static 
struct clusterConf *cConf = NULL;

struct builtIn
{
    short counter;
    char padding1[6];   
    const char *name;
    const char *des;
    enum valueType valuetype;
    enum orderType ordertype;
    int flags;
    int interval;
}; 

// #define TYPE1  RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL
// #define TYPE2  RESF_BUILTIN | RESF_GLOBAL
// #define TYPE3  RESF_BUILTIN | RESF_GLOBAL | RESF_LIC
const unsigned int TYPE1 = RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL; // the following three declerations belong in liblsf/conf.h
const unsigned int TYPE2 = RESF_BUILTIN | RESF_GLOBAL;
const unsigned int TYPE3 = RESF_BUILTIN | RESF_GLOBAL | RESF_LIC;

// static 
struct builtIn builtInRes[] = { // FIXME FIXME FIXME find out where this LS_NUMERIC comes from
    {  0, "      ",  "r15s",   "15-second CPU run queue length",              LS_NUMERIC, INCR, TYPE1 | RESF_RELEASE, 15 },
    {  1, "      ",  "r1m",    "1-minute CPU run queue length (alias: cpu)",  LS_NUMERIC, INCR, TYPE1 | RESF_RELEASE, 15 },
    {  2, "      ",  "r15m",   "15-minute CPU run queue length",              LS_NUMERIC, INCR, TYPE1 | RESF_RELEASE, 15 },
    {  3, "      ",  "ut",     "1-minute CPU utilization (0.0 to 1.0)",       LS_NUMERIC, INCR, TYPE1, 15  },
    {  4, "      ",  "pg",     "Paging rate (pages/second)",                  LS_NUMERIC, INCR, TYPE1, 15  },
    {  5, "      ",  "io",     "Disk IO rate (Kbytes/second)",                LS_NUMERIC, INCR, TYPE1, 15  },
    {  6, "      ",  "ls",     "Number of login sessions (alias: login)",     LS_NUMERIC, INCR, TYPE1, 30  },
    {  7, "      ",  "it",     "Idle time (minutes) (alias: idle)",           LS_NUMERIC, DECR, TYPE1, 30  },
    {  8, "      ",  "tmp",    "Disk space in /tmp (Mbytes)",                 LS_NUMERIC, DECR, TYPE1, 120 },
    {  9, "      ",  "swp",    "Available swap space (Mbytes) (alias: swap)", LS_NUMERIC, DECR, TYPE1, 15  },
    { 10, "      ",  "mem",    "Available memory (Mbytes)",                   LS_NUMERIC, DECR, TYPE1, 15  },
    { 11, "      ",  "ncpus",  "Number of CPUs",                              LS_NUMERIC, DECR, TYPE2, 0   },
    { 12, "      ",  "ndisks", "Number of local disks",                       LS_NUMERIC, DECR, TYPE2, 0   },
    { 13, "      ",  "maxmem", "Maximum memory (Mbytes)",                     LS_NUMERIC, DECR, TYPE2, 0   },
    { 14, "      ",  "maxswp", "Maximum swap space (Mbytes)",                 LS_NUMERIC, DECR, TYPE2, 0   },
    { 15, "      ",  "maxtmp", "Maximum /tmp space (Mbytes)",                 LS_NUMERIC, DECR, TYPE2, 0   },
    { 16, "      ",  "cpuf",   "CPU factor",                                  LS_NUMERIC, DECR, TYPE2, 0   },
    { 18, "      ",  "type",   "Host type",                                   LS_STRING,  NA,   TYPE2, 0   },
    { 19, "      ",  "model",  "Host model",                                  LS_STRING,  NA,   TYPE2, 0   },
    { 20, "      ",  "status", "Host status",                                 LS_STRING,  NA,   TYPE2, 0   },
    { 21, "      ",  "rexpri", "Remote execution priority",                   LS_NUMERIC, NA,   TYPE2, 0   },
    { 22, "      ",  "server", "LSF server host",                             LS_BOOLEAN, NA,   TYPE2, 0   },
    { 23, "      ",  "hname",  "Host name",                                   LS_STRING,  NA,   TYPE2, 0   },
    { -1, "      ",  NULL,      NULL,                                         LS_NUMERIC, INCR, TYPE1, 0   }
};

struct HostsArray
{
    unsigned int size;
    char padding[4];
    char **hosts;
};

struct keymap {
    unsigned int position;
    char padding1[4];
    const char *key;
    char *val;
};

const int builtInRes_ID[] = { // FIXME FIXME FIXME FIXME careful when compiling with the -Wkitchen-sink
    1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309,
    1310, 1311, 1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319,
    1320, 1321
};

//
// Function prototypes
////////////////////////////////////////////////////////

char do_Cluster     (FILE *fp, size_t *lineNum, const char *fname);
char do_HostTypes   (FILE *fp, size_t *lineNum, const char *fname);
char do_HostModels  (FILE *fp, size_t *lineNum, const char *fname);
char do_Resources   (FILE *fp, size_t *lineNum, const char *fname);
char do_Index       (FILE *fp, size_t *lineNum, const char *fname);
char do_Manager     (FILE *fp, const char *fname, size_t *lineNum, char const *clustermanager, int lookupAdmins);
char do_Hosts       (FILE *fp, const char *fname, size_t *lineNum, struct lsInfo *myInfo);
char do_Clparams    (FILE *fp, const char *fname, size_t *lineNum);

// static 
char addHostType (char *);
// static 
char addHostModel (char *model, char *arch, double factor);
// static 
char setIndex    (struct keymap *keyList, const char *fname, size_t lineNum);

unsigned int resNameDefined (char *);

void initClusterInfo (struct clusterInfo *);
void freeClusterInfo (struct clusterInfo *);
void initHostInfo (struct hostInfo *);
void freeHostInfo (struct hostInfo *);
// static 
void initResTable (void);
// static 
void putThreshold (int indx, struct hostEntry *hostEntryPtr, int position, char *val, float def);
void liblsf_putThreshold ( unsigned int indx, struct hostInfo *host, long position, const char *val, float def);


// static 
int wgetClusAdmins (char *line, const char *lsfile,   size_t *lineNum, char *secName);
int getClusAdmins  (char *line, const char *filename, size_t *lineNum, char *secName, int lookupAdmins);
// static 
struct admins *getAdmins_       (char *line, const char *fname, size_t *lineNum, const char *secName, int lookupAdmins);
struct admins *liblsf_getAdmins (char *line, const char *fname, size_t *lineNum, const char *secName, int lookupAdmins);

// static 
int validWindow ( const char *wordpair, const char *context);

// static 
int setAdmins (struct admins *, int);
// static 
void freeKeyList (struct keymap *keyList);
// static 
int validType (char *type);
int doResourceMap (FILE *fp, const char *lsfile, size_t *lineNum);
// static 
int addResourceMap        (char *resName, char *location, const char *lsfile, size_t lineNum, int *isDefault);
int liblsf_addResourceMap (char *resName, char *location, const char *lsfile, size_t lineNum);

// static 
unsigned int parseHostList ( const char *hostList, const char *lsfile, size_t lineNum, char ***hosts, int *isDefault);
int liblsf_parseHostList   ( const char *hostList, const char *lsfile, size_t lineNum, char ***hosts);
//  was : 
// static struct lsSharedResourceInfo *addResource (char *resName, int nHosts, char **hosts, char *value, char *fileName, size_t lineNum);
struct lsSharedResourceInfo *liblsf_addResource ( const char *resName, unsigned long nHosts, char **hosts, char *value, const char *fileName, size_t lineNum);
// static 
struct sharedResource *addResource (char *resName, unsigned int nHosts, char **hosts, char *value, const char *fileName, size_t lineNum, int resourceMap);
int liblsf_addHostInstance(struct lsSharedResourceInfo *sharedResource, unsigned int nHosts, char **hostNames, char *value);
// static 
int addHostInstance (struct sharedResource *sharedResource, unsigned int nHosts, char **hostNames, char *value, int resourceMap);
char addHost (struct hostInfo *host, const char *filename, size_t *lineNum);

int convertNegNotation_ (char **, struct HostsArray *);
// int resolveBaseNegHosts (char *, char **, struct HostsArray *);
int resolveBaseNegHosts ( const char *inHosts, char **outHosts, struct HostsArray *array); // correct invocation

void freeSA_ (char **list, unsigned int num);

int putValue (struct keymap *keyList, const char *key, char *value);
int isInlist (char **adminNames, const char *userName, unsigned int actAds);
char *getNextValue (char **line);
int keyMatch (struct keymap *keyList, char *line, int exact);
int isSectionEnd (char *linep, const char *lsfile, const size_t *lineNum, const char *sectionName);
char *getBeginLine (FILE *fp, size_t *lineNum);
int readHvalues (struct keymap *keyList, char *linep, FILE *fp, const char *lsfile, size_t *lineNum, int exact, const char *section);
int mapValues (struct keymap *keyList, char *line);
// FIXME also found in lproto.h, should be in lib/conf.h
// int putInLists (char *word, struct admins *admins, unsigned integer *numAds, char *forWhat);
int parse_time ( const char *word, float *hour, unsigned int *day);
struct clusterConf *ls_readcluster_ex ( const char *filename, struct lsInfo *info, int lookupAdmins);

int ls_setAdmins (struct admins *admins, int mOrA);
void freekeyval (struct keymap keylist[]);
char *parsewindow (char *linep, const char *filename, size_t *lineNum, const char *section);

