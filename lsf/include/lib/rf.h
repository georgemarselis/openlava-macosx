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
	mode_t mode;
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


// #define RF_MAXHOSTS 5
// #define RF_CMD_MAXHOSTS 0
// #define RF_CMD_RXFLAGS 2

// unsigned int RF_MAXHOSTS     = 5;
// unsigned int RF_CMD_MAXHOSTS = 0;
// unsigned int RF_CMD_RXFLAGS  = 2;

enum RF {
	RF_CMD_MAXHOSTS = 0,
	RF_CMD_RXFLAGS  = 2,
	RF_MAXHOSTS     = 5
};

int rxFlags = 0;
unsigned int maxOpen = NOFILE;
unsigned int nrh = 0;
unsigned int maxnrh = RF_MAXHOSTS;
// #define RF_SERVERD "_rf_Server_"
const char RF_SERVERD[] = "_rf_Server_";

/* lib/liblsf/rf.c */
struct rHosts *rhConnect(const char *host);
struct rHosts *allocRH(void);
struct rHosts *rhFind(const char *host);
unsigned int ls_ropen(const char *host, const char *fn, int flags, mode_t mode);
int ls_rclose(int fd);
unsigned int ls_rwrite(int fd, const char *buf, size_t len);
unsigned int ls_rread (int fd, const char *buf, size_t len);
off_t ls_rlseek(int fd, off_t offset, int whence);
int ls_rfstat(int fd, struct stat *st);
int ls_rfcontrol(int command, int arg);
int ls_rfterminate(const char *host);
int rhTerminate( const char *host);
int ls_rstat(const char *host, const char *fn, struct stat *st);
char *ls_rgetmnthost(const char *host, const char *fn);
int ls_conntaskport(pid_t rpid);
int ls_runlink(const char *host, const char *fn);

