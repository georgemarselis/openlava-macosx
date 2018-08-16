/* $Id: nios.h 397 2007-11-26 19:04:00Z mblack $
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


// #define NL_SETN         29

// #define LSF_NIOS_DEBUG 0
// #define LSF_PTY 1
// #define LSB_INTERACT_MSG_ENH 2
// #define LSB_INTERACT_MSG_INTVAL 3
// #define LSF_NIOS_RES_HEARTBEAT 4
// #define LSF_NIOS_JOBSTATUS_INTERVAL 5
// #define LSB_INTERACT_MSG_EXITTIME 6
#define WAIT_BLOCK(o) (!((o) & WNOHANG))

enum NIOS_PARAMS {
  LSF_NIOS_DEBUG = 0,
  LSF_PTY,
  LSB_INTERACT_MSG_ENH,
  LSB_INTERACT_MSG_INTVAL,
  LSF_NIOS_RES_HEARTBEAT,
  LSF_NIOS_JOBSTATUS_INTERVAL,
  LSB_INTERACT_MSG_EXITTIME
};

struct config_param niosParams[] = {
	{ "LSF_NIOS_DEBUG",              NULL },
	{ "LSF_ENABLE_PTY",              NULL },
	{ "LSB_INTERACT_MSG_ENH",        NULL },
	{ "LSB_INTERACT_MSG_INTVAL",     NULL },
	{ "LSF_NIOS_RES_HEARTBEAT",      NULL },
	{ "LSF_NIOS_JOBSTATUS_INTERVAL", NULL },
	{ "LSB_INTERACT_MSG_EXITTIME",   NULL },
	{ NULL,                          NULL }
};

// #define STDIN_FD  0
// #define STDOUT_FD 1
// #define STDERR_FD 2
enum filedescriptors { // FIXME FIXME FIXME eliminate need for this enum
	STDIN_FD = 0,
	STDOUT_FD,
	STDERR_FD
};

// extern void checkJobStatus (int numTries);

// #define MSG_POLLING_INTR 60
const unsigned short MSG_POLLING_INTR = 60;
// #define NIOS_MAX_TASKTBL       10024
const unsigned int NIOS_MAX_TASKTBL = 10024;
// #define MAX_TRY_TIMES           20
const unsigned int MAX_TRY_TIMES = 20;
// #define ERR_SYSTEM      122
const unsigned short ERR_SYSTEM = 122;
// #define MIN_CPU_TIME 0.0001
const float MIN_CPU_TIME = 0.0001f;
// #define BLANKLEN   22
const unsigned short BLANKLEN = 22;
// #define WIDTH      80
const unsigned short WIDTH = 80;

// extern LS_LONG_INT atoi64_ (char *ptr);
// extern int requeued;

pid_t niosPid;

int cursor = 0;
int chfd;
pid_t ppid;
int usepty;
int niosSyncTasks = 0;
int lineBuffered = 1;
char *taggingFormat = (char *) NULL;
int stdoutSync = 0;
int heartbeatInterval = 0;
int jobStatusInterval = 0;
int standalone = FALSE;
int niosSbdMode = FALSE;
unsigned long jobId = 0;
int pendJobTimeout = 0;
int msgInterval = 0;

fd_set nios_rmask;
fd_set nios_wmask;
int endstdin;
int io_fd;
int directecho = FALSE;
int inbg;
int remon;
char buf[BUFSIZ];
int niosDebug = 0;
int maxtasks;
int maxfds;


int stdinBufEmptyEvent = 0;


int exit_sig = 0;
int exit_status = 0;
int got_eof = FALSE;
int got_status = FALSE;
int callbackAccepted = FALSE;
int sent_tstp = FALSE;
int msgEnabled = FALSE;
int standaloneTaskDone = 0;

int forwardTSTP = 0;

int cli_nios_fd[2] = {-1, -1 };
int standalone;
int niosSbdMode;
int heartbeatInterval;
int jobStatusInterval;
int pendJobTimeout;
int msgInterval;
unsigned long jobId;


typedef enum
{
	LIB_NIOS_RTASK,
	LIB_NIOS_RWAIT,
	LIB_NIOS_REM_ON,
	LIB_NIOS_REM_OFF,
	LIB_NIOS_SETSTDIN,
	LIB_NIOS_GETSTDIN,
	LIB_NIOS_EXIT,
	LIB_NIOS_SUSPEND,
	LIB_NIOS_SETSTDOUT,
	LIB_NIOS_SYNC
} libNiosRequest;

typedef enum
{
	JOB_STATUS_ERR,
	JOB_STATUS_UNKNOWN,
	JOB_STATUS_FINISH,
	JOB_STATUS_KNOWN
} JOB_STATUS;

typedef enum
{
	CHILD_OK,
	NONB_RETRY,
	CHILD_FAIL,
	REM_ONOFF,
	STDIN_FAIL,
	STDIN_OK,
	NIOS_OK,
	STDOUT_FAIL,
	STDOUT_OK,
	SYNC_FAIL,
	SYNC_OK
} libNiosReply;


struct lslibNiosHdr
{
	int opCode;
	char padding[4];
	size_t len;
};


struct lslibNiosWaitReq
{
	struct lslibNiosHdr hdr;
	struct
	{
		int options;
		int tid;
	} r;
};

struct lslibNiosWaitReply
{
	struct lslibNiosHdr hdr;
	struct
	{
		int pid;
		int status;
		struct rusage ru;
	} r;
};

struct lslibNiosRTask
{
	struct lslibNiosHdr hdr;
	struct
	{
		int pid;
		struct in_addr peer;
	} r;
};

struct lslibNiosStdout
{
	struct lslibNiosHdr hdr;
	struct
	{
		int set_on;
		char padding[4];
		size_t len;
	} r;
	char *format;
};

struct lslibNiosStdin
{
	struct lslibNiosHdr hdr;
	struct
	{
		int set_on;
		char padding[4];
		size_t len;
	} r;
	int *rpidlist;
};

struct lslibNiosGetStdinReply
{
	struct lslibNiosHdr hdr;
	int *rpidlist;
};

struct finishStatus
{
	int got_eof;
	int got_status;
	int sendSignal;
};

void serv (char **, int);
void PassSig (int);

void exSuspend (struct lslibNiosHdr *);
void do_newtask (void);
void emusig (int, int);
void reset_uid (void);
void conin (int);
void setStdout (struct lslibNiosHdr *);
void setStdin (struct lslibNiosHdr *);
void getStdin (struct lslibNiosHdr *);
void rtask (struct lslibNiosHdr *);
void rwait (struct lslibNiosHdr *);
void exExit (struct lslibNiosHdr *);
void remOn (struct lslibNiosHdr *);
void remOff (struct lslibNiosHdr *);
int die (void);
int acceptCallBack (int);

int cmpJobStates (struct jobInfoEnt *);
int printJobSuspend ( unsigned long jobId );
void prtJobStateMsg (struct jobInfoEnt *, struct jobInfoHead *);
char *get_status (struct jobInfoEnt *);
struct loadIndexLog *initLoadIndex (void);
void prtLine (char *);
void JobExitInfo (void);
void checkPendingJobStatus (int s);
JOB_STATUS getJobStatus ( unsigned long jobId, struct jobInfoEnt **job, struct jobInfoHead **jobHead);
int JobStateInfo ( unsigned long jobId );
int ls_niosetdebug (int);
void kill_self (int, int);
char *getTimeStamp (void);
