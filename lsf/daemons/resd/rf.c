/* $Id: res.rf.c 397 2007-11-26 19:04:00Z mblack $
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

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/rf.h"
#include "lib/res.h"
#include "lib/xdrres.h"
#include "daemons/libresd/resout.h"

#define NL_SETN         29

#define LSRCP_MSGSIZE 65536

static int clearSock (int sock, int len);
static int rread (int sock, struct LSFHeader *hdr);
static int rwrite (int sock, struct LSFHeader *hdr);
static int rclose (int sock, struct LSFHeader *hdr);
static int rlseek (int sock, struct LSFHeader *hdr);
static int ropen (int sock, struct LSFHeader *hdr);
static int rstat (int sock, struct LSFHeader *hdr);
static int rfstat (int sock, struct LSFHeader *hdr);
static int rgetmnthost (int sock, struct LSFHeader *hdr);
static int runlink (int sock, struct LSFHeader *hdr);

void
rfServ_ (int acceptSock)
{
  static char __func__] = "rfServ_()";
  struct LSFHeader msgHdr;
  struct LSFHeader buf;
  struct sockaddr_in from;
  socklen_t fromLen = sizeof (from);
  int sock;
  XDR xdrs;

  sock = accept (acceptSock, (struct sockaddr *) &from, &fromLen);
  if (sock < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeHdr_");
	  closesocket (acceptSock);
	  return;
	}

  xdrmem_create (&xdrs, (char *) &buf, sizeof (buf), XDR_DECODE);

  for (;;)
	{

	  XDR_SETPOS (&xdrs, 0);
	  if (readDecodeHdr_ (sock,
			  (char *) &buf, SOCK_READ_FIX, &xdrs, &msgHdr) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeHdr_");
	  closesocket (sock);
	  xdr_destroy (&xdrs);
	  return;
	}

	  switch (msgHdr.opCode)
	{
	case RF_OPEN:
	  ropen (sock, &msgHdr);
	  break;

	case RF_CLOSE:
	  rclose (sock, &msgHdr);
	  break;

	case RF_WRITE:
	  rwrite (sock, &msgHdr);
	  break;

	case RF_READ:
	  rread (sock, &msgHdr);
	  break;

	case RF_STAT:
	  rstat (sock, &msgHdr);
	  break;

	case RF_GETMNTHOST:
	  rgetmnthost (sock, &msgHdr);
	  break;

	case RF_FSTAT:
	  rfstat (sock, &msgHdr);
	  break;

	case RF_LSEEK:
	  rlseek (sock, &msgHdr);
	  break;

	case RF_UNLINK:
	  runlink (sock, &msgHdr);
	  break;

	case RF_TERMINATE:
	  closesocket (sock);
	  return;

	default:
	  ls_errlog (stderr, _i18n_msg_get (ls_catd, NL_SETN, 602,
						"%s: Unknown opcode %d"),
			 __func__, msgHdr.opCode);
	  xdr_destroy (&xdrs);
	  break;
	}
	}

}

static int
ropen (int sock, struct LSFHeader *hdr)
{
	static char __func__] = "ropen()";
	int fd = 0 ;
	XDR xdrs = { };
	struct ropenReq req = { };
	char *buf = malloc( sizeof( char ) * LSRCP_MSGSIZE + 1 ); // FIXME FIXME FIXME get rid of label
	char *fn  = malloc( sizeof( char ) * MAXFILENAMELEN + 1 ); // FIXME FIXME FIXME get rid of label

	req.fn = fn;

	xdrmem_create (&xdrs, buf, LSRCP_MSGSIZE, XDR_DECODE);
	if (readDecodeMsg_ (sock, buf, hdr, SOCK_READ_FIX, &xdrs, (char *) &req, xdr_ropenReq, NULL))
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeMsg_");
	  xdr_destroy (&xdrs);
	  closesocket (sock);
	  return (-1);
	}
	xdr_destroy (&xdrs);

  if (req.flags & LSF_O_CREAT_DIR)
	{
	  req.flags &= ~LSF_O_CREAT_DIR;


	  if (createSpoolSubDir (fn) < 0)
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf,
			  sizeof (struct LSFHeader),
			  NULL, SOCK_WRITE_FIX, NULL) < 0)
		{
		  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
		  closesocket (sock);
		  return (-1);
		}
	  return (0);
	}
	}


  if ((fd = open (req.fn, req.flags, req.mode)) == -1)
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL)
	  < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  closesocket (sock);
	  return (-1);
	}
	  return (0);
	}


  if (lsSendMsg_ (sock, fd, 0, NULL, buf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  closesocket (sock);
	  close (fd);
	  return (-1);
	}

	free(buf);
	free(fn);

  return 0;
}


static int
rclose (int sock, struct LSFHeader *hdr)
{
  static char __func__] = "rclose()";
  int reqfd = 0 ;
  char *buf = malloc( sizeof( char ) * LSRCP_MSGSIZE + 1 );
  XDR xdrs;

  xdrmem_create (&xdrs, buf, LSRCP_MSGSIZE, XDR_DECODE);
  if (readDecodeMsg_ (sock, buf, hdr, SOCK_READ_FIX, &xdrs, (char *) &reqfd,
			  xdr_int, NULL))
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeMsg_");
	  xdr_destroy (&xdrs);
	  closesocket (sock);
	  return (-1);
	}
  xdr_destroy (&xdrs);

  if (close (reqfd) == -1)
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf,
			  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL)
	  < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	}
	  return (0);
	}

  if (lsSendMsg_ (sock, 0, 0, NULL, buf,
		  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}

   free(buf);
  return 0;
}

static int
rwrite (int sock, struct LSFHeader *hdr)
{
  static char __func__] = "rwrite()";
  struct rrdwrReq req = { };
  char *msgBuf = malloc( sizeof( char ) * LSRCP_MSGSIZE + 1 );
  XDR xdrs = { };
  char *buf = NULL;
  ssize_t result = 0;

  xdrmem_create (&xdrs, msgBuf, LSRCP_MSGSIZE, XDR_DECODE);
  if (readDecodeMsg_ (sock, msgBuf, hdr, SOCK_READ_FIX, &xdrs, (char *) &req, // FIXME FIXME FIXME FIXME is that (char *) cast appropriate? replace? 
			  xdr_rrdwrReq, NULL))
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDeocdeMsg_");
	  xdr_destroy (&xdrs);
	  close (sock);
	  return (-1);
	}
  xdr_destroy (&xdrs);

  buf = malloc( sizeof( char ) * req.len  + 1 ) ;
  if ( NULL == buf ) 
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, msgBuf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL)
	  < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	}
	  clearSock (sock, req.len);
	  return (0);
	}

  if (SOCK_READ_FIX (sock, buf, req.len) != req.len)
	{
	  ls_errlog (stderr, I18N_FUNC_D_FAIL_M, __func__, "SOCK_READ_FIX", req.len);
	  goto fail;
	}

	result = write (req.fd, buf, req.len) ;
	if( result == -1 && errno )
	{
	  goto fail;
	}


  if (lsSendMsg_ (sock, 0, req.len, NULL, msgBuf,
		  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}

	free( buf ); // FIXME FIXME FIXME move above previous if statement to save req.len * sizeof( char ) bytes
	free( msgBuf );

	return 0;

fail: // FIXME FIXME FIXME FIXME remove goto

  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, msgBuf,
		  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}

	free( buf );
	free( msgBuf );

	return (0);
}



static int
rread (int sock, struct LSFHeader *hdr)
{
	static char __func__] = "rread()";
	struct rrdwrReq req = { };
	XDR xdrs            = { };
	ssize_t readResult  = 0;
	char *buf           = NULL;
	char *msgBuf        = malloc( sizeof ( char ) * LSRCP_MSGSIZE + 1) ;

  xdrmem_create (&xdrs, msgBuf, LSRCP_MSGSIZE, XDR_DECODE);
  if (readDecodeMsg_ (sock, msgBuf, hdr, SOCK_READ_FIX, &xdrs, (char *) &req, xdr_rrdwrReq, NULL)) // FIXME FIXME FIXME is this cast correct?
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeMsg_");
	  xdr_destroy (&xdrs);
	  close (sock);
	  return -1;
	}
  xdr_destroy (&xdrs);

  buf = malloc( sizeof(char) * req.len + 1 ); 
  if( NULL == buf )
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, msgBuf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	}
	  return 0;
	}

	readResult = read( req.fd, buf, req.len );
	if ( readResult < 0 ) // FIXME FIXME is it ok to only consider the return value and not the error? should it not send more details?
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
		{
		  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
		  close (sock);
		  return -1;
		}
		return 0;
	}
	else {
		req.len = readResult;
	}

	if (lsSendMsg_ (sock, 0, req.len, NULL, msgBuf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
		ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
		close (sock);
		free (buf);
		free(msgBuf);
		return -1;
	}

	if (SOCK_WRITE_FIX (sock, buf, req.len) != req.len)
	{
		ls_errlog (stderr, I18N_FUNC_FAIL_M, __func__, "SOCK_WRITE_FIX");
		close (sock);
		free (buf);
		free(msgBuf);
		return -1;
	}

	free (buf);
	free(msgBuf);
	return (0);

// fail: // FIXME FIXME FIXME FIXME 
//   free (buf);

//   if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf,
// 		  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
// 	{
// 	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
// 	  close (sock);
// 	  return (-1);
// 	}
//   return (0);

}

static int
rlseek (int sock, struct LSFHeader *hdr)
{
	static char __func__] = "rlseek()";
	struct rlseekReq req = { };
	XDR xdrs = { };
	off_t pos = 0;
	char *msgBuf = malloc( sizeof ( char ) * LSRCP_MSGSIZE + 1 );
	int returnValue = 0;

	xdrmem_create (&xdrs, msgBuf, LSRCP_MSGSIZE, XDR_DECODE);
	if (readDecodeMsg_ (sock, msgBuf, hdr, SOCK_READ_FIX, &xdrs, (char *) &req, xdr_rlseekReq, NULL)) // FIXME FIXME FIXME is this cast correct? should it not be void * or something?
	{
		ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeMsg_");
		xdr_destroy (&xdrs);
		close (sock);
		free( msgBuf );
		return -1;
	}
	xdr_destroy (&xdrs);

	if ((pos = lseek (req.fd, req.offset, req.whence)) < 0)
	{
		if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, msgBuf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
		{
			ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
			free( msgBuf );
			close (sock);
			return -1;
		}
	}

	if (lsSendMsg_ (sock, 0, pos, NULL, msgBuf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
		ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
		close (sock);
		free( msgBuf );
		return -1;
	}

	free( msgBuf );
	return 0;

// fail:
//   if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, msgBuf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
// 	{
// 	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
// 	  close (sock);
// 	  return (-1);
// 	}
}



static int
clearSock (int sock, int len)
{
  static char __func__] = "clearSock()";
  int l;
  char buf[LSRCP_MSGSIZE];

  for (; len; len -= l)
	{
	  l = len > LSRCP_MSGSIZE ? LSRCP_MSGSIZE : len;
	  if (SOCK_READ_FIX (sock, buf, l) != l)
	{
	  ls_errlog (stderr, I18N_FUNC_D_FAIL_M, __func__, "SOCK_READ_FIX", l);
	  close (sock);
	  return (-1);
	}
	}

  return (0);
}

static int
rstat (int sock, struct LSFHeader *hdr)
{
  static char __func__] = "rstat()";
  struct stat st;
  char buf[LSRCP_MSGSIZE];
  char fn[MAXFILENAMELEN];
  XDR xdrs;
  struct stringLen fnStr;

  fnStr.len = MAXFILENAMELEN;
  fnStr.name = fn;

  xdrmem_create (&xdrs, buf, LSRCP_MSGSIZE, XDR_DECODE);
  if (readDecodeMsg_ (sock, buf, hdr, SOCK_READ_FIX, &xdrs,
			  (char *) &fnStr, xdr_stringLen, NULL))
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDeocdeMsg_");
	  xdr_destroy (&xdrs);
	  close (sock);
	  return (-1);
	}
  xdr_destroy (&xdrs);

  if (stat (fn, &st) == -1)
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf,
			  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL)
	  < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}
	  return (0);
	}

  if (lsSendMsg_ (sock, 0, 0, (char *) &st, buf, LSRCP_MSGSIZE, xdr_stat,
		  SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}

  return (0);
}


static int
rfstat (int sock, struct LSFHeader *hdr)
{
  static char __func__] = "rfstat()";
  int reqfd;
  char msgBuf[LSRCP_MSGSIZE];
  XDR xdrs;
  struct stat st;

  xdrmem_create (&xdrs, msgBuf, LSRCP_MSGSIZE, XDR_DECODE);
  if (readDecodeMsg_
	  (sock, msgBuf, hdr, SOCK_READ_FIX, &xdrs, (char *) &reqfd, xdr_int,
	   NULL))
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeMsg_");
	  xdr_destroy (&xdrs);
	  close (sock);
	  return (-1);
	}
  xdr_destroy (&xdrs);

  if (fstat (reqfd, &st) == -1)
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, msgBuf,
			  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL)
	  < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}
	  return (0);
	}

  if (lsSendMsg_ (sock, 0, 0, (char *) &st, msgBuf, LSRCP_MSGSIZE, xdr_stat,
		  SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}

  return (0);
}


static int
rgetmnthost (int sock, struct LSFHeader *hdr)
{
  static char __func__] = "rgetmnthost()";
  char buf[LSRCP_MSGSIZE];
  char fn[MAXFILENAMELEN], *host;
  XDR xdrs;
  struct stringLen fnStr;
  struct stringLen hostStr;

  fnStr.len = MAXFILENAMELEN;
  fnStr.name = fn;

  xdrmem_create (&xdrs, buf, LSRCP_MSGSIZE, XDR_DECODE);
  if (readDecodeMsg_ (sock, buf, hdr, SOCK_READ_FIX, &xdrs,
			  (char *) &fnStr, xdr_stringLen, NULL))
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeMsg_");
	  xdr_destroy (&xdrs);
	  close (sock);
	  return (-1);
	}
  xdr_destroy (&xdrs);

  if ((host = ls_getmnthost (fn)) == NULL)
	{
	  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf,
			  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL)
	  < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}
	  return (0);
	}

  hostStr.len = MAXHOSTNAMELEN;
  hostStr.name = host;
  if (lsSendMsg_
	  (sock, 0, 0, (char *) &hostStr, buf, LSRCP_MSGSIZE, xdr_stringLen,
	   SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  close (sock);
	  return (-1);
	}

  return (0);
}


#ifdef __sun__
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

static int
runlink (int sock, struct LSFHeader *hdr) // FIXME FIXME is int the right choice here?
{
  static char __func__]    = "runlink()";
  XDR xdrs               = { };
  struct stat st         = { };
  struct stringLen fnStr = { };
  char *buf              = malloc( sizeof( char ) * LSRCP_MSGSIZE + 1 );
  char *fn               = malloc( sizeof( char ) * MAXFILENAMELEN + 1 );

  fnStr.len = MAXFILENAMELEN; // FIXME FIXME FIXME replace this by OS filename defined assigned into a constant.
  fnStr.name = fn;

  xdrmem_create (&xdrs, buf, LSRCP_MSGSIZE, XDR_DECODE);
  if (readDecodeMsg_ (sock, buf, hdr, SOCK_READ_FIX, &xdrs, (char *) &fnStr, xdr_stringLen, NULL)) // FIXME FIXME FIXME FIXME is this cast correct? should it not be void * or something?
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "readDecodeMsg_");
	  xdr_destroy (&xdrs);
	  close (sock);
			  free( buf );
			  free( fn );
	  return (-1);
	}
  xdr_destroy (&xdrs);

  if ((stat (fn, &st) == 0) && (st.st_mode & S_IFDIR))
	{


	  DIR *dirp = NULL;
	  struct dirent *dp = NULL;
	  char *path = malloc( sizeof(char) * MAXPATHLEN + 1 ); // FIXME FIXME FIXME replace with OS define assigned into a const

	  if ((dirp = opendir (fn)) == NULL)
	{
		  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
			{
			  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
			  closesocket (sock);
			  free( path );
			  free( buf );
			  free( fn );
			  return -1;
			}
			  free( path );
			  free( buf );
			  free( fn );
		  return 0;
	}

	  for (dp = readdir (dirp); dp != NULL; dp = readdir (dirp))
	{
	  if (strcmp (dp->d_name, ".") != 0 && strcmp (dp->d_name, "..") != 0)
		{
		  
		  free( path );
		  path = malloc( sizeof( char ) * strlen( dp->d_name )  + 1 ) ;
		  sprintf (path, "%s/%s", fn, dp->d_name);
		  rmdir (path);
		  unlink (path);
		  free( path );

		}
	}

	  closedir (dirp);
	  if (rmdir (fn) != 0)
	{
		  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
			{
			  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
			  closesocket (sock);
			  free( path );
			  free( buf );
			  free( fn );
			  return (-1);
			}
			  free( path );
			  free( buf );
			  free( fn );
		  return (0);
	}
	}

  if (unlink (fn) < 0)
	{
		  if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
			{
			  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
			  closesocket (sock);
			  free( buf );
			  free( fn );
			  return (-1);
			}
			  free( buf );
			  free( fn );
		  return (0);
	}

  if (lsSendMsg_ (sock, 0, 0, NULL, buf, sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
	{
	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
	  closesocket (sock);
			  free( buf );
			  free( fn );
	  return (-1);
	}

	free( buf );
	free( fn );
  return (0);
}

// errrtn: // FIXME FIXME FIXME FIXME FIXME remove 

//   if (lsSendMsg_ (sock, -errnoEncode_ (errno), 0, NULL, buf,
// 		  sizeof (struct LSFHeader), NULL, SOCK_WRITE_FIX, NULL) < 0)
// 	{
// 	  ls_errlog (stderr, I18N_FUNC_FAIL_MM, __func__, "lsSendMsg_");
// 	  closesocket (sock);
// 	  return (-1);
// 	}
//   return (0);
// }
