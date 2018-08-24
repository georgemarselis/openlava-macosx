/* $Id: lsb.sub.c 397 2007-11-26 19:04:00Z mblack $
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
#include "lsb/lsb.h"
#include "lib/words.h"


#define LSF_NIOSDIR 0
#define SKIPSPACE(sp)      while (isspace(*(sp))) (sp)++; // FIXME FIXME this bullshit got to go


char *getSpoolHostBySpoolFile (const char *spoolFile);
typedef struct lsbSpoolInfo
{
  char *srcFile;  // [MAX_FILENAME_LEN]; // FIXME FIXME FIXME FIXME change to dynamic allocation
  char *spoolFile; // [MAX_FILENAME_LEN];

} LSB_SPOOL_INFO_T;

typedef enum spoolOptions
{
  SPOOL_INPUT_FILE,
  SPOOL_COMMAND
} spoolOptions_t;

LSB_SPOOL_INFO_T *copySpoolFile (const char *srcFilePath, spoolOptions_t option);
int removeSpoolFile (const char *hostName, const char *destinFileFullPath);

int optionFlag = FALSE;
char *optionFileName; // [MAX_LSF_NAME_LEN]; // FIXME FIXME FIXME change to dynamic allocation
//char *loginShell;

char *additionEsubInfo = NULL;
struct hTab *bExceptionTab;
int mySubUsage_ (void *);
int bExceptionTabInit (void);
void subUsage_ (int, char **);
 char *niosArgv[5];
char *niosPath; // [MAX_FILENAME_LEN]; // FIXME FIXME FIXME change to dynamic allocation

const char *defaultSpoolDir;
void sub_perror (char *usrMsg);
char *my_getopt (int, char **, char *, char **);
char *getNextLine_ (FILE * fp, int confFormat);
uid_t getuid (void);
char *lsb_sysmsg (void);


typedef int (*bException_handler_t) (void *);

typedef struct bException
{
  char *name;
  bException_handler_t handler;
} bException_t;

char **environ;
char *yyerr;
//int _lsb_conntimeout;
//int lsbMode_;
static char *useracctmap = NULL;
static struct lenData ed = { 0, NULL };

int getChkDir (char *, char *);



void makeCleanToRunEsub( void );
char *translateString (char *);
void modifyJobInformation (struct submit *);
void compactXFReq (struct submit *);
char *wrapCommandLine (char *);
char *unwrapCommandLine (char *);
int checkEmptyString (char *);
int stringIsToken (char *, char *);
int stringIsDigitNumber (char *s);
int processXFReq (char *key, char *line, struct submit *jobSubReq);
char *extractStringValue (char *line);

void PRINT_ERRMSG0( char **errMsg, char *fmt);
void PRINT_ERRMSG1( char **errMsg, char *fmt, char *msg1);
void PRINT_ERRMSG2( char **errMsg, char *fmt, char *msg1, char *msg2);
void PRINT_ERRMSG3( char **errMsg, char *fmt, char *msg1, char *msg2, char *msg3);


int checkSubDelOption ( struct submit *req, int option );
int checkSubDelOption2( struct submit *req, int option2 );
int checkRLDelOption( struct submit *req, int rLimit );

int getCommonParams (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep);
int getOtherParams (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth, LSB_SUB_SPOOL_FILE_T * subSpoolFiles);
int gettimefor (char *toptarg, time_t * tTime);
int setOption_ (int argc, char **argv, char *template, struct submit *req, int mask, int mask2, char **errMsg);
struct submit *parseOptFile_ (char *filename, struct submit *req, char **errMsg);


/////////////////////////////////////////////////////
// function prototypes ( @ file scope )
void trimSpaces (char *str);
int parseXF (struct submit *, char *, char **);
int checkLimit (int limit, int factor);
long  send_batch (struct submitReq *, struct lenData *, struct submitReply *, struct lsfAuth *);
int dependCondSyntax (char *);
int createJobInfoFile (struct submit *, struct lenData *);
long subRestart (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth);
long subJob (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth);
int getUserInfo (struct submitReq *, struct submit *);
char *acctMapGet (int *, char *);
int xdrSubReqSize (struct submitReq *req);
void postSubMsg (struct submit *, long, struct submitReply *);
int readOptFile (char *filename, char *childLine);
const LSB_SPOOL_INFO_T *chUserCopySpoolFile (const char *srcFile, spoolOptions_t fileType);


// nios
int createNiosSock (struct submitReq *);
void startNios (struct submitReq *, int, long) __attribute__ ((noreturn));


int parseLine (char *line, int *embedArgc, char ***embedArgv, int option);
int parseScript (FILE * from, int *embedArgc, char ***embedArgv, int option);
int addLabel2RsrcReq (struct submit *subreq);
int CopyCommand (char **, int);
void sub_perror (char *);


