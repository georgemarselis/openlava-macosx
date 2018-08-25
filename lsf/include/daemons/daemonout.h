/* $Id: daemonout.h 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/hdr.h"
#include "lib/xdr.h"
#include "lib/lproto.h"
#include "lsb/sig.h"
#include "lsb/lsbatch.h"
#include "lsf.h"
#include "lib/common_structs.h"

// #define BATCH_MASTER_PORT   40000
// #define ALL_HOSTS      "all"

static const unsigned int BATCH_MASTER_PORT = 40000;
static const char ALL_HOSTS[] = "all";

#define  PUT_LOW(word, s)  (word = (s | (word & ~0x0000ffff)))
#define  PUT_HIGH(word, s) (word = ((s << 16) | (word & 0x0000ffff)))
#define  GET_LOW(s, word)  (s = word & 0x0000ffff)
#define  GET_HIGH(s, word) (s = (word >> 16) & 0x0000ffff)


// #define PREPARE_FOR_OP          1024
// #define READY_FOR_OP            1023
static const unsigned short PREPARE_FOR_OP = 1024;
static const unsigned short READY_FOR_OP   = 1023;

// #define RSCHED_LISTSEARCH_BY_EXECJID       0
// #define RSCHED_LISTSEARCH_BY_EXECLUSNAME   1
static enum RSCHED_LISTSEARCH {
	RSCHED_LISTSEARCH_BY_EXECJID = 0,
	RSCHED_LISTSEARCH_BY_EXECLUSNAME = 1	
} RSCHED_LISTSEARCH;

// FIXME FIXME sort labels, remove numbers, make sure everything is compared using the labels and not the numbers
static enum mbdReqType {
	BATCH_JOB_SUB        = 1,
	BATCH_JOB_INFO       = 2,
	BATCH_JOB_PEEK       = 3,
	BATCH_JOB_SIG        = 4,

	BATCH_HOST_INFO      = 5,

	BATCH_QUEUE_INFO     = 6,

	BATCH_GRP_INFO       = 7,
	BATCH_QUEUE_CTRL     = 8,
	BATCH_RECONFIG       = 9,
	BATCH_HOST_CTRL      = 10,

	BATCH_JOB_SWITCH     = 11,
	BATCH_JOB_MOVE       = 12,
	BATCH_JOB_MIG        = 13,

	BATCH_STATUS_JOB     = 15,
	BATCH_SLAVE_RESTART  = 16,
	BATCH_USER_INFO      = 17,
	BATCH_PARAM_INFO     = 20,

	BATCH_JOB_MODIFY     = 22,
	BATCH_JOB_EXECED     = 25,
	BATCH_JOB_MSG        = 27,

	BATCH_STATUS_MSG_ACK = 28,

	BATCH_DEBUG          = 29,
	BATCH_RESOURCE_INFO  = 30,
	BATCH_RUSAGE_JOB     = 32,

	BATCH_JOB_FORCE      = 37,

	BATCH_UNUSED_38      = 38,
	BATCH_UNUSED_39      = 39,
	BATCH_STATUS_CHUNK   = 40,
	BATCH_SET_JOB_ATTR   = 90

} mbdReqType;

// #define SUB_RLIMIT_UNIT_IS_KB 0x80000000
static const unsigned long SUB_RLIMIT_UNIT_IS_KB = 0x80000000;


// #define SHELLLINE "#! /bin/sh\n\n"
// #define CMDSTART "# LSBATCH: User input\n"
// #define CMDEND "# LSBATCH: End user input\n"
// #define ENVSSTART "# LSBATCH: Environments\n"
// #define LSBNUMENV "#LSB_NUM_ENV="
// #define EDATASTART "# LSBATCH: edata\n"
// #define AUXAUTHSTART "# LSBATCH: aux_auth_data\n"
// #define EXITCMD "exit `expr $? \"|\" $ExitStat`\n"
// #define WAITCLEANCMD "\nExitStat=$?\nwait\n# LSBATCH: End user input\ntrue\n"
// #define TAILCMD "'; export "
// #define TRAPSIGCMD "$LSB_TRAPSIGS\n$LSB_RCP1\n$LSB_RCP2\n$LSB_RCP3\n"
// #define JOB_STARTER_KEYWORD "%USRCMD"
// #define SCRIPT_WORD "_USER_\\SCRIPT_"
// #define SCRIPT_WORD_END "_USER_SCRIPT_"
// #define LOAD_REPLY_SHARED_RESOURCE 0x1

// NOTE The output script that executes the job starts here
static const char SHELLLINE[]           = "#! /bin/sh\n\n"; // FIXME FIXME FIXME: !# evn sh, instead
static const char CMDSTART[]            = "# LSBATCH: User input\n";
static const char CMDEND[]              = "# LSBATCH: End user input\n";
static const char ENVSSTART[]           = "# LSBATCH: Environments\n";
static const char LSBNUMENV[]           = "#LSB_NUM_ENV=";
static const char EDATASTART[]          = "# LSBATCH: edata\n";
static const char AUXAUTHSTART[]        = "# LSBATCH: aux_auth_data\n";
static const char EXITCMD[]             = "exit `expr $? \"|\" $ExitStat`\n";
static const char WAITCLEANCMD[]        = "\nExitStat=$?\nwait\n# LSBATCH: End user input\ntrue\n";
static const char TAILCMD[]             = "'; export ";
static const char TRAPSIGCMD[]          = "$LSB_TRAPSIGS\n$LSB_RCP1\n$LSB_RCP2\n$LSB_RCP3\n";
static const char JOB_STARTER_KEYWORD[] = "%USRCMD";
static const char SCRIPT_WORD[]         = "_USER_\\SCRIPT_";
static const char SCRIPT_WORD_END[]     = "_USER_SCRIPT_";
static const unsigned short LOAD_REPLY_SHARED_RESOURCE = 0x1;


// duplicate: 4 define
// #define CALL_SERVER_NO_WAIT_REPLY 0x1
// #define CALL_SERVER_USE_SOCKET    0x2
// #define CALL_SERVER_NO_HANDSHAKE  0x4
// #define CALL_SERVER_ENQUEUE_ONLY  0x8
static enum CALL_SERVER {
	CALL_SERVER_NO_WAIT_REPLY = 0x1,
	CALL_SERVER_USE_SOCKET    = 0x2,
	CALL_SERVER_NO_HANDSHAKE  = 0x4,
	CALL_SERVER_ENQUEUE_ONLY  = 0x8,
} CALL_SERVER;


typedef enum
{
	MBD_NEW_JOB_KEEP_CHAN = 0,
	MBD_NEW_JOB = 1,
	MBD_SIG_JOB = 2,
	MBD_SWIT_JOB = 3,
	MBD_PROBE = 4,
	MBD_REBOOT = 5,
	MBD_SHUTDOWN = 6,
	CMD_SBD_DEBUG = 7,
	UNUSED_8 = 8,
	MBD_MODIFY_JOB = 9,

	SBD_JOB_SETUP = 100,
	SBD_SYSLOG = 101,
	SBD_DONE_MSG_JOB = 102,

	RM_JOB_MSG = 200,
	RM_CONNECT = 201,


	CMD_SBD_REBOOT = 300,
	CMD_SBD_SHUTDOWN = 301
} sbdReqType;


struct lenDataList
{
	int numJf;
	char padding1[4];
	struct lenData *jf;
};


void initTab (struct hTab *tabPtr);
struct hEnt *addMemb (struct hTab *tabPtr, long member);
char remvMemb (struct hTab *tabPtr, long member);
struct hEnt *chekMemb (struct hTab *tabPtr, long member);
struct hEnt *addMembStr (struct hTab *tabPtr, char *member);
char remvMembStr (struct hTab *tabPtr, char *member);
struct hEnt *chekMembStr (struct hTab *tabPtr, char *member);
void convertRLimit (int *pRLimits, int toKb);
int limitIsOk_ (int *rLimits);

int handShake_ (int, char, int);


int call_server (char *host, unsigned short serv_port, char *req_buf, size_t req_size,  char **rep_buf, struct LSFHeader *replyHdr, int conn_timeout, int recv_timeout, int *connectedSock, int (*postSndFunc) (), int *postSndFuncArg, int flags);

int sndJobFile_ (int, struct lenData *);

void freeUnixGrp (struct group *);
struct group *copyUnixGrp (struct group *);

void freeGroupInfoReply (struct groupInfoReply *reply);

void appendEData (struct lenData *jf, struct lenData *ed);

