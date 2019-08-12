/*
 * $Id: lib.tid.c 397 2007-11-26 19:04:00Z mblack $ Copyright (C) 2007
 * Platform Computing Inc
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

// #define TID_BNUM   23
enum {
 TID_BNUM = 23
} TID_BNUM_;

#define tid_index(x)   (x%TID_BNUM)

struct tid *tid_buckets[TID_BNUM];


/* tid.c */
unsigned long tid_register(unsigned long taskid, int socknum, uint16_t taskPort, const char *host, bool_t doTaskInfo);
unsigned long tid_remove(unsigned long taskid);
struct tid *tid_find( unsigned long taskid);
struct tid *tidFindIgnoreConn_( unsigned long taskid);
void tid_lostconnection(int socknum);
int tidSameConnection_(int socknum, unsigned long *ntaskids, unsigned long **tidArray);

