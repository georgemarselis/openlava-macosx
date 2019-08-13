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
// #include <rpc/types.h>
// #include <rpc/xdr.h>

// #include "lib/lib.h"
#include "lib/xdr.h"
#include "lib/xdrmisc.h"
#include "lib/lproto.h"
#include "lib/misc.h"

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
    *word1 = (pid_t) header->refCode; // FIXME FIXME FIXME FIXME this whole thing smells like generic function
    *word1 <<= 16;
    *word1 = *word1 | (header->opCode & 0x0000FFFF);
    *word2 = header->length;
    *word3 = header->version;
    *word3 <<= 16;
    *word3 = *word3 | (header->reserved & 0x0000FFFF);
    *word4 = header->reserved0;

    return;
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

    if (xdrs->x_op == XDR_ENCODE) {
        encodeHdr (&word1, &word2, &word3, &word4, header);
    }

    if (!xdr_int (xdrs, &word1) || !xdr_u_long (xdrs, &word2) || !xdr_u_int (xdrs, &word3) || !xdr_u_int (xdrs, &word4)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        header->refCode  = (unsigned int) word1 >> 16; // FIXME FIXME FIXME into the debugger you go, search for decoding struct and/or man 1 rpcgen
        header->opCode   = (uint16_t) word1 & 0xFFFF;
        header->length   = word2;
        header->version  = word3 >> 16;
        header->reserved = word3 & 0xFFFF;
        assert( word4 <= USHRT_MAX );
        header->reserved0 = (unsigned short) word4;
    }

    return TRUE;
}


bool_t
xdr_packLSFHeader ( char *buf, struct LSFHeader *header)
{
    XDR xdrs;
    char hdrBuf[LSF_HEADER_LEN]; // LSF_HEADER_LEN is in header.h as a #define

    memset( hdrBuf, '\0', strlen( hdrBuf ) );

    xdrmem_create (&xdrs, hdrBuf, LSF_HEADER_LEN, XDR_ENCODE);

    if (!xdr_LSFHeader (&xdrs, header)) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        return FALSE;
    }

    memcpy (buf, hdrBuf, XDR_GETPOS (&xdrs));
    xdr_destroy (&xdrs);

    return TRUE;
}

bool_t
xdr_encodeMsg (XDR * xdrs, const char *data, struct LSFHeader *hdr, bool_t (*xdr_func) (), int options, struct lsfAuth *auth)
{
    unsigned int length = 0;

    assert( options );
    XDR_SETPOS (xdrs, LSF_HEADER_LEN);

    hdr->version = OPENLAVA_VERSION;

    if (auth) {
        if (!xdr_lsfAuth (xdrs, auth, hdr)){
            return FALSE;
        }
    }

    if (data) {
        if (!(*xdr_func) (xdrs, data, hdr)){
            return FALSE;
        }
    }

    length = XDR_GETPOS (xdrs);
    hdr->length = length - LSF_HEADER_LEN;

    XDR_SETPOS (xdrs, 0);
    if (!xdr_LSFHeader (xdrs, hdr)){
        return FALSE;
    }

    XDR_SETPOS (xdrs, length);
    return TRUE;
}

bool_t
xdr_arrayElement (XDR * xdrs, const char *data, struct LSFHeader * hdr, bool_t (*xdr_func) (), ...)
{
    va_list ap;
    unsigned int *nextElementOffset = 0; 
    unsigned int pos = 0;
    char *cp = NULL;

    va_start (ap, xdr_func);

    pos = XDR_GETPOS (xdrs);

    if (xdrs->x_op == XDR_ENCODE) {
        XDR_SETPOS (xdrs, pos + NET_INTSIZE_);
    }
    else {
        assert( *nextElementOffset <= INT_MAX );
        if (!xdr_u_int (xdrs, nextElementOffset)) {
                return FALSE;
        }
    }

    cp = va_arg (ap, char *);
    if (cp) {
        if (!(*xdr_func) (xdrs, data, hdr, cp)) {
            return FALSE;
        }
    }
    else {
        if (!(*xdr_func) (xdrs, data, hdr)) {
                return FALSE;
        }
    }

    if (xdrs->x_op == XDR_ENCODE) {
        *nextElementOffset = XDR_GETPOS (xdrs) - pos;
        XDR_SETPOS (xdrs, pos);
        assert( *nextElementOffset <= INT_MAX );
            if (!xdr_u_int (xdrs, nextElementOffset)) { // FIXME FIXME FIXME FIXME we got to revisit this
                    return FALSE;
            }
    }

    XDR_SETPOS (xdrs, (pos + *nextElementOffset));

    return TRUE;
}

bool_t
xdr_array_string (XDR * xdrs, char **astring, unsigned int maxlen, unsigned int arraysize)
{

    char line[MAX_LINE_LEN];
    char *sp = line;

    memset( line, '\0', strlen( line ) );

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
// #ifdef __LINUX__
        return xdr_long( xdrs, t );
// #elif defined(__APPLE__)
//         return xdr_long( xdrs, (int *)t ); // FIXME FIXME FIXME FIXME we got to revisit this
// #else
//         #error
// #endif
}

int
readDecodeHdr_ (int s, const char *buf, long (*readFunc) (), XDR * xdrs, struct LSFHeader *hdr)
{
    if ((*readFunc) (s, buf, LSF_HEADER_LEN) != LSF_HEADER_LEN) {
        lserrno = LSE_MSG_SYS;
        return -2; // FIXME FIXME FIXME replace with *possitive* meaningful number
    }

    if (!xdr_LSFHeader (xdrs, hdr)) {
        lserrno = LSE_BAD_XDR;
        return -1; // FIXME FIXME FIXME replace with *possitive* meaningful number
    }

    return 0;
}

int
readDecodeMsg_ (int s, const char *buf, struct LSFHeader *hdr, long (*readFunc) (),  XDR * xdrs, const char *data, bool_t (*xdrFunc) (), struct lsfAuth *auth)
{
    assert( hdr->length <= LONG_MAX);
    if ((*readFunc) (s, buf, hdr->length) != (long) hdr->length) { // FIXME the cast here is correct, but it would be nice to see if we research futher along to get rid of it
        lserrno = LSE_MSG_SYS;
        return -2; // FIXME FIXME FIXME replace with *possitive* meaningful number
    }

    if (auth) {
        if (!xdr_lsfAuth (xdrs, auth, hdr)) {
            lserrno = LSE_BAD_XDR;
            return -1; // FIXME FIXME FIXME replace with *possitive* meaningful number
        }
    }

    if (!(*xdrFunc) (xdrs, data, hdr)) {
        lserrno = LSE_BAD_XDR;
        return -1; // FIXME FIXME FIXME replace with *possitive* meaningful number
    }

    return 0;
}


// FIXME FIXME size_t length , size_t (*writeFunc)
int
writeEncodeMsg_ (int s, const char *buf, unsigned int length, struct LSFHeader *hdr, const char *data, long (*writeFunc) (), bool_t (*xdrFunc) (), int options)
{
    XDR xdrs;

    xdrmem_create (&xdrs, strdup(buf), length, XDR_ENCODE);

    if (!xdr_encodeMsg (&xdrs, data, hdr, xdrFunc, options, NULL)) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        return -1; // FIXME FIXME FIXME replace with *possitive* meaningful number
    }

    if ((*writeFunc) (s, buf, XDR_GETPOS (&xdrs)) != XDR_GETPOS (&xdrs)) {
        lserrno = LSE_MSG_SYS;
        xdr_destroy (&xdrs);
        return -2; // FIXME FIXME FIXME replace with *possitive* meaningful number
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

    if (!xdr_LSFHeader (&xdrs, hdr)) {
        lserrno = LSE_BAD_XDR;
        xdr_destroy (&xdrs);
        return -1; // FIXME FIXME FIXME replace with *possitive* meaningful number
    }

    xdr_destroy (&xdrs);

// FIXME FIXME FIXME try tofind a better way to write the struct to the function..... (void *), maybe?

    if ((*writeFunc) (s, (char *) &buf, LSF_HEADER_LEN) != LSF_HEADER_LEN) {
        lserrno = LSE_MSG_SYS;
        return -2; // FIXME FIXME FIXME replace with *possitive* meaningful number
    }

    return 0;
}


bool_t
xdr_stringLen (XDR * xdrs, struct stringLen *str, struct LSFHeader *hdr)
{

    char *name = strdup(str->name);

    assert( hdr->length);
    if (xdrs->x_op == XDR_DECODE) {
        str->name = NULL;
    }

    assert( str->length <= UINT_MAX);
    if (!xdr_string (xdrs, &name, (unsigned int) str->length)) { // NOFIX cast is fine
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
    xdr_u_int (xdrs, (unsigned int *) &limits->rlim_maxh))) {
        return FALSE;
    }

    return TRUE;
}

bool_t
xdr_portno (XDR * xdrs, uint16_t *portno)
{
    uint32_t length = 2;
    char *sp =  malloc( 32* sizeof( char ) + 1);
    memset( sp, '\0', strlen( sp ) );

    if (xdrs->x_op == XDR_DECODE) {
        *portno = 0;
    }

    sprintf( sp, "%u", *portno ); // turn the address into a string

    return xdr_bytes( xdrs, &sp, &length, length );
}


bool_t
xdr_address (XDR * xdrs, u_int * addr)
{
    uint32_t length = NET_INTSIZE_;
    char *sp =  malloc( 32* sizeof( char ) + 1);
    memset( sp, '\0', strlen( sp ) );

    if (xdrs->x_op == XDR_DECODE) {
        *addr = 0;
    }

    sprintf( sp, "%u", *addr ); // turn the address into a string

    return xdr_bytes( xdrs, &sp, &length, length );
}



bool_t
xdr_debugReq (XDR * xdrs, struct debugReq * debugReq, struct LSFHeader *hdr)
{
    static char *sp = NULL;
    static char *phostname = NULL;

    assert( hdr->length);
    sp = debugReq->logFileName;

    if (xdrs->x_op == XDR_DECODE) {
        debugReq->logFileName = NULL;

        if (phostname == NULL) {
            phostname = malloc (MAXHOSTNAMELEN *sizeof(char) );
            if (phostname == NULL) {
                return FALSE;
            }
        }
        strcpy( debugReq->hostName, phostname );
        phostname = NULL;
    }
    else {
        phostname = debugReq->hostName;
    }

    if (!(xdr_int (xdrs, &debugReq->opCode)
    && xdr_int (xdrs, &debugReq->level)
    && xdr_int (xdrs, &debugReq->logClass)
    && xdr_int (xdrs, &debugReq->options)
    && xdr_string (xdrs, &phostname, MAXHOSTNAMELEN)
    && xdr_string (xdrs, &sp, MAX_PATH_LEN))) {
        return FALSE;
    }

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
    size_t cc = 0;

    if (s == NULL) {
        return 4;
    }

    // cc = ALIGNWORD_ (strlen (s) + 1);
    cc = strlen( s ) + 1;

    return (int) cc;
}
