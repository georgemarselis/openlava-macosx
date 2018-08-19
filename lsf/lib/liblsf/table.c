/*
 * Copyright (C) 2011 David Bigagli
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

#include <stdio.h>


#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/table.h"

/* insList_()
 * Add the elemPtr in the list at destPtr address.
 */
void
insList_ (struct hLinks *elemPtr, struct hLinks *destPtr)
{
    elemPtr->bwPtr = destPtr->bwPtr;
    elemPtr->fwPtr = destPtr;
    destPtr->bwPtr->fwPtr = elemPtr;
    destPtr->bwPtr = elemPtr;
}

void
remList_ (struct hLinks *elemPtr)
{

    if (elemPtr == NULL || elemPtr == elemPtr->bwPtr || !elemPtr)
        {
            return;
        }

    elemPtr->fwPtr->bwPtr = elemPtr->bwPtr;
    elemPtr->bwPtr->fwPtr = elemPtr->fwPtr;

}

void
initList_ (struct hLinks *headPtr)
{
    headPtr->bwPtr = headPtr;
    headPtr->fwPtr = headPtr;

}

void
h_initTab_ (hTab * tabPtr, unsigned long numSlots)
{
    struct hLinks *slotPtr = tabPtr->slotPtr;
    unsigned long getClosestPrimeResult = 0;

    tabPtr->numEnts = 0;

    /* Our hash table works best if we have its size
     * as prime number.
     */
    // FIXME FIXME generate highest-needed prime number?
    getClosestPrimeResult = getClosestPrime (numSlots);
    tabPtr->size = getClosestPrimeResult;
    tabPtr->slotPtr = malloc (sizeof (struct hLinks) * tabPtr->size);
    for ( size_t i = 0; i < tabPtr->size; i++, slotPtr++) {
        initList_ (slotPtr);
    }

}

void
h_freeTab_ (hTab * tabPtr, void (*freeFunc) (void *))
{
    struct hLinks *hTabEnd = NULL;
    struct hLinks *slotPtr = NULL;
    struct hEnt *hEntPtr = NULL;

    slotPtr = tabPtr->slotPtr;
    hTabEnd = &(slotPtr[tabPtr->size]);

    for (; slotPtr < hTabEnd; slotPtr++)
        {

            while (slotPtr != slotPtr->bwPtr)
    {

        hEntPtr = (hEnt *) slotPtr->bwPtr;
        remList_ ((struct hLinks *) hEntPtr);
        FREEUP (hEntPtr->keyname);

        if (hEntPtr->hData != NULL)
            {
                if (freeFunc != NULL)
        (*freeFunc) ((void *) hEntPtr->hData);
                else
        {
            free (hEntPtr->hData);
            hEntPtr->hData = NULL;
        }
            }

        free (hEntPtr);
    }
        }

    free ((char *) tabPtr->slotPtr);
    tabPtr->slotPtr = (struct hLinks *) NULL;
    tabPtr->numEnts = 0;
}

int
h_TabEmpty_ (hTab *tabPtr)
{
    return tabPtr->numEnts == 0;
}

void
h_delTab_ (hTab *tabPtr)
{
    h_freeTab_ (tabPtr, (HTAB_DATA_DESTROY_FUNC_T) NULL);
}

/* h_getEnt_()
 * Get an entry from the hash table based on
 * a given key.
 */
struct hEnt *
h_getEnt_ (hTab *tabPtr, const char *key)
{
    if (tabPtr->numEnts == 0) {
        return NULL;
    }

    return (h_findEnt (key, &(tabPtr->slotPtr[getAddr (tabPtr, key)])));

}

/* h_addEnt_()
 * Add an entry to a previously created hash table.
 */
struct hEnt *
h_addEnt_ (hTab *tabPtr, const char *key, int *newPtr)
{
    struct hEnt *hEntPtr = NULL;
    char *keyPtr = key;
    struct hLinks *hList = NULL;

    hList = &(tabPtr->slotPtr[getAddr (tabPtr, keyPtr)]);
    hEntPtr = h_findEnt ((char *) keyPtr, hList);

    if (NULL != hEntPtr && NULL != newPtr) {
        *newPtr = FALSE;
        return hEntPtr;
    }

    if (tabPtr->numEnts >= RESETLIMIT * tabPtr->size) {
        resetTab (tabPtr);
        hList = &(tabPtr->slotPtr[getAddr (tabPtr, (char *) keyPtr)]);
    }

    /* Create a new entry and increase the counter
     * of entries.
     */
    hEntPtr = malloc (sizeof (hEnt));
    hEntPtr->keyname = putstr_ ((char *) keyPtr);
    hEntPtr->hData = NULL;
    insList_ ((struct hLinks *) hEntPtr, hList);
    if (newPtr != NULL)
        *newPtr = TRUE;
    tabPtr->numEnts++;

    return hEntPtr;

}

/* h_delEnt_()
 */
void
h_delEnt_ (hTab * tabPtr, hEnt * hEntPtr)
{

    if (hEntPtr != NULL)
        {

            remList_ ((struct hLinks *) hEntPtr);
            free (hEntPtr->keyname);
            if (hEntPtr->hData != NULL)
    free ((char *) hEntPtr->hData);
            free ((char *) hEntPtr);
            tabPtr->numEnts--;
        }

}

/* h_rmEnt_()
 */
void
h_rmEnt_ (hTab * tabPtr, hEnt * hEntPtr)
{

    if (hEntPtr != (hEnt *) NULL)
        {
            remList_ ((struct hLinks *) hEntPtr);
            free (hEntPtr->keyname);
            free ((char *) hEntPtr);
            tabPtr->numEnts--;
        }

}


/* h_firstEnt_()
 * Get the first element from the hash table.
 * Starting the iteration on the table itself.
 */
struct hEnt *
h_firstEnt_ (hTab * tabPtr, sTab * sPtr)
{

    sPtr->tabPtr = tabPtr;
    sPtr->nIndex = 0;
    sPtr->hEntPtr = NULL;

    if (tabPtr->slotPtr)
        {
            return h_nextEnt_ (sPtr);
        }
    else
        {

            return (NULL);
        }

}

/* h_nextEnt_()
 * Get the next entry from the hash table till it is
 * empty.
 */
struct hEnt *
h_nextEnt_ (sTab * sPtr)
{
    struct hLinks *hList = NULL;
    struct hEnt *hEntPtr = NULL;

    hEntPtr = sPtr->hEntPtr;

    while (hEntPtr == NULL || (struct hLinks *) hEntPtr == sPtr->hList)
    {
        if (sPtr->nIndex >= sPtr->tabPtr->size) {
            return ((hEnt *) NULL);
        }

        hList = &(sPtr->tabPtr->slotPtr[sPtr->nIndex]);
        sPtr->nIndex++;
        if (hList != hList->bwPtr)
        {
            hEntPtr = (hEnt *) hList->bwPtr;
            sPtr->hList = hList;
            break;
        }

    }

    sPtr->hEntPtr = (hEnt *) ((struct hLinks *) hEntPtr)->bwPtr;

    return hEntPtr;

}

/* getAddr()
 * Compute a hash index, almost right from K&R
 */
static unsigned int
getAddr (hTab * tabPtr, const char *key)
{
    unsigned int hashIndex= 0;

    while (*key) {
        hashIndex = (hashIndex * 128 + (unsigned int)(*key++) ) % tabPtr->size;
    }

    return hashIndex;

}

static hEnt *
h_findEnt (const char *key, struct hLinks *hList)
{
    struct hEnt *hEntPtr = NULL;

    for (hEntPtr = (hEnt *) hList->bwPtr;
             hEntPtr != (hEnt *) hList;
             hEntPtr = (hEnt *) ((struct hLinks *) hEntPtr)->bwPtr)
        {
            if (strcmp (hEntPtr->keyname, key) == 0) {
                return hEntPtr;
            }
        }

    return NULL;

}

/* resetTab()
 */
static void
resetTab (hTab * tabPtr)
{
    size_t lastSize = 0;
    unsigned int slot = 0;
    struct hLinks *lastSlotPtr;
    struct hLinks *lastList;
    struct hEnt *hEntPtr = NULL;

    lastSlotPtr = tabPtr->slotPtr;
    lastSize = tabPtr->size;

    h_initTab_ (tabPtr, tabPtr->size * RESETFACTOR);

    for (lastList = lastSlotPtr; lastSize > 0; lastSize--, lastList++)
        {
            while (lastList != lastList->bwPtr)
    {
        hEntPtr = (hEnt *) lastList->bwPtr;
        remList_ ((struct hLinks *) hEntPtr);
        slot = getAddr (tabPtr, (char *) hEntPtr->keyname);
        insList_ ((struct hLinks *) hEntPtr,
                (struct hLinks *) (&(tabPtr->slotPtr[slot])));
        tabPtr->numEnts++;
    }
        }

    free (lastSlotPtr);

}

void
h_delRef_ (hTab * tabPtr, hEnt * hEntPtr)
{
    if (hEntPtr != (hEnt *) NULL)
        {
            remList_ ((struct hLinks *) hEntPtr);
            free (hEntPtr->keyname);
            free (hEntPtr);
            tabPtr->numEnts--;
        }

}

void
h_freeRefTab_ (hTab * tabPtr)
{
    struct hLinks *hTabEnd, *slotPtr;
    struct hEnt *hEntPtr = NULL;

    slotPtr = tabPtr->slotPtr;
    hTabEnd = &(slotPtr[tabPtr->size]);

    for (; slotPtr < hTabEnd; slotPtr++)
        {

            while (slotPtr != slotPtr->bwPtr)
    {

        hEntPtr = (hEnt *) slotPtr->bwPtr;
        remList_ ((struct hLinks *) hEntPtr);
        FREEUP (hEntPtr->keyname);
        free (hEntPtr);
    }
        }

    free (tabPtr->slotPtr);
    tabPtr->slotPtr = NULL;
    tabPtr->numEnts = 0;
}

/* getClosestPrime()
 * Get the nearest prime >= x.
 */
static size_t
getClosestPrime (unsigned long x)
{
    size_t n = sizeof (primes) / sizeof (primes[0]);

    for ( size_t cc = 0; cc < n; cc++) {

        if (x < primes[cc]) {
            return primes[cc];
        }
    }

    return primes[n - 1];
}
