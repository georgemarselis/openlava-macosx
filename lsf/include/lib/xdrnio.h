/* $Id: lib.xdrnio.c 397 2007-11-26 19:04:00Z mblack $
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

bool_t xdr_resConnect(  XDR *xdrs, struct resConnect  *connectPtr, struct LSFHeader *hdr );
bool_t xdr_niosConnect( XDR *xdrs, struct niosConnect *conn,       struct LSFHeader *hdr );
bool_t xdr_resCmdBill(  XDR *xdrs, struct resCmdBill  *cmd,        struct LSFHeader *hdr );
bool_t xdr_resSetenv(   XDR *xdrs, struct resSetenv   *envp,       struct LSFHeader *hdr );
bool_t xdr_resChdir(    XDR *xdrs, struct resChdir    *ch,         struct LSFHeader *hdr );
bool_t xdr_resStty(     XDR *xdrs, struct resStty     *tty,        struct LSFHeader *hdr );
bool_t xdr_resGetRusage(XDR *xdrs, struct resRusage   *rusageReq,  struct LSFHeader *hdr );
bool_t xdr_resRKill(    XDR *xdrs, struct resRKill    *rkill,      struct LSFHeader *hdr );
bool_t xdr_resControl(  XDR *xdrs, struct resControl  *ctrl,       struct LSFHeader *hdr );

