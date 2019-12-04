/* $Id: lib.load.c 397 2007-11-26 19:04:00Z mblack $
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
// #include "lib/lproto.h"
#include "lib/xdr.h"
#include "lib/load.h"
#include "lib/syslog.h"
#include "lib/info.h"
#include "lib/host.h"
#include "lib/misc.h"
#include "daemons/liblimd/lim.h"
// #define LOAD_INFO_THRESHOLD 75

char *
namestofilter_ (char **indxs )
{
        size_t i = 0;
        size_t len = 0;
         static char *filter;

        if (filter) {
                free (filter);
        }

        for(i = 0, len = 0; indxs[i]; i++) {    // FIXME FIXME FIXME FIXME SHITTY POINTER ARITHMETIC
                len += strlen (indxs[i]);
        }

        if (!len || len > MAX_LINE_LEN) {
                lserrno = LSE_BAD_ARGS;
                return NULL;
        }

        len += i ;
        filter = malloc( len + 1 );
        if( NULL == filter && ENOMEM == errno ) {
                lserrno = LSE_MALLOC;
                return NULL;
        }

        filter[0] = '\0';
        for (i = 0; indxs[i]; i++) {
                strcat (filter, indxs[i]);
                strcat (filter, ":");
        }
        
        return filter;
}

struct hostLoad *
ls_loadinfo (char *resreq, size_t *numhosts, int options, const char *fromhost, char **hostlist, size_t listsize, char ***indxnamelist)
{
    int isClus          = 0;
    char *indexfilter   = NULL;
    unsigned long *num  = 0;
    unsigned long *initialValueofNum    = 0;
    char tempresreq[MAX_LINE_LEN]         = { '\0' };
    struct decisionReq loadReq;
    
    *initialValueofNum = 1;

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
    }

    if (!indxnamelist) {
        lserrno = LSE_BAD_ARGS;
        return NULL;
    }

    if (hostlist && listsize) {
        loadReq.ofWhat = OF_HOSTS;
    }
    else {
        loadReq.ofWhat = OF_ANY;
    }

    loadReq.options = options;
    strcpy (loadReq.hostType, " ");

    if (numhosts == 0) {
        num = initialValueofNum;
    }
    else {
        num = numhosts;
    }

    loadReq.numHosts = *num;

    if (!*num) {
        if (loadReq.ofWhat == OF_HOSTS) {
            loadReq.numHosts = listsize;
        }
        else {
            loadReq.numHosts = 9999;
        }
        loadReq.options &= ~EXACT;
    }

    if (loadReq.ofWhat == OF_HOSTS)
    {

        unsigned long i    = 0;            
        unsigned long j    = 0;
        char clusterinlist = '\0';

        loadReq.numPrefs = listsize + 1;
        loadReq.preferredHosts = (char **) calloc ( loadReq.numPrefs, sizeof (char *));
        if (loadReq.preferredHosts == NULL)
        {
            lserrno = LSE_MALLOC;
            return NULL;
        }

        for (i = 1; i < loadReq.numPrefs; i++)
        {
            if ((isClus = ls_isclustername (hostlist[i - 1])) < 0)
            {
                break;
            }
            else if (isClus == 0)
            {
                if ((Gethostbyname_ (hostlist[i - 1])) == NULL)
                {
                    lserrno = LSE_BAD_HOST;
                    break;
                }
                loadReq.preferredHosts[i] = putstr_ (hostlist[i - 1]);
            }
            else
            {
                loadReq.preferredHosts[i] = putstr_ (hostlist[i - 1]);
                clusterinlist = 1;
            }

            if (loadReq.preferredHosts[i] == NULL)
            {
                for (j = 1; j < i; j++) {
                    free (loadReq.preferredHosts[j]);
                }
                lserrno = LSE_MALLOC;
                break;
            }
        }

        if (i < loadReq.numPrefs)
        {
            for (j = 1; j < i; j++) {
                free (loadReq.preferredHosts[j]);
            }
            free (loadReq.preferredHosts);
            return NULL;
        }

        if (*num > listsize && (loadReq.options & EXACT) && !clusterinlist)
        {
            lserrno = LSE_NO_HOST;
            for (j = 1; j < i; j++) {
                free (loadReq.preferredHosts[j]);
            }
            free (loadReq.preferredHosts);
            return NULL;
        }


        if (clusterinlist && !*num)
        {
            loadReq.numHosts = ULONG_MAX;
            loadReq.options &= ~EXACT;
        }

        }
    else
    {
        loadReq.numPrefs = *initialValueofNum; 
        loadReq.preferredHosts = (char **) calloc( loadReq.numPrefs, sizeof (char *));
        if (loadReq.preferredHosts == NULL)
        {
            lserrno = LSE_MALLOC;
            return NULL;
            }
    }

        if (*indxnamelist)
        {
            indexfilter = namestofilter_ (*indxnamelist);
            if (!indexfilter)
                return NULL;
        }
        else
            indexfilter = NULL;

        if (resreq && indexfilter)
        {
            if ((strlen (resreq) + strlen (indexfilter)) < MAX_LINE_LEN - 8)
            {
                char tmp[MAX_LINE_LEN / 2];
                sprintf (tmp, "filter[%s]", indexfilter);
                strcpy (tempresreq, resreq);
                strcat (tempresreq, tmp);
                resreq = tempresreq;
            }
            else
            {
                lserrno = LSE_BAD_ARGS;
                return NULL;
            }
        }
        else if (indexfilter)
        {
            sprintf (tempresreq, "filter[%s]", indexfilter);
            resreq = tempresreq;
        }

        if (logclass & (LC_TRACE)) {
            ls_syslog (LOG_DEBUG, "%s: loadReq.ofWhat=%d loadReq.numHosts=%1.0ld loadReq..numPrefs=%1.0ld", __func__, loadReq.ofWhat, loadReq.numHosts, loadReq.numPrefs);
        }

    return loadinfo_ (resreq, &loadReq, fromhost, num, indxnamelist);

}

struct hostLoad *
ls_load (char *resreq, size_t *numhosts, int options, const char *fromhost)
{
    char **dummynl = NULL;

    return ls_loadinfo (resreq, numhosts, options, fromhost, NULL, 0, &dummynl);

}

struct hostLoad *
ls_loadofhosts (char *resreq, size_t *numhosts, int options, const char *fromhost, char **hostlist, size_t listsize)
{
    char **dummynl = NULL;

    return ls_loadinfo(resreq, numhosts, options, fromhost, hostlist, listsize, &dummynl);

}

struct hostLoad *
loadinfo_ (char *resReq, struct decisionReq *loadReqPtr, const char *fromhost, unsigned long *numHosts, char ***outnlist)
{
    static struct loadReply loadReply;
    char *hname;
    int options = 0;

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
    }

    if (loadReqPtr->numHosts <= 0) {
        lserrno = LSE_BAD_ARGS;
        for (unsigned long i = 0; i < loadReqPtr->numPrefs; i++) {
            FREEUP (loadReqPtr->preferredHosts[i]);
        }

        FREEUP (loadReqPtr->preferredHosts);
        return NULL;
    }

    if (!fromhost)  {
        if ((hname = ls_getmyhostname ()) == NULL) {
            for (unsigned long i = 0; i < loadReqPtr->numPrefs; i++) {
                FREEUP (loadReqPtr->preferredHosts[i]);
            }
            FREEUP (loadReqPtr->preferredHosts);
            return NULL;
        }
        loadReqPtr->preferredHosts[0] = putstr_ (hname);
    }
    else {
        loadReqPtr->preferredHosts[0] = putstr_ (fromhost);
    }

    if (loadReqPtr->preferredHosts[0] == NULL) {
        lserrno = LSE_MALLOC;
        for (unsigned long i = 0; i < loadReqPtr->numPrefs; i++) {
            FREEUP (loadReqPtr->preferredHosts[i]);
        }
        FREEUP (loadReqPtr->preferredHosts);
        return NULL;
    }

    if (resReq) {
        strncpy (loadReqPtr->resReq, resReq, MAX_LINE_LEN);
    }
    else {
        strcpy (loadReqPtr->resReq, " ");
    }

    loadReqPtr->resReq[MAX_LINE_LEN - 1] = '\0';
    if (loadReqPtr->ofWhat == OF_HOSTS && loadReqPtr->numPrefs == 2 && loadReqPtr->numHosts == 1 && equalHost_ (loadReqPtr->preferredHosts[1], loadReqPtr->preferredHosts[0])) {
        options |= _LOCAL_;
    }
    else{
        options |= _USE_TCP_;
    }

    if (callLim_ (LIM_LOAD_REQ, loadReqPtr, xdr_decisionReq, &loadReply, xdr_loadReply, NULL, options, NULL) < 0) {
        for (unsigned long i = 0; i < loadReqPtr->numPrefs; i++) {
            FREEUP (loadReqPtr->preferredHosts[i]);
        }
        FREEUP (loadReqPtr->preferredHosts);
        return NULL;
    }
    if (loadReply.flags & LOAD_REPLY_SHARED_RESOURCE)
        {
            sharedResConfigured_ = TRUE;
        }
    for (unsigned long i = 0; i < loadReqPtr->numPrefs; i++) {
        FREEUP (loadReqPtr->preferredHosts[i]);
    }
    FREEUP (loadReqPtr->preferredHosts);
    *numHosts = loadReply.nEntry;
    *outnlist = loadReply.indicies;
    return loadReply.loadMatrix;

// error: was here
/*    for (unsigned long i = 0; i < loadReqPtr->numPrefs; i++) {
        FREEUP (loadReqPtr->preferredHosts[i]);
    }
    FREEUP (loadReqPtr->preferredHosts);
    return NULL;
*/
}

int
ls_loadadj (char *resreq, struct placeInfo *placeinfo, size_t listsize)
{
    struct jobXfer loadadjReq;
    int callLimResult = 0;
    
    if (listsize <= 0 || placeinfo == NULL) {
        lserrno = LSE_BAD_ARGS;
        return -1;
    }

    loadadjReq.numHosts = listsize;
    loadadjReq.placeInfo = placeinfo;
    for( size_t i = 0; i < listsize; i++) {
        
        if (Gethostbyname_ (placeinfo[i].hostName) == NULL) {
            return -1;
        }
        
        // strcpy (placeinfo[i].hostName, placeinfo[i].hostName); // f

        // if (placeinfo[i].numtask < 0) {
        //     placeinfo[i].numtask = 0;
        // }
    }

    if (resreq != NULL) {
        strcpy (loadadjReq.resReq, resreq);
    }
    else {
        loadadjReq.resReq[0] = '\0';
    }

    callLimResult = callLim_ (LIM_LOAD_ADJ, &loadadjReq, xdr_jobXfer, NULL, NULL, NULL, _USE_TCP_, NULL);
    if ( callLimResult < 0 ) {
        return -1;
    }

    return 0;
}
