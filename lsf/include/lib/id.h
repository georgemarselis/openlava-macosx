/* $Id: lib.id.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
#include "lib/so.h"

#define NL_SETN 23
#define IDLIB_SO_NAME "liblsfid.so"

#define GET_LSF_USER         "getLSFUser_"
#define GET_LSF_USER_BY_NAME "getLSFUserByName_"
#define GET_LSF_USER_BY_UID  "getLSFUserByUid_"
#define GET_OS_USER_NAME     "getOSUserName_"
#define GET_OS_UID           getOSUid_

static int defGetLSFUser (char *lsfUserName, unsigned int lsfUserNameSize);
static int defGetLSFUserByName (const char *osUserName, char *lsfUserName, unsigned int lsfUserNameSize);
static int defGetLSFUserByUid (uid_t uid, char *lsfUserName, unsigned int lsfUserNameSize);
static int defGetOSUserName (const char *lsfUserName, char *osUserName, unsigned int osUserNameSize);
static int defGetOSUid (const char *lsfUserName, uid_t * uid);

typedef int (*GET_LSF_USER_BY_NAME_FN_T) (const char *osUserName, char *lsfUserName, unsigned int lsfUserNameSize);
typedef int (*GET_LSF_USER_BY_UID_FN_T) (uid_t uid, char *lsfUserName, unsigned int lsfUserNameSize);
typedef int (*GET_OS_USER_NAME_FN_T) (const char *lsfUserName, char *osUserName, unsigned int osUserNameSize);
typedef int (*GET_OS_UID_FN_T) (const char *lsfUserName, uid_t * uid);
typedef int (*GET_LSF_USER_FN_T) (char *lsfUserName, unsigned int lsfUserNameSize);

typedef struct
{
	bool_t initialized;
	bool_t initFailed;
	SO_HANDLE_T handle;
	char padding[4];
	GET_LSF_USER_FN_T getLSFUser_;
	GET_LSF_USER_BY_NAME_FN_T getLSFUserByName_;
	GET_LSF_USER_BY_UID_FN_T getLSFUserByUid_;
	GET_OS_USER_NAME_FN_T getOSUserName_;
	GET_OS_UID_FN_T getOSUid_;
} IDLIB_INFO_T;


static IDLIB_INFO_T idLib_ = {
	FALSE,
	FALSE,
	0,
	"    ",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};
