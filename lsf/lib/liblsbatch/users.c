/* $Id: lsb.users.c 397 2007-11-26 19:04:00Z mblack $
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
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

#include "lsb/lsb.h"
#include "lsb/xdr.h"


struct userInfoEnt *
lsb_userinfo (char **users, uint *numUsers)
{
  mbdReqType mbdReqtype;
  XDR xdrs;
  struct LSFHeader hdr;
  char *reply_buf;
  char *request_buf;
  struct userInfoReply userInfoReply, *reply;
  static struct infoReq userInfoReq;
  uint i = 0;
  unsigned long cc = 0;
  unsigned long numReq = 0;
  char lsfUserName[MAXLINELEN];
  int returnValue = 0;

  if (numUsers)
    {
      numReq = *numUsers;
      *numUsers = 0;
    }
/*  if (numReq < 0)
    {
      lsberrno = LSBE_BAD_ARG;
      return (NULL);
    }*/

  if (userInfoReq.names)
    free (userInfoReq.names);

  if (numUsers == NULL || numReq == 0)
    {
      userInfoReq.numNames = 0;
      if ((userInfoReq.names = (char **) malloc (sizeof (char *))) == NULL)
  {
    lsberrno = LSBE_NO_MEM;
    return (NULL);
  }
      userInfoReq.names[0] = "";
      cc = 1;
    }
  else if (numReq == 1 && users == NULL)
    {
      if (getLSFUser_ (lsfUserName, MAXLINELEN) != 0)
  {
    return (NULL);
  }
      userInfoReq.numNames = 1;
      if ((userInfoReq.names = (char **) malloc (sizeof (char *))) == NULL)
  {
    lsberrno = LSBE_NO_MEM;
    return (NULL);
  }
      userInfoReq.names[0] = lsfUserName;
      cc = 1;
    }
  else
    {
      userInfoReq.names = (char **) calloc(numReq, sizeof (char *));
      if ( NULL == userInfoReq.names && ENOMEM == errno )
  {
    lsberrno = LSBE_NO_MEM;
    return (NULL);
  }
      assert( numReq <= UINT_MAX );
      userInfoReq.numNames = (uint)numReq;
      for (i = 0; i < numReq; i++)
  {
    if (users[i] && strlen (users[i]) + 1 < MAXHOSTNAMELEN)
      userInfoReq.names[i] = users[i];
    else
      {
        free (userInfoReq.names);
        lsberrno = LSBE_BAD_USER;
        *numUsers = i;
        return (NULL);
      }
  }
      cc = numReq;
    }
  userInfoReq.resReq = "";


  mbdReqtype = BATCH_USER_INFO;
  cc = sizeof (struct infoReq) + cc * MAXHOSTNAMELEN + cc + 100;
  if ((request_buf = malloc (cc)) == NULL)
    {
      lsberrno = LSBE_NO_MEM;
      return (NULL);
    }
  assert( cc <= INT_MAX );
  xdrmem_create (&xdrs, request_buf, (uint)cc, XDR_ENCODE);

  initLSFHeader_ (&hdr);
  hdr.opCode = mbdReqtype;
  if (!xdr_encodeMsg (&xdrs, (char *) &userInfoReq, &hdr, xdr_infoReq,
          0, NULL))
    {
      xdr_destroy (&xdrs);
      free (request_buf);
      lsberrno = LSBE_XDR;
      return (NULL);
    }

    assert( XDR_GETPOS (&xdrs) <= INT_MAX );
    returnValue = callmbd (NULL, request_buf, (int)XDR_GETPOS (&xdrs), &reply_buf, &hdr, NULL, NULL, NULL);
  if (-1 == returnValue )
    {
      xdr_destroy (&xdrs);
      free (request_buf);
      return (NULL);
    }
  xdr_destroy (&xdrs);
  free (request_buf);

  lsberrno = hdr.opCode;
  if (lsberrno == LSBE_NO_ERROR || lsberrno == LSBE_BAD_USER)
    {
      assert( cc <= UINT_MAX );
      xdrmem_create (&xdrs, reply_buf, XDR_DECODE_SIZE_ ((uint)cc), XDR_DECODE);
      reply = &userInfoReply;
      if (!xdr_userInfoReply (&xdrs, reply, &hdr))
  {
    lsberrno = LSBE_XDR;
    xdr_destroy (&xdrs);
    if (cc)
      free (reply_buf);
    return (NULL);
  }
      xdr_destroy (&xdrs);
      if (cc)
  free (reply_buf);
      if (lsberrno == LSBE_BAD_USER)
  {
    assert( reply->badUser >= 0);
    *numUsers = (uint)reply->badUser;
    return (NULL);
  }
  assert( reply->numUsers >=0 );
      *numUsers = (uint) reply->numUsers;
      return (reply->users);
    }

  if (cc)
    free (reply_buf);
  return (NULL);

}
