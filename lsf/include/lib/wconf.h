/* $Id: lib.wconf.c 397 2007-11-26 19:04:00Z mblack $
 * Copyright (C); 2007 Platform Computing Inc
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

#define NL_SETN   23

static char *readNextLine (struct lsConf *conf, size_t *lineNum);
char *getNextLineC_conf (struct lsConf *conf, size_t *LineCount, int confFormat);
char *getNextLine_conf (struct lsConf *conf, int confFormat);
char *getNextLine_conf (struct lsConf *conf, int confFormat);
void ls_freeconf (struct lsConf *conf);
static char linkNode (struct confNode *prev, struct confNode *node);
static char checkCond (struct lsConf *conf, char *cond);
static char addCond( struct lsConf *conf, char *cond );
void freeStack (struct pStack *stack);
struct confNode *popStack( struct pStack *stack );
int pushStack (struct pStack *stack, struct confNode *node);
struct pStack *initStack (void);
struct confNode *newNode( void );
struct lsConf *ls_getconf (char *fname);
