
#include "lib/putInLists.h"

int
putInLists (char *word, struct admins *admins, unsigned int *numAds, const char *forWhat)
{
	struct passwd *pw = NULL;
	char **tempNames  = NULL;
	gid_t *tempGids   = NULL;
	uid_t *tempIds    = NULL;

	if( !( pw = getpwnam( word ) ) ) {
		if (logclass & LC_TRACE) {
			/* catgets 5410 */
			ls_syslog (LOG_ERR, "5410: %s: <%s> is not a valid user on this host; ignored", __func__, word);
		}

		return 0;
	}

	if( isInlist( admins->adminNames, pw->pw_name, admins->nAdmins ) ) {
		/* catgets 5411  */
		ls_syslog (LOG_WARNING, "5411: %s: Duplicate user name <%s> %s; ignored", __func__, word, forWhat);
		return 0;
	}

	// pw-->pw_[gu]id is always equal or greater than zero. so, the cast is ok. 
	// an extra assertion does not hurt, BUT
	// the code should be investagated as to why does it need a negative UID number.
	// is it a standard thing to do? is there any other way of representing not finding
	// a specific UID?
	// 		Comment: in HP-UX user nobody has uid -2. Why? Cuz HP-UX

	if( NULL == pw ) { // FIXME FIXME FIXME FIXME this is definatelly wrong
		fprintf( stderr, "%s: error: pw is NULL", __func__ );
		exit( EXIT_FAILURE );
	}
	admins->adminIds[   admins->nAdmins ] = pw->pw_uid;
	admins->adminGIds[  admins->nAdmins ] = pw->pw_gid;
	admins->adminNames[ admins->nAdmins ] = putstr_ (pw->pw_name);
	admins->nAdmins += 1;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG, "putInLists: uid: %d; gid: %d; name: <%s>;", pw->pw_uid, pw->pw_gid, pw->pw_name);
	}

	if (admins->nAdmins >= *numAds) {

		*numAds   *= 2;
		tempIds   = realloc(admins->adminIds,   *numAds * sizeof( uid_t ) );
		tempGids  = realloc(admins->adminGIds,  *numAds * sizeof( gid_t ) );
		tempNames = realloc(admins->adminNames, *numAds * sizeof( char * ) );

		if (tempIds == NULL || tempGids == NULL || tempNames == NULL) {
			const char realloc[] = "realloc";
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, realloc );
			FREEUP (tempIds);
			FREEUP (tempGids);
			FREEUP (tempNames);

			FREEUP (admins->adminIds);
			FREEUP (admins->adminGIds);
			for ( unsigned int i = 0; i < admins->nAdmins; i++) {
				FREEUP (admins->adminNames[ i ]);
			}

			FREEUP (admins->adminNames);
			admins->nAdmins = 0;
			lserrno         = LSE_MALLOC;
			return -1;
		}
		else {
			admins->adminIds   = tempIds;
			admins->adminGIds  = tempGids;
			admins->adminNames = tempNames;
		}
	}

	return 0;
}
