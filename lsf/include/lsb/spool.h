/* $Id: lsb.spool.h 397 2007-11-26 19:04:00Z mblack $
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

#pragma once

typedef enum spoolOptions
{
  SPOOL_INPUT_FILE,
  SPOOL_COMMAND
} spoolOptions_t;

typedef struct lsbSpoolInfo
{
  char srcFile[MAX_FILENAME_LEN];
  char spoolFile[MAX_FILENAME_LEN];

} LSB_SPOOL_INFO_T;

typedef enum spoolCopyStatus
{
  SPOOL_COPY_SUCCESS = 0,
  SPOOL_COPY_EXISTS = 1,
  SPOOL_COPY_FAILURE = -1,
  SPOOL_COPY_INITREX_FAILED = -2
} spoolCopyStatus_t;

typedef struct listElement
{
  char *elementName;
  struct listElement *nextElement;
} listElement_t;

typedef struct listElement *listElementPtr_t;

typedef struct listHeader
{
  time_t creationTime;
  listElementPtr_t firstElement;
  listElementPtr_t bestElement;
} listHeader_t;

typedef struct listHeader *listHeaderPtr_t;

#define JOB_SPOOLDIR_DELIMITER "|"

#define SPOOL_LSF_INDIR   "lsf_indir"
#define SPOOL_LSF_CMDDIR  "lsf_cmddir"
#define SPOOL_FAILED_HOSTS   20

#define LSB_OK_HOST_LIST_UPDATE_PERIOD    300

listHeaderPtr_t okHostsListPtr_;

char *getLocalHostOfficialName ();
LSB_SPOOL_INFO_T *copySpoolFile (const char *srcFilePath, spoolOptions_t option);
char *findSpoolDir (const char *spoolHost);
spoolCopyStatus_t copyFileToHost (const char *localSrcFileFullPath, const char *hostName, const char *destinFileFullDir, const char *destinFileName);
int removeSpoolFile (const char *hostName, const char *destinFileFullPath);
char *getSpoolHostBySpoolFile (const char *spoolFile);

listHeaderPtr_t createListHeader ();
int deleteListHeader (const listHeaderPtr_t pListHeader);
int deleteList (const listHeaderPtr_t pListHeader);

listElementPtr_t createListElement (const char *elementName);
int deleteListElement (const listElementPtr_t pListElement);

listElementPtr_t addElementToList (const char *elementName, const listHeaderPtr_t pListHeader);
int removeElementFromList (const listElementPtr_t pListElement, const listHeaderPtr_t pListHeader);
listElementPtr_t getBestListElement (const listHeaderPtr_t pListHeader);

int setBestListElement (const listElementPtr_t pBestElement, const listHeaderPtr_t pListHeader);

static char *getTrimmedString (const char *stringToTrim);
static spoolCopyStatus_t cpLocalFiles (const char *localSrcFileFullPath, const char *outputFileName);
static spoolCopyStatus_t cpRemoteFiles (const char *localSrcFileFullPath, const char *hostName, const char *outputDirectory, const char *outputFileName);

static listHeaderPtr_t createSpoolHostsList ();
static listHeaderPtr_t updateSpoolHostsListIfOld (const listHeaderPtr_t pListHeader, time_t permittedTimeToLiveInSec);
static listHeaderPtr_t getSpoolHostsList ();

static listHeaderPtr_t createOrUpdateSpoolHostsList (time_t permittedTimeToLiveInSec);

char *getLocalHostOfficialName (void);
char *getLocalHostOfficialName ();
LSB_SPOOL_INFO_T *copySpoolFile (const char *srcFilePath, spoolOptions_t option);
char *findSpoolDir (const char *spoolHost);
spoolCopyStatus_t copyFileToHost (const char *localSrcFileFullPath,  const char *hostName, const char *destinFileFullDir, const char *destinFileName);
int removeSpoolFile (const char *hostName, const char *destinFileFullPath);
char *getSpoolHostBySpoolFile (const char *spoolFile);
listHeaderPtr_t createListHeader ();
int deleteListHeader (const listHeaderPtr_t pListHeader);
int deleteList (const listHeaderPtr_t pListHeader);

listElementPtr_t createListElement (const char *elementName);
int deleteListElement (const listElementPtr_t pListElement);

listElementPtr_t addElementToList (const char *elementName, const listHeaderPtr_t pListHeader);
int removeElementFromList (const listElementPtr_t pListElement, const listHeaderPtr_t pListHeader);
listElementPtr_t getBestListElement (const listHeaderPtr_t pListHeader);
int setBestListElement (const listElementPtr_t pBestElement,const listHeaderPtr_t pListHeader);
listHeaderPtr_t createListHeader ( void );
const char *getDefaultSpoolDir ();

