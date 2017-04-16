/* $Id: lib.term.c 397 2007-11-26 19:04:00Z mblack $
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

// #include <linux/termios.h>
#include <termios.h>


#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/term.h"
#include "daemons/libresd/resout.h"


int encodeTermios_( XDR *xdrs, struct termios *ptr_termios )
{
	speed_t speed_value = 0;
	unsigned int i = 0;

	if (!encode_mode (xdrs, ptr_termios->c_iflag, in_table, IN_TABLE_SIZE ) ) {
		return FALSE;
	}

	if (!encode_mode (xdrs, ptr_termios->c_oflag, out_table, OUT_TABLE_SIZE)) {
		return FALSE;
	}

	if( !( encode_mode (xdrs, ptr_termios->c_cflag, ctrl_table, CTRL_TABLE_SIZE ) && 
		 encode_mode (xdrs, ptr_termios->c_lflag, loc_table, LOC_TABLE_SIZE ) )
	) {
		return FALSE;
	}

	speed_value = cfgetospeed (ptr_termios);
	for( unsigned int i = 0; i < BAUD_TABLE_SIZE; i++) {
		if( speed_value == baud_table[i] ) {
			break;
		}
	}

	if( i == BAUD_TABLE_SIZE ) {
		i = 0;
	}

	if( !xdr_u_int( xdrs, &i ) ) {
		return FALSE;
	}

	for( i = 0; i < CHR_TABLE_SIZE; i++ ) {

		if (i < CHR_TABLE_SPLIT) {
			if( !xdr_u_char( xdrs, &ptr_termios->c_cc[ chr_table[i] ] ) ) {
				return FALSE;
			}
		}
		else {
			if (!xdr_u_char( xdrs, &ptr_termios->c_cc[ chr_table[i] ] ) )
				return FALSE;
		}
	}

	return TRUE;
}

int encode_mode( XDR *xdrs, tcflag_t mode_set, tcflag_t *attr_table, unsigned int table_count )
{

	int encode_set = 0;
	unsigned int i = 0;

	for( encode_set = 0, i = 0; i < table_count; i++ ) {
		if (attr_table[i] != (tcflag_t) 0 && (mode_set & attr_table[i]) == attr_table[i]) {
			encode_set |= 1 << i;
		}
	}

	return xdr_int( xdrs, &encode_set );
}

int decodeTermios_( XDR *xdrs, struct termios *ptr_termios )
{
	speed_t speed_value = 0;
	unsigned int i = 0;

	if( !decode_mode( xdrs, in_table, IN_TABLE_SIZE, &ptr_termios->c_iflag ) ) {
		return FALSE;
	}

	if( !decode_mode( xdrs, out_table, OUT_TABLE_SIZE, &ptr_termios->c_oflag ) ) {
		return FALSE;
	}

	if( !(	decode_mode (xdrs, ctrl_table, CTRL_TABLE_SIZE, &ptr_termios->c_cflag ) && 
			decode_mode (xdrs, loc_table, LOC_TABLE_SIZE, &ptr_termios->c_lflag )
		 )
	)
	return FALSE;


	if( !xdr_u_int( xdrs, &i ) ) {
		return FALSE;
	}

	speed_value = baud_table[i];
	cfsetospeed (ptr_termios, speed_value);


	for (i = 0; i < NCCS; i++) {
		ptr_termios->c_cc[i] = _POSIX_VDISABLE;
	}


	for( unsigned int i = 0; i < CHR_TABLE_SIZE; i++ ) {
		if (i < CHR_TABLE_SPLIT) {
			if (!xdr_u_char (xdrs, &ptr_termios->c_cc[ chr_table[i] ] ) ) {
				return FALSE;
			}
		}
		else {
			if (!xdr_u_char (xdrs, &ptr_termios->c_cc[ chr_table[i] ] ) ) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

int decode_mode( XDR *xdrs, tcflag_t *attr_table, unsigned int table_count, tcflag_t *mode_set )
{
	int encode_set = 0;
	unsigned int i = 0;

	if( !xdr_int( xdrs, &encode_set ) ) {
		return FALSE;
	}

	for( *mode_set = 0, i = 0; i < table_count; i++ ) {
		if( encode_set & ( 1 << i ) ) {
			*mode_set |= attr_table[i];
		}
	}

	return TRUE;
}
