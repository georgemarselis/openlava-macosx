/* $Id: lib.h 397 2007-11-26 19:04:00Z mblack $
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

#include "daemons/liblimd/limout.h"
// #include "daemons/libpimd/pimd.h"
#include "daemons/libresd/resd.h"
#include "lib/conn.h"
#include "lib/hdr.h"
#include "lib/lproto.h"
#include "lib/mls.h"
#include "lib/xdr.h"
#include "lib/xdrlim.h"
#include "lsb/lsb.h"
#include "lsf.h"

// #define getpgrp(n)  getpgrp()

#ifndef LOG_PRIMASK
#define LOG_PRIMASK     0xf
#define LOG_MASK(pri)   (1 << (pri))
#define LOG_UPTO(pri)   ((1 << ((pri)+1)) - 1)
#endif

#ifndef LOG_PRI
#define LOG_PRI(p)      ((p) & LOG_PRIMASK)
#endif

#define packshort_(buf, x)       memcpy(buf, (char *)&(x), sizeof(short))
#define packint_(buf, x)         memcpy(buf, (char *)&(x), sizeof(int))
#define pack_lsf_rlim_t_(buf, x) memcpy(buf, (char *)&(x), sizeof(lsf_rlim_t))


// #define MIN_REF_NUM          1000
// #define MAX_REF_NUM          32760
enum MINMAX_REF {
    MIN_REF_NUM          = 1000,
    MAX_REF_NUM          = 32760,
    MINMAX_REF_NULL
};

// #define PGID_LIST_SIZE  16
// #define PID_LIST_SIZE   64
// #define MAX_NUM_PID     300
enum LIST_SIZES {
    PGID_LIST_SIZE       = 16,
    PID_LIST_SIZE        = 64,
    MAX_NUM_PID          = 300,
    LIST_SIZES_NULL
};

///////////////////////////////////////////////////////
//
// The score so far:
//      lsfParams/genParams_
//      lsbParams
//      limdParams
//      resdParams
// So, it makes sense we will have
//      mbatchdParams
//      sbatchdParams
//      pemdParams
//      niosdParams



// #define _NON_BLOCK_         0x01
// #define _LOCAL_             0x02
// #define _USE_TCP_           0x04
// #define _KEEP_CONNECT_      0x08
// #define _USE_PRIMARY_       0x10
// #define _USE_PPORT_         0x20
enum PORTS {
    _NON_BLOCK_    = 0x01,
    _LOCAL_        = 0x02,
    _USE_TCP_      = 0x04,
    _KEEP_CONNECT_ = 0x08,
    _USE_PRIMARY_  = 0x10,
    _USE_PPORT_    = 0x20,
};
/* openlava call LSF_SERVER_HOSTS regardless of local lim.
 */
enum _SERVER_HOSTS_ONLY_ {
    _SERVER_HOSTS_ONLY_ = 0x40
};

enum PMUT {
    PRIMARY = 0,
    MASTER  = 1,
    UNBOUND = 2,
    TCP     = 3
};

enum RESNIOS_TIMEOUT {
    // RES_TIMEOUT  = 120,
    NIOS_TIMEOUT = 120
};

enum NOCODE {
    NOCODE = 10000
};

enum RSIG {
    RSIG_ID_ISTID  = 0x01,
    RSIG_ID_ISPID  = 0x02,
    RSIG_KEEP_CONN = 0x04
};

enum RID {
    RID_ISTID = 0x01,
    RID_ISPID = 0x02
};

#define NO_SIGS (~(sigmask(SIGTRAP) | sigmask(SIGEMT)))
#define SET_LSLIB_NIOS_HDR(hdr,opcode,l) { (hdr).opCode = (opcode); (hdr).len = (l); }

#define CLOSEFD(s) if ((s) >= 0) {close((s)); (s) = -1;}


// FIXME FIXME FIXME FIXME what we have here is a chicken-and-egg problem
typedef struct lsRequest LS_REQUEST_T;
typedef struct lsRequest requestType;
typedef int (*requestCompletionHandler) (struct lsRequest *requestType);
typedef int (*appCompletionHandler) (struct lsRequest *requestType, void *);

struct lsRequest
{
    unsigned long taskid; // FIXME tid stands for "task id". Should be pid_t or even better something else.
    unsigned int seqno;
    int connfd;
    unsigned int rc;
    int completed;
    // char padding[4];
    unsigned long replyBufLen;
    requestCompletionHandler replyHandler;
    appCompletionHandler appHandler;
    void *extra;
    void *replyBuf;
    void *appExtra;
};


struct taskMsg
{
    char *inBuf;
    char *outBuf;
    size_t length;
};

enum lsTMsgType
{
    LSTMSG_DATA,
    LSTMSG_IOERR,
    LSTMSG_EOF
};

struct lsTMsgHdr
{
    enum lsTMsgType type;
    char padding[4];
    char *msgPtr;
    size_t length;
};

struct tid
{
    bool_t isEOF;
    unsigned short taskPort;
    char padding[2];
    unsigned long rtaskid;
    int sock;
    int refCount;
    // int pid;
    unsigned long taskid;
    const char *host;
    struct lsQueue *tMsgQ;
    struct tid *link;
};

struct svrsock
{
    int sockfd;
    int port;
    struct sockaddr_in *localAddr;
    int backlog;
    int options;
};


// void _lostconnection_ (char *);

// int setLockOnOff_ (int, time_t, char *hname);

// struct lsRequest *lsReqHandCreate_ (int, int, int, void *, requestCompletionHandler, appCompletionHandler, void *);
void lsReqHandDestroy_ (struct lsRequest *);

// void ls_errlog (FILE * fd, const char *fmt, ...)
#if defined(__GNUC__) && defined(CHECK_PRINTF)
    __attribute__ ((format (printf, 2, 3)))
#endif

