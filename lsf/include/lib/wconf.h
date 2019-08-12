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

// #define NL_SETN   23
int  NL_SETN = 23;

struct pStack *blockStack;
struct pStack *ptrStack;

/* wconf.c */
struct lsConf *ls_getconf(const char *filename);
struct confNode *newNode(void);
struct pStack *initStack(void);
int pushStack(struct pStack *stack, struct confNode *node);
struct confNode *popStack(struct pStack *stack);
void freeStack(struct pStack *stack);
char addCond(struct lsConf *conf, char *cond);
char checkCond(struct lsConf *conf, char *cond);
char linkNode(struct confNode *prev, struct confNode *node);
void ls_freeconf(struct lsConf *conf);
void freeNode(struct confNode *node);
char *readNextLine( struct lsConf *conf, size_t *lineNum);
