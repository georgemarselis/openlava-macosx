/* $Id: nios.h 397 2007-11-26 19:04:00Z mblack $
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


typedef enum
{
  LIB_NIOS_RTASK,
  LIB_NIOS_RWAIT,
  LIB_NIOS_REM_ON,
  LIB_NIOS_REM_OFF,
  LIB_NIOS_SETSTDIN,
  LIB_NIOS_GETSTDIN,
  LIB_NIOS_EXIT,
  LIB_NIOS_SUSPEND,
  LIB_NIOS_SETSTDOUT,
  LIB_NIOS_SYNC
} libNiosRequest;

typedef enum
{
  JOB_STATUS_ERR,
  JOB_STATUS_UNKNOWN,
  JOB_STATUS_FINISH,
  JOB_STATUS_KNOWN
} JOB_STATUS;

#define WAIT_BLOCK(o) (!((o) & WNOHANG))


typedef enum
{
  CHILD_OK,
  NONB_RETRY,
  CHILD_FAIL,
  REM_ONOFF,
  STDIN_FAIL,
  STDIN_OK,
  NIOS_OK,
  STDOUT_FAIL,
  STDOUT_OK,
  SYNC_FAIL,
  SYNC_OK
} libNiosReply;


struct lslibNiosHdr
{
  int opCode;
  char padding[4];
  size_t len;
};


struct lslibNiosWaitReq
{
  struct lslibNiosHdr hdr;
  struct
  {
    int options;
    int tid;
  } r;
};

struct lslibNiosWaitReply
{
  struct lslibNiosHdr hdr;
  struct
  {
    int pid;
    int status;
    struct rusage ru;
  } r;
};

struct lslibNiosRTask
{
  struct lslibNiosHdr hdr;
  struct
  {
    int pid;
    struct in_addr peer;
  } r;
};

struct lslibNiosStdout
{
  struct lslibNiosHdr hdr;
  struct
  {
    int set_on;
    char padding[4];
    size_t len;
  } r;
  char *format;
};

struct lslibNiosStdin
{
  struct lslibNiosHdr hdr;
  struct
  {
    int set_on;
    char padding[4];
    size_t len;
  } r;
  int *rpidlist;
};

struct lslibNiosGetStdinReply
{
  struct lslibNiosHdr hdr;
  int *rpidlist;
};

struct finishStatus
{
  int got_eof;
  int got_status;
  int sendSignal;
};

int cli_nios_fd[2] = {-1, -1 };
int standalone;
int niosSbdMode;
int heartbeatInterval;
int jobStatusInterval;
int pendJobTimeout;
int msgInterval;
u_long jobId;

