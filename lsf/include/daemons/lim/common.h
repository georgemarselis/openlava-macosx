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

#ifndef LSF_LIM_COMMON_H
#define LSF_LIM_COMMON_H

#include <math.h>
#include <utmp.h>

#include "lim/lim.h"


#define  SWP_INTVL_CNT   45/exchIntvl
#define  TMP_INTVL_CNT 	120/exchIntvl
#define  PAGE_INTVL_CNT 120/exchIntvl


extern int pipefd[2];
extern struct limLock limLock;


static float getIoRate (float);
static float getpaging (float);
static float getswap (void);
static float idletime (int *);
static float tmpspace (void);
static void getusr (void);
static void smooth (float *, float, float);
void sendLoad (void);

static int numCpus (void);

int maxnLbHost = 0;
int ncpus = 1;
float cpu_usage = 0.0;

#endif
