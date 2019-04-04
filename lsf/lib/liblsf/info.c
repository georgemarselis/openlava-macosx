/* $Id: lib.info.c 397 2007-11-26 19:04:00Z mblack $
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

#include <unistd.h>

#include "lib/info.h"
#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/xdr.h"
#include "lib/xdrlim.h"
#include "lib/syslog.h"
#include "lib/misc.h"
#include "lib/initenv.h"
#include "lib/channel.h"
#include "lib/host.h"
#include "daemons/liblimd/lim.h"



char *
ls_getclustername (void)
{

    char *clName = malloc( sizeof( char ) * MAX_LSF_NAME_LEN + 1 ); // FIXME FIXME FIXME FIXME dynamic memory allocation and management

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
    }

    if( NULL == clName ) {
        if ( getname_ (LIM_GET_CLUSNAME, clName, MAX_LSF_NAME_LEN) < 0) {
            return NULL;
        }
    }

    return clName;
}

unsigned int
expandList1_ (char ***tolist, int num, int *bitmMaps, char **keys) // NOTE the ints are correct, we are talking bitmaps
{

    int ii = 0;
    unsigned int jj = 0;
    int isSet = 0;
    char **temp = NULL;
    size_t omgbecky = 0;

    if (num <= 0) {
        return 0;
    }

    omgbecky = (size_t) num; // FIXME num is checked above. cast ok
    temp = calloc ( omgbecky, sizeof (char *) + 1);
    if (  NULL == temp && ENOMEM == errno ) {
        lserrno = LSE_MALLOC;
        return 0;
    }

    for (ii = 0, jj = 0; ii < num; ii++) {
        TEST_BIT (ii, bitmMaps, isSet);
        if (isSet == 1) {
            temp[jj++] = keys[ii];
        }
    }
    
    if (jj > 0) {
      *tolist = temp;
    }
    else
    {
      FREEUP (temp);
      *tolist = NULL;
    }

    return jj;
}

unsigned int
expandList_ (char ***tolist, int mask, char **keys) // NOTE yeah we mean the int
{
    unsigned int lastElementCounter = 0;
    char *temp[32];                         // FIXME FIXME FIXME 32 element array looks awfuly particular. why?

    for ( unsigned int i = 0, j = 0; i < 32; i++) {
        if (mask & (1 << i)) {
            temp[j++] = keys[i];
            lastElementCounter = j;
        }
    }
    
    if (lastElementCounter > 0) {
        *tolist = calloc ( lastElementCounter, sizeof (char *));
        
        if (!*tolist) {

            lserrno = LSE_MALLOC;
            return 0;
        }
        
        for ( unsigned int i = 0; i < lastElementCounter; i++) {
            (*tolist)[i] = temp[i];
        }
    }
    else {
        *tolist = NULL;
    }
    
    return lastElementCounter;
}

int
copyAdmins_ (struct clusterInfo *clusPtr, struct shortCInfo *clusShort)
{

    if (clusShort->nAdmins <= 0) {
        return 0;
    }

    // check already done above
    clusPtr->adminIds = calloc( clusShort->nAdmins, sizeof (int));
    clusPtr->admins   = calloc(  clusShort->nAdmins, sizeof (char *));

    if (!clusPtr->admins || !clusPtr->adminIds) {
        // goto errReturn;
        FREEUP (clusPtr->admins);
        FREEUP (clusPtr->adminIds);
        lserrno = LSE_MALLOC;
        return -1;
    }

    for( unsigned int i = 0; i < clusShort->nAdmins; i++) {

        clusPtr->admins[i] = NULL;
        clusPtr->adminIds[i] = clusShort->adminIds[i];
        clusPtr->admins[i] = putstr_ (clusShort->admins[i]);
        
        if (clusPtr->admins[i] == NULL) {
        
            for ( unsigned int j = 0; j < i; j++) {
                FREEUP (clusPtr->admins[j]);
            }

            // goto errReturn;
            FREEUP (clusPtr->admins);
            FREEUP (clusPtr->adminIds);
            lserrno = LSE_MALLOC;
            return -1;
        }
    }
    
    return 0;

// errReturn:  // FIXME FIXME FIXME 
//     FREEUP (clusPtr->admins);
//     FREEUP (clusPtr->adminIds);
//     lserrno = LSE_MALLOC;
//     return -1;
}

struct clusterInfo *
expandSCinfo (struct clusterInfoReply *clusterInfoReply)
{
    static unsigned int nClusters = 0;
    static struct clusterInfo *clusterInfoPtr = NULL;
    struct shortLsInfo *lsInfoPtr;
    // int i, j, k;

    if (clusterInfoPtr) {
      
        for ( unsigned int i = 0; i < nClusters; i++) {
            free (clusterInfoPtr[i].resources);
            free (clusterInfoPtr[i].hostModels);
            free (clusterInfoPtr[i].hostTypes);
          
            if (clusterInfoPtr[i].nAdmins > 0) {
                for ( unsigned int j = 0; j < clusterInfoPtr[i].nAdmins; j++) {
                    FREEUP (clusterInfoPtr[i].admins[j]);
                }
                FREEUP (clusterInfoPtr[i].admins);
                FREEUP (clusterInfoPtr[i].adminIds);
            }
        }
      
        FREEUP (clusterInfoPtr);
    }

    clusterInfoPtr = calloc ( clusterInfoReply->nClusters, sizeof (struct clusterInfo));
 
    if (!clusterInfoPtr) {
        nClusters = 0;
        lserrno = LSE_MALLOC;
        return NULL;
    }

        nClusters = clusterInfoReply->nClusters;
        lsInfoPtr = clusterInfoReply->shortLsInfo;

    for ( unsigned int i = 0; i < clusterInfoReply->nClusters; i++) {

        strcpy (clusterInfoPtr[i].clusterName, clusterInfoReply->clusterMatrix[i].clName);
        strcpy (clusterInfoPtr[i].masterName,  clusterInfoReply->clusterMatrix[i].masterName);
        strcpy (clusterInfoPtr[i].managerName, clusterInfoReply->clusterMatrix[i].managerName);

        clusterInfoPtr[i].managerId  = clusterInfoReply->clusterMatrix[i].managerId;
        clusterInfoPtr[i].status     = clusterInfoReply->clusterMatrix[i].status;
        clusterInfoPtr[i].numServers = clusterInfoReply->clusterMatrix[i].numServers;
        clusterInfoPtr[i].numClients = clusterInfoReply->clusterMatrix[i].numClients;
        clusterInfoPtr[i].nAdmins    = clusterInfoReply->clusterMatrix[i].nAdmins;
      
        if (copyAdmins_ (&clusterInfoPtr[i], &clusterInfoReply->clusterMatrix[i]) < 0) {
            break;
        }

        clusterInfoPtr[i].resources = NULL;
        clusterInfoPtr[i].hostTypes = NULL;
        clusterInfoPtr[i].hostModels = NULL;

        if (clusterInfoReply->clusterMatrix[i].nRes == 0) {
            unsigned int resClass = clusterInfoReply->clusterMatrix[i].resClass;
            assert( resClass <= INT_MAX );
            clusterInfoPtr[i].nRes = expandList_ (&clusterInfoPtr[i].resources, (int) resClass, lsInfoPtr->resName);
        }
        else {
            unsigned int *resBitMaps = clusterInfoReply->clusterMatrix[i].resBitMaps;
            unsigned int nRes = clusterInfoReply->clusterMatrix[i].nRes;
            assert( *resBitMaps <= INT_MAX );
            assert( nRes <= INT_MAX );
            clusterInfoPtr[i].nRes = expandList1_ (&clusterInfoPtr[i].resources, (int) nRes, (int *) resBitMaps, lsInfoPtr->resName);
        }

        if (clusterInfoReply->clusterMatrix[i].nTypes == 0) {
            clusterInfoPtr[i].nTypes = expandList_ (&clusterInfoPtr[i].hostTypes, (int) clusterInfoReply->clusterMatrix[i].typeClass, lsInfoPtr->hostTypes);
        }
        else {
            unsigned int *hostTypeBitMaps = clusterInfoReply->clusterMatrix[i].hostTypeBitMaps;
            clusterInfoPtr[i].nTypes = expandList1_ (&clusterInfoPtr[i].hostTypes, (int) clusterInfoReply->clusterMatrix[i].nTypes, (int *)hostTypeBitMaps, lsInfoPtr->hostTypes);
        }

        if (clusterInfoReply->clusterMatrix[i].nModels == 0) {
            clusterInfoPtr[i].nModels = expandList_ (&clusterInfoPtr[i].hostModels, (int) clusterInfoReply->clusterMatrix[i].modelClass, lsInfoPtr->hostModels);
        }
        else {
            unsigned int *hostModelBitMaps = clusterInfoReply->clusterMatrix[i].hostModelBitMaps;
            clusterInfoPtr[i].nModels = expandList1_ (&clusterInfoPtr[i].hostModels, (int) clusterInfoReply->clusterMatrix[i].nModels, (int *) hostModelBitMaps, lsInfoPtr->hostModels);
        }
  
        if (i != clusterInfoReply->nClusters) {
            for ( unsigned int j = 0; j < i; j++) {
                FREEUP (clusterInfoPtr[j].resources);
                FREEUP (clusterInfoPtr[j].hostTypes);
                FREEUP (clusterInfoPtr[j].hostModels);
                if (clusterInfoPtr[j].nAdmins > 0) {

                    for ( unsigned int k = 0; k < clusterInfoPtr[j].nAdmins; k++) {
                        FREEUP (clusterInfoPtr[j].admins[k]);
                    }

                    FREEUP (clusterInfoPtr[j].admins);
                    FREEUP (clusterInfoPtr[j].adminIds);
                }

            }

            FREEUP (clusterInfoPtr);
            return (struct clusterInfo *) NULL;
        }
    }
    
    return clusterInfoPtr;

}

struct clusterInfo *
ls_clusterinfo (char *resReq, unsigned int *numclusters, char **clusterList, int listsize, int options)
{
    struct clusterInfoReq clusterInfoReq;
    static struct clusterInfoReply clusterInfoReply;
    struct shortLsInfo shortlsInfo;
    int count = 0;
    int ret_val = 0;

    if (listsize != 0 && clusterList == NULL)
    {
        lserrno = LSE_BAD_ARGS;
        return NULL;
    }

    if (initenv_ (NULL, NULL) < 0) {
        return NULL;
    }

    for (count = 0; count < listsize; count++) {
        ret_val = ls_isclustername (clusterList[count]);
        
        if (ret_val <= 0) {
            if (ret_val < 0) {
                return NULL;
            }

            lserrno = LSE_BAD_CLUSTER;
            return NULL;
        }
    }

    if (resReq) {
        clusterInfoReq.resReq = resReq;
    }
    else {
        strcpy( clusterInfoReq.resReq, "");
    }

    clusterInfoReq.clusters = clusterList;
    clusterInfoReq.listsize = listsize;
    clusterInfoReq.options = options;

    clusterInfoReply.shortLsInfo = &shortlsInfo;
    if (callLim_ (LIM_GET_CLUSINFO, &clusterInfoReq, xdr_clusterInfoReq, &clusterInfoReply, xdr_clusterInfoReply, NULL, 0, NULL) < 0) { 
        return NULL;
    }

    if (numclusters != NULL) {
        *numclusters = clusterInfoReply.nClusters;
    }
    
    return expandSCinfo (&clusterInfoReply);

}

char *
ls_getmastername (void)
{
    static char master[MAXHOSTNAMELEN];

    if (getname_ (LIM_GET_MASTINFO, master, MAXHOSTNAMELEN) < 0) {
        return NULL;
    }

    return master;
}

/* ls_getmastername2()
 * Get the master name by calling the LSF_SERVER_HOSTS
 * only.
 */
char *
ls_getmastername2( void )
{
    static char master[MAXHOSTNAMELEN];

    if (getname_ (LIM_GET_MASTINFO2, master, MAXHOSTNAMELEN) < 0) {
        return NULL;
    }

    return master;
}

int
getname_ (enum limReqCode limReqCode, char *name, size_t namesize)
{
    int options = 0;

    if (initenv_ (NULL, NULL) < 0) {
        return -1;
    }

    if (limReqCode == LIM_GET_CLUSNAME) 
    {
        struct stringLen str;
        str.name = name;
        str.len = namesize;
        if (callLim_ (limReqCode, NULL, NULL, &str, xdr_stringLen, NULL, _LOCAL_, NULL) < 0) {
            return -1;
        }
        
        return 0;
    }

  /* Force the library not to call _LOCAL_ LIM since
   * it may not know the master yet, this is the
   * case of LIM_ADD_HOST.
   */
    if (limReqCode == LIM_GET_MASTINFO2){
        options = _SERVER_HOSTS_ONLY_;
    }
    else {
        options = _LOCAL_;
    }

     /* no data to send */
     /* host LSF_SERVER_HOSTS */
    if (callLim_ (limReqCode, NULL, NULL, &masterInfo_, xdr_masterInfo, NULL, options, NULL) < 0) {
        return -1;
    }

    if (memcmp (&sockIds_[MASTER].sin_addr, &masterInfo_.addr, sizeof (in_addr_t))) {
        chanClose_(limchans_[MASTER]);
        limchans_[MASTER] = -1;
        chanClose_(limchans_[TCP]);
        limchans_[TCP] = -1;
    }

    memcpy (&sockIds_[MASTER].sin_addr, &masterInfo_.addr, sizeof (in_addr_t));
    memcpy (&sockIds_[TCP].sin_addr, &masterInfo_.addr, sizeof (in_addr_t));
    sockIds_[TCP].sin_port = masterInfo_.portno;
    masterknown_ = TRUE;
    strncpy (name, masterInfo_.hostName, namesize);
    name[namesize - 1] = '\0';

    return 0;
}

char *
ls_gethosttype ( const char *hostname )
{

    struct hostInfo *hostinfo = NULL;
    static char hostType[MAX_LSF_NAME_LEN];
    char default_param[ ] = "-";

    // memset( hostType , 0, strlen( hostType) );

    if (hostname == NULL) {
        if ((hostname = ls_getmyhostname ()) == NULL) {
            return NULL;
        }
    }

    hostinfo = ls_gethostinfo ( default_param, NULL, &hostname, 1, 0);
    if (hostinfo == NULL) {
        return NULL;
    }

    strcpy (hostType, hostinfo[0].hostType);
    return hostType;
}

char *
ls_gethostmodel( const char *hostname )
{
    struct hostInfo *hostinfo = NULL;
    static char hostModel[MAX_LSF_NAME_LEN];
    char default_param[ ] = "-";

    if (hostname == NULL) {
        if ((hostname = ls_getmyhostname ()) == NULL) {
            return NULL;
        }
    }

    hostinfo = ls_gethostinfo( default_param, NULL, &hostname, 1, 0);
    if (hostinfo == NULL) {
        return NULL;
    }

    strcpy (hostModel, hostinfo[0].hostModel);
    return hostModel;
}


// ls_gethostfactor:
//      return the host factor for the specified host
//
//  in:  hostname
//  out: NULL if
//          hostname is not found
//          hostinfo for the specified host is not found
//       double host factor if
//          hostname and hostinfo are found
double *
ls_gethostfactor( const char *hostname )
{
    struct hostInfo *hostinfo = NULL;
    double *cpufactor         = 0;
    char default_param[ ]     = "-";


    if (hostname == NULL) {
        if ((hostname = ls_getmyhostname ()) == NULL) {
            return NULL;
        }
    }

    hostinfo = ls_gethostinfo( default_param, NULL, &hostname, 1, 0);
    if (hostinfo == NULL) {
        return NULL;
    }

    *cpufactor = hostinfo->cpuFactor;
    return cpufactor;
}

double *
ls_getmodelfactor( const char *modelname )
{
    double *cpuf = 0;
    struct stringLen str;

    if (!modelname) {
        return ls_gethostfactor (NULL);
    }

    if (initenv_ (NULL, NULL) < 0) {
        return NULL;
    }

    str.name = strdup( modelname );
    str.len = MAX_LSF_NAME_LEN;
    if (callLim_ (LIM_GET_CPUF, &str, xdr_stringLen, cpuf, xdr_float, NULL, 0, NULL) < 0) {
        return NULL;
    }
    
    return cpuf;
}

struct hostInfo *
expandSHinfo (struct hostInfoReply *hostInfoReply)
{
    unsigned long nHost  = 0;
    struct hostInfo *hostInfoPtr = NULL;
    unsigned long final_counter = 0;
    struct shortLsInfo *lsInfoPtr = NULL;

    if (hostInfoPtr)
    {
        for (unsigned long i = 0; i < nHost; i++) {
            FREEUP (hostInfoPtr[i].resources);
        }

        FREEUP (hostInfoPtr);
    }

    hostInfoPtr = malloc ( hostInfoReply->nHost * sizeof (struct hostInfo));
    if (!hostInfoPtr) {
        nHost = 0;
        lserrno = LSE_MALLOC;
        return NULL;
    }

    nHost = hostInfoReply->nHost;
    lsInfoPtr = hostInfoReply->shortLsInfo;

    for (unsigned long i = 0; i < hostInfoReply->nHost; i++)
    {
        unsigned int indx = 0;
        const char unknown[ ] = "unknown";
        strcpy (hostInfoPtr[i].hostName, hostInfoReply->hostMatrix[i].hostName);
        hostInfoPtr[i].windows     = hostInfoReply->hostMatrix[i].windows;
        hostInfoPtr[i].maxCpus     = hostInfoReply->hostMatrix[i].maxCpus;
        hostInfoPtr[i].maxMem      = hostInfoReply->hostMatrix[i].maxMem;
        hostInfoPtr[i].maxSwap     = hostInfoReply->hostMatrix[i].maxSwap;
        hostInfoPtr[i].maxTmp      = hostInfoReply->hostMatrix[i].maxTmp;
        hostInfoPtr[i].nDisks      = hostInfoReply->hostMatrix[i].nDisks;
        hostInfoPtr[i].isServer    = (hostInfoReply->hostMatrix[i].flags & HINFO_SERVER) != 0;
        hostInfoPtr[i].rexPriority = hostInfoReply->hostMatrix[i].rexPriority;

        hostInfoPtr[i].numIndx = hostInfoReply->nIndex;
        hostInfoPtr[i].busyThreshold = hostInfoReply->hostMatrix[i].busyThreshold;

        indx = hostInfoReply->hostMatrix[i].hTypeIndx;
        if( indx == MAX_TYPES ) {
        	strcpy( hostInfoPtr[i].hostType,  unknown );
        	strcpy( hostInfoPtr[i].hostModel, unknown );
        	hostInfoPtr[i].cpuFactor = 1.0F;
        }
        else {
        	strcpy( hostInfoPtr[i].hostType, lsInfoPtr->hostTypes[indx]);
        	strcpy( hostInfoPtr[i].hostModel, lsInfoPtr->hostModels[indx] );
        	hostInfoPtr[i].cpuFactor = lsInfoPtr->cpuFactors[indx];
        }
        // hostInfoPtr[i].hostType  = (indx == MAX_TYPES) ? "unknown" : lsInfoPtr->hostTypes[indx];
        // hostInfoPtr[i].hostModel = (indx == MAX_MODELS) ? "unknown" : lsInfoPtr->hostModels[indx];
        // hostInfoPtr[i].cpuFactor = (indx == MAX_MODELS) ? 1.0 : lsInfoPtr->cpuFactors[indx];

        if (hostInfoReply->hostMatrix[i].nRInt == 0) {
            unsigned int resClass = hostInfoReply->hostMatrix[i].resClass;
            assert( resClass < INT_MAX );
            hostInfoPtr[i].nRes = expandList_ (&hostInfoPtr[i].resources, (int) resClass, lsInfoPtr->resName);
        }
        else {
            unsigned int nRes = lsInfoPtr->nRes;
            unsigned int *resBitMaps = hostInfoReply->hostMatrix[i].resBitMaps;
            assert( nRes <= INT_MAX);
            assert( *resBitMaps <= INT_MAX );
            hostInfoPtr[i].nRes = expandList1_ (&hostInfoPtr[i].resources, (int) nRes, (int *) resBitMaps, lsInfoPtr->resName);
        }

        final_counter = i;
    }

    if (final_counter != hostInfoReply->nHost) {

        for (unsigned long j = 0; j < final_counter; j++) {
            free (hostInfoPtr[j].resources);
        }
        
        FREEUP (hostInfoPtr);
        lserrno = LSE_MALLOC;
        
        return (struct hostInfo *) NULL;
    }

    return hostInfoPtr;
}

struct hostInfo *
ls_gethostinfo (char *resReq, size_t *numhosts, const char **hostlist, size_t listsize, int options)
{
    char *hname = NULL;
    int cc = 0;
    struct shortLsInfo lsInfo;
    struct decisionReq hostInfoReq;
    static struct hostInfoReply hostInfoReply;
    
    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);
    }

    if ((hname = ls_getmyhostname ()) == NULL) {
        return NULL;
    }

    if (listsize) {
        unsigned long i = 0;

        if (hostlist == NULL) {
            lserrno = LSE_BAD_ARGS;
            return NULL;
        }

        hostInfoReq.preferredHosts = calloc ( listsize + 1, sizeof( hostInfoReq.preferredHosts ) ) ;
        if (hostInfoReq.preferredHosts == NULL) {
            lserrno = LSE_MALLOC;
            return NULL;
        }

        for (i = 0; i < listsize; i++) {
            if (hostlist[i] == NULL) {
                lserrno = LSE_BAD_ARGS;
                break;
            }

            cc = ls_isclustername (hostlist[i]);
            if (cc <= 0) {

                if (cc < 0) {
                    break;
                }
                if (Gethostbyname_ (hostlist[i]) == NULL) {
                    lserrno = LSE_BAD_HOST;
                    break;
                }
                hostInfoReq.preferredHosts[i + 1] = putstr_ (hostlist[i]);
            }
            else {
                hostInfoReq.preferredHosts[i + 1] = putstr_ (hostlist[i]);
            }

            if (!hostInfoReq.preferredHosts[i + 1])  {
                lserrno = LSE_MALLOC;
                break;
            }
        }
        if (i < listsize) {
            for (unsigned long j = 1; j < i + 1; j++)  {
                free (hostInfoReq.preferredHosts[j]);
            }
            free (hostInfoReq.preferredHosts);
            return NULL;
        }
        
        hostInfoReq.ofWhat = OF_HOSTS;
    }
    else {
        hostInfoReq.preferredHosts = (char **) calloc (1, sizeof (char *));
        if (hostInfoReq.preferredHosts == NULL) {
            lserrno = LSE_MALLOC;
            return NULL;
        }
        
        hostInfoReq.ofWhat = OF_ANY;
    }
    
    hostInfoReq.options = options;
    strcpy (hostInfoReq.hostType, " ");
    hostInfoReq.preferredHosts[0] = putstr_ (hname);
    hostInfoReq.numPrefs = listsize + 1;

    if (resReq != NULL) {
        strcpy (hostInfoReq.resReq, resReq);
    }
    else {
        strcpy (hostInfoReq.resReq, " ");
    }

    hostInfoReply.shortLsInfo = &lsInfo;
    hostInfoReq.numHosts = 0;
    cc = callLim_ (LIM_GET_HOSTINFO, &hostInfoReq, xdr_decisionReq, &hostInfoReply, xdr_hostInfoReply, NULL, _USE_TCP_, NULL);

    for ( unsigned long i = 0; i < hostInfoReq.numPrefs; i++) {
        free (hostInfoReq.preferredHosts[i]);
    }
    free (hostInfoReq.preferredHosts);
    if (cc < 0) {
        return NULL;
    }

    if (numhosts != NULL) {
        *numhosts = hostInfoReply.nHost;
    }
  
    return expandSHinfo (&hostInfoReply);

}

struct lsInfo *
ls_info (void)
{
    static struct lsInfo lsInfo;

    if (initenv_ (NULL, NULL) < 0) {
        return NULL;
    }

    if (callLim_ (LIM_GET_INFO, NULL, NULL, &lsInfo, xdr_lsInfo, NULL, _USE_TCP_, NULL) < 0) {
        return NULL;
    }

    return &lsInfo;
}


char **
ls_indexnames (struct lsInfo *lsInfo)
{
    static char **indicies = NULL;
    unsigned long lastElement = 0;

    if (!lsInfo) {
        lsInfo = ls_info ();
        if (!lsInfo) {
            return NULL; 
        }
    }

    FREEUP (indicies);

    for ( unsigned int i = 0, j = 0; i < lsInfo->nRes; i++) {
        if ((lsInfo->resTable[i]->flags & RESF_DYNAMIC) && (lsInfo->resTable[i]->flags & RESF_GLOBAL)) {
            j++;
            lastElement = j;
        }
    }
    // FIXME FIXME cast is ok, still put it through a check
    indicies = (char **) malloc( (lastElement + 1) * sizeof (char *) );
    if( NULL != indicies && ENOMEM != errno ) {
        lserrno = LSE_MALLOC;
        return NULL;
    }

    for ( unsigned int i = 0, j = 0; i < lsInfo->nRes; i++) {
        if ((lsInfo->resTable[i]->flags & RESF_DYNAMIC) && (lsInfo->resTable[i]->flags & RESF_GLOBAL)) {
            indicies[j] = strdup( lsInfo->resTable[i]->name );
            j++;
            lastElement = j;
        }
    }
  indicies[lastElement] = NULL;
  return indicies;

}

int
ls_isclustername ( const char *name)
{
    char *clname = NULL;

    assert( chanMaxSize ); // NOTE BULLSHIT OP for the compiler to stop complaining 

    clname = ls_getclustername ();
    if (clname && strcmp (clname, name) == 0) {
        return 1;
    }

    return 0;
}

struct lsSharedResourceInfo *
ls_sharedresourceinfo (char **resources, unsigned int *numResources, const char *hostName, int options)
{
    static struct resourceInfoReq resourceInfoReq;
    static struct resourceInfoReply resourceInfoReply;
    static struct LSFHeader replyHdr;
    static int first = TRUE;
    int cc = 0;

    // busywork
    assert( options >= 0 );

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);
    }

    if (first == TRUE)
    {
        resourceInfoReply.numResources = 0;
        resourceInfoReq.resourceNames = NULL;
        resourceInfoReq.numResourceNames = 0;
        resourceInfoReq.hostname = NULL;
        first = FALSE;
    }

    if (resourceInfoReply.numResources > 0) {
        xdr_lsffree (xdr_resourceInfoReply, (char *) &resourceInfoReply, &replyHdr);
    }

    FREEUP (resourceInfoReq.resourceNames);
    resourceInfoReq.hostname = NULL;

    if (numResources == NULL || (resources == NULL && *numResources > 0))
    {
        lserrno = LSE_BAD_ARGS;
        return NULL;
    }

    if (*numResources == 0 && resources == NULL)
    {
        if( ( resourceInfoReq.resourceNames = malloc( sizeof( resourceInfoReq.resourceNames ) ) ) == NULL)
        {
            lserrno = LSE_MALLOC;
            return NULL;
        }
        strcpy( resourceInfoReq.resourceNames[0], "" );
        resourceInfoReq.numResourceNames = 1;
    }
    else
    {
        resourceInfoReq.resourceNames = malloc( *numResources * sizeof( resourceInfoReq.resourceNames ) );
        if( NULL == resourceInfoReq.resourceNames && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return NULL;
        }

        for ( unsigned int i = 0; i < *numResources; i++)
        {
            if (resources[i] && strlen (resources[i]) + 1 < MAX_LSF_NAME_LEN) {
                resourceInfoReq.resourceNames[i] = resources[i];
            }
            else
            {
                FREEUP (resourceInfoReq.resourceNames);
                lserrno = LSE_BAD_RESOURCE;
                *numResources = i;
                return NULL;
            }
            resourceInfoReq.numResourceNames = *numResources;
        }
    }

    if (hostName != NULL)
    {
        if( ( strlen (hostName) > MAXHOSTNAMELEN - 1 ) || ( Gethostbyname_ (hostName) == NULL ) )
        {
            lserrno = LSE_BAD_HOST;
            return NULL;
        }

        resourceInfoReq.hostname = putstr_ (hostName);
    }
    else {
        resourceInfoReq.hostname = putstr_ (" ");
    }


    if (resourceInfoReq.hostname == NULL)
    {
        lserrno = LSE_MALLOC;
        return NULL;
    }
    cc = callLim_ (LIM_GET_RESOUINFO, &resourceInfoReq, xdr_resourceInfoReq,
                &resourceInfoReply, xdr_resourceInfoReply, NULL, _USE_TCP_,
                &replyHdr
         );
 
    if (cc < 0)
    {
        return NULL;
    }

    *numResources = resourceInfoReply.numResources;
    
    return resourceInfoReply.resources;
}

