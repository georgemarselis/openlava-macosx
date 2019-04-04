/* $Id: lib.hdr.h 397 2007-11-26 19:04:00Z mblack $
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

// FIXME FIXME FIXME FIXME FIXME prc headers have been removed from glibc ever since version 2.14. using tinyrpc now, but this should differenciate between different versionf of glibc library
#include <tirpc/rpc/types.h>
#ifndef __XDR_HEADER__
#include <tirpc/rpc/xdr.h>
#endif

#include <sys/stat.h>

#include "lsf.h"

// openlava 2.0 header breaks compatibility with 1.0 but offers more flexibility and room for growth
struct LSFHeader { // FIXME FIXME FIXME FIXME FIXME ensure compatibility with Platform LSF
    unsigned short version;
    unsigned short reserved0;
    unsigned short reserved;
    char paddin1[2];
    size_t length;
    short opCode;
    char   padding2[2];
    pid_t  refCode; 
};

/* always use this macro to size up memory buffers
 * for protocol header.
 */
// #define LSF_HEADER_LEN (sizeof(struct LSFHeader))
const unsigned int LSF_HEADER_LEN = sizeof( struct LSFHeader ) ;

struct stringLen {
    const char *name;
    size_t len;
};

struct lenData {
    size_t len;
    char *data;
};

// #define AUTH_HOST_NT  0x01
// #define AUTH_HOST_UX  0x02
enum AUTH_HOST {
    AUTH_HOST_NT = 0x01,
    AUTH_HOST_UX = 0x02 
} ;

#define EAUTH_SIZE 4096 // FIXME FIXME FIXME FIXME FIXME set value from configure.ac
// const unsigned int EAUTH_SIZE = 4096;
struct lsfAuth
{
    uid_t uid;
    uid_t gid;
    char lsfUserName[MAX_LSF_NAME_LEN]; // FIXME FIXME FIXME change to pointer
    enum { CLIENT_SETUID, CLIENT_IDENT, CLIENT_DCE, CLIENT_EAUTH } kind;
    int options;

    union authBody {
        struct eauth {
            size_t len;
            char data[EAUTH_SIZE];
        } eauth;
        int filler;
    
        struct lenData authToken;
    } k;
};


struct lsfLimit
{
    rlim_t rlim_maxh;
    rlim_t rlim_curl;
    rlim_t rlim_curh;
    rlim_t rlim_maxl;
};


