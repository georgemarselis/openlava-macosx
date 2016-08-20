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

#define CPUSTATES 4
#define LINUX_LDAV_FILE "/proc/loadavg"
// #define ut_name   ut_user
// #define nonuser(ut) ((ut).ut_type != USER_PROCESS) // moved to rload.h


///////////////////////////////////////////////////
//
// Function prototypes
// 
int getPage (double *page_in, double *page_out, bool_t isPaging);
int readMeminfo (void);
int numCpus (void);
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