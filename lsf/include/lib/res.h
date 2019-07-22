/* $Id: lib.res.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/hdr.h"
#include "lib/lib.h"
#include "daemons/libresd/resout.h"
/****************************************************************
 * All externs below are defined/initialized in lib/liblsf/res.c
 *
 */

// #define RSETENV_SYNCH   1
// #define RSETENV_ASYNC   2
enum RSETENV {
	RSETENV_SYNCH = 1,
	RSETENV_ASYNC = 2
};

// globals 
int lsf_res_version               = -1;
unsigned int currentsocket_       =  0;
unsigned int requestSN            =  0;
unsigned int requestHighWaterMark =  0;
unsigned int globCurrentSN        =  0;

// static unsigned int requestHighWaterMark =  0;
fd_set connection_ok_;
struct sockaddr_in res_addr_;
struct lsQueue *requestQ;


// #define REQUESTSN ((requestSN < USHRT_MAX) ? requestSN++ : (requestSN=11 , 10))


unsigned int getCurrentSN(void);
unsigned int setCurrentSN(unsigned int currentSN);
int ls_connect(const char *host);
int lsConnWait_(const char *host);
int sendCmdBill_(int s, enum resCmd cmd, struct resCmdBill *cmdmsg, int *retsock, struct timeval *timeout);
int rsetenv_(const char *host, char **envp, int option);
int ls_chdir(const char *host, const char *dir);
struct lsRequest *lsReqHandCreate_(pid_t taskID, unsigned int seqno, int connfd, void *extra, requestCompletionHandler replyHandler, appCompletionHandler appHandler, void *appExtra);
int ackAsyncReturnCode_(int s, struct LSFHeader *replyHdr);
int enqueueTaskMsg_(int s, pid_t taskID, struct LSFHeader *msgHdr);
unsigned int expectReturnCode_(int s, unsigned int seqno, struct LSFHeader *repHdr);
unsigned int resRC2LSErr_(unsigned int resRC);
unsigned int ackReturnCode_(int s);
int getLimits(struct lsfLimit *limits);
int callRes_(int s, enum resCmd cmd, const char *data, const char *reqBuf, size_t reqLen, bool_t (*xdrFunc )(), int *rd, struct timeval *timeout, struct lsfAuth *auth);
int do_rstty1_(const char *host, int async);
int do_rstty2_(int s, int io_fd, int redirect, int async);
int rgetRusageCompletionHandler_(struct lsRequest *request);
struct lsRequest *lsIRGetRusage_(pid_t rpid, struct jRusage *ru, appCompletionHandler appHandler, void *appExtra, int options);
int lsGetRProcRusage(const char *host, int pid, struct jRusage *ru, int options);
struct lsRequest *lsGetIRProcRusage_(const char *host, int tid, pid_t pid, struct jRusage *ru, appCompletionHandler appHandler, void *appExtra);
int lsRGetRusage(int rpid, struct jRusage *ru, int options);
int sendSig_(const char *host, pid_t rpid, int sig, int options);
int lsRSig_ (const char *host, pid_t rpid, int sig, int options);
int ls_rkill(pid_t rtid, int sig);
int lsMsgRdy_(unsigned int taskid, size_t *msgLen);
void tMsgDestroy_(void *extra);
int lsMsgRcv_(unsigned int taskid, const char *buffer, size_t len, int options);

int lsMsgSnd2_( int *socket,  unsigned short opcode, const char *buffer, size_t len, int options);
int lsMsgSnd_ ( unsigned int taskid,                 const char *buffer, size_t len, int options);

int lsMsgWait_(int inTidCnt, unsigned int *tidArray, unsigned int *rdyTidCnt, unsigned int inFdCnt, int *fdArray, unsigned int *rdyFdCnt, int *outFdArray, struct timeval *timeout, int options);
int lsReqCmp_ ( const char *val, struct lsRequest *reqEnt, int hint);
int lsReqTest_(struct lsRequest *request);
int lsReqWait_(struct lsRequest *request, int options);
void lsReqFree_(struct lsRequest *request);
void _lostconnection_(const char *hostName);
