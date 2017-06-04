/* $Id: lsb.reconfig.c 397 2007-11-26 19:04:00Z mblack $
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
#include <pwd.h>

#include "lsb/lsb.h"

int
lsb_reconfig ( unsigned int configFlag)
{
	int cc = 0;
	unsigned int tmp = 0;
	char *reply_buf  = NULL;
	char request_buf[MSGSIZE];
	struct LSFHeader hdr;
	struct lsfAuth auth;
	mbdReqType mbdReqtype;
	XDR xdrs;

	mbdReqtype = BATCH_RECONFIG;

	if (authTicketTokens_ (&auth, NULL) == -1) {
		return -1;
	}

	xdrmem_create (&xdrs, request_buf, MSGSIZE, XDR_ENCODE);

	initLSFHeader_ (&hdr);
	hdr.opCode = mbdReqtype;
	assert( configFlag >= 0 );
	tmp = configFlag;
	hdr.reserved = tmp;

	if (!xdr_encodeMsg (&xdrs, NULL, &hdr, NULL, 0, &auth))
	{
		lsberrno = LSBE_XDR;
		return -1;
	}

	assert( XDR_GETPOS (&xdrs) <= INT_MAX );
	if ((cc = callmbd (NULL, request_buf, (int)XDR_GETPOS (&xdrs), &reply_buf, &hdr, NULL, NULL, NULL)) == -1)
	{
		xdr_destroy (&xdrs);
		return -1;
	}
	xdr_destroy (&xdrs);
	if (cc) {
		free (reply_buf);
	}

	lsberrno = hdr.opCode;
	if (lsberrno == LSBE_NO_ERROR) {
		return 0;
	}
	else {
		return -1;
	}

	return 255;
}
