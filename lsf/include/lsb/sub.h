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


#define PRINT_ERRMSG0(errMsg, fmt)\
    {\
	if (errMsg == NULL)\
	    fprintf(stderr, fmt);\
        else\
	    sprintf(*errMsg, fmt);\
    }
#define PRINT_ERRMSG1(errMsg, fmt, msg1)\
    {\
	if (errMsg == NULL)\
	    fprintf(stderr, fmt, msg1);\
        else\
	    sprintf(*errMsg, fmt, msg1);\
    }
#define PRINT_ERRMSG2(errMsg, fmt, msg1, msg2)\
    {\
	if (errMsg == NULL)\
	    fprintf(stderr, fmt, msg1, msg2);\
        else\
	    sprintf(*errMsg, fmt, msg1, msg2);\
    }
#define PRINT_ERRMSG3(errMsg, fmt, msg1, msg2, msg3)\
    {\
	if (errMsg == NULL)\
	    fprintf(stderr, fmt, msg1, msg2, msg3);\
        else\
	    sprintf(*errMsg, fmt, msg1, msg2, msg3);\
    }

static int optionFlag = FALSE;
char optionFileName[MAXLSFNAMELEN];
char *loginShell;

static char *additionEsubInfo = NULL;
static hTab *bExceptionTab;

int mySubUsage_ (void *);
int bExceptionTabInit (void);
void subUsage_ (int, char **);
char *niosArgv[5];
char niosPath[MAXFILENAMELEN];

extern const char *defaultSpoolDir;
extern void sub_perror (char *usrMsg);
extern char *my_getopt (int, char **, char *, char **);
extern char *getNextLine_ (FILE * fp, int confFormat);
extern uid_t getuid (void);
extern char *lsb_sysmsg (void);

static const char *getDefaultSpoolDir ();
static void trimSpaces (char *str);
static int parseXF (struct submit *, char *, char **);
static int checkLimit (int limit, int factor);



typedef int (*bException_handler_t) (void *);

typedef struct bException
{
  char *name;
  bException_handler_t handler;
} bException_t;

extern char **environ;
extern char *yyerr;
extern int _lsb_conntimeout;
extern int lsbMode_;
static char *useracctmap = NULL;
static struct lenData ed = { 0, NULL };

static LS_LONG_INT send_batch (struct submitReq *, struct lenData *, struct submitReply *, struct lsfAuth *);
static int dependCondSyntax (char *);
static int createJobInfoFile (struct submit *, struct lenData *);
static LS_LONG_INT subRestart (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth);
static LS_LONG_INT subJob (struct submit *jobSubReq, struct submitReq *submitReq, struct submitReply *submitRep, struct lsfAuth *auth);
static int getUserInfo (struct submitReq *, struct submit *);
static char *acctMapGet (int *, char *);

static int xdrSubReqSize (struct submitReq *req);
static int createNiosSock (struct submitReq *);

int getChkDir (char *, char *);
static void startNios (struct submitReq *, int, LS_LONG_INT);
static void postSubMsg (struct submit *, LS_LONG_INT, struct submitReply *);
static int readOptFile (char *filename, char *childLine);

static const LSB_SPOOL_INFO_T *chUserCopySpoolFile (const char *srcFile, spoolOptions_t fileType);

extern void makeCleanToRunEsub ();
extern char *translateString (char *);
extern void modifyJobInformation (struct submit *);
extern void compactXFReq (struct submit *);
extern char *wrapCommandLine (char *);
extern char *unwrapCommandLine (char *);
extern int checkEmptyString (char *);
extern int stringIsToken (char *, char *);
extern int stringIsDigitNumber (char *s);
extern int processXFReq (char *key, char *line, struct submit *jobSubReq);
extern char *extractStringValue (char *line);


