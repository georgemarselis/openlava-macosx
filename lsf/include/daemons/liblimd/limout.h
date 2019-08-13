/*
 *
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
#include "lib/header.h"

// #define   LIM_UNLOCK_USER      0
// #define   LIM_LOCK_USER        1
// #define   LIM_UNLOCK_MASTER    2
// #define   LIM_LOCK_MASTER      3

enum {
    LIM_UNLOCK_USER   = 0,
    LIM_LOCK_USER     = 1,
    LIM_UNLOCK_MASTER = 2,
    LIM_LOCK_MASTER   = 3
} LIM_LOCK;

// #define   LIM_LOCK_STAT_USER      0x1
// #define   LIM_LOCK_STAT_MASTER    0x2

enum {
    LIM_LOCK_STAT_USER    = 0x1,
    LIM_LOCK_STAT_MASTER = 0x2
} LIM_LOCK_STAT;

#define   LOCK_BY_USER(stat)     (((stat) & LIM_LOCK_STAT_USER) != 0)
#define   LOCK_BY_MASTER(stat)   (((stat) & LIM_LOCK_STAT_MASTER) != 0)

// #define   WINDOW_RETRY         0
// #define   WINDOW_OPEN          1
// #define   WINDOW_CLOSE         2
// #define   WINDOW_FAIL          3

enum {
   WINDOW_RETRY = 0,
   WINDOW_OPEN  = 1,
   WINDOW_CLOSE = 2,
   WINDOW_FAIL  = 3
} WINDOW;

// #define HINFO_SERVER    0x01
// #define HINFO_SHARED_RESOURCE   0x02

enum {
    HINFO_SERVER          = 0x01,
    HINFO_SHARED_RESOURCE = 0x02
};

enum ofWhat { 
    OF_ANY,
    OF_HOSTS,
    OF_TYPE
};

// typedef enum ofWhat ofWhat_t;

enum limReqCode
{
    LIM_PLACEMENT       = 1,
    LIM_LOAD_REQ        = 2,
    LIM_LOAD_ADJ        = 3,
    LIM_GET_CLUSNAME    = 4,
    LIM_GET_MASTINFO    = 5,
    LIM_GET_HOSTINFO    = 6,
    LIM_GET_CPUF        = 7,
    LIM_GET_INFO        = 8,
    LIM_GET_CLUSINFO    = 9,
    LIM_PING            = 10,
    LIM_CHK_RESREQ      = 11,
    LIM_DEBUGREQ        = 12,
    LIM_GET_RESOUINFO   = 13,
    LIM_ADD_HOST        = 14,
    LIM_RM_HOST         = 15,
    LIM_GET_MASTINFO2   = 16,
    LIM_REBOOT          = 50,
    LIM_LOCK_HOST       = 51,
    LIM_SERV_AVAIL      = 52,
    LIM_SHUTDOWN        = 53,
    LIM_LOAD_UPD        = 100,
    LIM_JOB_XFER        = 101,
    LIM_MASTER_ANN      = 102,
    LIM_CONF_INFO       = 103,
    LIM_CLUST_INFO      = 200,
    LIM_HINFO_REQ       = 201,
    LIM_HINFO_REPLY     = 202,
    LIM_LINFO_REQ       = 203,
    LIM_LINFO_REPLY     = 204
};

enum limReplyCode
{
    LIME_NO_ERR         = 1,
    LIME_WRONG_MASTER   = 2,
    LIME_BAD_RESREQ     = 3,
    LIME_NO_OKHOST      = 4,
    LIME_NO_ELHOST      = 5,
    LIME_BAD_DATA       = 6,
    LIME_BAD_REQ_CODE   = 7,
    LIME_MASTER_UNKNW   = 8,
    LIME_DENIED         = 9,
    LIME_IGNORED        = 10,
    LIME_UNKWN_HOST     = 11,
    LIME_UNKWN_MODEL    = 12,
    LIME_LOCKED_AL      = 13,
    LIME_NOT_LOCKED     = 14,
    LIME_BAD_SERVID     = 15,
    LIME_NAUTH_HOST     = 16,
    LIME_UNKWN_RNAME    = 17,
    LIME_UNKWN_RVAL     = 18,
    LIME_NO_ELCLUST     = 20,
    LIME_NO_MEM         = 21,
    LIME_BAD_FILTER     = 22,
    LIME_BAD_RESOURCE   = 23,
    LIME_NO_RESOURCE    = 24,
    LIME_KWN_MIGRANT    = 25
};


struct decisionReq {
    int options;
    char hostType[MAX_LSF_NAME_LEN];
    char resReq[MAX_LINE_LEN];
    enum ofWhat ofWhat;
    unsigned long numPrefs;
    unsigned long numHosts;
    char **preferredHosts;
};

struct placeReply
{
    size_t numHosts;
    struct placeInfo *placeInfo;
};

struct jobXfer
{
    char resReq[MAX_LINE_LEN];
    size_t numHosts;
    struct placeInfo *placeInfo;
};


// #define FIRST_LIM_PRIV      LIM_REBOOT
// #define FIRST_LIM_LIM       LIM_LOAD_UPD
// #define FIRST_INTER_CLUS    LIM_CLUST_INFO

enum { 
    FIRST_LIM_PRIV   = LIM_REBOOT,
    FIRST_LIM_LIM    = LIM_LOAD_UPD,
    FIRST_INTER_CLUS = LIM_CLUST_INFO 
} FIRST;


// #define LOAD_REPLY_SHARED_RESOURCE 0x1 // duplicate

struct loadReply
{
    unsigned long nEntry;
    unsigned long nIndex;
    int flags;
    char padding[4];
    char **indicies;
    struct hostLoad *loadMatrix;
};

struct shortHInfo
{
    char hostName[MAXHOSTNAMELEN];      // FIXME FIXME FIXME FIXME this must be made into a pointer and init'ed properly
    unsigned int hTypeIndx;
    unsigned int hModelIndx;
    unsigned int maxCpus;
    unsigned int maxMem;
    unsigned int maxSwap;
    unsigned int maxTmp;
    unsigned int nDisks;
    unsigned int resClass;
    int flags;
    int rexPriority;
    unsigned int nRInt;
    char padding[4];
    char *windows;
    unsigned int *resBitMaps;
    double *busyThreshold;

};

struct shortLsInfo
{
    int *stringResBitMaps;
    int *numericResBitMaps;
    unsigned int nTypes;
    unsigned int nRes;
    unsigned int nModels;
    char padding[4];
    double cpuFactors[MAX_MODELS];
    char *hostModels[MAX_MODELS];
    char *hostTypes[MAX_TYPES];
    char **resName;
    
};

struct hostInfoReply
{
    unsigned long nIndex;
    unsigned long nHost;
    struct shortLsInfo *shortLsInfo;
    struct shortHInfo *hostMatrix;
};

struct clusterInfoReq
{
    unsigned int listsize;
    int options;
    char *resReq;
    char **clusters;
};

struct shortCInfo
{
    char *clName;      // [MAX_LSF_NAME_LEN];
    char *masterName;  // [MAXHOSTNAMELEN];
    char *managerName; // [MAX_LSF_NAME_LEN];
    uid_t managerId;
    unsigned int status;
    unsigned int resClass;
    unsigned int typeClass;
    unsigned int modelClass;
    unsigned int numIndx;
    unsigned int numUsrIndx;
    unsigned int usrIndxClass;
    unsigned int numServers;
    unsigned int numClients;
    unsigned int nAdmins;
    unsigned int nRes;
    unsigned int nTypes;
    unsigned int nModels;
    unsigned int *hostTypeBitMaps;
    unsigned int *hostModelBitMaps;
    unsigned int *resBitMaps;
    uid_t *adminIds;
    char **admins;
    
};

struct cInfo
{
    char clName[MAX_LSF_NAME_LEN];
    char masterName[MAXHOSTNAMELEN];
    char managerName[MAX_LSF_NAME_LEN];
    pid_t managerId;
    unsigned int status;
    unsigned int resClass;
    unsigned int typeClass;
    unsigned int modelClass;
    unsigned int numIndx;
    unsigned int numUsrIndx;
    unsigned int usrIndxClass;
    unsigned int numServers;
    unsigned int numClients;
    unsigned int nRes;
    unsigned int nTypes;
    unsigned int nModels;
    unsigned int nAdmins;
    pid_t *adminIds;
    unsigned int *resBitMaps;
    unsigned int *hostTypeBitMaps;
    unsigned int *hostModelBitMaps;
    char **admins;
    char **loadIndxNames;
    struct shortLsInfo shortInfo;
};

struct clusterInfoReply
{
    unsigned int nClusters;
    char paddding[4];
    struct shortLsInfo *shortLsInfo;
    struct shortCInfo *clusterMatrix;
};

struct masterInfo
{
    char padding[2];
    unsigned short portno;
    char hostName[MAXHOSTNAMELEN];
    in_addr_t addr;

};

struct clusterList
{
    char **clNames;
    unsigned int nClusters;
    char padding[4];
};

struct limLock
{
    unsigned int uid;
    int on;
    time_t time;
    char lsfUserName[MAX_LSF_NAME_LEN];
};
