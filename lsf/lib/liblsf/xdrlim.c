/*
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

// #ifdef __APPLE__
// #undef __LP64__ 
// #endif

#include <limits.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/xdrlim.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

static bool_t xdr_hostLoad(             XDR  *, struct hostLoad *,   struct LSFHeader *, char *);
static bool_t xdr_placeInfo(            XDR  *, struct placeInfo *,  struct LSFHeader *);
static bool_t xdr_shortLsInfo(          XDR  *, struct shortLsInfo *, struct LSFHeader *);
static bool_t xdr_resItem(              XDR  *, struct resItem *,    struct LSFHeader *);
static void freeUpMemp(                 char *, int);
static bool_t xdr_lsResourceInfo(       XDR  *, struct lsSharedResourceInfo *, struct LSFHeader *);
static bool_t xdr_lsResourceInstance(   XDR  *, struct lsSharedResourceInstance *, struct LSFHeader *);
int sharedResConfigured_ = FALSE;

static bool_t
xdr_placeInfo (XDR * xdrs, struct placeInfo *placeInfo, struct LSFHeader *hdr)
{
    char *sp;
    
    assert( hdr->length );
    
    sp = placeInfo->hostName;
    if (xdrs->x_op == XDR_DECODE) {
        sp[0] = '\0';
    }
    if (!(xdr_string (xdrs, &sp, MAXHOSTNAMELEN) && xdr_int (xdrs, &placeInfo->numtask))) {
        return FALSE;
    }

    return TRUE;
}

static bool_t
xdr_hostLoad (XDR * xdrs, struct hostLoad *loadVec, struct LSFHeader *hdr,  char *cp)
{
    char *sp;
    int nIndicies = atoi(cp);
    
    assert( hdr->length );
    
    sp = loadVec->hostName;
    if (xdrs->x_op == XDR_DECODE) {
        sp[0] = '\0';
    }
    
    if (!xdr_string (xdrs, &sp, MAXHOSTNAMELEN)) {
        return FALSE;
    }

    for ( int i = 0; i < 1 + GET_INTNUM (nIndicies); i++) {
        if (!xdr_int (xdrs, (int *) &loadVec->status[i])) {
            return FALSE;
        }
    }

    for (int i = 0; i < nIndicies; i++) {
        if (!xdr_float (xdrs, &loadVec->loadIndex[i])) {
            return FALSE;
        }
    }
    
    return TRUE;
}

bool_t
xdr_decisionReq (XDR * xdrs, struct decisionReq * decisionReqPtr, struct LSFHeader * hdr)
{
    char *sp1 = decisionReqPtr->hostType;
    char *sp2 = decisionReqPtr->resReq;
    
    assert( hdr->length );
    
    if (xdrs->x_op == XDR_DECODE) {
        decisionReqPtr->resReq[0] = '\0';
        decisionReqPtr->hostType[0] = '\0';
    }
    
    assert( decisionReqPtr->numHosts <= INT_MAX );
    assert( decisionReqPtr->numPrefs <= INT_MAX );
    if (    !xdr_enum   (xdrs, (enum_t *)&decisionReqPtr->ofWhat)    ||
            !xdr_int    (xdrs, &decisionReqPtr->options)   ||
            !xdr_string (xdrs, &sp1, MAXLSFNAMELEN)        ||
            !xdr_u_long (xdrs, &decisionReqPtr->numHosts)  ||
            !xdr_string (xdrs, &sp2, MAXLINELEN)           ||
            !xdr_u_long (xdrs, &decisionReqPtr->numPrefs))
        {
        return FALSE;
        }
    
    
    if (xdrs->x_op == XDR_DECODE) {
        assert( decisionReqPtr->numPrefs >= 0);
        decisionReqPtr->preferredHosts = calloc ( decisionReqPtr->numPrefs, sizeof (decisionReqPtr->preferredHosts));
        
        if ( NULL == decisionReqPtr->preferredHosts && ENOMEM == errno ) {
            return FALSE;
        }
    }
    
    assert( decisionReqPtr->numPrefs <= UINT_MAX );
    if (!xdr_array_string (xdrs, decisionReqPtr->preferredHosts, MAXHOSTNAMELEN, decisionReqPtr->numPrefs)) {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (decisionReqPtr->preferredHosts);
        }
        return FALSE;
    }
    
    return TRUE;
}

bool_t
xdr_placeReply (XDR * xdrs, struct placeReply *placeRepPtr,struct LSFHeader * hdr)
{
    int status = FALSE;
    static char *memp;
    
    assert( placeRepPtr->numHosts <= INT_MAX );            // leave it in case we ever pass over 32,600 hosts :D
    if (!xdr_u_long (xdrs, &placeRepPtr->numHosts)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        if (memp) {
            FREEUP (memp);
        }
        
        assert(placeRepPtr->numHosts >= 0);
        placeRepPtr->placeInfo = (struct placeInfo *) malloc ( placeRepPtr->numHosts * sizeof (struct placeInfo));
        if (!placeRepPtr->placeInfo) {
            return FALSE;
        }
        memp = (char *) placeRepPtr->placeInfo;
    }
    
    
    for (size_t i = 0; i < placeRepPtr->numHosts; i++) {
        status = xdr_arrayElement (xdrs, (char *) &placeRepPtr->placeInfo[i], hdr, xdr_placeInfo);
        if (XDR_DECODE == xdrs->x_op && !status )  {
            FREEUP (memp);
            return FALSE;
            }
    }
    return TRUE;
}

bool_t
xdr_loadReply (XDR * xdrs, struct loadReply *loadReplyPtr, struct LSFHeader *hdr)
{
    char *sp;
    int status = TRUE;
    static float *memp;
    
    assert( loadReplyPtr->nEntry <= INT_MAX );
    if (!(xdr_int (xdrs, (int *) &loadReplyPtr->nEntry)&& xdr_int (xdrs, (int *)&loadReplyPtr->nIndex))) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        
        u_long matSize  = 0;
        u_long nameSize = 0;
        u_long staSize  = 0;;
        u_long vecSize  = 0;
        u_long hlSize   = 0;
        float *currp;
        
        FREEUP (memp);
        if (loadReplyPtr->indicies == NULL) {
            assert(loadReplyPtr->nIndex  >= 0 );
            loadReplyPtr->indicies = malloc (sizeof (char *) * (loadReplyPtr->nIndex + 1 ) ); // FIXME FIXME FIXME what parenthesis should the + 1 be located
        }
        
        vecSize  = loadReplyPtr->nIndex * sizeof (float);
        hlSize   = ALIGNWORD_ (loadReplyPtr->nEntry * sizeof (struct hostLoad)); // FIXME FIXME FIXME what does ALIGNWORD_( ) do?
        matSize  = ALIGNWORD_((loadReplyPtr->nEntry + 1) * vecSize);
        nameSize = ALIGNWORD_ (loadReplyPtr->nIndex * MAXLSFNAMELEN);
        staSize  = ALIGNWORD_ ((1 + GET_INTNUM (loadReplyPtr->nIndex)) * sizeof (int));
        loadReplyPtr->loadMatrix = malloc (hlSize + matSize + nameSize + loadReplyPtr->nEntry * staSize);
        
        if (!loadReplyPtr->loadMatrix) {
            memp = NULL;
            return FALSE;
        }
        
        //memp = loadReplyPtr->loadMatrix;
        memp = loadReplyPtr->loadMatrix->loadIndex;
        
        currp = memp + hlSize;
        for (u_long i = 0; i < loadReplyPtr->nEntry; i++, currp += vecSize) {
            loadReplyPtr->loadMatrix[i].loadIndex = currp;
        }
        currp = memp + hlSize + matSize;
        for (u_long i = 0; i < loadReplyPtr->nIndex; i++, currp += MAXLSFNAMELEN) {
            char *buf = malloc( sizeof( char ) * sizeof( float ) + 1 );
            sprintf( buf, "%f", *currp );
            loadReplyPtr->indicies[i] = buf;
            free( buf ); // this probably makes things go wonky
        }
        for (u_long i = 0; i < loadReplyPtr->nEntry; i++, currp += staSize) {
            loadReplyPtr->loadMatrix[i].status = currp;
        }
    }
    
    for (u_long i = 0; i < loadReplyPtr->nIndex; i++) {
        sp = loadReplyPtr->indicies[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp = NULL; // FIME FIXME FIXME FIXME not quite sure if this is correct
        }
        if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
            return FALSE;
        }
    }
    
    loadReplyPtr->indicies[loadReplyPtr->nIndex - 1] = NULL;
    
    for (u_long i = 0; i < loadReplyPtr->nEntry; i++) {
        status = xdr_arrayElement (xdrs, (char *) &loadReplyPtr->loadMatrix[i], hdr, xdr_hostLoad, (char *) &loadReplyPtr->nIndex);
        
        if (!status) {
            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    if (!xdr_int (xdrs, &loadReplyPtr->flags)) {
        return FALSE;
    }
    
    return TRUE;
}

bool_t
xdr_jobXfer (XDR * xdrs, struct jobXfer * jobXferPtr, struct LSFHeader * hdr)
{
    char *sp;
    static char *memp;
    
    sp = jobXferPtr->resReq;
    if (xdrs->x_op == XDR_DECODE) {
        sp[0] = '\0';
    }
    
    assert( jobXferPtr->numHosts <= INT_MAX );
    if (!(xdr_int (xdrs, (int *) &jobXferPtr->numHosts) && xdr_string (xdrs, &sp, MAXLINELEN))) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {

        if( memp ) {
            FREEUP (memp);
        }
        jobXferPtr->placeInfo = (struct placeInfo *) malloc( jobXferPtr->numHosts * sizeof (struct placeInfo));
        if ( NULL == jobXferPtr->placeInfo && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return FALSE;
        }
        memp = (char *) jobXferPtr->placeInfo;
    }
    
    
    for ( size_t i = 0; i < jobXferPtr->numHosts; i++) {
        
        int status = xdr_arrayElement (xdrs, (char *) &jobXferPtr->placeInfo[i], hdr, xdr_placeInfo);
        if (!status) {
            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    return TRUE;
}

bool_t
xdr_hostInfoReply (XDR *xdrs, struct hostInfoReply *hostInfoReply, struct LSFHeader *hdr)
{
    int status = 0;
    static struct shortHInfo *memp = NULL;
    
    assert( hostInfoReply->nHost  <= INT_MAX );
    assert( hostInfoReply->nIndex <= INT_MAX );
    if (!(xdr_int (xdrs, (int *) &hostInfoReply->nHost) && xdr_int (xdrs, (int *)&hostInfoReply->nIndex))) {
        return FALSE;
    }
    
    if (!xdr_shortLsInfo (xdrs, hostInfoReply->shortLsInfo, hdr)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        
        u_long shISize = 0;
        u_long matSize = 0;
        u_long vecSize = 0;
        u_long resSize = 0;
        float *currp  = 0;
        
        if (memp) {
            FREEUP (memp);
        }
        
        shISize = ALIGNWORD_ (hostInfoReply->nHost * sizeof (struct shortHInfo));
        assert( hostInfoReply->nIndex >= 0 );
        vecSize = ALIGNWORD_ ( (u_long)hostInfoReply->nIndex * sizeof (float));
        resSize = (u_long)(GET_INTNUM (hostInfoReply->shortLsInfo->nRes) + GET_INTNUM (hostInfoReply->nIndex)) * sizeof (int);
        matSize = ALIGNWORD_ (hostInfoReply->nHost * (vecSize + resSize + MAXLINELEN + 100));
        
        hostInfoReply->hostMatrix = (struct shortHInfo *) malloc (shISize + matSize);
        if ( NULL == hostInfoReply->hostMatrix && ENOMEM == errno ) {
            return FALSE;
        }
        
        memp = hostInfoReply->hostMatrix;
        currp = memp->busyThreshold + shISize;                          // FIXME FIXME FIXME the fuck is a float = struct shortHInfo + ulong ?
        for ( u_long i = 0; i < hostInfoReply->nHost; i++) {     //          it makes more sense if float = memp->busyThreshold + ulong
            
            hostInfoReply->hostMatrix[i].busyThreshold = currp;         // FIXME FIXME FIXME FIXME  is this where currps should be assigned to?
            currp += vecSize;
            hostInfoReply->hostMatrix[i].resBitMaps    = (unsigned int *) currp; // FIXME FIXME FIXME FIXME   why is the same value assinged to three different members of the structure?
            currp += resSize;
            hostInfoReply->hostMatrix[i].windows       = (char *)currp; // FIXME FIXME FIXME FIXME   why is the same value assinged to three different members of the structure?
            currp += MAXLINELEN;
        }
    }
    
    for ( u_long i = 0; i < hostInfoReply->nHost; i++) {
        
        status = xdr_arrayElement (xdrs, (char *) &hostInfoReply->hostMatrix[i], hdr, xdr_shortHInfo, (char *) &hostInfoReply->nIndex);
        if (XDR_DECODE == xdrs->x_op && !status) {
            FREEUP (memp);
            return FALSE;
        }
    }

    for ( u_long i = 0; i < hostInfoReply->nHost; i++) {
        if (!xdr_int (xdrs, (int *) &(hostInfoReply->hostMatrix[i].maxMem))) {

            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    return TRUE;
    
}

bool_t
xdr_shortHInfo (XDR *xdrs, struct shortHInfo *shortHInfo, struct LSFHeader *hdr, char *nIndex)
{
    char window[MAXLINELEN];
    char *sp;
    char *sp1 = window;
    unsigned int tIndx = 0;
    unsigned int mIndx = 0;
    unsigned int mem   = 0;
    unsigned int cpus  = 0;
    unsigned int a = 0;
    unsigned int    b = 0;
    //    int     i = 0;
    long *nIndicies = 0;
    *nIndicies = atol( nIndex );
    
    assert( hdr->length );
    
    sp = shortHInfo->hostName;
    if (xdrs->x_op == XDR_DECODE) {
        sp[0] = '\0';
        sp1[0] = '\0';
    }
    
    if (xdrs->x_op == XDR_ENCODE) {

//        if (shortHInfo->hTypeIndx >= 0) {
            tIndx = MIN (MAXTYPES, shortHInfo->hTypeIndx);
//        }
//        else {
//            tIndx = MAXTYPES;
//        }
        
        tIndx &= 0x7FFF;
        
        if (shortHInfo->windows[0] != '-') {
            tIndx |= 0x8000;
        }
        
//        if (shortHInfo->hModelIndx >= 0) {
            mIndx = MIN (MAXMODELS, shortHInfo->hModelIndx);
//        }
//        else {
//            mIndx = MAXMODELS;
//        }
        
        a = tIndx << 16;
        a &= 0xffffffff;
        a = a + mIndx;
        mem = shortHInfo->maxMem;
        cpus = shortHInfo->maxCpus;
        b = mem << 16;
        b &= 0xffffffff;
        b = b + cpus;
        
        if (logclass & LC_TRACE)
            ls_syslog (LOG_DEBUG2, "xdr_shortHInfo: host <%s>  type = %d, model = %d", shortHInfo->hostName, tIndx, mIndx);
        }
    
        if (!xdr_string (xdrs, &sp, MAXHOSTNAMELEN) ||
            !xdr_u_int  (xdrs, &a) ||
            !xdr_u_int  (xdrs, &b) ||
            !xdr_u_int  (xdrs, &shortHInfo->resClass))
        {
            return FALSE;
        }
    
        if ((xdrs->x_op == XDR_ENCODE) && (shortHInfo->windows[0] != '-'))
        {
            sp1 = shortHInfo->windows;
            if (!xdr_string (xdrs, &sp1, MAXLINELEN)) {
                return FALSE;
        }
    }
    
    if (xdrs->x_op == XDR_DECODE) {

        if ((a >> 16) & 0x8000) {
        
            if (!xdr_string (xdrs, &sp1, MAXLINELEN)) {
                return FALSE;
            }
        }
        else {
            sp1[0] = '-';
            sp1[1] = '\0';
        }
        
        strcpy (shortHInfo->windows, sp1);
        shortHInfo->hTypeIndx = (a >> 16) & 0x7FFF;
        a <<= 16;
        
        a &= 0xffffffff;
        shortHInfo->hModelIndx = a >> 16; // FIXME FIXME FIXME why the roll?
        shortHInfo->maxMem = b >> 16;
        b <<= 16;
        
        b &= 0xffffffff;
        shortHInfo->maxCpus = (b >> 16);
    }
    
    for (int i = 0; i < *nIndicies; i++)  {
        if (!xdr_float (xdrs, &shortHInfo->busyThreshold[i])) {
            return FALSE;
        }
    }
    
    if (!xdr_int (xdrs, (int *)&shortHInfo->flags) ||
        !xdr_int (xdrs, (int *)&shortHInfo->rexPriority) ||
        !xdr_int (xdrs, (int *)&shortHInfo->nDisks) ||
        !xdr_int (xdrs, (int *)&shortHInfo->maxSwap) ||
        !xdr_int (xdrs, (int *)&shortHInfo->maxTmp))
    {
        return FALSE;
    }
    
    if (!xdr_int (xdrs, &shortHInfo->nRInt)) {
        return FALSE;
    }
    
    for (int i = 0; i < shortHInfo->nRInt; i++) {
        if (!xdr_u_int (xdrs, &shortHInfo->resBitMaps[i])) {
            return FALSE;
        }
    }
    
    if (shortHInfo->flags & HINFO_SHARED_RESOURCE) {
        sharedResConfigured_ = TRUE;
    }
    
    return TRUE;
}

static bool_t
xdr_shortLsInfo (XDR * xdrs, struct shortLsInfo *shortLInfo, struct LSFHeader *hdr)
{
    char *sp;
    static float *currp;
    static char *memp;
    
    assert( hdr->length );
    
    assert( shortLInfo->nRes    >= 0 );
    assert( shortLInfo->nTypes  >= 0 );
    assert( shortLInfo->nModels >= 0 );
    if (!xdr_int (xdrs, (int *)&shortLInfo->nRes) ||
        !xdr_int (xdrs, (int *)&shortLInfo->nTypes) ||
        !xdr_int (xdrs, (int *)&shortLInfo->nModels))
    {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        if (memp) {
            free (memp);
        }
        
        memp = (char *) malloc ((shortLInfo->nRes + shortLInfo->nTypes + shortLInfo->nModels) * MAXLSFNAMELEN + shortLInfo->nRes * sizeof (char *));
        if ( NULL == memp && ENOMEM == errno ) {
            return FALSE;
        }
        
        currp = (float *)memp; // FIXME FIXME FIXME FIXME FIXME lookup in debugger
        shortLInfo->resName = (char **) currp;  // FIXME FIXME FIXME FIXME FIXME lookup in debugger
        currp += shortLInfo->nRes * sizeof (char *);
        
        for ( u_long i = 0; i < shortLInfo->nRes; i++, currp += MAXLSFNAMELEN) {
            shortLInfo->resName[i] = (char *)currp;     // FIXME FIXME FIXME FIXME FIXME lookup in debugger
        }
        
        for ( u_long i = 0; i < shortLInfo->nTypes; i++, currp += MAXLSFNAMELEN) {
            shortLInfo->hostTypes[i] = (char *)currp;   // FIXME FIXME FIXME FIXME FIXME lookup in debugger
        }
        
        for ( u_long i = 0; i < shortLInfo->nModels; i++, currp += MAXLSFNAMELEN) {
            shortLInfo->hostModels[i] = (char *)currp;  // FIXME FIXME FIXME FIXME FIXME lookup in debugger
        }
    }
    
    for ( u_long i = 0; i < shortLInfo->nRes; i++) {

        sp = shortLInfo->resName[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp[0] = '\0';
        }
    
        if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
            return FALSE;
        }
    }
    
    for ( u_long i = 0; i < shortLInfo->nTypes; i++) {

        sp = shortLInfo->hostTypes[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp[0] = '\0';
        }
        
        if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
            return FALSE;
        }
    }
    
    for (u_long i = 0; i < shortLInfo->nModels; i++) {

        sp = shortLInfo->hostModels[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp[0] = '\0';
        }
        
        if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
            return FALSE;
        }
    }
    
    for (u_long i = 0; i < shortLInfo->nModels; i++) {
        if (!xdr_double( xdrs, &shortLInfo->cpuFactors[i] ) ) {
            return FALSE;
        }
    }
    
    return TRUE;
}

bool_t
xdr_limLock (XDR *xdrs, struct limLock *limLockPtr, struct LSFHeader *hdr)
{
    char *sp;
    
    assert( hdr->length );
    
    sp = limLockPtr->lsfUserName;
    if (xdrs->x_op == XDR_DECODE)
        {
        sp[0] = '\0';
        }
    
    assert( limLockPtr->uid >= 0 );
    if (!xdr_int (xdrs, &limLockPtr->on)
        || !xdr_time_t (xdrs, &limLockPtr->time)
        || !xdr_int (xdrs, (int *)&limLockPtr->uid))
    {
        return FALSE;
    }
    
    if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
        return FALSE;
    }
    
    return TRUE;
}

static bool_t
xdr_resItem (XDR * xdrs, struct resItem *resItem, struct LSFHeader *hdr)
{
    char *sp, *sp1;
    
    assert( hdr->length );
    
    sp = resItem->des;
    sp1 = resItem->name;
    if (xdrs->x_op == XDR_DECODE)
        {
        sp[0] = '\0';
        sp1[0] = '\0';
        }
    if (!xdr_string (xdrs, &sp1, MAXLSFNAMELEN) ||
        !xdr_string (xdrs, &sp, MAXRESDESLEN) ||
        !xdr_enum (xdrs, (int *) &resItem->valueType) ||
        !xdr_enum (xdrs, (int *) &resItem->orderType) ||
        !xdr_int (xdrs, &resItem->flags) || !xdr_int (xdrs, &resItem->interval))
        return FALSE;
    return TRUE;
    
}

bool_t
xdr_lsInfo (XDR * xdrs, struct lsInfo * lsInfoPtr, struct LSFHeader *hdr)
{
    char *sp = NULL;
    static char *memp = NULL;
    
    if (!xdr_u_int (xdrs, &lsInfoPtr->nRes)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        FREEUP (memp);
        assert( lsInfoPtr->nRes >= 0);
        lsInfoPtr->resTable = malloc ( lsInfoPtr->nRes * sizeof (struct resItem) );
        if ( NULL == lsInfoPtr->resTable && ENOMEM == errno ) {
            return FALSE;
        }
        memp = (char *) lsInfoPtr->resTable;
    }
    
    for ( unsigned int i = 0; i < lsInfoPtr->nRes; i++) {
        if (!xdr_arrayElement (xdrs, (char *) &lsInfoPtr->resTable[i], hdr, xdr_resItem)) {
            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    if( !xdr_u_int( xdrs, &lsInfoPtr->nTypes ) ) {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (memp);
        }
        return FALSE;
    }
    
    for ( unsigned int i = 0; i < lsInfoPtr->nTypes; i++) {
        sp = lsInfoPtr->hostTypes[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp[0] = '\0';
        }
        if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    if (!xdr_u_int( xdrs, &lsInfoPtr->nModels ) ) {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (memp);
        }
        return FALSE;
    }
    
    for ( unsigned int i = 0; i < lsInfoPtr->nModels; i++) {
        sp = lsInfoPtr->hostModels[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp[0] = '\0';
        }
        if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    if (!xdr_u_int (xdrs, &lsInfoPtr->numUsrIndx) || !xdr_u_int (xdrs, &lsInfoPtr->numIndx)) {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (memp);
        }
        return FALSE;
    }
    
    for ( unsigned int i = 0; i < lsInfoPtr->nModels; i++) {
        // assert( lsInfoPtr->cpuFactor <= DBL_MAX) // FIXME FIXME lookup max value of a double in limits.h
        if (!xdr_float (xdrs, (float *)&lsInfoPtr->cpuFactor[i]) && xdrs->x_op == XDR_DECODE) {
            FREEUP (memp);
            return FALSE;
        }
    }

    
    for ( unsigned int i = 0; i < lsInfoPtr->nModels; ++i) {
        sp = lsInfoPtr->hostArchs[i];
        if (xdrs->x_op == XDR_DECODE) {
            sp[0] = '\0';
        }
        if (!xdr_string (xdrs, &sp, MAXLSFNAMELEN)) {
            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    for ( unsigned int i = 0; i < lsInfoPtr->nModels; ++i) {
        if (!xdr_u_int( xdrs, &lsInfoPtr->modelRefs[ i ] ) ) {
            if (xdrs->x_op == XDR_DECODE) {
                FREEUP (memp);
            }
            return FALSE;
        }
    }
    
    return TRUE;
    
}

bool_t
xdr_masterInfo (XDR * xdrs, struct masterInfo * mInfoPtr, struct LSFHeader * hdr)
{
    char *sp;
    
    assert( hdr->length);
    
    sp = mInfoPtr->hostName;
    if (xdrs->x_op == XDR_DECODE) {
        sp[0] = '\0';
    }
    
    if (!xdr_string (xdrs, &sp, MAXHOSTNAMELEN)) {
        return FALSE;
    }
    
    if (!xdr_address (xdrs, &mInfoPtr->addr)) {
        return FALSE;
    }
    
    if (!xdr_portno (xdrs, &mInfoPtr->portno)) {
        return FALSE;
    }
    
    return TRUE;
}

bool_t
xdr_clusterInfoReq (XDR *xdrs, struct clusterInfoReq *clusterInfoReq, struct LSFHeader *hdr)
{
    char line[MAXLINELEN];
    char *sp = line;
    
    assert( hdr->length);
    
    if (xdrs->x_op == XDR_DECODE) {
        if (!xdr_string (xdrs, &sp, MAXLINELEN)) {
            return FALSE;
        }
        clusterInfoReq->resReq = putstr_ (line);
        if (clusterInfoReq->resReq == NULL) {
            return FALSE;
        }
    }
    else
    {
        if (!xdr_string (xdrs, &clusterInfoReq->resReq, MAXLINELEN)) {
            return FALSE;
        }
    }
    
    if (!xdr_int (xdrs, &clusterInfoReq->listsize) || !xdr_int (xdrs, &clusterInfoReq->options))
    {
        if (xdrs->x_op == XDR_DECODE) {
            free (clusterInfoReq->resReq);
        }
        return FALSE;
    }
    
    if (clusterInfoReq->listsize && xdrs->x_op == XDR_DECODE)
    {
    assert( clusterInfoReq->listsize <= INT_MAX );
        clusterInfoReq->clusters = (char **) calloc ( (u_long) clusterInfoReq->listsize, sizeof (char *));
        if( NULL == clusterInfoReq->clusters && ENOMEM == errno ) {
            free (clusterInfoReq->resReq);
            return FALSE;
        }
    }
    
    assert( clusterInfoReq->listsize >= 0);
    if (!xdr_array_string (xdrs, clusterInfoReq->clusters, MAXLSFNAMELEN, clusterInfoReq->listsize))
    {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (clusterInfoReq->resReq);
            FREEUP (clusterInfoReq->clusters);
        }
        return FALSE;
    }
    
    return TRUE;
    
}

bool_t
xdr_clusterInfoReply (XDR * xdrs, struct clusterInfoReply * clusterInfoReply, struct LSFHeader * hdr)
{
    static char *memp = NULL;
    static unsigned int nClusters = 0;

    if (!xdr_u_int (xdrs, &clusterInfoReply->nClusters)) {
        return FALSE;
    }

    if (!xdr_arrayElement (xdrs, (char *) clusterInfoReply->shortLsInfo, hdr, xdr_shortLsInfo)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        if (memp) {
            assert( nClusters <= INT_MAX );
            freeUpMemp (memp, nClusters);
        }

        memp = calloc (clusterInfoReply->nClusters, sizeof (struct shortCInfo));
        if ( NULL == memp && ENOMEM == errno ) {
            nClusters = 0;
            return FALSE;
        }
        
        clusterInfoReply->clusterMatrix = (struct shortCInfo *) memp; // FIXME FIXME FIXME FIXME FIXME member-for-member assignment!
    }

    for ( u_long i = 0; i < clusterInfoReply->nClusters; i++) {

        bool_t status = 0;
        status = xdr_arrayElement (xdrs, (char *) &clusterInfoReply->clusterMatrix[i], hdr, xdr_shortCInfo);
        
        if (!status) {

            if (xdrs->x_op == XDR_DECODE) {
                nClusters = 0;
                assert( i <= INT_MAX) ;
                freeUpMemp (memp, (int) i - 1);
            }
            
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_DECODE) {
        nClusters = clusterInfoReply->nClusters;
    }

    return TRUE;
}

static void
freeUpMemp (char *memp, int nClus)
{
    struct shortCInfo *clusterMatrix;

    clusterMatrix = (struct shortCInfo *) memp; // FIXME FIXME FIXME FIXME FIXME member-for-member assignment!
    for (int i = 0; i < nClus; i++) {

        FREEUP (clusterMatrix[i].resBitMaps);
        FREEUP (clusterMatrix[i].hostTypeBitMaps);
        FREEUP (clusterMatrix[i].hostModelBitMaps);

        if (clusterMatrix[i].nAdmins > 0) {
            for (u_long j = 0; j < clusterMatrix[i].nAdmins; j++) {
                FREEUP (clusterMatrix[i].admins[j]);
            }
            
            FREEUP (clusterMatrix[i].adminIds);
            FREEUP (clusterMatrix[i].admins);
        }
    }

    FREEUP (memp);
}

bool_t
xdr_shortCInfo (XDR * xdrs, struct shortCInfo *clustInfoPtr, struct LSFHeader *hdr)
{
    char *sp1, *sp2, *sp3;
    
    assert( hdr->length );
    
    sp1 = clustInfoPtr->clName;
    sp2 = clustInfoPtr->masterName;
    sp3 = clustInfoPtr->managerName;
    
    if (xdrs->x_op == XDR_DECODE) {
        sp1[0] = '\0';
        sp2[0] = '\0';
        sp3[0] = '\0';
        clustInfoPtr->nAdmins = 0;
        clustInfoPtr->adminIds = NULL;
        clustInfoPtr->admins = NULL;
    }
    
    if (!(xdr_string ( xdrs, &sp1, MAXLSFNAMELEN)       &&
          xdr_string ( xdrs, &sp2, MAXHOSTNAMELEN)      &&
          xdr_string ( xdrs, &sp3, MAXLSFNAMELEN)       &&
          xdr_u_int  ( xdrs, &clustInfoPtr->status)     &&
          xdr_u_int  ( xdrs, &clustInfoPtr->numServers) &&
          xdr_u_int  ( xdrs, &clustInfoPtr->numClients) &&
          xdr_u_int  ( xdrs, &clustInfoPtr->managerId)  &&
          xdr_u_int  ( xdrs, &clustInfoPtr->resClass)   &&
          xdr_u_int  ( xdrs, &clustInfoPtr->typeClass)  &&
          xdr_u_int  ( xdrs, &clustInfoPtr->modelClass) &&
          xdr_u_int  ( xdrs, &clustInfoPtr->numIndx)    &&
          xdr_u_int  ( xdrs, &clustInfoPtr->numUsrIndx) &&
          xdr_u_int  ( xdrs, &clustInfoPtr->usrIndxClass)))
    {
        return FALSE;
    }
    
    assert( clustInfoPtr->nAdmins >= 0 );
    if (!xdr_u_int (xdrs, &clustInfoPtr->nAdmins)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && clustInfoPtr->nAdmins > 0) {
        
        clustInfoPtr->admins   = malloc (clustInfoPtr->nAdmins * sizeof (char *));
        if ( NULL == clustInfoPtr->admins && ENOMEM == errno ) {
            FREEUP (clustInfoPtr->adminIds);
            FREEUP (clustInfoPtr->admins);
            clustInfoPtr->nAdmins = 0;
            return FALSE;
        }
        
        clustInfoPtr->adminIds = malloc (clustInfoPtr->nAdmins * sizeof (int));
        if ( NULL == clustInfoPtr->adminIds && ENOMEM == errno ) {
            FREEUP (clustInfoPtr->adminIds);
            FREEUP (clustInfoPtr->admins);
            clustInfoPtr->nAdmins = 0;
            return FALSE;
        }
    }
    
    for ( u_long i = 0; i < clustInfoPtr->nAdmins; i++) {
        if (!(xdr_var_string (xdrs, &clustInfoPtr->admins[i]) && xdr_u_int (xdrs, &clustInfoPtr->adminIds[i]))) {
            return FALSE;
        }
    }
    
    if (!xdr_u_int (xdrs, &clustInfoPtr->nRes)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && clustInfoPtr->nRes) {
        
        clustInfoPtr->resBitMaps = malloc( GET_INTNUM (clustInfoPtr->nRes) * sizeof( unsigned int ) );
        if ( NULL == clustInfoPtr->resBitMaps && ENOMEM == errno ) {
            clustInfoPtr->nRes = 0;
            return FALSE;
        }
    }
    
    for ( unsigned int i = 0; (i < clustInfoPtr->nRes && i < GET_INTNUM (clustInfoPtr->nRes)); i++) {
        if (!(xdr_u_int (xdrs, &clustInfoPtr->resBitMaps[i]))) {
            return FALSE;
        }
    }
    
    if (!xdr_u_int (xdrs, &clustInfoPtr->nTypes)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && clustInfoPtr->nTypes) {
        
        clustInfoPtr->hostTypeBitMaps = malloc ( GET_INTNUM (clustInfoPtr->nTypes) * sizeof( unsigned int ) );
        if ( NULL == clustInfoPtr->hostTypeBitMaps && ENOMEM == errno ) {
            clustInfoPtr->nTypes = 0;
            return FALSE;
        }
    }
    
    for ( unsigned int i = 0; (i < clustInfoPtr->nTypes && i < GET_INTNUM (clustInfoPtr->nTypes)); i++) {
        if (!(xdr_u_int (xdrs, &clustInfoPtr->hostTypeBitMaps[i]))) {
            return FALSE;
        }
    }
    
    if (!xdr_u_int (xdrs, &clustInfoPtr->nModels)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && clustInfoPtr->nModels) {
        
        assert(clustInfoPtr->nModels >= 0);
        clustInfoPtr->hostModelBitMaps = malloc ( GET_INTNUM( clustInfoPtr->nModels ) * sizeof( unsigned int ) );
        if ( NULL == clustInfoPtr->hostModelBitMaps && ENOMEM == errno ) {
            clustInfoPtr->nModels = 0;
            return FALSE;
        }
    }
    
    for ( unsigned int i = 0; (i < clustInfoPtr->nModels && i < GET_INTNUM (clustInfoPtr->nModels)); i++) {
        if (!(xdr_u_int (xdrs, &clustInfoPtr->hostModelBitMaps[i]))) {
            return FALSE;
        }
    }
    
    return TRUE;
    
}

bool_t
xdr_cInfo (XDR *xdrs, struct cInfo *cInfo, struct LSFHeader *hdr)
{
    
    char *sp1 = NULL;
    char *sp2 = NULL;
    char *sp3 = NULL;
    
    sp1 = cInfo->clName;
    sp2 = cInfo->masterName;
    sp3 = cInfo->managerName;
    
    if (xdrs->x_op == XDR_DECODE) { // FIXME FIXME FIXME FIXME pointers should be cleaned out before NULLed
        sp1[0] = '\0';
        sp2[0] = '\0';
        sp3[0] = '\0';
        cInfo->nAdmins = 0;
        cInfo->adminIds = NULL;
        cInfo->admins = NULL;
    }
    
    if (!(xdr_string (xdrs, &sp1, MAXLSFNAMELEN) &&
          xdr_string (xdrs, &sp2, MAXHOSTNAMELEN)&&
          xdr_string (xdrs, &sp3, MAXLSFNAMELEN) &&
          xdr_u_int    (xdrs, &cInfo->status)      &&
          xdr_u_int    (xdrs, &cInfo->numServers)  &&
          xdr_u_int    (xdrs, &cInfo->numClients)  &&
          xdr_int      (xdrs, &cInfo->managerId)   &&
          xdr_u_int    (xdrs, &cInfo->resClass)    &&
          xdr_u_int    (xdrs, &cInfo->typeClass)   &&
          xdr_u_int    (xdrs, &cInfo->modelClass)  &&
          xdr_u_int    (xdrs, &cInfo->numIndx)     &&
          xdr_u_int    (xdrs, &cInfo->numUsrIndx)  &&
          xdr_u_int    (xdrs, &cInfo->usrIndxClass)))
    {
        return FALSE;
    }
    
    assert( cInfo->nAdmins <= INT_MAX );
    if (!xdr_int (xdrs, (int *)&cInfo->nAdmins))  {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && cInfo->nAdmins > 0) {
        
        cInfo->admins = malloc (cInfo->nAdmins * sizeof( cInfo->admins ) );
        if ( NULL == cInfo->admins && ENOMEM == errno ) {
            FREEUP (cInfo->adminIds);
            FREEUP (cInfo->admins);
            cInfo->nAdmins = 0;
            return FALSE;
        }
        
        cInfo->adminIds = malloc (cInfo->nAdmins * sizeof( unsigned int ) );
        if ( NULL == cInfo->adminIds && ENOMEM == errno ) {
            FREEUP (cInfo->adminIds);
            FREEUP (cInfo->admins);
            cInfo->nAdmins = 0;
            return FALSE;
        }
    }
    
    for (u_long i = 0; i < cInfo->nAdmins; i++) {
        if (!(xdr_var_string (xdrs, &cInfo->admins[i]) && xdr_int (xdrs, &cInfo->adminIds[i])))  {
            return FALSE;
        }
    }
    
    if (!xdr_u_int (xdrs, &cInfo->nRes)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && cInfo->nRes) {
        cInfo->resBitMaps = malloc( GET_INTNUM( cInfo->nRes ) * sizeof( unsigned int ) );
        if ( NULL == cInfo->resBitMaps && ENOMEM == errno ) {
            cInfo->nRes = 0;
            return FALSE;
        }
    }
    
    for( unsigned int i = 0; (i < cInfo->nRes && i < GET_INTNUM (cInfo->nRes)); i++) {
        if (!(xdr_u_int (xdrs, &cInfo->resBitMaps[i]))) {
            return FALSE;
        }
    }
    
    if (cInfo->numIndx > 0 && xdrs->x_op == XDR_DECODE) {
        
        assert( cInfo->numIndx >= 0);
        cInfo->loadIndxNames = (char **) calloc ( (u_long)cInfo->numIndx, sizeof (char *));
        if (NULL == cInfo->loadIndxNames && ENOMEM == errno ) {
            return FALSE;
        }
    }
    
    if (!xdr_shortLsInfo (xdrs, &(cInfo->shortInfo), hdr)) {
        return FALSE;
    }
    
    if (cInfo->numIndx > 0) {
        if (!xdr_array_string (xdrs, cInfo->loadIndxNames, MAXLSFNAMELEN, cInfo->numIndx)) {
            FREEUP (cInfo->loadIndxNames);
            return FALSE;
        }
    }
    
    if (!xdr_u_int (xdrs, &cInfo->nTypes)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && cInfo->nTypes) {
        assert( cInfo->nTypes >=0 );
        cInfo->hostTypeBitMaps = malloc( GET_INTNUM( cInfo->nTypes ) * sizeof( unsigned int ) );
        if ( NULL == cInfo->hostTypeBitMaps && ENOMEM == errno ) {
            cInfo->nTypes = 0;
            return FALSE;
        }
    }
    
    
    for (unsigned int  i = 0; (i < cInfo->nTypes && i < GET_INTNUM (cInfo->nTypes)); i++) {
        if (!(xdr_u_int (xdrs, &cInfo->hostTypeBitMaps[i]))) {
            return FALSE;
        }
    }
    
    if (!xdr_u_int (xdrs, &cInfo->nModels)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && cInfo->nModels) {
        
        cInfo->hostModelBitMaps = calloc( GET_INTNUM( cInfo->nModels ) * sizeof( unsigned int ), sizeof( unsigned int ) );
        if (NULL == cInfo->hostModelBitMaps && ENOMEM == errno) {
            cInfo->nModels = 0;
            return FALSE;
        }
    }
    
    for( unsigned int i = 0; (i < cInfo->nModels && i < GET_INTNUM (cInfo->nModels)); i++) {
        if (!(xdr_u_int (xdrs, &cInfo->hostModelBitMaps[i]))) {
            return FALSE;
        }
    }
    
    return TRUE;
    
}

bool_t
xdr_resourceInfoReq (XDR * xdrs, struct resourceInfoReq * resourceInfoReq, struct LSFHeader * hdr)
{

    assert( hdr->length );
    
    if (xdrs->x_op == XDR_DECODE) {
        resourceInfoReq->hostName         = NULL;
        resourceInfoReq->resourceNames    = NULL;
        resourceInfoReq->numResourceNames = 0;
    }

    if (!(xdr_u_int (xdrs, &resourceInfoReq->numResourceNames) && xdr_int (xdrs, &resourceInfoReq->options))) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && resourceInfoReq->numResourceNames > 0)
        {
        assert( resourceInfoReq->numResourceNames >= 0);
        resourceInfoReq->resourceNames = malloc( resourceInfoReq->numResourceNames * sizeof( resourceInfoReq->resourceNames ) + 1 );
        if( NULL == resourceInfoReq->resourceNames  && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return FALSE;
        }

    }
    
    for ( unsigned int  i = 0; i < resourceInfoReq->numResourceNames; i++) {
        if (!xdr_var_string (xdrs, &resourceInfoReq->resourceNames[i])) {
            resourceInfoReq->numResourceNames = i;
            return FALSE;
        }
    }
    
    if (!xdr_var_string (xdrs, &resourceInfoReq->hostName)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_FREE && resourceInfoReq->numResourceNames > 0) {
        FREEUP (resourceInfoReq->resourceNames);
        resourceInfoReq->numResourceNames = 0;
    }

    return TRUE;
    
}

bool_t
xdr_resourceInfoReply (XDR * xdrs, struct resourceInfoReply * resourceInfoReply, struct LSFHeader * hdr)
{
    int status = 0;
    
    
    if (xdrs->x_op == XDR_DECODE) {
        resourceInfoReply->numResources = 0;
        resourceInfoReply->resources = NULL;
    }
    if (!(xdr_u_int (xdrs, &resourceInfoReply->numResources) && xdr_u_int (xdrs, &resourceInfoReply->badResource))) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && resourceInfoReply->numResources > 0) {
        
        assert( resourceInfoReply->numResources >= 0);
        resourceInfoReply->resources = malloc( resourceInfoReply->numResources * sizeof( struct lsSharedResourceInfo ) );
        if ( NULL == resourceInfoReply->resources && resourceInfoReply->resources && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return FALSE;
        }
    }
    
    for ( unsigned int i = 0; i < resourceInfoReply->numResources; i++) {
        status = xdr_arrayElement (xdrs, (char *) &resourceInfoReply->resources[i], hdr, xdr_lsResourceInfo); // FIXME FIXME FIXME fatal error: incompatible pointer types passing 'struct lsSharedResourceInfo *' to parameter of type 'char *'
        if (!status) {
            resourceInfoReply->numResources = i;
            return FALSE;
        }
    }
    
    if (xdrs->x_op == XDR_FREE && resourceInfoReply->numResources > 0 ) {
        FREEUP (resourceInfoReply->resources);
        resourceInfoReply->numResources = 0;
    }
    return TRUE;
}

static bool_t
xdr_lsResourceInfo (XDR * xdrs, struct lsSharedResourceInfo *lsResourceInfo, struct LSFHeader *hdr)
{
    int status = 0;
    
    if (xdrs->x_op == XDR_DECODE) {
        lsResourceInfo->resourceName = NULL;
        lsResourceInfo->instances    = NULL;
        lsResourceInfo->nInstances   = 0;
    }
 
    if (!(xdr_var_string (xdrs, &lsResourceInfo->resourceName) &&  xdr_int (xdrs, &lsResourceInfo->nInstances))) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && lsResourceInfo->nInstances > 0) {

        assert( lsResourceInfo->nInstances >= 0 );
        lsResourceInfo->instances = malloc( lsResourceInfo->nInstances * sizeof (struct lsSharedResourceInstance));
        if ( NULL == lsResourceInfo->instances && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return FALSE;
        }
    }
    
    for ( int i = 0; i < lsResourceInfo->nInstances; i++) {
        status = xdr_arrayElement (xdrs, (char *) &lsResourceInfo->instances[i], hdr, xdr_lsResourceInstance); // FIXME FIXME FIXME fatal error: incompatible pointer types passing 'struct lsSharedResourceInfo *' to parameter of type 'char *'
        if (!status) {
            lsResourceInfo->nInstances = i;
            return FALSE;
        }
    }
    if (xdrs->x_op == XDR_FREE && lsResourceInfo->nInstances > 0) {
        FREEUP (lsResourceInfo->instances);
        lsResourceInfo->nInstances = 0;
    }

    return TRUE;
}

static bool_t
xdr_lsResourceInstance (XDR * xdrs, struct lsSharedResourceInstance *instance, struct LSFHeader *hdr)
{
    assert( hdr->length );
    
    if (xdrs->x_op == XDR_DECODE) {
        instance->value = NULL;
        instance->hostList = NULL;
        instance->nHosts = 0;
    }
    
    if (!(xdr_var_string (xdrs, &instance->value) && xdr_int (xdrs, &instance->nHosts))) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && instance->nHosts > 0) {
        
        assert( instance->nHosts >= 0 );
        instance->hostList = (char **) malloc ( (u_long)instance->nHosts * sizeof (char *));
        if ( NULL == instance->hostList && ENOMEM == errno ) {
            lserrno = LSE_MALLOC;
            return FALSE;
        }
    }
    
    assert( instance->nHosts >= 0 );
    if (!xdr_array_string (xdrs, instance->hostList, MAXHOSTNAMELEN, instance->nHosts)) {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (instance->hostList);
            instance->nHosts = 0;
        }

        return FALSE;
    }

    if (xdrs->x_op == XDR_FREE && instance->nHosts > 0) {
        FREEUP (instance->hostList);
        instance->nHosts = 0;
    }

    return TRUE;
}

/* xdr_hostEntry()
 */
bool_t
xdr_hostEntry (XDR * xdrs, struct hostEntry *hPtr, struct LSFHeader *hdr)
{
    char *s;
    
    assert( hdr->length );
    
    s = hPtr->hostName;
    if (!xdr_string (xdrs, &s, MAXHOSTNAMELEN)) {
        return FALSE;
    }
    
    s = hPtr->hostModel;
    if (!xdr_string (xdrs, &s, MAXLSFNAMELEN))
        return FALSE;
    
    s = hPtr->hostType;
    if (!xdr_string (xdrs, &s, MAXLSFNAMELEN)) {
        return FALSE;
    }
    
    if( !xdr_int(   xdrs, &hPtr->rcv )       ||
        !xdr_int(   xdrs, &hPtr->nDisks )    ||
        !xdr_float( xdrs, &hPtr->cpuFactor ) )
    {
        return FALSE;
    }
    
    /* this must not be zero... somehow the caller
     * must make sure it is at least 11...
     */
    if (!xdr_int (xdrs, &hPtr->numIndx)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && hPtr->numIndx > 0) {

        assert( hPtr->numIndx >= 0);
        hPtr->busyThreshold = (float *)calloc ((u_long)hPtr->numIndx, sizeof (float));
        if (NULL == hPtr->busyThreshold && ENOMEM == errno ) {
            return FALSE;
        }
    }
    
    for (int cc = 0; cc < hPtr->numIndx; cc++) {
        if (!xdr_float (xdrs, &hPtr->busyThreshold[cc])) {
            return FALSE;
        }
    }
        
    
    if (!xdr_int (xdrs, &hPtr->nRes)) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE && hPtr->nRes > 0) {
        assert( hPtr->nRes >= 0);
        hPtr->resList = (char **)calloc( (u_long)hPtr->nRes, sizeof (char *));
        if( NULL == hPtr->resList && ENOMEM == errno) {
            return FALSE;
        }
    }
    
    for ( int cc = 0; cc < hPtr->nRes; cc++) {
        if (!xdr_var_string (xdrs, &hPtr->resList[cc])) {
            return FALSE;
        }
    }
    
    if (!xdr_int (xdrs, &hPtr->rexPriority) || !xdr_var_string (xdrs, &hPtr->window)) {
        return FALSE;
    }
    
    return TRUE;
}

bool_t
xdr_hostName (XDR *xdrs, char *hostname, struct LSFHeader *hdr)
{
    assert( hdr->length );
    if (!xdr_string (xdrs, &hostname, MAXHOSTNAMELEN)) {
        return FALSE;
    }
    
    return TRUE;
}
