/* $Id: daemons.c 397 2007-11-26 19:04:00Z mblack $
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
#include <string.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include "daemons/daemons.h"
#include "libint/lsi18n.h"
#include "lsb/spool.h"

// #define NL_SETN   10


int
init_ServSock (u_short port)
{
  int ch;

  ch = chanServSocket_ (SOCK_STREAM, ntohs (port), 1024, CHAN_OP_SOREUSE);
  if (ch < 0)
    {
      ls_syslog (LOG_ERR, "%s: chanServSocket_() failed to get socket %m", __func__);
      return -1;
    }

  return ch;
}

int
rcvJobFile (int chfd, struct lenData *jf)
{
  static char __func__] = "rcvJobFile";
  long cc = 0;
  unsigned long returnValue = 0;


  jf->data = NULL;
  jf->len = 0;

  if ((cc = chanRead_ (chfd, NET_INTADDR_ (&jf->len), NET_INTSIZE_)) != NET_INTSIZE_)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "chanRead_");
      return -1;
    }

  jf->len = ntohl (jf->len);
  assert( jf->len <= INT_MAX );
  jf->data = my_malloc ( jf->len, "rcvJobFile");

  cc = chanRead_ (chfd, jf->data, jf->len);
  assert( cc >= 0);
  returnValue = (unsigned long) cc;
  if ( returnValue != jf->len)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "chanRead_");
      free (jf->data);
      return -1;
    }

  return 0;
}

int
do_readyOp (XDR *xdrs, int chanfd, struct sockaddr_in *from, struct LSFHeader *reqHdr)
{
    static char __func__] = "do_readyOp";
    XDR xdrs2;
    struct Buffer *buf;
    struct LSFHeader replyHdr;

    assert( reqHdr->length );
    assert( from->sin_port );
    assert( xdrs->x_handy );


  if (chanAllocBuf_ (&buf, sizeof (struct LSFHeader)) < 0)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
      return (-1);
    }
  initLSFHeader_ (&replyHdr);
  replyHdr.opCode = READY_FOR_OP;
  replyHdr.length = 0;

  xdrmem_create (&xdrs2, buf->data, sizeof (struct LSFHeader), XDR_ENCODE);
  if (!xdr_LSFHeader (&xdrs2, &replyHdr))
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_LSFHeader");
      xdr_destroy (&xdrs2);
      return (-1);

    }

  buf->len = XDR_GETPOS (&xdrs2);

  if (chanEnqueue_ (chanfd, buf) < 0)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "chanEnqueue_");
      xdr_destroy (&xdrs2);
      return (-1);
    }

  xdr_destroy (&xdrs2);
  return (0);
}


void
childRemoveSpoolFile (const char *spoolFile, int options, const struct passwd *pwUser)
{
    char __func__]    = "childRemoveSpoolFile";
    char apiName[]  = "ls_initrex";
    char *hostName  = malloc( sizeof( char ) * MAXHOSTNAMELEN + 1 ); // FIXME FIXME FIXME FIXME free memory after use
    char *errMsg    = malloc( sizeof( char ) * MAX_LINE_LEN + 1 ); // FIXME FIXME FIXME FIXME free memory after use
    char *dirName   = malloc( sizeof( char ) * MAX_LINE_LEN + 1 ); // FIXME FIXME FIXME FIXME free memory after use
    char *fromHost  = NULL;
    static char *sp = NULL ;
    pid_t pid = 0;
    int status = 0;

    status = -1;

  if ((fromHost = getSpoolHostBySpoolFile (spoolFile)) != NULL)
    {
      strcpy (hostName, fromHost);
    }
  else
    {
      /* catgets 8000 */
      ls_syslog (LOG_ERR, "catgets 8000: Unable to get spool host from the string \'%s\'", (spoolFile ? spoolFile : "NULL"));
      goto Done;
    }

  if (pwUser == NULL)
    {
      /* catgets 8001 */
      ls_syslog (LOG_ERR, "catgets 8001: %s: Parameter const struct passwd * pwUser is NULL", __func__);
      goto Done;
    }


  sp = getLowestDir_ (spoolFile);
  if (sp)
    {
      strcpy (dirName, sp);
    }
  else
    {
      strcpy (dirName, spoolFile);
    }

  /* catgets 3000 */
  sprintf (errMsg, "catgets 3000: %s: Unable to remove spool file:\n,\'%s\'\n on host %s\n", __func__, dirName, fromHost);

  if (!(options & FORK_REMOVE_SPOOL_FILE))
    {

      if ((options & CALL_RES_IF_NEEDED))
  {
    if (ls_initrex (1, 0) < 0)
      {
        status = -1;
        /* catgets 3001 */
        sprintf (errMsg, "catgets 3001: %s: %s failed when trying to delete %s from %s\n",  __func__, apiName, dirName, fromHost);
        goto Error;
      }
  }

      chuser (pwUser->pw_uid);

      status = removeSpoolFile (hostName, dirName);

      chuser (batchId);

      if (status != 0)
  {
    goto Error;
  }
      goto Done;
    }

  switch (pid = fork ())
    {
    case 0:

      if (debug < 2)
  {
    closeExceptFD (-1);
  }

      if ((options & CALL_RES_IF_NEEDED))
  {
    if (ls_initrex (1, 0) < 0)
      {
        status = -1;
        /* catgets 3001 */
        sprintf (errMsg, "3001: %s: %s failed when trying to delete %s from %s\n", __func__, apiName, dirName, fromHost);
        goto Error;
      }
  }

      chuser (pwUser->pw_uid);
      status = 0;
      if (removeSpoolFile (hostName, dirName) == 0)
  {
    exit (0);
  }
      else
  {
    exit (-1);
  }
      //goto Done;
      //break;

    case -1:

      if (logclass & (LC_FILE))
  {
    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
  }
      status = -1;
      /* catgets 3002 */
      sprintf (errMsg, "catgets 3002: %s: Unable to fork to remove spool file:\n,\'%s\'\n on host %s\n", __func__, dirName, fromHost);
      goto Error;
    default:
      status = 0;
      goto Done;
    }

Error: // FIXME FIXME FIXME FIXME FIXME remove label

  if (status == -1)
    {
      lsb_merr (errMsg);
    }
Done: // FIXME FIXME FIXME FIXME FIXME remove label
  return;
}
