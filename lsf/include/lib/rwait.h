// Added by George Marselis <george@marsel.is>

#pragma once

#include "lsf.h"

/* #define SIGEMT SIGBUS */
// #define LS_CSO_ASYNC_NT       (0x0001)
// #define LS_CSO_PRIVILEGE_PORT (0x0002)
static enum LS_CSO {
	LS_CSO_ASYNC_NT       = 0x0001,
	LS_CSO_PRIVILEGE_PORT = 0x0002
} LS_CSO;

static int isPamBlockWait = 0;

/* rwait.c */
int  ls_rwait     ( LS_WAIT_T *status, int options, struct rusage *ru );
int  ls_rwaittid  ( int tid, LS_WAIT_T *status, int options, struct rusage *ru );
int  rwait_       ( int tid, LS_WAIT_T *status, int options, struct rusage *ru );
int  readWaitReply( LS_WAIT_T *status, struct rusage *ru );
void restartRWait ( sigset_t oldMask );
void usr1Handler  ( int sig );
