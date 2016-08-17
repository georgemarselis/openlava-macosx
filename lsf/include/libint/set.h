/* $Id: set.h 397 2007-11-26 19:04:00Z mblack $
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

struct listSet
{
  int elem;
  struct listSet *next;
};

void listSetFree (struct listSet *);
struct listSet *listSetAlloc (int);
int listSetEqual (struct listSet *, struct listSet *);
struct listSet *listSetUnion (struct listSet *, struct listSet *);
struct listSet *listSetIntersect (struct listSet *, struct listSet *);
struct listSet *listSetDuplicate (struct listSet *);
int listSetIn (int, struct listSet *);
struct listSet *listSetInsert (int, struct listSet *);
struct listSet *listSetSub (struct listSet *, struct listSet *);

