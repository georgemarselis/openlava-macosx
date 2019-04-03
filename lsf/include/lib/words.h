/* $Id: lib.words.h 397 2007-11-26 19:04:00Z mblack $
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

#define PUSH_STACK(s, n) { if( pushStack( s, n ) < 0 ) { goto Error; } }

// #define NODE_LEFT_DONE  1
// #define NODE_ALL_DONE   2
// #define NODE_PASED      3
enum NODE {
	NODE_LEFT_DONE = 1,
	NODE_ALL_DONE,
	NODE_PASED
};

/* lib/liblsf/words.c */
char *getNextWord_  ( const char **line);
char *getNextWord1_ ( const char **line );
char *a_getNextWord_( const char **line );
char *getNextWordSet( char **line, const char *set );
int   charInSet     ( char c, const char *set );
char *getNextValueQ_( const char **line, char ch1, char ch2 );
int   stripQStr     ( char *q, char *str );
int   addQStr       ( FILE *log_fp, char *str );
void  subNewLine_   ( char *instr );
char *nextline_     ( FILE *fp );