/* $Id: usleep.c 397 2007-11-26 19:04:00Z mblack $
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

#include <sys/types.h>
#include <sys/time.h>

// minnum input for this function is 0
// what is the max? why doesn't it return the select statement result?
//      max is the max time to wait to connect to a server process
//      in milliseconds

// void millisleep( int waittime )
//      use select() to wait for msec milliseconds
void
millisleep_ (int msec)
{
  	struct timeval dtime;

  	if (msec < 1)  {
    	return;
  	}

  	dtime.tv_sec = msec / 1000;
  	/*
		liblsf/usleep.c:35:49: error: implicit conversion loses integer precision: 'long' to '__darwin_suseconds_t' (aka 'int')
      	[-Werror,-Wshorten-64-to-32]
        dtime.tv_usec = (msec - dtime.tv_sec * 1000) * (long)1000;

        I will have to live with this, cuz i do not know how to fix it in an orderly fashiopn
  	*/
  	dtime.tv_usec = (msec - dtime.tv_sec * 1000) * 1000; // FIXME FIXME

    select (0, 0, 0, 0, &dtime); // select() was? the only way to get millisecond
                                 // accurassy in Unix

}
