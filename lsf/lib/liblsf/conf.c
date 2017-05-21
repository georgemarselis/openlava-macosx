/* $Id: lib.conf.c 397 2007-11-26 19:04:00Z mblack $
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

#include <ctype.h>
#include <grp.h>
#include <strings.h>
#include <pwd.h>

#include "lib/lproto.h"
#include "lib/conf.h"

// #define NL_SETN 42



int
initResTable_ (void)
{
	struct resItem *resTable;
	unsigned int i;

	resTable = malloc (1000 * sizeof (struct resItem) + 1 * sizeof( struct resItem ) ); // FIXME FIXME FIXME FIXME '1000' is awfuly partifular
	if (!resTable)
		{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		return -1;
		}
	i = 0;
	lsinfo.numIndx = 0;
	lsinfo.numUsrIndx = 0;
	while (builtInRes[i].name != NULL)
		{
		strcpy (resTable[i].name, builtInRes[i].name);
		strcpy (resTable[i].des, _i18n_msg_get (ls_catd, NL_SETN, builtInRes_ID[i], builtInRes[i].des));
		resTable[i].valueType = builtInRes[i].valuetype;
		resTable[i].orderType = builtInRes[i].ordertype;
		resTable[i].interval = builtInRes[i].interval;
		resTable[i].flags = builtInRes[i].flags;
		if (resTable[i].flags & RESF_DYNAMIC) {
			lsinfo.numIndx++;
		}
		i++;
		}
	lsinfo.nRes = i;
	lsinfo.resTable = resTable;
	return 0;
}


struct sharedConf *ls_readshared (char *filename)
{
	FILE *fp = NULL;
	char *cp = NULL;
	char *word = NULL;
	char modelok, resok, clsok, typeok;
	size_t lineNum = 0;

	lserrno = LSE_NO_ERR;
	if (__func__ == NULL)
		{
		/* catgets 5050 */
		ls_syslog (LOG_ERR, (_i18n_msg_get(ls_catd, NL_SETN, 5050, "%s: filename is NULL")), __func__);
		lserrno = LSE_NO_FILE;
		return NULL;
		}

	lsinfo.nRes = 0;
	FREEUP (lsinfo.resTable);
	lsinfo.nTypes = 0;
	lsinfo.nModels = 0;
	lsinfo.numIndx = 0;
	lsinfo.numUsrIndx = 0;

	if (sConf == NULL) {
		sConf = malloc (sizeof (struct sharedConf));
		if (NULL == sConf && ENOMEM == errno ) {
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
			lserrno = LSE_MALLOC;
			return NULL;
		}
		sConf->lsinfo = &lsinfo;
		sConf->clusterName = NULL;
		sConf->servers = NULL;
	}
	else {
		FREEUP (sConf->clusterName);
		FREEUP (sConf->servers);
		sConf->lsinfo = &lsinfo;
	}

	modelok = FALSE;
	resok = FALSE;
	clsok = FALSE;
	typeok = FALSE;

	if (initResTable_() < 0) {
		lserrno = LSE_MALLOC;
		return NULL;
	}
	fp = fopen (__func__, "r");
	if (fp == NULL) {
		/* catgets 5052 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5052, "%s: Can't open configuration file <%s>.")) , __func__, filename);
		lserrno = LSE_NO_FILE;
		return NULL;
	}

	for (;;) {
		if ((cp = getBeginLine (fp, &lineNum)) == NULL) {
			fclose (fp);
			if (!modelok)
				{
				/* catgets 5053 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5053, "%s: HostModel section missing or invalid")), filename);
				}
			if (!resok)
				{
				/* catgets 5054 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5054, "%s: Resource section missing or invalid")), filename);
				}
			if (!typeok)
				{
				/* catgets 5055 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5055, "%s: HostType section missing or invalid")), filename);
				}
			if (!clsok)
				{
				/* catgets 5056 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5056, "%s: Cluster section missing or invalid")), filename);
				}
			return sConf;
			}


		word = getNextWord_ (&cp);
		if (!word)
			{
			/* catgets 5057 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5057, "%s: %s(%d): Section name expected after Begin; ignoring section")), __func__, filename, lineNum);
			doSkipSection (fp, &lineNum, filename, "unknown");
			continue;
			}
		else
			{
			if (strcasecmp (word, "host") == 0)
				{
				/* catgets 5103 */
				ls_syslog (LOG_INFO, I18N (5103, "%s: %s(%d): section %s no longer needed in this version, ignored"), __func__, filename, lineNum, word);
				continue;
				}

			if (strcasecmp (word, "hosttype") == 0)
				{
				if (do_HostTypes (fp, &lineNum, filename)) {
					typeok = TRUE;
				}
				continue;
				}

			if (strcasecmp (word, "hostmodel") == 0)
				{
				if (do_HostModels (fp, &lineNum, filename)) {
					modelok = TRUE;
				}
				continue;
				}

			if (strcasecmp (word, "resource") == 0)
				{
				if (do_Resources (fp, &lineNum, filename)) {
					resok = TRUE;
				}
				continue;
				}

			if (strcasecmp (word, "cluster") == 0)
				{
				if (do_Cluster (fp, &lineNum, filename)) {
					clsok = TRUE;
				}
				continue;
				}

			if (strcasecmp (word, "newindex") == 0)
				{
				do_Index (fp, &lineNum, filename);
				continue;
				}

			/* catgets 5058 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5058, "%s: %s(%d): Invalid section name %s; ignoring section")), __func__, filename, lineNum, word);
			doSkipSection (fp, &lineNum, filename, word);
			}
		}
}

char do_Index (FILE * fp, size_t *lineNum, char *filename)
{
	const short structArraySize = 4;
	int *returnValue[4] = {0, 0, 0, 0};
	char *linep;

	char *textValues[4] = { "INTERVAL", "INCREASING", "DESCRIPTION", "NAME"};

	struct keymap *keyList = malloc( structArraySize * sizeof( struct keymap ) );

	short n = 0;
	while( n < structArraySize ) {
		*returnValue[n] = sscanf( textValues[n], "%s", (keyList + n)->key );
		(keyList + n)->val = NULL;
		(keyList + n)->position = 0;

		n++;
	}


	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep)
		{
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "index");
		return FALSE;
		}

	if (isSectionEnd (linep, filename, lineNum, "newindex"))
		{
		/* catgets 5061 */
		ls_syslog (LOG_WARNING, (_i18n_msg_get (ls_catd, NL_SETN, 5061, "%s: %s(%d): empty section")),  __func__, filename, *lineNum);
		return FALSE;
		}

	if (strchr (linep, '=') == NULL)
		{
		if (!keyMatch (keyList, linep, TRUE))
			{
			/* catgets 5062 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5062, "%s: %s(%d): keyword line format error for section newindex; ignoring section")), __func__, filename, *lineNum);
			doSkipSection (fp, lineNum, filename, "newindex");
			return FALSE;
			}

		while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
			{
			if (isSectionEnd (linep, filename, lineNum, "newindex")) {
				return TRUE;
			}
			if (mapValues (keyList, linep) < 0)
				{
				/* catgets 5063 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5063, "%s: %s(%d): values do not match keys for section newindex; ignoring line")), __func__, filename, *lineNum);
				continue;
				}

			setIndex (keyList, filename, *lineNum);
			}
		}
	else
		{
		if (readHvalues (keyList, linep, fp, filename, lineNum, TRUE, "newindex") < 0) {
			return FALSE;
		}
		setIndex (keyList, filename, *lineNum);
		return TRUE;
		}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "newindex");
	return TRUE;
}

char setIndex (struct keymap *keyList, char *filename, size_t lineNum)
{
	unsigned int resIdx = 0;

	if (keyList == NULL) {
		return FALSE;
	}

	if (strlen (keyList[3].val) >= MAXLSFNAMELEN) {
		/* catgets 5065 */
		ls_syslog (LOG_ERR, "catgets 5065: %s: %s(%d): Name %s is too long (maximum is %d chars); ignoring index", __func__, filename, lineNum, keyList[3].val, MAXLSFNAMELEN - 1);
		return FALSE;
	}

	if (strpbrk (keyList[3].val, ILLEGAL_CHARS) != NULL) {
		/* catgets 5066 */
		ls_syslog (LOG_ERR, "catgets 5066 %s: %s(%d): illegal character (one of %s), ignoring index %s", __func__, filename, lineNum, ILLEGAL_CHARS, keyList[3].val);
		return FALSE;
	}

	assert( resNameDefined (keyList[3].val) > 0 ); // FIXME FIXME FIXME 3? change that to a enum
	// resIdx = (unsigned int) resNameDefined (keyList[3].val); // FIXME FIXME FIXME check if there is a reason for the cast here
	resIdx = resNameDefined (keyList[3].val); // FIXME FIXME FIXME check if there is a reason for the cast here
	if (!(lsinfo.resTable[resIdx].flags & RESF_DYNAMIC)) {
		/* catgets 5067 */
		ls_syslog (LOG_ERR, "catgets 5067: %s: %s(%d): Name %s is not a dynamic resource; ignored", __func__, filename, lineNum, keyList[3].val);
		return FALSE;
	}
	else {
		resIdx = lsinfo.nRes;
	}

	lsinfo.resTable[resIdx].interval = atoi (keyList[0].val); 	// FIXME FIXME FIXME change 0 to appropriate label in enum
	lsinfo.resTable[resIdx].orderType =(strcasecmp (keyList[1].val, "y") == 0) ? INCR : DECR; // FIXME FIXME FIXME change 1 to appropriate label in enum

	strcpy (lsinfo.resTable[resIdx].des, keyList[2].val);
	strcpy (lsinfo.resTable[resIdx].name, keyList[3].val);
	lsinfo.resTable[resIdx].valueType = LS_NUMERIC;
	lsinfo.resTable[resIdx].flags = RESF_DYNAMIC | RESF_GLOBAL;

	if (resIdx == lsinfo.nRes)
		{
		lsinfo.numUsrIndx++;
		lsinfo.numIndx++;
		lsinfo.nRes++;
		}

	FREEUP (keyList[0].val);
	FREEUP (keyList[1].val);
	FREEUP (keyList[2].val);
	FREEUP (keyList[3].val);
	return TRUE;
}

char do_HostTypes (FILE * fp, size_t *lineNum, char *filename)
{
	struct keymap keyList[] = {
		{0, "    " , NULL, "TYPENAME"},
		{0, "    " , NULL, NULL}
	};
	char *linep = NULL;

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep)
		{
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "hostType");
		return FALSE;
		}

	if (isSectionEnd (linep, filename, lineNum, "HostType")) {
		return FALSE;
	}

	if (strchr (linep, '=') == NULL)
		{
		if (!keyMatch (keyList, linep, TRUE))
			{
				/* catgets 5070 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5070, "%s: %s(%d): keyword line format error for section HostType, ignoring section")), __func__, filename, *lineNum);
			doSkipSection (fp, lineNum, filename, "HostType");
			return FALSE;
			}

		while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
			{
			if (isSectionEnd (linep, filename, lineNum, "HostType")) {
				return TRUE;
			}
			if (mapValues (keyList, linep) < 0)
				{
					/* catgets 5071 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5071, "%s: %s(%d): values do not match keys for section cluster, ignoring line")), __func__, filename, *lineNum);
				continue;
				}

			if (strpbrk (keyList[0].val, ILLEGAL_CHARS) != NULL) // FIXME FIXME FIXME replace [0] wtih enum label
				{
				/* catgets 5072 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5072, "%s: %s(%d): illegal character (one of %s), ignoring type %s")), __func__, filename, *lineNum, ILLEGAL_CHARS, keyList[0].val); // FIXME FIXME FIME replace [0] with enum LABEL
				FREEUP (keyList[0].val); // FIXME FIXME FIXME replace [0] wtih enum label
				continue;
				}

			addHostType (keyList[0].val); // FIXME FIXME FIXME replace [0] wtih enum label
			FREEUP (keyList[0].val);
			}
		}
	else
		{
		ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "HostType");
		doSkipSection (fp, lineNum, filename, "HostType");
		return FALSE;
		}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "hostType");

	return TRUE;
}

char
addHostType (char *type)
{

	if (type == NULL) {
		return FALSE;
	}

	if (lsinfo.nTypes == MAXTYPES)
		{
		/* catgets 5075 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5075, "%s: Too many host types defined in section HostType. You can only define up to %d host types; host type %s ignored")), __func__, MAXTYPES, type);
		return FALSE;
		}

	for (unsigned int i = 0; i < lsinfo.nTypes; i++) {
		if (strcmp (lsinfo.hostTypes[i], type) != 0) {
			continue;
		}
		/* catgets 5076 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5076, "%s: host type %s multiply defined")), __func__, type);
		return FALSE;
	}

	strcpy (lsinfo.hostTypes[lsinfo.nTypes], type);
	lsinfo.nTypes++;

	return TRUE;
}

char do_HostModels (FILE * fp, size_t *lineNum, char *filename)
{
	char *linep;
	struct keymap keyList[] = {
		{0, "    " , NULL, "MODELNAME"},
		{0, "    " , NULL, "CPUFACTOR"},
		{0, "    " , NULL, "ARCHITECTURE"},
		{0, "    " , NULL, NULL}
	};
	char *sp   = NULL;
	char *word = NULL;

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep) {
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "hostModel");
		return FALSE;
	}

	if (isSectionEnd (linep, filename, lineNum, "hostmodel")) {
		return FALSE;
	}

	if (strchr (linep, '=') == NULL) {
		if (!keyMatch (keyList, linep, FALSE)) {
			/* catgets 5078 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5078, "%s: %s(%d): keyword line format error for section hostmodel, ignoring section")), __func__, filename, *lineNum);
			doSkipSection (fp, lineNum, filename, filename);
			return FALSE;
		}

		while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {

			if (isSectionEnd (linep, filename, lineNum, "hostmodel")) {
				return TRUE;
			}

			if (mapValues (keyList, linep) < 0) {
				/* catgets 5079 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5079, "%s: %s(%d): values do not match keys for section hostmodel, ignoring line")), __func__, filename, *lineNum);
				continue;
			}

			if (!isanumber_ (keyList[1].val) || atof (keyList[1].val) <= 0) { // FIXME FIXME FIXME replace 1 with uname label
				/* catgets 5080 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5080, "%s: %s(%d): Bad cpuFactor for host model %s, ignoring line")), __func__, filename, *lineNum, keyList[0].val); // FIXME FIXME FIXME replace 0 with enum label
				FREEUP (keyList[0].val); // FIXME FIXME FIXME replace 0 with uname label
				FREEUP (keyList[1].val); // FIXME FIXME FIXME replace 1 with uname label
				FREEUP (keyList[2].val); // FIXME FIXME FIXME replace 2 with uname label
				continue;
			}

			if (strpbrk (keyList[0].val, ILLEGAL_CHARS) != NULL) {
				/* catgets 5081 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5081, "%s: %s(%d): illegal character (one of %s), ignoring model %s")), __func__, filename, *lineNum, ILLEGAL_CHARS, keyList[0].val);
				FREEUP (keyList[0].val); // FIXME FIXME FIXME replace 0 with uname label
				FREEUP (keyList[1].val); // FIXME FIXME FIXME replace 1 with uname label
				FREEUP (keyList[2].val); // FIXME FIXME FIXME replace 2 with uname label
				continue;
			}

			sp = keyList[2].val;
			if (sp && sp[0]) {
				while ((word = getNextWord_ (&sp)) != NULL) {
					addHostModel (keyList[0].val, word, atof (keyList[1].val)); // FIXME FIXME FIXME replace 1 with uname label
				}
			}
			else {
				addHostModel (keyList[0].val, NULL, atof (keyList[1].val)); // FIXME FIXME FIXME replace 1 with uname label
				FREEUP (keyList[0].val); // FIXME FIXME FIXME replace 0 with uname label
				FREEUP (keyList[1].val); // FIXME FIXME FIXME replace 1 with uname label
				FREEUP (keyList[2].val); // FIXME FIXME FIXME replace 2 with uname label
			}
		}
	}
	else {
		ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "HostModel");
		doSkipSection (fp, lineNum, filename, "HostModel");
		return FALSE;
	}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "HostModel");
	return TRUE;
}

char addHostModel (char *model, char *arch, double factor)
{
	if (model == NULL) {
		return FALSE;
	}

	if (lsinfo.nModels == MAXMODELS) {
		/* catgets 5084 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5084, "%s: Too many host models defined in section HostModel. You can only define up to %d host models; host model %s ignored")), __func__, MAXMODELS, model);
		return FALSE;
	}

	for (unsigned int i = 0; i < lsinfo.nModels; ++i) {
		if (arch == 0 || strcmp (lsinfo.hostArchs[i], arch) != 0) {
			continue;
		}
		/* FIXME FIXME two lines below:
		 left over from previously dangling if above. might need to be moved inside the if brackets */
		/* catgets 5085 */
		ls_syslog (LOG_ERR, I18N (5085, "%s: Duplicate host architecture type found in section HostModel. Architecture type must be unique; host model %s ignored"), __func__, model);

		return FALSE;
	}

	strcpy (lsinfo.hostModels[lsinfo.nModels], model);
	strcpy (lsinfo.hostArchs[lsinfo.nModels], arch ? arch : "");
	lsinfo.cpuFactor[lsinfo.nModels] = factor;
	lsinfo.nModels++;

	return TRUE;
}


#define RKEY_RESOURCENAME 0
#define RKEY_TYPE         1
#define RKEY_INTERVAL     2
#define RKEY_INCREASING   3
#define RKEY_RELEASE      4
#define RKEY_DESCRIPTION  5

/* FIXME FIXME
 This function will need
 1. test cases to make sure it does what it is supposed to do
 2. testing to see it does what it is supposed to do
 3. debugging and profing to make sure nothing spills out of bounds or rolls over

 */
char do_Resources (FILE * fp, size_t *lineNum, char *filename)
{
	int nres = 0;
	char *linep;
	struct keymap keyList[] = {
		{0, "    " , NULL, "RESOURCENAME"},
		{0, "    " , NULL, "TYPE"},
		{0, "    " , NULL, "INTERVAL"},
		{0, "    " , NULL, "INCREASING"},
		{0, "    " , NULL, "RELEASE"},
		{0, "    " , NULL, "DESCRIPTION"},
		{0, "    " , NULL, NULL}
	}; // FIXME FIXME FIXME needs additional enum to translate labels to 

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep) {
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, "do_Resources", __func__, *lineNum, "resource");
		return FALSE;
	}

	if (isSectionEnd (linep, filename, lineNum, "resource")) {
		return FALSE;
	}

	if (strchr (linep, '=') == NULL) {
		if (!keyMatch (keyList, linep, FALSE)) {
			/* catgets 5086 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5086, "%s: %s(%d): keyword line format error for section resource, ignoring section")), __func__, filename, *lineNum);
			ls_syslog (LOG_ERR, "do_Resources: %s", linep);
			doSkipSection (fp, lineNum, filename, "resource");
			return FALSE;
		}

		while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) {
			if (isSectionEnd (linep, filename, lineNum, "resource")) {
				return TRUE;
			}

			if (mapValues (keyList, linep) < 0) {
				/* catgets 5087 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5087, "%s: %s(%d): values do not match keys for section resource, ignoring line")), __func__, filename, *lineNum);
				continue;
			}

			if (strlen (keyList[RKEY_RESOURCENAME].val) >= MAXLSFNAMELEN - 1) {
				/* catgets 5088 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5088, "%s: %s(%d): Resource name %s too long in section resource. Should be less than %d characters,  ignoring line")), __func__, filename, *lineNum, keyList[RKEY_RESOURCENAME].val, MAXLSFNAMELEN - 1);
				freeKeyList (keyList);
				continue;
			}

			if (resNameDefined (keyList[RKEY_RESOURCENAME].val) >= 0) {
				/* catgets 5089 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5089, "%s: %s(%d): Resource name %s reserved or previously defined. Ignoring line")), __func__, filename, *lineNum, keyList[RKEY_RESOURCENAME].val);
				freeKeyList (keyList);
				continue;
			}

			if (strpbrk (keyList[RKEY_RESOURCENAME].val, ILLEGAL_CHARS) != NULL) {
				/* catgets 5090 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5090, "%s: %s(%d): illegal character (one of %s): in resource name:%s, section resource, ignoring line")), __func__, filename, *lineNum, ILLEGAL_CHARS, keyList[RKEY_RESOURCENAME].val);
				freeKeyList (keyList);
				continue;
			}

			if (isdigit (keyList[RKEY_RESOURCENAME].val[0])) {
				/* catgets 5091 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5091, "%s: %s(%d): Resource name <%s> begun with a digit is illegal; ignored")), __func__, filename, *lineNum, keyList[RKEY_RESOURCENAME].val);
				freeKeyList (keyList);
				continue;
			}

			lsinfo.resTable[lsinfo.nRes].name[0] = '\0';
			lsinfo.resTable[lsinfo.nRes].des[0] = '\0';
			lsinfo.resTable[lsinfo.nRes].flags = RESF_GLOBAL;
			lsinfo.resTable[lsinfo.nRes].valueType = LS_BOOLEAN;
			lsinfo.resTable[lsinfo.nRes].orderType = NA;
			lsinfo.resTable[lsinfo.nRes].interval = 0;

			strcpy (lsinfo.resTable[lsinfo.nRes].name, keyList[RKEY_RESOURCENAME].val);


			if (keyList[RKEY_TYPE].val != NULL && keyList[RKEY_TYPE].val[0] != '\0') {
				int type;

				/* FIXME
				 following line needs to go; be assigned to declaration of type and
				 use the assert() to check if it is greater or equal to zero
				 */
				if ((type = validType (keyList[RKEY_TYPE].val)) >= 0) {
					assert( type >= 0 );
					lsinfo.resTable[lsinfo.nRes].valueType = (enum valueType) type;
				}
				else {
					/* catgets 5092 */
					ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5092, "%s: %s(%d): resource type <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource")), __func__, filename, *lineNum, keyList[RKEY_TYPE].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
					freeKeyList (keyList);
					continue;
				}
			}

			if (keyList[RKEY_INTERVAL].val != NULL && keyList[RKEY_INTERVAL].val[0] != '\0') {

				int interval;

				if ((interval = atoi (keyList[RKEY_INTERVAL].val)) > 0) {

					lsinfo.resTable[lsinfo.nRes].interval = interval;
					if (lsinfo.resTable[lsinfo.nRes].valueType == LS_NUMERIC) {
						lsinfo.resTable[lsinfo.nRes].flags |= RESF_DYNAMIC;
					}
					else {
						/* catgets 5093 */
						ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5093, "%s: %s(%d): INTERVAL <%s> for resource <%s> should be a integer greater than 0; ignoring resource <%s> in section resource")), __func__, filename, *lineNum, keyList[RKEY_INTERVAL].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
						freeKeyList (keyList);
						continue;
					}
				}
			}

			if (keyList[RKEY_INCREASING].val != NULL && keyList[RKEY_INCREASING].val[0] != '\0') {

				if (lsinfo.resTable[lsinfo.nRes].valueType == LS_NUMERIC) {
					if (!strcasecmp (keyList[RKEY_INCREASING].val, "N")) {
						lsinfo.resTable[lsinfo.nRes].orderType = DECR;
					}
					else if (!strcasecmp (keyList[RKEY_INCREASING].val, "Y")) {
						lsinfo.resTable[lsinfo.nRes].orderType = INCR;
					}
					else {
						/* catgets 5094 */
						ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5094, "%s: %s(%d): INCREASING <%s> for resource <%s> is not valid; ignoring resource <%s> in section resource")), __func__, filename, *lineNum, keyList[RKEY_INCREASING].val, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
						freeKeyList (keyList);
						continue;
					}
				}
				else {
					char *syslog_result = NULL;
					unsigned int valueTypeResult = LS_BOOLEAN;
					unsigned int orderTypeResult = lsinfo.resTable[lsinfo.nRes].orderType;
					syslog_result = malloc( sizeof( char ) * 10 + 1);
					if( valueTypeResult == orderTypeResult )  {
					   strcpy( syslog_result, "BOOLEAN" );
					}
					else {
						strcpy( syslog_result, "STRING" );
					};
 
					/* catgets 5095 */
					ls_syslog (LOG_ERR, "%s: %s(%d): INCREASING <%s> is not used by the resource <%s> with type <%s>; ignoring INCREASING", __func__, filename, *lineNum, keyList[RKEY_INCREASING].val, keyList[RKEY_RESOURCENAME].val, syslog_result );
					free( syslog_result );
				}
			}
			else if (lsinfo.resTable[lsinfo.nRes].valueType == LS_NUMERIC) {
				/* catgets 5096 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5096, "%s: %s(%d): No INCREASING specified for a numeric resource <%s>; ignoring resource <%s> in section resource")), __func__, filename, *lineNum, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
				freeKeyList (keyList);
				continue;
			}
			else {
				printf( "man this is some fucked up code...\n");
				assert(1);
			}

			/* FIXME FIXME possible dangling if issue:
			 if bellow, might be part of the else block above

			 this shit will need the debugger to make sense
			 (or a boatload of test cases)
			 */
			if (keyList[RKEY_RELEASE].val != NULL && keyList[RKEY_RELEASE].val[0] != '\0') {

				if (lsinfo.resTable[lsinfo.nRes].valueType == LS_NUMERIC) {

					if (!strcasecmp (keyList[RKEY_RELEASE].val, "Y")) {
						lsinfo.resTable[lsinfo.nRes].flags |= RESF_RELEASE;
					}
					else if (strcasecmp (keyList[RKEY_RELEASE].val, "N")) {
						/*catgets 5212 */
						ls_syslog (LOG_ERR, I18N (5212, "%s:%s(%d): RELEASE defined for resource <%s> should be 'Y', 'y', 'N' or 'n' not <%s>; ignoring resource <%s> in section resource"), __func__, filename, *lineNum, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RELEASE].val, keyList[RKEY_RESOURCENAME].val);
						freeKeyList (keyList);
						continue;
					}
					else {
						printf( "WTF am i doing here? dangling else problem or parameters out of wack. Investige in conf.c\n");
					}
				}
				else {
					/*catgets 5213 */
					ls_syslog (LOG_ERR, I18N (5213, "%s:%s(%d): RELEASE cannot be defined for resource <%s> which isn't a numeric resource; ignoring resource <%s> in section resource"), __func__, filename, *lineNum, keyList[RKEY_RESOURCENAME].val, keyList[RKEY_RESOURCENAME].val);
					freeKeyList (keyList);
					continue;
				}
			}
			else {
				if (lsinfo.resTable[lsinfo.nRes].valueType == LS_NUMERIC) {
					lsinfo.resTable[lsinfo.nRes].flags |= RESF_RELEASE;
				}
			}

		} /* end while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL) */

	}

	/* FIXME FIXME possible dangling if issue:
	 if bellow, might be part of the else block above
	 */

	strncpy (lsinfo.resTable[lsinfo.nRes].des, keyList[RKEY_DESCRIPTION].val, MAXRESDESLEN);
	if (lsinfo.resTable[lsinfo.nRes].interval > 0 && (lsinfo.resTable[lsinfo.nRes].valueType == LS_NUMERIC)) {
		lsinfo.numUsrIndx++;
		lsinfo.numIndx++;
		lsinfo.nRes++;
		nres++;
		freeKeyList (keyList);
	}
	else {
		ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "resource");
		return FALSE;
	}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "resource");
	return TRUE;
}

int resNameDefined (char *name) { // FIXME FIXME FIXME only INT_MAX nRes can be here.

	if (name == NULL) {
		return -1;
	}

	for (unsigned int i = 0; i < lsinfo.nRes; i++) {
		if (strcmp (name, lsinfo.resTable[i].name) == 0) {
			assert( i <= INT_MAX );
			return (int) i;
		}
	}

	return -1;
}

struct clusterConf *ls_readcluster_ex (char *filename, struct lsInfo *info, int lookupAdmins)
{
	struct lsInfo myinfo;
	char *word = NULL;
	FILE *fp = NULL;
	char *cp = NULL;
	size_t lineNum = 0;
	int Error  = FALSE;
	int aorm   = FALSE;
	int count1 = 0;
	int count2 = 0;
	int counter= 0;

	lserrno = LSE_NO_ERR;
	if ( filename  == NULL) {
		/* catgets 5050 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5050, "%s: filename is NULL")), __func__);
		lserrno = LSE_NO_FILE;
		return NULL;
	}

	if (info == NULL) {
		/* catgets 5100 */
		ls_syslog (LOG_ERR, (_i18n_msg_get(ls_catd, NL_SETN, 5100, "%s: LSF information is NULL")), __func__);
		lserrno = LSE_NO_FILE;
		return NULL;
	}


	if (cConf == NULL) {
		cConf = malloc (sizeof (struct clusterConf));
		if ( NULL == cConf && ENOMEM == errno )
			{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
			lserrno = LSE_MALLOC;
			return NULL;
			}
		cConf->clinfo = NULL;
		cConf->hosts = NULL;
		cConf->numHosts = 0;
		cConf->numShareRes = 0;
		cConf->shareRes = NULL;
	}
	else
		{
		for ( unsigned int i = 0; i < cConf->numHosts; i++)  {
			freeHostInfo (&cConf->hosts[i]);
		}

		FREEUP (cConf->hosts);
		cConf->numHosts = 0;
		for ( int i = 0; i < cConf->numShareRes; i++) // FIXME FIXME FIXME can cConf->numShareRes be unsigned int?
			{
			FREEUP (cConf->shareRes[i].resourceName);
			for ( unsigned int j = 0; j < cConf->shareRes[i].nInstances; j++)
				{
				FREEUP (cConf->shareRes[i].instances[j].value);
				for ( unsigned int k = 0; k < cConf->shareRes[i].instances[j].nHosts; k++)
					{
					FREEUP (cConf->shareRes[i].instances[j].hostList[k]);
					}
				FREEUP (cConf->shareRes[i].instances[j].hostList);
				}
			FREEUP (cConf->shareRes[i].instances);
			}

		FREEUP (cConf->shareRes);
		cConf->shareRes = NULL;
		cConf->numShareRes = 0;
		}

	freeClusterInfo (&clinfo);
	initClusterInfo (&clinfo);
	cConf->clinfo = &clinfo;
	count1 = 0;
	count2 = 0;

	myinfo = *info;
	assert( info->nRes > 0 );
	myinfo.resTable = malloc( info->nRes * sizeof (struct resItem));

	if (info->nRes && (myinfo.resTable == NULL) ) {
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		lserrno = LSE_MALLOC;
		return NULL;
	}

	counter = 0;
	for ( unsigned int i = 0; i < info->nRes; i++)
		{
		if (info->resTable[i].flags & RESF_DYNAMIC) {
			memcpy (&myinfo.resTable[counter], &info->resTable[i], sizeof (struct resItem));
			counter++;
		}
		}

	for ( unsigned int i = 0; i < info->nRes; i++)
		{
		if (!(info->resTable[i].flags & RESF_DYNAMIC)) {
			memcpy (&myinfo.resTable[counter], &info->resTable[i], sizeof (struct resItem));
			counter++;
		}
		}

	if ((fp = fopen (filename, "r")) == NULL) {
		FREEUP (myinfo.resTable);

		ls_syslog (LOG_INFO, I18N_FUNC_S_FAIL, filename, "fopen", __func__);
		lserrno = LSE_NO_FILE;
		return NULL;
	}

	for (;;) {

		cp = getBeginLine (fp, &lineNum);
		if (!cp) {

			fclose (fp);
			if (cConf->numHosts) {

				FREEUP (myinfo.resTable);
				if (Error) {
					return NULL;
				}
				else {
					return cConf;
				}
			}
		}
		else {

			FREEUP (myinfo.resTable);
			/* catgets 5104 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5104, "%s: %s(%d): No hosts configured.")), __func__, filename, lineNum);
			return cConf;
		}

		word = getNextWord_ (&cp);
		if (!word)
			{
			/* catgets 5105 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5105, "%s: %s(%d): Keyword expected after Begin. Ignoring section")), __func__, filename, lineNum);
			doSkipSection (fp, &lineNum, filename, "unknown");
			}
		else if (strcasecmp (word, "clustermanager") == 0)
			{
			count1++;
			if (count1 > 1)
				{
				/* catgets 5106 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5106, "%s: %s(%d): More than one %s section defined; ignored.")),  __func__, filename, lineNum, word);
				doSkipSection (fp, &lineNum, filename, word);
				}
			else
				{
				if (!do_Manager(fp, filename, &lineNum, "clustermanager", lookupAdmins) && aorm != TRUE)
					{
					Error = TRUE;
					}
				else
					aorm = TRUE;
				}
			continue;
			}
		else if (strcasecmp (word, "clusteradmins") == 0)
			{
			count2++;
			if (count2 > 1)
				{
				/* catgets 5107 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5107, "%s: %s(%d): More than one %s section defined; ignored.")), __func__, filename, lineNum, word);
				doSkipSection (fp, &lineNum, filename, word);
				}
			else
				{
				if (!do_Manager(fp, filename, &lineNum, "clusteradmins", lookupAdmins)
					&& aorm != TRUE)
					{
					Error = TRUE;
					}
				else
					aorm = TRUE;
				}
			continue;
			}
		else if (strcasecmp (word, "parameters") == 0)
			{
			if (!do_Clparams (fp, filename, &lineNum))
				Error = TRUE;
			continue;
			}
		else if (strcasecmp (word, "host") == 0)
			{
			if (!do_Hosts (fp, filename, &lineNum, &myinfo))
				Error = TRUE;
			continue;
			}
		else if (strcasecmp (word, "resourceMap") == 0)
			{
			if (doResourceMap (fp, filename, &lineNum) < 0)
				Error = TRUE;
			continue;
			}
		else
			{
			/* catgets 5108 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5108, "%s %s(%d): Invalid section name %s, ignoring section")), __func__, filename, lineNum, word);
			doSkipSection (fp, &lineNum, filename, word);
			}
	}
}

struct clusterConf *ls_readcluster (char *filename, struct lsInfo *info)
{
	return ls_readcluster_ex (filename, info, TRUE);
}

void freeClusterInfo (struct clusterInfo *cls)
{

	if (cls != NULL) {
		for ( unsigned int i = 0; i < cls->nRes; i++) {
			FREEUP (cls->resources[i]);
		}
		for ( unsigned int i = 0; i < cls->nTypes; i++) {
			FREEUP (cls->hostTypes[i]);
		}
		for ( unsigned int i = 0; i < cls->nModels; i++) {
			FREEUP (cls->hostModels[i]);
		}
		for ( unsigned int i = 0; i < cls->nAdmins; i++) {
			FREEUP (cls->admins[i]);
		}

		FREEUP (cls->admins);
		FREEUP (cls->adminIds);
	}
}

void initClusterInfo (struct clusterInfo *cls)
{
	if (cls != NULL)
		{
		strcpy (cls->clusterName, "");
		cls->status = 0;
		strcpy (cls->masterName, "");
		strcpy (cls->managerName, "");
		cls->managerId = 0;
		cls->numServers = 0;
		cls->numClients = 0;
		cls->nRes = 0;
		cls->resources = NULL;
		cls->nTypes = 0;
		cls->hostTypes = NULL;
		cls->nModels = 0;
		cls->hostModels = NULL;
		cls->nAdmins = 0;
		cls->adminIds = NULL;
		cls->admins = NULL;
		}
}


void freeHostInfo (struct hostInfo *host)
{
	if (host != NULL) {
		FREEUP (host->hostType);
		FREEUP (host->hostModel);
		for ( unsigned int i = 0; i < host->nRes; i++) {
			FREEUP (host->resources[i]);
		}
		FREEUP (host->resources);
		FREEUP (host->windows);
		FREEUP (host->busyThreshold);
	}
}


void initHostInfo (struct hostInfo *host)
{
	if (host != NULL)
		{
		strcpy (host->hostName, "");
		host->hostType = NULL;
		host->hostModel = NULL;
		host->cpuFactor = 0;
		host->maxCpus = 0;
		host->maxMem = 0;
		host->maxSwap = 0;
		host->maxTmp = 0;
		host->nDisks = 0;
		host->nRes = 0;
		host->resources = NULL;
		host->windows = NULL;
		host->numIndx = 0;
		host->busyThreshold = NULL;
		host->isServer = 0;
		host->rexPriority = 0;
		}
}

char do_Manager (FILE * fp, char *filename, size_t *lineNum, char *secName, int lookupAdmins)
{
	char *linep = NULL;
	struct keymap keyList1[] = {
		{0, "    " , NULL, "MANAGER"},
		{0, "    " , NULL, NULL}
	};
	struct keymap keyList2[] = {
		{0, "    " , NULL, "ADMINISTRATORS"},
		{0, "    " , NULL, NULL}
	};
	struct keymap *keyList;

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep) {
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, secName);
		return FALSE;
	}

	if (isSectionEnd (linep, filename, lineNum, secName)) {
		return FALSE;
	}

	if (strcmp (secName, "clustermanager") == 0) {
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
			if (getClusAdmins(keyList[0].val, filename, lineNum, secName, lookupAdmins) < 0)
				{
				FREEUP (keyList[0].val);
				return FALSE;
				}
			else
				{
				FREEUP (keyList[0].val);
				return TRUE;
				}
			}
		}
	else
		{
		if (readHvalues (keyList, linep, fp, filename, lineNum, TRUE, secName) < 0) {
			return FALSE;
		}
		if (getClusAdmins
			(keyList[0].val, filename, lineNum, secName, lookupAdmins) < 0)
			{
			FREEUP (keyList[0].val);
			return FALSE;
			}
		else
			{
			FREEUP (keyList[0].val);
			return TRUE;
			}
		}
	return TRUE;
}

int getClusAdmins (char *line, char *filename, size_t *lineNum, char *secName, int lookupAdmins)
{
	struct admins *admins;
	static char lastSecName[40];

	admins = liblsf_getAdmins( line, filename, lineNum, secName, lookupAdmins );
	if (admins->nAdmins <= 0)
		{
		/* catgets 5118 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5118, "%s: %s(%d): No valid user for section %s: %s")), __func__, filename, *lineNum, secName, line);
		return -1;
		}
	if (strcmp (secName, "clustermanager") == 0 &&
		strcmp (lastSecName, "clusteradmins") == 0)
		{
		strcpy (lastSecName, "");
		if (setAdmins (admins, A_THEN_M) < 0)
			return -1;
		}
	else if (strcmp (lastSecName, "clustermanager") == 0 &&
			 strcmp (secName, "clusteradmins") == 0)
		{
		strcpy (lastSecName, "");
		if (setAdmins (admins, M_THEN_A) < 0)
			return -1;
		}
	else
		{
		if (setAdmins (admins, M_OR_A) < 0)
			return -1;
		}
	strcpy (lastSecName, secName);
	return 0;
}

struct admins *liblsf_getAdmins (char *line, char *filename, size_t *lineNum, char *secName, int lookupAdmins)
{

	static int first = TRUE;
	static struct admins admins;

	unsigned int numAds = 0;
	char *sp = NULL, *word = NULL;
	struct passwd *pw = NULL;
	struct group *unixGrp = NULL;

	assert( filename != NULL );    /* FIXME these three asserts are here so the compiler will not moan */
	assert( lineNum != NULL );  /*  Deeper investgation is needed as to where are these three vars used in */
	assert( secName != NULL );


	/* FIXME FIXME
	 this 'first' deal must go. i suspect that the reason it exists is to limit
	 contention between re-reading. or some sort of SIGHUP
	 */
	if (first == FALSE) {

		for ( unsigned int i = 0; i < admins.nAdmins; i++) {
			FREEUP (admins.adminNames[i]);
		}
		FREEUP (admins.adminNames);

		FREEUP (admins.adminIds);
		FREEUP (admins.adminGIds);
	}

	first = FALSE;
	admins.nAdmins = 0;
	sp = line;
	while ((word = getNextWord_ (&sp)) != NULL) {
		numAds++;
	}
	if (numAds)
		{
		admins.adminIds   = malloc( numAds * sizeof (uid_t)  );
		admins.adminGIds  = malloc( numAds * sizeof (gid_t)  );
		admins.adminNames = malloc( numAds * sizeof (char *) );
		if (admins.adminIds == NULL || admins.adminGIds == NULL || admins.adminNames == NULL)
			{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "getAdmins", "malloc");
			FREEUP (admins.adminIds);
			FREEUP (admins.adminGIds);
			FREEUP (admins.adminNames);
			admins.nAdmins = 0;
			lserrno = LSE_MALLOC;
			return &admins;
			}
		}
	else {
		return &admins;
	}

	sp = line;
	while ((word = getNextWord_ (&sp)) != NULL) {
		char *forWhat = "for LSF administrator";

		if (lookupAdmins) {
			if ((pw = getpwlsfuser_ (word)) != NULL) {
				if (putInLists (word, &admins, &numAds, forWhat) < 0) {
					return &admins;
				}
			}
			else if ((unixGrp = getgrnam (word)) != NULL) {
				int i = 0;
				while (unixGrp->gr_mem[i] != NULL) {
					if( putInLists(unixGrp->gr_mem[i++], &admins, &numAds, forWhat) < 0) {
						return &admins;
					}
				}
			}
			else {
				if (putInLists (word, &admins, &numAds, forWhat) < 0) {
					return &admins;
				}
			}
		}
		else {
			if (putInLists (word, &admins, &numAds, forWhat) < 0) {
				return &admins;
			}
		}
	}

	return &admins;
}

int
setAdmins (struct admins *admins, int mOrA)
{
	uid_t workNAdmins     = 0;
	uid_t tempNAdmins     = 0;
	uid_t *tempAdminIds   = NULL;
	uid_t *workAdminIds   = NULL;
	char **tempAdminNames = NULL;
	char **workAdminNames = NULL;

	tempNAdmins = admins->nAdmins + clinfo.nAdmins;
	if( tempNAdmins ) {
		tempAdminIds   =  malloc ( tempNAdmins * sizeof ( tempAdminIds ));
		tempAdminNames =  malloc ( tempNAdmins * sizeof ( *tempAdminNames ) + 1 ); // FIXME FIXME FIXME FIXME FIXME certain malloc failure
	}
	else {
		tempAdminIds = NULL;
		tempAdminNames = NULL;
	}

	if (!tempAdminIds || !tempAdminNames) {
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		FREEUP (tempAdminIds);
		FREEUP (tempAdminNames);
		return -1;
	}

	if (mOrA == M_THEN_A) {
		workNAdmins = clinfo.nAdmins;
		workAdminIds = clinfo.adminIds;
		workAdminNames = clinfo.admins;
	}
	else {
		workNAdmins = admins->nAdmins;
		workAdminIds = admins->adminIds;
		workAdminNames = admins->adminNames;
	}

	for ( unsigned int i = 0; i < workNAdmins; i++) {

		tempAdminIds[i] = workAdminIds[i];
		if ((tempAdminNames[i] = putstr_ (workAdminNames[i])) == NULL)
			{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");

			for ( unsigned int k = 0; k < i; k++) {
				FREEUP (tempAdminNames[k]);
			}

			FREEUP (tempAdminIds);
			FREEUP (tempAdminNames);
			return -1;
			}
	}

	tempNAdmins = workNAdmins;
	if (mOrA == M_THEN_A)
		{
		workNAdmins = admins->nAdmins;
		workAdminIds = admins->adminIds;
		workAdminNames = admins->adminNames;
		}
	else if (mOrA == A_THEN_M) {
		workNAdmins = clinfo.nAdmins;
		workAdminIds = clinfo.adminIds;
		workAdminNames = clinfo.admins;
	}
	else {
		workNAdmins = 0;
	}


	for (unsigned i = 0; i < workNAdmins; i++)
		{
		if (isInlist (tempAdminNames, workAdminNames[i], tempNAdmins)) {
			continue;
		}

		tempAdminIds[tempNAdmins] = workAdminIds[i];
		if ((tempAdminNames[tempNAdmins] = putstr_ (workAdminNames[i])) == NULL)
			{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
			for ( unsigned int k = 0; k < tempNAdmins; k++) {
				FREEUP (tempAdminNames[k]);
			}

			FREEUP (tempAdminIds);
			FREEUP (tempAdminNames);
			return -1;
			}

		tempNAdmins++;
		}
	if (clinfo.nAdmins > 0)
		{
		for ( unsigned int i = 0; i < clinfo.nAdmins; i++) {
			FREEUP (clinfo.admins[i]);
		}
		FREEUP (clinfo.adminIds);
		FREEUP (clinfo.admins);
		}

	clinfo.nAdmins = tempNAdmins;
	clinfo.adminIds = tempAdminIds;
	clinfo.admins = tempAdminNames;

	return 0;
}

char
do_Hosts (FILE * fp, char *filename, size_t *lineNum, struct lsInfo *info)
{
	char *sp        = NULL;
	char *word      = NULL;
	char **resList  = NULL;
	char *linep     = NULL;
	int ignoreR     = FALSE;
	unsigned int n = 0 ;/*FIXME FIXME
						 * WARNING DANGER WARNING SHITTY CODE
						 * n is shared among many blocks of code.
						 * MUST MUST MUST disentagle
						 */

	unsigned int numAllocatedResources = 0;
	struct hostInfo host;
	static struct keymap *keyList = NULL;


	/* FIXME FIXME HOSTNAME_ was supposed to be HOSTNAME, mixes with HOSTNAME FROM lsftcl.h */
	/* FIXME FIXME the following shit should have been better organised in the assignment
	 as static char doResources was fixed
	 */
	assert( info->numIndx > 0 );
	const unsigned int HOSTNAME_ = info->numIndx;
	const unsigned int MODEL     = info->numIndx + 1;
	const unsigned int TYPE      = info->numIndx + 2;
	const unsigned int ND        = info->numIndx + 3;
	const unsigned int RESOURCES = info->numIndx + 4;
	const unsigned int RUNWINDOW = info->numIndx + 5;
	const unsigned int REXPRI0   = info->numIndx + 6;
	const unsigned int SERVER0   = info->numIndx + 7;
	const unsigned int R         = info->numIndx + 8;
	const unsigned int S         = info->numIndx + 9;
	const unsigned short NUM_ALLOCATED_RESOURCES = 64;

	FREEUP (keyList);

	/* FIXME FIXME
	 this ... 11 ... is peculiar. Why 11? must investigate deeper
	 */

	keyList = malloc( ( info->numIndx + 11 ) * sizeof( struct keymap ) ); // FIXME FIXME + 11 ?

	if ( NULL == keyList && ENOMEM == errno ) {
		return FALSE;
	}

	initkeylist (keyList, HOSTNAME, (int) S + 1, info); // FIXME FIXME is cast supposed to be here?
	keyList[HOSTNAME_].key      = "HOSTNAME";
	keyList[MODEL].key          = "MODEL";
	keyList[TYPE].key           = "TYPE";
	keyList[ND].key             = "ND";
	keyList[RESOURCES].key      = "RESOURCES";
	keyList[RUNWINDOW].key      = "RUNWINDOW";
	keyList[REXPRI0].key        = "REXPRI";
	keyList[SERVER0].key        = "SERVER";
	keyList[R].key              = "R";
	keyList[S].key              = "S";
	keyList[S + 1].key          = NULL;

	initHostInfo (&host);

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep) {
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "host");
		return FALSE;
	}

	if (isSectionEnd (linep, filename, lineNum, "host")) {
		/* catgets 5135 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5135, "%s: %s(%d): empty host section")), __func__, filename, *lineNum);
		return FALSE;
	}

	if (strchr (linep, '=') == NULL) {

		if (!keyMatch (keyList, linep, FALSE))
			{
			/* catgets 5136 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5136, "%s: %s(%d): keyword line format error for section host, ignoring section")), __func__, filename, *lineNum);
			doSkipSection (fp, lineNum, filename, "host");
			return FALSE;
			}


		for( int i = 0; keyList[i].key != NULL; i++) {
			if (keyList[i].position != -1) {
				continue;
			}

			if( ( strcasecmp ( "hostname", keyList[i].key ) == 0 ) || ( strcasecmp( "model", keyList[i].key) == 0 ) ||
			   ( strcasecmp ( "type", keyList[i].key ) == 0 ) || ( strcasecmp( "resources", keyList[i].key) == 0 ) ) {
				/* catgets 5137 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5137, "%s: %s(%d): keyword line: key %s is missing in section host, ignoring section")), __func__, filename, *lineNum, keyList[i].key);
				doSkipSection (fp, lineNum, filename, "host");

				for( int j = 0; keyList[j].key != NULL; j++) {
					if (keyList[j].position != -1) {
						FREEUP (keyList[j].val);
					}
				}

				return FALSE;
			}
		}
	}

	if (keyList[R].position != -1 && keyList[SERVER0].position != -1) {
		/* catgets 5138 */
		ls_syslog (LOG_WARNING, (_i18n_msg_get (ls_catd, NL_SETN, 5138, "%s: %s(%d): keyword line: conflicting keyword definition: you cannot define both 'R' and 'SERVER'. 'R' ignored")), __func__, filename, *lineNum);
		ignoreR = TRUE;
	}

	while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)  {

		freekeyval (keyList);
		initHostInfo (&host);

		if (isSectionEnd (linep, filename, lineNum, "host")) {
			return TRUE;
		}

		if (mapValues (keyList, linep) < 0)
			{
			/* catgets 5139  */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5139, "%s: %s(%d): values do not match keys for section host, ignoring line")), __func__, filename, *lineNum);
			continue;
			}

		if (strlen (keyList[HOSTNAME].val) > MAXHOSTNAMELEN) {
			/* catgets 5140 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5140, "%s: %s(%d): too long host name, ignored.")), __func__, filename, *lineNum);
			continue;
		}

		if (Gethostbyname_ (keyList[HOSTNAME].val) == NULL) {
			/* catgets 5141 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5141, "%s: %s(%d): Invalid hostname %s in section host. Ignoring host")), __func__, filename, *lineNum, keyList[HOSTNAME].val);
			continue;
		}

		strcpy (host.hostName, keyList[HOSTNAME].val);

		if ((host.hostModel = putstr_ (keyList[MODEL].val)) == NULL) {
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
			lserrno = LSE_MALLOC;
			freeHostInfo (&host);
			freekeyval (keyList);
			doSkipSection (fp, lineNum, filename, "host");
			return FALSE;
		}

		if ((host.hostType = putstr_ (keyList[TYPE].val)) == NULL) {
			ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc");
			lserrno = LSE_MALLOC;
			freeHostInfo (&host);
			freekeyval (keyList);
			doSkipSection (fp, lineNum, filename, "host");
			return FALSE;
		}

		if (keyList[ND].position != -1) {
			assert( atoi(keyList[ND].val) >= 0);
			host.nDisks = atoi( keyList[ND].val );
		}
		else {
			host.nDisks = INFINIT_INT;
		}

		host.busyThreshold = malloc( info->numIndx * sizeof ( host.busyThreshold ) );
		if ( NULL == host.busyThreshold && ENOMEM == errno ) {

			/* FIXME
			 this lserrno and logging to syslog about memory allocation must either
			 go or get a lot more serious somehow
			 */
			assert( info->numIndx );
			ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc");
			lserrno = LSE_MALLOC;
			freeHostInfo (&host);
			freekeyval (keyList);
			doSkipSection (fp, lineNum, filename, "host");

			return FALSE;
		}

		liblsf_putThreshold( R15S, &host, keyList[R15S].position, keyList[R15S].val, INFINIT_LOAD );
		liblsf_putThreshold( R1M,  &host, keyList[R1M].position,  keyList[R1M].val,  INFINIT_LOAD );
		liblsf_putThreshold( R15M, &host, keyList[R15M].position, keyList[R15M].val, INFINIT_LOAD );
		liblsf_putThreshold( UT,   &host, keyList[UT].position,   keyList[UT].val,   INFINIT_LOAD );

		if (host.busyThreshold[UT] > 1.0 && host.busyThreshold[UT] < INFINIT_LOAD) {
			ls_syslog (LOG_INFO, (_i18n_msg_get(ls_catd, NL_SETN, 5145, "%s: %s(%d): value for threshold ut <%2.2f> is greater than 1, assumming <%5.1f%%>")), __func__, filename, *lineNum, host.busyThreshold[UT], host.busyThreshold[UT]);
			host.busyThreshold[UT] /= 100.0;
		}
		liblsf_putThreshold( PG,  &host, keyList[PG].position,  keyList[PG].val,   INFINIT_LOAD );
		liblsf_putThreshold( IO,  &host, keyList[IO].position,  keyList[IO].val,   INFINIT_LOAD );
		liblsf_putThreshold( LS,  &host, keyList[LS].position,  keyList[LS].val,   INFINIT_LOAD );
		liblsf_putThreshold( IT,  &host, keyList[IT].position,  keyList[IT].val,  -INFINIT_LOAD );
		liblsf_putThreshold( TMP, &host, keyList[TMP].position, keyList[TMP].val, -INFINIT_LOAD );
		liblsf_putThreshold( SWP, &host, keyList[SWP].position, keyList[SWP].val, -INFINIT_LOAD );
		liblsf_putThreshold( MEM, &host, keyList[MEM].position, keyList[MEM].val, -INFINIT_LOAD );

		for (unsigned int i = NBUILTINDEX; i < NBUILTINDEX + info->numUsrIndx; i++) {
			if (info->resTable[i].orderType == INCR) {
				assert( i <= INT_MAX );
				liblsf_putThreshold ((int)i, &host, keyList[i].position, keyList[i].val, INFINIT_LOAD);
			}
			else {
				assert( i <= INT_MAX );
				liblsf_putThreshold ((int)i, &host, keyList[i].position, keyList[i].val, -INFINIT_LOAD);
			}
		}

		for (unsigned int i = NBUILTINDEX + info->numUsrIndx; i < info->numIndx; i++) {

			host.busyThreshold[i] = INFINIT_LOAD;
			host.numIndx = info->numIndx;
			numAllocatedResources = NUM_ALLOCATED_RESOURCES;
			resList = calloc (numAllocatedResources, sizeof( *resList ) );

			if (resList == NULL) {
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
			}

			n = 0;
			sp = keyList[RESOURCES].val;
			while ((word = getNextWord_ (&sp)) != NULL) {

				// FIXME FIXME FIXME
				// WARNING DANGER WARNING DANGER
				// (WAS A) SHITTY CODE ASSIGNS TO LOOP VARIABLE
				// FIXME FIXME FIXME
				// MUST INVESTIGATE WHY
				for ( unsigned int j = 0; j < n; j++) {
					if (!strcmp (word, resList[j])) {
						break;
					}
				}

				// casts of n to int must be thrown out, for all instanses in this function
				if (i < n) {
					/* catgets 5146 */
					ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5146, "%s: %s(%d): Resource <%s> multiply specified for host %s in section host. Ignored.")), __func__, filename, *lineNum, word, host.hostName);
					continue;
				}
				else {

					/* FIXME FIXME WTF IS THIS SHIT? why isn't this an else if() condition?!? */
					if (n >= numAllocatedResources) {

						numAllocatedResources *= 2;
						resList = realloc (resList, numAllocatedResources * (sizeof (char *)));

						if (resList == NULL) {
							lserrno = LSE_MALLOC;
							ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
							freeHostInfo (&host);
							freekeyval (keyList);
							doSkipSection (fp, lineNum, filename, "host");
							return FALSE;
						}
					}

					if ((resList[n] = putstr_ (word)) == NULL) {

						ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
						lserrno = LSE_MALLOC;

						for ( int j = 0; j < (int) n; j++) {
							FREEUP (resList[j]);
						}
						FREEUP (resList);
						freeHostInfo (&host);
						freekeyval (keyList);
						doSkipSection (fp, lineNum, filename, "host");
						return FALSE;
					}

					n++; /* FIXME FIXME ... the fuck is this increment doing down here? why isn't it
						  * in the increment section of the for loop?
						  */

				} // end f (i < n)/else

			} // end while ((word = getNextWord_ (&sp)) != NULL)

		} // end for (int i = NBUILTINDEX + info->numUsrIndx; i < info->numIndx; i++)

		resList[n] = NULL;

		/* FIXME
		 * the hostInfo struct needs a bit of looking over. The following code example
		 *      host.nRest = n;
		 * n lives at the top of this function (ffs). It used to be int, i changed it
		 * to unsigned int and kept any assert( n >= 0 ) laying around; redundant, yes.
		 *
		 * hostInfo.nRest is of type int. n is of type unsigned int.
		 *
		 * the casted assignment below is legal, and will not create any issues, but
		 * this looks like a bigger problem with the code base as there are a number
		 * of assignments here needing a cast for no good reason.
		 *
		 * when coming back to this piece of code the investigation
		 *      take a look at the following clues
		 *
		 * 1. what are the possible value ranges for the hostInfo fields?
		 *      if >= 0, then please redeclare them all as unsigned int.
		 *          insert assert()s for upper limits
		 *
		 * 2. why is n shared among multiple code blocks? can this code be refactored
		 *      withtout breaking it.
		 *
		 *  In order to proceed with the above, function tests must be implemented.
		 *
		 *  which means, we just bumped the tests up in priority before coding
		 *  anything more. the editing in this current run must be kept to a cool minimum
		 *      - gmarselis
		 *
		 */
		host.nRes = n;

		host.resources = malloc( n * sizeof( host.resources ) + 1 );
		if ( ( NULL == host.resources) && ENOMEM == errno ) {

			assert( n );

			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
			lserrno = LSE_MALLOC;

			for ( int j = 0; j < (int) n; j++) {
				FREEUP (resList[j]);
			}

			FREEUP (resList);
			freeHostInfo (&host);
			freekeyval (keyList);
			doSkipSection (fp, lineNum, filename, "host");
			return FALSE;
		}

		for( int i = 0; i < (int) n; i++) {
			if ((host.resources[i] = putstr_ (resList[i])) == NULL) {

				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
				lserrno = LSE_MALLOC;

				for ( int j = 0; j < (int) n; j++) {
					FREEUP (resList[j]);
				}

				FREEUP (resList);
				freeHostInfo (&host);
				freekeyval (keyList);
				doSkipSection (fp, lineNum, filename, "host");
				return FALSE;
			}
		}

		for ( int j = 0; j < (int) n; j++) {
			FREEUP (resList[j]);
		}
		FREEUP (resList);

		host.rexPriority = DEF_REXPRIORITY;
		if (keyList[REXPRI0].position != -1) {
			host.rexPriority = atoi (keyList[REXPRI0].val);
		}

		host.isServer = 1;
		if (keyList[R].position != -1) {
			if (!ignoreR) {     // FIXME shitty code. isServer and val should be the same type
				host.isServer = (char) atoi (keyList[R].val);
			}
		}

		if (keyList[SERVER0].position != -1) { // FIXME shitty code. isServer and val should be the same type
			host.isServer = (char) atoi (keyList[SERVER0].val); // FIXME FIXME is cast here justified?
		}

		host.windows = NULL;
		if (keyList[RUNWINDOW].position != -1) {

			if (strcmp (keyList[RUNWINDOW].val, "") == 0) {
				host.windows = NULL;
			}
			else {
				host.windows = parsewindow (keyList[RUNWINDOW].val, filename, lineNum, "Host");

				if (host.windows == NULL) {
					ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
					lserrno = LSE_MALLOC;
					freeHostInfo (&host);
					freekeyval (keyList);
					doSkipSection (fp, lineNum, filename, "host");
					return FALSE;
				}
			}
		}

		addHost (&host, filename, lineNum);
	} // end while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)

	// FIXME FIXME FIXME
	//      this chunk came out of somewhere. after manual indentation and adding of brackets,
	//      it could nto stuck anywhere. find out where this is supposed to go.
	/*    else
	 {
	 ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "host");
	 doSkipSection (fp, lineNum, __func__, "host");
	 return FALSE;
	 }*/

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "host");
	return TRUE;
}

void
liblsf_putThreshold (int indx, struct hostInfo *host, long position, char *val, float def)
{
	if ( NULL == host ) {
		return;
	}

	if (position != -1) {
		if (strcmp (val, "") == 0) {
			host->busyThreshold[indx] = def;
		}
		else {
			/* FIXME FIXME
			 *      must investigate upper and lower limits of host->busyThreshold and set the
			 *      type appropriately
			 */
			host->busyThreshold[indx] = (float) atof (val);
		}
	}
	else {
		host->busyThreshold[indx] = def;
	}

	return;

}

char addHost (struct hostInfo *host, char *filename, size_t *lineNum)
{
	struct hostInfo *newlist;

	if (NULL == host ) {
		return FALSE;
	}

	for ( unsigned int i = 0; i < cConf->numHosts; i++) {

		if (! equalHost_(cConf->hosts[i].hostName, host->hostName) ) {
			continue;
		}

		/* catgets 5163 */
		ls_syslog (LOG_WARNING, (_i18n_msg_get (ls_catd, NL_SETN, 5163, "%s: %s(%d): host <%s> redefined, using previous definition")), __func__, filename, *lineNum, host->hostName);
		freeHostInfo (host);

		return FALSE;
	}

	cConf->numHosts++;

	newlist = malloc( cConf->numHosts * sizeof( struct hostInfo ) );
	if ( ( NULL == newlist ) && ENOMEM == errno ) {

		assert( cConf->numHosts );  // FIXME FIXME if numHosts is always >= 0
									//      change numHosts type to unsigned long
									//      remove assertion
									//      remove cast
									// search for more shit like that

		// FIXME FIXME FIXME
		//      ALL CASTS WITHIN THIS FILE SHOULD BE TREATED WITH SUSPICION
		//      DELETE CASTS, RECOMPILE AND ASSIGN PROPER TYPES TO EACH

		ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", cConf->numHosts * sizeof (struct hostInfo));
		cConf->numHosts--;
		lserrno = LSE_MALLOC;
		return FALSE;
	}

	for (unsigned int i = 0; i < cConf->numHosts - 1; i++) {
		newlist[i] = cConf->hosts[i];
	}

	newlist[cConf->numHosts - 1] = *host;
	FREEUP (cConf->hosts);
	cConf->hosts = newlist;

	return TRUE;
}

void initkeylist (struct keymap keyList[], int m, int n, struct lsInfo *info)
{

	for ( int i = 0; i < m - 1; i++) {
		keyList[i].key = "";
	}

	for ( int i = 0; i < n; i++) {
		keyList[i].val = NULL;
		keyList[i].position = 0;
	}

	if (NULL == info) {
		int i = 0;
		unsigned int index = 0;

		while ( NULL != builtInRes[i].name ) {

			if (builtInRes[i].flags & RESF_DYNAMIC) {
				keyList[index++].key = builtInRes[i].name;
			}
			i++;
		}
	}
	else
		{
		unsigned int index = 0;
		for (unsigned int i = 0; i < info->nRes; i++) {
			if ((info->resTable[i].flags & RESF_DYNAMIC) && index < info->numIndx) {
				keyList[index++].key = info->resTable[i].name;
			}
		}
		}
}

void freekeyval (struct keymap keylist[])
{
	for ( int cc = 0; NULL != keylist[cc].key; cc++) {
		if ( NULL != keylist[cc].val ) {
			FREEUP (keylist[cc].val);
		}
	}
}

char *parsewindow (char *linep, char *filename, size_t *lineNum, char *section)
{
	char *sp, *windows, *word, *save;

	if (linep == NULL){
		return NULL;
	}

	sp = linep;

	windows = putstr_ (sp);
	if (windows == NULL) {
		return NULL;
	}

	*windows = '\0';
	while ((word = getNextWord_ (&sp)) != NULL)
		{
		save = putstr_ (word);
		if (save == NULL)
			{
			FREEUP (windows);
			return NULL;
			}
		if (validWindow (word, section) < 0)
			{
			/* catgets 5165 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5165, "File %s section %s at line %d: Bad time expression <%s>; ignored.")), filename, section, *lineNum, save);
			lserrno = LSE_CONF_SYNTAX;
			FREEUP (save);
			continue;
			}
		if (*windows != '\0')
			strcat (windows, " ");
		strcat (windows, save);
		FREEUP (save);
		}

	if (windows[0] == '\0')
		{
		FREEUP (windows);
		}
	return windows;

}

int validWindow (char *wordpair, char *context)
{
	int oday, cday;
	float ohour, chour;
	char *sp;
	char *word;

	sp = strchr (wordpair, '-');
	if (!sp)
		{
		/* catgets 5166 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5166, "Bad time expression in %s")), context);
		return -1;
		}

	*sp = '\0';
	sp++;
	word = sp;

	if (parse_time (word, &chour, &cday) < 0)
		{
		ls_syslog (LOG_ERR,(_i18n_msg_get(ls_catd, NL_SETN, 5166, "Bad time expression in %s")), context);
		return -1;
		}

	word = wordpair;

	if (parse_time (word, &ohour, &oday) < 0)
		{
		ls_syslog (LOG_ERR, (_i18n_msg_get(ls_catd, NL_SETN, 5166, "Bad time expression in %s")), context);
		return -1;
		}

	if (((oday && cday) == 0) && (oday != cday))
		{
		/* catgets 5169 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5169, "Ambiguous time in %s")), context);
		return -1;
		}

	return 0;

}

/*
 FIXME FIXME FIXME
 Why on earth is this function parsing time?
 there are standard functions to do that with
 */
int parse_time (char *word, float *hour, int *day)
{

	float min = 0.0;
	char *sp;

	/*
	 FIXME FIXME FIXME wat? assignment to 0?
	 */
	*hour = 0.0;
	*day = 0;


	sp = strrchr (word, ':');
	if (!sp) {

		if (!isint_ (word) || atoi (word) < 0) {
			return -1;
		}

		*hour = (float) atof (word);
		if (*hour > 23)

			return -1;

	}
	else {
		*sp = '\0';
		sp++;

		if (!isint_ (sp) || atoi (sp) < 0) {
			return -1;
		}

		min = atoi (sp);

		if (min > 59) {
			return -1;
		}

		sp = strrchr (word, ':');

		if (!sp) {

			if (!isint_ (word) || atoi (word) < 0) {
				return -1;
			}

			*hour = (float) atof (word);

			if (*hour > 23) {
				return -1;
			}
		}
		else {

			*sp = '\0';
			sp++;

			if (!isint_ (sp) || atoi (sp) < 0) {
				return -1;
			}

			*hour = (float) atof (sp);

			if (*hour > 23) {
				return -1;
			}

			if (!isint_ (word) || atoi (word) < 0){
				return -1;
			}

			*day = atoi (word);
			if (*day == 0) {
				*day = 7;
			}

			if (*day < 1 || *day > 7) {
				return -1;
			}
		}
	}

	*hour += min / 60.0;

	return 0;
}

char do_Cluster (FILE * fp, size_t *lineNum, char *filename)
{
	char *linep;
	struct keymap keyList[] = {
		{ 0, "    ", "CLUSTERNAME", NULL },
		{ 0, "    ", "SERVERS",     NULL },
		{ 0, "    ", NULL,          NULL }
	};
	char *servers;
	bool_t found = FALSE;

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep)
		{
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "cluster");
		return FALSE;
		}

	if (isSectionEnd (linep, filename, lineNum, "cluster"))
		return FALSE;

	if (strchr (linep, '=') == NULL)
		{
		if (!keyMatch (keyList, linep, FALSE))
			{
			/* catgets 5171 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5171, "%s: %s(%d): keyword line format error for section cluster; ignoring section")), __func__, filename, *lineNum);
			doSkipSection (fp, lineNum, filename, "cluster");
			return FALSE;
			}

		if (keyList[0].position == -1)
			{
			 /* catgets 5172 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5172, "%s: %s(%d): keyword line: key %s is missing in section cluster; ignoring section")), __func__, filename, *lineNum, keyList[0].key);
			doSkipSection (fp, lineNum, filename, "cluster");
			return FALSE;
			}

		while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
			{
			if (isSectionEnd (linep, filename, lineNum, "cluster"))
				return TRUE;
			if (found)
				return TRUE;

			if (mapValues (keyList, linep) < 0)
				{
				 /* catgets 5173 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5173, "%s: %s(%d): values do not match keys for section cluster, ignoring line")), __func__, filename, *lineNum);
				continue;
				}

			if (keyList[1].position != -1)
				servers = keyList[1].val;
			else
				servers = NULL;

			if ((sConf->clusterName = putstr_ (keyList[0].val)) == NULL)
				{
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
				FREEUP (keyList[0].val);
				if (keyList[1].position != -1) {
					FREEUP (keyList[1].val);
				}
				return FALSE;
				}

			if ((sConf->servers = putstr_ (servers)) == NULL)
				{
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
				FREEUP (keyList[0].val);
				if (keyList[1].position != -1) {
					FREEUP (keyList[1].val);
				}
				return FALSE;
				}

			found = TRUE;
			FREEUP (keyList[0].val);
			if (keyList[1].position != -1) {
				FREEUP (keyList[1].val);
			}
		}
	}
	else
		{
		ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "cluster");
		doSkipSection (fp, lineNum, filename, "cluster");
		return FALSE;
		}

	ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, filename, *lineNum, "cluster");
	return FALSE;
}


#define EXINTERVAL              0
#define ELIMARGS                1
#define PROBE_TIMEOUT           2
#define ELIM_POLL_INTERVAL      3
#define HOST_INACTIVITY_LIMIT   4
#define MASTER_INACTIVITY_LIMIT 5
#define RETRY_LIMIT             6
#define ADJUST_DURATION         7
#define LSF_ELIM_DEBUG          8
#define LSF_ELIM_BLOCKTIME      9
#define LSF_ELIM_RESTARTS       10

char do_Clparams (FILE * clfp, char *lsfile, size_t *lineNum)
{
	char *linep;
	struct keymap keyList[] = {
		{ 0, "    ", "EXINTERVAL",              NULL },
		{ 0, "    ", "ELIMARGS",                NULL },
		{ 0, "    ", "PROBE_TIMEOUT",           NULL },
		{ 0, "    ", "ELIM_POLL_INTERVAL",      NULL },
		{ 0, "    ", "HOST_INACTIVITY_LIMIT",   NULL },
		{ 0, "    ", "MASTER_INACTIVITY_LIMIT", NULL },
		{ 0, "    ", "RETRY_LIMIT",             NULL },
		{ 0, "    ", "ADJUST_DURATION",         NULL },
		{ 0, "    ", "LSF_ELIM_DEBUG",          NULL },
		{ 0, "    ", "LSF_ELIM_BLOCKTIME",      NULL },
		{ 0, "    ", "LSF_ELIM_RESTARTS",       NULL },
		{ 0, "    ", NULL,                      NULL }
	};

	linep = getNextLineC_ (clfp, lineNum, TRUE);
	if (!linep) {
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, "do_Clparams", lsfile, *lineNum, "parameters");
		return FALSE;
	}

	if (isSectionEnd (linep, lsfile, lineNum, "parameters")) {
		return TRUE;
	}

	if (strchr (linep, '=') == NULL) {
		/* catgets 5195 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5195, "%s: %s(%d): vertical section not supported, ignoring section")), "do_Clparams", lsfile, *lineNum);
		doSkipSection (clfp, lineNum, lsfile, "parameters");
		return FALSE;
	}
	else {
		if (readHvalues (keyList, linep, clfp, lsfile, lineNum, FALSE, "parameters") < 0)  {
			return FALSE;
		}
		return TRUE;
	}
}

void freeKeyList (struct keymap *keyList)
{
	for ( int i = 0; keyList[i].key != NULL; i++) {
		if (keyList[i].position != -1) {
			FREEUP (keyList[i].val);
		}
	}
}


int validType (char *type)
{
	if (type == NULL)  {
		return -1;
	}

	if (!strcasecmp (type, "Boolean")) {
		return LS_BOOLEAN;
	}

	if (!strcasecmp (type, "String")) {
		return LS_STRING;
	}

	if (!strcasecmp (type, "Numeric")) {
		return LS_NUMERIC;
	}

	if (!strcmp (type, "!")) {
		return LS_EXTERNAL;
	}

	return -1;
}

#define RKEY_RESOURCE_NAME  0
#define RKEY_LOCATION    1

int doResourceMap (FILE * fp, char *lsfile, size_t *lineNum)
{

	char *linep;
	struct keymap keyList[] = {
		{ 0, "    ", "RESOURCENAME", NULL },
		{ 0, "    ", "LOCATION",     NULL },
		{ 0, "    ", NULL,           NULL }
	};
	int resNo = 0;

	linep = getNextLineC_ (fp, lineNum, TRUE);
	if (!linep) {
		ls_syslog (LOG_ERR, I18N_PREMATURE_EOF, __func__, lsfile, *lineNum, "resourceMap");
		return -1;
	}

	if (isSectionEnd (linep, lsfile, lineNum, "resourceMap")) {
		/* catgets 5109 */
		ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 5109, "%s: %s(%d): Empty resourceMap, no keywords or resources defined."), __func__, lsfile, *lineNum);
		return -1;
	}

	if (strchr (linep, '=') == NULL)
		{
		if (!keyMatch (keyList, linep, TRUE))
			{
			/* catgets 5197 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5197, "%s: %s(%d): keyword line format error for section resource, ignoring section")), __func__, lsfile, *lineNum);
			doSkipSection (fp, lineNum, lsfile, "resourceMap");
			return -1;
			}


		while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)
			{
			if (isSectionEnd (linep, lsfile, lineNum, "resourceMap")) {
				return 0;
			}
			if (mapValues (keyList, linep) < 0)
				{
				/* catgets 5198 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5198, "%s: %s(%d): values do not match keys for resourceMap section, ignoring line")), __func__, lsfile, *lineNum);
				continue;
				}

			if ((resNo = resNameDefined (keyList[RKEY_RESOURCE_NAME].val)) < 0)
				{
				ls_syslog (LOG_ERR, (_i18n_msg_get(ls_catd, NL_SETN, 5199, "%s: %s(%d): Resource name <%s> is  not defined; ignoring line")), __func__, lsfile, *lineNum, keyList[RKEY_RESOURCE_NAME].val);
				freeKeyList (keyList);
				continue;
				}
			if (keyList[RKEY_LOCATION].val != NULL && keyList[RKEY_LOCATION].val[0] != '\0')
			{

				if (strstr (keyList[RKEY_LOCATION].val, "all ") && strchr (keyList[RKEY_LOCATION].val, '~')) // FIXME FIXME "all " ? what's with the extra space
				{

					struct HostsArray array;
					int result = 0;

					array.size = 0;
					array.hosts = malloc( cConf->numHosts * sizeof( array.hosts ) ); // FIXME FIXME FIXME almost certain buffer overflow
					if (!array.hosts)
					{
						ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "doresourcemap", "malloc");
						freeKeyList (keyList);
						return -1;
					}
					for ( unsigned int cnt = 0; cnt < cConf->numHosts; cnt++)
					{
						array.hosts[array.size] =
						strdup (cConf->hosts[cnt].hostName);
						if (!array.hosts[array.size])
						{
							freeSA_ (array.hosts, array.size);
							ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, "doresourcemap", "malloc");
							freeKeyList (keyList);
							return -1;
						}
						array.size++;
					}

					result = convertNegNotation_( &( keyList[ RKEY_LOCATION ].val ), &array );
					if (result == 0)
					{
						/* catgets 5397 */
						ls_syslog (LOG_WARNING, I18N (5397, "%s: %s(%d): convertNegNotation_: all the hosts are to be excluded %s !"), __func__, lsfile, *lineNum, keyList[RKEY_LOCATION].val);
					}
					else if (result < 0)
					{
						/* catgets 5398 */
						ls_syslog (LOG_WARNING, I18N (5398, "%s: %s(%d): convertNegNotation_: Wrong syntax \'%s\'"), __func__, lsfile, *lineNum, keyList[RKEY_LOCATION].val);
					}
					freeSA_ (array.hosts, array.size);
					}

				if (liblsf_addResourceMap (keyList[RKEY_RESOURCE_NAME].val, keyList[RKEY_LOCATION].val, lsfile, *lineNum) < 0)
					{
					/* catgets 5200 */
					ls_syslog (LOG_ERR, I18N (5200, "%s: %s(%d): addResourceMap() failed for resource <%s>; ignoring line"), __func__, lsfile, *lineNum, keyList[RKEY_RESOURCE_NAME].val);
					freeKeyList (keyList);
					continue;
					}

				lsinfo.resTable[resNo].flags &= ~RESF_GLOBAL;
				lsinfo.resTable[resNo].flags |= RESF_SHARED;
				resNo = 0;
				}
			else
				{
				/* catgets 5201 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5201, "%s: %s(%d): No LOCATION specified for resource <%s>; ignoring the line")), __func__, lsfile, *lineNum, keyList[RKEY_RESOURCE_NAME].val);
				freeKeyList (keyList);
				continue;
				}
			freeKeyList (keyList);
			}
		}
	else
		{
		ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, lsfile, *lineNum, "resource");
		return -1;
		}
	return 0;

}


int liblsf_addResourceMap (char *resName, char *location, char *lsfile, size_t lineNum)
{
	struct lsSharedResourceInfo *resource;
	int i, j, numHosts = 0, first = TRUE, error;
	char **hosts = NULL, initValue[MAXFILENAMELEN], *sp, *cp, ssp, *instance;
	char externalResourceFlag[] = "!";
	char *tempHost;

	if (resName == NULL || location == NULL)
		{
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5203, "%s: %s(%d): Resource name <%s> location <%s>")), __func__, lsfile, lineNum, (resName ? resName : "NULL"), (location ? location : "NULL"));  /* catgets 5203 */
		return -1;
		}

	if (!strcmp (location, "!"))
		{
		initValue[0] = '\0';
		tempHost = (char *) externalResourceFlag; // FIXME FIXME FIXME is this cast justified?
		hosts = &tempHost;
		if ((resource = liblsf_addResource (resName, 1, hosts, initValue, lsfile, lineNum)) == NULL)
			{
			/* catgets 5209 */
			ls_syslog (LOG_ERR, I18N (5209, "%s: %s(%d): %s() failed; ignoring the instance <%s>"), __func__, lsfile, lineNum, "addResource", "!");
			return -1;
			}
		return 0;
		}

	resource = NULL;
	sp = location;

	i = 0;
	while (*sp != '\0')
		{
		if (*sp == '[') {
			i++;
		}
		else if (*sp == ']') {
			i--;
		}
		sp++;
		}
	sp = location;
	if (i != 0)
		{
		/* catgets 5204 */
		ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5204, "%s: %s(%d): number of '[' is not match that of ']' in <%s> for resource <%s>; ignoring")), __func__, lsfile, lineNum, location, resName);
		return -1;
		}

	while (sp != NULL && sp[0] != '\0')
		{
		for (j = 0; j < numHosts; j++) {
			FREEUP (hosts[j]);
		}
		FREEUP (hosts);
		numHosts = 0;
		error = FALSE;
		instance = sp;
		initValue[0] = '\0';
		while (*sp == ' ' && *sp != '\0') {
			sp++;
		}
		if (*sp == '\0') {
			if (first == TRUE) {
				return -1;
			}
			else {
				return 0;
			}
		}
		cp = sp;
		while (isalnum (*cp)) {
			cp++;
		}
		if (cp != sp)
			{
			ssp = cp[0];
			cp[0] = '\0';
			strcpy (initValue, sp);
			cp[0] = ssp;
			if (isspace (*cp)) {
				cp++;
			}
			if (*cp != '@') {
				error = TRUE;
			}
			sp = cp + 1;
			}
		if (isspace (*sp))
			sp++;

		if (*sp != '[' && *sp != '\0')
			{
			/* catgets 5205 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5205, "%s: %s(%d): Bad character <%c> in instance; ignoring")), __func__, lsfile, lineNum, *sp);
			sp++;
			}
		if (isspace (*sp)) {
			sp++;
		}
		if (*sp == '[')
			{
			sp++;
			cp = sp;
			while (*sp != ']' && *sp != '\0') {
				sp++;
			}
			if (*sp == '\0')
				{
				/* catgets 5206 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5206, "%s: %s(%d): Bad format for instance <%s>; ignoring the instance")), __func__, lsfile, lineNum, instance);
				return -1;
				}
			if (error == TRUE)
				{
				sp++;
				ssp = *sp;
				*sp = '\0';
				/* catgets 5207 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5207, "%s: %s(%d): Bad format for instance <%s>; ignoringthe instance")), __func__, lsfile, lineNum, instance);
				*sp = ssp;
				continue;
				}
			*sp = '\0';
			sp++;
			if ((numHosts = liblsf_parseHostList (cp, lsfile, lineNum, &hosts)) <= 0)
				{
				/* catgets 5208 */
				ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5208, "%s: %s(%d): %s(%s) failed; ignoring the instance <%s%s>")), __func__, lsfile, lineNum, "parseHostList", cp, instance, "]");
				continue;
				}

			if (resource == NULL)
				{
				if ((resource = liblsf_addResource (resName, numHosts, hosts, initValue, lsfile, lineNum)) == NULL)
					/* catgets 5209 */
					ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5209, "%s: %s(%d): %s() failed; ignoring the instance <%s>")), __func__, lsfile, lineNum, "addResource", instance);
				}
			else
				{
				if (liblsf_addHostInstance (resource, numHosts, hosts, initValue) < 0)
					/* catgets 5210 */
					ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5210, "%s: %s(%d): %s() failed; ignoring the instance <%s>")), __func__, lsfile, lineNum, __func__, instance);
				}
			continue;
			}
		else
			{
			/* catgets 5211 */
			ls_syslog (LOG_ERR, (_i18n_msg_get (ls_catd, NL_SETN, 5211, "%s: %s(%d): No <[>  for instance in <%s>; ignoring")), __func__, lsfile, lineNum, location);
			while (*sp != ']' && *sp != '\0') {
				sp++;
			}
			if (*sp == '\0') {
				return -1;
			}
			sp++;
			}
		}
	for (j = 0; j < numHosts; j++) {
		FREEUP (hosts[j]);
	}
	FREEUP (hosts);
	return 0;

}


int liblsf_parseHostList (char *hostList, char *lsfile, size_t lineNum, char ***hosts)
{
	char *host = NULL;
	char *sp = NULL;
	char **hostTable = NULL;
	size_t  numHosts = 0;

	assert( NULL != lsfile );   // FIXME all three assertions got to go
	assert( 0 != lineNum );     // investigate if a file and a line number
								// are indeed passed over

	if ( NULL == hostList ) {
		return -1;
	}

	sp = hostList;
	while ( NULL != (host = getNextWord_ (&sp)) ) {
		numHosts++;
	}

	assert( numHosts >= 0 );
	hostTable = malloc( numHosts * sizeof (char *));
	if( NULL == hostTable && ENOMEM == errno) {
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		return -1;
	}

	sp = hostList;
	numHosts = 0;
	while ((host = getNextWord_ (&sp)) != NULL) {

		if ( NULL == (hostTable[numHosts] = putstr_ (host)) ) {

			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
			for ( unsigned int i = 0; i < numHosts; i++) {
				FREEUP (hostTable[i]);
			}

			FREEUP (hostTable);

			return -1;
		}
		numHosts++;
	}
	if ( 0 == numHosts ) {
		FREEUP (hostTable);
		return -1;
	}

	*hosts = hostTable;

	return numHosts;

}


struct lsSharedResourceInfo *liblsf_addResource (char *resName, int nHosts, char **hosts, char *value, char *filename, size_t lineNum)
{
	int nRes;
	struct lsSharedResourceInfo *resInfo;

	assert( NULL != filename );     // FIX ME. these two got to go
	assert( 0 != lineNum );

	if (resName == NULL || hosts == NULL) {
		return NULL;
	}

	assert( cConf->numShareRes >= 0 ); // FIXME is numShareRes always >= 0 ?
	resInfo = myrealloc (cConf->shareRes, sizeof (struct lsSharedResourceInfo) *( cConf->numShareRes + 1 ));
	if (NULL == resInfo && ENOMEM == errno ) {
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myrealloc");
		return NULL;
	}

	cConf->shareRes = resInfo;
	nRes = cConf->numShareRes;

	if ((resInfo[nRes].resourceName = putstr_ (resName)) == NULL)
		{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		return NULL;
		}

	resInfo[nRes].nInstances = 0;
	resInfo[nRes].instances = NULL;
	if (liblsf_addHostInstance (resInfo + nRes, nHosts, hosts, value) < 0)
		{
		free (resInfo[nRes].resourceName);
		return NULL;
		}

	cConf->numShareRes++;

	return resInfo + nRes;

}

int liblsf_addHostInstance (struct lsSharedResourceInfo *sharedResource, unsigned int nHosts, char **hostNames, char *value)
{
	
	int inst = 0;
	struct lsSharedResourceInstance *instance = NULL;
	
	if (nHosts <= 0 || hostNames == NULL) {
		return -1;
	}
	
	assert( sharedResource->nInstances >= 0 ); // FIXME has to go.
	instance = myrealloc (sharedResource->instances, sizeof (struct lsSharedResourceInstance) * ( sharedResource->nInstances + 1));
	
	if (NULL == instance )
		{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "myrealloc");
		return -1;
		}
	
	sharedResource->instances = instance;
	inst = sharedResource->nInstances;
	
	if ((instance[inst].value = putstr_ (value)) == NULL)
		{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "putstr_");
		return -1;
		}
	
	assert( nHosts >= 0 );  // FIXME has to go ^_^
	instance[inst].nHosts = nHosts;
	instance[inst].hostList = malloc (sizeof (char *) * nHosts);
	if( NULL == instance[inst].hostList && ENOMEM == errno ) {
		ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc");
		free (instance[inst].value);
		return -1;
	}
	
	for ( unsigned int i = 0; i < nHosts; i++) {
		instance[inst].hostList[i] = putstr_ (hostNames[i]);
		if ( NULL ==  instance[inst].hostList[i] ) {
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "putstr_");
			for (i--; i > 0; i--) {
				free (instance[inst].hostList[i]);
			}
			
			free (instance[inst].hostList);
			free (instance[inst].value);
			return -1;
		}
	}
	
	sharedResource->nInstances++;
	
	return 0;
	
}


int convertNegNotation_ (char **value, struct HostsArray *array)
{
	char *buffer = strdup (value[0]);
	char *sp1 = strstr (buffer, "all ");
	char *sp2 = sp1;
	char *ptr = NULL;
	int cnt   = 0;
	char *save = NULL;
	char *outHosts = NULL;
	int result = -1;
	
	if (!buffer)
		{
		lserrno = LSE_MALLOC;
		goto clean_up;
		}
	
	for (cnt = 0; (sp2 > buffer) && sp2[0] != '['; cnt++)
		{
		sp2--;
		}
	
	if (!sp2 || sp2 < buffer)
		{
		goto clean_up;
		}
	
	if (cnt > 1)
		{
		memmove (sp2 + 1, sp1, strlen (sp1) + 1);
		}
	
	sp1 = sp2;
	while (sp2 && sp2[0] != ']')
		{
		sp2++;
		}
	
	if (!sp1 || !sp2)
		{
		goto clean_up;
		}
	
	ls_syslog (LOG_DEBUG, "convertNegNotation_: the original string is \'%s\'", value[0]);
	
	ptr = sp1;
	save = getNextValueQ_ (&sp1, '[', ']');
	if (!save)
		{
		goto clean_up;
		}
	
	result = resolveBaseNegHosts (save, &outHosts, array);
	if (result >= 0)
		{
		char *new_value;
		
		*ptr = 0;
		new_value =
		malloc (strlen (buffer) + strlen (outHosts) + strlen (sp2) + 2);
		if (!new_value)
			{
			lserrno = LSE_MALLOC;
			goto clean_up;
			}
		strcpy (new_value, buffer);
		strcat (new_value, "[");
		strcat (new_value, outHosts);
		strcat (new_value, sp2);
		
		FREEUP (value[0]);
		value[0] = new_value;
		}
	
clean_up: // FIXME FIXME FIXME remove goto label
	
	if (lserrno == LSE_MALLOC)
		{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		}
	
	FREEUP (buffer);
	FREEUP (outHosts);
	
	return result;
	
}


void freeSA_ (char **list, unsigned int num)
{
	if (list == NULL || num <= 0) {
		return;
	}
	
	for ( unsigned int i = 0; i < num; i++) {
		FREEUP (list[i]);
	}
	FREEUP (list);
}


int resolveBaseNegHosts (char *inHosts, char **outHosts, struct HostsArray *array)
{
	unsigned int in_num     = 0;
	unsigned int neg_num    = 0;
	unsigned int size       = 0;
	unsigned int counter    = 0;
	char *buffer    = strdup (inHosts);
	char *save      = buffer;
	char *word;
	char **inTable  = NULL;
	char **outTable = NULL;
	
	inTable = array->hosts;
	in_num = array->size;
	
	outTable = malloc( array->size * sizeof( outTable ) + 1 ); // FIXME FIXME FIXME certain buffer overflow
	if (!buffer || !inTable || !outTable) {
		lserrno = LSE_MALLOC;
		freeSA_ (outTable, neg_num);
		FREEUP (buffer);
		FREEUP (outHosts[0]);
		FREEUP (save);
		return -1;
	}
	
	if ((word = getNextWord_ (&buffer)) != NULL)
		{
		if (strcmp (word, "all")) {
			freeSA_ (outTable, neg_num);
			FREEUP (buffer);
			FREEUP (outHosts[0]);
			FREEUP (save);
			return -1;            
		}
		for ( unsigned int j = 0; j < in_num; j++) {
			size += strlen (inTable[j]);
		}
		}
	else {
		freeSA_ (outTable, neg_num);
		FREEUP (buffer);
		FREEUP (outHosts[0]);
		FREEUP (save);
		return -1;
	}
	
	while ((word = getNextWord_ (&buffer)) != NULL) {
		if (word[0] == '~') {
			word++;
			if (!isalnum (word[0])) {
				freeSA_ (outTable, neg_num);
				FREEUP (buffer);
				FREEUP (outHosts[0]);
				FREEUP (save);
				return -1;
			}
		}
		else {
			continue;
		}
		
		outTable[neg_num] = strdup (word);
		if (!outTable[neg_num]) {
			lserrno = LSE_MALLOC;
			freeSA_ (outTable, neg_num);
			FREEUP (buffer);
			FREEUP (outHosts[0]);
			FREEUP (save);
			return -1;
		}
		neg_num++;
		if (((neg_num - array->size) % array->size) == 0)
			{
			outTable = realloc( outTable, ( array->size + neg_num) * sizeof ( outTable ));
			if (!outTable)
				{
				lserrno = LSE_MALLOC;
				freeSA_ (outTable, neg_num);
				FREEUP (buffer);
				FREEUP (outHosts[0]);
				FREEUP (save);
				return -1;
				}
			}
	}
	
	for ( unsigned int j = 0; j < neg_num; j++)
		{
		for ( unsigned int k = 0; k < in_num; k++)
			{
			if (inTable[k] && equalHost_ (inTable[k], outTable[j]))
				{
				size -= strlen (inTable[k]);
				free (inTable[k]);
				inTable[k] = NULL;
				}
			}
		FREEUP (outTable[j]);
		}
	
	outHosts[0] = (char *) malloc( (unsigned long) size + (unsigned long) in_num);
	if (!outHosts[0]) {
		lserrno = LSE_MALLOC;
		freeSA_ (outTable, neg_num);
		FREEUP (buffer);
		FREEUP (outHosts[0]);
		FREEUP (save);
		return -1;
	}
	
	outHosts[0][0] = 0;
	for ( unsigned int j = 0; j < in_num; j++) {
		if (inTable[j])
			{
			strcat (outHosts[0], (const char *) inTable[j]);
			FREEUP (inTable[j]);
			strcat (outHosts[0], " ");
			counter++;
			}
	}
	
	if (outHosts[0][0]) {
		outHosts[0][strlen (outHosts[0]) - 1] = '\0';
	}
	
	FREEUP (outTable);
	FREEUP (save);
	
	assert( counter <= INT_MAX);
	return (int) counter;
}
