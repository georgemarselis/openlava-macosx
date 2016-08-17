/* $Id: lsb.h 397 2007-11-26 19:04:00Z mblack $
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
#include <unistd.h>

#include "daemons/daemonout.h"
#include "lib/hdr.h"
#include "lib/lproto.h"
#include "lsb/lsbatch.h"
#include "lsf.h"


#ifndef MIN
#define MIN(x,y)        ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))
#endif

#define DEF_COMMITTED_RUN_TIME_FACTOR 0.0  // FIXME FIXME FIXME why is this here?

extern struct config_param lsbParams[];  // FIXME FIXME FIXME FIXME FIXME put in specific header
int initenv_ (struct config_param *, char *);
int sig_encode (int);


#define DEFAULT_API_CONNTIMEOUT 10
#define DEFAULT_API_RECVTIMEOUT 0

// preserved for historical reasons, will be slowly pushed out
#define LSB_DEBUG         0
// #define LSB_SHAREDIR      1
#define LSB_SBD_PORT      2
#define LSB_MBD_PORT      3
#define LSB_DEBUG_CMD     4
#define LSB_TIME_CMD      5
#define LSB_CMD_LOGDIR    6
#define LSB_CMD_LOG_MASK  7
#define LSB_API_CONNTIMEOUT 9
#define LSB_API_RECVTIMEOUT 10
#define LSB_SERVERDIR 11
#define LSB_MODE 12
#define LSB_SHORT_HOSTLIST 13
#define LSB_INTERACTIVE_STDERR 14
#define LSB_32_PAREN_ESC     15

#define LSB_API_QUOTE_CMD     14


typedef struct lsbSubSpoolFile
{
  char inFileSpool[MAXFILENAMELEN];
  char commandSpool[MAXFILENAMELEN];
} LSB_SUB_SPOOL_FILE_T;


int creat_p_socket (void);
int serv_connect (char *, ushort, int);
int getServerMsg (int, struct LSFHeader *, char **rep_buf);
int callmbd (char *, char *, int, char **, struct LSFHeader *, int *, int (*)(), int *);
int cmdCallSBD_ (char *, char *, int, char **, struct LSFHeader *, int *);


int PutQStr (FILE *, char *);
int Q2Str (char *, char *);
int authTicketTokens_ (struct lsfAuth *, char *);

char *getNextValue0 (char **line, char, char);
int readNextPacket (char **, int, struct LSFHeader *, int);
void closeSession (int);
void upperStr (char *, char *);
char *getUnixSpoolDir (char *);
char *getNTSpoolDir (char *);
char *getMasterName (void);
ushort get_mbd_port (void);
ushort get_sbd_port (void);
int getAuth (struct lsfAuth *);

int chUserRemoveSpoolFile (const char *hostName, const char *spoolFile);
void prtBETime_ (struct submit *);
int runBatchEsub (struct lenData *, struct submit *);
