

// see if the resName is defined in the global catalog
//     RETURN TYPES:
//	INT_U_MAX if name is not defined at all
//          i resName index, if found
//  INT_U_MAX if we ever reach the end of te function 
unsigned int resNameDefined ( const char *name ) // FIXME FIXME FIXME only INT_MAX nRes can be here.
{

	if (name == NULL) {
		lserrno = ENULLFUNCTIONINPUT;
		return INT_U_MAX;
	}

	assert( lsinfo.nRes > 0 ); // get rid of the global dependency 
	for (unsigned int i = 0; i < lsinfo.nRes; i++) {
		if (strcmp (name, lsinfo.resTable[i].name) == 0) {
			assert( i <= INT_MAX );
			return i;
		}
	}

	fprintf( stderr, "%s: you are not supposed to be here!\n", __func__ );
	ls_syslog( LOG_ERR, "%s: you are not supposed to be here!\n", __func__ );
	lserrno = ENOTSUPPOSEDTOBEHERE;
	return INT_U_MAX;
}
