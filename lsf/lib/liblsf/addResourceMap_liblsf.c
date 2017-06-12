
int liblsf_addResourceMap ( const char *resName, const char *location, const char *lsfile, size_t lineNum, int *isDefault )
{
	int resNo                             = 0;
	int dynamic                           = 0;
	int defaultWord                       = FALSE;
	int kek                               = FALSE;
	int error                             = FALSE; // no error
	int first                             = TRUE;
	char *cp                              = NULL;
	char *sp                              = NULL;
	char *ssp                             = NULL;
	char *instance                        = NULL;
	char *tempHost                        = NULL;
	char *initValue                       = malloc( MAXFILENAMELEN * sizeof(char) );
	char externalResourceFlag[]           = "!";
	unsigned int numCycle                 = 0;
	unsigned int numHosts                 = 0;
	char *instance                        = NULL;
	char **hosts                          = NULL;
	struct hostNode *hPtr                 = NULL;
	struct lsSharedResourceInfo *resource = NULL;

	assert( isDefault ); // FIXME FIXME FIXME, so effectively similarly named functions are one and the same and I just have to merge them. Noice.
	memset( initValue, 0, strlen( initValue ) ) ; // is this the same value as MAXFILENAMELEN * sizeof(char)?

	if (resName == NULL || location == NULL) {

		lsferrno = ENORESNAME;	// lsferrno is a global reference
		if( NULL == location ) {
			lsferrno |=  lsferr | ENOLOCATION;
		}
		/* catgets 5203/5382 */
		ls_syslog (LOG_ERR, "catgets 5203/5382: %s: %s(%d): Resource name \"%s\", location \"%s\"", __func__, lsfile, lineNum, (resName ? resName : "NULL"), (location ? location : "NULL"));
		return -1;
	}

	if ((resNo = resNameDefined (resName)) < 0) {
		/* catgets 5275 */
		ls_syslog (LOG_ERR, "catgets 5275: %s: %s(%d): Resource name <%s> not defined", __func__, lsfile, lineNum, resName);
		return -1;
	}

	dynamic = (allInfo.resTable[resNo].flags & RESF_DYNAMIC); // NOT THE SAME
	resource = inHostResourcs_e (resName); // NOT THE SAME

	if (!strcmp (location, "!"))
		{
		// initValue[0] = '\0';
		strcmp( tempHost, externalResourceFlag, strlen( externalResourceFlag ) ); // FIXME FIXME FIXME FIXME see if this strcmp is correct
		hosts = &tempHost;
		if ((resource = liblsf_addResource (resName, 1, hosts, initValue, lsfile, lineNum)) == NULL) { // FIXME FIXME FIXME FIXME liblsf_addResource(): one more function to investigate for duplicity
			const char liblsfAddResourceString[] = "liblsf_addResource";
			if ((numHosts = liblsf_parseHostList (cp, lsfile, lineNum, &hosts)) <= 0) // FIXME FIXME FIXME FIXME liblsf_parseHostList(): one more function to investigate for duplicity
			/* catgets 5209 */
			ls_syslog (LOG_ERR, "catgets 5209: %s: %s(%d): %s() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, liblsfAddResourceString, "!");
			return -1;
		}
		return 0;
	} // NOT THE SAME // NOW THE SAME

	sp = location;
	while (*sp != '\0') { // FIXME FIXME FIXME FIXME FIXME United Soviets of KEKistan... use bison; don't code your own parser. PLEASE.
		if (*sp == '[') {
			++kek;
		}
		else if (*sp == ']') {
			--kek;
		}
		sp++;
	}
	sp = location;

	if( kek ) {
		/* catgets 5204/5204 */
		ls_syslog (LOG_ERR, "catgets 5204/5383: %s: %s(%d): number of '[' is not match that of ']' in <%s> for resource <%s>; ignoring", __func__, lsfile, lineNum, location, resName);
		return -1;
	}

	while (sp != NULL && sp[0] != '\0') {
		for ( unsigned int j = 0; j < numHosts; j++) {
			FREEUP (hosts[j]);
		}
		FREEUP (hosts);
		instance = sp;
		defaultWord = FALSE;
		// initValue[0] = '\0';
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
		if (isspace (*sp)) {
			sp++;
		}

		if (*sp != '[' && *sp != '\0')
			{
			/* catgets 5205 */
			ls_syslog (LOG_ERR, "catgets 5205: %s: %s(%d): Bad character <%c> in instance; ignoring", __func__, lsfile, lineNum, *sp);
			sp++;
			}
		if (isspace (*sp)) {
			sp++;
		}
		if (*sp == '[')	{
			sp++;
			cp = sp;
			while (*sp != ']' && *sp != '\0') {
				sp++;
			}
			if (*sp == '\0') {
				/* catgets 5206 */
				ls_syslog (LOG_ERR, "catgets 5206: %s: %s(%d): Bad format for instance <%s>; ignoring the instance", __func__, lsfile, lineNum, instance);
				return -1;
			}
			if (error == TRUE) {
				sp++;
				ssp = *sp;
				*sp = '\0';
				/* catgets 5207 */
				ls_syslog (LOG_ERR, "catgets: 5207: %s: %s(%d): Bad format for instance <%s>; ignoringthe instance", __func__, lsfile, lineNum, instance);
				*sp = ssp;
				continue;
			}
			*sp = '\0';
			sp++;
			if ((numHosts = liblsf_parseHostList (cp, lsfile, lineNum, &hosts)) <= 0) // FIXME FIXME FIXME FIXME liblsf_addHostInstance(): one more function to investigate for duplicity
				{
				/* catgets 5208 */
				ls_syslog (LOG_ERR, "catgets 5208: %s: %s(%d): %s(%s) failed; ignoring the instance <%s%s>", __func__, lsfile, lineNum, "parseHostList", cp, instance, "]");
				continue;
			}

			if (resource == NULL) {
				if ((resource = liblsf_addResource (resName, numHosts, hosts, initValue, lsfile, lineNum)) == NULL) // FIXME FIXME FIXME FIXME liblsf_addResource(): one more function to investigate for duplicity
					/* catgets 5209 */
					ls_syslog (LOG_ERR, "catgets 5209: %s: %s(%d): %s() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, liblsfAddResourceString, instance);
			}
			else {
				if (liblsf_addHostInstance (resource, numHosts, hosts, initValue) < 0) // FIXME FIXME FIXME FIXME liblsf_addHostInstance(): one more function to investigate for duplicity
					/* catgets 5210 */
					ls_syslog (LOG_ERR, "catgets 5210: %s: %s(%d): %s() failed; ignoring the instance <%s>", __func__, lsfile, lineNum, liblsfAddResourceString, instance);
			}
			continue;
		}
		else {
			/* catgets 5211 */
			ls_syslog (LOG_ERR, "catgets 5211: %s: %s(%d): No <[>  for instance in <%s>; ignoring", __func__, lsfile, lineNum, location);
			while (*sp != ']' && *sp != '\0') {
				sp++;
			}
			if (*sp == '\0') {
				return -1;
			}
			sp++;
		}
	}
	for ( unsigned int j = 0; j < numHosts; j++) {
		FREEUP (hosts[j]);
	}

	free( initValue );
	FREEUP (hosts);
	return 0;
}
