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

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <stdio.h>
#include <stdlib.h>

#include "daemons/libresd/resout.h"
#include "libint/intlibout.h"
#include "libint/list.h"
#include "lib/osal.h"
#include "lib/xdr.h"
#include "lsf.h"

#ifndef _BSD
#define _BSD
#endif

typedef gid_t GETGROUPS_T;

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

int rexecPriority;
extern struct client *clients[]; // FIXME FIXME FIXME FIXME FIXME attach specific header!
int client_cnt;
struct child **children;
int child_cnt;
char *Myhost;
char *myHostType;

int lastChildExitStatus;

int sbdMode;
int sbdFlags;
#define SBD_FLAG_STDIN  0x1
#define SBD_FLAG_STDOUT 0x2
#define SBD_FLAG_STDERR 0x4
#define SBD_FLAG_TERM   0x8

int accept_sock;
char child_res;
char child_go;
char res_interrupted;
char *gobuf;
char allow_accept;
extern char magic_str[]; // FIXME FIXME FIXME FIXME FIXME attach specific header!
int child_res_port;
int parent_res_port;
fd_set readmask, writemask, exceptmask;

int ctrlSock;
struct sockaddr_in ctrlAddr;

int on;
int off;
int debug;
int res_status;

char *lsbJobStarter;

extern char res_logfile[]; // FIXME FIXME FIXME FIXME FIXME attach specific header!
int res_logop;
int restart_argc;
char **restart_argv;
char *env_dir;

#define MAXCLIENTS_HIGHWATER_MARK 100
#define MAXCLIENTS_LOWWATER_MARK  1


#define DOREAD  0
#define DOWRITE 1
#define DOEXCEPTION 2
#define DOSTDERR 3

#define PTY_TEMPLATE    "/dev/ptyXX"

#define PTY_SLAVE_INDEX   (sizeof(PTY_TEMPLATE) - 6)
#define PTY_ALPHA_INDEX   (sizeof(PTY_TEMPLATE) - 3)
#define PTY_DIGIT_INDEX   (sizeof(PTY_TEMPLATE) - 2)

#define PTY_FIRST_ALPHA   'p'
# define PTY_LAST_ALPHA   'v'

#define   BUFSTART(x)    ((char *) ((x)->buf) + sizeof(struct LSFHeader))

#define CLOSE_IT(fd)     if (fd>=0) {close(fd); fd = INVALID_FD;}

// moved to $(include_dir)/lib.h
// enum {
// 	LSF_RES_DEBUG,
// 	LSF_SERVERDIR,
// 	LSF_AUTH,
// 	LSF_LOGDIR,
// 	LSF_ROOT_REX,
// 	LSF_LIM_PORT,
// 	LSF_RES_PORT,
// 	LSF_ID_PORT,
// 	LSF_USE_HOSTEQUIV,
// 	LSF_RES_ACCTDIR,
// 	LSF_RES_ACCT,
// 	LSF_DEBUG_RES, // FIXME FIXME FIXME LSF_RES_DEBUG and LSF_DEBUG_RES , replace with single tag
// 	LSF_TIME_RES,
// 	LSF_LOG_MASK,
// 	LSF_RES_RLIMIT_UNLIM,
// 	LSF_CMD_SHELL,
// 	LSF_ENABLE_PTY,
// 	LSF_TMPDIR,
// 	LSF_BINDIR,
// 	LSF_LIBDIR,
// 	LSF_RES_TIMEOUT,
// 	LSF_RES_NO_LINEBUF,
// 	LSF_MLS_LOG,
// } status;

struct config_param resParams[] = {
    { "LSF_RES_DEBUG",          NULL },
    { "LSF_SERVERDIR",          NULL },
    { "LSF_AUTH",               NULL },
    { "LSF_LOGDIR",             NULL },
    { "LSF_ROOT_REX",           NULL },
    { "LSF_LIM_PORT",           NULL },
    { "LSF_RES_PORT",           NULL },
    { "LSF_ID_PORT",            NULL },
    { "LSF_USE_HOSTEQUIV",      NULL },
    { "LSF_RES_ACCTDIR",        NULL },
    { "LSF_RES_ACCT",           NULL },
    { "LSF_DEBUG_RES",          NULL }, // FIXME FIXME FIXME LSF_RES_DEBUG and LSF_DEBUG_RES , replace with single tag
    { "LSF_TIME_RES",           NULL },
    { "LSF_LOG_MASK",           NULL },
    { "LSF_RES_RLIMIT_UNLIM",   NULL },
    { "LSF_CMD_SHELL",          NULL },
    { "LSF_ENABLE_PTY",         NULL },
    { "LSF_TMPDIR",             NULL },
    { "LSF_BINDIR",             NULL },
    { "LSF_LIBDIR",             NULL },
    { "LSF_RES_TIMEOUT",        NULL },
    { "LSF_RES_NO_LINEBUF",     NULL },
    { "LSF_MLS_LOG",            NULL },
    //////////////////////// end inclusion from resd.h
    { "LSF_CONFDIR",            NULL },
    // { "LSF_SERVERDIR" ,      NULL },
    { "LSF_LIM_DEBUG",          NULL },
    // { "LSF_RES_DEBUG",       NULL },
    { "LSF_STRIP_DOMAIN",       NULL },
    // { "LSF_LIM_PORT",        NULL },
    // { "LSF_RES_PORT",        NULL },
    // { "LSF_LOG_MASK",        NULL },
    { "LSF_SERVER_HOSTS",       NULL },
    // { "LSF_AUTH",            NULL },
    // { "LSF_USE_HOSTEQUIV",   NULL },
    // { "LSF_ID_PORT",         NULL },
    // { "LSF_RES_TIMEOUT",     NULL },
    { "LSF_API_CONNTIMEOUT",    NULL },
    { "LSF_API_RECVTIMEOUT",    NULL },
    { "LSF_AM_OPTIONS",         NULL },
    // { "LSF_TMPDIR",          NULL },
    // { "LSF_LOGDIR",          NULL },
    { "LSF_SYMBOLIC_LINK",      NULL },
    { "LSF_MASTER_LIST",        NULL },
    // { "LSF_MLS_LOG",         NULL },
    { "LSF_INTERACTIVE_STDERR", NULL },
    { "NO_HOSTS_FILE",          NULL },
    { "LSB_SHAREDIR",           NULL },
    {NULL,                      NULL}  
};



struct relaylinebuf
{
#ifndef LINE_BUFSIZ       // this is not supposed to be here, but once removed
#define LINE_BUFSIZ 4096  // compiler throws error
#endif
  char buf[LINE_BUFSIZ + sizeof (struct LSFHeader) + 1];
  char *bp;
  int bcount;
};

// typedef struct relaylinebuf RelayLineBuf;

struct relaybuf
{
	char buf[BUFSIZ + sizeof (struct LSFHeader)];
	char *bp;
	int bcount;
};

// typedef struct relaybuf RelayBuf;

struct channel
{
	int fd;
	struct relaybuf *rbuf;
	int rcount;
	struct relaybuf *wbuf;
	int wcount;
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
	struct relaybuf *rbuf;
	int rcount;
	struct relaylinebuf *wbuf;
	int wcount;
	int opCode;
};

// struct nioschannel niosChannel;  // FIXME FIXME FIXME remove typedef from struct


typedef struct ttystruct
{
  struct termios attr;
  struct winsize ws;
#    if defined(hpux) || defined(__hpux)
  struct ltchars hp_ltchars;
#    endif
} ttyStruct;    // FIXME FIXME FIXME remove typedef from struct


struct client
{
  int client_sock;
  pid_t ruid;
  pid_t gid;
  char *username;
  char *clntdir;
  char *homedir;
  ttyStruct tty;
  char **env;
  int ngroups;
  GETGROUPS_T groups[NGROUPS_MAX];
  struct hostent hostent;
  struct lenData eexec;
};

struct child
{
	struct client *backClnPtr;
	pid_t rpid;
	pid_t pid;

	int refcnt;
	int info;
	int stdio;
	struct outputchannel *std_out;
	struct outputchannel *std_err;

	struct niosChannel *remsock;
	int rexflag;
	char server;
	char c_eof;
	char running;
	char sigchild;
	LS_WAIT_T wait;
	struct sigStatusUsage *sigStatRu;
	int endstdin;
	struct relaybuf *i_buf;
	int stdin_up;

	char slavepty[sizeof (PTY_TEMPLATE)];

	char **cmdln;
	time_t dpTime;
	char *cwd;
	char username[MAXLSFNAMELEN];
	char fromhost[MAXHOSTNAMELEN];
	int sent_eof;
	int sent_status;
};

struct resChildInfo
{
  struct resConnect *resConnect;
  struct lsfAuth *lsfAuth;
  struct passwd *pw;
  struct hostent *host;
  u_short parentPort;
  int currentRESSN;
  int res_status;
};



// typedef 
struct taggedConn
{
	struct niosChannel *sock;
	int rtag;
	int wtag;
	int *task_duped;
	int num_duped;
}; //taggedConn_t;

typedef struct resNotice
{
	struct resNotice *forw, *back;
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



/*********************************************/
/* these structures where moved over here from
 * resout.h because of scoping issues.
 * investigate what's up
 */

struct resSignal
{
	pid_t pid;
	int sigval;
};

struct resCmdBill
{
	u_short retport;
	char padding1[2];
	pid_t rpid;
	int filemask;
	int priority;
	int options;
	// char cwd[MAXPATHLEN];
	char *cwd;
	char padding2[4];
	char **argv;
	struct lsfLimit lsfLimits[LSF_RLIM_NLIMITS];  // this should be changed to a pointer
};

struct resSetenv
{
	char **env;
};

#define RES_RID_ISTID          0x01
#define RES_RID_ISPID          0x02

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

#define RES_RPID_KEEPPID 0x01

struct resRusage
{
  pid_t rid;
  int whatid;
  int options;
};

struct resChdir
{
  char dir[MAXFILENAMELEN];
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

// typedef struct nioschannel
// {
//   int fd;
//   struct RelayBuf *rbuf;
//   int rcount;
//   struct RelayLineBuf *wbuf;
//   int wcount;
//   int opCode;
// } niosChannel;

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

// extern
struct taggedConn conn2NIOS;
// extern
LIST_T *resNotifyList;

// extern
int currentRESSN;


#define LSB_UTMP           0

#define SIG_NT_CTRLC        2000
#define SIG_NT_CTRLBREAK    2001

struct config_param resParams[];
extern struct config_param resConfParams[]; // FIXME FIXME FIXME FIXME FIXME attach specific header!

#define RES_REPLYBUF_LEN   4096

#define RESS_LOGBIT         0x00000001

void init_res (void);
void resExit_ (int exitCode);
long  nb_write_fix (int s, char *buf, size_t len);
int ptymaster (char *);
int ptyslave (char *);
void doacceptconn (void);
void dochild_stdio (struct child *, int);
void dochild_remsock (struct child *, int);
void dochild_buffer (struct child *, int);
void dochild_info (struct child *, int);
void doclient (struct client *);
void ptyreset (void);
void stdout_flush (struct child *chld);
void doResParentCtrl (void);
enum resAck sendResParent (struct LSFHeader * msgHdr, char *msgBuf, bool_t (*xdrFunc) ());
int sendReturnCode (int, int);

void donios_sock (struct child **, int);
int deliver_notifications (LIST_T *);

void term_handler (int);
void sigHandler (int);
void child_handler (void);
void child_handler_ext (void);
void getMaskReady (fd_set * rm, fd_set * wm, fd_set * em);
void display_masks (fd_set *, fd_set *, fd_set *);

size_t b_read_fix   (int s, char *buf, size_t len);
long b_write_fix  (int s, char *buf, size_t len);

int lsbJobStart (char **, u_short, char *, int);

void childAcceptConn (int, struct passwd *, struct lsfAuth *, struct resConnect *, struct hostent *);

void resChild (char *, char *);
int resParent (int, struct passwd *, struct lsfAuth *, struct resConnect *, struct hostent *);
bool_t isLSFAdmin (const char *);

bool_t xdr_resChildInfo (XDR *, struct resChildInfo *, struct LSFHeader *);

void rfServ_ (int);

char *pty_translate (char *);
int check_valid_tty (char *);

void resAcctWrite (struct child *);
void initResLog (void);
char resAcctFN[MAXFILENAMELEN];
int resLogOn;
int resLogcpuTime;
void initRU (struct rusage *);
void resParentWriteAcct (struct LSFHeader *, XDR *, int);

int findRmiDest (int *, int *);

void delete_child (struct child *);
void destroy_child (struct child *);
int resSignal (struct child *chld, struct resSignal sig);

void dumpClient (struct client *, char *);
void dumpChild (struct child *, int, char *);



#define UTMP_CHECK_CODE "sbdRes"