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

/****************************************************************
 * All externs below are defined/initialized in lib/liblsf/res.c
 *
 */
extern int lsf_res_version;
extern int totsockets_;
extern int currentsocket_;
extern fd_set connection_ok_;
extern struct sockaddr_in res_addr_;
extern struct lsQueue *requestQ;
extern unsigned int requestSN;
extern unsigned int requestHighWaterMark;
extern unsigned int globCurrentSN;

#define REQUESTSN ((requestSN < USHRT_MAX) ? requestSN++ : (requestSN=11 , 10))




FILE      *ls_popen(int s, char *command, char *type);
int        ackAsyncReturnCode_(int s, struct LSFHeader *replyHdr);
int        ackReturnCode_(int s);
int        callRes_(int s, enum resCmd cmd, char *data, char *reqBuf, size_t reqLen, bool_t (*xdrFunc )(), int *rd, struct timeval *timeout, struct lsfAuth *auth);
int        do_rstty1_(char *host, int async);
int        do_rstty2_(int s, int io_fd, int redirect, int async);
int        do_rstty_(int s, int io_fd, int redirect);
int        enqueueTaskMsg_(int s, pid_t taskID, struct LSFHeader *msgHdr);
int        expectReturnCode_(int s, pid_t seqno, struct LSFHeader *repHdr);
int        getLimits (struct lsfLimit *);
int        ls_chdir(char *host, char *dir);
int        ls_connect(char *host);
int        ls_pclose(FILE *stream);
int        ls_rkill(pid_t rtid, int sig);
int        ls_rsetenv(char *host, char **envp);
int        ls_rsetenv_async(char *host, char **envp);
int        ls_rstty(char *host);
int        lsConnWait_(char *host);
int        lsGetRProcRusage(char *host, int pid, struct jRusage *ru, int options);
int        lsMsgRcv_(pid_t taskid, char *buffer, size_t len, int options);
int        lsMsgRdy_(pid_t taskid, size_t *msgLen);
int        lsMsgSnd2_(int *sock, unsigned short opcode, char *buffer, size_t len, int options);
int        lsMsgSnd_(pid_t taskid, char *buffer, size_t len, int options);
int        lsMsgWait_(int inTidCnt, pid_t *tidArray, int *rdyTidCnt, int inFdCnt, int *fdArray, int *rdyFdCnt, int *outFdArray, struct timeval *timeout, int options);
int        lsReqCmp_(char *val, char *reqEnt, int hint);
int        lsReqTest_(struct lsRequest *request);
int        lsReqWait_(struct lsRequest *request, int options);
int        lsRGetRusage(int rpid, struct jRusage *ru, int options);
int        lsRSig_(char *host, pid_t rid, int sig, int options);
int        resRC2LSErr_(int resRC);
int        rgetRusageCompletionHandler_(struct lsRequest *request);
int        rsetenv_(char *host, char **envp, int option);
int        rstty_(char *host);
int        rstty_async_(char *host);
int        sendCmdBill_(int s, enum resCmd cmd, struct resCmdBill *cmdmsg, int *retsock, struct timeval *timeout);
int        sendSig_(char *host, pid_t rid, int sig, int options);
struct lsRequest        *lsGetIRProcRusage_(char *host, int tid, pid_t pid, struct jRusage *ru, appCompletionHandler appHandler, void *appExtra);
struct lsRequest        *lsIRGetRusage_(pid_t rpid, struct jRusage *ru, appCompletionHandler appHandler, void *appExtra, int options);
struct lsRequest    *lsReqHandCreate_(pid_t tid, int seqno, int connfd, void *extra, requestCompletionHandler replyHandler, appCompletionHandler appHandler, void *appExtra);
unsigned int         getCurrentSN(void);
unsigned int         setCurrentSN(unsigned int currentSN);
void        _lostconnection_(char *hostName);
void        lsReqFree_(struct lsRequest *request);
void        tMsgDestroy_(void *extra);
