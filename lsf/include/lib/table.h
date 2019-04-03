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

#define HTAB_ZERO_OUT(HashTab) \
{ \
    (HashTab)->numEnts = 0; \
    (HashTab)->size = 0; \
}

#define HTAB_NUM_ELEMENTS(HashTab) (HashTab)->numEnts

#define FOR_EACH_HTAB_ENTRY(Key, Entry, HashTab) \
{ \
struct sTab __searchPtr__; \
    (Entry) = h_firstEnt_((HashTab), &__searchPtr__); \
    for ((Entry) = h_firstEnt_((HashTab), &__searchPtr__); \
         (Entry); (Entry) = h_nextEnt_(&__searchPtr__)) { \
   (Key)   = (char *) (Entry)->keyname;

#define END_FOR_EACH_HTAB_ENTRY  }}

#define FOR_EACH_HTAB_DATA(Type, Key, Data, HashTab) \
{ \
struct sTab __searchPtr__; \
    struct hEnt *__hashEnt__; \
    __hashEnt__ = h_firstEnt_((HashTab), &__searchPtr__); \
    for (__hashEnt__ = h_firstEnt_((HashTab), &__searchPtr__); \
         __hashEnt__; __hashEnt__ = h_nextEnt_(&__searchPtr__)) { \
        (Data) = (Type *) __hashEnt__->hData; \
  (Key)   = (char *) __hashEnt__->keyname;

#define END_FOR_EACH_HTAB_DATA  }}

// #define RESETFACTOR     2
// #define RESETLIMIT      1.5
// #define DEFAULT_SLOTS   11
const unsigned int DEFAULT_SLOTS = 11;
const unsigned int RESETFACTOR   = 2;
const float        RESETLIMIT    = 1.500f;

/* Double linked list addressed by each
 * hash table slot.
 */
struct hLinks
{
  struct hLinks *fwPtr;
  struct hLinks *bwPtr;
};// hLinks;

/* This is a slot entry the table
 */
// typedef 
struct hEnt
{
  struct hLinks *fwPtr;
  struct hLinks *bwPtr;
  void *hData;
  const char *keyname;
};// hEnt;

/* This is the hash table itself.
 */
// typedef 
struct hTab
{
  struct hLinks *slotPtr;
  // long numEnts;
  size_t numEnts;
  size_t size;
}; // hTab;


//typedef 
struct sTab
{
  size_t nIndex;
  struct hEnt *hEntPtr;
  struct hTab *tabPtr;
  struct hLinks *hList;
}; // sTab;


typedef void (*HTAB_DATA_DESTROY_FUNC_T) (void *);

// unsigned long primes[] = {       // FIXME FIXME why the fuck are these primes here?
//                                         // and why the fuck don't we just generate them?
//     101, 1009, 5009, 10007, 20011, 50021, 100003,
//     200003, 500009, 1030637
// }; // defined but not used

struct hEnt *h_findEnt (const char *, struct hLinks *);
unsigned int getAddr (struct hTab *, const char *);
void resetTab ( struct hTab *);
size_t getClosestPrime (unsigned long x);

void insList_ (struct hLinks *, struct hLinks *);
void remList_ (struct hLinks *);
void initList_ (struct hLinks *);
void h_initTab_ ( struct hTab *tabPtr, unsigned long numSlots);
void h_freeTab_ ( struct hTab *, void (*destroy) (void *));
int h_TabEmpty_ ( struct hTab *);
void h_delTab_ ( struct hTab *);
struct hEnt *h_getEnt_ ( struct hTab *tabPtr, const char *key);
struct hEnt *h_addEnt_ ( struct hTab *, const char *, int *);
void h_delEnt_ ( struct hTab *, struct hEnt *);
void h_rmEnt_ ( struct hTab *, struct hEnt *);
struct hEnt *h_firstEnt_ ( struct hTab *, struct sTab *);
struct hEnt *h_nextEnt_ ( struct sTab *);
void h_freeRefTab_ ( struct hTab *);
void h_delRef_ ( struct hTab *, struct hEnt *);
