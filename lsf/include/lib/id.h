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

// #include "lib/lib.h"
// #include "lib/so.h"

#include <sys/types.h>
#include <stddef.h>

// #define NL_SETN 23
// #define IDLIB_SO_NAME "liblsfid.so"
// #define GET_LSF_USER         "getLSFUser_"
// #define GET_LSF_USER_BY_NAME "getLSFUserByName_"
// #define GET_LSF_USER_BY_UID  "getLSFUserByUid_"
// #define GET_OS_USER_NAME     "getOSUserName_"
// #define GET_OS_UID           "getOSUid_"

// const char GET_LSF_USER[ ]         = "getLSFUser_";
// const char GET_LSF_USER_BY_NAME[ ] = "getLSFUserByName_";
// const char GET_LSF_USER_BY_UID[ ]  = "getLSFUserByUid_";
// const char GET_OS_USER_NAME[ ]     = "getOSUserName_";
// const char GET_OS_UID[ ]           = "getOSUid_";

// const char IDLIB_SO_NAME[] = "liblsfid.so";

// https://stackoverflow.com/questions/14134245/iso-c-void-and-function-pointers

typedef int (*GET_LSF_USER_BY_NAME_FN_T) ( const char *osUserName,  const char  *lsfUserName,       unsigned int lsfUserNameSize );
typedef int (*GET_LSF_USER_BY_UID_FN_T)  ( const uid_t uid,         const char  *lsfUserName,       unsigned int lsfUserNameSize );
typedef int (*GET_OS_USER_NAME_FN_T)     ( const char *lsfUserName, const char  *osUserName,        unsigned int osUserNameSize  );
typedef int (*GET_OS_UID_FN_T)           ( const char *lsfUserName,                           const uid_t *uid );
typedef int (*GET_LSF_USER_FN_T)         ( const char *lsfUserName,                                 unsigned int lsfUserNameSize );

// typedef
struct IDLIB_INFO_T
{
    unsigned short initialized;
    unsigned short initFailed;
    int handle;
    // char padding[8];
    GET_LSF_USER_FN_T getLSFUser_;
    GET_LSF_USER_BY_NAME_FN_T getLSFUserByName_;
    GET_LSF_USER_BY_UID_FN_T getLSFUserByUid_;
    GET_OS_USER_NAME_FN_T getOSUserName_;
    GET_OS_UID_FN_T getOSUid_;
};

struct IDLIB_INFO_T idLib_ = {
    0,
    0,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* id.c */
void initIdLibDefaults         ( struct IDLIB_INFO_T *idLib );
int initIdLib                  ( struct IDLIB_INFO_T *idLib );
unsigned short checkInit       ( struct IDLIB_INFO_T *idLib );
int getLSFUser_                ( const char *lsfUserName, unsigned int lsfUserNameSize );
int getLSFUserByName_          ( const char *osUserName, const char *lsfUserName, unsigned int lsfUserNameSize );
int getLSFUserByUid_           ( const uid_t uid, const char *lsfUserName, unsigned int lsfUserNameSize);
int getOSUserName_             ( const char *lsfUserName, const char *osUserName, unsigned int osUserNameSize );
int getOSUid_                  ( const char *lsfUserName, const uid_t *uid);
struct passwd *getpwlsfuser_   ( const char *lsfUserName );
struct passwd *getpwdirlsfuser_( const char *lsfUserName );
int defGetLSFUser              ( const char *lsfUserName, unsigned int lsfUserNameSize );
int defGetLSFUserByName        ( const char *osUserName, const char *lsfUserName, unsigned int lsfUserNameSize );
int defGetLSFUserByUid         ( const uid_t uid, const char *lsfUserName, unsigned int lsfUserNameSize);
int defGetOSUserName           ( const char *lsfUserName, const char *osUserName, unsigned int osUserNameSize);
int defGetOSUid                ( const char *lsfUserName, const uid_t *uid);

