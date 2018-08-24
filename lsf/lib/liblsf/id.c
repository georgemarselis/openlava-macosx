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

#include "lib/id.h"

static void
initIdLibDefaults (IDLIB_INFO_T * idLib)
{
	idLib->getLSFUser_        = defGetLSFUser;
	idLib->getLSFUserByName_  = defGetLSFUserByName;
	idLib->getLSFUserByUid_   = defGetLSFUserByUid;
	idLib->getOSUserName_     = defGetOSUserName;
	idLib->getOSUid_          = defGetOSUid;
}

static int
initIdLib (IDLIB_INFO_T * idLib)
{

	int retcode = -1;
	char *serverDir = NULL;
	char *libPath = NULL;
// #define LIB_FORMAT_STR "%s/%s"

	idLib->initialized = TRUE;

	serverDir = genParams_[LSF_SERVERDIR].paramValue;
	if (serverDir == NULL)
	{
		if (logclass & LC_TRACE)
		{
			ls_syslog (LOG_DEBUG, "%s: No id library found, using defaults",
				__func__);
		}
		initIdLibDefaults (idLib);
		retcode = 0;
		goto cleanup;
	}

	libPath = malloc (strlen (serverDir) + strlen (IDLIB_SO_NAME) + 2);
	if (libPath == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		goto cleanup;
	}

	sprintf (libPath, "%s/%s", serverDir, IDLIB_SO_NAME);

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG, "%s: Loading library from path %s", __func__, libPath);
	}

	idLib->handle = soOpen_ (libPath);
	if (idLib->handle == 0)
	{
		if (logclass & LC_TRACE)
		{
			ls_syslog (LOG_DEBUG,
				"%s: No id library loaded (%k), using defaults", __func__);
		}
		initIdLibDefaults (idLib);
		retcode = 0;
		goto cleanup;
	}

	idLib->getLSFUser_ = (GET_LSF_USER_FN_T)
	soSym_ (idLib->handle, "getLSFUser_");
	if (idLib->getLSFUser_ == NULL)
	{
	  /* catgets 6351 */
		ls_syslog (LOG_ERR, "6351: %s: Error loading symbol %s from library %s: %k", __func__, "getLSFUser_", libPath);
		goto cleanup;
	}
	idLib->getLSFUserByName_ = (GET_LSF_USER_BY_NAME_FN_T)
	soSym_ (idLib->handle, "getLSFUserByName_");
	if (idLib->getLSFUserByName_ == NULL)
	{
	  /* catgets 6351 */
		ls_syslog (LOG_ERR, "6351: %s: Error loading symbol %s from library %s: %k", __func__, "getLSFUserByName_", libPath);
		goto cleanup;
	}
	idLib->getLSFUserByUid_ = (GET_LSF_USER_BY_UID_FN_T)
	soSym_ (idLib->handle, "getLSFUserByUid_");
	if (idLib->getLSFUserByUid_ == NULL)
	{
	  /* catgets 6351 */
		ls_syslog (LOG_ERR, "6351: %s: Error loading symbol %s from library %s: %k", __func__, "getLSFUserByUid_", libPath);
		goto cleanup;
	}
	idLib->getOSUserName_ = (GET_OS_USER_NAME_FN_T)
	soSym_ (idLib->handle, "getOSUserName_");
	if (idLib->getOSUserName_ == NULL)
	{
		/* catgets 6351 */
		ls_syslog (LOG_ERR, "6351: %s: Error loading symbol %s from library %s: %k", __func__, "getOSUserName_", libPath);
		goto cleanup;
	}
	
	// FIXME FIXME FIXME FIXME
	// check with the debugger
	idLib->getOSUid_ = (GET_OS_UID_FN_T) soSym_ (idLib->handle, "getOSUid_");
	if ( NULL == idLib->getOSUid_ ) {
		/* catgets 6351 */
		ls_syslog (LOG_ERR, "6351: %s: Error loading symbol %s from library %s: %k", __func__, getOSUid_, libPath);
		goto cleanup;
	}

	retcode = 0;

cleanup:  // FIXME FIXME FIXME FIXME FIXME remove cleanup: label

FREEUP (libPath);

if (retcode != 0)
{
	idLib->initFailed = TRUE;

	idLib->getLSFUser_ = NULL;
	idLib->getLSFUserByName_ = NULL;
	idLib->getLSFUserByUid_ = NULL;
	idLib->getOSUserName_ = NULL;
	idLib->getOSUid_ = NULL;
}

return retcode;
}

static bool_t
checkInit (IDLIB_INFO_T * idLib)
{
	if (!idLib->initialized)
	{
		initIdLib (idLib);
	}
	if (idLib->initFailed)
	{
		lserrno = LSE_INTERNAL;
		return FALSE;
	}
	return TRUE;
}

int
getLSFUser_ (char *lsfUserName, unsigned int lsfUserNameSize)
{
	int rc = 0;

	if (!checkInit (&idLib_))
	{
		return -1;
	}

	rc = idLib_.getLSFUser_ (lsfUserName, lsfUserNameSize);
	if (rc != LSE_NO_ERR)
	{
		lserrno = rc;
		return -1;
	}
	else
	{
		return 0;
	}
}

int
getLSFUserByName_ (const char *osUserName, char *lsfUserName, unsigned int lsfUserNameSize)
{
	int rc = 0;

	if (!checkInit (&idLib_))
	{
		return -1;
	}

	rc = idLib_.getLSFUserByName_ (osUserName, lsfUserName, lsfUserNameSize);
	if (rc != LSE_NO_ERR)
	{
		lserrno = rc;
		return -1;
	}
	else
	{
		return 0;
	}
}

int
getLSFUserByUid_ (uid_t uid, char *lsfUserName, unsigned int lsfUserNameSize)
{
	int rc = 0;

	if (!checkInit (&idLib_))
	{
		return -1;
	}

	rc = idLib_.getLSFUserByUid_ (uid, lsfUserName, lsfUserNameSize);
	if (rc != LSE_NO_ERR)
	{
		lserrno = rc;
		return -1;
	}
	else
	{
		return 0;
	}
}

int
getOSUserName_ (const char *lsfUserName, char *osUserName, unsigned int osUserNameSize)
{
	int rc;

	if (!checkInit (&idLib_))
	{
		return -1;
	}

	rc = idLib_.getOSUserName_ (lsfUserName, osUserName, osUserNameSize);
	if (rc != LSE_NO_ERR)
	{
		lserrno = rc;
		return -1;
	}
	else
	{
		return 0;
	}
}


int
getOSUid_ (const char *lsfUserName, uid_t * uid)
{
	int rc;

	if (!checkInit (&idLib_))
	{
		return -1;
	}

	rc = idLib_.getOSUid_ (lsfUserName, uid);
	if (rc != LSE_NO_ERR)
	{
		lserrno = rc;
		return -1;
	}

	return 0;
}

struct passwd *getpwlsfuser_( const char *lsfUserName ) 
{
	struct passwd *pw = NULL;
	char osUserName[MAX_LSF_NAME_LEN];

	memset ( osUserName, '\0', strlen( osUserName ) );

	if (getOSUserName_ (lsfUserName, osUserName, sizeof (osUserName)) < 0)
	{
		lserrno = LSE_INTERNAL;
		return NULL;
	}

	if ((pw = getpwnam (osUserName)) == NULL)
	{
		lserrno = LSE_BADUSER;
		return NULL;
	}

	return pw;
}

struct passwd *
getpwdirlsfuser_ (const char *lsfUserName)
{
	struct passwd *pw = NULL;
	char osUserName[MAX_LSF_NAME_LEN];

	memset ( osUserName, 0, strlen( osUserName ) );

	if (getOSUserName_ (lsfUserName, osUserName, sizeof (osUserName)) < 0)
	{
		lserrno = LSE_INTERNAL;
		return NULL;
	}

	if ((pw = getpwnam (osUserName)) == NULL)
	{
		lserrno = LSE_BADUSER;
		return NULL;
	}

	return pw;
}

static int
defGetLSFUser (char *lsfUserName, unsigned int lsfUserNameSize)
{
	struct passwd *pw = NULL;

	lsfUserName[0] = '\0';

	if ((pw = getpwuid (getuid ())) == NULL)
	{
		return LSE_BADUSER;
	}

	if (strlen (pw->pw_name) + 1 > lsfUserNameSize)
	{
		return LSE_BAD_ARGS;
	}
	strcpy (lsfUserName, pw->pw_name);

	return LSE_NO_ERR;
}

static int
defGetLSFUserByName (const char *osUserName, char *lsfUserName, unsigned int lsfUserNameSize)
{
	lsfUserName[0] = '\0';

	if (strlen (osUserName) + 1 > lsfUserNameSize)
	{
		return LSE_BAD_ARGS;
	}
	strcpy (lsfUserName, osUserName);

	return LSE_NO_ERR;
}

static int
defGetLSFUserByUid (uid_t uid, char *lsfUserName, unsigned int lsfUserNameSize)
{
	struct passwd *pw = NULL;

	lsfUserName[0] = '\0';

	if ((pw = getpwuid (uid)) == NULL)
	{
		return LSE_BADUSER;
	}

	if (strlen (pw->pw_name) + 1 > lsfUserNameSize)
	{
		return LSE_BAD_ARGS;
	}
	strcpy (lsfUserName, pw->pw_name);

	return LSE_NO_ERR;
}

static int
defGetOSUserName (const char *lsfUserName, char *osUserName, unsigned int osUserNameSize)
{
	osUserName[0] = '\0';

	if (strlen (lsfUserName) + 1 > osUserNameSize)
	{
		return LSE_BAD_ARGS;
	}
	strcpy (osUserName, lsfUserName);

	return LSE_NO_ERR;
}

static int
defGetOSUid (const char *lsfUserName, uid_t * uid)
{
	struct passwd *pw = NULL;

	if ((pw = getpwnam (lsfUserName)) == NULL)
	{
		return LSE_BADUSER;
	}

	*uid = pw->pw_uid;

	return LSE_NO_ERR;
}
