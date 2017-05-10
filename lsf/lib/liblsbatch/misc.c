/* $Id: lsb.misc.c 397 2007-11-26 19:04:00Z mblack $
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

#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <ctype.h>

#include "lsb/lsb.h"
#include "lsb/misc.h"
#include "lib/table.h"

// #define NL_SETN     13
// #define MEMBERSTRLEN   20

const char LONG_INT_FORMAT[] = "%ld";

void
initTab (struct hTab *tabPtr)
{

	if (tabPtr)
		{
			h_initTab_ (tabPtr, 50);
		}

}

hEnt *
addMemb (struct hTab *tabPtr, LS_LONG_INT member)  // FIXME FIXME FIXME replae LS_LONG_INT with 64-bit integer
{
	char *memberStr = malloc( sizeof (char ) * sizeof( LS_LONG_INT ) * 8 + 1 );
	hEnt *ent = NULL;
	int newNumber = 0;

	if (tabPtr)
		{
			sprintf (memberStr, LONG_INT_FORMAT, member);  // it's a const char global
			ent = h_addEnt_ (tabPtr, memberStr, &newNumber);
			if (!newNumber)
		{
			free( memberStr);
			return NULL;
		}
			else {
					free( memberStr);
					return ent;
				}
		}
	free( memberStr);
	return NULL;
}

char
remvMemb (struct hTab *tabPtr, LS_LONG_INT member)
{
	char *memberStr = malloc( sizeof (char ) * sizeof( LS_LONG_INT ) * 8 + 1 );;
	hEnt *ent = NULL;

	if (tabPtr)
		{
			sprintf (memberStr, LONG_INT_FORMAT, member); // it's a const char global
			if ((ent = h_getEnt_ (tabPtr, memberStr)) == NULL) {
					free( memberStr);
					return FALSE;
				}
			else
		{
			ent->hData = NULL;
			h_delEnt_ (tabPtr, ent);
			free( memberStr);
			return TRUE;
		}
		}
	free( memberStr );
	return FALSE;
}

hEnt *
chekMemb (struct hTab * tabPtr, LS_LONG_INT member)
{
		hEnt *ent = NULL;

		if (tabPtr)
		{
			char *memberStr = malloc( sizeof (char ) * sizeof( LS_LONG_INT ) * 8 + 1 );
			sprintf (memberStr, LONG_INT_FORMAT, member);
			ent = h_getEnt_ (tabPtr, memberStr);
			free( memberStr );
		}
		return ent;
}

hEnt *
addMembStr (struct hTab * tabPtr, char *memberStr)
{
	hEnt *ent = NULL;
	int newNumber = 0;

	if (tabPtr && memberStr)
		{
			ent = h_addEnt_ (tabPtr, memberStr, &newNumber);
			if (!newNumber)
		{
			return NULL;
		}
			else
		return ent;
		}
	return NULL;
}

char
remvMembStr (struct hTab *tabPtr, char *memberStr)
{
	hEnt *ent = NULL;

	if (tabPtr && memberStr)
		{
			if ((ent = h_getEnt_ (tabPtr, memberStr)) == NULL) {
				 return FALSE;
			}
			else
		{
			ent->hData = NULL;
			h_delEnt_ (tabPtr, ent);
			return TRUE;
		}
		}
	return FALSE;
}

hEnt *
chekMembStr (struct hTab * tabPtr, char *memberStr)
{
	hEnt *ent = NULL;

	if (tabPtr && memberStr)
		{
			ent = h_getEnt_ (tabPtr, memberStr);
		}

	return ent;
}

struct sortIntList *
initSortIntList (int increased)
{
	struct sortIntList *headerPtr;
	if ((headerPtr = malloc (sizeof (struct sortIntList))) == NULL)
		{
			return NULL;
		}
	headerPtr->forw = headerPtr->back = headerPtr;
	headerPtr->value = increased;
	return headerPtr;
}

int
insertSortIntList (struct sortIntList *header, int value)
{
	struct sortIntList *listPtr = NULL;
	struct sortIntList *newPtr  = NULL;

	listPtr = header->forw;
	while (listPtr != header)
		{

				if (listPtr->value == value) {
						return 0;
				}
			if (header->value)
		{

			if (listPtr->value > value)
				break;
			listPtr = listPtr->forw;
		}
			else
		{

			if (listPtr->value < value)
				break;
			listPtr = listPtr->forw;
		}
		}
	if ((newPtr = malloc (sizeof (struct sortIntList))) == NULL) {
		return -1;
	}
	newPtr->forw        = listPtr;
	newPtr->back        = listPtr->back;
	listPtr->back->forw = newPtr;
	listPtr->back       = newPtr;
	newPtr->value       = value;
	return 0;

}

struct sortIntList *
getNextSortIntList (struct sortIntList *header, struct sortIntList *current, int *value)
{
	struct sortIntList *nextPtr = NULL;

	nextPtr = current->forw;
	if (nextPtr == header) {
			return NULL;
	}
	*value = nextPtr->value;
	return nextPtr;

}

void
freeSortIntList (struct sortIntList *header)
{
	struct sortIntList *listPtr;
	struct sortIntList *nextPtr;

	listPtr = header->forw;
	while (listPtr != header)
		{
			nextPtr = listPtr->forw;
			free (listPtr);
			listPtr = nextPtr;
		}
	free (listPtr);
	return;

}

int
getMinSortIntList (struct sortIntList *header, int *minValue)
{
	if (header == header->forw) {
		return -1;
	}
	if (header->value) {
		*minValue = header->forw->value;
	}
	else {
		*minValue = header->back->value;
	}
	return 0;

}

int
getMaxSortIntList (struct sortIntList *header, int *maxValue)
{

	if (header == header->forw) {
		return -1;
	}
	if (header->value) {
		*maxValue = header->back->value;
	}
	else {
		*maxValue = header->forw->value;
	}
	return 0;

}

int
getTotalSortIntList (struct sortIntList *header)
{
	struct sortIntList *listPtr = NULL;
	int total = 0;

	listPtr = header->forw;
	while (listPtr != header)
		{
			total++;
			listPtr = listPtr->forw;
		}
	return total;

}

int
sndJobFile_ (int s, struct lenData *jf)
{
	unsigned int nlen = htonl (jf->len);

	if (b_write_fix (s, NET_INTADDR_ (&nlen), NET_INTSIZE_) != NET_INTSIZE_)
		{
			lsberrno = LSBE_SYS_CALL;
			return -1;
		}

	assert( jf->len <= LONG_MAX );
	if (b_write_fix (s, jf->data, jf->len) != (long)jf->len)
		{
			lsberrno = LSBE_SYS_CALL;
			return -1;
		}

	return 0;
}


void
upperStr (char *in, char *out)
{
	for (; *in != '\0'; in++, out++) {
		*out = (char) toupper (*in);
	}
	*out = '\0';
}


void
copyJUsage (struct jRusage *to, struct jRusage *from)
{
	struct pidInfo *newPidInfo = NULL;
	int *newPgid = NULL;

	to->mem = from->mem;
	to->swap = from->swap;
	to->utime = from->utime;
	to->stime = from->stime;

	if (from->npids)
	{
			assert( from->npids >= 0 );
			newPidInfo = (struct pidInfo *) calloc ((unsigned long)from->npids, sizeof (struct pidInfo));
			if( NULL == newPidInfo && ENOMEM == errno )
			{
					if (to->npids) {
							FREEUP (to->pidInfo);
					}
					to->pidInfo = newPidInfo;
					to->npids = from->npids;
					assert( from->npids >= 0 );
					memcpy ((char *) to->pidInfo, (char *) from->pidInfo, (unsigned long)from->npids * sizeof (struct pidInfo));
			}
	}
	else if (to->npids)
	{
		FREEUP (to->pidInfo);
		to->npids = 0;
	}

	if (from->npgids)
	{
			assert( from->npgids >= 0 );
			newPgid = (int *) calloc ((unsigned long)from->npgids, sizeof (int));
			if (newPgid && ENOMEM == errno ) {
					return;
			}

			if (to->npgids) {
					FREEUP (to->pgid);
			}

			to->pgid = newPgid;
			to->npgids = from->npgids;
			assert (from->npgids >= 0);
			memcpy ((char *) to->pgid, (char *) from->pgid, (unsigned long)from->npgids * sizeof (int));
	}
	else if (to->npgids) {
			FREEUP (to->pgid);
			to->npgids = 0;
	}

	return;
}

void
convertRLimit (int *pRLimits, int toKb)
{

	for ( size_t i = 0; i < LSF_RLIM_NLIMITS; i++)
		{
			switch (i)
		{
		case LSF_RLIMIT_FSIZE:
		case LSF_RLIMIT_DATA:
		case LSF_RLIMIT_STACK:
		case LSF_RLIMIT_CORE:
		case LSF_RLIMIT_RSS:
		case LSF_RLIMIT_VMEM:
			if (pRLimits[i] > 0)
				{
					if (toKb)
				{
					pRLimits[i] /= 1024;
				}
					else
				{
					pRLimits[i] *= 1024;
				}
				}
			break;
		}
		}
}

int
limitIsOk_ (int *rLimits)
{
#define EXCEED_MAX_INT(x) ( (x) > 0 ? (unsigned int)(x) >> 21 : 0 )

	if (EXCEED_MAX_INT (rLimits[LSF_RLIMIT_FSIZE])
			|| EXCEED_MAX_INT (rLimits[LSF_RLIMIT_DATA])
			|| EXCEED_MAX_INT (rLimits[LSF_RLIMIT_STACK])
			|| EXCEED_MAX_INT (rLimits[LSF_RLIMIT_CORE])
			|| EXCEED_MAX_INT (rLimits[LSF_RLIMIT_RSS])
			|| EXCEED_MAX_INT (rLimits[LSF_RLIMIT_SWAP]))
		{
			return 0;
		}
	else
		{
			return 1;
		}

	return -1;
}

char *
lsb_splitName (char *str, unsigned int *number)
{
	static char name[4 * sizeof(char) * MAXLINELEN ];
	static unsigned int nameNum = 0;
	int twoPartFlag = 0;
	unsigned int counter = 0;

	if (str == NULL || number == NULL)
	{
			/* catgets 5650 */
			ls_syslog (LOG_ERR, "5650: %s: bad input.\n", __func__);
			return NULL;
	}


	twoPartFlag = 0;
	for ( unsigned long i = 0; i < strlen (str); i++) {
			if (str[i] != '*') {
					name[counter] = str[i];
					counter++;
			}
			else {
					twoPartFlag = 1;
// #warning FIXME FIXME FIXME FIXME FIXME  what the fuck are you doing putting a nul here?
					name[counter] = '\0';
					counter = 0;
					sscanf (name, "%d", &nameNum);
					if (nameNum <= 0)
					{
							nameNum = 1;
							ls_syslog (LOG_ERR, "5651: %s: bad input format.  Assuming 1 host.\n", __func__);
					}
			}
	}

	name[counter] = '\0';
	if (twoPartFlag == 0 || counter == 0) {
			nameNum = 1;
	}

	*number = nameNum;
	return name;
}

struct nameList *
lsb_compressStrList (char **strList, unsigned int numStr)
{

	static struct nameList nameList;
	int headPtr    = 0;
	unsigned long numSameStr = 0;
	unsigned long hIndex     = 0;


	if (nameList.names != NULL)
	{
			for ( unsigned long i = 0; i < nameList.listSize; i++) {
					FREEUP (nameList.names[i]);
			}

			FREEUP (nameList.names);
			FREEUP (nameList.counter);
	}

	nameList.listSize = 0;
	nameList.names = NULL;
	nameList.counter = NULL;
	if (numStr <= 0) {
			return (struct nameList *) & nameList;
	}

	if (strList == NULL) {
			return NULL;
	}

	assert( numStr >= 0 );
	nameList.names = calloc( numStr, sizeof (char * ) ); // FIXME FIXME FIXME FIXME this assignement may be wrong. along with all similar ones in this file
	nameList.counter = calloc( numStr, sizeof (int));
	if ( ( NULL == nameList.names && ENOMEM == errno ) || 
			 ( NULL == nameList.counter && ENOMEM == errno ) )
	{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
			FREEUP (nameList.names);
			FREEUP (nameList.counter);
			return (struct nameList *) NULL;
	}

	headPtr = 0;
	numSameStr = 1;
	for (unsigned int i = 1; i < numStr; i++) {
					if (strList[i])
					{
							if (strcmp (strList[i], strList[headPtr]) == 0)
							{
									numSameStr++;
									continue;
							}

							nameList.names[hIndex] = putstr_ (strList[headPtr]);
							headPtr = i;
							nameList.counter[hIndex] = numSameStr;
							hIndex++;
							numSameStr = 1;
					}

					nameList.names[hIndex] = putstr_ (strList[headPtr]);
					nameList.counter[hIndex] = numSameStr;
					hIndex++;
					nameList.listSize = hIndex;

			nameList.names = realloc (nameList.names, nameList.listSize * sizeof (char *));
			nameList.counter = realloc (nameList.counter, nameList.listSize * sizeof (int));
			if ( ( NULL == nameList.names && ENOMEM == errno ) || 
					 ( NULL == nameList.counter && ENOMEM == errno ) )
			{
					ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "realloc");
// FIXME FIXME FIXME FIXME FIXME this may be out of the loop, needs test case and debugger
					for ( unsigned long j = 0; j < nameList.listSize; j++) {
							FREEUP (nameList.names[j]);
					}
					FREEUP (nameList.names);
					FREEUP (nameList.counter);
					nameList.listSize = 0;
					return NULL;
			}
	}

	return (struct nameList *) & nameList;
}


char *
lsb_printNameList (struct nameList * nameList, int format)
{

		static char *namestr   = NULL;
		char *buf              = NULL;
		unsigned long namelen  = 0;
		unsigned long allocLen = 0;
		unsigned long buflen   = 0;
	
		if (nameList == NULL) {
			/* catgets 5652 */
			ls_syslog (LOG_ERR, "5652: %s: NULL input.\n", __func__);
			return NULL;
		}

		if (namestr != NULL) {
				FREEUP (namestr);
		}

		allocLen = MAXLINELEN;
		namestr = calloc (allocLen, sizeof (char));
		if (!namestr) {
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
				return (char *) NULL;
		}

		for ( unsigned long i = 0; i < nameList->listSize; i++)
		{

				if (format == PRINT_SHORT_NAMELIST)
				{
						buf = calloc (MAXLINELEN, sizeof (char));
						if ( NULL == buf && ENOMEM == errno )
						{
							 ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
							 return NULL;
						}

						sprintf (buf, "%ld*%s ", nameList->counter[i], nameList->names[i]);
						buflen = strlen (buf);
				}
				else if (format == PRINT_MCPU_HOSTS)
				{
								buf = (char *) calloc (MAXLINELEN, sizeof (char));
								if ( NULL == buf && ENOMEM == errno )
								{
									 ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
									 return NULL;
								}
								sprintf (buf, "%s %ld ", nameList->names[i], nameList->counter[i]);
								buflen = strlen (buf);
				}
				else
				{
						namelen = strlen (nameList->names[i]);
						buflen = (namelen + 1) * nameList->counter[i] + 1;
						buf = (char *) calloc (buflen, sizeof (char));
						if ( NULL == buf && ENOMEM == errno )
						{
								ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
								return NULL;
						}
						
						for ( unsigned long j = 0; j < nameList->counter[i]; j++)  {
								sprintf (buf + (unsigned long)j * (namelen + 1), "%s ", nameList->names[i]);
						}
				}

				if (buflen + strlen (namestr) >= allocLen)
				{
						allocLen += buflen + MAXLINELEN;
						namestr = (char *) realloc (namestr, allocLen * sizeof (char));
						if ( NULL == namestr && ENOMEM == errno )
						{
								ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "realloc");
								return NULL;
						}
				}
				strcat (namestr, buf);
				FREEUP (buf);
		}

		return namestr;
}


struct nameList *
lsb_parseLongStr (char *string)
{
	static struct nameList nameList;
	unsigned long numStr     = strlen (string) / 2 + 1;
	unsigned long numSameStr = 0;
	char *prevStr = NULL;
	char *curStr  = NULL;

		if (string == NULL || strlen (string) <= 0)
		{
				/* catgets 5653 */
				ls_syslog (LOG_ERR, "5653: %s: bad input", __func__); 
				return NULL;
		}


		if (nameList.names != NULL)
		{
				for ( unsigned long i = 0; i < nameList.listSize; i++) {
						FREEUP (nameList.names[i]);
				}
				FREEUP (nameList.names);
				FREEUP (nameList.counter);
		}

		nameList.listSize = 0;
		nameList.names = calloc (numStr, sizeof (char *));
		nameList.counter = calloc (numStr, sizeof (int));
		if ( ( NULL == nameList.names && ENOMEM == errno ) || 
				 ( NULL == nameList.counter && ENOMEM == errno ) )
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
			FREEUP (nameList.names);
			FREEUP (nameList.counter);
			return NULL;
		}

		numSameStr = 1;
		prevStr = putstr_ (getNextWord_ (&string));
		if (strlen (prevStr) <= 0)
		{
				/* catgets 5654 */
				ls_syslog (LOG_ERR, "5654: %s: blank input\n", __func__);
				FREEUP (prevStr);
				FREEUP (nameList.names);
				FREEUP (nameList.counter);

				return &nameList; // FIXME FIXME FIXME what does this do?
																								// FIXME FIXME FIXME test case
		}


		while ((curStr = getNextWord_ (&string)) != NULL)
		{
				if (strcmp (curStr, prevStr))
				{
						if (nameList.listSize == numStr)
						{
								/* catgets 5655 */
								ls_syslog (LOG_ERR, "5655: %s: list exceeded allocated memory (shouldn't happen)\n", __func__);
								return NULL;
						}

						nameList.names[nameList.listSize] = prevStr;
						nameList.counter[nameList.listSize] = numSameStr;
						nameList.listSize++;
						numSameStr = 1;
						prevStr = putstr_ (curStr);
				}
				else {
						numSameStr++;
				}
		}

		nameList.names[nameList.listSize] = prevStr;
		nameList.counter[nameList.listSize] = numSameStr;
		nameList.listSize++;
		nameList.names   = realloc (nameList.names,   nameList.listSize * sizeof( nameList.listSize ) );
		nameList.counter = realloc (nameList.counter, nameList.listSize * sizeof( nameList.listSize ) );
		if ( ( NULL == nameList.names && ENOMEM == errno ) || 
				 ( NULL == nameList.counter && ENOMEM == errno ) )
		{
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "realloc");
				for ( unsigned long i = 0; i < nameList.listSize; i++) {
						FREEUP (nameList.names[i]);
				}
				FREEUP (nameList.names);
				FREEUP (nameList.counter);
				nameList.listSize = 0;

				return NULL;
		}
		return &nameList;

}


struct nameList *
lsb_parseShortStr (char *string, int format)
{

	static struct nameList nameList;
	unsigned long numStr = strlen (string) / 2 + 1;
	unsigned int numSameStr;
	char namestr[4 * MAXLINELEN];
	char *name;
	char *curStr;

	if (string == NULL || strlen (string) <= 0)
		{
			FREEUP (nameList.names);
			FREEUP (nameList.counter);
			return (struct nameList *) NULL;
		}

	if (nameList.names != NULL)
		{
				for ( unsigned long i = 0; i < nameList.listSize; i++) {
						free (nameList.names[i]);
				}

				free (nameList.names);
				free (nameList.counter);
		}

	nameList.listSize = 0;
	nameList.names    = calloc (numStr, sizeof ( nameList.names ) );
	nameList.counter  = calloc (numStr, sizeof ( nameList.counter ) );

	if (!nameList.names || !nameList.counter)
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
			FREEUP (nameList.names);
			FREEUP (nameList.counter);
			return (struct nameList *) NULL;
		}

	curStr = getNextWord_ (&string);

	while (curStr != NULL)
		{

			if (nameList.listSize >= numStr)
		{
			 /* catgets 5656 */
			ls_syslog (LOG_ERR, "5656: %s: list exceeded allocated memory (shouldn't happen)\n", __func__);
			return NULL;
		}

			if (format == PRINT_MCPU_HOSTS)
		{
			sprintf (namestr, "%s", curStr);
			name = namestr;
			if ((curStr = getNextWord_ (&string)) == NULL)
				{
					/* catgets 5657 */
					ls_syslog (LOG_ERR, "5657: %s: LSB_MCPU_HOSTS format error\n", __func__);
					FREEUP (nameList.names);
					FREEUP (nameList.counter);
					return NULL;
				}
			numSameStr = atoi (curStr);
		}
			else
		name = lsb_splitName (curStr, &numSameStr);
			if (name != NULL)
		{
			nameList.names[nameList.listSize] = putstr_ (name);
			nameList.counter[nameList.listSize] = numSameStr;
			nameList.listSize++;
		}

			curStr = getNextWord_ (&string);
		}


		nameList.names   = realloc( nameList.names,   nameList.listSize * sizeof ( nameList.listSize ) );
		nameList.counter = realloc( nameList.counter, nameList.listSize * sizeof ( nameList.listSize ) );
		if ( ( NULL == nameList.names  && ENOMEM == errno ) || 
				 ( NULL == nameList.counter && ENOMEM == errno ) 
			 )
		{
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "realloc");
				for ( unsigned long i = 0; i < nameList.listSize; i++) {
						FREEUP (nameList.names[i]);
				}

				FREEUP (nameList.names);
				FREEUP (nameList.counter);
				nameList.listSize = 0;

				return NULL;
		}
		return &nameList;

}

char *
getUnixSpoolDir (char *spoolDir)
{
		char *pTemp = NULL;
		if ((pTemp = strchr (spoolDir, '|')) != NULL)
		{

				int i = 0;
				*pTemp = '\0';
				while (isspace (*(--pTemp))) {
						i++;
				}

				if (i != 0) {
						*(++pTemp) = '\0';
				}

				pTemp = spoolDir;
		}
		else if (spoolDir[0] == '/')
		{
				pTemp = spoolDir;
		}
		else {
			pTemp = NULL;
		}

		return pTemp;
}

char *
getNTSpoolDir (char *spoolDir)
{
	char *pTemp = NULL;
	if ((pTemp = strchr (spoolDir, '|')) != NULL)
		{
			++pTemp;

			while ((pTemp[0] != '\0') && (pTemp[0] == ' '))
		{
			++pTemp;
		}
		}
	else if ((spoolDir[0] == '\\') || (spoolDir[1] == ':'))
		{
			pTemp = spoolDir;
		}
	else
		{
			pTemp = NULL;
		}
	return pTemp;

}

int lsb_array_idx( int jobId )
{
	if( -1 == jobId ){
		return 0;
	}
	else {
		return ( jobId >> sizeof( jobId ) ) & LSB_MAX_ARRAY_IDX;
	}

	return -2;
}

int lsb_arrayj_jobid( int jobId ) 
{
	if( -1 == jobId ) {
		return 0;
	}
	else {
		return ( jobId >> sizeof( jobId ) ) & LSB_MAX_ARRAY_IDX;
	}

	return -2;
}

void
jobId64To32 (LS_LONG_INT interJobId, unsigned int *jobId, unsigned int *jobArrElemId) // FIXME FIXME FIXME 32 bit support has to go.
{
	*jobArrElemId = lsb_array_idx (interJobId);
	*jobId = lsb_arrayj_jobid (interJobId);
}


void
jobId32To64 (LS_LONG_INT *interJobId, unsigned int jobId, unsigned int jobArrElemId) // FIXME FIXME FIXME 32 bit support has to go.
{
		assert( jobId >= 0);
		unsigned long temp = LSB_JOBID (jobId, jobArrElemId);
		assert( temp <= LONG_MAX );
		*interJobId = (long)temp;
}


int
supportJobNamePattern (char *jobname)
{
	char *p = NULL, *q = NULL;

	p = jobname;
	while ((p != NULL) && ((q = strchr (p, '*')) != NULL)) {
		q++;
		p = q;

		if (*q == '\0' || *q == '/' || *q == '*')  {
			continue;
		}
		return -1;
	}

	return 0;
}
