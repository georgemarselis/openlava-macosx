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
