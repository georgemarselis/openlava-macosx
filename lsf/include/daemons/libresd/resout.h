/*
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

#include <sys/ioctl.h>
#include <termios.h>

#include "lib/hdr.h"
#include "daemons/libresd/resd.h"

#ifndef TIOCGWINSZ
struct winsize
{
  unsigned short ws_row;
  unsigned short ws_col;
  unsigned short ws_xpixel;
  unsigned short ws_ypixel;
};
#endif

/*enum resAck
{
  RESE_OK,
  RESE_SIGCHLD,
  RESE_NOMORECONN,
  RESE_BADUSER,
  RESE_ROOTSECURE,
  RESE_DENIED,
  RESE_REQUEST,
  RESE_CALLBACK,
  RESE_NOMEM,
  RESE_FATAL,
  RESE_PTYMASTER,
  RESE_PTYSLAVE,
  RESE_SOCKETPAIR,
  RESE_FORK,
  RESE_REUID,
  RESE_CWD,
  RESE_INVCHILD,
  RESE_KILLFAIL,
  RESE_VERSION,
  RESE_DIRW,
  RESE_NOLSF_HOST,
  RESE_NOCLIENT,
  RESE_RUSAGEFAIL,
  RESE_RES_PARENT,
  RESE_FILE,
  RESE_NOVCL,
  RESE_NOSYM,
  RESE_VCL_INIT,
  RESE_VCL_SPAWN,
  RESE_EXEC,
  RESE_ERROR_LAST,
  RESE_MLS_INVALID,
  RESE_MLS_CLEARANCE,
  RESE_MLS_DOMINATE,
  RESE_MLS_RHOST
};
*/
enum resCmd
{
    RES_FAIL          = 0,
    RES_CONNECT       = 1,
    RES_SERVER        = 2,
    RES_EXEC          = 3,
    RES_SETENV        = 4,
    RES_INITTTY       = 5,
    RES_RKILL         = 6,
    RES_CONTROL       = 7,
    RES_CHDIR         = 8,
    RES_GETPID        = 9,
    RES_RUSAGE        = 10,
    RES_NONRES        = 11,
    RES_DEBUGREQ      = 12,
    RES_ACCT          = 13,
    RES_SETENV_ASYNC  = 14,
    RES_INITTTY_ASYNC = 15,
    RES_RMI           = 99
};


struct resConnect
{
    struct lenData eexec;
};


// typedef struct relaybuf
// {
//     char buf[BUFSIZ + sizeof (struct LSFHeader)];
//     char *bp;
//     int bcount;
// };

// typedef struct relaybuf RelayBuf;

#define LINE_BUFSIZ 4096

// struct relaylinebuf
// {
//   char buf[LINE_BUFSIZ + sizeof (struct LSFHeader) + 1];
//   char *bp;
//   int bcount;
// };

// typedef struct relaylinebuf RelayLineBuf;

// typedef struct channel
// {
//   int fd;
//   RelayBuf *rbuf;
//   int rcount;
//   RelayBuf *wbuf;
//   int wcount;
// } Channel;

/* struct niosConnect {
    int rpid;
    int exitStatus;
    int terWhiPendStatus;
};

struct niosStatus {
    resAck ack;

    struct sigStatusUsage {
  int ss;
  struct rusage ru;
    } s;
};

struct resSignal {
    int pid;
    int sigval;
};

*/

#ifdef LS_WAIT_INT
#define SETTERMSIG(x,y) ((x) = ((x) & 0xFFFFFF80) | ( (y) & 0x7F))
#define SETSTOPSIG(x,y) ((x) = ((x) & 0xFFFF00FF) | (((y) & 0xFF)<<8))
#define LS_WTERMSIG(x)     ((*(int *)&x)&0x7F)
#define LS_WIFEXITED(x)    (((*(int *)&x)&0xFF)==0)
#define LS_WEXITSTATUS(x)  (((*(int *)&x)>>8)&0xFF)
#define LS_WSTOPSIG(x)     (((*(int *)&x)>>8)&0xFF)
#define LS_WIFSTOPPED(x)   ((int)((*(int *)&x)&0xFF) == 0177 && \
    				(int)((*(int *)&x)&0xFF00) != 0)
#define LS_WIFSIGNALED(x)       ((int)((*(int *)&x)&0xFF) > 0 && \
                                        (int)((*(int *)&x)&0xFF00) == 0)
#else

#define SETTERMSIG(x,y) (x).w_termsig = (y)
#define SETSTOPSIG(x,y) (x).w_stopsig = (y)
#define LS_WTERMSIG WTERMSIG
#define LS_WIFEXITED WIFEXITED
#define LS_WEXITSTATUS WEXITSTATUS
#define LS_WSTOPSIG WSTOPSIG
#define LS_WIFSTOPPED WIFSTOPPED
#define LS_WIFSIGNALED WIFSIGNALED
#endif