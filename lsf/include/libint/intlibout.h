/* $Id: intlibout.h 397 2007-11-26 19:04:00Z mblack $
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
 
#include "libint/link.h"
#include "libint/list2.h"
#include "libint/listset.h"
#include "libint/lsftcl.h"
#include "libint/resreq.h"

#include "lsf.h"

#define MINPASSWDLEN_LS      (3)
#define EXIT_NO_ERROR        (0)
#define EXIT_FATAL_ERROR     (-1)
#define EXIT_WARNING_ERROR (-2)
#define EXIT_RUN_ERROR     (-8)

#define MAXADDRSTRING 256

struct windows
{
  struct windows *nextwind;
  float opentime;
  float closetime;
};

typedef struct windows windows_t;

struct dayhour
{
  short day;
  char padding[2];
  float hour;
};

struct listEntry
{
    int entryData;
    char padding[4];
    struct listEntry *forw;
    struct listEntry *back;

};

// static char chosenPath[MAX_PATH_LEN];

void daemonize_ (void);
void saveDaemonDir_ (char *);
char *getDaemonPath_ (char *, char *);
int mychdir_ (char *, struct hostent *);
int myopen_ (char *, int, int, struct hostent *);
FILE *myfopen_ (const char *filename, const char *type, struct hostent * hp);
int mystat_ (char *, struct stat *, struct hostent *);
int mychmod_ (char *, mode_t, struct hostent *);
int mymkdir_ (char *, mode_t, struct hostent *);
void myexecv_ (char *, char **, struct hostent *);
int myunlink_ (char *, struct hostent *, int);
int myrename_ (char *, char *, struct hostent *);
int addWindow (char *wordpair, windows_t * week[], char *context);
void insertW (windows_t ** window, float ohour, float chour);
void checkWindow (struct dayhour *dayhour, char *active, time_t * wind_edge, windows_t * wp, time_t now);
void getDayHour (struct dayhour *dayPtr, time_t nowtime);
void delWindow (windows_t * wp);

int hostOk (char *, int);
int hostIsLocal (char *);
int getHostAttribNonLim (char *hname, int updateIntvl);
void initParse (struct lsInfo *);
int getResEntry (const char *);
int hostValue (void);
int getBootTime (time_t *);
int procChangeUser_ (char *);
char *encryptPassLSF (char *);
char *decryptPassLSF (char *);
char *encryptByKey_ (char *, char *);
char *decryptByKey_ (char *, char *);
int matchName (char *, char *);
int readPassword (char *);
char **parseCommandArgs (char *, char *);
int FCLOSEUP (FILE ** fp);
int withinAddrRange (char *addrRange, char *address);
int validateAddrRange (char *addrRange);
char *mystrncpy (char *s1, const char *s2, size_t n);
void openChildLog (const char *defLogFileName, const char *confLogDir, int use_stderr, char **confLogMaskPtr);
void cleanDynDbgEnv (void);
struct listEntry *mkListHeader (void);
void offList (struct listEntry *);
void inList (struct listEntry *, struct listEntry *);
int getResourceNames (int, char **, int, char **);
void displayShareResource (int, char **, int, int, int);
int makeShareField (char *, int, char ***, char ***, char ***);
char *getMAC (int *length);
char *mac2hex (char *mac, int len);

