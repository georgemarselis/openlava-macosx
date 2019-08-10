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

/* tid.c */
int tid_register(pid_t taskid, int socknum, u_short taskPort, const char *host, bool_t doTaskInfo);
int tid_remove(unsigned int taskid);
struct tid *tid_find( pid_t taskid);
struct tid *tidFindIgnoreConn_( pid_t taskid);
void tid_lostconnection(int socknum);
int tidSameConnection_(int socknum, unsigned int *ntids, unsigned int **tidArray);

