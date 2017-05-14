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

#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/queue.h"
#include "lib/xdrnio.h"
#include "lib/xdrrf.h"
#include "lib/tid.h"
#include "lib/res.h" // FIXME FIXME FIXME delete, move functions to appropriate header file

/* #define SIGEMT SIGBUS */

extern char **environ;
extern int gethostbysock_ (int, char *);

int lsf_res_version               = -1;
int totsockets_                   =  0;
int currentsocket_                =  0;
unsigned int requestSN            =  0;
// static unsigned int requestHighWaterMark =  0;
fd_set connection_ok_;
struct sockaddr_in res_addr_;
struct lsQueue *requestQ;
static int getLimits (struct lsfLimit *);
static int mygetLimits (struct lsfLimit *);

#define REQUESTSN ((requestSN < USHRT_MAX) ? requestSN++ : (requestSN=11 , 10))

int callRes_ (int s, enum resCmd cmd, char *data, char *reqBuf, size_t reqLen, bool_t (*xdrFunc) (), int *rd, struct timeval *timeout, struct lsfAuth *auth);
int   do_rstty1_ (char *host, int async);
int   do_rstty2_ (int s, int io_fd, int redirect, int async);
FILE *ls_popen   (int s, char *command, char *type);
int   ls_pclose  (FILE * stream);
int   rsetenv_   (char *host, char **envp, int option);
int   enqueueTaskMsg_ (int s, pid_t taskID, struct LSFHeader *msgHdr);
int   sendSig_   (char *host, pid_t rid, int sig, int options);
int   rgetRusageCompletionHandler_ (struct lsRequest *request);
int   ls_rstty (char *host);


unsigned int  getCurrentSN( void ) {
    return globCurrentSN;
}

unsigned int  setCurrentSN( unsigned int currentSN ) {
    unsigned int rvalue = 0; // on success return 0;

    assert( currentSN <= INT_MAX ); // FIXME FIXME FIXME once first debuging is done, remove. We are trying to catch any function sneaking in garbage.
    globCurrentSN = currentSN;

    return rvalue;
}


int
ls_connect (char *host)
{
    int s = 0;
    int descriptor[2] = { 0, 0 };
    int resTimeout;
    size_t size = 0;
    char *reqBuf = NULL;
    struct lsfAuth auth;
    struct resConnect connReq;
    struct hostent *hp = NULL;
    char official[MAXHOSTNAMELEN] = "";
 
    if (genParams_[LSF_RES_TIMEOUT].paramValue) {
        resTimeout = atoi (genParams_[LSF_RES_TIMEOUT].paramValue);
    }
    else {
        resTimeout = RES_TIMEOUT;
    }

    if (_isconnected_ (host, descriptor)) {
        return descriptor[0];
    }

    if ((hp = Gethostbyname_ (host)) == NULL) {
        lserrno = LSE_BAD_HOST;
        return -1;
    }

    strcpy (official, hp->h_name);
    memcpy ((char *) &res_addr_.sin_addr, (char *) hp->h_addr_list[0], hp->h_length);
    if ((rootuid_) && (genParams_[LSF_AUTH].paramValue == NULL))
    {
        if (currentsocket_ > (int)(FIRST_RES_SOCK + (unsigned int)totsockets_ - 1))
        {
            lserrno = LSE_NOMORE_SOCK;
            return -1;
        }
        
        s = currentsocket_;
        currentsocket_++;
    } 
    else {

        if ((s = CreateSockEauth_ (SOCK_STREAM)) < 0) {
            return -1;
        }
    }

  putEauthClientEnvVar ("user");
  putEauthServerEnvVar ("res");

#ifdef INTER_DAEMON_AUTH
    putEnv ("LSF_EAUTH_AUX_PASS", "yes");
#endif


    if (getAuth_ (&auth, official) == -1){
        closesocket (s);
        return -1;
    }

    runEsub_ (&connReq.eexec, NULL);

    size =  sizeof (struct LSFHeader) + sizeof (connReq) + 
            sizeof (struct lsfAuth) + (size_t)ALIGNWORD_ (connReq.eexec.len);

    reqBuf = malloc(5 * size * sizeof (reqBuf) );  // FIXME FIXME why 5?
    if (NULL == reqBuf && ENOMEM == errno ) {
        lserrno = LSE_MALLOC;
          CLOSESOCKET (s);

        if (connReq.eexec.len > 0) {
            free (connReq.eexec.data);
        }
        free (reqBuf);
        return -1;
    }

    assert( resTimeout >= 0 );
    if (b_connect_ (s, (struct sockaddr *) &res_addr_, sizeof (res_addr_), (unsigned int)resTimeout) < 0) {
        lserrno = LSE_CONN_SYS;
          
        if (connReq.eexec.len > 0) {
            free (connReq.eexec.data);
        }
        free (reqBuf);
        return -1;
    }

    if (callRes_ (s, RES_CONNECT, (char *) &connReq, reqBuf, size, xdr_resConnect, 0, 0, &auth) == -1) {
        
        if (connReq.eexec.len > 0) {
            free (connReq.eexec.data);
        }
        free (reqBuf);
        return -1;
    }

    if (connReq.eexec.len > 0) {
        free (connReq.eexec.data);
    }

    free (reqBuf);

    connected_ (official, s, -1, getCurrentSN( ) );
    return s;
}

int
lsConnWait_ (char *host)
{
    int s = 0;
    int descriptor[2] = { 0, 0 };

    if (_isconnected_ (host, descriptor)) {
        assert( 0 != descriptor[0] );
        s = descriptor[0];
    }
    else {
        return -1;
    }

    if (!FD_ISSET (s, &connection_ok_)) {

        FD_SET (s, &connection_ok_);
        if (ackReturnCode_ (s) < 0)
        {
            closesocket (s);
            _lostconnection_ (host);
            return -1;
        }
    }

  return 0;
}

int
sendCmdBill_ (int s, enum resCmd cmd, struct resCmdBill *cmdmsg, int *retsock, struct timeval *timeout)
{
    char *buf;
    int cc = 0;
    size_t bufsize = 1024;

    bufsize += ( ALIGNWORD_ (strlen (cmdmsg->cwd)) + sizeof (int) );
    for ( int i = 0; cmdmsg->argv[i]; i++) {
        bufsize += ( ALIGNWORD_ (strlen (cmdmsg->argv[i])) + sizeof (int) );
    }

    buf = (char *) malloc (bufsize);
    if ( NULL == buf && ENOMEM == errno ) {
      lserrno = LSE_MALLOC;
      return -1;
    }

    umask (cmdmsg->filemask = (mode_t) umask (0));
    cmdmsg->priority = 0;
    if (getLimits (cmdmsg->lsfLimits) < 0) {
        lserrno = LSE_LIMIT_SYS;
        free (buf);
        return -1;
    }

    cc = callRes_ (s, cmd, (char *) cmdmsg, buf, bufsize, xdr_resCmdBill, retsock, timeout, NULL);
    free (buf);
    return cc;
}

FILE *
ls_popen (int s, char *command, char *type)
{

    assert( s );
    assert( *command );
    assert( *type );
    return (FILE *) 0;    // FIXME FIXME FIXME what the heck?! 
                            //      put it under the debugger and eliminate
                            //      code that reaches here
}

int
ls_pclose (FILE * stream)
{
    stream = NULL;

    assert( stream == NULL );
    return 0;     // FIXME FIXME FIXME what the heck?! 
                    //      put it under the debugger and eliminate
                    //      code that reaches here
}

#define RSETENV_SYNCH   1
#define RSETENV_ASYNC   2
int
rsetenv_ (char *host, char **envp, int option )
{
    int i = 0;
    int s = 0;
    int descriptor[2];
    char *sendBuf;
    enum resCmd resCmdOption = RES_FAIL;
    struct resSetenv envMsg;
    
    size_t bufferSize = 512;

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "rsetenv_: Entering this routine...");
    }

    if (!envp) {
        return 0;
    }

    for (i = 0; envp[i] != NULL; i++) {
        bufferSize += ALIGNWORD_ (strlen (envp[i])) + sizeof (int);
    }


    sendBuf = (char *) malloc( bufferSize );
    if ( NULL == sendBuf && ENOMEM == errno ) {
        lserrno = LSE_MALLOC;
        return -1;
    }

    if (_isconnected_ (host, descriptor)) {
        s = descriptor[0];
    }
    else if ((s = ls_connect (host)) < 0) {
        free(sendBuf); 
        return -1;
    }
    else {
        printf( "oops, i made a first booboo: rsetenv_()\n");
    }

    if (!FD_ISSET (s, &connection_ok_)) {

        FD_SET (s, &connection_ok_);
        if (ackReturnCode_ (s) < 0) {
            closesocket (s);
            _lostconnection_ (host);
            free (sendBuf);
            return -1;
        }
    }

    envMsg.env = envp;
    if (option == RSETENV_SYNCH) {
        resCmdOption = RES_SETENV;
    }
    else if (option == RSETENV_ASYNC) {
        resCmdOption = RES_SETENV_ASYNC;
    }
    else {
        printf( "oops, i made a second booboo: rsetenv_()\n");
    }
    
    if (callRes_ (s, resCmdOption, (char *) &envMsg, sendBuf, bufferSize,
    xdr_resSetenv, 0, 0, NULL) == -1)
    {
      closesocket (s);
      _lostconnection_ (host);
      free (sendBuf);
      return -1;
    }

    free (sendBuf);

    if (option == RSETENV_SYNCH) {
        if (ackReturnCode_ (s) < 0) {
            closesocket (s);
            _lostconnection_ (host);
            return -1;
        }
    }

    return 0;
}

int
ls_rsetenv_async (char *host, char **envp)
{
    return rsetenv_ (host, envp, RSETENV_ASYNC);
}

int
ls_rsetenv (char *host, char **envp)
{
    return rsetenv_ (host, envp, RSETENV_SYNCH);
}

int
ls_chdir (char *host, char *dir)
{
    int s = 0;
    int descriptor[2] = { 0, 0 };
    struct {
        struct LSFHeader hdr;
        struct resChdir ch;
    } buf;
    struct resChdir chReq;

    if (_isconnected_ (host, descriptor)) {
        s = descriptor[0];
    }
    else if ((s = ls_connect (host)) < 0) {
        return -1;
    }
    else {
        printf( "I done goofed up! ls_chdir()\n");
    }

    if (!FD_ISSET (s, &connection_ok_)) {

        FD_SET (s, &connection_ok_);
        if (ackReturnCode_ (s) < 0) {
            closesocket (s);
            _lostconnection_ (host);
            return -1;
        }
    }

  if (dir[0] != '/' && dir[1] != ':' && (dir[0] != '\\' || dir[1] != '\\'))
    {
      lserrno = LSE_BAD_ARGS;
      return -1;
    }

  strcpy (chReq.dir, dir);

  if (callRes_ (s, RES_CHDIR, (char *) &chReq, (char *) &buf,
    sizeof (buf), xdr_resChdir, 0, 0, NULL) == -1)
    {
      closesocket (s);
      _lostconnection_ (host);
      return -1;
    }

  if (ackReturnCode_ (s) < 0)
    {
      if (lserrno == LSE_RES_DIRW)
  return -2;
      else
  return -1;
    }

  return 0;
}

struct lsRequest *
lsReqHandCreate_ ( pid_t tid, int seqno, int connfd, void *extra, requestCompletionHandler replyHandler,  appCompletionHandler appHandler, void *appExtra)
{
    struct lsRequest *request;
    
    request = (struct lsRequest *) malloc (sizeof (struct lsRequest));
    if (!request) {
        lserrno = LSE_MALLOC;
        return NULL;
    }

    request->tid = tid;
    assert( seqno >= 0 );
    request->seqno =  seqno;
    request->connfd = connfd;
    request->completed = FALSE;
    request->extra = extra;
    request->replyHandler = replyHandler;
    request->appHandler = appHandler;
    request->appExtra = appExtra;

    return request;
}

int
ackAsyncReturnCode_ (int s, struct LSFHeader *replyHdr)
{
    char cseqno;
    int seqno;


    int rc   = 0;
    ssize_t len = 0;
    struct lsQueueEntry *reqEntry;
    struct lsRequest *reqHandle;

    seqno = replyHdr->refCode;
    assert( seqno <= CHAR_MAX && seqno >= 0);
    cseqno = seqno;     // FIXME FIXME FIXME FIXME FIXME DA FAQ 
                        // GARBAGE GARBAGE GARBAGE
    reqEntry = lsQueueSearch_ (0, &cseqno, requestQ);
    if (reqEntry == NULL)
    {
        lserrno = LSE_PROTOC_RES;
        return -1;
    }

    lsQueueEntryRemove_ (reqEntry);
    reqHandle = (struct lsRequest *) reqEntry->data;    //  FIXME FIXME FIXME FIXME FIXME
                                                        //      member-by-member assignment!

    reqEntry->data = NULL;

    reqHandle->rc = replyHdr->opCode;
    reqHandle->completed = TRUE;
    if (replyHdr->length > 0) {

        reqHandle->replyBuf = malloc (replyHdr->length);
        if ( NULL == reqHandle->replyBuf && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return -1;
        }

        len = b_read_fix (s, reqHandle->replyBuf, replyHdr->length);
        assert( replyHdr->length <= LONG_MAX );
        if (len != (ssize_t) replyHdr->length) {
            free (reqHandle->replyBuf);
            lserrno = LSE_MSG_SYS;
            return -1;
        }

        assert( len >= 0);
        reqHandle->replyBufLen = (unsigned long)len;
    }

    if (reqHandle->replyHandler) {
        rc = (*(reqHandle->replyHandler)) (reqHandle);
        if (replyHdr->length > 0) {
            free (reqHandle->replyBuf);
        }
        
        if (rc < 0) {
            if (reqHandle->appHandler) {
                rc = (*(reqHandle->appHandler)) (reqHandle, reqHandle->appExtra);
            }

            lsQueueEntryDestroy_ (reqEntry, requestQ);
            return rc;
        }
    }

    if (reqHandle->appHandler) {
        rc = (*(reqHandle->appHandler)) (reqHandle, reqHandle->appExtra);
    }

    lsQueueEntryDestroy_ (reqEntry, requestQ);
    return rc;
}

int
enqueueTaskMsg_ (int s, pid_t taskID, struct LSFHeader *msgHdr)
{
    char *msgBuf = NULL;
    struct tid *tEnt = NULL;
    struct lsTMsgHdr *header = NULL;

    tEnt = tid_find (taskID);
    if (tEnt == NULL) {
        return -1;
    }

    header = (struct lsTMsgHdr *) malloc (sizeof (struct lsTMsgHdr));
    if (!header) {
        lserrno = LSE_MALLOC;
        return -1;
    }

    header->len = 0;
    header->msgPtr = NULL;
    if (s < 0) {
        header->type = LSTMSG_IOERR;
        lsQueueDataAppend_ ((char *) header, tEnt->tMsgQ);
        return 0;
    }

    /* or 0xffff ?
     */
    if (msgHdr->reserved == 0 && msgHdr->length == 0) {
        header->type = LSTMSG_EOF;
        lsQueueDataAppend_ ((char *) header, tEnt->tMsgQ);
        return 0;
    }

    if (msgHdr->length == 0) {
        msgBuf = malloc (1);
    }
    else {
        msgBuf = malloc (msgHdr->length);
    }

    if( NULL == msgBuf && ENOMEM == errno ) {
        lserrno = LSE_MALLOC;
        return -1;
    }

    assert( msgHdr->length <= LONG_MAX );
    if (b_read_fix (s, (char *) msgBuf, msgHdr->length) != (ssize_t) msgHdr->length) {
        free (msgBuf);
        lserrno = LSE_MSG_SYS;
        return -1;
    }

    header->type = LSTMSG_DATA;
    header->len = msgHdr->length;
    header->msgPtr = msgBuf;

    lsQueueDataAppend_ ((char *) header, tEnt->tMsgQ);

    return 0;
}

int
expectReturnCode_ (int s, pid_t seqno, struct LSFHeader *repHdr)
{
  struct LSFHeader buf;
  XDR xdrs;
  int rc = 0;

  xdrmem_create (&xdrs, (char *) &buf, sizeof (struct LSFHeader), XDR_DECODE);
  for (;;)
    {
      if (logclass & LC_TRACE)
  ls_syslog (LOG_DEBUG, "%s: calling readDecodeHdr_...", __func__);
      xdr_setpos (&xdrs, 0);
      if (readDecodeHdr_ (s, (char *) &buf, &b_read_fix, &xdrs, repHdr) < 0)
  {
    xdr_destroy (&xdrs);
    return -1;
  }

      if (repHdr->opCode == RES_NONRES)
  {
    rc = enqueueTaskMsg_ (s, repHdr->refCode, repHdr);
    if (rc < 0)
      {
        xdr_destroy (&xdrs);
        return rc;
      }
  }
      else
  {
    if (repHdr->refCode == seqno)
      break;

    rc = ackAsyncReturnCode_ (s, repHdr);
    if (rc < 0)
      {
        xdr_destroy (&xdrs);
        return rc;
      }
  }
    }
  xdr_destroy (&xdrs);
  return 0;

}

int
resRC2LSErr_ (int resRC)
{
    switch (resRC) {
        case RESE_OK:
            lserrno = 0;
            break;
        case RESE_NOMORECONN:
            lserrno = LSE_RES_NOMORECONN;
            break;
        case RESE_BADUSER:
            lserrno = LSE_BADUSER;
            break;
        case RESE_ROOTSECURE:
            lserrno = LSE_RES_ROOTSECURE;
            break;
        case RESE_DENIED:
            lserrno = LSE_RES_DENIED;
            break;
        case RESE_REQUEST:
            lserrno = LSE_PROTOC_RES;
            break;
        case RESE_CALLBACK:
            lserrno = LSE_RES_CALLBACK;
            break;
        case RESE_NOMEM:
            lserrno = LSE_RES_NOMEM;
            break;
        case RESE_FATAL:
            lserrno = LSE_RES_FATAL;
            break;
        case RESE_CWD:
            lserrno = LSE_RES_DIR;
        break;
        
        case RESE_PTYMASTER:
        case RESE_PTYSLAVE:
            lserrno = LSE_RES_PTY;
            break;
        case RESE_SOCKETPAIR:
            lserrno = LSE_RES_SOCK;
            break;
        case RESE_FORK:
            lserrno = LSE_RES_FORK;
        break;
            case RESE_INVCHILD:
            lserrno = LSE_RES_INVCHILD;
            break;
        case RESE_KILLFAIL:
            lserrno = LSE_RES_KILL;
            break;
        case RESE_VERSION:
            lserrno = LSE_RES_VERSION;
            break;
        case RESE_DIRW:
            lserrno = LSE_RES_DIRW;
            break;
        case RESE_NOLSF_HOST:
            lserrno = LSE_NLSF_HOST;
            break;
        case RESE_RUSAGEFAIL:
            lserrno = LSE_RES_RUSAGE;
            break;
        case RESE_RES_PARENT:
            lserrno = LSE_RES_PARENT;
            break;
        case RESE_MLS_INVALID:
            lserrno = LSE_MLS_INVALID;
            break;
        case RESE_MLS_CLEARANCE:
            lserrno = LSE_MLS_CLEARANCE;
            break;
        case RESE_MLS_DOMINATE:
            lserrno = LSE_MLS_DOMINATE;
            break;
        case RESE_MLS_RHOST:
            lserrno = LSE_MLS_RHOST;
            break;
        default:
            lserrno = NOCODE + resRC;
    }

 return lserrno;
}

int
ackReturnCode_ (int s)
{   
    int rc = 0;
    int getcurseqnoReturnValue = 0;
    struct LSFHeader repHdr;
    char hostname[MAXHOSTNAMELEN];

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
    }

    gethostbysock_ (s, hostname);
    getcurseqnoReturnValue = _getcurseqno_ (hostname);
    setCurrentSN(  getcurseqnoReturnValue );
    rc = expectReturnCode_ (s, getCurrentSN( ), &repHdr);
    if (rc < 0) {
        return rc;
    }

    lsf_res_version = (int) repHdr.version;

    rc = resRC2LSErr_ (repHdr.opCode);

    if (rc == 0) {
        return 0;
    }
    else {
        return -1;
    }

}

static int
getLimits (struct lsfLimit *limits)
{
    return mygetLimits (limits);
}

static int
mygetLimits (struct lsfLimit *limits)
{
  int i;
  struct rlimit rlimit;

  for (i = 0; i < LSF_RLIM_NLIMITS; i++)
    {
      rlimit.rlim_cur = RLIM_INFINITY;
      rlimit.rlim_max = RLIM_INFINITY;
      rlimitEncode_ (&limits[i], &rlimit, i);
    }

#ifdef  RLIMIT_CPU
  if (getrlimit (RLIMIT_CPU, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_CPU], &rlimit, LSF_RLIMIT_CPU);
#endif

#ifdef  RLIMIT_FSIZE
  if (getrlimit (RLIMIT_FSIZE, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_FSIZE], &rlimit, LSF_RLIMIT_FSIZE);
#endif

#ifdef RLIMIT_DATA
  if (getrlimit (RLIMIT_DATA, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_DATA], &rlimit, LSF_RLIMIT_DATA);
#endif

#ifdef RLIMIT_STACK
  if (getrlimit (RLIMIT_STACK, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_STACK], &rlimit, LSF_RLIMIT_STACK);
#endif

#ifdef RLIMIT_CORE
  if (getrlimit (RLIMIT_CORE, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_CORE], &rlimit, LSF_RLIMIT_CORE);
#endif

#ifdef RLIMIT_NOFILE
  if (getrlimit (RLIMIT_NOFILE, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_NOFILE], &rlimit, LSF_RLIMIT_NOFILE);
#endif

#ifdef RLIMIT_OPEN_MAX
  if (getrlimit (RLIMIT_OPEN_MAX, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_OPEN_MAX], &rlimit, LSF_RLIMIT_OPEN_MAX);
#endif

#ifdef RLIMIT_VMEM
  if (getrlimit (RLIMIT_VMEM, &rlimit) < 0) {
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_VMEM], &rlimit, LSF_RLIMIT_VMEM);
#endif

#ifdef RLIMIT_RSS
  if (getrlimit (RLIMIT_RSS, &rlimit) < 0){
    return -1;
  }
  rlimitEncode_ (&limits[LSF_RLIMIT_RSS], &rlimit, LSF_RLIMIT_RSS);
#endif

  return 0;
}

int
callRes_ (int s, enum resCmd cmd, char *data, char *reqBuf, size_t reqLen, bool_t (*xdrFunc) (), int *rd, 
    struct timeval *timeout, struct lsfAuth *auth)
{
    ssize_t cc = 0;
    int t  = 0;
    int nready = 0;
    char hostname[MAXHOSTNAMELEN];
    socklen_t fromsize;
    sigset_t oldMask;
    sigset_t newMask;
    struct sockaddr_in from;
    
    blockALL_SIGS_ (&newMask, &oldMask);

    memset (hostname, 0, sizeof (hostname));

    setCurrentSN( REQUESTSN );

    gethostbysock_ (s, hostname);
    if (strcmp (hostname, "LSF_HOST_NULL")) {
        _setcurseqno_ (hostname, getCurrentSN( ) );
    }
    
    cc = lsSendMsg_ (s, cmd, 0, data, reqBuf, reqLen, xdrFunc, b_write_fix, auth);
    if (cc < 0) {
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return -1;
    }

    if (!rd) {
        sigprocmask (SIG_SETMASK, &oldMask, NULL);
        return 0;
    }

    nready = rd_select_ (*rd, timeout);
    if (nready <= 0)
    {
        if (nready == 0) {
            lserrno = LSE_TIME_OUT;
        }
        else {
            lserrno = LSE_SELECT_SYS;
        }
      sigprocmask (SIG_SETMASK, &oldMask, NULL);
      return -1;
    }

  fromsize = sizeof (from);
  t = b_accept_ (*rd, (struct sockaddr *) &from, &fromsize);
  if (t < 0)
    {
      lserrno = LSE_ACCEPT_SYS;
      sigprocmask (SIG_SETMASK, &oldMask, NULL);
      return -1;
    }

  closesocket (*rd);
  *rd = t;

  sigprocmask (SIG_SETMASK, &oldMask, NULL);

  return 0;

}       /* callRes_() */

int
ls_rstty (char *host)
{
    assert ( *host );
    return 0;
}

int
rstty_ (char *host)
{
  return do_rstty1_ (host, FALSE);
}

int
rstty_async_ (char *host)
{

  return do_rstty1_ (host, TRUE);
}

int
do_rstty1_ (char *host, int async)
{
  int s, descriptor[2];
  int redirect, io_fd;

  redirect = 0;
  if (isatty (0))
    {
      if (!isatty (1))
  redirect = 1;
      io_fd = 0;
    }
  else if (isatty (1))
    {
      redirect = 1;
      io_fd = 1;
    }
    else {
        return 0;
    }

  if (_isconnected_ (host, descriptor))
    s = descriptor[0];
  else if ((s = ls_connect (host)) < 0)
    return -1;

  if (!FD_ISSET (s, &connection_ok_))
    {
      FD_SET (s, &connection_ok_);
      if (ackReturnCode_ (s) < 0)
  {
    closesocket (s);
    _lostconnection_ (host);
    return -1;
  }
    }

  if (do_rstty2_ (s, io_fd, redirect, async) == -1)
    {
      closesocket (s);
      _lostconnection_ (host);
      return -1;
    }

  if (!async)
    {
      if (ackReturnCode_ (s) < 0)
  {
    closesocket (s);
    _lostconnection_ (host);
    return -1;
  }
    }

  return 0;
}


int
do_rstty_ (int s, int io_fd, int redirect)
{
    return do_rstty2_ (s, io_fd, redirect, FALSE);
}

int
do_rstty2_ (int s, int io_fd, int redirect, int async)
{
  static int termFlag = FALSE;
  static struct resStty tty;
  char *buf = malloc( sizeof( char ) * MSGSIZE + 1);
  char *cp  = NULL;


  if (!termFlag) {
        termFlag = TRUE;
        tcgetattr (io_fd, &tty.termattr);

        // original if (getpgrp (0) != tcgetpgrp (io_fd)) {
        if (getpgrp () != tcgetpgrp (io_fd)) {
            tty.termattr.c_cc[VEOF] = 04;
            tty.termattr.c_lflag |= ICANON;
            tty.termattr.c_lflag |= ECHO;
        }
        
        if (redirect) {
            tty.termattr.c_lflag &= ~ECHO; // FIXME check that this expresion will never assign negative (it won't, but still)
        }

        if ((cp = getenv ("LINES")) != NULL) {
            assert( atoi(cp) >= 0 && atoi(cp) <= SHRT_MAX );
            tty.ws.ws_row = (unsigned short) atoi (cp);
        }
        else {
            tty.ws.ws_row = 24;
        }
        
        if ((cp = getenv ("COLUMNS")) != NULL) {
            assert( atoi(cp) >= 0 && atoi(cp) <= SHRT_MAX );
            tty.ws.ws_col = (unsigned short) atoi (cp);
        }
        else {
            tty.ws.ws_col = 80;
            tty.ws.ws_xpixel = tty.ws.ws_ypixel = 0;
        }

        if (!async) {
            int callResResult = callRes_ (s, RES_INITTTY, (char *) &tty, buf, MSGSIZE, xdr_resStty, 0, 0, NULL);
            if ( -1 == callResResult ) {
                return -1;
            }
        }
        else {
            int callResResult = callRes_ (s, RES_INITTTY_ASYNC, (char *) &tty, buf, MSGSIZE, xdr_resStty, 0, 0, NULL);
            if ( -1 == callResResult) {
                return -1;
            }
        }
    }

  return 0;
}

int
rgetRusageCompletionHandler_ (struct lsRequest *request)
{
    XDR xdrs;
    int rc;

    rc = resRC2LSErr_ (request->rc);
    if (rc != 0) {
        return -1;
    }

    assert( request->replyBufLen <= INT_MAX); //FIXME FIXME must go over struct lsRequest
    xdrmem_create (&xdrs, request->replyBuf, (unsigned int) request->replyBufLen, XDR_DECODE);
    if( !xdr_jRusage (&xdrs, (struct jRusage *) request->extra, NULL) ) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        return -1;
    }

    xdr_destroy (&xdrs);
    return 0;

}

LS_REQUEST_T *
lsIRGetRusage_ (pid_t rpid, struct jRusage * ru, appCompletionHandler appHandler, void *appExtra, int options)
{
    int s;
  
    struct requestbuf {
        struct resRusage rusageReq;
        char padding[4];
        struct LSFHeader hdr;
    } requestBuf;

    struct lsRequest *request; 
    struct resRusage rusageReq;
    struct tid *tid;
    char host[MAXHOSTNAMELEN];

    tid = tid_find (rpid);
    if ( NULL == tid ) {
        return NULL;
    }

    s = tid->sock;
    gethostbysock_ (s, host);

    if (!FD_ISSET (s, &connection_ok_)) {
        FD_SET (s, &connection_ok_);
        
        if (ackReturnCode_ (s) < 0) {
            closesocket (s);
            _lostconnection_ (host);
            return NULL;
        }
    }

    rusageReq.rid = rpid;
    if (options == 0 || (options & RID_ISTID)) {
        rusageReq.whatid = RES_RID_ISTID;
    }
    else {
        rusageReq.whatid = RES_RID_ISPID;
    }

    if (callRes_ (s, RES_RUSAGE, (char *) &rusageReq, (char *) &requestBuf, sizeof (requestBuf), xdr_resGetRusage, 0, 0, NULL) == -1) // FIXME FIXME FIXME char for data? really?
    {
        closesocket (s);
        _lostconnection_ (host);
        
        return NULL;
    }

    request = lsReqHandCreate_ (rpid, getCurrentSN( ), s, ru, rgetRusageCompletionHandler_, appHandler, appExtra); // FIXME FIXME FIXME char for data? really?

    if (request == NULL) {
        return NULL;
    }

    if (lsQueueDataAppend_ ((char *) request, requestQ)) { // FIXME FIXME FIXME char for data? really?
        lsReqFree_ (request);
        return NULL;
    }

    return request;
}

int
lsGetRProcRusage (char *host, int pid, struct jRusage *ru, int options)
{
    struct requestbuf {
        struct resRusage rusageReq;
        char padding[4];
        struct LSFHeader hdr;
    } requestBuf;

    struct lsRequest *request;
    struct resRusage rusageReq;
    int s;
    int descriptor[2];
    int callResResult = 0;

    if (_isconnected_ (host, descriptor)) {
        s = descriptor[0];
    }
    else {
        lserrno = LSE_LOSTCON;
        return -1;
    }

    if (!FD_ISSET (s, &connection_ok_)) {
        FD_SET (s, &connection_ok_);
        
        if (ackReturnCode_ (s) < 0) {
            closesocket (s);
            _lostconnection_ (host);
            return -1;
        }
    }

    rusageReq.rid = pid;
    rusageReq.whatid = RES_RID_ISPID;

    if (options & RES_RPID_KEEPPID) {
        rusageReq.options |= RES_RPID_KEEPPID;
    }
    else {
        rusageReq.options = 0;
    }
    callResResult = callRes_ (s, RES_RUSAGE, (char *) &rusageReq, (char *) &requestBuf, sizeof (requestBuf), xdr_resGetRusage, 0, 0, NULL); // FIXME FIXME FIXME char for data? really?
    if ( -1 == callResResult )  {
      closesocket (s);
      _lostconnection_ (host);
      return -1;
    }

    request = lsReqHandCreate_ (pid, getCurrentSN( ), s, ru, rgetRusageCompletionHandler_, NULL, NULL);

    if (request == NULL) {
        return -1;
    }

    if (lsQueueDataAppend_ ( (char *) request, requestQ)) { // FIXME FIXME FIXME char for data? really?
        return -1;
    }

    if (!request) {
        return -1;
    }

    if (lsReqWait_ (request, 0) < 0) {
        return -1;
    }

    lsReqFree_ (request);

    return 0;

}

LS_REQUEST_T *
lsGetIRProcRusage_ (char *host, int tid, pid_t pid, struct jRusage * ru, appCompletionHandler appHandler, void *appExtra)
{
    struct requestbuf {
        struct resRusage rusageReq;
        char padding[4];
        struct LSFHeader hdr;
    } requestBuf;

    struct lsRequest *request;
    struct resRusage rusageReq;
    int s = 0;
    int descriptor[2]= {0, 0};

  if (_isconnected_ (host, descriptor))
    s = descriptor[0];
  else
    {
      lserrno = LSE_LOSTCON;
      return NULL;
    }

  if (!FD_ISSET (s, &connection_ok_))
    {
      FD_SET (s, &connection_ok_);
      if (ackReturnCode_ (s) < 0)
  {
    closesocket (s);
    _lostconnection_ (host);
    return NULL;
  }
    }

  rusageReq.rid = pid;
  rusageReq.whatid = RES_RID_ISPID;

  if (callRes_ (s, RES_RUSAGE, (char *)&rusageReq, (char *)&requestBuf, // FIXME FIXME FIXME this is wrong, yo. data as char?
    sizeof (requestBuf), xdr_resGetRusage, 0, 0, NULL) == -1)
    {
      closesocket (s);
      _lostconnection_ (host);
      return NULL;
    }
    request = lsReqHandCreate_ (tid, getCurrentSN( ), s, ru, rgetRusageCompletionHandler_, appHandler, appExtra);

    if (request == NULL) {
        return NULL;
    }

  if (lsQueueDataAppend_ ((char *) request, requestQ))
    {
      lsReqFree_ (request);
      return NULL;
    }

  return request;

}

int
lsRGetRusage (int rpid, struct jRusage *ru, int options)
{
    LS_REQUEST_T *request = NULL;

    request = lsIRGetRusage_ (rpid, ru, NULL, NULL, options);

    if( !request ) {
        return -1;
    }

    if( lsReqWait_( request, 0 ) < 0 ) {
        return -1;
    }
    lsReqFree_ (request);

    return 0;
}

int
sendSig_ (char *host, pid_t rid, int sig, int options)
{
    struct Buff {
        struct resRKill rk;
        char padding[4];
        struct LSFHeader hdr;
    } buf;
    
    struct resRKill killReq;
    int resCallReturnValue = 0;
    int descriptor[2] = { 0,0 };
    int s = 0;

    if (_isconnected_ (host, descriptor)) {
        s = descriptor[0];
    }
    else {
        lserrno = LSE_LOSTCON;
        return -1;
    }

    if (!FD_ISSET (s, &connection_ok_)) {
        FD_SET (s, &connection_ok_);
        if (ackReturnCode_ (s) < 0) {
            closesocket (s);
            _lostconnection_ (host);
            return -1;
        }
    }
    
    if (sig >= _NSIG || sig < 0) {  // FIXME FIXME FIXME substitute _NSIG for appropriate signal or appropriate way of fixing it
        lserrno = LSE_BAD_ARGS;
        return -1;
    }

    killReq.rid = rid;

    if (options & RSIG_ID_ISTID) {
        killReq.whatid = RES_RID_ISTID;
    }
    else if (options & RSIG_ID_ISPID) {
        killReq.whatid = RES_RID_ISPID;
    }
    else {
        lserrno = LSE_BAD_ARGS;
        return -1;
    }

    killReq.signal = sig_encode (sig);

    resCallReturnValue = callRes_ (s, RES_RKILL, (char *) &killReq, (char *) &buf, sizeof (buf), xdr_resRKill, 0, 0, NULL);
    if ( -1 == resCallReturnValue ) {
        closesocket (s);
        _lostconnection_ (host);
        return -1;
    }

    if (ackReturnCode_ (s) < 0)
    {
        if (options & RSIG_KEEP_CONN) {
            return -1;
        }

        closesocket (s);
        _lostconnection_ (host);
        return -1;
    }

    return 0;
}

int
lsRSig_ (char *host, pid_t rid, int sig, int options)
{
    struct connectEnt *conns;
    int nconns  = 0;
    int rc      = 0;
    
    if (!options) {
        options = RSIG_ID_ISTID;
    }

    if ((!(options & (RSIG_ID_ISTID | RSIG_ID_ISPID))) ) {
        lserrno = LSE_BAD_ARGS;
        return -1;
    }

    if (NULL == host) {
        nconns = _findmyconnections_ (&conns);
        if (nconns == 0) {
            return 0;
        }

        for ( int i = 0; i < nconns; i++) {
            host = conns[i].hostname;
            rc = sendSig_ (host, 0, sig, options);
            if( rc < 0) {
                return rc; 
            }
        }
    }
    else {
      return sendSig_ (host, rid, sig, options);
    }
    
    return 0;
}

int
ls_rkill (pid_t rtid, int sig)
{
    int s = 0;
    int rc = 0;
    struct tid *tid;
    char *host = (char *) malloc( sizeof(MAXHOSTNAMELEN) + 1 );

    if ( 0 == rtid ) {
        rc = lsRSig_ (NULL, rtid, sig, RSIG_ID_ISTID);
        return rc;
    }

    tid = tid_find (rtid);
    if( NULL == tid ) {
        return -1;
    }

    s = tid->sock;
    gethostbysock_ (s, host);

    rc = lsRSig_ (host, rtid, sig, RSIG_ID_ISTID);
    return rc;

}


int
lsMsgRdy_ (pid_t taskid, size_t *msgLen)
{
    struct tid *tEnt;
    struct lsQueueEntry *qEnt;
    struct lsTMsgHdr *header;

    tEnt = tidFindIgnoreConn_ (taskid);
    if (tEnt == NULL) {
        return -1;
    }

    if( !LS_QUEUE_EMPTY (tEnt->tMsgQ) ) {
        qEnt = tEnt->tMsgQ->start->forw;
        header = (struct lsTMsgHdr *) qEnt->data;   // FIXME FIXME FIXME FIXME
                                                    //      member-for-member assignment
        if (msgLen != NULL) {
            *msgLen = header->len;
        }
        return 1;
    }
    else {
        return 0;
    }

}

void
tMsgDestroy_ (void *extra)
{
  struct lsTMsgHdr *header = (struct lsTMsgHdr *) extra;

  if (!header)
    return;

  if (header->msgPtr)
    free (header->msgPtr);

  free (header);

}



int
lsMsgRcv_ (pid_t taskid, char *buffer, size_t len, int options)
{
    struct tid *tEnt;
    struct lsQueueEntry *qEnt;
    int rc = 0;
    int nrdy = 0;

    assert( options ); // FIXME find out if any calls actually 

    tEnt = tidFindIgnoreConn_ (taskid);
    if ( NULL == tEnt ) {
        return -1;
    }

// Second Again label was here. replaced by while loop 
    while( nrdy <= 0 ) {

        if (tEnt->isEOF) {
            lserrno = LSE_RES_INVCHILD;
            return -1;
        }

        if (lsMsgRdy_ (taskid, NULL)) {
            struct lsTMsgHdr *header;

            qEnt = lsQueueDequeue_ (tEnt->tMsgQ);

            if (qEnt == NULL) {
                lserrno = LSE_MSG_SYS;
                return -1;
            }

            header = (struct lsTMsgHdr *) qEnt->data;   // FIXME FIXME FIXME FIXME
                                                        //      member-for-member assignment
            if (header->len > len) {
                lserrno = LSE_MSG_SYS;
                lsQueueEntryDestroy_ (qEnt, tEnt->tMsgQ);
                return -1;
            }

            if (header->type == LSTMSG_IOERR) {
                lserrno = LSE_LOSTCON;
                lsQueueEntryDestroy_ (qEnt, tEnt->tMsgQ);
                return -1;
            }
            else if (header->type == LSTMSG_EOF) {
                tEnt->isEOF = TRUE;
                lsQueueEntryDestroy_ (qEnt, tEnt->tMsgQ);
                return 0;
            }

            memcpy ((char *) buffer, (char *) header->msgPtr, header->len);

            assert( header->len <= INT_MAX );
            rc = (int) header->len;             //FIXME FIXME horrible shoehorning. must revistit function return types
            lsQueueEntryDestroy_ (qEnt, tEnt->tMsgQ);
            return rc;
        }
        else {

            if (tEnt->sock < 0) {
                lserrno = LSE_LOSTCON;
                tid_remove (taskid);
                return -1;
            }
            
            rc = lsMsgWait_ (1, &taskid, &nrdy, 0, NULL, NULL, NULL, NULL, 0);
            if (rc < 0) {
                return rc;
            }

            if (nrdy > 0)
                return rc;
            else
                return -1;
        }

    } // end while( nrdy <= 0 )

    return rc; // FIXME do a check if this code is correct
}

int
lsMsgSnd2_ (int *sock, unsigned short opcode, char *buffer, size_t len, int options)
{
    struct LSFHeader header;
    XDR xdrs;
    long rc = 0;
    char *headerBuf = malloc( sizeof( char )*sizeof( struct LSFHeader ) + 1 );
    char *hostname  = malloc( sizeof( char )*MAXHOSTNAMELEN + 1 );

    assert( options ); // FIXME either the assert or int options has to go

    if (*sock < 0) {
      return -1;
    }

#ifdef OS_HAS_PTHREAD
    pthread_mutex_lock (&fdLSLIBWriteMutex);
#endif

    header.opCode = opcode;
    setCurrentSN( REQUESTSN );
    header.refCode = REQUESTSN;
    header.length = len;
    header.reserved = 0;

    gethostbysock_ (*sock, hostname);
    if (strcmp (hostname, "LSF_HOST_NULL")) {
        _setcurseqno_ (hostname, getCurrentSN( ) );
    }

    xdrmem_create (&xdrs, headerBuf, LSF_HEADER_LEN, XDR_ENCODE);
    if (!xdr_LSFHeader (&xdrs, &header)) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        rc = -1;
    }
    xdr_destroy (&xdrs);

    rc = b_write_fix (*sock, headerBuf, LSF_HEADER_LEN);
    if (rc < 0 && EPIPE == errno ) {
        close (*sock);
        *sock = -1;
        _lostconnection_ (hostname);
        lserrno = LSE_LOSTCON;
        rc = -1;
    }

    rc = b_write_fix (*sock, buffer, len);
    if (rc < 0 && errno == EPIPE ) {
        close (*sock);
        *sock = -1;
        _lostconnection_ (hostname);
        lserrno = LSE_LOSTCON;
        rc = -1;
    }
    if (ackReturnCode_ (*sock) < 0) {
        rc = -1;
    }

// AbortSnd2: was here
#ifdef OS_HAS_PTHREAD
    pthread_mutex_unlock (&fdLSLIBWriteMutex);
#endif

    assert( rc <= INT_MAX );
    return (int) rc; 
}

int
lsMsgSnd_ (pid_t taskid, char *buffer, size_t len, int options)
{
    struct LSFHeader header;
    char headerBuf[sizeof (struct LSFHeader)];
    XDR xdrs;
    struct tid *tEnt;
    long rc = 0;
    char hostname[MAXHOSTNAMELEN];

    assert( options ); // FiXME; either the assert or int options has to go

    tEnt = tid_find (taskid);
    if (!tEnt) {
        return -1;
    }

    if (tEnt->sock < 0) {
        lserrno = LSE_LOSTCON;
        return -1;
    }

#ifdef OS_HAS_PTHREAD
    pthread_mutex_lock (&fdLSLIBWriteMutex);
#endif

    header.opCode   = RES_NONRES;
    setCurrentSN( REQUESTSN );
    header.refCode  = getCurrentSN();
    header.length   = len;
    assert( taskid >= 0 );
    header.reserved = taskid;

    gethostbysock_ (tEnt->sock, hostname);
    if (strcmp (hostname, "LSF_HOST_NULL")) {
        _setcurseqno_ (hostname, getCurrentSN( ) );
    }

    xdrmem_create (&xdrs, headerBuf, LSF_HEADER_LEN, XDR_ENCODE);
    if (!xdr_LSFHeader (&xdrs, &header)) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        rc = -1;
    }
    
    xdr_destroy (&xdrs);
    rc = b_write_fix (tEnt->sock, headerBuf, LSF_HEADER_LEN);
    if ( 0 > rc && EPIPE == errno )
    {
        close (tEnt->sock);
        tEnt->sock = -1;
        _lostconnection_ (hostname);
        lserrno = LSE_LOSTCON;
        rc = -1;
    }

    rc = b_write_fix (tEnt->sock, buffer, len);
    if ( 0 > rc && EPIPE == errno) {
        close (tEnt->sock);
        tEnt->sock = -1;
        _lostconnection_ (hostname);
        lserrno = LSE_LOSTCON;
        rc = -1;
    }
    if (ackReturnCode_ (tEnt->sock) < 0) {
        rc = -1;
    }

// AbortSnd: was here

#ifdef OS_HAS_PTHREAD
    pthread_mutex_unlock (&fdLSLIBWriteMutex);
#endif

    assert( rc <= INT_MAX );
    return (int) rc;
}

int
lsMsgWait_ (int inTidCnt, pid_t *tidArray, int *rdyTidCnt, int inFdCnt, int *fdArray, int *rdyFdCnt, int *outFdArray, struct timeval *timeout, int options)
{
    long maxfd      = 0;
    int nready      = 0;
    int rc          = 0;
    int rdycnt      = 0;
    int nBitsSet    = 0;
    struct tid *taskEnt;
    struct LSFHeader msgHdr;

    int goAgain = 0;

    fd_set rm;
    XDR xdrs;
    bool_t tMsgQNonEmpty;
    bool_t anythingRdy;

    char hdrBuf[sizeof (struct LSFHeader)];

    assert( options ); // FIXME

    if ((!rdyTidCnt && !rdyFdCnt) || (!tidArray && !fdArray) || (!inTidCnt && !inFdCnt)) {
        return 0;
    }

    for ( int i = 0; i < inFdCnt; i++) {
        outFdArray[i] = -1;
    }

// Label Again: was here, replaced with while loop

    while( ! goAgain ) {

        tMsgQNonEmpty = FALSE;

        FD_ZERO (&rm);
        nBitsSet = 0;
        if (rdyTidCnt) {
            *rdyTidCnt = 0;
        }
        if (rdyFdCnt) {
            *rdyFdCnt = 0;
        }

        if (inFdCnt > 0 && fdArray) {
            for ( int i = 0; i < inFdCnt; i++)
            {
                if (FD_NOT_VALID (fdArray[i]))
                {
                    lserrno = LSE_BAD_ARGS;
                    rc = -1;
                    return rc;
                }
                FD_SET (fdArray[i], &rm);
                nBitsSet++;
            }

            rdycnt = 0;
            if (inTidCnt && tidArray)
            {
                for ( int i = 0; i < inTidCnt; i++)
                {
                    rc = lsMsgRdy_ (tidArray[i], NULL);
                    if (rc > 0)
                    {

                        tMsgQNonEmpty = TRUE;
                        rdycnt++;
                        continue;
                    }

                    taskEnt = tid_find (tidArray[i]);

                    if (taskEnt == NULL)
                    {
                        rc = -1;
                        return rc;
                    }

                    if (FD_NOT_VALID (taskEnt->sock))
                    {
                        lserrno = LSE_BAD_ARGS;
                        rc = -1;
                        return rc;
                    }

                    nBitsSet++;
                    
                    FD_SET (taskEnt->sock, &rm);
                }
                
                if (tMsgQNonEmpty)
                {
                    *rdyTidCnt = rdycnt;
                    return 0;
                }
            }

            if (nBitsSet == 0) {
                return 0;
            }

            maxfd = sysconf (_SC_OPEN_MAX);
            if (maxfd > 1024) {
                maxfd = 1024 - 1;           // FIXME FIXME FIXME probably a bug here
                                            //      investigate why is there a 1024 - 1 assignment
            }
            
            assert( maxfd <= INT_MAX && maxfd >= INT_MIN );
            nready = select( (int)maxfd, &rm, NULL, NULL, timeout);

            if (nready < 0) {
                if (errno == EINTR) {
                    return rc;
                }
            }
            else {
                lserrno = LSE_SELECT_SYS;
                rc = -1;
                return rc;
            }
        }

        if (rdyFdCnt) {

            rdycnt = 0;
            for ( int i = 0; i < inFdCnt; i++) {
                if (FD_ISSET (fdArray[i], &rm))
                {
                    rdycnt++;
                    outFdArray[i] = fdArray[i];
                }
                else
                outFdArray[i] = -1;
            }
            *rdyFdCnt = rdycnt;
        }

        if ( rdyTidCnt && nready == 0 )  {

            *rdyTidCnt = 0;
            return 0;
        }

        if (rdyTidCnt) {

            rdycnt = 0;
            xdrmem_create (&xdrs, hdrBuf, sizeof (struct LSFHeader), XDR_DECODE);

            for ( int i = 0; i < inTidCnt; i++) {
                taskEnt = tidFindIgnoreConn_ (tidArray[i]);

                if (taskEnt == NULL) {
                    rc = -1;
                    xdr_destroy (&xdrs);
                    return rc;
                }

                if (FD_NOT_VALID (taskEnt->sock)) {
                    continue;
                }

                if (!FD_ISSET (taskEnt->sock, &rm)) {
                    continue;
                }

                xdr_setpos (&xdrs, 0);
                rc = readDecodeHdr_ (taskEnt->sock, hdrBuf, b_read_fix, &xdrs, &msgHdr);

                if (rc < 0) {

                    int nTids = 0;
                    int *tidSameConns = 0;
                    int tidIDx = 0;

                    rc = tidSameConnection_ (taskEnt->sock, &nTids, &tidSameConns);
                    for (tidIDx = 0; tidIDx < nTids; tidIDx++) {

                        assert( tidSameConns[tidIDx] >= 0 );
                        rc = enqueueTaskMsg_ (-1, tidSameConns[tidIDx], NULL);
                        if (rc < 0) {

                            free (tidSameConns);
                            xdr_destroy (&xdrs);
                            return rc;
                        }
                    }
                
                    rdycnt += nTids;
                    free (tidSameConns);
                    _lostconnection_ (taskEnt->host);
                }
                else if (msgHdr.opCode != RES_NONRES) {

                    rc = ackAsyncReturnCode_ (taskEnt->sock, &msgHdr);
                    if (rc < 0 ) {
                        xdr_destroy (&xdrs);
                        return rc;
                    }
                }
                else {

                    rc = enqueueTaskMsg_ (taskEnt->sock, msgHdr.refCode, &msgHdr);
                    if (rc < 0) {
                        xdr_destroy (&xdrs);
                        return rc;
                    }

                    FD_CLR (taskEnt->sock, &rm);
                    rdycnt++;

                } // end if (rc < 0)
            
            } // end for (i = 0; i < inTidCnt; i++) 

            xdr_destroy (&xdrs);
            *rdyTidCnt = rdycnt;

        }  // end if (rdyTidCnt)

        anythingRdy = FALSE;
        if (*rdyTidCnt > 0 ) {
            anythingRdy = TRUE;
        }

        if (*rdyFdCnt > 0) {
            anythingRdy = TRUE;
        }


        if (!anythingRdy) {
            goAgain = 1;
        }

    } // end while( goAgain )

    rc = 0;  // FIXME FIXME must return real value

    return rc;

}

int
lsReqCmp_ (char *val, char *reqEnt, int hint)
{
    pid_t seqno = 0;
    struct lsRequest *req;

    assert(hint);

    req = (struct lsRequest *) reqEnt;  // FIXME FIXME FIXME FIXME
                                        //      got to take a look at the data in reqEnt and then
                                        //      make a propper piece by piece assignment to the 
                                        //      struct
    seqno = (pid_t) *val;       // FIXME FIXME FIXME is cast applicable?

    if (seqno == req->seqno) {
        return 0;
    }
    else if (seqno > req->seqno) {
        return -1;
    }
    else {
        return 1;
    }
}

int
lsReqTest_ (LS_REQUEST_T * request)
{
    if (!request) {
        return FALSE;
    }

    if (!request->completed) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

int
lsReqWait_ (LS_REQUEST_T * request, int options)
{
    int rc = 0;
    struct LSFHeader header;

    assert( options );  // FIXME has to go, or options has to go

    if (!request) {
        return -1;
    }

    if (lsReqTest_ (request)) {
        return 0;
    }

    rc = expectReturnCode_ (request->connfd, request->seqno, &header);
    if (rc < 0) {
        return rc;
    }

    rc = ackAsyncReturnCode_ (request->connfd, &header);
    return rc;

}


void
lsReqFree_ (LS_REQUEST_T * request)
{
  if (request)
    free ((void *) request);
}

void
_lostconnection_ (char *hostName)
{
  int connSockNum;

  connSockNum = getConnectionNum_ (hostName);
  if (connSockNum < 0)
    return;

  tid_lostconnection (connSockNum);
  FD_CLR (connSockNum, &connection_ok_);

}
