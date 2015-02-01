/* $Id: lib.xdrmisc.c 397 2007-11-26 19:04:00Z mblack $
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

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/xdr.h"

bool_t my_xdr_float( XDR *xdrs, float *fp );
bool_t xdr_pidInfo (XDR *xdrs, struct pidInfo *pidInfo, struct LSFHeader *hdr);


bool_t
xdr_LSFlong (XDR * xdrs, long *l)
{
    struct {
        long high;
        long low;
    } longNum;
    
    if (xdrs->x_op == XDR_ENCODE ) {
#if (LONG_BIT == 64)
        longNum.high = *l >> 32;
        longNum.low = *l & 0x00000000FFFFFFFF;
#else
        longNum.high = (*l < 0) ? -1 : 0;
        longNum.low = *l;
#endif
    }

    assert( longNum.high <= INT_MAX);
    assert ( longNum.low <= INT_MAX);
    if (!(xdr_int (xdrs, (int *) &longNum.high) && xdr_int (xdrs, (int *) &longNum.low))) {
        return (FALSE);
    }
    
    if (xdrs->x_op == XDR_ENCODE) {
        return (TRUE);
    }
    
#if (LONG_BIT == 64)
    *l = ((long) longNum.high) << 32;
    *l = *l | ((long) longNum.low & 0x00000000FFFFFFFF);
#else
    if (longNum.high > 0 || (longNum.high == 0 && (longNum.low & 0x80000000))) {
        *l = INT_MAX;
    }
    else if (longNum.high < -1 || (longNum.high == -1 && !(longNum.low & 0x80000000))) {
        *l = INT_MIN;
    }
    else {
        *l = longNum.low;
    }
#endif
    
    return (TRUE);
}
 
bool_t
xdr_stat (XDR *xdrs, struct stat *st, struct LSFHeader *hdr)
{
    int i = 0;

    assert( hdr->length );

    if (xdrs->x_op == XDR_ENCODE) {
        i = st->st_dev;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_dev = i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        assert( st->st_ino <= INT_MAX );
        i = (int)st->st_ino;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        assert( i >= 0 );
        st->st_ino = (__darwin_ino64_t ) i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        assert( i >= 0 );
        i = (mode_t) st->st_mode;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        assert( i >= 0);
        st->st_mode = (mode_t) i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        i = st->st_nlink;
    }

    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        assert( i >=0 && i <= SHRT_MAX);
        st->st_nlink = (nlink_t) i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        assert( st->st_uid <= INT_MAX);
        i = (int) st->st_uid;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        assert( i >= 0 );
        st->st_uid = (uid_t) i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        assert( st->st_gid <= INT_MAX );
        i = (int) st->st_gid;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        assert( i > 0 );
        st->st_gid = (gid_t) i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        i = st->st_rdev;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_rdev = i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        assert( st->st_size <= INT_MAX );
        i = (int) st->st_size;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_size = i;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        assert( st->st_atime <= INT_MAX && st->st_atime >= INT_MIN);
        i = (int) st->st_atime;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_atime = i;
    }
    
    if (xdrs->x_op == XDR_ENCODE) {
        assert( st->st_mtime <= INT_MAX && st->st_mtime >= INT_MIN );
        i = (int) st->st_mtime;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_mtime = i;
    }

    if (xdrs->x_op == XDR_ENCODE) {
        assert( st->st_ctime <= INT_MAX && st->st_ctime >= INT_MIN );
        i = (int) st->st_ctime;
    }
    if (!xdr_int (xdrs, &i)) {
        return (FALSE);
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_ctime = i;
    }

    return (TRUE);
 }
 
bool_t
xdr_lsfRusage (XDR *xdrs, struct lsfRusage *lsfRu)
{
    assert( lsfRu->ru_utime <= __DBL_MAX__ && lsfRu->ru_utime >= __DBL_MIN__ );
    if( !(xdr_double (xdrs, (double *) &lsfRu->ru_utime)    &&
          xdr_double (xdrs, (double *)&lsfRu->ru_stime)     &&  // FIXME FIXME FIXME  read up about allignment
          xdr_double (xdrs, (double *)&lsfRu->ru_maxrss)    &&
          xdr_double (xdrs, (double *)&lsfRu->ru_ixrss)     &&
          xdr_double (xdrs, &lsfRu->ru_ismrss)              &&
          xdr_double (xdrs, (double *)&lsfRu->ru_idrss)     &&
          xdr_double (xdrs, (double *)&lsfRu->ru_isrss)     &&
          xdr_double (xdrs, (double *)&lsfRu->ru_minflt)    &&
          xdr_double (xdrs, (double *)&lsfRu->ru_majflt)    &&
          xdr_double (xdrs, (double *)&lsfRu->ru_nswap)     &&
          xdr_double (xdrs, (double *)&lsfRu->ru_inblock)   &&
          xdr_double (xdrs, (double *)&lsfRu->ru_oublock)   &&
          xdr_double (xdrs, &lsfRu->ru_ioch)                &&
          xdr_double (xdrs, (double *)&lsfRu->ru_msgsnd)    &&
          xdr_double (xdrs, (double *)&lsfRu->ru_msgrcv)    &&
          xdr_double (xdrs, (double *)&lsfRu->ru_nsignals)            &&
          xdr_double (xdrs, (double *)&lsfRu->ru_nvcsw)               &&
          xdr_double (xdrs, (double *)&lsfRu->ru_nivcsw)              &&
          xdr_double (xdrs, &lsfRu->ru_exutime)))
    {
        return (FALSE);
    }

    return (TRUE);
}
 
bool_t
xdr_var_string (XDR *xdrs, char **astring)
{
    unsigned int  pos = 0;
    unsigned long len = 0;
    
    if (xdrs->x_op == XDR_FREE) {
        FREEUP (*astring);
        return TRUE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        
        pos = (*(xdrs)->x_ops->x_getpostn)(xdrs);
        *astring = NULL;
        
        assert( len <= INT_MAX);
        if (!xdr_int (xdrs, (int *)&len) || ((*astring = malloc (len + 1)) == NULL)) {
            return FALSE;
        }
        
        XDR_SETPOS (xdrs, pos);
        
    }
    else {
        len = strlen (*astring);
    }

    assert( len + 1 <= INT_MAX );
    if (!xdr_string( xdrs, astring, (unsigned int)(len + 1) )) {

        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (*astring);
        }
        return FALSE;
    }
    
    return TRUE;
 }
 
bool_t
xdr_lenData (XDR *xdrs, struct lenData *ld)
{
    char *sp;

    assert( ld-> len <= INT_MAX );
    if (!xdr_int (xdrs, (int *) &ld->len)) {
        return (FALSE);
    }

    if (xdrs->x_op == XDR_FREE) {
        FREEUP (ld->data);
        return (TRUE);
    }
    
    if (ld->len == 0) {
        ld->data = NULL;
        return (TRUE);
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        ld->data = (char *) malloc (ld->len);
        if ( NULL == ld->data  && ENOMEM == errno ) {
            return (FALSE);
        }
    }
    
    sp = ld->data;
    assert( ld->len <= UINT_MAX );
    if (!xdr_bytes (xdrs, &sp, (u_int *) & ld->len, (unsigned int)ld->len)) {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (ld->data);
        }
        return (FALSE);
    }
    
    return (TRUE);
 }
 
 
bool_t
xdr_lsfAuth (XDR *xdrs, struct lsfAuth *auth, struct LSFHeader *hdr)
{
    
    char *sp;
    
    sp = auth->lsfUserName;
    if (xdrs->x_op == XDR_DECODE) {
        sp[0] = '\0';
    }

    assert( auth->uid <= INT_MAX );
    if (!(xdr_int (xdrs, (int *)&auth->uid) && xdr_int (xdrs, (int *)&auth->gid) && xdr_string (xdrs, &sp, MAXLSFNAMELEN))) {
        return (FALSE);
    }

    if (!xdr_enum (xdrs, (int *) &auth->kind)) {
        return (FALSE);
    }

    switch (auth->kind)
    {
        case CLIENT_DCE:

            assert( auth->k.authToken.len <= INT_MAX );
            if (!xdr_int (xdrs, (int *)&auth->k.authToken.len)) {
                return (FALSE);
            }
            
            if (xdrs->x_op == XDR_DECODE) {
                auth->k.authToken.data = (void *) malloc (auth->k.authToken.len);
                if ( NULL == auth->k.authToken.data && ENOMEM == errno)
                    return (FALSE);
            }

            assert( auth->k.authToken.len <= INT_MAX );
            if (!xdr_bytes (xdrs, (char **) &auth->k.authToken.data, (u_int *) & auth->k.authToken.len, (unsigned int) auth->k.authToken.len)) {
                return (FALSE);
            }
            
            break;
        
        case CLIENT_EAUTH:
            assert( auth->k.authToken.len <= INT_MAX );
            if (!xdr_int (xdrs, (int *) &auth->k.eauth.len)) {
                return (FALSE);
            }
            
            sp = auth->k.eauth.data;
            assert( auth->k.authToken.len <= INT_MAX );
            if (!xdr_bytes (xdrs, &sp, (u_int *) & auth->k.eauth.len, (unsigned int)auth->k.eauth.len)) {
                return (FALSE);
            }
            break;
        case CLIENT_SETUID:
        case CLIENT_IDENT:
        // default:
        
            if (!xdr_arrayElement (xdrs, (char *) &auth->k.filler, hdr, xdr_int)) {
                return (FALSE);
            }
        break;
    }

    if (xdrs->x_op == XDR_ENCODE) {
        auth->options = AUTH_HOST_UX;
    }
    
    if (!xdr_int (xdrs, &auth->options)) {
        return (FALSE);
    }
    
    
    return (TRUE);
}
 
bool_t
my_xdr_float (XDR *xdrs, float *fp)
{
    return (xdr_float (xdrs, fp));
 }
 
int
xdr_lsfAuthSize (struct lsfAuth *auth)
{
    int sz = 0;
    
    if (auth == NULL) {
        return (sz);
    }

    sz +=   ALIGNWORD_ (sizeof (auth->uid)) + ALIGNWORD_ (sizeof (auth->gid)) +
            ALIGNWORD_ (strlen (auth->lsfUserName)) + ALIGNWORD_ (sizeof (auth->kind));
    
    switch (auth->kind)
    {
        case CLIENT_DCE:
            sz += ALIGNWORD_ (sizeof (auth->k.authToken.len)) + ALIGNWORD_ (auth->k.authToken.len);
            break;
        
        case CLIENT_EAUTH:
            sz += ALIGNWORD_ (sizeof (auth->k.eauth.len)) + ALIGNWORD_ (auth->k.eauth.len);
            break;

        case CLIENT_SETUID:
        case CLIENT_IDENT:
        // default:
            sz += ALIGNWORD_ (sizeof (auth->k.filler));
            break;
    }
    sz += ALIGNWORD_ (sizeof (auth->options));
    
    return (sz);
 }
 
bool_t
xdr_pidInfo (XDR *xdrs, struct pidInfo *pidInfo, struct LSFHeader *hdr)
{
    assert( hdr->length >= 0 );
    
     if (!xdr_int (xdrs, &pidInfo->pid)) {
            return FALSE;
     }
     if (!xdr_int (xdrs, &pidInfo->ppid)) {
            return FALSE;
     }
     if (!xdr_int (xdrs, &pidInfo->pgid)) {
            return FALSE;
     }
     if (!xdr_int (xdrs, &pidInfo->jobid)) {
            return FALSE;
     }

    return (TRUE);
 }
 
bool_t
xdr_jRusage (XDR * xdrs, struct jRusage * runRusage, struct LSFHeader * hdr)
{
    if (xdrs->x_op == XDR_FREE)
    {
        FREEUP (runRusage->pidInfo);
        FREEUP (runRusage->pgid);
        return (TRUE);
    }
    
    if (xdrs->x_op == XDR_DECODE)
    {
        runRusage->pidInfo = NULL;
        runRusage->pgid = NULL;
    }
    
    if (!(xdr_int (xdrs, &runRusage->mem)    &&
          xdr_int (xdrs, &runRusage->swap)   &&
          xdr_int (xdrs, &runRusage->utime)  &&
          xdr_int (xdrs, &runRusage->stime))
       )
    {
        return (FALSE);
    }
    
    
    
    if (!(xdr_int (xdrs, &runRusage->npids))) {
        return (FALSE);
    }

    if (xdrs->x_op == XDR_DECODE && runRusage->npids) {

        assert( runRusage->npids >= 0 );
        runRusage->pidInfo = calloc ((unsigned long)runRusage->npids, sizeof (struct pidInfo));
        if ( NULL == runRusage->pidInfo && ENOMEM == errno) {
            runRusage->npids = 0;
            return (FALSE);
        }
    }
    
    for ( int i = 0; i < runRusage->npids; i++) {

        if (!xdr_arrayElement(xdrs, (char *) &(runRusage->pidInfo[i]), hdr, xdr_pidInfo)) {

            if (xdrs->x_op == XDR_DECODE) {

                FREEUP (runRusage->pidInfo);
                runRusage->npids = 0;
                runRusage->pidInfo = NULL;
            }
            return (FALSE);
        }
    }
    
    if (!(xdr_int (xdrs, &runRusage->npgids))) {
        return (FALSE);
    }

    if (xdrs->x_op == XDR_DECODE && runRusage->npgids) {

        assert( runRusage->npgids >= 0 );
        runRusage->pgid = (pid_t *) calloc ( (unsigned long)runRusage->npgids, sizeof (int));
        if ( NULL == runRusage->pgid && ENOMEM == errno ) {
            runRusage->npgids = 0;
            return (FALSE);
        }
    }
    
    for ( int i = 0; i < runRusage->npgids; i++) {
        
        if (!xdr_arrayElement (xdrs, (char *) &(runRusage->pgid[i]), hdr, xdr_int)) {

            if (xdrs->x_op == XDR_DECODE) {

                FREEUP (runRusage->pgid);
                runRusage->npgids = 0;
                runRusage->pgid = NULL;
            }
            return (FALSE);
        }
    }
    return (TRUE);
}
