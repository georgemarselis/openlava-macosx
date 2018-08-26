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
#include "daemons/libpimd/pimd.h"
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
//      genParams
//      lsfParams
//      lsbParams
//      limdParams
//      resdParams
// So, it makes sense we will have
//      mbatchdParams
//      sbatchdParams
//      pemdParams
//      niosdParams



// LSF

static enum LSF_PARAMS {
	LSF_DEBUG                   = 0,
	LSF_SERVERDIR               = 1,
	LSF_ENVDIR                  = 2, // FIXME FIXME newly inserted variable
	LSF_LOGDIR                  = 3,
	LSF_LOG_MASK                = 8, // or 19
	LSF_ID_PORT                 = 9,
	LSF_AUTH                    = 10,
	LSF_DEBUG_RES               = 11,
	LSF_TIME_RES                = 12,
	LSF_ROOT_REX                = 13,
	LSF_RES_RLIMIT_UNLIM        = 14,
	LSF_CMD_SHELL               = 15,
	LSF_ENABLE_PTY              = 16,
	LSF_TMPDIR                  = 17,
	LSF_BINDIR                  = 18, // or 20
	// LSF_LOG_MASK                = 19,
	LSF_RES_NO_LINEBUF          = 21,
	LSF_CONFDIR                 = 23,
	LSF_GETPWNAM_RETRY          = 28,
	LSF_AUTH_DAEMONS            = 33,
	LSF_LIBDIR                  = 40,
	LSF_MLS_LOG                 = 46,
	// LSF_LIM_DEBUG               = 2
	LSF_RES_ACCTDIR             = 9,
	LSF_RES_ACCT                = 10,
	LSF_USE_HOSTEQUIV           = 122,
	LSF_NULL
} LSF_STATUS;

static struct config_param lsfParams_[ ] = { // FIXME FIXME FIXME FIXME genParams order has to match with above enum LSF_LSB
	{ "LSF_DEBUG",              NULL }, // 0
	{ "LSF_SERVERDIR",          NULL }, // 1
	{ "LSF_ENVDIR",             NULL }, // 2
	{ "LSF_LOGDIR",             NULL }, // 3
	{ "LSF_LIM_PORT",           NULL }, // 5
	{ "LSF_RES_PORT",           NULL }, // 6
	{ "LSF_LOG_MASK",           NULL }, // 8 // or 19
	{ "LSF_ID_PORT",            NULL }, // 9
	{ "LSF_AUTH",               NULL }, // 10
	{ "LSF_USE_HOSTEQUIV",      NULL }, // 12
	{ "LSF_ROOT_REX",           NULL }, // 13
	{ "LSF_TMPDIR",             NULL }, // 17
	{ "LSF_BINDIR",             NULL }, // 18 // or 20
	{ "LSF_CONFDIR",            NULL }, // 23
	{ "LSF_GETPWNAM_RETRY",     NULL }, // 28
	{ "LSF_AUTH_DAEMONS",       NULL }, // 33
	{ "LSF_LIBDIR",             NULL }, // 40
	{ "LSF_MLS_LOG",            NULL }, // 46
	{ "LSF_STRIP_DOMAIN",       NULL }, // N+1
	{ "LSF_SERVER_HOSTS",       NULL }, // N+2
	{ "LSF_API_CONNTIMEOUT",    NULL }, // N+3
	{ "LSF_API_RECVTIMEOUT",    NULL }, // N+4
	{ "LSF_AM_OPTIONS",         NULL }, // N+5
	{ "LSF_SYMBOLIC_LINK",      NULL }, // N+6
	{ "LSF_MASTER_LIST",        NULL }, // N+7
	{ "LSF_INTERACTIVE_STDERR", NULL }, // N+8
	{ "LSF_SHAREDIR",           NULL }, // N+9
	{ "LSF_CONF_RETRY_INT",     NULL }, // N+10
	{ "LSF_CONF_RETRY_MAX",     NULL }, // N+11
	{ "LSF_HOSTS_FILE",         NULL }, // N+12 // renamed from HOSTS_FILE
	{ "LSF_NO_HOSTS_FILE",      NULL }, // N+13 // renamed from NO_HOSTS_FILE
	{ NULL,                     NULL }
};

static enum LSB_PARAMS {
	LSB_DEBUG                   = 0,
	LSB_CONFDIR                 = 1,
	LSB_SHAREDIR                = 4,
	LSB_MAILTO                  = 5,
	LSB_MAILPROG                = 6,
	LSB_MBD_PORT                = 8,
	LSB_SBD_PORT                = 7,

	LSB_CRDIR                   = 11,
	LSB_DEBUG_MBD               = 14,
	LSB_DEBUG_SBD               = 15,
	LSB_TIME_MBD                = 16,
	LSB_TIME_SBD                = 17,
	LSB_SIGSTOP                 = 18,

	LSB_MBD_CONNTIMEOUT         = 21,
	LSB_SBD_CONNTIMEOUT         = 22,
	LSB_MBD_MAILREPLAY          = 24,
	LSB_MBD_MIGTOPEND           = 25,
	LSB_SBD_READTIMEOUT         = 26,
	LSB_MBD_BLOCK_SEND          = 27,
	LSB_MEMLIMIT_ENFORCE        = 29,

	LSB_BSUBI_OLD               = 30,
	LSB_STOP_IGNORE_IT          = 31,
	LSB_HJOB_PER_SESSION        = 32,
	LSB_REQUEUE_HOLD            = 34,
	LSB_SMTP_SERVER             = 35,
	LSB_MAILSERVER              = 36,
	LSB_MAILSIZE_LIMIT          = 37,
	LSB_REQUEUE_TO_BOTTOM       = 38,
	LSB_ARRAY_SCHED_ORDER       = 39,

	LSB_QPOST_EXEC_ENFORCE      = 41,
	LSB_MIG2PEND                = 42,
	LSB_UTMP                    = 43,
	LSB_JOB_CPULIMIT            = 44,
	LSB_RENICE_NEVER_AT_RESTART = 45,
	LSB_JOB_MEMLIMIT            = 47,
	LSB_MOD_ALL_JOBS            = 48,
	LSB_SET_TMPDIR              = 49,

	LSB_PTILE_PACK              = 50,
	LSB_SBD_FINISH_SLEEP        = 51,
	LSB_VIRTUAL_SLOT            = 52,
	LSB_STDOUT_DIRECT           = 53,

	// found around the code base
	LSB_DEBUG_CMD               = 4,
	LSB_CMD_LOG_MASK            = 7,
	LSB_API_CONNTIMEOUT         = 9,
	LSB_API_RECVTIMEOUT         = 10,
	LSB_SERVERDIR               = 11,
	LSB_MODE                    = 12,
	LSB_SHORT_HOSTLIST          = 13,
	LSB_INTERACTIVE_STDERR      = 14,
	LSB_32_PAREN_ESC            = 15,
	LSB_API_QUOTE_CMD           = 14, // or 16

	LSB_NULL = 999

} LSB_PARAMS;


static struct config_param lsbParams[ ] = {
	{ "LSB_DEBUG",                   NULL }, // 0
	{ "LSB_CONFDIR",                 NULL }, // 1
	{ "LSB_SHAREDIR",                NULL }, // 4
	{ "LSB_MAILTO",                  NULL }, // 5
	{ "LSB_MAILPROG",                NULL }, // 6
	{ "LSB_MBD_PORT",                NULL }, // 8
	{ "LSB_SBD_PORT",                NULL }, // 7

	{ "LSB_CRDIR",                   NULL }, // 11
	{ "LSB_DEBUG_MBD",               NULL }, // 14
	{ "LSB_DEBUG_SBD",               NULL }, // 15
	{ "LSB_TIME_MBD",                NULL }, // 16
	{ "LSB_TIME_SBD",                NULL }, // 17
	{ "LSB_SIGSTOP",                 NULL }, // 18

	{ "LSB_MBD_CONNTIMEOUT",         NULL }, // 21
	{ "LSB_SBD_CONNTIMEOUT",         NULL }, // 22
	{ "LSB_MBD_MAILREPLAY",          NULL }, // 24
	{ "LSB_MBD_MIGTOPEND",           NULL }, // 25
	{ "LSB_SBD_READTIMEOUT",         NULL }, // 26
	{ "LSB_MBD_BLOCK_SEND",          NULL }, // 27
	{ "LSB_MEMLIMIT_ENFORCE",        NULL }, // 29

	{ "LSB_BSUBI_OLD",               NULL }, // 30
	{ "LSB_STOP_IGNORE_IT",          NULL }, // 31
	{ "LSB_HJOB_PER_SESSION",        NULL }, // 32
	{ "LSB_REQUEUE_HOLD",            NULL }, // 34
	{ "LSB_SMTP_SERVER",             NULL }, // 35
	{ "LSB_MAILSERVER",              NULL }, // 36
	{ "LSB_MAILSIZE_LIMIT",          NULL }, // 37
	{ "LSB_REQUEUE_TO_BOTTOM",       NULL }, // 38
	{ "LSB_ARRAY_SCHED_ORDER",       NULL }, // 39

	{ "LSB_QPOST_EXEC_ENFORCE",      NULL }, // 41
	{ "LSB_MIG2PEND",                NULL }, // 42
	{ "LSB_UTMP",                    NULL }, // 43
	{ "LSB_JOB_CPULIMIT",            NULL }, // 44
	{ "LSB_RENICE_NEVER_AT_RESTART", NULL }, // 45
	{ "LSB_JOB_MEMLIMIT",            NULL }, // 47
	{ "LSB_MOD_ALL_JOBS",            NULL }, // 48
	{ "LSB_SET_TMPDIR",              NULL }, // 49

	{ "LSB_PTILE_PACK",              NULL }, // 50
	{ "LSB_SBD_FINISH_SLEEP",        NULL }, // 51
	{ "LSB_VIRTUAL_SLOT",            NULL }, // 52
	{ "LSB_STDOUT_DIRECT",           NULL }, // 53

	// doubly-assigned values or newly found values
	{ "LSB_DEBUG_CMD",               NULL }, // 4  
	{ "LSB_TIME_CMD",                NULL }, // 5
	{ "LSB_CMD_LOGDIR",              NULL }, // 6
	{ "LSB_LOG_MASK",                NULL }, // ? FIXME FIXME FIXME FIXME
	{ "LSB_CMD_LOG_MASK",            NULL }, // 7 
	{ "LSB_API_CONNTIMEOUT",         NULL }, // 9
	{ "LSB_API_RECVTIMEOUT",         NULL }, // 10
	{ "LSB_SERVERDIR",               NULL }, // 11
	{ "LSB_MODE",                    NULL }, // 12
	{ "LSB_SHORT_HOSTLIST",          NULL }, // 13
	{ "LSB_API_QUOTE_CMD",           NULL }, // 14 // or 16
	{ "LSB_32_PAREN_ESC",            NULL }, // 15

	{ NULL,                          NULL }
};

// RESD

enum RES_PARAMS {
	RES_DEBUG                     = 0,
	RES_PORT                      = 6,    // default port: 36002
	RES_TIMEOUT                   = 20,   // default timeout: 
	RES_TIMING                    = 666,  // made up value
	RES_NULL
};

static struct config_param resDaemonParams[ ] = {
	{ "RES_DEBUG",                NULL }, // 0
	{ "RES_PORT",              "36002" }, // 6   // default port: 36002 // FIXME FIXME FIXME FIXME insert into configure.ac
	{ "RES_TIMEOUT",              NULL }, // 20  // default timeout: 
	{ "RES_TIMING",               NULL }, // 666 // made up value 
	{ NULL,                       NULL }
};

// PIMD

static enum PIM_API {
	PIM_API_TREAT_JID_AS_PGID = 0x1,
	PIM_API_UPDATE_NOW        = 0x2,
	PIM_API_NULL
} PIM_API;

// #define PIM_SLEEP_TIME 3
// #define PIM_UPDATE_INTERVAL 30
static enum PIM_PARAMS {
	PIM_DEBUG             = -1,
	PIM_INFODIR           =  0,
	PIM_SLEEP_TIME        =  1, // or 3
	PIM_PORT              =  3, // made up value
	PIM_SLEEPTIME_UPDATE  =  4, // what is the difference between this and PIM_UPDATE_INTERVAL
	PIM_UPDATE_INTERVAL   =  30,
	PIM_NULL
} PIM_PARAMS;

static struct config_param pimDaemonParams[ ] = {
	{ "PIM_DEBUG",                NULL }, // -1
	{ "PIM_INFODIR",              NULL }, //  0
	{ "PIM_SLEEPTIME",            NULL }, //  1
	{ NULL,                       NULL },
	{ "PIM_PORT",                 NULL }, //  3 // made up value
	{ "PIM_SLEEPTIME_UPDATE",     NULL }, //  4
	{ "PIM_UPDATE_INTERVAL",      NULL }, //  30
	{ NULL,                       NULL }
};

// LIMD

enum LIM_PARAMS {
	LIM_DEBUG                         = 0,
	LIM_PORT                          = 5,
	LIM_TIMING                        = 6,
	LIM_NO_MIGRANT_HOSTS              = 55,
	LIM_NULL
};

static struct config_param limDaemonParams[ ] = {
	{ "LIM_DEBUG",                    NULL }, // 0
	{ "LIM_PORT",                     NULL }, // 5   // default port: 
	{ "LIM_TIMING",                   NULL }, // 6 
	{ "LIM_NO_MIGRANT_HOSTS",         NULL }, // 55
	{ NULL,                           NULL }
};

// MDB

enum MDB_PARAMS {
	MBD_DEBUG                         = 0,
	MBD_PORT                          = 3,
	MBD_DONT_FORK                     = 54,
	MBD_TIMING                        = 666, // made up value
	MBD_NULL
};

static struct config_param mbdDaemonParams[ ] = {
	{ "MBD_DEBUG",                    NULL }, // 0	
	{ "MBD_PORT",                     NULL }, // 3   // default port: 
	{ "MBD_DONT_FORK",                NULL }, // 54
	{ "MBD_TIMING",                   NULL }, // 666 // made up value
	{ NULL,                           NULL }
};

// SDB

enum SDB_PARAMS {
	SBD_DEBUG                         = 0,
	SBD_PORT                          = 3,
	SBD_DONT_FORK                     = 54,
	SBD_TIMING                        = 666,
	SBD_NULL
};

static struct config_param sbdDaemonParams[ ] = {
	{ "SBD_DEBUG",                    NULL }, // 0
	{ NULL,                           NULL },
	{ NULL,                           NULL },
	{ "SBD_PORT",                     NULL }, // 3
	{ "SBD_DONT_FORK",                NULL }, // 54
	{ "SBD_TIMING",                   NULL }, // 666 // made up value
	{ NULL,                           NULL }
};


// static enum limParams_t {
// 	LIM_DEBUG,
// // #define LIM_PORT        36000 // FIXME FIXME FIXME FIXME FIXME set appropriate configuration variable in configure.ac
// // #define RES_PORT        36002 // FIXME FIXME FIXME FIXME FIXME set appropriate configuration variable in configure.ac 
// 	LIM_PORT, // FIXME FIXME FIXME FIXME FIXME set appropriate configuration variable in configure.ac; 3600 by default
// 	LIM_TIME,
// 	LIM_IGNORE_CHECKSUM,
// 	LIM_JACKUP_BUSY,
// 	LIM_COMPUTE_ONLY,
// 	// LIM_NO_MIGRANT_HOSTS,
// 	LIM_NO_FORK,
// 	LSF_DEBUG_LIM,
// 	LSF_TIME_LIM,
// 	LIM_RSYNC_CONFIG
// } limParams_t;

// enum resdParams_t {
// 	RES_PORT_DEFAULT = 36002 // FIXME FIXME FIXME FIXME FIXME make it configurable in configure.ac; 36002 by default
// } resdParams_t;


// static struct config_param limParams[] = {
// 	{ "LIM_DEBUG",            NULL },
// 	{ "LIM_PORT",             NULL },
// 	{ "LIM_TIME",             NULL },
// 	{ "LIM_IGNORE_CHECKSUM",  NULL },
// 	{ "LIM_JACKUP_BUSY",      NULL },
// 	{ "LIM_COMPUTE_ONLY",     NULL },
// 	{ "LIM_NO_MIGRANT_HOSTS", NULL },
// 	{ "LIM_NO_FORK",          NULL },
// 	{ "LSF_DEBUG_LIM",        NULL },
// 	{ "LSF_TIME_LIM",         NULL },
// 	{ "LIM_RSYNC_CONFIG",     NULL },
// 	{ NULL,                   NULL },
// };

// static struct config_param debParams[] = {
// 	{ "LSF_DEBUG_CMD",      NULL },
// 	{ "LSF_TIME_CMD",       NULL },
// 	{ "LSF_CMD_LOGDIR",     NULL },
// 	{ "LSF_CMD_LOG_MASK",   NULL },
// 	{ "LSF_LOG_MASK_DEBUG", NULL },
// 	{ NULL,                 NULL }
// };

// static enum debugParams_t {
// 	LSF_DEBUG_CMD,
// 	LSF_TIME_CMD,
// 	LSF_CMD_LOGDIR,
// 	LSF_CMD_LOG_MASK,
// 	LSF_LOG_MASK_DEBUG
// } debugParams_t;


// static enum niosResCmd { 
// 	NIOS2RES_EOF,
// 	NIOS2RES_HEARTBEAT,
// 	NIOS2RES_SETTTY,
// 	NIOS2RES_SIGNAL,
// 	NIOS2RES_STDIN,
// 	NIOS2RES_TIMEOUT
// } niosResCmd;

// static enum resNiosCmd {
// 	RES2NIOS_CONNECT,
// 	RES2NIOS_EOF,
// 	RES2NIOS_NEWTASK,
// 	RES2NIOS_REQUEUE,
// 	RES2NIOS_STATUS,
// 	RES2NIOS_STDERR,
// 	RES2NIOS_STDOUT
// } resNiosCmd;

// typedef enum status genparams_t;

#define AM_LAST  (!(genParams_[LSF_AM_OPTIONS].paramValue && \
									strstr(genParams_[LSF_AM_OPTIONS].paramValue, \
															AUTOMOUNT_LAST_STR)))

#define AM_NEVER (genParams_[LSF_AM_OPTIONS].paramValue && \
									strstr(genParams_[LSF_AM_OPTIONS].paramValue, \
															AUTOMOUNT_NEVER_STR))


#define LOOP_ADDR       0x7F000001

// #define _NON_BLOCK_         0x01
// #define _LOCAL_             0x02
// #define _USE_TCP_           0x04
// #define _KEEP_CONNECT_      0x08
// #define _USE_PRIMARY_       0x10
// #define _USE_PPORT_         0x20
enum PORTS {
	_NON_BLOCK_ = 0x01,
	_LOCAL_ = 0x02,
	_USE_TCP_ = 0x04,
	_KEEP_CONNECT_ = 0x08,
	_USE_PRIMARY_ = 0x10,
	_USE_PPORT_ = 0x20,
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
	RSIG_ID_ISTID = 0x01,
	RSIG_ID_ISPID = 0x02,
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
	int tid;
	pid_t seqno;
	int connfd;
	int rc;
	int completed;
	char padding[4];
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
	size_t len;
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
	size_t len;
};

struct tid
{
	bool_t isEOF;
	unsigned short taskPort;
	char padding[2];
	int rtid;
	int sock;
	int refCount;
	int pid;
	char *host;
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

// #define LS_CSO_ASYNC_NT       (0x0001)
// #define LS_CSO_PRIVILEGE_PORT (0x0002)
static enum LS_CSO {
	LS_CSO_ASYNC_NT       = 0x0001,
	LS_CSO_PRIVILEGE_PORT = 0x0002
} LS_CSO;

extern int isPamBlockWait;
// static struct lsQueue *requestQ;
// static unsigned int requestSN;

// static struct sockaddr_in limSockId_;
// static struct sockaddr_in limTcpSockId_;
extern char rootuid_;
// static struct sockaddr_in res_addr_;
// static fd_set connection_ok_;
// static int currentsocket_;
// static int totsockets_;
// extern int cli_nios_fd[]; // FIXME FIXME FIXME FIXME FIXME put in specific header
// static short nios_ok_;
// static struct masterInfo masterInfo_;
// static int masterknown_;
// static char *indexfilter_;
// static char *stripDomains_;
// static struct hTab conn_table;

// static char **environ;

// void initconntbl_ (void);
// void inithostsock_ (void);
// int gethostbysock_ (int, char *);
// int _isconnected_ (char *, int *);
// int _getcurseqno_ (char *);
// void _setcurseqno_ ( const char *hostname, unsigned int currentSN);
void _lostconnection_ (char *);
// int _findmyconnections_ (struct connectEnt **);

int setLockOnOff_ (int, time_t, char *hname);

struct lsRequest *lsReqHandCreate_ (int, int, int, void *, requestCompletionHandler, appCompletionHandler, void *);
void lsReqHandDestroy_ (struct lsRequest *);

int lsConnWait_ (char *);
int lsMsgWait_ (int inTidCnt, pid_t *tidArray, int *rdyTidCnt, int inFdCnt, int *fdArray, int *rdyFdCnt, int *outFdArray, struct timeval *timeout, int options);
int lsMsgRdy_ (pid_t taskid, size_t *msgLen);
int lsMsgRcv_ (pid_t taskid, char *buffer, size_t len, int options);
int lsMsgSnd_ (pid_t taskid, char *buffer, size_t len, int options);
int lsMsgSnd2_ (int *sock, unsigned short opcode, char *buffer, size_t len, int options);
int lsReqTest_ (LS_REQUEST_T *);
int lsReqWait_ (LS_REQUEST_T *, int);
void lsReqFree_ (LS_REQUEST_T *);
int lsRSig_ (char *host, pid_t rid, int sig, int options);
int lsRGetpid_ (int, int);
void *lsRGetpidAsync_ (int, int *);
LS_REQUEST_T *lsIRGetRusage_ (pid_t rpid, struct jRusage * ru, appCompletionHandler appHandler, void *appExtra, int options);
int lsRGetRusage (int, struct jRusage *, int);
int lsGetRProcRusage (char *, int, struct jRusage *, int);
LS_REQUEST_T *lsGetIRProcRusage_ (char *, int, int, struct jRusage *, appCompletionHandler, void *);

// int initenv_ (struct config_param *, char *);
// int readconfenv_ (struct config_param *, struct config_param *, char *);

int callLim_ (enum limReqCode, void *, bool_t (*)(), void *, bool_t (*)(), char *, int, struct LSFHeader *);
int initLimSock_ (void);

void err_return_ (enum limReplyCode);

struct hostLoad *loadinfo_ (char *resReq, struct decisionReq *loadReqPtr, char *fromhost, unsigned long *numHosts, char ***outnlist);

// struct hostent *Gethostbyname_ (char *);
// short getRefNum_ (void);
// unsigned int isint_ (char *);
// char islongint_ (char *);
// int isdigitstr_ (char *);
// LS_LONG_INT atoi64_ (char *);
// char *chDisplay_ (char *);
// void strToLower_ (char *);
// void initLSFHeader_ (struct LSFHeader *);
// int isMasterCrossPlatform (void);
// int isAllowCross (char *);

char **placement_ (char *resReq, struct decisionReq *placeReqPtr, char *fromhost, size_t *num);

// int sig_encode (int);    
// int sig_decode (int);
// int getSigVal (char *);
// char *getSigSymbolList (void);
// char *getSigSymbol (int);

// typedef struct svrsock struct svrsock;
// int setLSFChanSockOpt_ (int newOpt);

// int CreateSock_ (int);
// int CreateSockEauth_ (int);
// int Socket_ (int, int, int);
// int get_nonstd_desc_ (int);
// int TcpCreate_ (int, int);
// int opensocks_ (int);
// struct svrsock *svrsockCreate_ (unsigned short, int, struct sockaddr_in *, int);
// int svrsockAccept_ (struct svrsock *, int);
// char *svrsockToString_ (struct svrsock *);
// void svrsockDestroy_ (struct svrsock *);
// int TcpConnect_ (char *, unsigned short, struct timeval *);

// char *getMsgBuffer_ (int fd, size_t *bufferSize);

// int expSyntax_ (char *);

int tid_register (pid_t taskid, int socknum, unsigned short taskPort, char *host, bool_t doTaskInfo);
int tid_remove (pid_t taskid);
struct tid *tid_find (pid_t taskid);
struct tid *tidFindIgnoreConn_(pid_t taskid);
void tid_lostconnection (int);
int tidSameConnection_ (int, int *, int **);

int callRes_ (int s, enum resCmd cmd, char *data, char *reqBuf, size_t reqLen, bool_t (*xdrFunc) (), int *rd, struct timeval *timeout, struct lsfAuth *auth);
int sendCmdBill_ (int, enum resCmd, struct resCmdBill *, int *, struct timeval *);
void ls_errlog (FILE * fd, const char *fmt, ...)
#if defined(__GNUC__) && defined(CHECK_PRINTF)
	__attribute__ ((format (printf, 2, 3)))
#endif
;

// void ls_verrlog (FILE * fd, const char *fmt, va_list ap);
