/* $Id: lib.dir.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/structs/genParams.h"
#include "lsf.h"

// typedef enum status genparams_t;
// #define AUTOMOUNT_LAST_STR  "AMFIRST"      // copied over from lib/lproto.h
const char AUTOMOUNT_LAST_STR[]  = "AMFIRST"; // copied over from lib/lproto.h
// #define AUTOMOUNT_NEVER_STR "AMNEVER"      // copied over from lib/lproto.h
const char AUTOMOUNT_NEVER_STR[] = "AMNEVER"; // copied over from lib/lproto.h

#define AM_LAST  (!(genParams_[LSF_AM_OPTIONS].paramValue && strstr(genParams_[LSF_AM_OPTIONS].paramValue, AUTOMOUNT_LAST_STR)))
#define AM_NEVER (genParams_[LSF_AM_OPTIONS].paramValue && strstr(genParams_[LSF_AM_OPTIONS].paramValue, AUTOMOUNT_NEVER_STR))

// int AM_LAST  = !( genParams_[ LSF_AM_OPTIONS ].paramValue && strstr( genParams_[ LSF_AM_OPTIONS ].paramValue, "AMFIRST" ) ); // FIXME FIXME FIXME FIXME init early in the init process
// int AM_NEVER =    genParams_[ LSF_AM_OPTIONS ].paramValue && strstr( genParams_[ LSF_AM_OPTIONS ].paramValue, "AMNEVER" );   // FIXME FIXME FIXME FIXME init early in the init process

// #define LOOP_ADDR       0x7F000001
// unsigned long LOOP_ADDR = 0x7F000001; enum LOOP_ADDR in include/daemons/libresd/init.h
// static 
struct hTab hashTab;

char chosenPath[MAX_PATH_LEN];

/* dir.c */
char *usePath ( const char *path );
int   mychdir_( const char *path, struct hostent *hp );
int   tryPwd  ( const char *path, const char *pwdpath );
int   putin_  ( int instatus, char *inkey, int inkeylen, char *inval, int invallen, char *indata );
int   getMap_ ( void );
int   netHostChdir( const char *path, struct hostent *hp );
char *mountNet_( struct hostent *hp );
int   myopen_  ( const char *filename, int flags, mode_t mode, struct hostent *hp );
FILE *myfopen_ ( const char *filename, const char *type, struct hostent *hp );
int   mystat_  ( const char *filename, struct stat *sbuf, struct hostent *hp );
int   mychmod_ ( const char *filename, mode_t mode, struct hostent *hp );
void  myexecv_ ( const char *filename, char **argv, struct hostent *hp );
int   myunlink_( const char *filename, struct hostent *hp, int doMount );
int   mymkdir_ ( const char *filename, mode_t mode, struct hostent *hp );
int   myrename_( const char *from,     const char *to, struct hostent *hp );
