/* $Id: lim.info.c 397 2007-11-26 19:04:00Z mblack $
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

#include "daemons/liblimd/limd.h"
#include "daemons/liblimd/info.h"
// #include "lib/lproto.h"
#include "lib/xdrlim.h"
#include "lib/xdr.h"




void
pingReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
    char buf[MSGSIZE / 4];
    enum limReplyCode limReplyCode;
    struct LSFHeader replyHdr;
    XDR xdrs2;

    limReplyCode     = LIME_NO_ERR;
    replyHdr.opCode  = (short) limReplyCode;
    replyHdr.refCode = reqHdr->refCode;
    xdrmem_create (&xdrs2, buf, MSGSIZE / 4, XDR_ENCODE);
    if (!xdr_LSFHeader (&xdrs2, &replyHdr) || !xdr_string (&xdrs2, &myHostPtr->hostName, MAXHOSTNAMELEN))
    {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_string");
        xdr_destroy (&xdrs2);
        return;
    }

    if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_", sockAdd2Str_ (from));
        xdr_destroy (&xdrs2);
        return;
    }

    xdr_destroy (&xdrs2);
    return;
}

void
clusInfoReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
        XDR xdrs2;
    char *buf = malloc( sizeof( char ) * MSGSIZE * 2 + 1 ); // FIXME FIXME FIXME FIXME declare to appropr size; free when not used.
    enum limReplyCode limReplyCode;
    struct LSFHeader replyHdr;
    struct clusterInfoReply clusterInfoReply;
    struct clusterInfoReq clusterInfoReq;

    limReplyCode = LIME_NO_ERR;
    clusterInfoReply.clusterMatrix = NULL;

    memset (&clusterInfoReq, 0, sizeof (clusterInfoReq));
    if (!xdr_clusterInfoReq (xdrs, &clusterInfoReq, reqHdr))
    {
        ls_syslog (LOG_WARNING, I18N_FUNC_FAIL, __func__, "xdr_clusterInfoReq");
        limReplyCode = LIME_BAD_DATA;
        goto Reply1;
    }

    if (!masterMe && clusterInfoReq.options == FROM_MASTER)
    {
        wrongMaster (from, buf, reqHdr, -1);
        if (clusterInfoReq.resReq) {
            FREEUP (clusterInfoReq.resReq);
        }
        return;
    }

    clusterInfoReply.shortLsInfo = getCShortInfo (reqHdr);

    clusterInfoReply.nClusters = 1;
    clusterInfoReply.clusterMatrix = malloc (sizeof (struct shortCInfo));
    if (clusterInfoReply.clusterMatrix == NULL)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
        limReplyCode = LIME_NO_MEM;
        goto Reply;
    }

    strcpy (clusterInfoReply.clusterMatrix[0].clName, myClusterPtr->clName);
    if ((myClusterPtr->status & CLUST_INFO_AVAIL) && (masterMe || (!masterMe && myClusterPtr->masterPtr != NULL)))
    {
        clusterInfoReply.clusterMatrix[0].status = CLUST_STAT_OK;
        strcpy (clusterInfoReply.clusterMatrix[0].masterName,  myClusterPtr->masterPtr->hostName);
        strcpy (clusterInfoReply.clusterMatrix[0].managerName, myClusterPtr->managerName);

        clusterInfoReply.clusterMatrix[0].managerId    = myClusterPtr->managerId;
        clusterInfoReply.clusterMatrix[0].numServers   = myClusterPtr->numHosts;
        clusterInfoReply.clusterMatrix[0].numClients   = myClusterPtr->numClients;
        clusterInfoReply.clusterMatrix[0].resClass     = myClusterPtr->resClass;
        clusterInfoReply.clusterMatrix[0].typeClass    = myClusterPtr->typeClass;
        clusterInfoReply.clusterMatrix[0].modelClass   = myClusterPtr->modelClass;
        clusterInfoReply.clusterMatrix[0].numIndx      = myClusterPtr->numIndx;
        clusterInfoReply.clusterMatrix[0].numUsrIndx   = myClusterPtr->numUsrIndx;
        clusterInfoReply.clusterMatrix[0].usrIndxClass = myClusterPtr->usrIndxClass;
        clusterInfoReply.clusterMatrix[0].nAdmins      = myClusterPtr->nAdmins;
        clusterInfoReply.clusterMatrix[0].adminIds     = myClusterPtr->adminIds;
        clusterInfoReply.clusterMatrix[0].admins       = myClusterPtr->admins;
        if (reqHdr->version < 4) {
            clusterInfoReply.clusterMatrix[0].nRes     = 0;
        }
        else {
            clusterInfoReply.clusterMatrix[0].nRes       = allInfo.nRes;
            clusterInfoReply.clusterMatrix[0].resBitMaps = myClusterPtr->resBitMaps;
        }
        clusterInfoReply.clusterMatrix[0].nTypes           = allInfo.nTypes;
        clusterInfoReply.clusterMatrix[0].hostTypeBitMaps  = myClusterPtr->hostTypeBitMaps;
        clusterInfoReply.clusterMatrix[0].nModels          = allInfo.nModels;
        clusterInfoReply.clusterMatrix[0].hostModelBitMaps = myClusterPtr->hostModelBitMaps;

        if (logclass & (LC_TRACE | LC_COMM)) {
            ls_syslog (LOG_DEBUG1, "clusterInfo:clusterInfoReply.clusterMatrix[%d].nRes=%d, name=%s", 0, clusterInfoReply.clusterMatrix[0].nRes, clusterInfoReply.clusterMatrix[0].masterName);
        }
        else {
            clusterInfoReply.clusterMatrix[0].status         = CLUST_STAT_UNAVAIL;
            clusterInfoReply.clusterMatrix[0].masterName[0]  = '\0';
            clusterInfoReply.clusterMatrix[0].managerName[0] = '\0';
            clusterInfoReply.clusterMatrix[0].managerId      = 0;
            clusterInfoReply.clusterMatrix[0].numServers     = 0;
            clusterInfoReply.clusterMatrix[0].numClients     = 0;
            clusterInfoReply.clusterMatrix[0].resClass       = 0;
            clusterInfoReply.clusterMatrix[0].typeClass      = 0;
            clusterInfoReply.clusterMatrix[0].modelClass     = 0;
            clusterInfoReply.clusterMatrix[0].numIndx        = 0;
            clusterInfoReply.clusterMatrix[0].numUsrIndx     = 0;
            clusterInfoReply.clusterMatrix[0].usrIndxClass   = 0;
            clusterInfoReply.clusterMatrix[0].nAdmins        = 0;
            clusterInfoReply.clusterMatrix[0].nRes           = 0;
            clusterInfoReply.clusterMatrix[0].nTypes         = 0;
            clusterInfoReply.clusterMatrix[0].nModels        = 0;
        }
    }

Reply:
    free (clusterInfoReq.resReq);

Reply1:
    initLSFHeader_ (&replyHdr);
    replyHdr.opCode = (short) limReplyCode;
    replyHdr.refCode = reqHdr->refCode;

    xdrmem_create (&xdrs2, buf, MSGSIZE * 2, XDR_ENCODE);
    if (!xdr_LSFHeader (&xdrs2, &replyHdr))
    {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
        if (clusterInfoReply.clusterMatrix != NULL) {
            free (clusterInfoReply.clusterMatrix);
        }
        xdr_destroy (&xdrs2);
        return;
    }

    if (limReplyCode == LIME_NO_ERR)
    {
        if (!xdr_clusterInfoReply (&xdrs2, &clusterInfoReply, &replyHdr))
        {
            ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "xdr_clusterInfoReply");
            xdr_destroy (&xdrs2);
            if (clusterInfoReply.clusterMatrix != NULL) {
                free (clusterInfoReply.clusterMatrix);
            }
            return;
        }
    }

    if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0) {

        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "chanSendDgram_", sockAdd2Str_ (from));
        xdr_destroy (&xdrs2);
        if (clusterInfoReply.clusterMatrix != NULL) {
            free (clusterInfoReply.clusterMatrix);
        }
        return;
    }

    if (clusterInfoReply.clusterMatrix != NULL) {
        free (clusterInfoReply.clusterMatrix);
    }

    free( buf );
    xdr_destroy (&xdrs2);
    return;
}

void
clusNameReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
  XDR xdrs2;
  char buf[MSGSIZE];
  enum limReplyCode limReplyCode;
  char *sp;
  struct LSFHeader replyHdr;

  memset (&buf, 0, sizeof (buf));

  initLSFHeader_ (&replyHdr);

  limReplyCode = LIME_NO_ERR;
  replyHdr.opCode = (short) limReplyCode;
  replyHdr.refCode = reqHdr->refCode;

  /* send back my cluster name
   */
  sp = myClusterPtr->clName;

  xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);

  if (!xdr_LSFHeader (&xdrs2, &replyHdr)
      || !xdr_string (&xdrs2, &sp, MAX_LSF_NAME_LEN))
    {
      ls_syslog (LOG_ERR, "\
%s: failed decoding message from %s", __func__, sockAdd2Str_ (from));
      xdr_destroy (&xdrs2);
      return;
    }

  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
    {
      ls_syslog (LOG_ERR, "\
%s: failed sending message %d bytes to", __func__, strlen (buf), sockAdd2Str_ (from));
      xdr_destroy (&xdrs2);
      return;
    }

  xdr_destroy (&xdrs2);
  return;
}

void
masterInfoReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
  char *buf = malloc( sizeof( char ) * MSGSIZE + 1 );
  XDR xdrs2 = { };
  enum limReplyCode limReplyCode;
  struct hostNode *masterPtr = NULL;
  struct LSFHeader replyHdr  = { };
  struct masterInfo masterInfo = { };

    memset (&buf, 0, sizeof (buf));

    initLSFHeader_ (&replyHdr);
    if (!myClusterPtr->masterKnown && myClusterPtr->prevMasterPtr == NULL) {
        limReplyCode = LIME_MASTER_UNKNW;
    }
    else {
        limReplyCode = LIME_NO_ERR;
    }

  xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);
  replyHdr.opCode = (short) limReplyCode;
  replyHdr.refCode = reqHdr->refCode;

  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
    {
      ls_syslog (LOG_ERR, "%s: failed encode xdr_LSFHeader() to %s", __func__, sockAdd2Str_ (from));
      xdr_destroy (&xdrs2);
      return;
    }

  if (limReplyCode == LIME_NO_ERR)
    {
      masterPtr =
  myClusterPtr->masterKnown ? myClusterPtr->masterPtr : myClusterPtr->
  prevMasterPtr;
      strcpy (masterInfo.hostName, masterPtr->hostName);
      masterInfo.addr = masterPtr->addr[0];
      masterInfo.portno = masterPtr->statInfo.portno;

      if (!xdr_masterInfo (&xdrs2, &masterInfo, &replyHdr))
  {
    ls_syslog (LOG_ERR, "\
%s: failed encode xdr_masterInfo() to %s", __func__, sockAdd2Str_ (from));
    xdr_destroy (&xdrs2);
    return;
  }
    }

  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
    {
      ls_syslog (LOG_ERR, "\
%s: failed chanSendDgram_() %d bytes to %s", __func__, XDR_GETPOS (&xdrs2), sockAdd2Str_ (from));
      xdr_destroy (&xdrs2);
      return;
    }

  xdr_destroy (&xdrs2);
}

void
hostInfoReq (XDR * xdrs, struct hostNode *fromHostP, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint s)
{
    char *buf;
    XDR xdrs2;
    enum limReplyCode limReplyCode;
    struct hostInfoReply hostInfoReply;
    struct decisionReq hostInfoRequest;
    struct resVal resVal;
    int ncandidates = 0 ;
    int cc = 0;
    int propt = 0;
    uint bufSize = 0;
    struct LSFHeader replyHdr;
    char *replyStruct;
    char fromEligible = 'a';
    char clName = 'a';
    struct tclHostData tclHostData;

    if (logclass & (LC_TRACE | LC_HANG | LC_COMM)) {
        ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);
    }

    initResVal (&resVal);
    ignDedicatedResource = TRUE;
    if (!xdr_decisionReq (xdrs, &hostInfoRequest, reqHdr)) {
        limReplyCode = LIME_BAD_DATA;
        goto Reply1;
    }

    if (!(hostInfoRequest.ofWhat == OF_HOSTS && hostInfoRequest.numPrefs == 2 && equalHost_ (hostInfoRequest.preferredHosts[1], myHostPtr->hostName))) {
        if (!masterMe) {
            char tmpBuf[MSGSIZE];
            wrongMaster (from, tmpBuf, reqHdr, s);
            for (uint i = 0; i < hostInfoRequest.numPrefs; i++) {
                free (hostInfoRequest.preferredHosts[i]);
            }
            free (hostInfoRequest.preferredHosts);

            return;
        }
    }

    if (!validHosts (hostInfoRequest.preferredHosts, hostInfoRequest.numPrefs, &clName, hostInfoRequest.options)) {
        limReplyCode = LIME_UNKWN_HOST;
        ls_syslog (LOG_INFO, "%s: validHosts() failed for bad cluster/host name requested for <%s>", __func__, sockAdd2Str_ (from));
        goto Reply;
    }

    propt = PR_SELECT;
    if (hostInfoRequest.options & DFT_FROMTYPE) {
        propt |= PR_DEFFROMTYPE;
    }

    getTclHostData (&tclHostData, myHostPtr, myHostPtr, TRUE);
    tclHostData.ignDedicatedResource = ignDedicatedResource;
    cc = parseResReq (hostInfoRequest.resReq, &resVal, &allInfo, propt);
    if (cc != PARSE_OK || evalResReq (resVal.selectStr, &tclHostData, hostInfoRequest.options & DFT_FROMTYPE) < 0)
    {
        if (cc == PARSE_BAD_VAL) {
            limReplyCode = LIME_UNKWN_RVAL;
        }
        else if (cc == PARSE_BAD_NAME) {
            limReplyCode = LIME_UNKWN_RNAME;
        }
        else {
            limReplyCode = LIME_BAD_RESREQ;
        }
        goto Reply;
    }

    strcpy (hostInfoRequest.hostType, (fromHostP->hTypeNo >= 0) ? shortInfo.hostTypes[fromHostP->hTypeNo] : "unknown");
    fromHostPtr = fromHostP;
    ncandidates = getEligibleSites (&resVal, &hostInfoRequest, 1, &fromEligible);
    if (ncandidates <= 0) {
        limReplyCode = ncandidates ? LIME_NO_MEM : LIME_NO_OKHOST;
        goto Reply;
    }

    hostInfoReply.shortLsInfo = getCShortInfo (reqHdr);
    hostInfoReply.nHost = ncandidates;
    hostInfoReply.nIndex = allInfo.numIndx;
    hostInfoReply.hostMatrix = calloc (ncandidates, sizeof (struct shortHInfo));
    if (hostInfoReply.hostMatrix == NULL) {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
        limReplyCode = LIME_NO_MEM;
        goto Reply;
    }

    for (uint i = 0; i < ncandidates; i++) {
        struct shortHInfo *infoPtr;
        infoPtr = &hostInfoReply.hostMatrix[i];
        if (candidates[i]->infoValid) {

            infoPtr->maxCpus = candidates[i]->statInfo.maxCpus;
            infoPtr->maxMem = candidates[i]->statInfo.maxMem;
            infoPtr->maxSwap = candidates[i]->statInfo.maxSwap;
            infoPtr->maxTmp = candidates[i]->statInfo.maxTmp;
            infoPtr->nDisks = candidates[i]->statInfo.nDisks;
            infoPtr->resBitMaps = candidates[i]->resBitMaps;
            infoPtr->nRInt = GET_INTNUM (allInfo.nRes);
        }
        else {
            infoPtr->maxCpus = 0;
            infoPtr->maxMem = 0;
            infoPtr->maxSwap = 0;
            infoPtr->maxTmp = 0;
            infoPtr->nDisks = 0;
            infoPtr->resBitMaps = candidates[i]->resBitMaps;
            infoPtr->nRInt = GET_INTNUM (allInfo.nRes);
        }
        infoPtr->hTypeIndx = candidates[i]->hTypeNo;
        infoPtr->hModelIndx = candidates[i]->hModelNo;
        infoPtr->resClass = candidates[i]->resClass;
        infoPtr->windows = candidates[i]->windows;
        strcpy (infoPtr->hostName, candidates[i]->hostName);
        infoPtr->busyThreshold = candidates[i]->busyThreshold;

        if (candidates[i]->hostInactivityCount == -1) {
            infoPtr->flags = 0;
        }
        else {
            infoPtr->flags = HINFO_SERVER;
        }

        if (definedSharedResource (candidates[i], &allInfo) == TRUE) {
            infoPtr->flags |= HINFO_SHARED_RESOURCE;
        }
        infoPtr->rexPriority = candidates[i]->rexPriority;
    }
    limReplyCode = LIME_NO_ERR;

Reply:
    for ( uint i = 0; i < hostInfoRequest.numPrefs; i++) {
        free (hostInfoRequest.preferredHosts[i]);
    }
    free (hostInfoRequest.preferredHosts);

Reply1:
    freeResVal (&resVal);
    initLSFHeader_ (&replyHdr);
    replyHdr.opCode = (short) limReplyCode;
    replyHdr.refCode = reqHdr->refCode;

    if (limReplyCode == LIME_NO_ERR) {
        replyStruct = (char *) &hostInfoReply;
        bufSize = ALIGNWORD_ (MSGSIZE + hostInfoReply.nHost * (128 + hostInfoReply.nIndex * 4));
        bufSize = MAX (bufSize, 4 * MSGSIZE);
    }
    else {
      replyStruct = NULL;
      bufSize = 512;
    }

    buf = malloc (bufSize);
    if (!buf)
    {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
        if (limReplyCode == LIME_NO_ERR) {
            free (hostInfoReply.hostMatrix);
        }
        return;
    }

    xdrmem_create (&xdrs2, buf, bufSize, XDR_ENCODE);

    if (!xdr_encodeMsg (&xdrs2, replyStruct, &replyHdr, xdr_hostInfoReply, 0, NULL))
    {
        ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_encodeMsg");
        xdr_destroy (&xdrs2);
        if (limReplyCode == LIME_NO_ERR)
            free (hostInfoReply.hostMatrix);
        return;
    }

    if (limReplyCode == LIME_NO_ERR) {
        free (hostInfoReply.hostMatrix);
    }

    if (s < 0) {
        cc = chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from);
    }
    else {
        cc = chanWrite_ (s, buf, XDR_GETPOS (&xdrs2));
    }

    free (buf);

    if (cc < 0)
    {
        ls_syslog (LOG_ERR, "%s: Failed in sending lshosts reply to %s len %d: %m", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
        xdr_destroy (&xdrs2);
        return;
    }

    xdr_destroy (&xdrs2);
    return;

}

void
infoReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint s)
{
    char *buf = NULL;
    XDR xdrs2 = { };
    enum limReplyCode limReplyCode;
    struct LSFHeader replyHdr = { };
    size_t len = 0;
    int cc = 0;

  if (buf == NULL)
    {
      len = sizeof (struct lsInfo) + allInfo.nRes * sizeof (struct resItem) + 10000;
      if (!(buf = (char *) malloc (len)))
  {
    ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
    return;
  }
    }
  if (logclass & (LC_TRACE | LC_HANG | LC_COMM))
    ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);

  limReplyCode = LIME_NO_ERR;

  xdrmem_create (&xdrs2, buf, len, XDR_ENCODE);
  initLSFHeader_ (&replyHdr);
  replyHdr.opCode = (short) limReplyCode;
  replyHdr.refCode = reqHdr->refCode;

  if (!xdr_encodeMsg (&xdrs2, (char *) &allInfo, &replyHdr, xdr_lsInfo, 0, NULL))
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_encodeMsg");
      xdr_destroy (&xdrs2);
      return;
    }

  if (s < 0)
    cc = chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from);
  else
    cc = chanWrite_ (s, buf, XDR_GETPOS (&xdrs2));


  if (cc < 0)
    {
        /* catgets 7401 */
        ls_syslog (LOG_ERR, "7401: %s: Failed in sending lsinfo back to %s (len=%d) %m", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
      xdr_destroy (&xdrs2);
      return;
    }

  xdr_destroy (&xdrs2);
  return;

}

void
cpufReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
    char buf[MSGSIZE];
  XDR xdrs2;
  enum limReplyCode limReplyCode;
  char hostModel[MAX_LSF_NAME_LEN];
  char *sp = hostModel;
  hEnt *hashEntPtr = NULL;
  struct LSFHeader replyHdr;

  if (!xdr_string (xdrs, &sp, MAX_LSF_NAME_LEN))
    {
      limReplyCode = LIME_BAD_DATA;
      goto Reply;
    }


  if (!masterMe)
    {
      wrongMaster (from, buf, reqHdr, -1);
      return;
    }

  hashEntPtr = h_getEnt_ (&hostModelTbl, hostModel);
  if (!hashEntPtr)
    {
      limReplyCode = LIME_UNKWN_MODEL;
      goto Reply;
    }
  limReplyCode = LIME_NO_ERR;

Reply:
  replyHdr.opCode = (short) limReplyCode;
  replyHdr.refCode = reqHdr->refCode;
  xdrmem_create (&xdrs2, buf, MSGSIZE, XDR_ENCODE);
  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
      xdr_destroy (&xdrs2);
      return;
    }

  if (limReplyCode == LIME_NO_ERR)
    {
      if (!xdr_float (&xdrs2, (float *) hashEntPtr->hData))
  {
    ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_float");
    xdr_destroy (&xdrs2);
    return;
  }
    }

  if (chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from) < 0)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "chanSendDgram_",
     sockAdd2Str_ (from));
      xdr_destroy (&xdrs2);
      return;
    }

  xdr_destroy (&xdrs2);
  return;

}

int
validHosts (char **hostList, int num, char *clName, int options)
{
  struct clusterNode *clPtr;
  int cc;

  myClusterPtr->status |= CLUST_ELIGIBLE;
  *clName = FALSE;
  clPtr = myClusterPtr;

  for (cc = 0; cc < num; cc++)
    {

      if (findHostbyList (clPtr->hostList, hostList[cc]) == NULL)
  {
    ls_syslog (LOG_WARNING, "%s: Unknown host %s in request", __func__, hostList[cc]);
    return FALSE;
  }
    }

  return TRUE;
}

struct shortLsInfo *
getCShortInfo (struct LSFHeader *reqHdr)
{
  int i;

  if (reqHdr->version >= 6)
    {
      return (&shortInfo);
    }

  oldShortInfo.nRes = shortInfo.nRes;
  oldShortInfo.resName = shortInfo.resName;
  if (shortInfo.nTypes > MAX_TYPES_31)
    {
      oldShortInfo.nTypes = MAX_TYPES_31;
    }
  else
    {
      oldShortInfo.nTypes = shortInfo.nTypes;
    }
  for (i = 0; i < oldShortInfo.nTypes; i++)
    {
      oldShortInfo.hostTypes[i] = shortInfo.hostTypes[i];
    }
  if (shortInfo.nModels > MAX_MODELS_31)
    {
      oldShortInfo.nModels = MAX_MODELS_31;
    }
  else
    {
      oldShortInfo.nModels = shortInfo.nModels;
    }
  for (i = 0; i < oldShortInfo.nModels; i++)
    {
      oldShortInfo.hostModels[i] = shortInfo.hostModels[i];
      oldShortInfo.cpuFactors[i] = shortInfo.cpuFactors[i];
    }

  if (reqHdr->version < 4)
    {
      if (shortInfo.nRes > MAXSRES)
  {
    oldShortInfo.nRes = MAXSRES;
  }
      else
  {
    oldShortInfo.nRes = shortInfo.nRes;
  }
      oldShortInfo.resName = shortInfo.resName;
    }

  return (&oldShortInfo);
}

void
resourceInfoReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint s)
{
    char buf1[MSGSIZE];
  char *buf;
  XDR xdrs2;
  char *replyStruct;
  enum limReplyCode limReplyCode;
  struct LSFHeader replyHdr;
  struct resourceInfoReq resourceInfoReq;
  struct resourceInfoReply resourceInfoReply;
  int cc = 0;

  if (logclass & (LC_TRACE | LC_HANG | LC_COMM))
    ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);

  limReplyCode = LIME_NO_ERR;

  if (!masterMe)
    {
      wrongMaster (from, buf1, reqHdr, s);
      return;
    }

  if (!xdr_resourceInfoReq (xdrs, &resourceInfoReq, reqHdr))
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_resourceInfoReq");
      limReplyCode = LIME_BAD_DATA;
      cc = MSGSIZE;
    }
  else
    {
      limReplyCode =  checkResources (&resourceInfoReq, &resourceInfoReply, &cc);
      if (limReplyCode != LIME_NO_ERR) {
            cc = MSGSIZE;
      }
      else {
            cc += 4 * MSGSIZE;
      }
    }

  xdr_lsffree (xdr_resourceInfoReq, (char *) &resourceInfoReq, reqHdr);


  if ((buf = (char *) malloc (cc)) == NULL)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
      freeResourceInfoReply (&resourceInfoReply);
      return;
    }

    xdrmem_create (&xdrs2, buf, cc, XDR_ENCODE);
    initLSFHeader_ (&replyHdr);
    replyHdr.opCode = (short) limReplyCode;
    replyHdr.refCode = reqHdr->refCode;
    if (limReplyCode == LIME_NO_ERR) {
        replyStruct = (char *) &resourceInfoReply;
    }
    else {
        replyStruct = (char *) NULL;
    }

  if (!xdr_encodeMsg(&xdrs2, replyStruct, &replyHdr, xdr_resourceInfoReply, 0, NULL))
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_encodeMsg");
      FREEUP (buf);
      xdr_destroy (&xdrs2);
      freeResourceInfoReply (&resourceInfoReply);
      return;
    }

    if (s < 0) {
        cc = chanSendDgram_ (limSock, buf, XDR_GETPOS (&xdrs2), from);
    }
    else {
        cc = chanWrite_ (s, buf, XDR_GETPOS (&xdrs2));
    }

  if (cc < 0)
    {
      /* catgets 7402 */
      ls_syslog (LOG_ERR, "7402: %s: Failed in sending lsresources reply to %s (len=%d): %m", __func__, sockAdd2Str_ (from), XDR_GETPOS (&xdrs2));
      FREEUP (buf);
      xdr_destroy (&xdrs2);
      freeResourceInfoReply (&resourceInfoReply);
      return;
    }

  FREEUP (buf);
  xdr_destroy (&xdrs2);
  freeResourceInfoReply (&resourceInfoReply);
  return;

}

int
checkResources (struct resourceInfoReq *resourceInfoReq,
    struct resourceInfoReply *reply, int *len)
{
    int i, j, allResources = FALSE, found = FALSE;
  enum limReplyCode limReplyCode;
  char *host;

  if (resourceInfoReq->numResourceNames == 1
      && !strcmp (resourceInfoReq->resourceNames[0], ""))
    {
      allResources = TRUE;
    }
  *len = 0;
  reply->numResources = 0;

  if (numHostResources == 0)
    return LIME_NO_RESOURCE;

  if (resourceInfoReq->hostName == NULL
      || (resourceInfoReq->hostName
    && !strcmp (resourceInfoReq->hostName, " ")))
    host = NULL;
  else
    {

      if (findHostbyList (myClusterPtr->hostList,
        resourceInfoReq->hostName) == NULL)
  {
     /* catgets 7403 */
    ls_syslog (LOG_ERR, "7403: %s: Host <%s>  is not used by cluster <%s>", __func__, resourceInfoReq->hostName, myClusterName);
    return LIME_UNKWN_HOST;
  }
      host = resourceInfoReq->hostName;
    }

  reply->numResources = 0;
  reply->resources = malloc( sizeof ( struct lsSharedResourceInfo) * numHostResources  );
  if ( NULL == reply->resources )
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
      return (LIME_NO_MEM);
    }
        *len = numHostResources * sizeof (struct lsSharedResourceInfo) + sizeof (struct resourceInfoReply);

  for (i = 0; i < resourceInfoReq->numResourceNames; i++)
    {
      found = FALSE;
      for (j = 0; j < numHostResources; j++)
  {
    if (allResources == FALSE
        && (strcmp (resourceInfoReq->resourceNames[i],
        hostResources[j]->resourceName)))
      continue;
    found = TRUE;
    if ((limReplyCode =
         copyResource (reply, hostResources[j], len,
           host)) != LIME_NO_ERR)
      {
        return limReplyCode;
      }
    reply->numResources++;
    if (allResources == FALSE)
      break;
  }
      if (allResources == FALSE && found == FALSE)
  {

    return LIME_UNKWN_RNAME;
  }
      found = FALSE;
      if (allResources == TRUE)
  break;
    }
  return LIME_NO_ERR;

}

int
copyResource (struct resourceInfoReply *reply,
        struct sharedResource *resource, int *len, char *hostName)
{
    int i, j, num, cc = 0, found = FALSE, numInstances;

  num = reply->numResources;
  reply->resources[num].resourceName = resource->resourceName;
  cc += strlen (resource->resourceName) + 1;


  if ((reply->resources[num].instances = (struct lsSharedResourceInstance *)
       malloc (resource->numInstances
         * sizeof (struct lsSharedResourceInstance))) == NULL)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
      return (LIME_NO_MEM);
    }
  cc += resource->numInstances * sizeof (struct lsSharedResourceInstance);
  reply->resources[num].nInstances = 0;
  numInstances = 0;
  reply->resources[num].instances[numInstances].nHosts = 0;

  for (i = 0; i < resource->numInstances; i++)
    {
      if (hostName)
  {
    for (j = 0; j < resource->instances[i]->nHosts; j++)
      {
        if (equalHost_
      (hostName, resource->instances[i]->hosts[j]->hostName))
    {
      found = TRUE;
      break;
    }
        else
    continue;
      }
  }
      if (hostName && found == FALSE)
  continue;

      found = FALSE;
      reply->resources[num].instances[numInstances].value =
  resource->instances[i]->value;
      reply->resources[num].instances[numInstances].nHosts = 0;
      if ((reply->resources[num].instances[numInstances].hostList =
     (char **) malloc (resource->instances[i]->nHosts
           * sizeof (char *))) == NULL)
  {
    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
    return (LIME_NO_MEM);
  }
      cc += strlen (resource->instances[i]->value) + 1;
      for (j = 0; j < resource->instances[i]->nHosts; j++)
  {
    reply->resources[num].instances[numInstances].hostList[j] =
      resource->instances[i]->hosts[j]->hostName;
    cc += MAXHOSTNAMELEN;
  }
      reply->resources[num].instances[numInstances].nHosts =
  resource->instances[i]->nHosts;
      numInstances++;
    }
  reply->resources[num].nInstances = numInstances;
  *len += cc;
  return LIME_NO_ERR;

}

void
freeResourceInfoReply (struct resourceInfoReply *reply)
{
  int i, j;

  if (reply == NULL || reply->numResources <= 0 || reply->resources == NULL)
    return;
  for (i = 0; i < reply->numResources; i++)
    {
      for (j = 0; j < reply->resources[i].nInstances; j++)
  FREEUP (reply->resources[i].instances[j].hostList);
      FREEUP (reply->resources[i].instances);
    }
  FREEUP (reply->resources);
}
