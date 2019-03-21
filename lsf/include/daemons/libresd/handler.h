// created by George Marselis <george@marsel.is> April 2019

#pragma once

/* lsf/daemons/resd/handler.c */
void doacceptconn(void);
void childAcceptConn(int s, struct passwd *pw, struct lsfAuth *auth, struct resConnect *connReq, struct hostent *hostp);
int setClUid(struct client *cli_ptr);
int recvConnect(int s, struct resConnect *connReq, size_t (*readFunc )(), struct lsfAuth *auth);
void doclient(struct client *cli_ptr);
int forwardTaskMsg(int srcSock, int destSock, struct LSFHeader *msgHdr, char *hdrbuf, char *dataBuf, bool_t noAck, int pid);
void dochild_info(struct child *chld, int op);
void resChdir(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs);
void resSetenv(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs);
void resStty(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs, int async);
void resRKill(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs);
void resGetpid(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs);
void resRusage(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs);
void resTaskMsg(struct client *cli_ptr, struct LSFHeader *msgHdr, char *hdrBuf, char *dataBuf, XDR *xdrs);
void doResParentCtrl(void);
void resControl(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs, int childSock);
enum resAck sendResParent(struct LSFHeader *msgHdr, char *msgBuf, bool_t (*xdrFunc )());
int checkPermResCtrl(struct client *cli_ptr);
void resRexec(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs);
int resFindPamJobStarter(void);
struct child *doRexec(struct client *cli_ptr, struct resCmdBill *cmdmsg, int retsock, int taskSock, int server, enum resAck *ack);
pid_t forkPty(struct client *cli_ptr, int *pty, int *sv, int *info, int *errSock, char *pty_name, enum resAck *ack, int retsock, int echoOff);
pid_t forkSV(struct client *cli_ptr, int *sv, int *info, int *errSock, enum resAck *ack);
void setPGid(struct client *cli_ptr, int tflag);
enum resAck parentPty(int *pty, int *sv, char *pty_name);
void set_noecho(int fd);
enum resAck childPty(struct client *cli_ptr, int *pty, int *sv, char *pty_name, int echoOff);
char *stripHomeUnix(const char *curdir, const char *home);
char *stripHomeNT(const char *curdir, const char *home);
void rexecChild(struct client *cli_ptr, struct resCmdBill *cmdmsg, int server, int taskSock, int *pty, int *sv, int *info, int *err, int retsock, pid_t *pid);
void lsbExecChild(struct resCmdBill *cmdmsg, int *pty, int *sv, int *err, int *info, pid_t *pid);
void execit(char **uargv, char *jobStarter, pid_t *pid, int stdio, int taskSock, int loseRoot);
void delete_client(struct client *cli_ptr);
void delete_child(struct child *cp);
void kill_child(struct child *cp);
int unlink_child(struct child *cp);
int notify_client(int s, int rpid, enum resAck ack, struct sigStatusUsage *sigStatRu);
void setptymode(ttyStruct *tts, int slave);
void dochild_stdio(struct child *chld, int op);
int resSignal(struct child *chld, struct resSignal sig);
void simu_eof(struct child *chld, int which);
void freeblk(char **blk);
char **copyArray(char **from);
void child_handler(void);
void child_handler_ext(void);
int notify_sigchild(struct child *cp);
void term_handler(int signum);
void sigHandler(int signum);
void declare_eof_condition(struct child *childPtr, int which);
int matchExitVal(int val, char *requeueEval);
void eof_to_nios(struct child *chld);
void eof_to_client(struct child *chld);
int pairsocket(int af, int type, int protocol, int *sv);
int sendReturnCode(int s, int code);
void child_channel_clear(struct child *chld, struct outputchannel *channel);
void setlimits(struct lsfLimit *lsfLimits);
void mysetlimits(struct lsfLimit *lsfLimits);
int setCliEnv(struct client *cl, char *envName, char *value);
int addCliEnv(struct client *cl, char *envName, char *value);
int lsbJobStart(char **jargv, u_short retPort, char *host, int usePty);
int ttyCallback(int s, ttyStruct *tty);
void resDebugReq(struct client *cli_ptr, struct LSFHeader *msgHdr, XDR *xdrs, int childSock);
void doReopen(void);
uid_t setEUid(uid_t uid);
void dumpResCmdBill(struct resCmdBill *bill);
int changeEUid(uid_t uid);
void dumpClient(struct client *client, char *why);
void dumpChild(struct child *child, int operation, char *why);
void dochild_buffer(struct child *chld, int op);
void donios_sock(struct child **children, int op);
int addResNotifyList(LIST_T *list, int rpid, int opCode, enum resAck ack, struct sigStatusUsage *sigStatRu);
int deliver_notifications(LIST_T *list);
int notify_nios(int retsock, int rpid, int opCode);
int resUpdatetty(struct LSFHeader msgHdr);
void cleanUpKeptPids(void);

// void          child_channel_clear   ( struct child *, struct outputchannel *);
// void          declare_eof_condition ( struct child *, int );
// void          simu_eof      ( struct  child *, int );
// void          resSetenv     ( struct client *, struct LSFHeader *, XDR * );
// void          doReopen      ( void );
// void          resDebugReq   ( struct client *, struct LSFHeader *, XDR *, int );
// void          resChdir      ( struct client *, struct LSFHeader *, XDR * );
// void          resStty       ( struct client *, struct LSFHeader *, XDR *, int );
// void          resRKill      ( struct client *, struct LSFHeader *, XDR * );
// void          resGetpid     ( struct client *, struct LSFHeader *, XDR * );
// void          resRusage     ( struct client *, struct LSFHeader *, XDR * );
// void          resControl    ( struct client *, struct LSFHeader *, XDR *, int );
// void          resRexec      ( struct client *, struct LSFHeader *, XDR * );
// void          resTaskMsg    ( struct client *, struct LSFHeader *, char *, char *, XDR * );
// int           forwardTaskMsg( int, int, struct LSFHeader *, char *, char *,  bool_t, int );
// struct child *doRexec       ( struct client *, struct resCmdBill *, int, int, int, enum resAck * );
// void          rexecChild    ( struct client *, struct resCmdBill *, int, int, int *, int *, int *, int *, int, int * );
// enum resAck   childPty      ( struct client *, int *, int *, char *, int );
// enum resAck   parentPty     ( int *pty, int *sv, char * );
// int           forkPty       ( struct client *, int *, int *, int *, int *, char *, enum resAck *, int, int );
// int           forkSV        ( struct client *, int *, int *, int *, enum resAck * );
// void          execit        ( char **uargv, char *jobStarter, pid_t *pid, int stdio, int taskSock, int loseRoot );
// void          lsbExecChild  ( struct resCmdBill *cmdmsg, int *pty, int *sv, int *err, int *info, pid_t *pid );

// void   delete_client ( struct client * );
// int    unlink_child  ( struct child *  );
// void   kill_child    ( struct child *  );
// int    notify_client ( int, int, enum resAck, struct sigStatusUsage * );
// void   eof_to_nios   ( struct child * );
// void   eof_to_client ( struct child * );
// void   setptymode    ( ttyStruct *, int );
// void   freeblk       ( char ** );
// char **copyArray     ( char ** );

// int  notify_sigchild (struct child *);
// int  pairsocket (int, int, int, int[2]);

// int  recvConnect (int s, struct resConnect *connReq, size_t (*readFunc) (), struct lsfAuth *auth);
// int  setClUid (struct client *cli_ptr);
// int  checkPermResCtrl (struct client *);

// int  ttyCallback (int, ttyStruct *);
// void setPGid (struct client *cli_ptr, int tflag);

// uid_t setEUid (uid_t uid);
// int changeEUid (uid_t uid);

// int notify_nios (int, int, int);
// int addResNotifyList (LIST_T *, int, int, enum resAck, struct sigStatusUsage *);

// int matchExitVal (int, char *);


// void setlimits   ( struct lsfLimit * );
// void mysetlimits ( struct lsfLimit * );
// int  addCliEnv   ( struct client *, char *, char * );
// int  setCliEnv   ( struct client *, char *, char * );
// int  resUpdatetty( struct LSFHeader );

// void dumpResCmdBill       ( struct resCmdBill * );
// void cleanUpKeptPids      ( void );
// int  resFindPamJobStarter ( void );



#define SET_RLIMIT(rlim, limit,loglevel)                                \
    if (setrlimit(rlim, &limit) < 0 && getuid() == 0) {                 \
        if (loglevel == LOG_INFO && errno == EINVAL) {                  \
            ls_syslog(loglevel,_i18n_msg_get(ls_catd , NL_SETN, 5299,   \
                                             "setrlimit(Resource Limit %d) failed: %m: soft %f hard %f , may be larger than the kernel allowed limit\n"), \
                      rlim, (double)limit.rlim_cur, (double)limit.rlim_max); \
        }else{                                                          \
            ls_syslog(LOG_ERR, _i18n_msg_get(ls_catd , NL_SETN, 5100,   \
                                             "setrlimit(Resouce Limit %d) failed: %m: soft %f hard %f infi=%f\n"), \
                      rlim, (double) limit.rlim_cur,                    \
                      (double) limit.rlim_max, (double) RLIM_INFINITY); \
        }                                                               \
    }               /* catgets 5100 */


// #define PAIRSOCKET_TIMEOUT      30
const unsigned int PAIRSOCKET_TIMEOUT = 30;

typedef enum
{
    PTY_BAD,
    PTY_GOOD,
    PTY_NEW,
    PTY_UGLY
    PTY_NOMORE
} status_t;

struct hand_shake
{
    status_t code;
    char buffer[MAX_PATH_LEN];
};


int currentRESSN = 0;

extern char **environ;
extern ttyStruct defaultTty;
extern int initLimSock_ (void);

short is_resumed = FALSE;

// #define NL_SETN         29
// #define CHILD_DELETED     2
const unsigned short CHILD_DELETED = 2;

