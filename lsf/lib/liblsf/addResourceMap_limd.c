int
addResourceMap_limd ( const char *resName, const char *location, const char *lsfile, size_t lineNum, int *isDefault)
{
	int defaultWord = FALSE;
	int first       = TRUE;
	int error       = 0;
	int resNo       = 0;
	int dynamic     = 0;
	char *sp        = NULL;
	char *cp        = NULL;
	char ssp        = 'a';
	char *instance  = NULL;
	char *initValue = NULL;
	char **hosts    = NULL;
	unsigned int numHosts   = 0;
	unsigned int numCycle   = 0;
	struct hostNode *hPtr = NULL;
	struct sharedResource *resource = NULL;

	int i = 0;

	*isDefault = FALSE;

	if (resName == NULL || location == NULL) {
		/* catgets 5382 */
		ls_syslog (LOG_ERR, "5382: %s: %s(%d): Resource name <%s> location <%s>", __func__, lsfile, lineNum,resName ? resName : "NULL", (location ? location : "NULL"));
		return -1;
	}

	if ((resNo = resNameDefined (resName)) < 0) {
		/* catgets 5275 */
		ls_syslog (LOG_ERR, "5275: %s: %s(%d): Resource name <%s> not defined", __func__, lsfile, lineNum, resName);
		return -1;
	}

	dynamic = (allInfo.resTable[resNo].flags & RESF_DYNAMIC);

	resource = inHostResourcs (resName);
	sp = location;


	i = 0;
	while (*sp != '\0') {
		if (*sp == '[') {
			i++;
		}
		else if (*sp == ']') {
			i--;
		}
		sp++;
	}
	if (i != 0) {
		/* catgets 5383 */
		ls_syslog (LOG_ERR, "5383: %s: %s(%d): number of '[' is not match that of ']' in <%s> for resource <%s>; ignoring", __func__, lsfile, lineNum, location, resName);
		return -1;
	}

	if ((initValue = malloc (4 * sizeof (char))) == NULL)
		{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
		return -1;
		}
	sp = location;

	while (sp != NULL && sp[0] != '\0') {
		for ( unsigned int j = 0; j < numHosts; j++) {
			FREEUP (hosts[j]);
		}
		FREEUP (hosts);
		numHosts = 0;
		error = FALSE;
		instance = sp;
		initValue[0] = '\0';
		defaultWord = FALSE;
		while (*sp == ' ' && *sp != '\0') {
			sp++;
		}
		if (*sp == '\0') {
			FREEUP (initValue);
			if (first == TRUE) {
				return -1;
			}
			else {
				return 0;
			}
		}
		cp = sp;
		if (*cp != '[' && *cp != '\0') {
			while (*cp && *cp != '@' && !(!iscntrl (*cp) && isspace (*cp))) {
				cp++;
			}
		}

		if (cp != sp) {
			unsigned long lsize = 0;
			ssp = cp[0];
			cp[0] = '\0';
			lsize = (strlen (sp) + 1) * sizeof (char);
			if ((initValue = realloc ( initValue, lsize)) == NULL) {
				ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "realloc", lsize);
			}
			strcpy (initValue, sp);
			if (!isdigitstr_ (initValue)  && allInfo.resTable[resNo].valueType == LS_NUMERIC) {
				/* catgets 5386 */
				ls_syslog (LOG_ERR, "5386: %s: %s(%d): Invalid characters (%s) used as NUMERIC resource value; ignoring", __func__, lsfile, lineNum, initValue);
				FREEUP (initValue);
				return -1;
			}
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

		if (*sp != '[' && *sp != '\0') {
			/* catgets 5384 */
			ls_syslog (LOG_ERR, "5384: %s: %s(%d): Bad character <%c> in instance; ignoring", __func__, lsfile, lineNum, *sp);
			sp++;
		}
		if (isspace (*sp)) {
			sp++;
		}
		if (*sp == '[') {
			sp++;
			cp = sp;
			while (*sp != ']' && *sp != '\0') {
				sp++;
			}
			if (*sp == '\0') {
				/* catgets 5385 */
				ls_syslog (LOG_ERR, "5385: %s: %s(%d): Bad format for instance <%s>; ignoring the instance",  __func__, lsfile, lineNum, instance);
				FREEUP (initValue);
				return -1;
			}
			if (error == TRUE) {
				sp++;
				ssp = *sp;
				*sp = '\0';
				ls_syslog (LOG_ERR, "5385: %s: %s(%d): Bad format for instance <%s>; ignoringthe instance", __func__, lsfile, lineNum, instance);
				*sp = ssp;
				continue;
			}
			*sp = '\0';
			sp++;

			if (initValue[0] == '\0' && !dynamic) {
				/* catgets 5276 */
				ls_syslog (LOG_ERR, "5276: %s: %s(%d): Value must be defined for resource; ignoring resource <%s>, instance <%s>", __func__, lsfile, lineNum, resName, instance);
				continue;
			}

			if (initValue[0] != '\0' && dynamic) {
				/* catgets 5277 */
				ls_syslog (LOG_ERR, "5277: %s: %s(%d): Value <%s> ignored for dynamic resource <%s>, instance <%s>", __func__, lsfile, lineNum, initValue, resName, instance);
				initValue[0] = '\0';
			}

			numHosts = parseHostList (cp, lsfile, lineNum, &hosts,  &defaultWord);
			if ( 0 == numHosts ) {
				/* catgets 5387 */
				ls_syslog (LOG_ERR, "5387: %s: %s(%d): getHostList(%s) failed; ignoring the instance <%s>", __func__, lsfile, lineNum, cp, instance);
				lim_CheckError = WARNING_ERR;
				continue;
			}
			if (defaultWord == TRUE) {
				*isDefault = TRUE;

				if (numHosts > 1) {
					/* catgets 5388 */
					ls_syslog (LOG_ERR, "5388: %s: %s(%d):  Other host is specified with reserved word <default> in the instance <%s> for resource <%s>;ignoring other hosts", __func__, lsfile, lineNum, instance, resName);
				}

				if (resource && resource->numInstances > 1) {
					/* catgets 5389 */
					ls_syslog (LOG_ERR, "5389: %s: %s(%d):  Other instances are specified with the instance <%s> for resource <%s>; ignoring the instance", __func__, lsfile, lineNum, instance, resName);
					break;
				}
			}
			if (defaultWord == TRUE) {
				numCycle = numofhosts;
				FREEUP (hosts[0]);
			}
			else {
				numCycle = 1;
			}

			for (unsigned int j = 0; j < numCycle; j++) {
				if (defaultWord == TRUE) {

					if (dynamic) {
						defaultRunElim = TRUE;
					}

					if (j == 0) {
						hPtr = myClusterPtr->hostList;
					}
					else {
						hPtr = hPtr->nextPtr;
					}
					if (hPtr == NULL) {
						break;
					}
					FREEUP (hosts[0]);
					hosts[0] = putstr_ (hPtr->hostName);
					numHosts = 1;
				}
				if (resource == NULL) {

					if (!(defaultWord && dynamic && allInfo.resTable[resNo].valueType == LS_NUMERIC) && (resource = addResource (resName, numHosts, hosts, initValue, lsfile, lineNum, TRUE)) == NULL) {
						/* catgets 5390 */
						ls_syslog (LOG_ERR, "5390: %s: %s(%d): addResource() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, instance);
					}
				}
				else {
					if (addHostInstance (resource, numHosts, hosts, initValue, TRUE) < 0)
						/* catgets 5391 */
						ls_syslog (LOG_ERR, "5391: %s: %s(%d): addHostInstance() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, instance);
					}
				}
			defaultWord = FALSE;
			continue;
			}
		else {
			/* catgets 5392 */
			ls_syslog (LOG_ERR, "5392: %s: %s(%d): No <[>  for instance in <%s>; ignoring", __func__, lsfile, lineNum, location);
			while (*sp != ']' && *sp != '\0') {
				sp++;
			}
			if (*sp == '\0') {
				FREEUP (initValue);
				return -1;
			}
			sp++;
		}
	}
	for (unsigned int j = 0; j < numHosts; j++) {
		FREEUP (hosts[j]);
	}
	FREEUP (hosts);
	FREEUP (initValue);
	return 0;
}