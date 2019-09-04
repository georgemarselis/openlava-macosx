/*$Id: lsb.spool.h 397 2007-11-26 19:04:00Z mblack $
 *Copyright (C) 2007 Platform Computing Inc
 *
 *This program is free software; you can redistribute it and/or modify
 *it under the terms of version 2 of the GNU General Public License as
 *published by the Free Software Foundation.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.

 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#pragma once

// typedef enum spoolOptions
enum spoolOptions
{
    SPOOL_INPUT_FILE = 0,
    SPOOL_COMMAND    = 1
};
// } spoolOptions_t;

// typedef struct lsbSpoolInfo
struct lsbSpoolInfo
{
    const char srcFile[MAX_FILENAME_LEN];
    const char spoolFile[MAX_FILENAME_LEN];
};
// } LSB_SPOOL_INFO_T;

// typedef enum spoolCopyStatus
enum spoolCopyStatus
{
    SPOOL_COPY_SUCCESS        =  0,
    SPOOL_COPY_EXISTS         =  1,
    SPOOL_COPY_FAILURE        = -1,
    SPOOL_COPY_INITREX_FAILED = -2
// } spoolCopyStatus_t;
};

// typedef struct listElement
struct listElement
{
    const char *elementName;
    struct listElement *nextElement;
};
// } listElement_t;

// typedef struct listElement *listElementPtr_t;

// typedef struct listHeader
struct listHeader
{
    time_t creationTime;
    struct listElement *firstElement;
    struct listElement *bestElement;
};
// } listHeader_t;

// typedef struct listHeader *listHeaderPtr_t;

// #define JOB_SPOOLDIR_DELIMITER "|"
const char JOB_SPOOLDIR_DELIMITER[ ] = "|"; // man 5 lsb.params: In a mixed UNIX/Windows cluster, specify one path for the UNIX platform and one for the Windows platform. Separate the two paths by a pipe character (|). Example: JOB_SPOOL_DIR=/usr/share/lsf_spool | \\HostA\share\spooldir

// #define SPOOL_LSF_INDIR   "lsf_indir"
const char SPOOL_LSF_INDIR[ ] = "lsf_indir"
// #define SPOOL_LSF_CMDDIR  "lsf_cmddir"
const char SPOOL_LSF_CMDDIR[ ] = "lsf_cmddir"
// #define SPOOL_FAILED_HOSTS   20
enum SPOOL_FAILED {
    SPOOL_FAILED_HOSTS = 20
}

// #define LSB_OK_HOST_LIST_UPDATE_PERIOD    300
enum LSB_OK_HOST {
    LSB_OK_HOST_LIST_UPDATE_PERIOD = 300
};

struct listHeader *okHostsListPtr_;

char *getLocalHostOfficialName ();
struct lsbSpoolInfo *copySpoolFile (const char *srcFilePath, enum spoolOptions option);
char *findSpoolDir (const char *spoolHost);
enum spoolCopyStatus copyFileToHost (const char *localSrcFileFullPath, const char *hostName, const char *destinFileFullDir, const char *destinFileName);
int removeSpoolFile (const char *hostName, const char *destinFileFullPath);
char *getSpoolHostBySpoolFile (const char *spoolFile);

struct listHeader *createListHeader ();
int deleteListHeader (const struct listHeader *pListHeader);
int deleteList (const struct listHeader *pListHeader);

struct listElement *createListElement (const char *elementName);
int deleteListElement (const struct listElement *pListElement);

struct listElement *addElementToList (const char *elementName, const struct listHeader *pListHeader);
int removeElementFromList (const struct listElement *pListElement, const struct listHeader *pListHeader);
struct listElement *getBestListElement (const struct listHeader *pListHeader);

int setBestListElement (const struct listElement *pBestElement, const struct listHeader *pListHeader);

char *getTrimmedString (const char *stringToTrim);
enum spoolCopyStatus cpLocalFiles (const char *localSrcFileFullPath, const char *outputFileName);
enum spoolCopyStatus cpRemoteFiles (const char *localSrcFileFullPath, const char *hostName, const char *outputDirectory, const char *outputFileName);

struct listHeader *createSpoolHostsList ();
struct listHeader *updateSpoolHostsListIfOld (const struct listHeader *pListHeader, time_t permittedTimeToLiveInSec);
struct listHeader *getSpoolHostsList ();

struct listHeader *createOrUpdateSpoolHostsList (time_t permittedTimeToLiveInSec);

char *getLocalHostOfficialName (void);
char *getLocalHostOfficialName ();
struct lsbSpoolInfo *copySpoolFile (const char *srcFilePath, enum spoolOptions option);
char *findSpoolDir (const char *spoolHost);
enum spoolCopyStatus copyFileToHost (const char *localSrcFileFullPath,  const char *hostName, const char *destinFileFullDir, const char *destinFileName);
int removeSpoolFile (const char *hostName, const char *destinFileFullPath);
char *getSpoolHostBySpoolFile (const char *spoolFile);
struct listHeader *createListHeader ();
int deleteListHeader (const struct listHeader *pListHeader);
int deleteList (const struct listHeader *pListHeader);

struct listElement *createListElement (const char *elementName);
int deleteListElement (const struct listElement *pListElement);

struct listElement *addElementToList (const char *elementName, const struct listHeader *pListHeader);
int removeElementFromList (const struct listElement *pListElement, const struct listHeader *pListHeader);
struct listElement *getBestListElement (const struct listHeader *pListHeader);
int setBestListElement (const struct listElement *pBestElement,const struct listHeader *pListHeader);
struct listHeader *createListHeader ( void );
const char *getDefaultSpoolDir ();

