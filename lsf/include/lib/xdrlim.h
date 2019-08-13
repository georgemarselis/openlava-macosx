/* $Id: lib.xdrlim.h 397 2007-11-26 19:04:00Z mblack $
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
 
#include "lib/header.h"
// #include "lib/lproto.h"
#include "lib/xdrlim.h"
#include "daemons/liblimd/limout.h"


/* xdrlim.c */ // FIXME FIXME FIXME FIXME use column aligner from sublime text to align these function definition
bool_t xdr_placeInfo(XDR *xdrs, struct placeInfo *placeInfo, struct LSFHeader *hdr);
bool_t xdr_hostLoad(XDR *xdrs, struct hostLoad *loadVec, struct LSFHeader *hdr, const char *cp);
bool_t xdr_decisionReq(XDR *xdrs, struct decisionReq *decisionReqPtr, struct LSFHeader *hdr);
bool_t xdr_placeReply(XDR *xdrs, struct placeReply *placeRepPtr, struct LSFHeader *hdr);
bool_t xdr_loadReply(XDR *xdrs, struct loadReply *loadReplyPtr, struct LSFHeader *hdr);
bool_t xdr_jobXfer(XDR *xdrs, struct jobXfer *jobXferPtr, struct LSFHeader *hdr);
bool_t xdr_hostInfoReply(XDR *xdrs, struct hostInfoReply *hostInfoReply, struct LSFHeader *hdr);
bool_t xdr_shortHInfo(XDR *xdrs, struct shortHInfo *shortHInfo, struct LSFHeader *hdr, char *nIndex);
bool_t xdr_shortLsInfo(XDR *xdrs, struct shortLsInfo *shortLInfo, struct LSFHeader *hdr);
bool_t xdr_limLock(XDR *xdrs, struct limLock *limLockPtr, struct LSFHeader *hdr);
bool_t xdr_resItem(XDR *xdrs, struct resItem *resItem, struct LSFHeader *hdr);
bool_t xdr_lsInfo(XDR *xdrs, struct lsInfo *lsInfoPtr, struct LSFHeader *hdr);
bool_t xdr_masterInfo(XDR *xdrs, struct masterInfo *mInfoPtr, struct LSFHeader *hdr);
bool_t xdr_clusterInfoReq(XDR *xdrs, struct clusterInfoReq *clusterInfoReq, struct LSFHeader *hdr);
bool_t xdr_clusterInfoReply(XDR *xdrs, struct clusterInfoReply *clusterInfoReply, struct LSFHeader *hdr);
void freeUpMemp(void *memp, unsigned int nClus);
bool_t xdr_shortCInfo(XDR *xdrs, struct shortCInfo *clustInfoPtr, struct LSFHeader *hdr);
bool_t xdr_cInfo(XDR *xdrs, struct cInfo *cInfo, struct LSFHeader *hdr);
bool_t xdr_resourceInfoReq(XDR *xdrs, struct resourceInfoReq *resourceInfoReq, struct LSFHeader *hdr);
bool_t xdr_resourceInfoReply(XDR *xdrs, struct resourceInfoReply *resourceInfoReply, struct LSFHeader *hdr);
bool_t xdr_lsResourceInfo(XDR *xdrs, struct lsSharedResourceInfo *lsResourceInfo, struct LSFHeader *hdr);
bool_t xdr_lsResourceInstance(XDR *xdrs, struct lsSharedResourceInstance *instance, struct LSFHeader *hdr);
bool_t xdr_hostEntry(XDR *xdrs, struct hostEntry *hPtr, struct LSFHeader *hdr);
bool_t xdr_hostName(XDR *xdrs, char *hostname, struct LSFHeader *hdr);
