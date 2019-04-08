
#pragma once

#define C_CONNECTED(cent)                                               \
    ((cent).rpid && ((cent).sock.fd != -1) && ((cent).rtime == 0))

#define DISCONNECT(id) {if (conn[id].rpid > 0)          \
            connIndexTable[conn[id].rpid-1] = -1;       \
        close(conn[id].sock.fd);                  \
        conn[id].sock.fd = -1;                          \
        conn[id].rpid = 0;                              \
        FD_CLR(id, &socks_bit);}
LIST_T *notifyList;


// #define NL_SETN         29
typedef struct dead_tid
{
  int tid;
  struct dead_tid *next;
} Dead_tid;


typedef struct dead_rpid
{
  int rpid;
  LS_WAIT_T status;
  struct rusage ru;
  struct dead_rpid *next;
} Dead_rpid;

typedef struct taskNotice
{
  struct taskNotice *forw, *back;
  int tid;
  int opCode;
} taskNotice_t;

typedef struct rtaskInfo
{
  struct rtaskInfo *forw, *back;
  int tid;
  time_t rtime;
  int eof;
  int dead;
} rtaskInfo_t;

struct connInfo
{
	Channel sock;
	pid_t rpid;
	time_t rtime;
	int bytesWritten;
	int eof;
	int dead;
	RelayLineBuf *rbuf;
	LIST_T *taskList;
	int rtag;
	int wtag;
	char *hostname;
	Dead_tid *deadtid;
};

struct {
	int empty;
	size_t length;
	fd_set socks;
	// char buf[BUFSIZ + LSF_HEADER_LEN];
	char *buf;
	size_t buf_lenght = BUFSIZ + sizeof( struct LSFHeader );
} writeBuf;

struct sigbuf
{
	int sigval;
	struct sigbuf *next;
};



int do_newconnect (int);
int get_connect (int, struct LSFHeader *);
int get_status (int, struct LSFHeader *, LS_WAIT_T *);
void setMask (fd_set *);
int bury_task (LS_WAIT_T, struct rusage *, int);
int do_setstdin (int, int);
int flush_buffer (void);
int check_timeout_task (void);
void add_list (struct nioInfo *, int, int, LS_WAIT_T *);
int deliver_signal (int);
int flush_sigbuf (void);
int flush_databuf (void);
int flush_eof (void);
int deliver_eof (void);
int sendUpdatetty ();
void checkHeartbeat (int nready);
int sendHeartbeat (void);
void checkJobStatus (int numTries);
JOB_STATUS getJobStatus (size_t jid, struct jobInfoEnt **job, struct jobInfoHead **jobHead);
int JobStateInfo (size_t jid);
bool_t compareTaskId (rtaskInfo_t *, int *, int);
rtaskInfo_t *getTask (LIST_T *, int);
rtaskInfo_t *addTask (LIST_T *);
void removeTask (LIST_T *, rtaskInfo_t *);
int addNotifyList (LIST_T *, int, int);
int addTaskList (int, int);
int notify_task (int, int);
int readResMsg (int);
int getFirstFreeIndex (void);
int doAcceptResCallback_( int s, struct niosConnect * );

int requeued = 0;
// #define MAXLOOP 10000
const unsigned int MAXLOOP = 10000;
// #define CHECK_PERIOD     4
const unsigned short = 4;
// #define MAX_READ_RETRY         5
const unsigned short MAX_READ_RETRY = 5;

Dead_rpid *dead_rpid = NULL;
fd_set socks_bit;
fd_set ncon_bit;

struct connInfo *conn     = NULL;
struct nioEvent *ioTable  = NULL;
struct nioEvent *ioTable1 = NULL;
struct sigbuf *sigBuf     = NULL;
struct nioInfo abortedTasks;


int lastConn = 0;

int maxfds          = 0;
int maxtasks        = 0;
int *connIndexTable = 0;
int count_unconn    = 0;
int acceptSock      = 0;
int sendEof         = 0;

int nioDebug = 0;
time_t lastCheckTime = 0;

// extern int msgInterval;

// extern char *getTimeStamp ();
// extern void kill_self (int, int);

// extern int JobStateInfo (size_t jid);