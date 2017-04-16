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

#define NODE_LEFT_DONE  1
#define NODE_ALL_DONE   2
#define NODE_PASED      3

char *getNextValueQ_( char **line, char ch1, char ch2 );
char *getNextValueQ_( char **line, char ch1, char ch2 );
char *getNextWord_( char **line );
char *getNextLineC_( FILE * p, size_t *LineCount, int confFormat );

#define PUSH_STACK(s, n) { if( pushStack( s, n ) < 0 ) { goto Error; } }

struct pStack *blockStack;
struct pStack *ptrStack;

struct confNode *newNode (void);
void freeNode (struct confNode *);
char linkNode (struct confNode *, struct confNode *);
char *readNextLine (struct lsConf *conf, size_t *lineNum);

char addCond (struct lsConf *, char *);
char checkCond (struct lsConf *, char *);
