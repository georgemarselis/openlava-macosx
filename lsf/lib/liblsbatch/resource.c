/* $Id: lsb.resource.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
#include "lsb/lsb.h"
#include "lib/xdr.h"

struct lsbSharedResourceInfo *
lsb_sharedresourceinfo (char **resources, unsigned int *numResources, char *hostName, int options)
{
    static struct lsbShareResourceInfoReply lsbResourceInfoReply; // FIXME FIXME full init
    static struct LSFHeader hdr;            // FIXME FIXME full init
    struct resourceInfoReq resourceInfoReq; // FIXME FIXME full init
    mbdReqType mbdReqtype = 0;
    XDR xdrs;               // FIXME FIXME full init
    XDR xdrs2;              // FIXME FIXME full init
    char *clusterName     = NULL;
    char *request_buf     = NULL;
    char *reply_buf       = NULL;
    int cc                = 0;
    
    assert( options );

    if (logclass & (LC_TRACE)) {
        ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __PRETTY_FUNCTION__);
    }

    if (lsbResourceInfoReply.numResources > 0) {
        xdr_lsffree (xdr_lsbShareResourceInfoReply, (char *) &lsbResourceInfoReply, &hdr);
    }

    if( numResources == NULL || ( resources == NULL && *numResources != 0) ||
        ( resources != NULL && *numResources == 0)
      )
    {
      lsberrno = LSBE_BAD_ARG;
      return (NULL);
    }
  resourceInfoReq.options = 0;

  if (*numResources == 0)
    {

      resourceInfoReq.resourceNames =(char **) malloc (sizeof (char *));
      if (NULL == resourceInfoReq.resourceNames && ENOMEM == errno )
    {
      lsberrno = LSBE_NO_MEM;
      return (NULL);
    }
      resourceInfoReq.resourceNames[0] = "";
      resourceInfoReq.numResourceNames = 1;
      cc += 2;
    }
  else
    {
      assert( *numResources >= 0 );
      resourceInfoReq.resourceNames = (char **) malloc ((unsigned long)*numResources * sizeof (char *));
      if ( NULL == resourceInfoReq.resourceNames && ENOMEM == errno )
    {
      lsberrno = LSBE_NO_MEM;
      return (NULL);
    }
      for ( unsigned int i = 0; i < *numResources; i++)
    {
      if (resources[i] && strlen (resources[i]) + 1 < MAXLSFNAMELEN)
        {
          resourceInfoReq.resourceNames[i] = resources[i];
          cc += MAXLSFNAMELEN;
        }
      else
        {
          free (resourceInfoReq.resourceNames);
          lserrno = LSBE_BAD_RESOURCE;
          return (NULL);
        }
    }
      resourceInfoReq.numResourceNames = *numResources;
    }
    if (hostName != NULL)
    {
        if (ls_isclustername (hostName) <= 0)
        {
            if (strlen (hostName) + 1 < MAXHOSTNAMELEN)
            {
                resourceInfoReq.hostName = hostName;
            }
                else
                {
                lsberrno = LSBE_BAD_HOST;
                return NULL;
            }
        }
        else
        {
            clusterName = hostName;
            cc += MAXHOSTNAMELEN;
        }
    }
    else {
        resourceInfoReq.hostName = " ";
    }


    mbdReqtype = BATCH_RESOURCE_INFO;
    assert ( cc >= 0 && cc <= INT_MAX);
    cc = sizeof (struct resourceInfoReq) + cc + 100; // FIXME FIXME FIXME test case, find longest length needed
    request_buf = malloc( sizeof( char ) * cc );
    if ( NULL == request_buf && ENOMEM == errno )
    {
        lsberrno = LSBE_NO_MEM;
        return NULL;
    }
    xdrmem_create (&xdrs, request_buf, MSGSIZE, XDR_ENCODE);
    initLSFHeader_ (&hdr);
    hdr.opCode = mbdReqtype;
    if (!xdr_encodeMsg(&xdrs, (char *) &resourceInfoReq, &hdr, xdr_resourceInfoReq, 0, NULL))
    {
        lsberrno = LSBE_XDR;
        xdr_destroy (&xdrs);
        free(request_buf);
        free(resourceInfoReq.resourceNames);;
        return NULL;
    }

    assert( XDR_GETPOS (&xdrs) <= INT_MAX);
    cc = callmbd (clusterName, request_buf, XDR_GETPOS (&xdrs), &reply_buf, &hdr, NULL, NULL, NULL);
    if (-1 == cc )
    {
        xdr_destroy (&xdrs);
        free(request_buf);
        free(resourceInfoReq.resourceNames);
        return NULL;
    }
    free(request_buf);
    free(resourceInfoReq.resourceNames);

  lsberrno = hdr.opCode;
  if (lsberrno == LSBE_NO_ERROR)
    {
      assert( XDR_DECODE_SIZE_ (cc) >= 0 );
      xdrmem_create (&xdrs2, reply_buf, XDR_DECODE_SIZE_(cc), XDR_DECODE);
      if (!xdr_lsbShareResourceInfoReply (&xdrs2, &lsbResourceInfoReply, &hdr))
    {
        lsberrno = LSBE_XDR;
        xdr_destroy (&xdrs2);
        if (cc) {
            free(reply_buf); 
        }
      return (NULL);
    }
      xdr_destroy (&xdrs2);
              if (cc) {
            free(reply_buf); 
        }

      *numResources = lsbResourceInfoReply.numResources;
      return (lsbResourceInfoReply.resources);
    }

          if (cc) {
            free(reply_buf); 
        }

  return (NULL);
}
