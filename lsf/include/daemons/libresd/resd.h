/* $Id: res.h 397 2007-11-26 19:04:00Z mblack $
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

#include <tirpc/rpc/types.h>
#include <tirpc/rpc/xdr.h>
#include <stdio.h>
#include <stdlib.h>

#include "lsf.h"
#include "config.h"
#include "lib/xdr.h"
#include "lib/osal.h"
#include "lib/lproto.h"
#include "libint/list.h"
#include "libint/intlibout.h"
#include "daemons/libresd/resout.h"
#include "daemons/libresd/resd.h"

// #ifndef _BSD
// #define _BSD
// #endif

// typedef gid_t GETGROUPS_T;

/* LOL
*
#ifdef PROJECT_CST
#  undef _LS_VERSION_
#  ifdef __STDC__
#    ifdef DATE
#      define _LS_VERSION_ ("LSF 2.2c, " DATE "\nCopyright 1992-1996 Platform Computing Corporation\n")
#    else
#      define _LS_VERSION_ ("LSF 2.2c, " __DATE__ "\nCopyright 1992-1996 Platform Computing Corporation\n")
#    endif
#  else
#    define _LS_VERSION_ ("LSF 2.2c \nCopyright 1992-1996 Platform Computing Corporation\n")
#  endif
#endif 
*/

// #define UTMP_CHECK_CODE "sbdRes"

const char UTMP_CHECK_CODE[ ] = "sbdRes";


#define MAXCLIENTS_HIGHWATER_MARK 100
#define MAXCLIENTS_LOWWATER_MARK  1
// const unsigned int MAXCLIENTS_HIGHWATER_MARK = 100;
// const unsigned int MAXCLIENTS_LOWWATER_MARK  = 1;


// #define DOREAD      0
// #define DOWRITE     1
// #define DOEXCEPTION 2
// #define DOSTDERR    3

enum DO {
	DOREAD      = 0,
	DOWRITE     = 1,
	DOEXCEPTION = 2,
	DOSTDERR    = 3
} DO;

// #define PTY_TEMPLATE     "/dev/ptyXX"

const char PTY_TEMPLATE[ ] = "/dev/ptyXX";

// #define PTY_SLAVE_INDEX  (sizeof(PTY_TEMPLATE) - 6)
// #define PTY_ALPHA_INDEX  (sizeof(PTY_TEMPLATE) - 3)
// #define PTY_DIGIT_INDEX  (sizeof(PTY_TEMPLATE) - 2)

const unsigned int PTY_SLAVE_INDEX = sizeof(PTY_TEMPLATE) - 6; // FIXME FIXME FIXEME - WAT
const unsigned int PTY_ALPHA_INDEX = sizeof(PTY_TEMPLATE) - 3; // FIXME FIXME FIXEME - THE
const unsigned int PTY_DIGIT_INDEX = sizeof(PTY_TEMPLATE) - 2; // FIXME FIXME FIXEME - FUCK

// #define PTY_FIRST_ALPHA  'p'
// #define PTY_LAST_ALPHA   'v'
const char PTY_FIRST_ALPHA = 'p';
const char PTY_LAST_ALPHA  = 'v';

#define BUFSTART(x)      ((char *) ((x)->buf) + sizeof(struct LSFHeader))

// #define INVALID_FD      (-1)
enum FD {
	INVALID_FD = -1
} FD;

#define FD_IS_VALID(x)  ((x) >= 0 && (x) < sysconf(_SC_OPEN_MAX) )
#define FD_NOT_VALID(x) ( ! FD_IS_VALID(x))

#define CLOSE_IT(fd)     if (fd>=0) {close(fd); fd = INVALID_FD;}

enum SBD_FLAG {
	SBD_FLAG_STDIN  = 0x1,
	SBD_FLAG_STDOUT = 0x2,
	SBD_FLAG_STDERR = 0x4,
	SBD_FLAG_TERM   = 0x8
} SBD_FLAG;

enum RES_RPID_KEEPPID {
	RES_RPID_KEEPPID = 0x01
};

enum RES_RID {
	RES_RID_ISTID = 0x01,
	RES_RID_ISPID = 0x02
} RES_RID;

struct relaylinebuf// FIXME FIXME FIXME FIXME struct relaylinebuf and struct relaybuf are identical; should be consolidated
{
	char *bp;
	// char buf[ LINE_BUFSIZ + sizeof (struct LSFHeader)];
	char buf[ LINE_BUFSIZ * sizeof (struct LSFHeader) ]; // FIXME FIXME FIXME FIXME must create a pseudo-constructor which allocates the correct size
	int bcount;
	char padding1[4];
};

// typedef struct relaylinebuf RelayLineBuf;

struct relaybuf // FIXME FIXME FIXME FIXME struct relaylinebuf and struct relaybuf are identical; should be consolidated
{
	char *bp;
	// char buf[ LINE_BUFSIZ + sizeof (struct LSFHeader)];
	char buf[ LINE_BUFSIZ * sizeof (struct LSFHeader)]; // FIXME FIXME FIXME FIXME must create a pseudo-constructor which allocates the correct size
	int bcount;
	char padding1[4];
};

// typedef struct relaybuf RelayBuf;

struct channel
{
	int fd;
	int rcount;
	int wcount;
	char padding1[ 4 ];
	struct relaybuf *rbuf;
	struct relaybuf *wbuf;
};

// typedef struct channel Channel;

struct outputchannel
{
	int fd;
	int endFlag;
	int retry;
	int bytes;
	struct relaylinebuf buffer;
};

//typedef struct outputchannel outputChannel;

struct niosChannel
{
	int fd;
	int rcount;
	int wcount;
	int opCode;
	struct relaybuf *rbuf;
	struct relaylinebuf *wbuf;
};

// struct nioschannel niosChannel;  // FIXME FIXME FIXME remove typedef from struct

//typedef 
struct ttyStruct
{
	struct termios attr;
	struct winsize ws;
#    if defined(hpux) || defined(__hpux)
	struct ltchars hp_ltchars;
#    endif
}; // ttyStruct;

struct client
{
	pid_t ruid;
	gid_t gid;
	int client_sock;
	int ngroups;
	char *username;
	char *clntdir;
	char *homedir;
	char **env;
	struct tty_struct *tty;
	GETGROUPS_T groups[NGROUPS_MAX];
	char padding1[8];
	struct hostent hostent;
	struct lenData eexec;
};

struct child
{
	char username[MAX_LSF_NAME_LEN];
	char fromhost[MAXHOSTNAMELEN];
	char slavepty[sizeof (PTY_TEMPLATE)];
	char padding1[5];
	pid_t rpid;
	pid_t pid;
	int refcnt;
	int info;
	int stdio;
	int rexflag;
	int endstdin;
	int sent_eof;
	int sent_status;
	char server;
	char c_eof;
	char running;
	char sigchild;
	LS_WAIT_T wait;
	int stdin_up;
	char *cwd;
	char **cmdln;
	time_t dpTime;
	struct relaybuf *i_buf;
	struct client *backClnPtr;
	struct sigStatusUsage *sigStatRu;
	struct outputchannel *std_out;
	struct outputchannel *std_err;
	struct niosChannel *remsock;
};

struct resChildInfo
{
	unsigned short parentPort;
	char padding1[2];
	int currentRESSN;
	int res_status;
	char padding[4];
	struct resConnect *resConnect;
	struct lsfAuth *lsfAuth;
	struct passwd *pw;
	struct hostent *host;
};



// typedef 
struct taggedConn
{
	int rtag;
	int wtag;
	int num_duped;
	char padding1[4];
	int *task_duped;
	struct niosChannel *sock;
}; //taggedConn_t;

typedef struct resNotice
{
	struct resNotice *forw;
	struct resNotice *back;
	pid_t rpid;
	int retsock;
	int opCode;
	enum resAck {

		RESE_OK,
		RESE_SIGCHLD,
		RESE_NOMORECONN,
		RESE_BADUSER,
		RESE_ROOTSECURE,
		RESE_DENIED,
		RESE_REQUEST,
		RESE_CALLBACK,
		RESE_NOMEM,
		RESE_FATAL,
		RESE_PTYMASTER,
		RESE_PTYSLAVE,
		RESE_SOCKETPAIR,
		RESE_FORK,
		RESE_REUID,
		RESE_CWD,
		RESE_INVCHILD,
		RESE_KILLFAIL,
		RESE_VERSION,
		RESE_DIRW,
		RESE_NOLSF_HOST,
		RESE_NOCLIENT,
		RESE_RUSAGEFAIL,
		RESE_RES_PARENT,
		RESE_FILE,
		RESE_NOVCL,
		RESE_NOSYM,
		RESE_VCL_INIT,
		RESE_VCL_SPAWN,
		RESE_EXEC,
		RESE_ERROR_LAST,
		RESE_MLS_INVALID,
		RESE_MLS_CLEARANCE,
		RESE_MLS_DOMINATE,
		RESE_MLS_RHOST
		} ack;
	struct sigStatusUsage *sigStatRu;
} resNotice_t;

struct resSignal
{
	pid_t pid;
	int sigval;
};

struct resCmdBill
{
	unsigned short retport;
	char padding1[2];
	pid_t rpid;
	int filemask;
	int priority;
	int options;
	char cwd[MAX_PATH_LEN]; // FIXME FIXME FIXME FIXME turn into dynamic allocation
	// char *cwd;
	char padding2[4];
	char **argv;
	struct lsfLimit lsfLimits[LSF_RLIM_NLIMITS];  // this should be changed to a pointer
};

struct resSetenv
{
	char **env;
};

struct resRKill
{
  pid_t rid;
  int whatid;
  int signal;
};

struct resPid
{
  pid_t rpid;
  pid_t pid;
};

struct resRusage
{
  pid_t rid;
  int whatid;
  int options;
};

struct resChdir
{
  char dir[MAX_FILENAME_LEN];
};

struct resControl
{
  int opCode;
  int data;
};


struct resStty
{
  struct termios termattr;
  struct winsize ws;
};

struct niosConnect
{
  pid_t rpid;
  int exitStatus;
  int terWhiPendStatus;
};

struct niosStatus
{
	enum resAck ack;
	char padding[4];

	struct sigStatusUsage
	{
		int ss;
		char padding[4];
		struct rusage *ru;
	} s;
};

/*********************************************/

int rexecPriority;
struct child **children;
// int  child_cnt    = 0;
// int  client_cnt   = 0;
unsigned int  child_cnt    = 0;
unsigned int  client_cnt   = 0;
char  *Myhost     = NULL;
char  *myHostType = NULL;

int lastChildExitStatus;

int sbdMode;
int sbdFlags;


int accept_sock;
char child_res;
char child_go;
char res_interrupted;
char *gobuf;
char allow_accept;

int child_res_port;
fd_set readmask;
int writemask;
int exceptmask;

int ctrlSock;

int res_status;

char *lsbJobStarter;

int restart_argc;
char **restart_argv;
char *env_dir;
int  rexecPriority = 0;

struct  client *clients[MAXCLIENTS_HIGHWATER_MARK + 1];


// taggedConn_t 
struct  taggedConn conn2NIOS;
struct _list *resNotifyList = NULL;

bool_t  vclPlugin = FALSE;

char  child_res = ' ';
char  child_go =  '\0';
char  res_interrupted = ' ';
char  *gobuf = NULL;

int  accept_sock = INVALID_FD;
char  allow_accept = 1;

int  ctrlSock = INVALID_FD;
struct  sockaddr_in ctrlAddr;

int  child_res_port = INVALID_FD;
int  parent_res_port = INVALID_FD;


int  on = 1;
int  off = 0;
int  debug = 0;
int  res_status = 0;

char  *lsbJobStarter = NULL;

int  sbdMode = FALSE;
int  sbdFlags = 0;

int  lastChildExitStatus = 0;

char  res_logfile[MAX_PATH_LEN];
int  res_logop;

int  restart_argc    = 0;
char  **restart_argv = NULL;

char  *env_dir = NULL;

unsigned int globCurrentSN;

const unsigned short SIG_NT_CTRLC     = 2000;
const unsigned short SIG_NT_CTRLBREAK = 2001;

const unsigned short RES_REPLYBUF_LEN = 4096;
const int RESS_LOGBIT = 0x00000001;

// struct  config_param resConfParams[] = {
// 	{"LSB_UTMP", NULL},
// 	{NULL,       NULL}
// };

/* daemons/resd/getproc.c */
// int getPPSGids_(int pid, int *ppid, int *sid, int *pgid);

/* daemons/resd/handler.c */
// void doacceptconn(void);
// void childAcceptConn(int s, struct passwd *pw, struct lsfAuth *auth, struct resConnect *connReq, struct hostent *hostp);
// void doclient(struct client *cli_ptr);
// void dochild_info(struct child *chld, int op);
// void doResParentCtrl(void);
// enum resAck sendResParent(struct LSFHeader *msgHdr, char *msgBuf, bool_t (*xdrFunc )());
// void delete_child(struct child *cp);
// void dochild_stdio(struct child *chld, int op);
// int resSignal(struct child *chld, struct resSignal sig);
// void child_handler(void);
// void child_handler_ext(void);
// void term_handler(int signum);
// void sigHandler(int signum);
// int matchExitVal(int val, char *requeueEval);
// int sendReturnCode(int s, int code);
// void child_channel_clear(struct child *chld, struct outputchannel *channel);
// int lsbJobStart(char **jargv, u_short retPort, char *host, int usePty);
// void dumpClient(struct client *client, char *why);
// void dumpChild(struct child *child, int operation, char *why);
// void dochild_buffer(struct child *chld, int op);
// void donios_sock(struct child **children, int op);
// int deliver_notifications( struct _list *list);

/* daemons/resd/init.c */
// void init_res(void);
// int resParent(int s, struct passwd *pw, struct lsfAuth *auth, struct resConnect *connReq, struct hostent *hostp);
// void resChild(char *arg, char *envdir);

/* daemons/resd/misc.c */
// bool_t xdr_resChildInfo(XDR *xdrs, struct resChildInfo *childInfo, struct LSFHeader *hdr);

/* daemons/resd/pty.c */
// void ptyreset(void);
// int ptymaster(char *line);
// int ptyslave(char *tty_name);
// char *pty_translate(char *pty_name);
// int check_valid_tty(char *tty_name);

/* daemons/resd/res.c */
void resd_usage(char *cmd);
void initSignals(void);
void getMaskReady(fd_set *rm, fd_set *wm, fd_set *em);
void display_masks(fd_set *rm, fd_set *wm, fd_set *em);
void put_mask(char *name, fd_set *mask);
void periodic(int signum);
void houseKeeping(void);
void unblockSignals(void);
void blockSignals(void);
void resExit_(int exitCode);

/* daemons/resd/rf.c */
// void rfServ_(int acceptSock);
// int ropen(int sock, struct LSFHeader *hdr);
// int rclose(int sock, struct LSFHeader *hdr);
// int rwrite(int sock, struct LSFHeader *hdr);
// int rread(int sock, struct LSFHeader *hdr);
// int rlseek(int sock, struct LSFHeader *hdr);
// int clearSock(int sock, int len);
// int rstat(int sock, struct LSFHeader *hdr);
// int rfstat(int sock, struct LSFHeader *hdr);
// int rgetmnthost(int sock, struct LSFHeader *hdr);
// int runlink(int sock, struct LSFHeader *hdr);

/* daemons/resd/tasklog.c */
// void initResLog(void);
// void resAcctWrite(struct child *child);
// void resParentWriteAcct(struct LSFHeader *msgHdr, XDR *xdrs, int sock);

