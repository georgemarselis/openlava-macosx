/* $Id: echkpnt.env.h 397 2007-11-26 19:04:00Z mblack $
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
 
#ifndef LSF_CHECKPOINT_ENVIRONMENT_H
#define LSF_CHECKPOINT_ENVIRONMENT_H

#define LSF_CHECKPOINT_VAR_FILE 		"/echeckpoint.var"

typedef struct varpair
{
  char *m_pVariable;
  char *m_pValue;
} VAR_PAIR_T;

typedef struct varTableItem
{
  VAR_PAIR_T *m_pVarPair;
  struct varTableItem *m_pNextItem;
} VAR_TABLE_ITEM_T;


char *getCheckpointVar (const char *);
int writeCheckpointVar (const char *, const char *);
int fileIsExist (const char *);

void freeTable_ ();

#endif
