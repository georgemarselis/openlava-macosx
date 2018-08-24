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
uint sizeOfResTable = 0;
uint numofhosts     = 0;
char mcServersSet   = FALSE;

int ELIMdebug, ELIMrestarts, ELIMblocktime;

#define M_THEN_A  1
#define A_THEN_M  2
#define M_OR_A    3

#define ILLEGAL_CHARS     ".!-=+*/[]@:&|{}'`\""

// struct hostNode *addHost_ (struct clusterNode *clPtr, struct hostEntry *hEntPtr,  char *window, char *fileName, uint *lineNumPtr );
// char   dotypelist (FILE * fp, uint *lineNum, char *lsfile);
// char   addHostModel (char *model, char *arch, double factor);
// struct clusterNode *addCluster (char *, char *);
// char   doclist (FILE * fp, uint *lineNum, char *lsfile);
// int    dohosts (FILE * clfp, struct clusterNode *clPtr, char *lsfile, uint *lineNum);
// int    doclparams (FILE * clfp, char *lsfile, uint *lineNum);
// char   dohostmodel (FILE * fp, uint *lineNum, char *lsfile);
// char   doresources (FILE * fp, uint *lineNum, char *lsfile);
// int    doresourcemap (FILE * fp, char *lsfile, uint *lineNum);
// char   doindex (FILE * fp, uint *lineNum, char *lsfile);
// int    readCluster2 (struct clusterNode *clPtr);
// int    domanager (FILE * clfp, char *lsfile, uint *lineNum, char *secName);
// char   setIndex (struct keymap *keyList, char *lsfile, uint lineNum);
// void   putThreshold (int indx, struct hostEntry *hostEntryPtr, int position, char *val, float def);
// int    modelNameToNo (char *);
// int    configCheckSum (char *, u_short *);
// int    reCheckClusterClass (struct clusterNode *);

// void   initResTable (void);
// char  *findClusterServers (char *);
// int    getClusAdmins (char *line, char *lsfile, uint *lineNum, char *secName);
// int    setAdmins (struct admins *, int);
// struct admins *getAdmins (char *line, char *fname, uint *lineNum, char *secName);
// void   freeKeyList (struct keymap *keyList);

// void   addMapBits (int, uint *, uint *);
// int    validType (char *);
// void   initResItem (struct resItem *);
// struct sharedResource *addResource (char *resName, uint nHosts, char **hosts, char *value, char *fileName, uint lineNum, int resourceMap);
// void   freeSharedRes (struct sharedResource *);
// // int    addHostInstance (struct sharedResource *sharedResource, uint nHosts, char **hostNames, char *value, int resourceMap);
// struct resourceInstance *addInstance (struct sharedResource *sharedResource, uint nHosts, char **hostNames, char *value);
// struct resourceInstance *initInstance (void);
// void   freeInstance (struct resourceInstance *);
// int    addHostList (struct resourceInstance *resourceInstance, uint nHosts, char **hostNames);
// int    doresourcemap (FILE * fp, char *lsfile, uint *lineNum);
// // int    addResourceMap (char *resName, char *location, char *lsfile, uint lineNum, int *isDefault);
// unsigned int parseHostList (const char *hostList, const char *lsfile, const size_t lineNum, char ***hosts)
// int    addHostNodeIns (struct resourceInstance *instance, uint nHosts, char **hostNames);
// struct resourceInstance *isInHostNodeIns (char *resName, uint numInstances, struct resourceInstance **instances);
// char  *validLocationHost (char *);
// char **getValidHosts (char *hostName, uint *numHost, struct sharedResource *resource);
// void   adjIndx (void);
// int    doubleResTable (char *lsfile, uint lineNum);
// int    saveHostIPAddr (struct hostNode *, struct hostent *);

// int readShared (void);

// int convertNegNotation_ (char **, struct HostsArray *);

// FILE *confOpen (char *filename, char *type);
// float mykey (void);

// void setExtResourcesDefDefault (char *);
// int setExtResourcesDef (char *);
// int setExtResourcesLoc (char *, int);
// struct extResInfo *getExtResourcesDef (char *);
// char *getExtResourcesLoc (char *);
// char *getExtResourcesValDefault (char *);
// char *getExtResourcesVal (char *);
