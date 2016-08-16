/* $Id: lsb.queues.c 397 2007-11-26 19:04:00Z mblack $
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
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#include "lsb/lsb.h"
#include "lib/xdr.h"

struct queueInfoEnt *
lsb_queueinfo (char **queues, uint *numQueues, char *hosts, char *users, int options)
{
  mbdReqType mbdReqtype;
  static struct infoReq queueInfoReq;
  static struct queueInfoReply reply;
  static struct queueInfoEnt **qInfo = NULL;
  struct queueInfoEnt **qTmp;
  XDR xdrs, xdrs2;
  char *request_buf;
  char *reply_buf;
  int cc = 0;

  static struct LSFHeader hdr;
  char *clusterName = NULL;

  assert( options );

  if (qInfo != NULL)
    {
      for ( uint i = 0; i < reply.numQueues; i++)
	{
	  xdr_lsffree (xdr_queueInfoEnt, (char *) qInfo[i], &hdr);
	}
    }

  if (numQueues == NULL)
    {
      lsberrno = LSBE_BAD_ARG;
      return NULL;
    }
  if ((queues == NULL && *numQueues > 1) )
    {
      lsberrno = LSBE_BAD_ARG;
      return NULL;
    }

  queueInfoReq.options = 0;

  if (queueInfoReq.names)
    {
      FREEUP (queueInfoReq.names);
    }

  if (numQueues == NULL || *numQueues == 0)
    queueInfoReq.options |= ALL_QUEUE;
  else if (queues == NULL && *numQueues == 1)
    queueInfoReq.options |= DFT_QUEUE;

  if ((queueInfoReq.options & ALL_QUEUE)
      || (queueInfoReq.options & DFT_QUEUE))
    {
      queueInfoReq.names = (char **) malloc (3 * sizeof (char *));
      if ( NULL == queueInfoReq.names && ENOMEM == errno )
	{
	  lsberrno = LSBE_NO_MEM;
	  return (NULL);
	}
      queueInfoReq.names[0] = "";
      queueInfoReq.numNames = 1;
      cc = 1;
    }
  else
    {
      assert( *numQueues >= 0);
      queueInfoReq.names = (char **) calloc ((unsigned long )*numQueues + 2, sizeof (char *));
      if ( NULL == queueInfoReq.names && ENOMEM == errno )
	{
	  lsberrno = LSBE_NO_MEM;
	  return (NULL);
	}
      queueInfoReq.numNames = *numQueues;
      for (uint i = 0; i < *numQueues; i++)
	{
	  if (queues[i] && strlen (queues[i]) + 1 < MAXHOSTNAMELEN)
	    queueInfoReq.names[i] = queues[i];
	  else
	    {
	      free (queueInfoReq.names);
	      queueInfoReq.names = NULL;
	      lsberrno = LSBE_BAD_QUEUE;
	      *numQueues = i;
	      return (NULL);
	    }
	}
      assert( queueInfoReq.numNames <= INT_MAX );
      cc = (int) queueInfoReq.numNames;
    }
  if (users != NULL)
    {
      if (strlen (users) + 1 < MAX_LSB_NAME_LEN)
	{
	  queueInfoReq.options |= CHECK_USER;
	  queueInfoReq.names[cc] = users;
	  cc++;
	}
      else
	{
	  lsberrno = LSBE_BAD_USER;
	  *numQueues = 0;
	  return (NULL);
	}
    }

  if (hosts != NULL)
    {
      if (ls_isclustername (hosts) <= 0)
	{
	  if (strlen (hosts) + 1 < MAXHOSTNAMELEN)
	    {
	      queueInfoReq.options |= CHECK_HOST;
	      queueInfoReq.names[cc] = hosts;
	      cc++;
	    }
	  else
	    {
	      lsberrno = LSBE_BAD_HOST;
	      *numQueues = 0;
	      return (NULL);
	    }
	}
      else
	clusterName = hosts;
    }
  queueInfoReq.resReq = "";


  mbdReqtype = BATCH_QUE_INFO;
  assert( cc >= 0);
  cc = (int)(sizeof (struct infoReq) + (unsigned long)cc * MAXHOSTNAMELEN + (unsigned long)cc + 100);
  request_buf = (char *)malloc ( (unsigned long)cc);
  if ( NULL == request_buf && ENOMEM == errno )
    {
      lsberrno = LSBE_NO_MEM;
      return (NULL);
    }
  xdrmem_create (&xdrs, request_buf, MSGSIZE, XDR_ENCODE);
  initLSFHeader_ (&hdr);
  hdr.opCode = mbdReqtype;
  if (!xdr_encodeMsg (&xdrs, (char *) &queueInfoReq, &hdr, xdr_infoReq, 0, NULL))
    {
      lsberrno = LSBE_XDR;
      xdr_destroy (&xdrs);
      free (request_buf);
      return (NULL);
    }

    assert( XDR_GETPOS (&xdrs) <= INT_MAX );
  cc = callmbd (clusterName, request_buf, (int)XDR_GETPOS (&xdrs), &reply_buf, &hdr, NULL, NULL, NULL);
  if ( -1 == cc)
    {
      xdr_destroy (&xdrs);
      free (request_buf);
      return (NULL);
    }

  xdr_destroy (&xdrs);
  free (request_buf);


  lsberrno = hdr.opCode;
  if (lsberrno == LSBE_NO_ERROR || lsberrno == LSBE_BAD_QUEUE)
    {
      assert( XDR_DECODE_SIZE_ (cc) >=0 );
      xdrmem_create (&xdrs2, reply_buf, (uint)XDR_DECODE_SIZE_ (cc), XDR_DECODE);
      if (!xdr_queueInfoReply (&xdrs2, &reply, &hdr))
	{
	  lsberrno = LSBE_XDR;
	  xdr_destroy (&xdrs2);
	  if (cc)
	    free (reply_buf);
	  *numQueues = 0;
	  return (NULL);
	}
      xdr_destroy (&xdrs2);
      if (cc)
	free (reply_buf);
      if (lsberrno == LSBE_BAD_QUEUE)
	{
	  *numQueues = reply.badQueue;
	  return (NULL);
	}
      assert( reply.numQueues >= 0 );
      qTmp = (struct queueInfoEnt **) myrealloc (qInfo, (unsigned long)reply.numQueues * sizeof (struct queueInfoEnt *));
      if ( NULL == qTmp && ENOMEM == errno )
	{
	  lsberrno = LSBE_NO_MEM;
	  return NULL;
	}
      qInfo = qTmp;
      for (uint i = 0; i < reply.numQueues; i++)
	qInfo[i] = &(reply.queues[i]);

      *numQueues = reply.numQueues;
      return (qInfo[0]);
    }

  if (cc)
    free (reply_buf);
  *numQueues = 0;
  return (NULL);

}
