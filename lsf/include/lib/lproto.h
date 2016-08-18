/* $Id: lproto.h 397 2007-11-26 19:04:00Z mblack $
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
#include "lib/conf.h"
#include "lib/channel.h"
#include "lib/hdr.h"
#include "lib/table.h"
#include "libint/lsi18n.h"
#include "daemons/libpimd/pimd.h"
#include "daemons/libresd/resd.h"
#include "daemons/libresd/resout.h"
 

#define BIND_RETRY_TIMES 100

struct admins
{
  char padding[4];
  unsigned int nAdmins;
  uid_t *adminIds;
  gid_t *adminGIds;
  char **adminNames;
};


void putMaskLevel (int, char **);

#define    MBD_DEBUG         1
#define    MBD_TIMING        2
#define    SBD_DEBUG         3
#define    SBD_TIMING        4
#define    LIM_DEBUG         5
#define    LIM_TIMING        6
#define    RES_DEBUG         7
#define    RES_TIMING        8

struct resPair
{
  char *name;
  char *value;
};

struct sharedResource
{
  char *resourceName;
  uint numInstances;
  char padding1[4];
  struct resourceInstance **instances;
};

struct resourceInfoReq
{
  uint numResourceNames;
  int options;
  char *hostName;
  char **resourceNames;
};

struct resourceInfoReply
{
    uint numResources;
    uint badResource;
    struct lsSharedResourceInfo *resources;
};


#define HOST_ATTR_SERVER        (0x00000001)
#define HOST_ATTR_CLIENT        (0x00000002)
#define HOST_ATTR_NOT_LOCAL     (0x00000004)
#define HOST_ATTR_NOT_READY     (0xffffffff)

int sharedResConfigured_;

#define VALID_IO_ERR(x) ((x) == EWOULDBLOCK || (x) == EINTR || (x) == EAGAIN)
#define BAD_IO_ERR(x)   ( ! VALID_IO_ERR(x))

#define INVALID_FD      (-1)
#define FD_IS_VALID(x)  ((x) >= 0 && (x) < sysconf(_SC_OPEN_MAX) )
#define FD_NOT_VALID(x) ( ! FD_IS_VALID(x))

#define AUTH_IDENT      "ident"
#define AUTH_PARAM_DCE  "dce"
#define AUTH_PARAM_EAUTH  "eauth"
#define AUTOMOUNT_LAST_STR "AMFIRST"
#define AUTOMOUNT_NEVER_STR "AMNEVER"

#define FREEUP(pointer)   if (pointer != NULL) {  \
                              free(pointer);      \
                              pointer = NULL;     \
                          }

#define STRNCPY(str1, str2, len)  { strncpy(str1, str2, len); \
                                    str1[len -1] = '\0';  \
                                  }

#define IS_UNC(a) \
        ((a!=NULL) && (*a == '\\') && (*(a+1) == '\\') ? TRUE : FALSE)

#define TRIM_LEFT(sp) if (sp != NULL) { \
                          while (isspace(*(sp))) (sp)++; \
                      }
#define TRIM_RIGHT(sp)     while (isspace(*(sp+strlen(sp)-1))) *(sp+strlen(sp)-1)='\0';

#define ALIGNWORD_(s)    (((s)&0xfffffffc) + 4)
#define NET_INTADDR_(a) ((char *) (a))

#define NET_INTSIZE_ 4

#define XDR_DECODE_SIZE_(a) (a)

#define LS_EXEC_T "LS_EXEC_T"


#define GET_INTNUM(i) ((i)/INTEGER_BITS + 1)
#define SET_BIT(bitNo, integers)           \
    integers[(bitNo)/INTEGER_BITS] |= (1<< (bitNo)%INTEGER_BITS);
#define CLEAR_BIT(bitNo, integers)           \
    integers[(bitNo)/INTEGER_BITS] &= ~(1<< (bitNo)%INTEGER_BITS);
#define TEST_BIT(bitNo, integers, isSet)  \
   {  \
      if (integers[(bitNo)/INTEGER_BITS] & (1<<(bitNo)%INTEGER_BITS))  \
          isSet = 1;         \
      else                   \
          isSet = 0;         \
   }

#define FOR_EACH_WORD_IN_SPACE_DELIMITED_STRING(String, Word) \
    if ((String) != NULL) { \
        char *Word; \
        while (((Word) = getNextWord_(&String)) != NULL) { \

#define END_FOR_EACH_WORD_IN_SPACE_DELIMITED_STRING }}

#define LSF_LIM_ERESOURCE_OBJECT        "liblimvcl.so"
#define LSF_LIM_ERESOURCE_VERSION       "lim_vcl_get_eres_version"
#define LSF_LIM_ERESOURCE_DEFINE        "lim_vcl_get_eres_def"
#define LSF_LIM_ERESOURCE_LOCATION      "lim_vcl_get_eres_loc"
#define LSF_LIM_ERESOURCE_VALUE         "lim_vcl_get_eres_val"
#define LSF_LIM_ERES_TYPE "!"

/* FIXME FIXME FIXME : int resCmd bellow may be not int! */
int lsResMsg_ (int, int resCmd, char *, char *, int, bool_t (*)(), int *, struct timeval *);
int expectReturnCode_ (int s, pid_t seqno, struct LSFHeader *repHdr);
int ackAsyncReturnCode_ (int, struct LSFHeader *);
int resRC2LSErr_ (int);
int ackReturnCode_ (int);


#define LSF_O_RDONLY    00000
#define LSF_O_WRONLY    00001
#define LSF_O_RDWR      00002
#define LSF_O_NDELAY    00004
#define LSF_O_NONBLOCK  00010
#define LSF_O_APPEND    00020
#define LSF_O_CREAT     00040
#define LSF_O_TRUNC     00100
#define LSF_O_EXCL      00200
#define LSF_O_NOCTTY    00400

#define LSF_O_CREAT_DIR 04000

int getConnectionNum_ (char *hostName);
void inithostsock_ (void);

int initenv_ (struct config_param *, char *);
char *lsTmpDir_;
short getMasterCandidateNoByName_ (char *);
char *getMasterCandidateNameByNo_ (short);
int getNumMasterCandidates_ ();
int initMasterList_ ();
int getIsMasterCandidate_ ();
void freeupMasterCandidate_ (int);
char *resetLSFUsreDomain (char *);


int runEsub_ (struct lenData *, char *);
int runEexec_ (char *, int, struct lenData *, char *);
int runEClient_ (struct lenData *, char **);
char *runEGroup_ (char *, char *);

int getAuth_ (struct lsfAuth *, char *);
int verifyEAuth_ (struct lsfAuth *, struct sockaddr_in *);
int putEauthClientEnvVar (char *);
int putEauthServerEnvVar (char *);

void sw_remtty (int);
void sw_loctty (int);


int niosCallback_ (struct sockaddr_in *from, u_short port, int rpid, int exitStatus, int terWhiPendStatus);

int sig_encode (int);
int sig_decode (int);
int getSigVal (char *);
char *getSigSymbolList (void);
char *getSigSymbol (int);
void (*Signal_ (int, void (*)(int))) (int);
int blockALL_SIGS_ (sigset_t *, sigset_t *);

int TcpCreate_ (int, int);

int encodeTermios_ (XDR *, struct termios *);
int decodeTermios_ (XDR *, struct termios *);
int rstty_ (char *host);
int rstty_async_ (char *host);
int do_rstty_ (int, int, int);

char isanumber_ (char *);
char islongint_ (char *);
unsigned int isint_ (char *);
int isdigitstr_ (char *);
char *putstr_ (const char *);
int ls_strcat (char *, int, char *);
char *mygetwd_ (char *);
char *chDisplay_ (char *);
void initLSFHeader_ (struct LSFHeader *);
struct group *mygetgrnam (const char *name);
void *myrealloc (void *ptr, size_t size);
char *getNextToken (char **sp);
int getValPair (char **resReq, int *val1, int *val2);
char *my_getopt (int nargc, char **nargv, char *ostr, char **errMsg);
int putEnv (char *env, char *val);
int Bind_ (int sockfd, struct sockaddr *myaddr, socklen_t addrlen);
const char *getCmdPathName_ (const char *cmdStr, long *cmdLen);
int replace1stCmd_ (const char *oldCmdArgs, const char *newCmdArgs, char *outCmdArgs, int outLen);
const char *getLowestDir_ (const char *filePath);
void getLSFAdmins_ (void);
bool_t isLSFAdmin_ (const char *name);
int isAllowCross (char *paramValue);
int isMasterCrossPlatform (void);
LS_LONG_INT atoi64_ (char *);

void stripDomain_ (char *);
int equalHost_ (const char *, const char *);
char *sockAdd2Str_ (struct sockaddr_in *);

struct hostent *Gethostbyname_ (char *);
struct hostent *Gethostbyaddr_ (in_addr_t *, socklen_t, int);
int getAskedHosts_ (char *optarg_, char ***askedHosts, uint *numAskedHosts, unsigned long *badIdx, int checkHost);
int lockHost_ (time_t, char *);
int unlockHost_ (char *);

int lsfRu2Str (FILE *, struct lsfRusage *);
int str2lsfRu (char *, struct lsfRusage *, int *);
void lsfRusageAdd_ (struct lsfRusage *, struct lsfRusage *);

void inserttask_ (char *, hTab *);
int deletetask_ (char *, hTab *);
long listtask_ (char ***taskList, hTab *tasktb, int sortflag);
int readtaskfile_ (char *, hTab *, hTab *, hTab *, hTab *, char);
int writetaskfile_ (char *, hTab *, hTab *, hTab *, hTab *);

int expSyntax_ (char *);

char *getNextLineC_ (FILE *fp, uint *lineNum, int confFormat);
char *getNextLine_ (FILE *, int);
char *getNextWord_ (char **);
char *getNextWord1_ (char **line);
char *getNextWordSet (char **, const char *);
char *getline_ (FILE * fp, int *);
char *getThisLine_ (FILE * fp, int *LineCount);
char *getNextValueQ_ (char **, char, char);
int stripQStr (char *q, char *str);
int addQStr (FILE *, char *str);
struct pStack *initStack (void);
int pushStack (struct pStack *, struct confNode *);
struct confNode *popStack (struct pStack *);
void freeStack (struct pStack *);

char *getNextLineD_ (FILE *fp, uint *lineNum, int truefalse);
char *getNextLineC_conf (struct lsConf *conf, uint *lineNum, int confFormat);
char *getNextLine_conf (struct lsConf *, int);
char *nextline_ (FILE *);
void subNewLine_ (char *);

void doSkipSection (FILE *fp, uint *lineNum, char *fname, char *unknown);
int isSectionEnd (char *linep, char *fname, uint *lineNum, char *newindex);
int keyMatch (struct keymap *keyList, char *line, int exact);
int mapValues (struct keymap *keyList, char *line);
int readHvalues (struct keymap *keyList, char *linep, FILE *fp, char *fname, uint *lineNum, int boolean, char *newindex);
char *getNextValue (char **line);
int putValue (struct keymap *keyList, char *key, char *value);
char *getBeginLine (FILE *fp, uint *lineNum);
int putInLists (char *word, struct admins *admins, uint *numAds, char *forWhat);
int isInlist (char **adminNames, char *userName, uint actAds);

void doSkipSection_conf (struct lsConf *conf, uint *lineNum, char *lsfile, char *sectionName);
char *getBeginLine_conf (struct lsConf *conf, uint *lineNum);

void defaultAllHandlers (void);

size_t  nb_read_fix  (int s, char *buf, size_t len);
long  nb_write_fix (int s, char *buf, size_t len);
long  nb_read_timeout (int s, char *buf, size_t len, int timeout);
int   b_write_timeout (int, char *, int, int);
int   detectTimeout_ (int, int);
int   b_connect_ (int s, struct sockaddr *name, socklen_t namelen, unsigned int timeout);
int   rd_select_ (int, struct timeval *);
int   b_accept_ (int, struct sockaddr *, socklen_t *);
int   blockSigs_ (int, sigset_t *, sigset_t *);
long  b_write_fix  (int s, char *buf, size_t len);
size_t b_read_fix   (int s, char *buf, size_t len);

int readDecodeHdr_ (int s, char *buf, size_t (*readFunc) (), XDR * xdrs, struct LSFHeader *hdr);
int readDecodeMsg_ (int s, char *buf, struct LSFHeader *hdr, size_t (*readFunc) (), XDR * xdrs, char *data, bool_t (*xdrFunc) (), struct lsfAuth *auth);
int writeEncodeMsg_ (int s, char *buf, uint len, struct LSFHeader *hdr, char *data, long (*writeFunc) (), bool_t (*xdrFunc) (), int options);
int writeEncodeHdr_ (int s, struct LSFHeader *sendHdr, long (*)());
int lsSendMsg_ (int s, unsigned short opCode, size_t hdrLength, char *data, char *reqBuf, size_t reqLen, bool_t (*xdrFunc) (), long (*writeFunc) (),  struct lsfAuth *auth);
int lsRecvMsg_ (int sock, char *buf, unsigned int bufLen, struct LSFHeader *hdr, char *data, bool_t (*xdrFunc) (), size_t (*readFunc) ());

int io_nonblock_ (int);
int io_block_ (int);

void millisleep_ (int);

void rlimitEncode_ (struct lsfLimit *, struct rlimit *, int);
void rlimitDecode_ (struct lsfLimit *, struct rlimit *, int);

void verrlog_ (int level, FILE * fp, const char *fmt, va_list ap);

int errnoEncode_ (int);
int errnoDecode_ (int);

int getLogClass_ (char *, char *);
int getLogMask (char **, char *);
void ls_openlog (const char *, const char *, int, char *);
void ls_closelog (void);
int ls_setlogmask (int maskpri);

void initkeylist (struct keymap *keyList, int, int, struct lsInfo *);
void freekeyval (struct keymap *keyList);
char *parsewindow (char *val, char *fname, uint *lineNum, char *host);

uint expandList_ (char ***tolist, uint mask, char **keys);
uint expandList1_ (char ***tolist, uint num, uint *bitmMaps, char **keys);

int osInit_ (void);
char *osPathName_ (char *);
char *osPathName_ (char *);
char *osHomeEnvVar_ (void);
int osProcAlive_ (int);
void osConvertPath_ (char *);

void xdr_lsffree (bool_t (*)(), char *, struct LSFHeader *);

int createUtmpEntry (char *, pid_t, char *);
int removeUtmpEntry (pid_t);

int createSpoolSubDir (const char *);


struct passwd *getpwlsfuser_ (const char *lsfUserName);
struct passwd *getpwdirlsfuser_ (const char *lsfUserName);

int getLSFUser_ (char *lsfUserName, unsigned int lsfUserNameSize);
int getLSFUserByName_ (const char *osUserName, char *lsfUserName, unsigned int lsfUserNameSize);
int getLSFUserByUid_ (uid_t uid, char *lsfUserName, unsigned int lsfUserNameSize);
int getOSUserName_ (const char *lsfUserName, char *osUserName, unsigned int osUserNameSize);
int getOSUid_ (const char *lsfUserName, uid_t * uid);

