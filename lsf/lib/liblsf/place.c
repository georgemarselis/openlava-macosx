/* $Id: lib.place.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/xdr.h"

char **
ls_placereq (char *resreq, size_t *numhosts, int options, char *fromhost)
{
    unsigned long i    = 0;
    unsigned long *num = 0;
    unsigned long *defaultHostNumberValue = 0;
    char **namelist;
    struct decisionReq placeReq;

    *defaultHostNumberValue = 1;
    
    if (!numhosts) {
        num = defaultHostNumberValue;
    }
    else {
        num = numhosts;
    }

    placeReq.ofWhat = OF_ANY;
    placeReq.options = options;
    strcpy (placeReq.hostType, " ");
    placeReq.numHosts = *num;

    if (!*num) {
        placeReq.numHosts = 9999;
        placeReq.options &= ~EXACT;
    }

    namelist = ls_findmyconnections ();
    for ( i = 0; namelist[i];) {    // SEEME SEEME SEEME eh?
        i++;                            // seriously? size of array? like this?
    }
    i++; // SEEME SEEME SEEME eh?
  
    // no assert, i >= 0
    placeReq.preferredHosts = (char **) calloc ( (unsigned long)i, sizeof (char *));
    
    if ( NULL == placeReq.preferredHosts && ENOMEM == errno  ) {
        lserrno = LSE_MALLOC;
        return NULL;
    }

    for ( i = 1; namelist[i - 1] != NULL; i++) {

        placeReq.preferredHosts[i] = putstr_ (namelist[i - 1]);
        
        if (placeReq.preferredHosts[i] == NULL) {
            
            for ( unsigned long j = 1; j < i; j++)  {
                free (placeReq.preferredHosts[j]);
            }
            
            free (placeReq.preferredHosts);
            lserrno = LSE_MALLOC;

                return NULL;
        }
    }

    placeReq.numPrefs = i;

    return placement_ (resreq, &placeReq, fromhost, num);
}

char **
ls_placeofhosts (char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize)
{
    unsigned long i    = 0;
    unsigned long *num = 0;
    unsigned long *defaultNumValue = 0;
    struct decisionReq placeReq;
    *defaultNumValue   = 1;

    if (!listsize || !hostlist) {
        lserrno = LSE_BAD_ARGS;
        return (NULL);
    }

    if (!numhosts) {
        num = defaultNumValue;
    }
    else {
        num = numhosts;
    }

    placeReq.ofWhat = OF_HOSTS;
    placeReq.options = options;
    strcpy (placeReq.hostType, " ");
    placeReq.numHosts = *num;

    if (!*num) {
        placeReq.numHosts = listsize;
        placeReq.options &= ~EXACT;
    }

    placeReq.numPrefs = listsize + 1;
    placeReq.preferredHosts = calloc ( placeReq.numPrefs, sizeof( placeReq.preferredHosts ) );
    if ( NULL == placeReq.preferredHosts && ENOMEM == errno ) {
        lserrno = LSE_MALLOC;
        return NULL;
    }

    for (i = 1; i < placeReq.numPrefs; i++)
    {

        if (ls_isclustername (hostlist[i - 1]) <= 0)
        {
            if (Gethostbyname_ (hostlist[i - 1]) == NULL)
            {
                lserrno = LSE_BAD_ARGS;
                break;
            }
        
            placeReq.preferredHosts[i] = putstr_ (hostlist[i - 1]);
        }
        else {
            placeReq.preferredHosts[i] = putstr_ (hostlist[i - 1]);
        }

        if (placeReq.preferredHosts[i] == NULL) {
            lserrno = LSE_MALLOC;
            break;
        }
    }

    if (i < placeReq.numPrefs) {
        for (unsigned long j = 1; j < i; j++) {
            free (placeReq.preferredHosts[j]);
        }
      
        free (placeReq.preferredHosts);
        
        return NULL;
    }

  return placement_(resreq, &placeReq, fromhost, num);

}

char **
placement_ (char *resReq, struct decisionReq *placeReqPtr, char *fromhost, size_t *numhosts)
{
    static struct placeReply placeReply;
    static char **hostnames;
    unsigned long numnames = 0;
/*    int i = 0;
    int j = 0;
    int k = 0;*/
    char *hname;
    int limReturnStatus = 0;

    if (initenv_ (NULL, NULL) < 0) {
        return NULL;
    }

    if (placeReqPtr->numHosts <= 0) {
        lserrno = LSE_BAD_ARGS;
        for (unsigned long i = 0; i < placeReqPtr->numPrefs; i++) {
            FREEUP (placeReqPtr->preferredHosts[i]);
        }
        FREEUP (placeReqPtr->preferredHosts);
        return NULL;
    }

    if (!fromhost)
    {
        hname = ls_getmyhostname ();
        if ( NULL == hname ) {
            for (unsigned long i = 0; i < placeReqPtr->numPrefs; i++) {
                FREEUP (placeReqPtr->preferredHosts[i]);
            }
            FREEUP (placeReqPtr->preferredHosts);
            return NULL;
        }
        
        placeReqPtr->preferredHosts[0] = putstr_ (hname);
    }
    else {
        placeReqPtr->preferredHosts[0] = fromhost;
    }

    if (placeReqPtr->preferredHosts[0] == NULL) {
        lserrno = LSE_MALLOC;
        for (unsigned long i = 0; i < placeReqPtr->numPrefs; i++) {
            FREEUP (placeReqPtr->preferredHosts[i]);
        }
        FREEUP (placeReqPtr->preferredHosts);
        return NULL;
    }

    if (resReq != NULL) {
        strcpy (placeReqPtr->resReq, resReq);
    }
    else {
        placeReqPtr->resReq[0] = '\0';
    }

    limReturnStatus = callLim_ (LIM_PLACEMENT, placeReqPtr,  xdr_decisionReq, &placeReply, xdr_placeReply, NULL, _USE_TCP_, NULL );
    
    if( limReturnStatus < 0) {
        for (unsigned long i = 0; i < placeReqPtr->numPrefs; i++) {
            FREEUP (placeReqPtr->preferredHosts[i]);
        }
        FREEUP (placeReqPtr->preferredHosts);
        return NULL;
    }

    for ( unsigned long i = 0; i < placeReqPtr->numPrefs; i++) {
        FREEUP (placeReqPtr->preferredHosts[i]);
    }

    FREEUP (placeReqPtr->preferredHosts);
    *numhosts = placeReply.numHosts;

    // numnames = 0;
    for (unsigned long i = 0; i < *numhosts; i++) {
        assert( placeReply.placeInfo[i].numtask >= 0);
        numnames += (unsigned long) placeReply.placeInfo[i].numtask;
    }

    if (hostnames) {
        free (hostnames);
    }

    hostnames = calloc ( numnames, sizeof( hostnames ) );
    if ( NULL == hostnames && ENOMEM == errno ) {
        lserrno = LSE_MALLOC;
        return NULL;
    }

    for (unsigned long i = 0, j = 0; i < *numhosts; i++) {
        for (int k = 0; k < placeReply.placeInfo[i].numtask; k++) {
            hostnames[j++] = placeReply.placeInfo[i].hostName;
        }
    }

    *numhosts = numnames;
    return hostnames;

//error: // FIXME FIXME FIXME the devil
}

/* ls_addhost()
 */
int
ls_addhost (struct hostEntry *hPtr)
{
  char *master;

  if (hPtr == NULL)
    {
      lserrno = LSE_BAD_ARGS;
      return -1;
    }

  /* Make sure the library calls the server
   * hosts as they have for sure the most
   * upto date master LIM information.
   */
  if ((master = ls_getmastername2 ()) == NULL)
    return -1;

  if (callLim_ (LIM_ADD_HOST,
    hPtr, xdr_hostEntry, NULL, NULL, NULL, _USE_TCP_, NULL) < 0)
    return -1;

  return 0;
}

/* ls_rmhost()
 */
int
ls_rmhost (const char *host)
{
  char *master;

  if (host == NULL)
    {
      lserrno = LSE_BAD_ARGS;
      return -1;
    }

  /* Dirty trick to force the library
   * to call all hosts in LSF_SERVER_HOSTS
   * and not the local LIM first...
   */
  if ((master = ls_getmastername2 ()) == NULL)
    return -1;

  if (callLim_ (LIM_RM_HOST,
    (char *) host,
    xdr_hostName, NULL, NULL, NULL, _USE_TCP_, NULL) < 0)
    return -1;

  return 0;
}
