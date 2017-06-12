/* $Id: resourcecmd.c 397 2007-11-26 19:04:00Z mblack $
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

#include "libint/intlibout.h"
#include "lib/lproto.h"

// #define NL_SETN      22

// FIXME enter variable names
void prtOneInstance (char *, struct lsSharedResourceInstance *);
int makeShare (char *, char ***, char ***, char ***, int (*)(struct resItem *));
int isStaticSharedResource (struct resItem *);
int isDynamicSharedResource (struct resItem *);
void prtTableHeader ();
int getResourceNames (int argc, char **argv, int optind, char **resourceNames);


int
getResourceNames (int argc, char **argv, int optind, char **resourceNames)
{

  int numNames = 0, i;

  if (argc >= optind + 1)
	{
	  for (numNames = 0; argc > optind; optind++)
	{
	  for (i = 0; i < numNames; i++)
		{
		  if (strcmp (resourceNames[i], argv[optind]) == 0)
		break;
		}

	  if (numNames == 0)
		{
		  resourceNames[numNames++] = argv[optind];
		}
	  else if (i == numNames)
		{
		  resourceNames[numNames] = argv[optind];
		  numNames++;
		}
	}
	}
  return (numNames);
}


void
displayShareResource (int argc, char **argv, int index, int flag, int extflag)
{

	unsigned int numRes = 0;
	struct lsSharedResourceInfo *lsResourceInfo = NULL;
	struct lsInfo *lsInfo = NULL;
	char **resources = NULL, **resourceNames = NULL;
	int firstFlag = 1;

	if ((lsInfo = ls_info ()) == NULL)
	{
		ls_perror ("lsinfo");
		exit (-10);             // FIXME FIXME FIXME POSIX does not allow for negative exit statuses
	}

	if (argc > index)
	{
		if ((resourceNames = malloc ((argc - index) * sizeof (char *))) == NULL)
		{
			lserrno = LSE_MALLOC;
			ls_perror ("lshosts");
			exit (-1); // FIXME FIXME FIXME POSIX does not allow for negative exit statuses
		}
		numRes = getResourceNames (argc, argv, index, resourceNames);
	}

	if (numRes > 0) {
		resources = resourceNames;
	}

	// TIMEIT( 0, (
	lsResourceInfo = ls_sharedresourceinfo (resources, &numRes, NULL, 0); // ), "ls_sharedresourceinfo");

	if (lsResourceInfo == NULL)
	{
		ls_perror ("ls_sharedresourceinfo");
		exit (-1); // FIXME FIXME FIXME POSIX does not allow for negative exit statuses
	}



	for( unsigned int k = 0; k < numRes; k++)
	{
		for( unsigned int i = 0; i < lsResourceInfo[k].nInstances; i++)
		{
			for ( unsigned int j = 0; j < lsInfo->nRes; j++)
			{

				if (!extflag) {  // FIXME FIXME refactor code to single if
					if (lsInfo->resTable[j].flags & RESF_EXTERNAL) {
						continue;
					}
				}

				if (strcmp (lsInfo->resTable[j].name, lsResourceInfo[k].resourceName) == 0)
				{
					if( flag == TRUE ) // FIXME FIXME put under consideration for refactoring
					{
						if (!(lsInfo->resTable[j].flags & RESF_DYNAMIC))
						{

							if( firstFlag )
							{
								firstFlag = 0;
								prtTableHeader ();
							}
							prtOneInstance (lsResourceInfo[ k ].resourceName, &( lsResourceInfo[ k ].instances[ i ] ) );
						}
					}
					else
					{
						if (lsInfo->resTable[j].flags & RESF_DYNAMIC) 
						{
							if (firstFlag)
							{
								firstFlag = 0;
								prtTableHeader ();
							}
							prtOneInstance( lsResourceInfo[ k ].resourceName, &( lsResourceInfo[ k ].instances[ i ] ) );
						}       
					}
				}
			}
		}
	}
	if (firstFlag)
	{
		if (flag) {
			/* catgets 753 */
			printf( "753: No static shared resources defined.\n" );
		}
		else {
			/* catgets 754 */
			printf( "754: No dynamic shared resources defined.\n" );
		}
	}

	FREEUP (resourceNames);

	return;
}

void
prtTableHeader ()
{
	char *res = NULL;
	char *val = NULL;
	char *loc = NULL;

	res = putstr_( _i18n_msg_get( ls_catd, NL_SETN, 750, "RESOURCE" ) ); /* catgets 750 */
	val = putstr_( _i18n_msg_get( ls_catd, NL_SETN, 751, "VALUE" ) );    /* catgets 751 */
	loc = putstr_(_i18n_msg_get ( ls_catd, NL_SETN, 752, "LOCATION" ) ); /* catgets 752 */

	if (res == NULL || val == NULL || loc == NULL)
	{
		fprintf( stderr, "%s: %s() failed", __func__ , "putstr_");
		return;
	}
	else {
		printf ("%-25s%20s%15s\n", res, val, loc);
	}

	return;
}

void
prtOneInstance (char *name, struct lsSharedResourceInstance *instance)
{
	unsigned int currentPos = 0;
	size_t len = 0;
	char space52[] = "                                                    "; // FIXME FIXME FIXME oddly specific
	// char fmt[10];  // FIXME FIXME FIXME oddly specific

	if ((len = strlen (instance->value)) <= 20) {
		currentPos = 45 + 7;
		printf ("%-25s%20s       ", name, instance->value);
	}
	else {
		currentPos = 25 + len + 2;
		printf ("%-25s%20s  ", name, instance->value);
	}

	for ( unsigned int i = 0; i < instance->nHosts; i++) {

		len = strlen (instance->hostList[i]);
		currentPos = currentPos + len + 1;
		// sprintf (fmt, "%s%zu%s ", "%", len, "s");
		if (currentPos > 80) {						// FIXME FIXME FIXME FIXME we will need some termcap.h magic here.
			printf ("\n%s", space52);
			currentPos = 52 + len + 1;
		}
		fprintf( stdout, "%s%zu%s ", "%", len, "s", instance->hostList[i] ); // FIXME FIXME FIXME FIXME won't break at 80 lines
	}
	printf ("\n");

	return;
}

int
makeShareField (char *hostname, int flag, char ***nameTable, char ***valueTable, char ***formatTable)
{
	if (flag == TRUE) {
		return makeShare (hostname, nameTable, valueTable, formatTable, &isStaticSharedResource);
	}
	else {
		return makeShare (hostname, nameTable, valueTable, formatTable, &isDynamicSharedResource);
	}
}

int
makeShare (char *hostname, char ***nameTable, char ***valueTable, char ***formatTable, int (*resourceSelect) (struct resItem *))
{
	static unsigned int first = TRUE;
	static struct lsSharedResourceInfo *resourceInfo = NULL;
	static struct lsInfo *lsInfo = NULL;
	static char **namTable = NULL;
	static char **valTable = NULL;
	static char **fmtTable = NULL;
	static unsigned int numRes = 0;
	static unsigned int nRes = 0;
	char *hPtr = NULL;
	unsigned int numHosts = 0;
	unsigned int found = 0;

	if (first == TRUE)
	{
		if ((lsInfo = ls_info ()) == NULL)
		{
			return (-1); // FIXME FIXME FIXME POSIX does not allow negative exit status
		}

		// TIMEIT( 0, ( 
		resourceInfo = ls_sharedresourceinfo (NULL, &numRes, NULL, 0 ); // ), "ls_sharedresourceinfo" );

		if( resourceInfo == NULL )
		{
			return (-1); // FIXME FIXME FIXME POSIX does not allow negative exit status
		}
		if( ( namTable = malloc( numRes * sizeof( char * ) ) ) == NULL )
		{
			lserrno = LSE_MALLOC;
			return (-1); // FIXME FIXME FIXME POSIX does not allow negative exit status
		}
		if( ( valTable = malloc( numRes * sizeof( char * ) ) ) == NULL )
		{
			lserrno = LSE_MALLOC;
			return (-1); // FIXME FIXME FIXME POSIX does not allow negative exit status
		}
		if( ( fmtTable = malloc( numRes * sizeof( char * ) ) ) == NULL)
		{
			lserrno = LSE_MALLOC;
			return (-1); // FIXME FIXME FIXME POSIX does not allow negative exit status
		}
		first = FALSE;
	}
	else
	{
		for( unsigned int i = 0; i < nRes; i++ )
		{
			FREEUP( fmtTable[i] );
		}
	}

	nRes = 0;
	for( unsigned int k = 0; k < numRes; k++)
	{
		found = FALSE;
		for( unsigned int j = 0; j < lsInfo->nRes; j++ )
		{
			if( strcmp( lsInfo->resTable[j].name, resourceInfo[k].resourceName ) == 0 )
			{
				if( resourceSelect( &lsInfo->resTable[ j ] ) )
				{
					found = TRUE;
					break;
				}
				break;
			}
		}

		if (!found)
		{
			continue;
		}

		namTable[ nRes ] = resourceInfo[ k ].resourceName;
		found = FALSE;
		for( unsigned int i = 0; i < resourceInfo[ k ].nInstances; i++ )
		{
			numHosts = resourceInfo[k].instances[i].nHosts;
			for( unsigned int ii = 0; ii < numHosts; ii++)
			{
				hPtr = resourceInfo[k].instances[i].hostList[ii];
				if( strcmp( hPtr, hostname ) == 0 )
				{
					valTable[ nRes ] = resourceInfo[ k ].instances[ i ].value;
					found = TRUE;
					break;
				}
			}
			if( found )
			{
				break;
			}
		}

		if (!found)
		{
			valTable[ nRes ] = "-";
		}
		nRes++;
	}

	if( nRes )
	{
		unsigned int j = 0;
		for( unsigned int i = 0; i < nRes; i++ )
		{
			char fmt[16] = ""; // FIXME 16? only 16?
			int nameLen = 0;

			nameLen = strlen( namTable[ i ] );
			sprintf( fmt, "%s%d.%d%s", "%", nameLen + 2, nameLen + 1, "s" );
			fmtTable[ j++ ] = putstr_( fmt );
		}
	}

	*nameTable   = namTable;
	*valueTable  = valTable;
	*formatTable = fmtTable;

	return( nRes );
}

int
isStaticSharedResource (struct resItem *resEnt)
{
  return ((!(resEnt->flags & RESF_DYNAMIC)) &&
	  ((resEnt->valueType & LS_STRING) ||
	   (resEnt->valueType & LS_NUMERIC)));
}

int
isDynamicSharedResource (struct resItem *resEnt)
{
  return (resEnt->flags & RESF_DYNAMIC);
}
