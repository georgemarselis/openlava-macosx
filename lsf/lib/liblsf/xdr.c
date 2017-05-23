/*
 * Copyright (C) David Bigagli
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

#include <limits.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

#include "lib/lib.h"
#include "lib/xdr.h"
#include "lib/lproto.h"

// // <rpc/xdr.h> defines an __LP64__ type
// #ifdef __APPLE__
// #undef __LP64__
// #endif

/* encodeHdr()
 * Pack the header into 16 4 unsigned integers,
 * if we don't pack xdr would align the shorts
 * into int and we 24 bytes instead of 16. Big deal?
 */
void encodeHdr ( pid_t *word1, size_t *word2, unsigned int *word3, unsigned int *word4, struct LSFHeader *header)
{
	*word1 = header->refCode;
	*word1 <<= 16;
	*word1 = *word1 | (header->opCode & 0x0000FFFF);
	*word2 = header->length;
	*word3 = header->version;
	*word3 <<= 16;
	*word3 = *word3 | (header->reserved & 0x0000FFFF);
	*word4 = header->reserved0;
}

bool_t xdr_LSFHeader (XDR * xdrs, struct LSFHeader *header)
{
  /* openlava 2.0 header encode and
   * decode operations.
   */
  pid_t  word1 = 0;
  size_t word2 = 0;
  unsigned int word3 = 0;
  unsigned int word4 = 0;

  if (xdrs->x_op == XDR_ENCODE)
	{
	  encodeHdr (&word1, &word2, &word3, &word4, header);
	}

  if (!xdr_u_int (xdrs, (unsigned int *)&word1) || !xdr_u_int (xdrs, (unsigned int *) &word2) || !xdr_u_int (xdrs, &word3) || !xdr_u_int (xdrs, &word4)) {
	return FALSE;
  }

  if (xdrs->x_op == XDR_DECODE)
	{
	  header->refCode = word1 >> 16;
	  header->opCode = word1 & 0xFFFF;
	  header->length = word2;
	  header->version = word3 >> 16;
	  header->reserved = word3 & 0xFFFF;
	  assert( word4 <= USHRT_MAX );
	  header->reserved0 = (unsigned short) word4;
	}

  return TRUE;
}


bool_t
xdr_packLSFHeader (char *buf, struct LSFHeader * header)
{
  XDR xdrs;
  char hdrBuf[LSF_HEADER_LEN];

  xdrmem_create (&xdrs, hdrBuf, LSF_HEADER_LEN, XDR_ENCODE);

  if (!xdr_LSFHeader (&xdrs, header))
	{
	  lserrno = LSE_BAD_XDR;
	  xdr_destroy (&xdrs);
	  return FALSE;
	}

  memcpy (buf, hdrBuf, XDR_GETPOS (&xdrs));
  xdr_destroy (&xdrs);

  return TRUE;
}

bool_t
xdr_encodeMsg (XDR * xdrs, char *data, struct LSFHeader *hdr, bool_t (*xdr_func) (), int options, struct lsfAuth *auth)
{
  unsigned int len = 0;

  assert( options );
  XDR_SETPOS (xdrs, LSF_HEADER_LEN);

  hdr->version = OPENLAVA_VERSION;

  if (auth)
	{
	  if (!xdr_lsfAuth (xdrs, auth, hdr))
  return FALSE;
	}

  if (data)
	{
	  if (!(*xdr_func) (xdrs, data, hdr))
  return FALSE;
	}

  len = XDR_GETPOS (xdrs);
  hdr->length = len - LSF_HEADER_LEN;

  XDR_SETPOS (xdrs, 0);
  if (!xdr_LSFHeader (xdrs, hdr))
	return FALSE;

  XDR_SETPOS (xdrs, len);
  return TRUE;
}

bool_t
xdr_arrayElement (XDR * xdrs, char *data, struct LSFHeader * hdr, bool_t (*xdr_func) (), ...)
{
  va_list ap;
  unsigned int nextElementOffset = 0; 
  unsigned int pos = 0;
  char *cp = NULL;

  va_start (ap, xdr_func);

  pos = XDR_GETPOS (xdrs);

  if (xdrs->x_op == XDR_ENCODE)
	{
	  XDR_SETPOS (xdrs, pos + NET_INTSIZE_);
	}
  else
	{
		assert( nextElementOffset <= INT_MAX );
		if (!xdr_int (xdrs, (int *)&nextElementOffset)) {
			return FALSE;
		}
	}

  cp = va_arg (ap, char *);
  if (cp)
	{
		if (!(*xdr_func) (xdrs, data, hdr, cp))
			return FALSE;
	}
  else
	{
		if (!(*xdr_func) (xdrs, data, hdr)) {
			return FALSE;
		}
	}

  if (xdrs->x_op == XDR_ENCODE)
	{
	  nextElementOffset = XDR_GETPOS (xdrs) - pos;
	  XDR_SETPOS (xdrs, pos);
	  assert( nextElementOffset <= INT_MAX );
		if (!xdr_int (xdrs, (int *)&nextElementOffset)) { // FIXME FIXME FIXME FIXME we got to revisit this
			return FALSE;
		}
	}


  XDR_SETPOS (xdrs, pos + nextElementOffset);
  return TRUE;
}

bool_t
xdr_array_string (XDR * xdrs, char **astring, unsigned int maxlen, unsigned int arraysize)
{

  char line[MAXLINELEN] = "";
  char *sp = line;

  for (unsigned int i = 0; i < arraysize; i++) {
	if (xdrs->x_op == XDR_FREE) {
	  FREEUP (astring[i]);
	}
	else if (xdrs->x_op == XDR_DECODE) {
	  if (!xdr_string (xdrs, &sp, maxlen) || (astring[i] = putstr_ (sp)) == NULL) {
		for (unsigned int j = 0; j < i; j++) {
		  FREEUP (astring[j]);
		}
		return FALSE;
	  }
	}
	else {
	  if (!xdr_string (xdrs, &astring[i], maxlen)) {
		return FALSE;
	  }
	}
  }

  return TRUE;
}

bool_t
xdr_time_t (XDR *xdrs, time_t *t)
{
#ifdef __LINUX__
	return xdr_long( xdrs, t );
#elif defined(__APPLE__)
	return xdr_long( xdrs, (int *)t ); // FIXME FIXME FIXME FIXME we got to revisit this
#else
	#error
#endif
}

int
readDecodeHdr_ (int s, char *buf, long (*readFunc) (), XDR * xdrs, struct LSFHeader *hdr)
{
  if ((*readFunc) (s, buf, LSF_HEADER_LEN) != LSF_HEADER_LEN)
	{
	  lserrno = LSE_MSG_SYS;
	  return -2;
	}

  if (!xdr_LSFHeader (xdrs, hdr))
	{
	  lserrno = LSE_BAD_XDR;
	  return -1;
	}

  return 0;
}

int
readDecodeMsg_ (int s, char *buf, struct LSFHeader *hdr, long (*readFunc) (),  XDR * xdrs,  char *data, bool_t (*xdrFunc) (), struct lsfAuth *auth)
{
	assert( hdr->length <= LONG_MAX);
	if ((*readFunc) (s, buf, hdr->length) != (long) hdr->length) // FIXME the cast here is correct, but it would be nice to see if we research futher along to get rid of it
	{
		lserrno = LSE_MSG_SYS;
		return -2;
	}

	if (auth)
	{
		if (!xdr_lsfAuth (xdrs, auth, hdr))
		{
			lserrno = LSE_BAD_XDR;
			return -1;
		}
	}

	if (!(*xdrFunc) (xdrs, data, hdr))
	{
		lserrno = LSE_BAD_XDR;
		return -1;
	}

  return 0;
}


// FIXME FIXME size_t len , size_t (*writeFunc)
int
writeEncodeMsg_ (int s, char *buf, unsigned int len, struct LSFHeader *hdr, char *data, long (*writeFunc) (), bool_t (*xdrFunc) (), int options)
{
  XDR xdrs;

  xdrmem_create (&xdrs, buf, len, XDR_ENCODE);

  if (!xdr_encodeMsg (&xdrs, data, hdr, xdrFunc, options, NULL))
	{
	  lserrno = LSE_BAD_XDR;
	  xdr_destroy (&xdrs);
	  return -1;
	}

  if ((*writeFunc) (s, buf, XDR_GETPOS (&xdrs)) != XDR_GETPOS (&xdrs))
	{
	  lserrno = LSE_MSG_SYS;
	  xdr_destroy (&xdrs);
	  return -2;
	}

  xdr_destroy (&xdrs);

  return 0;
}

int
writeEncodeHdr_ (int s, struct LSFHeader *hdr, long (*writeFunc) ())
{
  XDR xdrs;
  struct LSFHeader buf;

  initLSFHeader_ (&buf);
  hdr->length = 0;
  xdrmem_create (&xdrs, (char *) &buf, LSF_HEADER_LEN, XDR_ENCODE);

  if (!xdr_LSFHeader (&xdrs, hdr))
	{
	  lserrno = LSE_BAD_XDR;
	  xdr_destroy (&xdrs);
	  return -1;
	}

  xdr_destroy (&xdrs);

// FIXME FIXME FIXME try tofind a better way to write the struct to the function..... (void *), maybe?

  if ((*writeFunc) (s, (char *) &buf, LSF_HEADER_LEN) != LSF_HEADER_LEN)
	{
	  lserrno = LSE_MSG_SYS;
	  return -2;
	}

  return 0;
}


bool_t
xdr_stringLen (XDR * xdrs, struct stringLen * str, struct LSFHeader *hdr)
{
	assert( hdr->length);
	if (xdrs->x_op == XDR_DECODE) {
		str->name[0] = '\0';
	}

	assert( str->len <= UINT_MAX);
	if (!xdr_string (xdrs, &str->name, str->len)) {
		return FALSE;
	}

  return TRUE;
}

bool_t
xdr_lsfLimit (XDR * xdrs, struct lsfLimit * limits, struct LSFHeader *hdr)
{
	assert( hdr->length);

  if (!(xdr_u_int (xdrs, (unsigned int *) &limits->rlim_curl) &&
  xdr_u_int (xdrs, (unsigned int *) &limits->rlim_curh) &&
  xdr_u_int (xdrs, (unsigned int *) &limits->rlim_maxl) &&
  xdr_u_int (xdrs, (unsigned int *) &limits->rlim_maxh)))
	return FALSE;
  return TRUE;
}

bool_t
xdr_portno (XDR * xdrs, u_short * portno)
{
	uint32_t len = 2;
	char *sp;

 	if (xdrs->x_op == XDR_DECODE) {
		*portno = 0;
 	}

	sp = (char *) portno; // FIXME FIXME FIXME now to sure about this cast

	return xdr_bytes( xdrs, &sp, &len, len );
}


bool_t
xdr_address (XDR * xdrs, u_int * addr)
{
	uint32_t len = NET_INTSIZE_;
	char *sp;

	if (xdrs->x_op == XDR_DECODE) {
		*addr = 0;
	}

	sp = (char *) addr; // FIXME FIXME FIXME now to sure about this cast

	return xdr_bytes( xdrs, &sp, &len, len );
}



bool_t
xdr_debugReq (XDR * xdrs, struct debugReq * debugReq, struct LSFHeader *hdr)
{
  static char *sp = NULL;
  static char *phostname = NULL;

	assert( hdr->length);
  sp = debugReq->logFileName;

  if (xdrs->x_op == XDR_DECODE)
	{
	  debugReq->logFileName[0] = '\0';

	  if (phostname == NULL)
  {
	phostname = (char *) malloc (MAXHOSTNAMELEN);
	if (phostname == NULL)
	  return FALSE;
  }
	  debugReq->hostName = phostname;
	  phostname[0] = '\0';
	}
  else
	phostname = debugReq->hostName;

  if (!(xdr_int (xdrs, &debugReq->opCode)
  && xdr_int (xdrs, &debugReq->level)
  && xdr_int (xdrs, &debugReq->logClass)
  && xdr_int (xdrs, &debugReq->options)
  && xdr_string (xdrs, &phostname, MAXHOSTNAMELEN)
  && xdr_string (xdrs, &sp, MAXPATHLEN)))
	return FALSE;

  return TRUE;
}

void xdr_lsffree (bool_t (*xdr_func) (), char *objp, struct LSFHeader *hdr)
{

  XDR xdrs;

  xdrmem_create (&xdrs, NULL, 0, XDR_FREE);

  (*xdr_func) (&xdrs, objp, hdr);

  xdr_destroy (&xdrs);
}

int getXdrStrlen (char *s)
{
  unsigned int cc = 0;

  if (s == NULL)
	return 4;

  cc = ALIGNWORD_ (strlen (s) + 1);

  return (int) cc;
}
