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
#define TYPE1  RESF_BUILTIN | RESF_DYNAMIC | RESF_GLOBAL
#define TYPE2  RESF_BUILTIN | RESF_GLOBAL
#define TYPE3  RESF_BUILTIN | RESF_GLOBAL | RESF_LIC
#define DEF_REXPRIORITY 0


#define NL_SETN 42

static struct lsInfo lsinfo;
static struct clusterInfo clinfo;
static struct sharedConf sConf_;
static struct clusterConf cConf_;
static struct sharedConf *sConf = NULL;
static struct clusterConf *cConf = NULL;

struct builtIn
{
    char *name;
    char *des;
    enum valueType valuetype;
    enum orderType ordertype;
    int flags;
    int interval;
}; 

static struct builtIn builtInRes[24] = {
    {"r15s",   "15-second CPU run queue length",             LS_NUMERIC, INCR, TYPE1 | RESF_RELEASE, 15},
    {"r1m",    "1-minute CPU run queue length (alias: cpu)", LS_NUMERIC, INCR, TYPE1 | RESF_RELEASE, 15},
    {"r15m",   "15-minute CPU run queue length",             LS_NUMERIC, INCR, TYPE1 | RESF_RELEASE, 15},
    {"ut",     "1-minute CPU utilization (0.0 to 1.0)",      LS_NUMERIC, INCR, TYPE1, 15},
    {"pg",     "Paging rate (pages/second)",                 LS_NUMERIC, INCR, TYPE1, 15},
    {"io",     "Disk IO rate (Kbytes/second)",               LS_NUMERIC, INCR, TYPE1, 15},
    {"ls",     "Number of login sessions (alias: login)",    LS_NUMERIC, INCR, TYPE1, 30},
    {"it",     "Idle time (minutes) (alias: idle)",          LS_NUMERIC, DECR, TYPE1, 30},
    {"tmp",    "Disk space in /tmp (Mbytes)",                LS_NUMERIC, DECR, TYPE1, 120},
    {"swp",    "Available swap space (Mbytes) (alias: swap)",LS_NUMERIC, DECR, TYPE1, 15},
    {"mem",    "Available memory (Mbytes)",                  LS_NUMERIC, DECR, TYPE1, 15},
    {"ncpus",  "Number of CPUs",                             LS_NUMERIC, DECR, TYPE2, 0},
    {"ndisks", "Number of local disks",                      LS_NUMERIC, DECR, TYPE2, 0},
    {"maxmem", "Maximum memory (Mbytes)",                    LS_NUMERIC, DECR, TYPE2, 0},
    {"maxswp", "Maximum swap space (Mbytes)",                LS_NUMERIC, DECR, TYPE2, 0},
    {"maxtmp", "Maximum /tmp space (Mbytes)",                LS_NUMERIC, DECR, TYPE2, 0},
    {"cpuf",   "CPU factor",                                 LS_NUMERIC, DECR, TYPE2, 0},
    {"type",   "Host type",                                  LS_STRING,  NA,   TYPE2, 0},
    {"model",  "Host model",                                 LS_STRING,  NA,   TYPE2, 0},
    {"status", "Host status",                                LS_STRING,  NA,   TYPE2, 0},
    {"rexpri", "Remote execution priority",                  LS_NUMERIC, NA,   TYPE2, 0},
    {"server", "LSF server host",                            LS_BOOLEAN, NA,   TYPE2, 0},
    {"hname",  "Host name",                                  LS_STRING,  NA,   TYPE2, 0},
    {NULL,     NULL,                                         LS_NUMERIC, INCR, TYPE1, 0}
};

struct HostsArray
{
    uint size;
    char padding[4];
    char **hosts;
};

struct keymap {
    int position;
    char padding1[4];
    char *key;
    char *val;
};

static const int builtInRes_ID[] = {
  1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309,
  1310, 1311, 1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319,
  1320, 1321
};

char do_Cluster     (FILE *fp, uint *lineNum, char *fname);
char do_HostTypes   (FILE *fp, uint *lineNum, char *fname);
char do_HostModels  (FILE *fp, uint *lineNum, char *fname);
char do_Resources   (FILE *fp, uint *lineNum, char *fname);
char do_Index       (FILE *fp, uint *lineNum, char *fname);
char do_Manager     (FILE *fp, char *fname,   uint *lineNum, char *clustermanager, int lookupAdmins);
char do_Hosts       (FILE *fp, char *fname,   uint *lineNum, struct lsInfo *myInfo);
char do_Clparams    (FILE *fp, char *fname,   uint *lineNum);

static char addHostType (char *);
static char addHostModel (char *model, char *arch, double factor);
static char setIndex    (struct keymap *keyList, char *fname, uint lineNum);
char addHost     (struct hostInfo *host,  char *fname, uint *lineNum);

int resNameDefined (char *);

void initClusterInfo (struct clusterInfo *);
void freeClusterInfo (struct clusterInfo *);
void initHostInfo (struct hostInfo *);
void freeHostInfo (struct hostInfo *);
static void initResTable (void);
static void putThreshold (int indx, struct hostEntry *hostEntryPtr, int position, char *val, float def);
void liblsf_putThreshold (int indx, struct hostInfo *host, long position, char *val, float def);


static int wgetClusAdmins (char *line, char *lsfile, uint *lineNum, char *secName);
static struct admins *getAdmins_ (char *line, char *fname, uint *lineNum, char *secName, int lookupAdmins);
struct admins *liblsf_getAdmins (char *line, char *fname, uint *lineNum, char *secName, int lookupAdmins);

int parse_time (char *, float *, int *);
int validWindow (char *, char *);

static int setAdmins (struct admins *, int);
static void freeKeyList (struct keymap *keyList);
static int validType (char *type);
int doResourceMap (FILE *fp, char *lsfile, uint *lineNum);
static int addResourceMap (char *resName, char *location, char *lsfile, uint lineNum, int *isDefault);
int liblsf_addResourceMap (char *resName, char *location, char *lsfile, uint lineNum);

static uint parseHostList (char *hostList, char *lsfile, uint lineNum, char ***hosts, int *isDefault);
int liblsf_parseHostList (char *hostList, char *lsfile, uint lineNum, char ***hosts);
//  was : 
// static struct lsSharedResourceInfo *addResource (char *resName, int nHosts, char **hosts, char *value, char *fileName, uint lineNum);
struct lsSharedResourceInfo *liblsf_addResource (char *resName, int nHosts, char **hosts, char *value, char *fileName, uint lineNum);
static struct sharedResource *addResource (char *resName, uint nHosts, char **hosts, char *value, char *fileName, uint lineNum, int resourceMap);
int liblsf_addHostInstance(struct lsSharedResourceInfo *sharedResource, int nHosts, char **hostNames, char *value);
static int addHostInstance (struct sharedResource *sharedResource, uint nHosts, char **hostNames, char *value, int resourceMap);

int convertNegNotation_ (char **, struct HostsArray *);
int resolveBaseNegHosts (char *, char **, struct HostsArray *);

void freeSA_ (char **list, uint num);
