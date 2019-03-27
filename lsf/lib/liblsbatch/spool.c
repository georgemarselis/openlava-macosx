/* $Id: lsb.spool.c 397 2007-11-26 19:04:00Z mblack $
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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "lib/lproto.h"
#include "lib/mls.h"
#include "lib/rcp.h"
#include "lsb/lsb.h"
#include "lsb/spool.h"
#include "lsf.h"

extern char **environ;
const char *defaultSpoolDir = NULL;

#define NL_SETN     13

listHeaderPtr_t okHostsListPtr_ = NULL;

char *
getLocalHostOfficialName (void)
{
	size_t localHostLength = 0;
	char *returnLocalHostPtr = NULL;
	char localHost[MAXHOSTNAMELEN];
	struct hostent *hp;

	if (returnLocalHostPtr)
	{
		return returnLocalHostPtr;
	}


	localHostLength = sizeof (localHost);

	if (gethostname (localHost, localHostLength) == -1)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "gethostname");
		lsberrno = LSBE_SYS_CALL;
		return returnLocalHostPtr;
	}

	if ((hp = Gethostbyname_ (localHost)) != NULL)
	{
		strcpy (localHost, hp->h_name);
		returnLocalHostPtr = (char *) &localHost;
	}

	return returnLocalHostPtr;
}

LSB_SPOOL_INFO_T *
copySpoolFile (const char *srcFilePath, spoolOptions_t option)
{
	listElementPtr_t bestHostFromList = NULL;
	char dirSeparator[] = SPOOL_DIR_SEPARATOR;

	struct stat srcFilePathStat;
	char spoolHost[MAXHOSTNAMELEN];
	char localHost[MAXHOSTNAMELEN];
	LSB_SPOOL_INFO_T spoolFileInfoStruct;
	char srcFileFullPath[MAX_FILENAME_LEN];
	char srcFileName[MAX_FILENAME_LEN];
	char *startFileName = NULL;
	char destinationDir[MAX_FILENAME_LEN], destinationFile[MAX_FILENAME_LEN];
	char spoolDir[MAX_FILENAME_LEN];
	const char *spoolDirPtr = NULL;
	char spoolFileFullPath[MAX_FILENAME_LEN];
	spoolCopyStatus_t spoolCopyStatus = SPOOL_COPY_FAILURE;
	pid_t pid = 0;
	char pidString[16];
	time_t now = 0;
	LSB_SPOOL_INFO_T *rtnSpoolInfo = NULL;
	listHeaderPtr_t spoolHostsList = NULL;
	const char *officialHostNamePtr = NULL;


	spoolFileInfoStruct.srcFile[0] = '\0';
	spoolFileInfoStruct.spoolFile[0] = '\0';
	srcFileFullPath[0] = '\0';

	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}



	if ((srcFilePath == NULL) || (strlen (srcFilePath) == 0))
	{
	   /* catgets 5702 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5702, "%s: srcFilePath is NULL or empty"), __func__);
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}

	if ((option != SPOOL_INPUT_FILE) && (option != SPOOL_COMMAND))
	{
	   /* catgets 5706 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5706, "%s: option parameter is wrong"), __func__);
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}

	if ((pid = getpid ()) < 0)
	{
		lsberrno = LSBE_SYS_CALL;
		goto Error;
	}
	sprintf (pidString, "%d", pid);


	strcpy (dirSeparator, SPOOL_DIR_SEPARATOR);
	if (srcFilePath[0] == dirSeparator[0])
	{
		strcpy (srcFileFullPath, srcFilePath);
	}
	else
	{
		size_t size = MAX_FILENAME_LEN;



		if (getcwd (srcFileFullPath, size) == NULL)
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "getcwd");
			lsberrno = LSBE_SYS_CALL;
			goto Error;
		}

		if (srcFileFullPath[strlen (srcFileFullPath) - 1] != dirSeparator[0])
		{
			strcat (srcFileFullPath, dirSeparator);
		}

		strcat (srcFileFullPath, srcFilePath);
	}



	if ((stat (srcFileFullPath, &srcFilePathStat) == -1) && (errno == ENOENT))
	{
	   /* catgets 5704 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5704, "%s: source file \'%s\' does not exist"), __func__, srcFileFullPath);
		lsberrno = LSBE_SP_SRC_NOT_SEEN;
		goto Error;
	}


	{
		char *pEnd1, *pEnd2;

		if ((pEnd1 = strrchr (srcFileFullPath, '/')) == NULL)
		{
			pEnd1 = (char *) srcFileFullPath;
		}
		if ((pEnd2 = strrchr (srcFileFullPath, '\\')) == NULL)
		{
			pEnd2 = (char *) srcFileFullPath;
		}

		startFileName = (pEnd2 > pEnd1) ? pEnd2 : pEnd1;


		strcpy (srcFileName, startFileName + 1);
	}

	spoolHostsList =
	createOrUpdateSpoolHostsList (LSB_OK_HOST_LIST_UPDATE_PERIOD);

	if (spoolHostsList->firstElement == NULL)
	{
		goto Error;
	}



	officialHostNamePtr = getLocalHostOfficialName ();

	if (!officialHostNamePtr)
	{
		goto Error;
	}
	strcpy (localHost, officialHostNamePtr);




	while (1)
	{

		if (spoolCopyStatus == SPOOL_COPY_EXISTS)
		{
			goto RepeatAttempt;
		}


		bestHostFromList = getBestListElement (spoolHostsList);

		if (bestHostFromList == NULL)
		{
	  /* catgets 5705 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5705, "%s: Unable to copy file <%s>. $JOB_SPOOLDIR <%s> doesn't exist or permission denied."), __func__, srcFilePath, spoolDirPtr);
			goto Error;
		}
		else
		{
			strcpy (spoolHost, bestHostFromList->elementName);
		}


		if ((spoolDirPtr =findSpoolDir (bestHostFromList->elementName)) == NULL)
		{
			removeElementFromList (bestHostFromList, getSpoolHostsList ());
			continue;
		}

		strcpy (spoolDir, spoolDirPtr);

		RepeatAttempt:

		now = time (NULL);

		switch (option)
		{
			case SPOOL_INPUT_FILE:

			sprintf (destinationDir, "%s%s%s%s%s.%s.%ld.%s",
				spoolDir, dirSeparator, SPOOL_LSF_INDIR, dirSeparator,
				localHost, spoolHost, now, pidString);
			break;
			case SPOOL_COMMAND:

			sprintf (destinationDir, "%s%s%s%s%s.%s.%ld.%s",
				spoolDir, dirSeparator, SPOOL_LSF_CMDDIR, dirSeparator,
				localHost, spoolHost, now, pidString);
			break;
		}

		sprintf (destinationFile, "%s", srcFileName);
		spoolCopyStatus =
		copyFileToHost (srcFileFullPath, spoolHost, destinationDir,
			destinationFile);

		if (spoolCopyStatus == SPOOL_COPY_SUCCESS)
		{
			break;
		}

		if (spoolCopyStatus == SPOOL_COPY_EXISTS)
		{
			sleep (1);
			continue;
		}

		if (spoolCopyStatus == SPOOL_COPY_FAILURE)
		{
			removeElementFromList (bestHostFromList, getSpoolHostsList ());
			continue;
		}
	}


	sprintf (spoolFileFullPath, "%s%s%s", destinationDir, dirSeparator,
		destinationFile);

	sprintf (spoolFileInfoStruct.srcFile, "%s", srcFileFullPath);
	sprintf (spoolFileInfoStruct.spoolFile, "%s", spoolFileFullPath);
	rtnSpoolInfo = &spoolFileInfoStruct;

	Error:

	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: completed", __func__);
	}

	return rtnSpoolInfo;

}

char *
getTrimmedString (const char *stringToTrim)
{
	char *advancePtr = NULL;
	char *returnStringPtr = NULL;
	long i = 0;

	if (stringToTrim == NULL)
	{
		returnStringPtr = NULL;
		goto Ending;
	}

	returnStringPtr = (char *) stringToTrim;


	advancePtr = returnStringPtr;

	for (i = 0; (unsigned long)i < strlen (returnStringPtr); i++)
	{
		if ((returnStringPtr[i] != ' ') && (returnStringPtr[i] != '\t'))
		{
			break;
		}
		advancePtr++;
	}
	returnStringPtr = advancePtr;



	for (i = (long)strlen (returnStringPtr) - 1; i >= 0; i--)
	{
		if ((returnStringPtr[i] != ' ') && (returnStringPtr[i] != '\t'))
		{
			break;
		}
		returnStringPtr[i] = '\0';
	}

	Ending:

	return returnStringPtr;

}

char *
findSpoolDir (const char *spoolHost)
{
	char *returnStringPtr = NULL;
	char spoolDir[MAX_FILENAME_LEN];
	char *delimiterPtr = NULL;
	struct parameterInfo *parameterInfoPtr = NULL;


	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}
	if (spoolHost == NULL)
	{
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}
	parameterInfoPtr = lsb_parameterinfo (NULL, NULL, 0);

	if (parameterInfoPtr != NULL)
	{
		if (parameterInfoPtr->pjobSpoolDir != NULL)
		{
			returnStringPtr = parameterInfoPtr->pjobSpoolDir;
		}
	}

	if (returnStringPtr == NULL || returnStringPtr[0] == 0)
	{


		if (defaultSpoolDir == NULL)
		{
			strcpy (spoolDir, LSTMPDIR);
		}
		else
		{
			strcpy (spoolDir, defaultSpoolDir);
		}
	}
	else
	{

		delimiterPtr = strstr (returnStringPtr, JOB_SPOOLDIR_DELIMITER);

		if (delimiterPtr == NULL)
		{
			strcpy (spoolDir, returnStringPtr);
		}
		else
		{
			strncpy (spoolDir, returnStringPtr, delimiterPtr - returnStringPtr);
			spoolDir[delimiterPtr - returnStringPtr] = '\0';
		}
	}

	returnStringPtr = (char *) &spoolDir;


	returnStringPtr = getTrimmedString (returnStringPtr);

	Error:

	if (logclass & (LC_TRACE | LC_EXEC))
	{
		if (returnStringPtr == NULL)
		{
			ls_syslog (LOG_DEBUG, "%s: completed with failure", __func__);
		}
		else
		{
			ls_syslog (LOG_DEBUG, "%s: completed successfuly", __func__);
		}
	}

	return returnStringPtr;

}

listHeaderPtr_t
createSpoolHostsList ()
{
	listHeaderPtr_t returnValueList = NULL;
	listElementPtr_t addElement     = NULL;
	listElementPtr_t bestElement    = NULL;
	struct hostInfoEnt *hInfo       = NULL;
	struct hostInfoEnt *hPtr        = NULL;
	struct hostent     *hp          = NULL;
	char **hostPoint                = NULL;
	char *resReq                    = NULL;
	char localHost[MAXHOSTNAMELEN];
	char returnHost[MAXHOSTNAMELEN];
	unsigned int numHosts = 0;
	
	if ((returnValueList = createListHeader ()) == NULL) {
		okHostsListPtr_ = returnValueList;
		return returnValueList;
	}

	gethostname (localHost, MAXHOSTNAMELEN);

	// {
	if ((bestElement = addElementToList (localHost, returnValueList)) == NULL) {
		okHostsListPtr_ = returnValueList;
		return returnValueList;
	}
/*    setBestListElement (bestElement, returnValueList);
	  okHostsListPtr_ = returnValueList;
	return returnValueList;
	}
*/ 
	hInfo = lsb_hostinfo_ex (hostPoint, &numHosts, resReq, 0);

	if (!hInfo) {
		lsberrno = LSBE_LSBLIB;
		okHostsListPtr_ = returnValueList;
		return returnValueList;
	}

	for ( unsigned int i = 0; i < numHosts; i++) {

		hPtr = &(hInfo[i]);
		if (hPtr->hStatus == HOST_STAT_OK) {
			strcpy (returnHost, hPtr->host);
			if ((hp = Gethostbyname_ (returnHost)) != NULL) {
				strcpy (returnHost, hp->h_name);
			}
			else {
				okHostsListPtr_ = returnValueList;
				return returnValueList;
			}

			if ((addElement = addElementToList (returnHost, returnValueList)) == NULL) {
				okHostsListPtr_ = returnValueList;
				return returnValueList;
			}

			if (strcmp (returnHost, localHost) == 0) {
				returnValueList->bestElement = addElement;
			}
		}
	}

	okHostsListPtr_ = returnValueList;
	return returnValueList;
}


listHeaderPtr_t
getSpoolHostsList ()
{

	return okHostsListPtr_;
}

listHeaderPtr_t
updateSpoolHostsListIfOld (const listHeaderPtr_t pListHeader, time_t lifePeriod)
{
	listHeaderPtr_t returnValue = NULL;

	if (!pListHeader)
	{
		okHostsListPtr_ = returnValue;
		return returnValue;
	}

	if ((time (0) - pListHeader->creationTime) > lifePeriod)
	{


		deleteList (pListHeader);


		returnValue = createSpoolHostsList ();
	}
	else
	{
		returnValue = pListHeader;
	}

	okHostsListPtr_ = returnValue;
	return returnValue;
}

listHeaderPtr_t
createOrUpdateSpoolHostsList (time_t permittedTimeToLiveInSec)
{
	listHeaderPtr_t spoolHostsList;

	if ((spoolHostsList = getSpoolHostsList ()) == NULL)
	{
		spoolHostsList = createSpoolHostsList ();
	}
	else
	{
		spoolHostsList =
		updateSpoolHostsListIfOld (spoolHostsList, permittedTimeToLiveInSec);
	}

	return spoolHostsList;

}


spoolCopyStatus_t
cpLocalFiles (const char *localSrcFileFullPath, const char *outputFileName)
{
	spoolCopyStatus_t spoolCopyStatus = SPOOL_COPY_FAILURE;
	int input, output;
	char line[MAX_LINE_LEN + 1];
	ssize_t nItems;

	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	if ((localSrcFileFullPath == NULL) || (outputFileName == NULL))
	{
		goto Error;
	}


	input = open (localSrcFileFullPath, O_RDONLY, 0555);
	if (input == -1)
	{
		spoolCopyStatus = SPOOL_COPY_FAILURE;
		if (logclass & (LC_TRACE | LC_EXEC))
		{
	  ls_syslog (LOG_ERR, "%s: %s, spoolCopyStatus = SPOOL_COPY_FAILURE\n", __func__, I18N (5709, "Unable to open source file ")); /*catgets 5709 */
		}
		close (input);
		goto Error;
	}


	output = open (outputFileName, O_CREAT | O_EXCL | O_RDWR, 0700);

	if ((output == -1) && (errno == EEXIST))
	{
		spoolCopyStatus = SPOOL_COPY_EXISTS;

		if (logclass & (LC_TRACE | LC_EXEC))
		{
			ls_syslog (LOG_ERR, "%s: spoolCopyStatus = SPOOL_COPY_EXISTS\n",
				__func__);
		}
		goto Close;
	}

	if ( -1 == output)
	{
		spoolCopyStatus = SPOOL_COPY_FAILURE;
		if (logclass & (LC_TRACE | LC_EXEC))
		{
	  ls_syslog (LOG_ERR, "%s: %s \'%s\', spoolCopyStatus = SPOOL_COPY_FAILURE\n", __func__, I18N (5707, "Unable to create outputfile ")   /*catgets 5707 */
			, outputFileName);
	}

	if (createSpoolSubDir (outputFileName) != 0)
	{
		goto Close;
	}


	output = open (outputFileName, O_CREAT | O_EXCL | O_RDWR, 0700);
	if (output == -1)
	{
		lsberrno = LSBE_SYS_CALL;
		goto Close;
	}
}


spoolCopyStatus = SPOOL_COPY_SUCCESS;

while ((nItems = read (input, line, MAX_LINE_LEN)))
{

	assert( nItems >= 0 );
	if (write (output, line, (size_t) nItems) == -1)
	{
		spoolCopyStatus = SPOOL_COPY_FAILURE;
		if (logclass & (LC_TRACE | LC_EXEC))
		{
		  ls_syslog (LOG_ERR, "%s: spoolCopyStatus = SPOOL_COPY_FAILURE\n", __func__, I18N (5708, "write operation failure")); /*catgets 5708 */
		}
		goto Close;
	}
}



Close:
close (input);
close (output);

Error:

if (logclass & (LC_TRACE | LC_EXEC))
{
	if (spoolCopyStatus == SPOOL_COPY_FAILURE)
	{
		ls_syslog (LOG_DEBUG, "%s: completed with failure", __func__);
	}
	else
	{
		ls_syslog (LOG_DEBUG, "%s: completed successfuly", __func__);
	}
}

return spoolCopyStatus;

}


spoolCopyStatus_t
cpRemoteFiles (const char *localSrcFileFullPath, const char *hostName, const char *outputDirectory, const char *outputFileName)
{
	lsRcpXfer lsXfer;
	spoolCopyStatus_t spoolCopyStatus = SPOOL_COPY_FAILURE;
	char remoteHost[MAXHOSTNAMELEN];
	char remoteFileSpec[1 + MAXHOSTNAMELEN + 1 + MAX_FILENAME_LEN];
	char *buf = NULL;
	int iCount = 0;
	int output = -1;
	struct stat remoteFileStatusBuf;
	SIGFUNCTYPE oldsigTTIN, oldsigTTOU, oldsigINT, oldsigUSR1;
	char lsfUserName[MAX_LINE_LEN];
	struct hostent *hp;

	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	if ((localSrcFileFullPath == NULL)
		|| (hostName == NULL)
		|| (outputDirectory == NULL) || (outputFileName == NULL))
	{
		goto Error;
	}


	if (strstr (outputFileName, outputDirectory) == NULL)
	{
		goto Error;
	}

	oldsigTTIN = Signal_ (SIGTTIN, (SIGFUNCTYPE) SIG_IGN);
	oldsigTTOU = Signal_ (SIGTTOU, (SIGFUNCTYPE) SIG_IGN);
	oldsigINT = Signal_ (SIGINT, (SIGFUNCTYPE) SIG_IGN);
	oldsigUSR1 = Signal_ (SIGUSR1, (SIGFUNCTYPE) SIG_IGN);

	strcpy (remoteHost, hostName);

	if ((hp = Gethostbyname_ ((char *) hostName)) != NULL)
	{
		strcpy (remoteHost, hp->h_name);
	}

	remoteFileStatusBuf.st_size = -1;

	output =
	ls_rstat ((char *) remoteHost, (char *) outputFileName,
		&remoteFileStatusBuf);

	if ((output == 0) && (remoteFileStatusBuf.st_size != -1))
	{
		spoolCopyStatus = SPOOL_COPY_EXISTS;

		if (logclass & (LC_TRACE | LC_EXEC))
		{
			ls_syslog (LOG_ERR, "%s: spoolCopyStatus = SPOOL_COPY_EXISTS\n",
				__func__);
		}
		goto Done;
	}

	if (createXfer (&lsXfer))
	{
		ls_donerex ();
		goto Error;
	}


	sprintf (remoteFileSpec, "@%s:%s", hostName, outputFileName);


	lsXfer.szSourceArg = putstr_ (localSrcFileFullPath);


	if (getLSFUser_ (lsfUserName, MAX_LINE_LEN) != 0)
	{

		ls_donerex ();
		goto Error;
	}
	lsXfer.szHostUser = putstr_ (lsfUserName);
	parseXferArg ((char *) localSrcFileFullPath, &(lsXfer.szHostUser),
		&(lsXfer.szHost), &(lsXfer.ppszHostFnames[0]));
	lsXfer.szDestArg = putstr_ (remoteFileSpec);
	parseXferArg ((char *) remoteFileSpec, &(lsXfer.szDestUser),
		&(lsXfer.szDest), &(lsXfer.ppszDestFnames[0]));

	buf = (char *) malloc (LSRCP_MSGSIZE);

	if (!buf)
	{

		ls_donerex ();
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL_S, "cpRemoteFiles", "IO_SPOOL", _i18n_msg_get (ls_catd, NL_SETN, 5700, "try rcp...")); /* catgets 5700 */
		if (doXferRcp (&lsXfer, SPOOL_BY_LSRCP) < 0)
		{
			goto Destroy;
		}
		spoolCopyStatus = SPOOL_COPY_SUCCESS;
		goto Destroy;
	}

	for (iCount = 0; iCount < lsXfer.iNumFiles; iCount++)
	{
		if (copyFile (&lsXfer, buf, SPOOL_BY_LSRCP))
		{

			ls_donerex ();
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_S, "cpRemoteFiles", "IO_SPOOL", _i18n_msg_get (ls_catd, NL_SETN, 5700, "try rcp..."));

			if (doXferRcp (&lsXfer, SPOOL_BY_LSRCP) < 0)
			{
				goto Destroy;
			}
			spoolCopyStatus = SPOOL_COPY_SUCCESS;
			goto Destroy;
		}

		spoolCopyStatus = SPOOL_COPY_SUCCESS;

		if (logclass & (LC_FILE))
		{
			ls_syslog (LOG_DEBUG, "cpRemoteFiles(), copy file succeeded.");
		}
	}

	Destroy:
	ls_donerex ();

	Signal_ (SIGTTIN, (SIGFUNCTYPE) oldsigTTIN);
	Signal_ (SIGTTOU, (SIGFUNCTYPE) oldsigTTOU);
	Signal_ (SIGINT, (SIGFUNCTYPE) oldsigINT);
	Signal_ (SIGUSR1, (SIGFUNCTYPE) oldsigUSR1);

	if (destroyXfer (&lsXfer))
	{
		goto Error;
	}



	Done:
	Error:

	if (buf != NULL)
	{
		free (buf);
	}
	return spoolCopyStatus;

}

spoolCopyStatus_t
copyFileToHost (const char *localSrcFileFullPath, const char *hostName, const char *destinFullDir, const char *destinFileName)
{
	spoolCopyStatus_t spoolCopyStatus = SPOOL_COPY_FAILURE;
	char localHost[MAXHOSTNAMELEN];
	char hostNameTwin[MAXHOSTNAMELEN];
	char outputFileName[MAX_FILENAME_LEN];
	struct hostent *hp;

	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	if ((localSrcFileFullPath == NULL)
		|| (hostName == NULL)
		|| (destinFullDir == NULL) || (destinFileName == NULL))
	{
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}


	gethostname (localHost, MAXHOSTNAMELEN);
	strcpy (hostNameTwin, hostName);

	if ((hp = Gethostbyname_ ((char *) hostName)) != NULL)
	{
		strcpy (hostNameTwin, hp->h_name);
	}

	sprintf (outputFileName, "%s%s%s", destinFullDir, SPOOL_DIR_SEPARATOR,
		destinFileName);



	if (strcmp (localHost, hostNameTwin) == 0)
	{

		spoolCopyStatus = cpLocalFiles (localSrcFileFullPath, outputFileName);
	}
	else
	{

		spoolCopyStatus =
		cpRemoteFiles (localSrcFileFullPath, hostName, destinFullDir,
			outputFileName);
	}

	if (spoolCopyStatus == SPOOL_COPY_FAILURE)
	{
		lsberrno = LSBE_SP_COPY_FAILED;
	}
	Error:
	if (logclass & (LC_TRACE | LC_EXEC))
	{
		if (spoolCopyStatus == SPOOL_COPY_FAILURE)
		{
			ls_syslog (LOG_DEBUG, "%s: completed with failure", __func__);
		}
		else
		{
			ls_syslog (LOG_DEBUG, "%s: completed successfuly", __func__);
		}
	}

	return spoolCopyStatus;

}

int
removeSpoolFile (const char *hostName, const char *destinFileFullPath)
{
	pid_t pid;
	DIR *dirPtr = NULL;
	char szRshDest[MAX_FILENAME_LEN];
	char localHost[MAXHOSTNAMELEN];
	char hostNameTwin[MAXHOSTNAMELEN];
	struct hostent *hp;
	int returnValue = -1;


	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	if ((hostName == NULL) || (destinFileFullPath == NULL))
	{
		lsberrno = LSBE_BAD_ARG;
		goto Done;
	}

	gethostname (localHost, MAXHOSTNAMELEN);

	if (hostName == NULL)
	{
		strcpy (hostNameTwin, localHost);
	}
	else
	{
		strcpy (hostNameTwin, hostName);
	}

	if ((hp = Gethostbyname_ ((char *) hostName)) != NULL)
	{
		strcpy (hostNameTwin, hp->h_name);
	}

	strcpy (hostNameTwin, localHost);




	if (strcmp (localHost, hostNameTwin) == 0)
	{


		dirPtr = opendir (destinFileFullPath);

		if (dirPtr != NULL)
		{
			closedir (dirPtr);

			returnValue = rmDirAndFiles ((char *) destinFileFullPath);
			goto Done;
		}

		if (unlink (destinFileFullPath) == 0)
		{
			returnValue = 0;
			goto Done;
		}
		else
		{
			lsberrno = LSBE_SP_DELETE_FAILED;
			goto Done;
		}
	}



	if (ls_runlink (hostNameTwin, (char *) destinFileFullPath) == 0)
	{
		returnValue = 0;
		goto Done;
	}
	else
	{
		returnValue = -1;
		lsberrno = LSBE_SP_DELETE_FAILED;
	}


	switch (pid = fork ())
	{
		case 0:
			sprintf (szRshDest, "rm -rf %s", destinFileFullPath);
			execlp (RSHCMD, RSHCMD, hostName, szRshDest, NULL);
		return -1;
		break;

		case -1:

			if (logclass & (LC_FILE))
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
			lsberrno = LSBE_SP_FORK_FAILED;
			goto Done;
		break;
		default:

		if (waitpid (pid, 0, 0) < 0 && errno != ECHILD)
		{
			lsberrno = LSBE_SP_CHILD_DIES;
			goto Done;
		}

		returnValue = 0;
		break;
	}

	Done:
	if (logclass & (LC_TRACE | LC_EXEC))
	{
		if (returnValue != 0)
		{
			ls_syslog (LOG_DEBUG, "%s: completed with failure", __func__);
		}
		else
		{
			ls_syslog (LOG_DEBUG, "%s: completed successfuly", __func__);
		}
	}

	return returnValue;

}

char *
getSpoolHostBySpoolFile (const char *spoolFile)
{
	char *pEnd        = NULL;
	char *pBegin      = NULL;
	char *returnValue = NULL;
	char *localHost   = malloc( sizeof( char ) * MAXHOSTNAMELEN + 1 ); // FIXME FIXME FIXME FIXME free when done with
	char *buf         = malloc( sizeof( char ) * MAXHOSTNAMELEN + 1 ); // FIXME FIXME FIXME FIXME free when done with
	char *officialHostNamePtr = NULL;
	int numOfDot = 0;

	if (logclass & (LC_TRACE | LC_EXEC))
	{
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	if (spoolFile == NULL)
	{
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}


	officialHostNamePtr = getLocalHostOfficialName ();

	if (!officialHostNamePtr)
	{
		goto Error;
	}
	strcpy (localHost, officialHostNamePtr);

	strcpy (buf, spoolFile);

	pBegin = (char *) (buf + strlen (buf));

	if (strstr (buf, SPOOL_DIR_SEPARATOR) == NULL)
	{
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}

	while (buf[pBegin - buf] != SPOOL_DIR_SEPARATOR_CHAR)
	{
		pBegin--;
	}

	buf[pBegin - buf] = '\0';

	if (strstr (buf, ".") == NULL)
	{
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}


	while (buf[pBegin - buf] != '.')
	{
		pBegin--;
	}

	buf[pBegin - buf] = '\0';

	if (strstr (buf, ".") == NULL)
	{
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}


	while (buf[pBegin - buf] != '.')
	{
		pBegin--;
	}

	buf[pBegin - buf] = '\0';

	pEnd = pBegin;


	if (strstr (buf, SPOOL_DIR_SEPARATOR) == NULL)
	{
		lsberrno = LSBE_BAD_ARG;
		goto Error;
	}


	while (buf[pEnd - buf] != SPOOL_DIR_SEPARATOR_CHAR)
	{
		if (buf[pEnd - buf] == '.')
		{
			numOfDot++;
		}
		pEnd--;
	}

	pEnd++;


	numOfDot = numOfDot / 2 + 1;
	while (numOfDot > 0)
	{
		while (pEnd[0] != '.')
		{
			pEnd++;
		}
		numOfDot--;
		pEnd++;
	}



	returnValue = pEnd;

	Error:
	if (logclass & (LC_TRACE | LC_EXEC))
	{
		if (returnValue == NULL)
		{
			ls_syslog (LOG_DEBUG, "%s: completed with failure", __func__);
		}
		else
		{
			ls_syslog (LOG_DEBUG, "%s: completed successfuly", __func__);
		}
	}

	return returnValue;

}

listHeaderPtr_t
createListHeader ( void )
{
	listHeaderPtr_t returnValue = NULL;

	returnValue = (listHeaderPtr_t) malloc (sizeof (listHeader_t));

	if (!returnValue)
	{
		goto Done;
	}

	returnValue->creationTime = time (0);
	returnValue->firstElement = NULL;
	returnValue->bestElement = NULL;

	Done:
	return returnValue;

}

int
deleteListHeader (const listHeaderPtr_t pListHeader)
{
	int returnValue = -1;

	if (!pListHeader)
	{
		goto Done;
	}

	if (pListHeader->firstElement)
	{
		goto Done;
	}

	free (pListHeader);
	returnValue = 0;

	Done:
	return returnValue;

}

int
deleteList (const listHeaderPtr_t pListHeader)
{
	int returnValue = -1;
	listElementPtr_t pListElement;

	if (!pListHeader)
	{
		goto Done;
	}

	pListElement = pListHeader->firstElement;


	while (pListElement)
	{
		listElementPtr_t pFreeElement = pListElement;
		pListElement = pListElement->nextElement;
		returnValue = deleteListElement (pFreeElement);
		if (returnValue == -1)
		{
			goto Done;
		}
	}

	pListHeader->firstElement = NULL;
	returnValue = deleteListHeader (pListHeader);

	Done:
	return returnValue;

}

listElementPtr_t
createListElement (const char *elementName)
{
	listElementPtr_t returnValue = NULL;

	if (!elementName)
	{
		goto Done;
	}

	returnValue = (listElementPtr_t) malloc (sizeof (listElement_t));

	if (!returnValue)
	{
		goto Done;
	}

	returnValue->elementName = (char *) malloc (strlen (elementName) + 1);

	if (returnValue->elementName)
	{
		strcpy (returnValue->elementName, elementName);
	}

	returnValue->nextElement = NULL;

	Done:
	return returnValue;

}

int
deleteListElement (const listElementPtr_t pListElement)
{
	int returnValue = -1;

	if (!pListElement)
	{
		goto Done;
	}

	if (pListElement->elementName)
	{
		free (pListElement->elementName);
	}

	free (pListElement);
	returnValue = 0;

	Done:
	return returnValue;

}

listElementPtr_t
addElementToList (const char *elementName, const listHeaderPtr_t pListHeader)
{
	listElementPtr_t pFirstElement = NULL;
	listElementPtr_t pNewElement = NULL;

	if (!elementName || !pListHeader)
	{
		goto Done;
	}

	if ((pNewElement = createListElement (elementName)) == NULL)
	{
		goto Done;
	}

	pFirstElement = pListHeader->firstElement;
	pNewElement->nextElement = pFirstElement;
	pListHeader->firstElement = pNewElement;

	Done:
	return pNewElement;

}

int
removeElementFromList (const listElementPtr_t pListElement,
	const listHeaderPtr_t pListHeader)
{
	int returnValue = -1;
	listElementPtr_t pNextElement = NULL;
	listElementPtr_t pPrevElement = NULL;

	if (!pListElement || !pListHeader)
	{
		goto Done;
	}

	pPrevElement = pListHeader->firstElement;

	if (!pPrevElement)
	{
		returnValue = 0;
		goto Done;
	}

	pNextElement = pPrevElement->nextElement;

	if ((pListHeader->bestElement)
		&& (pListHeader->bestElement == pListElement))
	{
		pListHeader->bestElement = NULL;
	}

	if (pListHeader->firstElement == pListElement)
	{
		pListHeader->firstElement = pListHeader->firstElement->nextElement;
		deleteListElement (pPrevElement);
		returnValue = 0;

		goto Done;
	}


	while (pNextElement)
	{
		if (pListElement == pNextElement)
		{
			pPrevElement->nextElement = pNextElement->nextElement;
			deleteListElement (pNextElement);
			returnValue = 0;
			goto Done;
		}
		pPrevElement = pNextElement;
		pNextElement = pNextElement->nextElement;
	}

	Done:
	return returnValue;

}

int
setBestListElement (const listElementPtr_t pBestElement,
	const listHeaderPtr_t pListHeader)
{
	int returnValue = -1;

	if (!pListHeader)
	{
		goto Done;
	}

	pListHeader->bestElement = pBestElement;
	returnValue = 0;
	Done:
	return returnValue;

}

listElementPtr_t
getBestListElement (const listHeaderPtr_t pListHeader)
{
	listElementPtr_t returnValue = NULL;

	if (!pListHeader)
	{
		goto Done;
	}

	if (pListHeader->bestElement)
	{
		returnValue = pListHeader->bestElement;
	}
	else
	{
		returnValue = pListHeader->firstElement;
	}

	Done:
	return returnValue;

}
