/* $Id: lib.so.c 397 2007-11-26 19:04:00Z mblack $
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


#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "lib/so.h"

int
soOpen_ ( const char *libFileName )
{
	int strcmp_rvalue = 0;
    strcmp_rvalue = strcmp( libFileName, "");
 
    return strcmp_rvalue;
}


void
soClose_( int handle )
{
    // SO_HANDLE_T is int, typedef'ed
    assert( handle );
}


void *
soSym_( int handle, const char *entryName )
{
	// int strcmp_rvalue = 0;
    // SO_HANDLE_T is int, typedef'ed
    assert( handle );
    assert( entryName );
    // strcmp_rvalue = strcmp( entryName, "");
 
    return NULL;
}
