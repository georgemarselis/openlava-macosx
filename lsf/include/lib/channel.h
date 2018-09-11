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

#define CLOSECD(c) { chanClose_((c)); (c) = -1; }
#define CHAN_INIT_BUF(b)  memset((b), 0, sizeof(struct Buffer));

unsigned int chanerr = 0;

enum chanState {
	CH_FREE,
	CH_DISC,
	CH_PRECONN,
	CH_CONN,
	CH_WAIT,
	CH_INACTIVE
};

enum chanType { 
	CH_TYPE_UDP,
	CH_TYPE_TCP,
	CH_TYPE_LOCAL,
	CH_TYPE_PASSIVE,
	CH_TYPE_NAMEDPIPE
};


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
	unsigned int handle;
	unsigned int chanerr;
	char padding[4];
	struct Buffer *send;
	struct Buffer *recv;
};

struct chanData *channels = NULL;
unsigned long chanIndex = 0;


// #define CHAN_OP_PPORT    0x01
// #define CHAN_OP_CONNECT  0x02
// #define CHAN_OP_RAW      0x04
// #define CHAN_OP_NONBLOCK 0x10
// #define CHAN_OP_CLOEXEC  0x20
// #define CHAN_OP_SOREUSE  0x40
enum CHAN_OP {
	CHAN_OP_PPORT    = 0x01,
	CHAN_OP_CONNECT  = 0x02,
	CHAN_OP_RAW      = 0x04,
	CHAN_OP_NONBLOCK = 0x10,
	CHAN_OP_CLOEXEC  = 0x20,
	CHAN_OP_SOREUSE  = 0x40,
};


// #define CHAN_MODE_BLOCK   0x01
// #define CHAN_MODE_NONBLOCK  0x02
enum CHAN_MODE {
	CHAN_MODE_BLOCK     = 0x01,
	CHAN_MODE_NONBLOCK  = 0x02
};

// #define  CHANE_NOERR      0
// #define  CHANE_CONNECTED  1
// #define  CHANE_NOTCONN    2
// #define  CHANE_SYSCALL    3
// #define  CHANE_INTERNAL   4
// #define  CHANE_NOCHAN     5
// #define  CHANE_MALLOC     6
// #define  CHANE_BADHDR     7
// #define  CHANE_BADCHAN    8
// #define  CHANE_BADCHFD    9
// #define  CHANE_NOMSG      10
// #define  CHANE_CONNRESET  11

enum CHANE {
	CHANE_NOERR = 0,
	CHANE_CONNECTED = 1,
	CHANE_NOTCONN = 2,
	CHANE_SYSCALL = 3,
	CHANE_INTERNAL = 4,
	CHANE_NOCHAN = 5,
	CHANE_MALLOC = 6,
	CHANE_BADHDR = 7,
	CHANE_BADCHAN = 8,
	CHANE_BADCHFD = 9,
	CHANE_NOMSG = 10,
	CHANE_CONNRESET = 11,
	INVALID_HANDLE = 12  // FIXME FIXME this is definatelly wrong and it will break combatibility with LSF
};


static size_t chanMaxSize = 0;

// #define chanSend_  chanEnqueue_
// #define chanRecv_  chanDequeue_

int            chanInit_        ( void );
int            chanServSocket_  ( int type, unsigned short port, int backlog, int options);
int            chanClientSocket_( int domain, int type, int options);
int            chanAccept_      ( int chfd, struct sockaddr_in *from);
void           chanInactivate_  ( unsigned int chfd);
void           chanActivate_    ( unsigned int chfd);
int            chanConnect_     ( int chfd, struct sockaddr_in *peer, int timeout);
int            chanSendDgram_   ( int chfd, char *buf, int len, struct sockaddr_in *peer);
int            chanRcvDgram_    ( int chfd, char *buf, int len, struct sockaddr_in *peer, int timeout);
int            chanOpen_        ( unsigned int iaddr, unsigned short port, int options);
int            chanOpenSock_    ( int s, int options);
int            chanClose_       ( int chfd);
void           chanCloseAll_    ( void);
void           chanCloseAllBut_ ( int chfd);
int            chanSelect_      ( struct Masks *sockmask, struct Masks *chanmask, struct timeval *timeout);
int            chanEnqueue_     ( int chfd, struct Buffer *msg);
int            chanDequeue_     ( int chfd, struct Buffer **buf);
long           chanReadNonBlock_( int chfd, char *buf, size_t len, int timeout);
long           chanRead_        ( int chfd, char *buf, size_t len);
long           chanWrite_       ( int chfd, char *buf, size_t len);
int            chanRpc_         ( int chfd, struct Buffer *in, struct Buffer *out, struct LSFHeader *outhdr, int timeout);

unsigned long  chanSock_          ( unsigned int chfd );
int            chanSetMode_       ( unsigned int chfd, int mode );
void           doread             ( int chfd, struct Masks *chanmask );
void           dowrite            ( int chfd, struct Masks *chanmask );
struct Buffer *newBuf             ( void );
int            chanAllocBuf_      ( struct Buffer **buf, int size);
int            chanFreeBuf_       ( struct Buffer  *buf   );
int            chanFreeStashedBuf_( struct Buffer  *buf   );
void           dequeue_           ( struct Buffer  *entry );
void           enqueueTail_       ( struct Buffer  *entry, struct Buffer *pred );
int            findAFreeChannel   ( void );
