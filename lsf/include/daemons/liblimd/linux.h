/*
 * Copyright (C) 2011 David Bigagli
 *
 * $Id: lim.linux.h 397 2007-11-26 19:04:00Z mblack $
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


#include <dirent.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sysmacros.h>

#include "daemons/liblimd/common.h"

#define CPUSTATES 4
#define ut_name   ut_user
#define nonuser(ut) ((ut).ut_type != USER_PROCESS)
#define LINUX_LDAV_FILE "/proc/loadavg"


///////////////////////////////////////////////////
//
// Function prototypes
// 
int getPage (double *page_in, double *page_out, bool_t isPaging);
int readMeminfo (void);
int numCpus (void);
int queueLengthEx (float *r15s, float *r1m, float *r15m);
float queueLength ( void );
void cpuTime (double *itime, double *etime);
int realMem (float extrafactor);
float tmpspace (void);
float getswap (void);
float getpaging (float etime);
float getIoRate (float etime);
int readMeminfo (void);
void initReadLoad (int checkMode, int *kernelPerm);
const char *getHostModel (void);
int getPage (double *page_in, double *page_out, bool_t isPaging)



///////////////////////////////////////////////////
//
// Globals
// 
char *buffer; // [MSGSIZE];
long long uint main_mem   = 0;
long long uint free_mem   = 0;
long long uint shared_mem = 0;
long long uint buf_mem    = 0;
long long uint cashed_mem = 0;
long long uint swap_mem   = 0;
long long uint free_swap  = 0;
u_long prevRQ             = 0; 


double prev_time = 0;
double prev_idle = 0;
double prev_cpu_user = 0.0; // FIXME FIXME FIXME FIXME prev_cpu_user or prev_cpu_user_time ?
double prev_cpu_nice = 0.0;
double prev_cpu_sys  = 0.0;
double prev_cpu_idle = 0.0;