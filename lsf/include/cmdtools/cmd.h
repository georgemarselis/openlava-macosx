/* $Id: cmd.h 397 2007-11-26 19:04:00Z mblack $
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#include "lib/hdr.h"
#include "lib/lproto.h"
#include "lib/mls.h"
#include "lib/table.h"
#include "lsb/lsbatch.h"
#include "lsf.h"

#define MIN_CPU_TIME 0.0001

#define SIGCHK   -1
#define SIGDEL   -2
#define SIGFORCE -3

#define MAX_JOB_IDS  100

#define CMD_BSUB            1
#define CMD_BRESTART        2
#define CMD_BMODIFY         3

#define LONG_FORMAT     1
#define WIDE_FORMAT     2

#define QUEUE_HIST      1
#define HOST_HIST       2
#define MBD_HIST        3
#define SYS_HIST        4

#define BJOBS_PRINT             1
#define BHIST_PRINT_PRE_EXEC    2
#define BHIST_PRINT_JOB_CMD     3

#ifndef MIN
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define TRUNC_STR(s,len) \
{ \
    int mystrlen = strlen(s); \
    if (mystrlen > (len)) \
    {\
        s[0] = '*';\
         \
        memmove((s) + 1, (s) + mystrlen + 1 - (len), (len)); \
    }\
}

struct histReq
{
  int opCode;
  char **names;
  time_t eventTime[2];
  char *eventFileName;
  int found;
};

void prtLine (char *);
char *get_status (struct jobInfoEnt *job);
void prtHeader (struct jobInfoEnt *, int, int);
void prtJobSubmit (struct jobInfoEnt *, int, int);
void prtFileNames (struct jobInfoEnt *, int);
void prtSubDetails (struct jobInfoEnt *, char *, float);
void prtJobStart (struct jobInfoEnt *, int, int, int);
void prtJobFinish (struct jobInfoEnt *, struct jobInfoHead *);
void prtAcctFinish (struct jobInfoEnt *);
struct loadIndexLog *initLoadIndex (void);
int fillReq (int, char **, int, struct submit *);
void prtErrMsg (struct submit *, struct submitReply *);
void prtBTTime (struct jobInfoEnt *);
void prtJobReserv (struct jobInfoEnt *);
void displayLong (struct jobInfoEnt *, struct jobInfoHead *, float);

int lsbMode_;

void prtBETime_ (struct submit *);

int supportJobNamePattern (char *);


int repeatedName (char *, char **, int);
void jobInfoErr (LS_LONG_INT, char *, char *, char *, char *, int);
int printThresholds (float *, float *, int *, int *, int,
			    struct lsInfo *);
void prtResourceLimit (int *, char *, float, int *);
int getNames (int, char **, int, char ***, int *, char *);
int getJobIds (int, char **, char *, char *, char *, char *,
		      LS_LONG_INT **, int);
int getSpecJobIds (int, char **, LS_LONG_INT **, int *);
int getSpecIdxs (char *, int **);
int getOneJobId (char *, LS_LONG_INT *, int);
int gettimefor (char *toptarg, time_t * tTime);
int skipJob (int, int *, int);

void prtWord (int, const char *, int);
void prtWordL (int, const char *);
char *prtValue (int, int);
char *prtDash (int);


int searchEventFile (struct histReq *, int *);
int bmsg (int, char **);

void bmove (int, char **, int);

