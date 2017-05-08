/* $Id: lib.pim.h 397 2007-11-26 19:04:00Z mblack $
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
 
#include <limits.h>
#include <time.h>

enum lsPStatType
{
	LS_PSTAT_RUNNING,
	LS_PSTAT_INTERRUPTIBLE,
	LS_PSTAT_UNINTERRUPTIBLE,
	LS_PSTAT_ZOMBI,
	LS_PSTAT_STOPPED,
	LS_PSTAT_SWAPPED,
	LS_PSTAT_SLEEP,
	LS_PSTAT_EXITING
};

struct lsPidInfo
{
int pid;
	int ppid;
	int pgid;
	int jobid;
	int utime;
	int stime;
	int cutime;
	int cstime;
	int proc_size;
	int resident_size;
	int stack_size;
	enum lsPStatType status;
	char command[PATH_MAX]; // PATH_MAX is declared in limits.h
};

const unsigned int NL_SETN = 32;     // FIXME FIXME remove at earliest convience

int npidList                = 0;
struct pidInfo *pidList     = NULL;
struct lsPidInfo *pinfoList = NULL;
int npinfoList              = 0;
int npgidList               = 0;
int *pgidList               = NULL;
unsigned int hitPGid        = 0;
char *pimInfoBuf            = NULL;
unsigned long pimInfoLen    = 0;
int argOptions              = 0;

char *getNextString (char *, char *);
char *readPIMBuf (char *);
FILE *openPIMFile (char *pfile);
int inAddPList (struct lsPidInfo *pinfo);
int intoPidList (struct lsPidInfo *pinfo);
int pimPort (struct sockaddr_in *, char *);
int readPIMFile (char *);
struct jRusage *getJInfo_ (int npgid, int *pgid, unsigned short options, gid_t cpgid);
struct jRusage *readPIMInfo (int, int *);

