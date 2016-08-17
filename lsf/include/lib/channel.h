/* $Id: lib.channel.h 397 2007-11-26 19:04:00Z mblack $
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

#include <sys/types.h>

#include "lib/hdr.h"


enum chanState
{ CH_FREE,
  CH_DISC,
  CH_PRECONN,
  CH_CONN,
  CH_WAIT,
  CH_INACTIVE
};

enum chanType
{ CH_TYPE_UDP, CH_TYPE_TCP, CH_TYPE_LOCAL, CH_TYPE_PASSIVE,
  CH_TYPE_NAMEDPIPE
};

#define CHAN_OP_PPORT  		0x01
#define CHAN_OP_CONNECT		0x02
#define CHAN_OP_RAW		0x04
#define CHAN_OP_NONBLOCK        0x10
#define CHAN_OP_CLOEXEC         0x20
#define CHAN_OP_SOREUSE         0x40


#define CHAN_MODE_BLOCK 	0x01
#define CHAN_MODE_NONBLOCK 	0x02

#define CLOSECD(c) { chanClose_((c)); (c) = -1; }

#define CHAN_INIT_BUF(b)  memset((b), 0, sizeof(struct Buffer));

struct Buffer
{
  long pos;
  size_t len;
  int stashed;
  char padding[4];
  char *data;
  struct Buffer *forw;
  struct Buffer *back;
};

struct Masks
{
  fd_set rmask;
  fd_set wmask;
  fd_set emask;
};


struct chanData
{
  enum chanType type;
  enum chanState state;
  enum chanState prestate;
  uint handle;
  uint chanerr;
  char padding[4];
  struct Buffer *send;
  struct Buffer *recv;
};

#define  CHANE_NOERR      0
#define  CHANE_CONNECTED  1
#define  CHANE_NOTCONN    2
#define  CHANE_SYSCALL    3
#define  CHANE_INTERNAL   4
#define  CHANE_NOCHAN     5
#define  CHANE_MALLOC     6
#define  CHANE_BADHDR     7
#define  CHANE_BADCHAN    8
#define  CHANE_BADCHFD    9
#define  CHANE_NOMSG      10
#define  CHANE_CONNRESET  11

int chanInit_ (void);


#define chanSend_  chanEnqueue_
#define chanRecv_  chanDequeue_

int chanOpen_ (u_int, u_short, int);
int chanEnqueue_ (int chfd, struct Buffer *buf);
int chanDequeue_ (int chfd, struct Buffer **buf);

int chanSelect_ (struct Masks *, struct Masks *, struct timeval *timeout);
int chanClose_ (int chfd);
void chanCloseAll_ (void);
uint chanSock_ (uint chfd);

int chanServSocket_ (int, u_short, int, int);
int chanAccept_ (int, struct sockaddr_in *);

int chanClientSocket_ (int, int, int);
int chanConnect_ (int chfd, struct sockaddr_in *peer, int timeout);

int chanSendDgram_ (int, char *, int, struct sockaddr_in *);
int chanRcvDgram_ (int, char *, int, struct sockaddr_in *, int);
int chanRpc_ (int, struct Buffer *, struct Buffer *, struct LSFHeader *, int timeout);
long chanReadNonBlock_ (int chfd, char *buf, size_t len, int timeout);
long chanRead_ (int chfd, char *buf, size_t len);
long chanWrite_ (int, char *, size_t);


void chanInactivate_ (uint chfd);
void chanActivate_ (uint chfd);
void chanCloseAllBut_ (int chfd);


int chanAllocBuf_ (struct Buffer **buf, int size);
int chanFreeBuf_ (struct Buffer *buf);
int chanFreeStashedBuf_ (struct Buffer *buf);
int chanSetMode_ (uint chfd, int mode );
int chanSetMode_ (uint chfd, int mode );

unsigned int cherrno;
unsigned int chanIndex;

