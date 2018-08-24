/* $Id: lsb.rexecv.c 397 2007-11-26 19:04:00Z mblack $
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


#include "daemons/daemonout.h"

// #define NL_SETN 	13

char **environ;
char *loginShell;
int optionFlag;
char optionFileName[MAX_LSF_NAME_LEN];
int sig_decode (int);
int isatty (int);


int setOption_ (int argc, char **argv, char *template, struct submit *req, int mask, char **errMsg);
struct submit *parseOptFile_ (char *filename, struct submit *req, char **errMsg);
void subUsage_ (int, char **);

static int emptyCmd;


static char *commandline;

#define SKIPSPACE(sp)      while (isspace(*(sp))) (sp)++;

#define EMBED_INTERACT     0x01
#define EMBED_OPTION_ONLY  0x02
#define EMBED_BSUB         0x04
#define EMBED_RESTART      0x10
#define EMBED_QSUB         0x20

int fillReq2 (int argc, char **argv, int operate, struct submit *req);
void sub_perror (char *);
void prtBETime2 (struct submit req);
void prtErrMsg2 (struct submit *req, struct submitReply *reply);

