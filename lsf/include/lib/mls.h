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

typedef enum
{
	MLS_FATAL,
	MLS_INVALID,
	MLS_CLEARANCE,
	MLS_RHOST,
	MLS_DOMINATE
} mlsErrCode;

// static int mlsSbdMode;

/*#define lsfSetUid(uid)            lsfSetXUid(0, uid, uid, -1, setuid)
#define lsfSetEUid(uid)             lsfSetXUid(0, -1, uid, -1, seteuid)
#define lsfSetREUid(ruid, euid)     lsfSetXUid(0, ruid, euid, -1, setreuid)
#define lsfExecv(path, argv)        lsfExecX(path, argv, execv)
#define lsfExecvp(file, argv)       lsfExecX(file, argv, execvp)*/

// FIXME investigate if the third argument to lsfSetXUid can be set to the appropriate
// [s]uid_t type. if yes, try to see if there is an alternative to passing -1.
int lsfSetXUid (int, uid_t uid, gid_t gid, uid_t suid, int (*)());
void lsfExecLog (const char *);
int lsfExecX (char *path, char **argv, int (*func) ());

