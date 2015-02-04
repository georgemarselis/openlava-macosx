/*
 * $Id: lib.comm.c 397 2007-11-26 19:04:00Z mblack $ Copyright (C) 2007
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

void ls_minit( void );
int  ls_getrpid( void );
long ls_sndmsg (int tid, char *buf, size_t count);
long ls_rcvmsg (int tid, char *buf, size_t count);