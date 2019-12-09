/* $Id: lsb.misc.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/table.h"
#include "lib/header.h"

struct nameList
{
	unsigned long listSize;
	unsigned long *counter;
	char **names;
};


/* lib/liblsbatch/misc.c */
struct hEnt *addMemb(struct hTab *tabPtr, size_t member);
bool remvMemb(struct hTab *tabPtr, size_t member);
struct hEnt *chekMemb(struct hTab *tabPtr, size_t member);
struct hEnt *addMembStr(struct hTab *tabPtr, char *memberStr);
bool remvMembStr(struct hTab *tabPtr, char *memberStr);
struct sortIntList *initSortIntList(int increased);
bool insertSortIntList(struct sortIntList *header, int value);
struct sortIntList *getNextSortIntList(struct sortIntList *header, struct sortIntList *current, int *value);
void freeSortIntList(struct sortIntList *header);
bool getMinSortIntList(struct sortIntList *header, int *minValue);
bool getMaxSortIntList(struct sortIntList *header, int *maxValue);
size_t getTotalSortIntList(struct sortIntList *header);
int sndJobFile_(int s, struct lenData *jf);
void upperStr(char *in, char *out);
void copyJUsage(struct jRusage *to, struct jRusage *from);
void convertRLimit(int *pRLimits, int toKb);
int limitIsOk_(int *rLimits);
char *lsb_splitName(char *str, unsigned int *number);
struct nameList *lsb_compressStrList(char **strList, unsigned int numStr);
char *lsb_printNameList(struct nameList *nameList, int format);
struct nameList *lsb_parseLongStr(char *string);
struct nameList *lsb_parseShortStr(char *string, int format);
char *getUnixSpoolDir(char *spoolDir);
char *getNTSpoolDir(char *spoolDir);
int lsb_array_idx(int jobId);
int lsb_arrayj_jobid(int jobId);
void jobId64To32(size_t interJobId, unsigned int *jobId, unsigned int *jobArrElemId);
void jobId32To64(size_t *interJobId, unsigned int jobId, unsigned int jobArrElemId);
int supportJobNamePattern(char *jobname);
