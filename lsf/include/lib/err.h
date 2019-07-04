/*
 * Copyright (C) 2011-2012 David Bigagli Copyright (C) 2007 Platform
 * Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of version 2 of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#pragma once

// static int lserrno = LSE_NO_ERR;
int masterLimDown = FALSE;
// static int ls_nerr = LSE_NERR;

const char *ls_errmsg[] = {
    /* 0 */ "No error",
    /* 1 */ "XDR operation error",
    /* 2 */ "Failed in sending/receiving a message",
    /* 3 */ "Bad arguments",
    /* 4 */ "Cannot locate master LIM now, try later",
    /* 5 */ "LIM is down; try later",
    /* 6 */ "LIM protocol error",
    /* 7 */ "A socket operation has failed",
    /* 8 */ "Failed in an accept system call",
    /* 9 */ "Bad LSF task configuration file format",
    /* 10 */ "Not enough host(s) currently eligible",
    /* 11 */ "No host is eligible",
    /* 12 */ "Communication time out",
    /* 13 */ "Nios has not been started",
    /* 14 */ "Operation permission denied by LIM",
    /* 15 */ "Operation ignored by LIM",
    /* 16 */ "Host name not recognizable by LIM",
    /* 17 */ "Host already locked",
    /* 18 */ "Host was not locked",
    /* 19 */ "Unknown host model",
    /* 20 */ "A signal related system call failed",
    /* 21 */ "Bad resource requirement syntax",
    /* 22 */ "No remote child",
    /* 23 */ "Memory allocation failed",
    /* 24 */ "Unable to open file lsf.conf",
    /* 25 */ "Bad configuration environment, something missing in lsf.conf?",
    /* 26 */ "Lim is not a registered service",
    /* 27 */ "Res is not a registered service",
    /* 28 */ "RES is serving too many connections",
    /* 29 */ "Bad user ID",
    /* 30 */ "Root user rejected",
    /* 31 */ "User permission denied",
    /* 32 */ "Bad operation code",
    /* 33 */ "Protocol error with RES",
    /* 34 */ "RES callback fails; see RES error log for more details",
    /* 35 */ "RES malloc fails",
    /* 36 */ "Fatal error in RES; check RES error log for more details",
    /* 37 */ "RES cannot alloc pty",
    /* 38 */ "RES cannot allocate socketpair as stdin/stdout/stderr for task",
    /* 39 */ "RES fork fails",
    /* 40 */ "Running out of privileged socks",
    /* 41 */ "getwd failed",
    /* 42 */ "Connection is lost",
    /* 43 */ "No such remote child",
    /* 44 */ "Permission denied",
    /* 45 */ "Ptymode inconsistency on ls_rtask",
    /* 46 */ "Bad host name",
    /* 47 */ "NIOS protocol error",
    /* 48 */ "A wait system call failed",
    /* 49 */ "Bad parameters for setstdin",
    /* 50 */ "Insufficient list length for returned rpids",
    /* 51 */ "Invalid cluster name",
    /* 52 */ "Incompatible versions of tty params",
    /* 53 */ "Failed in a execv() system call",
    /* 54 */ "No such directory",
    /* 55 */ "Directory may not be accessible",
    /* 56 */ "Invalid service Id",
    /* 57 */ "Request from a non-LSF host rejected",
    /* 58 */ "Unknown resource name",
    /* 59 */ "Unknown resource value",
    /* 60 */ "Task already exists",
    /* 61 */ "Task does not exist",
    /* 62 */ "Task table is full",
    /* 63 */ "A resource limit system call failed",
    /* 64 */ "Bad index name list",
    /* 65 */ "LIM malloc failed",
    /* 66 */ "NIO not initialized",
    /* 67 */ "Bad syntax in lsf.conf",
    /* 68 */ "File operation failed",
    /* 69 */ "A connect sys call failed",
    /* 70 */ "A select system call failed",
    /* 71 */ "End of file",
    /* 72 */ "Bad lsf accounting record format",
    /* 73 */ "Bad time specification",
    /* 74 */ "Unable to fork child",
    /* 75 */ "Failed to setup pipe",
    /* 76 */ "Unable to access esub/eexec file",
    /* 77 */ "External authentication failed",
    /* 78 */ "Cannot open file",
    /* 79 */ "Out of communication channels",
    /* 80 */ "Bad communication channel",
    /* 81 */ "Internal library error",
    /* 82 */ "Protocol error with server",
    /* 83 */ "A system call failed",
    /* 84 */ "Failed to get rusage",
    /* 85 */ "No shared resources",
    /* 86 */ "Bad resource name",
    /* 87 */ "Failed to contact RES parent",
    /* 88 */ "i18n setlocale failed",
    /* 89 */ "i18n catopen failed",
    /* 90 */ "i18n malloc failed",
    /* 91 */ "Cannot allocate memory",
    /* 92 */ "Close a NULL-FILE pointer",
    /* 93 */ "Master LIM is down; try later",
    /* 94 */ "Requested label is not valid",
    /* 95 */ "Requested label is above your allowed range",
    /* 96 */ "Request label rejected by /etc/rhost.conf",
    /* 97 */ "Request label doesn't dominate current label",
    /* 98 */ "Migrant host already known to master LIM"
};

enum ERRORS { 
    NOSUCHTHING = 0,

    /***************************************
     * following errors are in MacOS 10.13
     */
    // EPERM,   // exists already /usr/include/sys/errno.h /* Operation not permitted */
    // ENOENT,  // exists already /usr/include/sys/errno.h /* No such file or directory */
    // ESRCH,   // exists already /usr/include/sys/errno.h /* No such process */
    // EINTR,   // exists already /usr/include/sys/errno.h /* Interrupted system call */
    // EIO,     // exists already /usr/include/sys/errno.h /* Input/output error */
    // ENXIO,   // exists already /usr/include/sys/errno.h /* Device not configured */
    // E2BIG,   // exists already /usr/include/sys/errno.h /* Argument list too long */
    // ENOEXEC, // exists already /usr/include/sys/errno.h /* Exec format error */
    // EBADF,   // exists already /usr/include/sys/errno.h /* Bad file descriptor */
    // ECHILD,  // exists already /usr/include/sys/errno.h /* No child processes */
    // EAGAIN,  // exists already /usr/include/sys/errno.h /* Resource temporarily unavailable */
    // ENOMEM,  // exists already /usr/include/sys/errno.h /* Cannot allocate memory */
    // EACCES,  // exists already /usr/include/sys/errno.h /* Permission denied */
    // EFAULT,  // exists already /usr/include/sys/errno.h /* Bad address */
    // NOTBLK,  // exists already /usr/include/sys/errno.h /* Block device required */
    // EBUSY,   // exists already /usr/include/sys/errno.h /* Device / Resource busy */
    // EEXIST,  // exists already /usr/include/sys/errno.h /* File exists */
    // EXDEV,   // exists already /usr/include/sys/errno.h /* Cross-device link */
    // ENODEV,  // exists already /usr/include/sys/errno.h /* Operation not supported by device */
    // ENOTDIR,
    // EISDIR,
    // EINVAL,
    // ENFILE,
    // EMFILE,
    // ENOTTY,
    // ETXTBSY,
    // EFBIG,
    // ENOSPC,
    // ESPIPE,
    // EROFS,
    // EMLINK,
    // EPIPE,
    // EDOM,
    // ERANGE,
    // EWOULDBLOCK,
    // EINPROGRESS,
    // EALREADY,
    // ENOTSOCK,
    // EDESTADDRREQ,
    // EMSGSIZE,
    // EPROTOTYPE,
    // ENOPROTOOPT,
    // EPROTONOSUPPORT,
    // ESOCKTNOSUPPORT,
    // EOPNOTSUPP,
    // EPFNOSUPPORT,
    // EAFNOSUPPORT,
    // EADDRINUSE,
    // EADDRNOTAVAIL,
    // ENETDOWN,
    // ENETUNREACH,
    // ENETRESET,
    // ECONNABORTED,
    // ECONNRESET,
    // ENOBUFS,
    // EISCONN,
    // ENOTCONN,
    // ESHUTDOWN,
    // ETOOMANYREFS,
    // ETIMEDOUT,
    // ECONNREFUSED,
    // ELOOP,
    // ENAMETOOLONG,
    // EHOSTDOWN,
    // EHOSTUNREACH,
    // ENOTEMPTY,
    // ESTALE,
    // EREMOTE,
    // EDEADLK,
    // ENOLCK,
    // ENOSYS,
    ENORESNAME = 4096,
    // ENOHOSTADDED 
    ELASTVALUE
};

enum flag{constant1, constant2, constant3 };

// enum ERRORS {
//     IGNOREME = 0,
//     EPERM, // /usr/include/asm-generic/errno-base.h #define EPERM            1      /* Operation not permitted */
//     ENOENT = 2,
//     ESRCH,
//     EINTR,
//     EIO,
//     ENXIO,
//     E2BIG,
//     ENOEXEC,
//     EBADF,
//     ECHILD,
//     EAGAIN,
//     ENOMEM,
//     EACCES,
//     EFAULT,
//     ENOTBLK,
//     EBUSY,
//     EEXIST,
//     EXDEV,
//     ENODEV,
//     ENOTDIR,
//     EISDIR,
//     EINVAL,
//     ENFILE,
//     EMFILE,
//     ENOTTY,
//     ETXTBSY,
//     EFBIG,
//     ENOSPC,
//     ESPIPE,
//     EROFS,
//     EMLINK,
//     EPIPE,
//     EDOM,
//     ERANGE,
//     EWOULDBLOCK,
//     EINPROGRESS,
//     EALREADY,
//     ENOTSOCK,
//     EDESTADDRREQ,
//     EMSGSIZE,
//     EPROTOTYPE,
//     ENOPROTOOPT,
//     EPROTONOSUPPORT,
//     ESOCKTNOSUPPORT,
//     EOPNOTSUPP,
//     EPFNOSUPPORT,
//     EAFNOSUPPORT,
//     EADDRINUSE,
//     EADDRNOTAVAIL,
//     ENETDOWN,
//     ENETUNREACH,
//     ENETRESET,
//     ECONNABORTED,
//     ECONNRESET,
//     ENOBUFS,
//     EISCONN,
//     ENOTCONN,
//     ESHUTDOWN,
//     ETOOMANYREFS,
//     ETIMEDOUT,
//     ECONNREFUSED,
//     ELOOP,
//     ENAMETOOLONG,
//     EHOSTDOWN,
//     EHOSTUNREACH,
//     ENOTEMPTY,
//     ESTALE,
//     EREMOTE,
//     EDEADLK,
//     ENOLCK,
//     ENOSYS,
//     ENOHOSTADDED,
//     ELASTVALUE
// };

enum    ENO {
    ENOHOSTADDED = 4095,
    // ENORESNAME = 4096
};


// const 
// char *err_str_(int errnum, const char *fmt, char *buf);
int errnoEncode_ (int eno);
int errnoDecode_ (int eno);
/* lib/liblsf/err.c */
void ls_errlog(FILE *fp, const char *fmt, ...);
char *err_str_(unsigned int errnum, const char *fmt, char *buf);
void verrlog_(int level, FILE *fp, const char *fmt, va_list ap);
char *ls_sysmsg(void);
void ls_perror(char *usrMsg);
