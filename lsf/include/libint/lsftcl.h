/* $Id: lsftcl.h 397 2007-11-26 19:04:00Z mblack $
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

typedef struct
{
	char *name;
	int clientData;
	char padding[4];
} attribFunc;


#define   CPUFACTOR  (nRes + numIndx)
#define   NDISK      (nRes + numIndx + 1)
#define   REXPRI     (nRes + numIndx + 2)
#define   MAXCPUS_   (nRes + numIndx + 3)
#define   MAXMEM     (nRes + numIndx + 4)
#define   MAXSWAP    (nRes + numIndx + 5)
#define   MAXTMP     (nRes + numIndx + 6)
#define   CPU_       (nRes + numIndx + 7)
#define   SERVER     (nRes + numIndx + 8)

#define   HOSTTYPE   1
#define   HOSTMODEL  2
#define   HOSTSTATUS 3
#define   HOSTNAME   4
#define   LAST_STRING (HOSTNAME + 1)
#define   DEFINEDFUNCTION 5

#define   TCL_CHECK_SYNTAX      0
#define   TCL_CHECK_EXPRESSION  1

struct tclHostData
{
	int maxCpus;
	int maxMem;
	int maxSwap;
	int maxTmp;
	int nDisks;
	int rexPriority;
	int ignDedicatedResource;
	int numResPairs;
	int flag;
	int overRideFromType;
	int *status;
	unsigned int *resBitMaps;
	unsigned int *DResBitMaps;
	short hostInactivityCount;
	char padding1[6];
	char *hostName;
	char *hostType;
	char *hostModel;
	char *fromHostType;
	char *fromHostModel;
	float cpuFactor;
	char padding2[4];
	float *loadIndex;
	struct resPair *resPairs;
};

struct tclLsInfo
{
	unsigned int numIndx;
	unsigned int nRes;
	int *stringResBitMaps;
	int *numericResBitMaps;
	char **resName;
	char **indexNames;
};

int initTcl (struct tclLsInfo *);
void freeTclLsInfo (struct tclLsInfo *tclLsInfo, int mode);
int evalResReq (char *resReq, struct tclHostData *hPtr2, char useFromType);


