/* $Id: lib.rf.h 397 2007-11-26 19:04:00Z mblack $
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


#define ABS(i) ((i) < 0 ? -(i) : (i))

struct rHosts
{
	int sock;
	int nopen;
	time_t atime;
	char *hname; 
	struct rHosts *next;
}; //*rHosts = NULL;

struct rfTab
{
	int fd;
	char padding[4];
	struct rHosts *host;
};// *ft = NULL;

// typedef
enum rfCmd
{
	RF_OPEN,
	RF_READ,
	RF_WRITE,
	RF_CLOSE,
	RF_STAT,
	RF_GETMNTHOST,
	RF_LSEEK,
	RF_FSTAT,
	RF_UNLINK,
	RF_TERMINATE
};// rfCmd;

struct ropenReq
{
	char *fn;
	int flags;
	int mode;
};


struct rrdwrReq
{
	int fd;
	char padding[4];
	size_t len;
};

struct rlseekReq
{
	int fd;
	int whence;
	off_t offset;
};


int nrh = 0;
int rxFlags = 0;
int maxnrh = RF_MAXHOSTS;
// #define RF_SERVERD "_rf_Server_"
const char RF_SERVERD[] = "_rf_Server_";

struct rHosts *rhConnect (char *host);
struct rHosts *allocRH   (void);
struct rHosts *rhFind    (char *host);
int            ls_ropen  (char *host, char *fn, int flags, int mode);
int            ls_rclose (int fd);
int            ls_rwrite (int fd, char *buf, size_t len);
int            ls_rread  (int fd, char *buf, size_t len);
int          rhTerminate (char *host);

