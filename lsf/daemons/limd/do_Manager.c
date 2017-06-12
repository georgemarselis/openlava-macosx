

#include "daemons/liblimd/do_Manager.h"



char do_Manager (FILE *fp, const char *filename, size_t *lineNum, const char *secName, int lookupAdmins)
{
	char *linep            = NULL;
	struct keymap *keyList = NULL;

	enum managers {
		MANAGERS,
		KEYNULLMANAGERS
	};

	enum administrators {
		ADMINISTRATORS,
		KEYNULLADMINISTRATORS
	};

	const char *keylistManagers[] = {
		"MANAGERS",
		NULL
	};

	const char *keylistAdministrators[] = {
		"ADMINISTRATORS",
		NULL
	};

	struct keymap keyList1[] = {
		{ MANAGERS,        "    ", keylistManagers[MANAGERS], NULL },
		{ KEYNULLMANAGERS, "    ", NULL, NULL }
	};

	struct keymap keyList2[] = {
		{ ADMINISTRATORS,        "    " , keylistAdministrators[ADMINISTRATORS], NULL },
		{ KEYNULLADMINISTRATORS, "    " , NULL, NULL }
	};

	const char clustermanager[]  = "clustermanager";

	if (lim_debug > 0 && lim_debug < 3) {

		char lsfUserName[MAXLSFNAMELEN];

		nClusAdmins   = 1;
		clusAdminIds  = malloc( sizeof (uid_t) );
		clusAdminGids = malloc( sizeof (uid_t) );
		if (getLSFUser_ (lsfUserName, sizeof ( lsfUserName)) < 0) {
			const char getLSFUser[] = "getLSFUser";
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, getLSFUser);
			return FALSE;
		}
		clusAdminIds[ADMINISTRATORS]   = getuid ();
		clusAdminGids[ADMINISTRATORS]  = getgid ();
		clusAdminNames                 = malloc (sizeof (char *));
		clusAdminNames[ADMINISTRATORS] = putstr_ (lsfUserName);
		doSkipSection (clfp, lineNum, lsfile, secName);
		if (lim_CheckMode > 0) {
			/* catgets 5289 */
			ls_syslog (LOG_ERR, "5289: %s: %s(%d): The cluster manager is the invoker <%s> in debug mode", __func__, lsfile, *lineNum, lsfUserName);
			return TRUE; // return here may be outside the conditional
		}
			// FIXME FIXME FIXME some calls to free() oughta be here
	}

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep) {
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, secName);
		return FALSE;
	}

	if (isSectionEnd (linep, filename, lineNum, secName)) {
		return FALSE;
	}

	if (strcmp (secName, clustermanager) == 0) {
		keyList = keyList1;
	}
	else {
		keyList = keyList2;
	}

	if (strchr (linep, '=') == NULL)
	{
		if (!keyMatch (keyList, linep, TRUE))
		{
			/* catgets 5116 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5116, "%s: %s(%d): keyword line format error for section %s, ignoring section")), __func__, filename, *lineNum, secName);
			doSkipSection (fp, lineNum, filename, secName);
			return FALSE;
		}

		if ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
		{
			if (isSectionEnd (linep, filename, lineNum, secName)) {
				return FALSE;
			}
			if (mapValues (keyList, linep) < 0)
			{
				/* catgets 5117 */
				ls_syslog (LOG_ERR, (_i18n_msg_get(ls_catd, NL_SETN, 5117, "%s: %s(%d): values do not match keys for section %s, ignoring section")),  __func__, filename, *lineNum, secName);
				doSkipSection (fp, lineNum, filename, secName);
				return FALSE;
			}
			if (ls_getClusAdmins(keyList[ADMINISTRATORS].val, filename, lineNum, secName, lookupAdmins) < 0)
			{
				FREEUP (keyList[ADMINISTRATORS].val);
				return FALSE;
			}
			else
			{
				FREEUP (keyList[ADMINISTRATORS].val);
				return TRUE;
			}
		}
	}
	else
	{
		if (readHvalues (keyList, linep, fp, filename, lineNum, TRUE, secName) < 0) {
			return FALSE;
		}
		if (ls_getClusAdmins(keyList[ADMINISTRATORS].val, filename, lineNum, secName, lookupAdmins) < 0)
		{
			FREEUP (keyList[ADMINISTRATORS].val);
			return FALSE;
		}
		else
		{
			FREEUP (keyList[ADMINISTRATORS].val);
			return TRUE;
		}
	}
	return TRUE;
}