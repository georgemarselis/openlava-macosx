
/* $Id: mls.h 397 2007-11-26 19:04:00Z mblack $
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

// #define NL_SETN 23

unsigned int totsockets_     = 0;
unsigned int currentsocket_  = 0;
int mlsSbdMode               = 0;
static char rootuid_         = '\0';

// FIXME investigate if the third argument to lsfSetXUid can be set to the appropriate
// [s]uid_t type. if yes, try to see if there is an alternative to passing -1
int lsfSetXUid ( int flag, uid_t ruid, uid_t euid, uid_t suid, int (*func) () );
int lsfExecX   ( const char *path, char **argv, int (*func) () );

int ls_fdbusy   ( int fd );
unsigned int opensocks_  ( unsigned int num );
unsigned int ls_initrex  ( unsigned int num, int options );
void lsfExecLog ( const char *cmd );
