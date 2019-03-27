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
#define MIN(x,y)        ((x) < (y) ? (x) : (y))  // FIXME FIXME replace with math.h functions
#endif

#ifndef MAX
#define MAX(x,y)        ((x) > (y) ? (x) : (y))  // FIXME FIXME replace with math.h functions
#endif

#define DEF_COMMITTED_RUN_TIME_FACTOR 0.0  // FIXME FIXME FIXME why is this here?

// extern struct config_param lsbParams[];  // FIXME FIXME FIXME FIXME FIXME put in specific header
// int initenv_ (struct config_param *, char *);
// int sig_encode (int);


#define DEFAULT_API_CONNTIMEOUT 10
#define DEFAULT_API_RECVTIMEOUT 0


typedef struct lsbSubSpoolFile
{
	char *inFileSpool;
	char *commandSpool;

} LSB_SUB_SPOOL_FILE_T;


int creat_p_socket (void);
int serv_connect (char *, unsigned short, int);
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
unsigned short get_mbd_port (void);
unsigned short get_sbd_port (void);
// int getAuth (struct lsfAuth *);

int chUserRemoveSpoolFile (const char *hostName, const char *spoolFile);
void prtBETime_ (struct submit *);
int runBatchEsub (struct lenData *, struct submit *);
