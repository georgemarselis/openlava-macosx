/* $Id: lib.eauth.c 397 2007-11-26 19:04:00Z mblack $
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

#include <sys/time.h>

// #define IO_TIMEOUT  2000
const unsigned short IO_TIMEOUT = 2000;


#define US_DIFF(t1, t2) ((t1->tv_sec - t2->tv_sec) * 1000000 + t1->tv_usec - t2->tv_usec )

// int  b_connect_ (int s, struct sockaddr *name, socklen_t namelen, unsigned int timeout);
// int  rd_select_ (int rd, struct timeval *timeout);
// int  b_accept_  (int s, struct sockaddr *addr, socklen_t * addrlen);
// long nb_read_timeout (int s, char *buf, size_t len, int timeout);
void unblocksig (int sig);
void alarmer_ (void);
