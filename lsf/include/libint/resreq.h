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

#pragma once
 
#include "lsf.h"
#include "libint/intlibout.h"
#include "lib/lproto.h"
#include "lib/table.h"

#define IS_DIGIT(s)  ( (s) >= '0' && (s) <= '9')
#define IS_LETTER(s) ( ((s) >= 'a' && (s) <= 'z') || ((s) >= 'A' && (s) <= 'Z'))
#define IS_VALID_OTHER(s) ((s) == '_'|| (s) == '~')

#define WILDCARD_STR  "any"
#define LOCAL_STR     "local"

enum PR {
	PR_SELECT      = 0x01,
	PR_ORDER       = 0x02,
	PR_RUSAGE      = 0x04,
	PR_FILTER      = 0x08,
	PR_DEFFROMTYPE = 0x10,
	PR_BATCH       = 0x20,
	PR_SPAN        = 0x40,
	PR_XOR         = 0x80
};

#define PR_ALL         PR_SELECT | PR_ORDER | PR_RUSAGE | PR_SPAN

// #define PARSE_OK       0
// #define PARSE_BAD_EXP  -1
// #define PARSE_BAD_NAME -2
// #define PARSE_BAD_VAL  -3
// #define PARSE_BAD_FILTER  -4
// #define PARSE_BAD_MEM     -5
enum PARSE {
	PARSE_OK         = 0,
	PARSE_BAD_EXP    = 1,
	PARSE_BAD_NAME   = 2,
	PARSE_BAD_VAL    = 3,
	PARSE_BAD_FILTER = 4,
	PARSE_BAD_MEM    = 5
};

// #define IDLETIME 5
enum IDLETIME {
	IDLETIME = 5
};

enum KEYS {
	KEY_DURATION = 1,
	KEY_HOSTS    = 2,
	KEY_PTILE    = 3,
	KEY_DECAY    = 4,
	NUM_KEYS     = 5
};

// FIXME FIXME FIXME FIXME FIXME turn ALLOC_STRING macro into a function
#define ALLOC_STRING( buffer, buffer_len, req_len) {    \
		if (buffer == NULL || buffer_len < req_len) {   \
			FREEUP(buffer);                             \
			buffer = malloc(req_len);                   \
			buffer_len = req_len;                       \
		}                                               \
	}
// FIXME FIXME FIXME FIXME FIXME turn REALLOC_STRING macro into a function
#define REALLOC_STRING(buffer, buffer_len, req_len) {   \
		if (buffer == NULL) {                           \
			buffer = malloc(req_len);                   \
			buffer_len = req_len;                       \
		}                                               \
		else if (buffer_len < req_len) {                \
			char *tmp;                                  \
			tmp = realloc(buffer, req_len);             \
			if (tmp == NULL)                            \
				FREEUP(buffer);                         \
			buffer = tmp;                               \
			buffer_len = req_len;                       \
		}                                               \
	}

struct resVal
{
	char *selectStr;
	int nphase;
	int genClass;
	int duration;
	int pTile;
	int options;
	unsigned int selectStrSize;
	unsigned int nindex;
	unsigned int numHosts;
	unsigned int maxNumHosts;
	int order[NBUILTINDEX];
	int *indicies;
	int *rusgBitMaps;
	float *val;
	float decay;
	char padding[4];
	char **xorExprs;
};

struct sections
{
	char *select;
	char *order;
	char *rusage;
	char *filter;
	char *span;
};

enum syntaxType
{ OLD, NEW, EITHER };

// struct hTab resNameTbl = { NULL, 0, 0 };
// struct hTab keyNameTbl = { NULL, 0, 0 };

int parseSection (char *, struct sections *);
int parseSelect (char *, struct resVal *, struct lsInfo *, bool_t, int);
int parseOrder (char *, struct resVal *, struct lsInfo *);
int parseFilter (char *, struct resVal *, struct lsInfo *);
int parseUsage (char *, struct resVal *, struct lsInfo *);
int parseSpan (char *, struct resVal *);
int resToClassNew (char *, struct resVal *, struct lsInfo *);
int resToClassOld (char *, struct resVal *, struct lsInfo *);
int setDefaults (struct resVal *, struct lsInfo *, int);
int getVal (char **, float *);
enum syntaxType getSyntax (char *);
int getKeyEntry (char *);
int getTimeVal (char **, float *);
void freeResVal (struct resVal *);
void initResVal (struct resVal *);
