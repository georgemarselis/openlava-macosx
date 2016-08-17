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

#include "lib/hdr.h"
#include "lib/lproto.h"
#include "daemons/daemonout.h"
#include "lsb/lsbatch.h"
#include "lib/common_structs.h"


bool_t xdr_time_t        ( XDR *,      time_t * );
bool_t xdr_lsfRusage     ( XDR *,      struct lsfRusage * );
bool_t xdr_lvector       ( XDR *,      float *, int );
bool_t xdr_array_string  ( XDR * xdrs, char **astring, uint maxlen, uint arraysize );
bool_t xdr_var_string    ( XDR *,      char ** );
bool_t xdr_stringLen     ( XDR *,      struct stringLen *, struct LSFHeader * );
bool_t xdr_lenData       ( XDR *,      struct lenData * );
bool_t xdr_lsfLimit      ( XDR *,      struct lsfLimit *, struct LSFHeader * );
bool_t xdr_portno        ( XDR *,      u_short * );
bool_t xdr_address       ( XDR *,      u_int * );
int getXdrStrlen         ( char * );

bool_t xdr_modifyReq     (XDR *, struct modifyReq *,     struct LSFHeader *);
bool_t xdr_submitReq     (XDR *, struct submitReq *,     struct LSFHeader *);
bool_t xdr_submitMbdReply(XDR *, struct submitMbdReply *,struct LSFHeader *);
bool_t xdr_signalReq     (XDR *, struct signalReq *,     struct LSFHeader *);
bool_t xdr_lsbMsg        (XDR *, struct lsbMsg *,        struct LSFHeader *);
bool_t xdr_controlReq    (XDR *, struct controlReq *,    struct LSFHeader *);
// bool_t xdr_debugReq      (XDR *, struct debugReq *,      struct LSFHeader *); // FIXME FIXME commented out due to debugger complaining. no apparent brokerage
bool_t xdr_infoReq       (XDR *, struct infoReq *,       struct LSFHeader *);
bool_t xdr_parameterInfo (XDR *, struct parameterInfo *, struct LSFHeader *);
bool_t xdr_userInfoEnt   (XDR *, struct userInfoEnt *,   struct LSFHeader *);
bool_t xdr_userInfoReply (XDR *, struct userInfoReply *, struct LSFHeader *);
bool_t xdr_hostInfoEnt   (XDR *, struct hostInfoEnt *,   struct LSFHeader *, uint *);
bool_t xdr_hostDataReply (XDR *, struct hostDataReply *, struct LSFHeader *);
bool_t xdr_queueInfoEnt  (XDR *xdrs, struct queueInfoEnt * qInfo, struct LSFHeader *hdr, uint *nIdx);
bool_t xdr_queueInfoReply(XDR *, struct queueInfoReply *,struct LSFHeader *);
bool_t xdr_jobInfoHead   (XDR *, struct jobInfoHead *,   struct LSFHeader *);
bool_t xdr_jobInfoReply  (XDR *, struct jobInfoReply *,  struct LSFHeader *);
bool_t xdr_jobInfoEnt    (XDR *, struct jobInfoEnt *,    struct LSFHeader *);
bool_t xdr_jobInfoReq    (XDR *, struct jobInfoReq *,    struct LSFHeader *);
bool_t xdr_jobPeekReq    (XDR *, struct jobPeekReq *,    struct LSFHeader *);
bool_t xdr_jobPeekReply  (XDR *, struct jobPeekReply *,  struct LSFHeader *);
bool_t xdr_jobMoveReq    (XDR *, struct jobMoveReq *,    struct LSFHeader *);
bool_t xdr_jobSwitchReq  (XDR *, struct jobSwitchReq *,  struct LSFHeader *);
bool_t xdr_groupInfoReply(XDR *, struct groupInfoReply *,struct LSFHeader *);
bool_t xdr_groupInfoEnt  (XDR *, struct groupInfoEnt *,  struct LSFHeader *);
bool_t xdr_migReq        (XDR *, struct migReq *,        struct LSFHeader *);
bool_t xdr_time_t        (XDR *, time_t *);
bool_t xdr_xFile         (XDR *, struct xFile *,         struct LSFHeader *);
bool_t xdr_var_string    (XDR *, char **);
// bool_t xdr_lsbShareResourceInfoReply (XDR *, struct lsbShareResourceInfoReply *, struct LSFHeader *hdr); // FIXME FIXME FIXME FIXME taken out because compiler was complaining about struct lsbShareResourceInfoReply * not being visible outside function. re-investigate, delete if not needed
bool_t xdr_runJobReq     (XDR *,             struct runJobRequest *,             struct LSFHeader *);
bool_t xdr_jobAttrReq    (XDR *,             struct jobAttrInfoEnt *,            struct LSFHeader *);

bool_t xdr_xFile (XDR *xdrs, struct xFile * xf, struct LSFHeader *hdr);

bool_t xdr_var_string (XDR *, char **);

static int lsbSharedResConfigured_ = FALSE;

bool_t xdr_submitReq (XDR *xdrs, struct submitReq *submitReq, struct LSFHeader *hdr);
bool_t xdr_jgrpInfoReply (XDR *xdrs, struct jobInfoReply * jobInfoReply, struct LSFHeader *hdr);

bool_t xdr_array_string (XDR * xdrs, char **astring, uint maxlen, uint arraysize);
