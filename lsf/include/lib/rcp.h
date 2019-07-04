/* $Id: lib.rcp.h 397 2007-11-26 19:04:00Z mblack $
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

// #ifndef LSF_LIB_RCP_H
// #define LSF_LIB_RCP_H

#include <sys/stat.h>

struct rcpXfer
{
    char *szSourceArg;
    char *szDestArg;
    char *szHostUser;
    char *szDestUser;
    char *szHost;
    char *szDest;
    char *ppszHostFnames[1]; // FIXME FIXME change the 1 to an easily understood label
    char *ppszDestFnames[1]; // FIXME FIXME change the 1 to an easily understood label
    struct hostent *pheHost;
    struct hostent *pheDest;
    int iOptions;
    char padding[4];
    size_t iNumFiles;
};


// #define RSHCMD "rsh" // FIXME FIXME FIXME FIXME rsh must go

// #define SPOOL_DIR_SEPARATOR "/"
// #define SPOOL_DIR_SEPARATOR_CHAR '/'

// #define SPOOL_BY_LSRCP      0x1

char RSHCMD[] = "rsh";
char SPOOL_DIR_SEPARATOR = '/';
char SPOOL_DIR_SEPARATOR_CHAR = '/';
int SPOOL_BY_LSRCP = 0x1;

#define FILE_ERRNO(errno) \
    (errno == ENOENT || errno == EPERM || errno == EACCES || \
     errno == ELOOP || errno == ENAMETOOLONG || errno == ENOTDIR || \
         errno == EBADF || errno == EFAULT || \
         errno == EEXIST || errno == ENFILE || errno == EINVAL || \
         errno == EISDIR || errno == ENOSPC || errno == ENXIO || \
         errno == EROFS || errno == ETXTBSY)

// #define LSRCP_MSGSIZE   1048576
size_t LSRCP_MSGSIZE = 1048576;

// extern int mystat_ (char *, struct stat *, struct hostent *);
// extern int myopen_ (char *, int, int, struct hostent *);
// extern char *usePath (char *path);
// extern int parseXferArg (char *arg, char **userName, char **hostName, char **fName);
// extern int createXfer (struct rcpXfer * lsXfer);
// extern int destroyXfer (struct rcpXfer * lsXfer);
// extern int copyFile (struct rcpXfer * lsXfer, char *buf, int option);
// extern int equivalentXferFile (struct rcpXfer * lsXfer, char *szLocalFile, char *szRemoteFile, struct stat *psLstat, struct stat *psRstat, char *szRhost);
// extern int doXferRcp (struct rcpXfer * lsXfer, int option);
// extern int rmDirAndFiles (char *dir);
// extern int rmDirAndFilesEx (char *, int);
// extern int createSpoolSubDir (const char *spoolFileFullPath);

int parseXferArg(char *arg, char **userName, char **hostName, char **fName);
int doXferRcp (struct rcpXfer *lsXfer, int option);
int createXfer(struct rcpXfer *lsXfer);
int destroyXfer(struct rcpXfer *lsXfer);
int equivalentXferFile(struct rcpXfer * lsXfer, const char *szLocalFile, const char *szRemoteFile, struct stat *psLstat, struct stat *psRstat, const char *szRhost);
int copyFile(struct rcpXfer *lsXfer, const char *buf, int option);
int rmDirAndFiles( const char *dir);
int rmDirAndFilesEx( const char *dir, int recur);
int createSpoolSubDir(const char *spoolFileFullPath);
