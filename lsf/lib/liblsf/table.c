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
#include "lib/table.h"
#include "lib/misc.h"

/* insList_()
 * Add the elemPtr in the list at destPtr address.
 */
void
insList_ (struct hLinks *elemPtr, struct hLinks *destPtr)
{
    elemPtr->bwPtr        = destPtr->bwPtr;
    elemPtr->fwPtr        = destPtr;
    destPtr->bwPtr->fwPtr = elemPtr;
    destPtr->bwPtr        = elemPtr;
}

void
remList_ (struct hLinks *elemPtr)
{

    if (elemPtr == NULL || elemPtr == elemPtr->bwPtr || !elemPtr) {
        return;
    }

    elemPtr->fwPtr->bwPtr = elemPtr->bwPtr;
    elemPtr->bwPtr->fwPtr = elemPtr->fwPtr;

    return;
}

void
initList_ (struct hLinks *headPtr)
{
    headPtr->bwPtr = headPtr;
    headPtr->fwPtr = headPtr;

    return;
}

void
h_initTab_ (struct hTab *tabPtr, size_t numSlots)
{
    struct hLinks *slotPtr = tabPtr->slotPtr;
    unsigned long getClosestPrimeResult = 0;

    tabPtr->numEnts = 0;

    /* Our hash table works best if we have its size
     * as prime number.
     */
    // FIXME FIXME generate highest-needed prime number?
    assert( numSlots > 1 );
    getClosestPrimeResult = getClosestPrime (numSlots);
    tabPtr->size = getClosestPrimeResult;
    tabPtr->slotPtr = malloc (sizeof (struct hLinks) * tabPtr->size);
    for ( size_t i = 0; i < tabPtr->size; i++, slotPtr++) {
        initList_ (slotPtr);
    }

    return;
}

void
h_freeTab_ (struct hTab *tabPtr, void (*freeFunc) (void *))
{
    struct hLinks *hTabEnd = NULL;
    struct hLinks *slotPtr = NULL;
    struct hEnt *hEntPtr   = NULL;

    slotPtr = tabPtr->slotPtr;
    hTabEnd = &(slotPtr[tabPtr->size]);

    for (; slotPtr < hTabEnd; slotPtr++) {

        while (slotPtr != slotPtr->bwPtr) {

            hEntPtr = (struct hEnt *) slotPtr->bwPtr;
            remList_ ((struct hLinks *) hEntPtr);
            // FREEUP (hEntPtr->keyname);

            if (hEntPtr->hData != NULL) {
                if (freeFunc != NULL) {
                    (*freeFunc) ((void *) hEntPtr->hData);
                }
                else {
                    free (hEntPtr->hData);
                    hEntPtr->hData = NULL;
                }
            }

            free (hEntPtr);
        }
    }

    free ( tabPtr->slotPtr);
    tabPtr->slotPtr = NULL;
    tabPtr->numEnts = 0;

    return;
}

int
h_TabEmpty_ (struct hTab *tabPtr)
{
    return tabPtr->numEnts == 0;
}

void
h_delTab_ (struct hTab *tabPtr)
{
    h_freeTab_ (tabPtr, (HTAB_DATA_DESTROY_FUNC_T) NULL);
}

/* h_getEnt_()
 * Get an entry from the hash table based on
 * a given key.
 */
struct hEnt *
h_getEnt_ (struct hTab *tabPtr, const char *key)
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
h_addEnt_ (struct hTab *tabPtr, const char *key, int *newPtr)
{
    char *keyPtr         = strdup( key );
    struct hEnt *hEntPtr = NULL;
    struct hLinks *hList = NULL;

    hList = &(tabPtr->slotPtr[getAddr (tabPtr, keyPtr)]);
    hEntPtr = h_findEnt ((char *) keyPtr, hList);

    if (NULL != hEntPtr && NULL != newPtr) {
        *newPtr = FALSE;
        return hEntPtr;
    }

    if (tabPtr->numEnts >= (RESETLIMIT/2) * tabPtr->size) {
        resetTab (tabPtr);
        hList = &(tabPtr->slotPtr[getAddr (tabPtr, (char *) keyPtr)]);
    }

    /* Create a new entry and increase the counter
     * of entries.
     */
    hEntPtr = malloc (sizeof (struct hEnt));
    hEntPtr->keyname = putstr_ ((char *) keyPtr);
    hEntPtr->hData = NULL;
    insList_ ((struct hLinks *) hEntPtr, hList);
    if (newPtr != NULL) {
        *newPtr = TRUE;
    }
    tabPtr->numEnts++;

    return hEntPtr;

}

/* h_delEnt_()
 */
void
h_delEnt_ (struct hTab *tabPtr, struct hEnt *hEntPtr)
{

    if (hEntPtr != NULL) {
        remList_ ((struct hLinks *) hEntPtr);
        // free (hEntPtr->keyname);
        if (hEntPtr->hData != NULL) {
            free ( hEntPtr->hData);
        }
        free ( hEntPtr );
        tabPtr->numEnts--;
    }

    return;
}

/* h_rmEnt_()
 */
void
h_rmEnt_ (struct hTab *tabPtr, struct hEnt *hEntPtr)
{

    if (hEntPtr != NULL) {
        remList_ ((struct hLinks *) hEntPtr);
        // free (hEntPtr->keyname);
        free ( hEntPtr );
        tabPtr->numEnts--;
    }

    return;
}


/* h_firstEnt_()
 * Get the first element from the hash table.
 * Starting the iteration on the table itself.
 */
struct hEnt *
h_firstEnt_ (struct hTab *tabPtr, struct sTab *sPtr)
{

    sPtr->tabPtr  = tabPtr;
    sPtr->nIndex  = 0;
    sPtr->hEntPtr = NULL;

    if (tabPtr->slotPtr) {
        return h_nextEnt_ (sPtr);
    }
    else {
        return NULL;
    }

    return NULL;
}

/* h_nextEnt_()
 * Get the next entry from the hash table till it is
 * empty.
 */
struct hEnt *
h_nextEnt_ ( struct sTab *sPtr)
{
    struct hLinks *hList = NULL;
    struct hEnt *hEntPtr = NULL;

    hEntPtr = sPtr->hEntPtr;

    while (hEntPtr == NULL || (struct hLinks *) hEntPtr == sPtr->hList) {

        if (sPtr->nIndex >= sPtr->tabPtr->size) {
            return NULL;
        }

        hList = &(sPtr->tabPtr->slotPtr[sPtr->nIndex]);
        sPtr->nIndex++;
        if (hList != hList->bwPtr) {

            hEntPtr = (struct hEnt *) hList->bwPtr;
            sPtr->hList = hList;
            break;
        }

    }

    sPtr->hEntPtr = (struct hEnt *) ((struct hLinks *) hEntPtr)->bwPtr;

    return hEntPtr;

}

/* getAddr()
 * Compute a hash index, almost right from K&R
 */
unsigned long
getAddr (struct hTab *tabPtr, const char *key)
{
    unsigned long hashIndex = 0;
    unsigned long hashKey   = (unsigned long) atol( key );

    while (*key) {
        hashIndex = ( hashIndex * 128 +  hashKey ) % tabPtr->size; // NOFIX cast is probably fine
        key++;
        hashKey++;
    }

    return hashIndex;

}

struct hEnt *
h_findEnt (const char *key, struct hLinks *hList)
{
    struct hEnt *hEntPtr = NULL;

    for (hEntPtr = (struct hEnt *) hList->bwPtr; hEntPtr != (struct hEnt *) hList; hEntPtr = (struct hEnt *) ((struct hLinks *) hEntPtr)->bwPtr) {
        if (strcmp (hEntPtr->keyname, key) == 0) {
            return hEntPtr;
        }
    }

    return NULL;
}

/* resetTab()
 */
void
resetTab (struct hTab *tabPtr)
{
    size_t lastSize    = 0;
    unsigned long slot = 0;
    struct hLinks *lastSlotPtr = NULL;
    struct hLinks *lastList    = NULL;
    struct hEnt *hEntPtr       = NULL;

    lastSlotPtr = tabPtr->slotPtr;
    lastSize    = tabPtr->size;

    h_initTab_ (tabPtr, tabPtr->size * RESETFACTOR);

    for (lastList = lastSlotPtr; lastSize > 0; lastSize--, lastList++) {
        while (lastList != lastList->bwPtr) {
            hEntPtr = (struct hEnt *) lastList->bwPtr;
            remList_ ((struct hLinks *) hEntPtr);
            slot = getAddr (tabPtr, hEntPtr->keyname);
            insList_ ((struct hLinks *) hEntPtr, (struct hLinks *) (&(tabPtr->slotPtr[slot])));
            tabPtr->numEnts++;
        }
    }

    free (lastSlotPtr);

    return;
}

void
h_delRef_ (struct hTab *tabPtr, struct hEnt *hEntPtr)
{
    if (hEntPtr != NULL) {
        remList_ ((struct hLinks *) hEntPtr);
        // free (hEntPtr->keyname);
        free (hEntPtr);
        tabPtr->numEnts--;
    }

    return;
}

void
h_freeRefTab_ (struct hTab *tabPtr)
{
    struct hLinks *hTabEnd = NULL; 
    struct hLinks *slotPtr = NULL;
    struct hEnt *hEntPtr   = NULL;

    slotPtr = tabPtr->slotPtr;
    hTabEnd = &(slotPtr[tabPtr->size]);

    for (; slotPtr < hTabEnd; slotPtr++) {

        while (slotPtr != slotPtr->bwPtr) {

            hEntPtr = ( struct hEnt *) slotPtr->bwPtr;
            remList_ ((struct hLinks *) hEntPtr);
            // FREEUP (hEntPtr->keyname);
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
size_t
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
