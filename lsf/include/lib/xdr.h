/* $Id: lib.xdr.h 397 2007-11-26 19:04:00Z mblack $
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

#include <rpc/types.h>
#ifndef __XDR_HEADER__
#include <rpc/xdr.h>
#endif
#include <stdio.h>

// #include "daemons/daemonout.h"
#include "lib/common_structs.h"
#include "lib/hdr.h"
// #include "lsb/lsbatch.h"

bool_t xdr_array_string              ( XDR *xdrs, char   **astring,                                 unsigned int maxlen,      unsigned int arraysize );
bool_t xdr_controlReq                ( XDR *xdrs, struct   controlReq                *,             struct LSFHeader *hdr     );
bool_t xdr_debugReq                  ( XDR *xdrs, struct   debugReq                  *,             struct LSFHeader *hdr     );
bool_t xdr_groupInfoEnt              ( XDR *xdrs, struct   groupInfoEnt              *,             struct LSFHeader *hdr     );
bool_t xdr_groupInfoReply            ( XDR *xdrs, struct   groupInfoReply            *,             struct LSFHeader *hdr     );
bool_t xdr_hostDataReply             ( XDR *xdrs, struct   hostDataReply             *,             struct LSFHeader *hdr     );
bool_t xdr_hostInfoEnt               ( XDR *xdrs, struct   hostInfoEnt               *,             struct LSFHeader *,       unsigned int * );
bool_t xdr_infoReq                   ( XDR *xdrs, struct   infoReq                   *,             struct LSFHeader *hdr     );
bool_t xdr_jgrpInfoReply             ( XDR *xdrs, struct   jobInfoReply              *jobInfoReply, struct LSFHeader *hdr);
bool_t xdr_jobAttrReq                ( XDR *xdrs, struct   jobAttrInfoEnt            *,             struct LSFHeader *hdr     );
bool_t xdr_jobInfoEnt                ( XDR *xdrs, struct   jobInfoEnt                *,             struct LSFHeader *hdr     );
bool_t xdr_jobInfoHead               ( XDR *xdrs, struct   jobInfoHead               *,             struct LSFHeader *hdr     );
bool_t xdr_jobInfoReply              ( XDR *xdrs, struct   jobInfoReply              *,             struct LSFHeader *hdr     );
bool_t xdr_jobInfoReq                ( XDR *xdrs, struct   jobInfoReq                *,             struct LSFHeader *hdr     );
bool_t xdr_jobMoveReq                ( XDR *xdrs, struct   jobMoveReq                *,             struct LSFHeader *hdr     );
bool_t xdr_jobPeekReply              ( XDR *xdrs, struct   jobPeekReply              *,             struct LSFHeader *hdr     );
bool_t xdr_jobPeekReq                ( XDR *xdrs, struct   jobPeekReq                *,             struct LSFHeader *hdr     );
bool_t xdr_jobSwitchReq              ( XDR *xdrs, struct   jobSwitchReq              *,             struct LSFHeader *hdr     );
bool_t xdr_lsbMsg                    ( XDR *xdrs, struct   lsbMsg                    *,             struct LSFHeader *hdr     );
bool_t xdr_lsbShareResourceInfoReply ( XDR *xdrs, struct   lsbShareResourceInfoReply *,             struct LSFHeader *hdr     );
bool_t xdr_lsfLimit                  ( XDR *xdrs, struct   lsfLimit                  *,             struct LSFHeader *hdr     );
bool_t xdr_migReq                    ( XDR *xdrs, struct   migReq                    *,             struct LSFHeader *hdr     );
bool_t xdr_modifyReq                 ( XDR *xdrs, struct   modifyReq                 *,             struct LSFHeader *hdr     );
bool_t xdr_parameterInfo             ( XDR *xdrs, struct   parameterInfo             *,             struct LSFHeader *hdr     );
bool_t xdr_queueInfoEnt              ( XDR *xdrs, struct   queueInfoEnt              *qInfo,        struct LSFHeader *hdr,    unsigned int *nIdx );
bool_t xdr_queueInfoReply            ( XDR *xdrs, struct   queueInfoReply            *,             struct LSFHeader *hdr     );
bool_t xdr_runJobReq                 ( XDR *xdrs, struct   runJobRequest             *,             struct LSFHeader *hdr     );
bool_t xdr_signalReq                 ( XDR *xdrs, struct   signalReq                 *,             struct LSFHeader *hdr     );
bool_t xdr_submitMbdReply            ( XDR *xdrs, struct   submitMbdReply            *,             struct LSFHeader *hdr     );
bool_t xdr_submitReq                 ( XDR *xdrs, struct   submitReq                 *submitReq,    struct LSFHeader *hdr     );
bool_t xdr_userInfoEnt               ( XDR *xdrs, struct   userInfoEnt               *,             struct LSFHeader *hdr     );
bool_t xdr_userInfoReply             ( XDR *xdrs, struct   userInfoReply             *,             struct LSFHeader *hdr     );
bool_t xdr_xFile                     ( XDR *xdrs, struct   xFile                     *xf,           struct LSFHeader *hdr     );
bool_t xdr_lenData                   ( XDR *xdrs, struct   lenData                   *              );
bool_t xdr_lsfRusage                 ( XDR *xdrs, struct   lsfRusage                 *              );
bool_t xdr_lvector                   ( XDR *xdrs, float   *, int                                      );
bool_t xdr_portno                    ( XDR *xdrs, unsigned short  *                                   );
bool_t xdr_time_t                    ( XDR *xdrs, time_t          *                                   );
bool_t xdr_var_string                ( XDR *xdrs, char           **                                   );
bool_t xdr_address                   ( XDR *xdrs, unsigned int    *                                   );

// bool_t xdr_stringLen     ( XDR *xdrs, struct stringLen *, struct LSFHeader *hdr ));

int lsbSharedResConfigured_ = FALSE;

int getXdrStrlen         ( char * );

void encodeHdr ( pid_t *word1, size_t *word2, unsigned int *word3, unsigned int *word4, struct LSFHeader *header);
// bool_t xdr_LSFHeader (XDR * xdrs, struct LSFHeader *header);
// bool_t xdr_packLSFHeader (char *buf, struct LSFHeader *hdr )header);
// bool_t xdr_encodeMsg (XDR * xdrs, char *data, struct LSFHeader *hdr, bool_t (*xdr_func) (), int options, struct lsfAuth *auth);
// bool_t xdr_arrayElement (XDR * xdrs, char *data, struct LSFHeader *hdr )hdr, bool_t (*xdr_func) (), ...);
int readDecodeHdr_ (int s, char *buf, long (*readFunc) (), XDR * xdrs, struct LSFHeader *hdr);
int readDecodeMsg_ (int s, char *buf, struct LSFHeader *hdr, long (*readFunc) (),  XDR * xdrs,  char *data, bool_t (*xdrFunc) (), struct lsfAuth *auth);
int writeEncodeMsg_ (int s, char *buf, unsigned int len, struct LSFHeader *hdr, char *data, long (*writeFunc) (), bool_t (*xdrFunc) (), int options);
// int getXdrStrlen (char *s);
// void xdr_lsffree (bool_t (*xdr_func) (), char *objp, struct LSFHeader *hdr);
