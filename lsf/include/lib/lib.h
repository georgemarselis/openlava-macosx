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
 
#include "lsf.h"
#include "daemons/liblimd/limout.h"
#include "daemons/libresd/resd.h"
#include "lib/lproto.h"
#include "lib/hdr.h"
#include "lib/xdrlim.h"
#include "lib/xdr.h"
#include "lib/mls.h"
#include "lsb/lsb.h"

struct taskMsg
{
    char *inBuf;
    char *outBuf;
    size_t len;
};

struct lsQueue *requestQ;
unsigned int requestSN;

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
    u_short taskPort;
    char padding[2];
    int rtid;
    int sock;
    int refCount;
    int pid;
    char *host;
    struct lsQueue *tMsgQ;
    struct tid *link;
};

// #define getpgrp(n)  getpgrp()

#ifndef LOG_PRIMASK
#define LOG_PRIMASK     0xf
#define LOG_MASK(pri)   (1 << (pri))
#define LOG_UPTO(pri)   ((1 << ((pri)+1)) - 1)
#endif

#ifndef LOG_PRI
#define LOG_PRI(p)      ((p) & LOG_PRIMASK)
#endif


#define MIN_REF_NUM          1000
#define MAX_REF_NUM          32760

// FIXME FIXME FIXME FIXM FIXME remove (char *)
#define packshort_(buf, x)       memcpy(buf, (char *)&(x), sizeof(short))
#define packint_(buf, x)         memcpy(buf, (char *)&(x), sizeof(int))
#define pack_lsf_rlim_t_(buf, x) memcpy(buf, (char *)&(x), sizeof(lsf_rlim_t))

enum
{
// start inclusion from resd.h
    LSF_RES_DEBUG,
    LSF_SERVERDIR,
    LSF_AUTH,
    LSF_LOGDIR,
    LSF_ROOT_REX,
    LSF_LIM_PORT,
    LSF_RES_PORT,
    LSF_ID_PORT,
    LSF_USE_HOSTEQUIV,
    LSF_RES_ACCTDIR,
    LSF_RES_ACCT,
    LSF_DEBUG_RES, // FIXME FIXME FIXME LSF_RES_DEBUG and LSF_DEBUG_RES , replace with single tag
    LSF_TIME_RES,
    LSF_LOG_MASK,
    LSF_RES_RLIMIT_UNLIM,
    LSF_CMD_SHELL,
    LSF_ENABLE_PTY,
    LSF_TMPDIR,
    LSF_BINDIR,
    LSF_LIBDIR,
    LSF_RES_TIMEOUT,
    LSF_RES_NO_LINEBUF,
    LSF_MLS_LOG,
//////////////////////// end inclusion from resd.h
    LSF_CONFDIR,
    // LSF_SERVERDIR,
    LSF_LIM_DEBUG,
    // LSF_RES_DEBUG,
    LSF_STRIP_DOMAIN,
    // LSF_LIM_PORT,
    // LSF_RES_PORT,
    // LSF_LOG_MASK,
    LSF_SERVER_HOSTS,
    // LSF_AUTH,
    // LSF_USE_HOSTEQUIV,
    // LSF_ID_PORT,
    // LSF_RES_TIMEOUT,
    LSF_API_CONNTIMEOUT,
    LSF_API_RECVTIMEOUT,
    LSF_AM_OPTIONS,
    // LSF_TMPDIR,
    // LSF_LOGDIR,
    LSF_SYMBOLIC_LINK,
    LSF_MASTER_LIST,
    // LSF_MLS_LOG,
    LSF_INTERACTIVE_STDERR,
    NO_HOSTS_FILE
} status;

#define LSF_RES_DEBUG                 0
#define LSF_SERVERDIR                 1
#define LSF_ENVDIR                    2 // FIXME FIXME newly inserted variable
#define LSF_LOGDIR                    3
#define LSF_LIM_PORT                  5
#define LSF_RES_PORT                  6
#define LSF_ID_PORT                   9
#define LSF_RES_ACCTDIR               9
#define LSF_RES_ACCT                  10
#define LSF_AUTH                      10
#define LSF_DEBUG_RES                 11
#define LSF_TIME_RES                  12
#define LSF_USE_HOSTEQUIV             12
// #define LSF_ROOT_REX               13
#define LSF_RES_RLIMIT_UNLIM          14
#define LSF_CMD_SHELL                 15
#define LSF_ENABLE_PTY                16
#define LSF_TMPDIR                    17
#define LSF_LOG_MASK                  19
// #define LSF_BINDIR                 18
#define LSF_RES_TIMEOUT               20  // FIXME FIXME move to resd.h
#define LSF_BINDIR                    20
#define LSF_RES_NO_LINEBUF            21
#define LSF_CONFDIR                   23
#define LSF_GETPWNAM_RETRY            28
#define LSF_AUTH_DAEMONS              33
#define LSF_LIBDIR                    40
#define LSF_MLS_LOG                   46


#define LSB_DEBUG                     0
#define LSB_CONFDIR                   1
// #define LSB_SHAREDIR               4
#define LSB_MAILTO                    5
#define LSB_MAILPROG                  6
// #define LSB_MBD_PORT               8
// #define LSB_SBD_PORT               7

#define LSB_CRDIR                     11
#define LSB_DEBUG_MBD                 14
#define LSB_DEBUG_SBD                 15
#define LSB_TIME_MBD                  16
#define LSB_TIME_SBD                  17
#define LSB_SIGSTOP                   18

#define LSB_MBD_CONNTIMEOUT           21
#define LSB_SBD_CONNTIMEOUT           22
#define LSB_MBD_MAILREPLAY            24
#define LSB_MBD_MIGTOPEND             25
#define LSB_SBD_READTIMEOUT           26
#define LSB_MBD_BLOCK_SEND            27
#define LSB_MEMLIMIT_ENFORCE          29

#define LSB_STOP_IGNORE_IT            31
#define LSB_HJOB_PER_SESSION          32
#define LSB_REQUEUE_HOLD              34
#define LSB_SMTP_SERVER               35
#define LSB_MAILSERVER                36
#define LSB_MAILSIZE_LIMIT            37
#define LSB_REQUEUE_TO_BOTTOM         38

#define LSB_QPOST_EXEC_ENFORCE        41
#define LSB_MIG2PEND                  42
// #define LSB_UTMP                   43
#define LSB_JOB_CPULIMIT              44
#define LSB_RENICE_NEVER_AT_RESTART   45
#define LSB_JOB_MEMLIMIT              47
#define LSB_MOD_ALL_JOBS              48
#define LSB_SET_TMPDIR                49

#define LSB_PTILE_PACK                50
#define LSB_SBD_FINISH_SLEEP          51
#define LSB_VIRTUAL_SLOT              52
#define LSB_STDOUT_DIRECT             53

#define LIM_NO_MIGRANT_HOSTS          55

// struct config_param genParams_[] = {
//     { "LSF_CONFDIR",            NULL },
//     { "LSF_SERVERDIR",          NULL },
//     { "LSF_LIM_DEBUG",          NULL },
//     { "LSF_RES_DEBUG",          NULL },
//     { "LSF_STRIP_DOMAIN",       NULL },
//     { "LSF_LIM_PORT",           NULL },
//     { "LSF_RES_PORT",           NULL },
//     { "LSF_LOG_MASK",           NULL },
//     { "LSF_SERVER_HOSTS",       NULL },
//     { "LSF_AUTH",               NULL },
//     { "LSF_USE_HOSTEQUIV",      NULL },
//     { "LSF_ID_PORT",            NULL },
//     { "LSF_RES_TIMEOUT",        NULL },
//     { "LSF_API_CONNTIMEOUT",    NULL },
//     { "LSF_API_RECVTIMEOUT",    NULL },
//     { "LSF_AM_OPTIONS",         NULL },
//     { "LSF_TMPDIR",             NULL },
//     { "LSF_LOGDIR",             NULL },
//     { "LSF_SYMBOLIC_LINK",      NULL },
//     { "LSF_MASTER_LIST",        NULL },
//     { "LSF_MLS_LOG",            NULL },
//     { "LSF_INTERACTIVE_STDERR", NULL },
//     { "HOSTS_FILE",             NULL },
//     { "LSB_SHAREDIR",           NULL },
//     { NULL,                     NULL }
// };

struct config_param lsfParams[] = {
    { "LSF_SERVERDIR", NULL },
    { "LSF_CONFDIR",   NULL },
    { "LSF_CONFDIR",   NULL },
    { "LSF_SHAREDIR",  NULL },
    { NULL,            NULL }
};

enum { 
    NIOS2RES_EOF,
    NIOS2RES_HEARTBEAT,
    NIOS2RES_SETTTY,
    NIOS2RES_SIGNAL,
    NIOS2RES_STDIN,
    NIOS2RES_TIMEOUT
} niosResCmd;

enum {
    RES2NIOS_CONNECT,
    RES2NIOS_EOF,
    RES2NIOS_NEWTASK,
    RES2NIOS_REQUEUE,
    RES2NIOS_STATUS,
    RES2NIOS_STDERR,
    RES2NIOS_STDOUT
} resNiosCmd;

typedef enum status genparams_t;

#define AM_LAST  (!(genParams_[LSF_AM_OPTIONS].paramValue && \
                                    strstr(genParams_[LSF_AM_OPTIONS].paramValue, \
                                                            AUTOMOUNT_LAST_STR)))

#define AM_NEVER (genParams_[LSF_AM_OPTIONS].paramValue && \
                                    strstr(genParams_[LSF_AM_OPTIONS].paramValue, \
                                                            AUTOMOUNT_NEVER_STR))


#define LOOP_ADDR       0x7F000001

#define _NON_BLOCK_         0x01
#define _LOCAL_             0x02
#define _USE_TCP_           0x04
#define _KEEP_CONNECT_      0x08
#define _USE_PRIMARY_       0x10
#define _USE_PPORT_         0x20
/* openlava call LSF_SERVER_HOSTS regardless of local lim.
 */
#define _SERVER_HOSTS_ONLY_       0x40

#define PRIMARY    0
#define MASTER     1
#define UNBOUND    2
#define TCP        3

#define RES_TIMEOUT 120
#define NIOS_TIMEOUT 120

#define NOCODE 10000

#define MAXCONNECT    256

#define RSIG_ID_ISTID   0x01

#define RSIG_ID_ISPID   0x02

#define RSIG_KEEP_CONN  0x04

#define RID_ISTID       0x01
#define RID_ISPID       0x02

#define NO_SIGS (~(sigmask(SIGTRAP) | sigmask(SIGEMT)))
#define SET_LSLIB_NIOS_HDR(hdr,opcode,l) \
        { (hdr).opCode = (opcode); (hdr).len = (l); }

#define CLOSEFD(s) if ((s) >= 0) {close((s)); (s) = -1;}

extern struct sockaddr_in sockIds_[]; // FIXME FIXME FIXME FIXME FIXME put in specific header
extern int limchans_[]; // FIXME FIXME FIXME FIXME FIXME put in specific header

extern struct config_param genParams_[]; // FIXME FIXME FIXME FIXME FIXME put in specific header
struct sockaddr_in limSockId_;
struct sockaddr_in limTcpSockId_;
char rootuid_;
struct sockaddr_in res_addr_;
fd_set connection_ok_;
int currentsocket_;
int totsockets_;
extern int cli_nios_fd[]; // FIXME FIXME FIXME FIXME FIXME put in specific header
short nios_ok_;
struct masterInfo masterInfo_;
int masterknown_;
char *indexfilter_;
char *stripDomains_;
hTab conn_table;

char **environ;

void initconntbl_ (void);
void inithostsock_ (void);
int connected_ (char *, int, int, int);
void hostIndex_ (char *, int);
int gethostbysock_ (int, char *);
int _isconnected_ (char *, int *);
int _getcurseqno_ (char *);
void _setcurseqno_ (char *hostname, uint currentSN);
void _lostconnection_ (char *);
int _findmyconnections_ (struct connectEnt **);

int setLockOnOff_ (int, time_t, char *hname);

typedef struct lsRequest LS_REQUEST_T;
typedef struct lsRequest requestType;
typedef int (*requestCompletionHandler) (requestType *);
typedef int (*appCompletionHandler) (LS_REQUEST_T *, void *);


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

struct lsRequest *lsReqHandCreate_ (int, int, int, void *, requestCompletionHandler, appCompletionHandler, void *);
void lsReqHandDestroy_ (struct lsRequest *);

int lsConnWait_ (char *);
int lsMsgWait_ (int inTidCnt, pid_t *tidArray, int *rdyTidCnt, int inFdCnt, int *fdArray, int *rdyFdCnt, int *outFdArray, struct timeval *timeout, int options);
int lsMsgRdy_ (pid_t taskid, size_t *msgLen);
int lsMsgRcv_ (pid_t taskid, char *buffer, size_t len, int options);
int lsMsgSnd_ (pid_t taskid, char *buffer, size_t len, int options);
int lsMsgSnd2_ (int *sock, ushort opcode, char *buffer, size_t len, int options);
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

int initenv_ (struct config_param *, char *);
int readconfenv_ (struct config_param *, struct config_param *, char *);
int ls_readconfenv (struct config_param *, char *);

int callLim_ (enum limReqCode, void *, bool_t (*)(), void *, bool_t (*)(), char *, int, struct LSFHeader *);
int initLimSock_ (void);

void err_return_ (enum limReplyCode);

struct hostLoad *loadinfo_ (char *resReq, struct decisionReq *loadReqPtr, char *fromhost, unsigned long *numHosts, char ***outnlist);

struct hostent *Gethostbyname_ (char *);
short getRefNum_ (void);
unsigned int isint_ (char *);
char islongint_ (char *);
int isdigitstr_ (char *);
LS_LONG_INT atoi64_ (char *);
char *chDisplay_ (char *);
void strToLower_ (char *);
void initLSFHeader_ (struct LSFHeader *);
int isMasterCrossPlatform (void);
int isAllowCross (char *);

char **placement_ (char *resReq, struct decisionReq *placeReqPtr, char *fromhost, size_t *num);

int sig_encode (int);
int sig_decode (int);
int getSigVal (char *);
char *getSigSymbolList (void);
char *getSigSymbol (int);

struct svrsock
{
    int sockfd;
    int port;
    struct sockaddr_in *localAddr;
    int backlog;
    int options;
};

typedef struct svrsock ls_svrsock_t;


#define LS_CSO_ASYNC_NT       (0x0001)
#define LS_CSO_PRIVILEGE_PORT (0x0002)

int setLSFChanSockOpt_ (int newOpt);

int CreateSock_ (int);
int CreateSockEauth_ (int);
int Socket_ (int, int, int);
int get_nonstd_desc_ (int);
int TcpCreate_ (int, int);
int opensocks_ (int);
ls_svrsock_t *svrsockCreate_ (u_short, int, struct sockaddr_in *, int);
int svrsockAccept_ (ls_svrsock_t *, int);
char *svrsockToString_ (ls_svrsock_t *);
void svrsockDestroy_ (ls_svrsock_t *);
int TcpConnect_ (char *, u_short, struct timeval *);

char *getMsgBuffer_ (int fd, size_t *bufferSize);

int expSyntax_ (char *);

int tid_register (pid_t taskid, int socknum, u_short taskPort, char *host, bool_t doTaskInfo);
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

void ls_verrlog (FILE * fd, const char *fmt, va_list ap);
int isPamBlockWait;


