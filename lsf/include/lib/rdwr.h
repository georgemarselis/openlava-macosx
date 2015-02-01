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

#include <unistd.h>

long 	nb_write_fix (int s, char *buf, size_t len);
long   	nb_read_fix  (int s, char *buf, size_t len);
long 	b_read_fix   (int s, char *buf, size_t len);
long 	b_write_fix  (int s, char *buf, size_t len);

void unblocksig (int sig);
int  b_connect_ (int s, struct sockaddr *name, socklen_t namelen, unsigned int timeout);
int  rd_select_ (int rd, struct timeval *timeout);
int  b_accept_  (int s, struct sockaddr *addr, socklen_t * addrlen);
long nb_read_timeout (int s, char *buf, size_t len, int timeout);
