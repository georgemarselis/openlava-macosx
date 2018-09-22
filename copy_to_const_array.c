#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "copy_to_const_array.h"


int main( int argc, char **argv )
{

	unsigned int return_status = 0; // in this program, all errors are major errors

	assert( argc );
	assert( argv );

	copy_array ( );
	return_status = compare_elements( );

	return (int) return_status;
}

unsigned int copy_array( void )
{
	// uid_t workNAdmins     = 0;
	// uid_t tempNAdmins     = 0;
	// uid_t *tempAdminIds   = NULL;
	// uid_t *workAdminIds   = NULL;
	// char **tempAdminNames = NULL;
	// char **workAdminNames = NULL;

	// const char **tempNames = NULL;
	// create the const array require to pass to isInlist()
		// one at a time, linear, not everything at once.
		// copy tempAdminNames to new **array
		// copy workAdminNames to new **array
		// copy tempNAdmins    to new int

	fprintf(stderr, "%s\n", "this is a stub" );
	return 0;
}

unsigned int compare_elements( void )
{
	fprintf(stderr, "%s\n", "this is a stub" );
	return 0;
}
