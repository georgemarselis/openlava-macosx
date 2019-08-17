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

// #ifdef __APPLE__
// #undef __LP64__ 
// #endif

#include <limits.h>
#include <sys/types.h>
#include <time.h>

#include "lib/lib.h"
#include "lib/syslog.h"
#include "lib/xdr.h"
#include "lib/xdrmisc.h"

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
    if (!(xdr_long (xdrs, &longNum.high) && xdr_long (xdrs, &longNum.low))) { // FIXME FIXME FIXME FIXME we got to revisit this
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_ENCODE) {
        return TRUE;
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
    
    return TRUE;
}
 
bool_t
xdr_stat (XDR *xdrs, struct stat *st, struct LSFHeader *hdr)
{
    size_t       st_dev   = 0;
    size_t       st_ino   = 0;
    unsigned int st_mode  = 0;
    size_t       st_nlink = 0;
    unsigned int st_uid   = 0;
    unsigned int st_gid   = 0;
    size_t       st_rdev  = 0;
    ssize_t      st_size  = 0;
    ssize_t         atime = 0;
    ssize_t         mtime = 0;
    ssize_t         ctime = 0;
    // int i = 0;

    assert( hdr->length ); // NOFIX struct LSFHeader *hdr is not used in this function.

    if (xdrs->x_op == XDR_ENCODE) {
        st_dev = st->st_dev;
    }
    if (!xdr_u_long (xdrs, &st_dev)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_dev = st_dev;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        st_ino = st->st_ino;
    }
    if (!xdr_u_long (xdrs, &st_ino)) {
        return FALSE;
    }
    if (xdrs->x_op == st_ino) {
        st->st_ino = st_ino;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        st_mode = st->st_mode;
    }
    if (!xdr_u_int (xdrs, &st_mode)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_mode = st_mode;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        st_nlink = st->st_nlink;
    }

    if (!xdr_u_long (xdrs, &st_nlink)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_nlink = st_nlink;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        st_uid = st->st_uid;
    }
    if (!xdr_u_int (xdrs, &st_uid)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_uid = st_uid;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        st_gid = st->st_gid;
    }
    if (!xdr_u_int (xdrs, &st_gid)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_gid = st_gid;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        st_rdev = st->st_rdev;
    }
    if (!xdr_u_long (xdrs, &st_rdev)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_rdev = st_rdev;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        st_size = st->st_size;
    }
    if (!xdr_long (xdrs, &st_size)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_size = st_size;
    }
    if (xdrs->x_op == XDR_ENCODE) {
        atime = st->st_atime;
    }
    if (!xdr_long (xdrs, &atime)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_atime = atime;
    }
    
    if (xdrs->x_op == XDR_ENCODE) {
        mtime = st->st_mtime;
    }
    if (!xdr_long (xdrs, &mtime)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_mtime = mtime;
    }

    if (xdrs->x_op == XDR_ENCODE) {
        ctime = st->st_ctime;
    }
    if (!xdr_long (xdrs, &ctime)) {
        return FALSE;
    }
    if (xdrs->x_op == XDR_DECODE) {
        st->st_ctime = ctime;
    }

    return TRUE;
 }
 
bool_t
xdr_lsfRusage (XDR *xdrs, struct lsfRusage *lsfRusage )
{

    if( !(xdr_time_t (xdrs, &lsfRusage->ru_utime    ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_time_t (xdrs, &lsfRusage->ru_stime    ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_maxrss   ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_ixrss    ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_double (xdrs, &lsfRusage->ru_ismrss   ) &&
          xdr_long   (xdrs, &lsfRusage->ru_idrss    ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_isrss    ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_minflt   ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_majflt   ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_nswap    ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_inblock  ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_oublock  ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_double (xdrs, &lsfRusage->ru_ioch     ) &&
          xdr_long   (xdrs, &lsfRusage->ru_msgsnd   ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_msgrcv   ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_nsignals ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_nvcsw    ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_nivcsw   ) && // FIXME FIXME FIXME FIXME we got to revisit this
          xdr_long   (xdrs, &lsfRusage->ru_exutime  )
         )
      )
    {
        return FALSE;
    }

    return TRUE;
}
 
bool_t
xdr_var_string (XDR *xdrs, char **astring) // NOFIX
{
    unsigned int  pos = 0;
    unsigned long length = 0;
    
    if (xdrs->x_op == XDR_FREE) {
        FREEUP (*astring);
        return TRUE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        
        pos = (*(xdrs)->x_ops->x_getpostn)(xdrs);
        *astring = NULL; // NOFIX
        
        assert( length <= INT_MAX);
        if (!xdr_u_long (xdrs, &length) || (( astring = malloc (length + 1)) == NULL)) { // FIXME FIXME FIXME FIXME we got to revisit this
            return FALSE;
        }
        
        XDR_SETPOS (xdrs, pos);
        
    }
    else {
        length = strlen (*astring);
    }

    assert( length <= INT_MAX );
    if (!xdr_string( xdrs, astring, (unsigned int)(length) )) {  // FIXME FIXME FIXME FIXME we got to revisit this

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
    char *sp = NULL;

    if (!xdr_u_long (xdrs, &ld->length)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_FREE) {
        FREEUP (ld->data);
        return TRUE;
    }
    
    if (ld->length == 0) {
        ld->data = NULL;
        return TRUE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        ld->data = malloc (ld->length);
        if ( NULL == ld->data  && ENOMEM == errno ) {
            return FALSE;
        }
    }
    
    sp = ld->data;
    assert( ld->length <= UINT_MAX );
    if (!xdr_bytes (xdrs, &sp, (unsigned int *) & ld->length, (unsigned int)ld->length)) {
        if (xdrs->x_op == XDR_DECODE) {
            FREEUP (ld->data);
        }
        return FALSE;
    }
    
    return TRUE;
 }
 
 
bool_t
xdr_lsfAuth (XDR *xdrs, struct lsfAuth *auth, struct LSFHeader *hdr)
{
    
    char *sp = NULL;
    
    sp = auth->lsfUserName;
    if (xdrs->x_op == XDR_DECODE) {
        sp = NULL;
    }

    assert( auth->uid <= INT_MAX );
    if (!(xdr_u_int (xdrs, &auth->uid) && xdr_u_int (xdrs, &auth->gid) && xdr_string (xdrs, &sp, MAX_LSF_NAME_LEN))) {
        return FALSE;
    }

    if (!xdr_enum (xdrs, &auth->kind)) {
        return FALSE;
    }

    switch (auth->kind)
    {
        case CLIENT_DCE:

            assert( auth->k.authToken.length <= INT_MAX );
            if (!xdr_u_long (xdrs, &auth->k.authToken.length)) {
                return FALSE;
            }
            
            if (xdrs->x_op == XDR_DECODE) {
                auth->k.authToken.data = malloc (auth->k.authToken.length);
                if ( NULL == auth->k.authToken.data && ENOMEM == errno)
                    return FALSE;
            }

            assert( auth->k.authToken.length <= INT_MAX );
            if (!xdr_bytes (xdrs, (char **) &auth->k.authToken.data, (unsigned int * ) &auth->k.authToken.length, (unsigned int) auth->k.authToken.length)) { // FIXME FIXME FIXME change the auth token length type, maybe?
                return FALSE;
            }
            
        break;
        
        case CLIENT_EAUTH:
            if (!xdr_u_long (xdrs, &auth->k.eauth.length)) {
                return FALSE;
            }
            
            sp = auth->k.eauth.data;
            assert( auth->k.authToken.length <= INT_MAX );
            if (!xdr_bytes (xdrs, &sp, (unsigned int *) &auth->k.eauth.length, (unsigned int) auth->k.eauth.length ) ) {
                return FALSE;
            }
        break;

        case CLIENT_SETUID:
        case CLIENT_IDENT:
        // default:
        
            // if (!xdr_arrayElement (xdrs, &auth->k.filler, hdr, xdr_int)) {
            if (!xdr_arrayElement (xdrs, &auth->k.eauth.data, hdr, xdr_int)) {
                return FALSE;
            }
        break;

        case CLIENT_NULL:
        default:
            fprintf( stderr,    "%s( ): default: you are not supposed to be here. auth->kind: %s\n", __func__, clientAuthDescription[ auth->kind ] );
            ls_syslog( LOG_ERR, "%s( ): default: you are not supposed to be here. auth->kind: %s\n", __func__, clientAuthDescription[ auth->kind ] );
        break;
    }

    if (xdrs->x_op == XDR_ENCODE) {
        auth->options = AUTH_HOST_UX;
    }
    
    if (!xdr_u_int (xdrs, &auth->options)) {
        return FALSE;
    }
    
    
    return TRUE;
}
 
// bool_t
// my_xdr_float (XDR *xdrs, float *fp)
// {
    // return (xdr_float (xdrs, fp));
// }
 
int
xdr_lsfAuthSize (struct lsfAuth *auth)
{
    int sz = 0;
    
    if (auth == NULL) {
        return (sz);
    }

    sz +=   ALIGNWORD_( sizeof( auth->uid ) )         + ALIGNWORD_( sizeof ( auth->gid ) ) +
            ALIGNWORD_( strlen( auth->lsfUserName ) ) + ALIGNWORD_( sizeof ( auth->kind ) );
    
    switch (auth->kind)
    {
        case CLIENT_DCE:
            sz += ALIGNWORD_ (sizeof (auth->k.authToken.length)) + ALIGNWORD_ (auth->k.authToken.length);
        break;
        
        case CLIENT_EAUTH:
            sz += ALIGNWORD_ (sizeof (auth->k.eauth.length)) + ALIGNWORD_ (auth->k.eauth.length);
        break;

        case CLIENT_SETUID:
        case CLIENT_IDENT:
        // default:
            sz += ALIGNWORD_ (sizeof (auth->k.filler));
        break;

        case CLIENT_NULL:
        default:
            fprintf( stderr,    "%s( ): default: you are not supposed to be here. auth->kind: %s\n", __func__, clientAuthDescription[ auth->kind ] );
            ls_syslog( LOG_ERR, "%s( ): default: you are not supposed to be here. auth->kind: %s\n", __func__, clientAuthDescription[ auth->kind ] );
        break;
    }
    sz += ALIGNWORD_ (sizeof (auth->options));
    
    return (sz);
 }
 
bool_t
xdr_pidInfo (XDR *xdrs, struct pidInfo *pidInfo, struct LSFHeader *hdr)
{
    assert( hdr->length );
    
    if (!xdr_int (xdrs, &pidInfo->pid)) {
        return FALSE;
    }
    if (!xdr_int (xdrs, &pidInfo->ppid)) {
        return FALSE;
    }
    if (!xdr_int (xdrs, &pidInfo->pgid)) {
        return FALSE;
    }
    if (!xdr_u_long (xdrs, &pidInfo->jobid)) {
        return FALSE;
    }

    return TRUE;
 }
 
bool_t
xdr_jRusage (XDR *xdrs, struct jRusage *runRusage, struct LSFHeader *hdr)
{
    if (xdrs->x_op == XDR_FREE)
    {
        FREEUP (runRusage->pidInfo);
        FREEUP (runRusage->pgid);
        return TRUE;
    }
    
    if (xdrs->x_op == XDR_DECODE)
    {
        runRusage->pidInfo = NULL;
        runRusage->pgid = NULL;
    }
    
    if (!(xdr_u_long (xdrs, &runRusage->mem)    &&
          xdr_u_long (xdrs, &runRusage->swap)   &&
          xdr_long   (xdrs, &runRusage->utime)  &&
          xdr_long   (xdrs, &runRusage->stime))
       )
    {
        return FALSE;
    }
    
    
    
    if (!(xdr_int (xdrs, &runRusage->npids))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE && runRusage->npids) {

        assert( runRusage->npids >= 0 );
        runRusage->pidInfo = calloc( (size_t) runRusage->npids, sizeof (struct pidInfo) );
        if ( NULL == runRusage->pidInfo && ENOMEM == errno) {
            runRusage->npids = 0;
            return FALSE;
        }
    }
    
    for ( pid_t i = 0; i < runRusage->npids; i++) {

        if (!xdr_arrayElement(xdrs, (char *) &(runRusage->pidInfo[i]), hdr, xdr_pidInfo)) {

            if (xdrs->x_op == XDR_DECODE) {

                FREEUP (runRusage->pidInfo);
                runRusage->npids   = 0;
                runRusage->pidInfo = NULL;
            }
            return FALSE;
        }
    }
    
    if (!(xdr_int (xdrs, &runRusage->npgids))) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE && runRusage->npgids) {

        assert( runRusage->npgids >= 0 );
        runRusage->pgid = calloc ( (unsigned long)runRusage->npgids, sizeof (int));
        if ( NULL == runRusage->pgid && ENOMEM == errno ) {
            runRusage->npgids = 0;
            return FALSE;
        }
    }
    
    for ( pid_t i = 0; i < runRusage->npgids; i++) {
        
        if (!xdr_arrayElement (xdrs, (char *) &(runRusage->pgid[i]), hdr, xdr_int)) {

            if (xdrs->x_op == XDR_DECODE) {

                FREEUP (runRusage->pgid);
                runRusage->npgids = 0;
                runRusage->pgid = NULL;
            }
            return FALSE;
        }
    }
    return TRUE;
}
