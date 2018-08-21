/* $Id: lib.conf.h 397 2007-11-26 19:04:00Z mblack $
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

#include "libint/bitset.h"

#define BADCH   ":"
// #define NL_SETN   23

#define MAXFLOAT_LOCAL  3.40282347e+38F

/*
#define fprintf(errMsg, fmt, msg1, msg2)   \
	{                                           \
		if (errMsg == NULL) {                   \
			fprintf(stderr, fmt, msg1, msg2);   \
		} \
		else {                                    \
			sprintf(*errMsg, fmt, msg1, msg2);   \
		} \
	}*/


static struct LSFAdmins
{
	unsigned int numAdmins;
	const char padding[4];
	char **names;

} LSFAdmins;

bool_t isLSFAdmin   ( const char *);
char *putstr_       ( const char *s );
int replace1stCmd_  ( const char *oldCmdArgs, const char *newCmdArgs, char *outCmdArgs, size_t outLen);
unsigned int isint_ ( const char *word);
int ls_strcat       ( char *trustedBuffer, int bufferLength, const char *strToAdd);
