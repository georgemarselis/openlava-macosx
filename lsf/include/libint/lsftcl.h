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
 *w
 */

#pragma once

// FIXME FIXME FIXME FIXME tcl/tcl.h must be moved to a variable in configure.ac
#include <tcl/tcl.h>
#include <sys/types.h>

struct tclHostData *hPtr;
struct Tcl_Interp *globinterp;
char overRideFromType;
char runTimeDataQueried;
unsigned int numIndx;
unsigned int nRes;
struct tclLsInfo *myTclLsInfo;

/* Arrays holding symbols used in resource requirement
 * expressions.
 */
int *ar;
int *ar2;
int *ar4;

typedef struct {
	char *name;
	int clientData;
	char padding[4];
} attribFunc;


// #define   CPUFACTOR_ (nRes + numIndx)
// #define   NDISK      (nRes + numIndx + 1)
// #define   REXPRI     (nRes + numIndx + 2)
// #define   MAXCPUS_   (nRes + numIndx + 3)
// #define   MAXMEM     (nRes + numIndx + 4)
// #define   MAXSWAP    (nRes + numIndx + 5)
// #define   MAXTMP     (nRes + numIndx + 6)
// #define   CPU_       (nRes + numIndx + 7)
// #define   SERVER     (nRes + numIndx + 8)

enum {
	CPUFACTOR, // = nRes + numIndx, // FIXME FIXME FIXME FIXME just how many "CPUFACTOR/CPU/SERVER/NDISK" declerations are there?
	NDISK,
	REXPRI,
	MAXCPUS,
	MAXMEM,
	MAXSWAP,
	MAXTMP,
	CPU,
	SERVER
};

// #define   HOSTTYPE   1
// #define   HOSTMODEL  2
// #define   HOSTSTATUS 3
// #define   HOSTNAME   4
// #define   LAST_STRING (HOSTNAME + 1)
// #define   DEFINEDFUNCTION 5

enum HOST {
	HOSTTYPE, 		// FIXME FIXME FIXME FIXME just how many "HOSTTYPE/HOSTNAME" declerations are there?
	HOSTMODEL,
	HOSTSTATUS,
	HOSTNAME,
	LAST_STRING = HOSTNAME + 1,
	DEFINEDFUNCTION
};

// #define   TCL_CHECK_SYNTAX      0
// #define   TCL_CHECK_EXPRESSION  1

enum TCL_CHECK {
	TCL_CHECK_SYNTAX,
	TCL_CHECK_EXPRESSION
};

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

int   initTcl        ( struct tclLsInfo * );
void  freeTclLsInfo  ( struct tclLsInfo *tclLsInfo, int mode );
int   evalResReq     ( char  *resReq, struct tclHostData *hPtr2, char useFromType);
int   numericValue   ( void  *clientData, Tcl_Interp *interp, Tcl_Value *args, Tcl_Value *resultPtr ); // FIXME void *clientData will probably cause issues in different architectures
int   booleanValue   ( void  *clientData, Tcl_Interp *interp, Tcl_Value *args, Tcl_Value *resultPtr );
int   stringValue    ( void  *clientData, Tcl_Interp *interp, int argc, const char *argv[] );
int   definedCmd     ( void  *clientData, Tcl_Interp *interp, int argc, const char *argv[] );
int   copyTclLsInfo  ( struct tclLsInfo *tclLsInfo );
char *getResValue    ( unsigned int resNo );
