/* $Id: lim.conf.h 397 2007-11-26 19:04:00Z mblack $
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

struct sharedResourceInstance *sharedResourceHead = NULL;
struct lsInfo allInfo;
struct shortLsInfo shortInfo;

hTab hostModelTbl;
static uint sizeOfResTable = 0;
static uint numofhosts     = 0;
static char mcServersSet   = FALSE;

int ELIMdebug, ELIMrestarts, ELIMblocktime;

#define M_THEN_A  1
#define A_THEN_M  2
#define M_OR_A    3

#define ILLEGAL_CHARS     ".!-=+*/[]@:&|{}'`\""

static struct hostNode *addHost_ (struct clusterNode *clPtr, struct hostEntry *hEntPtr,  char *window, char *fileName, uint *lineNumPtr );
static char   dotypelist (FILE * fp, uint *lineNum, char *lsfile);
static char   addHostModel (char *model, char *arch, double factor);
static struct clusterNode *addCluster (char *, char *);
static char   doclist (FILE * fp, uint *lineNum, char *lsfile);
static int    dohosts (FILE * clfp, struct clusterNode *clPtr, char *lsfile, uint *lineNum);
static int    doclparams (FILE * clfp, char *lsfile, uint *lineNum);
static char   dohostmodel (FILE * fp, uint *lineNum, char *lsfile);
static char   doresources (FILE * fp, uint *lineNum, char *lsfile);
static int    doresourcemap (FILE * fp, char *lsfile, uint *lineNum);
static char   doindex (FILE * fp, uint *lineNum, char *lsfile);
static int    readCluster2 (struct clusterNode *clPtr);
static int    domanager (FILE * clfp, char *lsfile, uint *lineNum, char *secName);
static char   setIndex (struct keymap *keyList, char *lsfile, uint lineNum);
static void   putThreshold (int indx, struct hostEntry *hostEntryPtr, int position, char *val, float def);
static int    modelNameToNo (char *);
static int    configCheckSum (char *, u_short *);
static int    reCheckClusterClass (struct clusterNode *);

static void   initResTable (void);
static char  *findClusterServers (char *);
static int    getClusAdmins (char *line, char *lsfile, uint *lineNum, char *secName);
static int    setAdmins (struct admins *, int);
static struct admins *getAdmins (char *line, char *fname, uint *lineNum, char *secName);
static void   freeKeyList (struct keymap *keyList);

static void   addMapBits (int, int *, int *);
static int    validType (char *);
static void   initResItem (struct resItem *);
static struct sharedResource *addResource (char *resName, uint nHosts, char **hosts, char *value, char *fileName, uint lineNum, int resourceMap);
static void   freeSharedRes (struct sharedResource *);
static int    addHostInstance (struct sharedResource *sharedResource, uint nHosts, char **hostNames, char *value, int resourceMap);
static struct resourceInstance *addInstance (struct sharedResource *sharedResource, uint nHosts, char **hostNames, char *value);
static struct resourceInstance *initInstance (void);
static void   freeInstance (struct resourceInstance *);
static int    addHostList (struct resourceInstance *resourceInstance, uint nHosts, char **hostNames);
static int    doresourcemap (FILE * fp, char *lsfile, uint *lineNum);
static int    addResourceMap (char *resName, char *location, char *lsfile, uint lineNum, int *isDefault);
static uint   parseHostList (char *hostList, char *lsfile, uint lineNum, char ***hosts, int *isDefault);
static int    addHostNodeIns (struct resourceInstance *instance, uint nHosts, char **hostNames);
static struct resourceInstance *isInHostNodeIns (char *resName, uint numInstances, struct resourceInstance **instances);
static char  *validLocationHost (char *);
static char **getValidHosts (char *hostName, uint *numHost, struct sharedResource *resource);
static void   adjIndx (void);
static int    doubleResTable (char *lsfile, uint lineNum);
static int    saveHostIPAddr (struct hostNode *, struct hostent *);

int readShared (void);

int convertNegNotation_ (char **, struct HostsArray *);

FILE *confOpen (char *filename, char *type);
float mykey (void);

static void setExtResourcesDefDefault (char *);
static int setExtResourcesDef (char *);
static int setExtResourcesLoc (char *, int);
struct extResInfo *getExtResourcesDef (char *);
char *getExtResourcesLoc (char *);
static char *getExtResourcesValDefault (char *);
char *getExtResourcesVal (char *);


#endif
