/* $Id: lib.table.h 397 2007-11-26 19:04:00Z mblack $
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

#define RESETFACTOR     2
#define RESETLIMIT      1.5
#define DEFAULT_SLOTS   11

/* Double linked list addressed by each
 * hash table slot.
 */
struct hLinks
{
  struct hLinks *fwPtr;
  struct hLinks *bwPtr;
};

/* This is a slot entry the table
 */
typedef struct hEnt
{
  struct hLinks *fwPtr;
  struct hLinks *bwPtr;
  void *hData;
  char *keyname;
} hEnt;

/* This is the hash table itself.
 */
typedef struct hTab
{
  struct hLinks *slotPtr;
  long numEnts;
  size_t size;
} hTab;


typedef struct sTab
{
  size_t nIndex;
  hEnt *hEntPtr;
  hTab *tabPtr;
  struct hLinks *hList;
} sTab;

#define HTAB_ZERO_OUT(HashTab) \
{ \
    (HashTab)->numEnts = 0; \
    (HashTab)->size = 0; \
}

#define HTAB_NUM_ELEMENTS(HashTab) (HashTab)->numEnts

#define FOR_EACH_HTAB_ENTRY(Key, Entry, HashTab) \
{ \
    sTab __searchPtr__; \
    (Entry) = h_firstEnt_((HashTab), &__searchPtr__); \
    for ((Entry) = h_firstEnt_((HashTab), &__searchPtr__); \
         (Entry); (Entry) = h_nextEnt_(&__searchPtr__)) { \
	 (Key)   = (char *) (Entry)->keyname;

#define END_FOR_EACH_HTAB_ENTRY  }}

#define FOR_EACH_HTAB_DATA(Type, Key, Data, HashTab) \
{ \
    sTab __searchPtr__; \
    hEnt *__hashEnt__; \
    __hashEnt__ = h_firstEnt_((HashTab), &__searchPtr__); \
    for (__hashEnt__ = h_firstEnt_((HashTab), &__searchPtr__); \
         __hashEnt__; __hashEnt__ = h_nextEnt_(&__searchPtr__)) { \
        (Data) = (Type *) __hashEnt__->hData; \
	(Key)   = (char *) __hashEnt__->keyname;

#define END_FOR_EACH_HTAB_DATA  }}

typedef void (*HTAB_DATA_DESTROY_FUNC_T) (void *);

void insList_ (struct hLinks *, struct hLinks *);
void remList_ (struct hLinks *);
void initList_ (struct hLinks *);
void h_initTab_ (hTab * tabPtr, unsigned long numSlots);
void h_freeTab_ (hTab *, void (*destroy) (void *));
int h_TabEmpty_ (hTab *);
void h_delTab_ (hTab *);
hEnt *h_getEnt_ (hTab *tabPtr, const char *key);
hEnt *h_addEnt_ (hTab *, const char *, int *);
void h_delEnt_ (hTab *, hEnt *);
void h_rmEnt_ (hTab *, hEnt *);
hEnt *h_firstEnt_ (hTab *, sTab *);
hEnt *h_nextEnt_ (sTab *);
void h_freeRefTab_ (hTab *);
void h_delRef_ (hTab *, hEnt *);

