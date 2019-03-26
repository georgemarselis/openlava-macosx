/* $Id: yparse.h 397 2007-11-26 19:04:00Z mblack $
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

#include "lsf.h"

#ifndef MIN
#define MIN(x, y)  ((x) <= (y)) ? (x):(y)
#endif
#ifndef MAX
#define MAX(x, y)  ((x) >= (y)) ? (x):(y)
#endif

#ifndef TRUE
#define TRUE     1
#endif
#ifndef FALSE
#define FALSE    0
#endif


#define INFINIT_INT    0x7fffffff
#define MAXTOKENLEN 302

struct intRegion
{
  int start;
  int end;
};

struct listLink
{
	struct intRegion iconList;
	struct listLink *next;
};

struct mallocList
{
	void *space;
	struct mallocList *next;
};

char *token;
char yyerr[MAX_LINE_LEN];
// char *yyerr;
#if !defined(__CYGWIN__) && !defined(__sun__)
FILE *yyout;
#endif
// struct mbd_func_type mbd_func; // FIXME FIXME FIXME FIXME FIXME single reference of this struct in the code!
struct mallocList *idxAllocHead;

int yylex (void);
void calerror (register char *);
char *safe_calloc (unsigned, unsigned);
void timerror (register char *);
void *yyalloc (struct mallocList **, int);
void yyfree (struct mallocList **, void *);
void yparseSucc (struct mallocList **);
void yparseFail (struct mallocList **);
struct calendarE *getCalExprNode ();
void idxerror (register char *);


int checkNameSpec (char *, char **);

