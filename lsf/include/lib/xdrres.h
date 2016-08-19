/* $Id: lib.xdrres.h 397 2007-11-26 19:04:00Z mblack $
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

#ifndef __XDR_HEADER__
#include <rpc/xdr.h>
#endif

#include "daemons/libresd/resd.h"
#include "lib/lproto.h"
#include "lib/hdr.h"
#include "lib/rf.h"

bool_t xdr_resConnect (XDR *, struct resConnect *, struct LSFHeader *);
int xdr_resCmdBill (XDR *, struct resCmdBill *, struct LSFHeader *);
int xdr_resSetenv (XDR *, struct resSetenv *, struct LSFHeader *);
int xdr_resRKill (XDR *, struct resRKill *, struct LSFHeader *);
int xdr_resGetpid (XDR *, struct resPid *, struct LSFHeader *);
bool_t xdr_resGetRusage (XDR *, struct resRusage *, struct LSFHeader *);
int xdr_resChdir (XDR *, struct resChdir *, struct LSFHeader *);
int xdr_resControl (XDR *, struct resControl *, struct LSFHeader *);

int xdr_resStty (XDR *, struct resStty *, struct LSFHeader *);
int xdr_niosConnect (XDR *, struct niosConnect *, struct LSFHeader *);
int xdr_niosStatus (XDR *, struct niosStatus *, struct LSFHeader *);
int xdr_resSignal (XDR *, struct resSignal *, struct LSFHeader *);

bool_t xdr_ropenReq (XDR *, struct ropenReq *, struct LSFHeader *);
bool_t xdr_rrdwrReq (XDR *, struct rrdwrReq *, struct LSFHeader *);
bool_t xdr_rlseekReq (XDR *, struct rlseekReq *, struct LSFHeader *);
bool_t xdr_noxdr (XDR *xdrs, size_t size, struct LSFHeader *hdr);
