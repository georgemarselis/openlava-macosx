/* $Id: lib.xdrres.c 397 2007-11-26 19:04:00Z mblack $
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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/rf.h"
#include "lib/xdrres.h"
#include "lib/xdrmisc.h"
#include "lib/term.h"
#include "lib/misc.h"

bool_t
xdr_resCmdBill (XDR *xdrs, struct resCmdBill *cmd, struct LSFHeader *hdr)
{
    char *sp = NULL;
    unsigned int argc = 0;
    int nlimits = 0;

    if (!(xdr_portno (xdrs, &cmd->retport) &&
          xdr_int (xdrs, &cmd->rpid) &&
          xdr_u_int (xdrs, &cmd->filemask) &&
          xdr_int (xdrs, &cmd->priority) && xdr_int (xdrs, &cmd->options)))
    {
        return FALSE;
    }

    sp = cmd->cwd;
    if (xdrs->x_op == XDR_DECODE) {
        sp = NULL;
    }

    if (!xdr_string (xdrs, &sp, MAX_PATH_LEN)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_ENCODE) {
        for (argc = 0; cmd->argv[argc]; argc++);
    }

    if (!xdr_u_int (xdrs, &argc)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        if ((cmd->argv = calloc ( argc + 1, sizeof (char *) ) ) == NULL)
            return FALSE;
    }

    for ( unsigned int i = 0; i < argc; i++) {

        if (!xdr_var_string (xdrs, &(cmd->argv[i]))) {

            if (xdrs->x_op == XDR_DECODE) {

                while (i--) {
                    free (cmd->argv[i]);
                }
                free (cmd->argv);
            }
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_DECODE) {
        cmd->argv[argc] = NULL;
    }
    else {
        nlimits = LSF_RLIM_NLIMITS;
    }

    if (!xdr_int (xdrs, &nlimits)) {
        if (xdrs->x_op == XDR_DECODE) {
            // goto DecodeQuit;
            for ( unsigned int i = 0; i < argc; i++) {
                free (cmd->argv[i]);
            }
            free (cmd->argv);
            return FALSE;
        }
        else {
            return FALSE;
        }
    }

    if (xdrs->x_op == XDR_DECODE) {
        nlimits = (nlimits < LSF_RLIM_NLIMITS) ? nlimits : LSF_RLIM_NLIMITS;
    }

    for (int i = 0; i < nlimits; i++) {

        if (!xdr_arrayElement (xdrs, (char *) &(cmd->lsfLimits[i]), hdr, xdr_lsfLimit)) {
            if (xdrs->x_op == XDR_DECODE) {
                // goto DecodeQuit;
                for ( unsigned int i = 0; i < argc; i++) {
                    free (cmd->argv[i]);
                }
                free (cmd->argv);
                return FALSE;
            }
            else {
                return FALSE;
            }
        }
    }

    return TRUE;

// DecodeQuit:
//     for ( int i = 0; i < argc; i++) {
//         free (cmd->argv[i]);
//     }
//     free (cmd->argv);
//     return FALSE;
}


bool_t
xdr_resSetenv (XDR *xdrs, struct resSetenv *envp, struct LSFHeader *hdr)
{
    char *sp = NULL;
    unsigned int nenv = 0;

    assert( hdr->length );

    if (xdrs->x_op == XDR_ENCODE) {
        for (nenv = 0; envp->env[nenv]; nenv++){
            ;
        }
    }

    if (!xdr_u_int (xdrs, &nenv)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_DECODE) {
        if ((envp->env = calloc ( nenv + 1, sizeof (char *))) == NULL) {
            return FALSE;
        }
    }

    for (unsigned int i = 0; i < nenv; i++) {
        if (xdrs->x_op == XDR_DECODE) {
            if (!xdr_var_string (xdrs, &envp->env[i])) {
                while (i--) {
                    free (envp->env[i]);
                }
                free (envp->env);
                return FALSE;
            }
        }
        else {

            sp = envp->env[i];
            if (strncmp (sp, "DISPLAY=", 8) == 0) {
                sp = chDisplay_ (sp);
            }
            if (!xdr_var_string (xdrs, &sp)) {
                return FALSE;
            }
        }
    }

    if (xdrs->x_op == XDR_DECODE) {
        envp->env[nenv] = NULL;
    }

    return TRUE;
}

bool_t
xdr_resRKill (XDR *xdrs, struct resRKill *rkill, struct LSFHeader *hdr)
{
    assert( hdr->length );
    if (!(xdr_u_long (xdrs, &rkill->rtaskid) && xdr_int (xdrs, &rkill->whatid) && xdr_int (xdrs, &rkill->signal))) {
        return FALSE;
    }

    return TRUE;
}

bool_t
xdr_resGetpid (XDR *xdrs, struct resPid *pidreq, struct LSFHeader *hdr)
{
    assert( hdr->length );
    // if (!(xdr_int (xdrs, &pidreq->rtaskid) && xdr_int (xdrs, &pidreq->pid))) {
    if ( !xdr_u_long (xdrs, &pidreq->rtaskid) ) {
        return FALSE;
    }

    return TRUE;
}

bool_t
xdr_resGetRusage (XDR *xdrs, struct resRusage *rusageReq, struct LSFHeader *hdr)
{
    assert( hdr->length );
    if (!(xdr_int (xdrs, &rusageReq->rtaskid)
          && xdr_int (xdrs, &rusageReq->whatid)
          && xdr_int (xdrs, &rusageReq->options)))
    {
        return FALSE;
    }

    return TRUE;
}


bool_t
xdr_resChdir (XDR *xdrs, struct resChdir *ch, struct LSFHeader *hdr)
{
    char *sp = ch->dir;

    assert( hdr->length );
    if (xdrs->x_op == XDR_DECODE) {
        sp = NULL;
    }

    if (!xdr_string (xdrs, &sp, MAX_FILENAME_LEN)) {
        return FALSE;
    }


    return TRUE;
}


bool_t
xdr_resControl (XDR *xdrs, struct resControl *ctrl, struct LSFHeader *hdr)
{
    assert( hdr->length );
    if (!(xdr_int (xdrs, &ctrl->opCode) && xdr_int (xdrs, &ctrl->data))) {
        return FALSE;
    }
    return TRUE;
}


bool_t
xdr_resStty (XDR *xdrs, struct resStty *tty, struct LSFHeader *hdr)
{
    assert( hdr->length );
    if (xdrs->x_op == XDR_ENCODE) {
        if (!encodeTermios_ (xdrs, &tty->termattr)) {
            return FALSE;
        }
    }
    else {
        if (!decodeTermios_ (xdrs, &tty->termattr)) {
            return FALSE;
        }
    }


    if (!(xdr_u_short (xdrs, &tty->ws.ws_row) &&
          xdr_u_short (xdrs, &tty->ws.ws_col) &&
          xdr_u_short (xdrs, &tty->ws.ws_xpixel) &&
          xdr_u_short (xdrs, &tty->ws.ws_ypixel)))
    {
        return FALSE;
    }

    return TRUE;
}

bool_t
xdr_ropenReq (XDR *xdrs, struct ropenReq *req, struct LSFHeader *hdr)
{
    int flags           = 0;
    char *sp            = req->fn;
    size_t length       = 0;


    assert( hdr->length );
    if (xdrs->x_op == XDR_DECODE) {
        length = MAX_FILENAME_LEN;
        sp = NULL;
    }
    else {
        assert( strlen (req->fn) + 1 <= UINT_MAX );
        length = strlen (req->fn) + 1;
    }

    assert( length <= UINT_MAX );
    if (!xdr_string (xdrs, &sp, (unsigned int) length)) {
        return FALSE;
    }

    if (xdrs->x_op == XDR_ENCODE) {
        flags = 0;
        if (req->flags & O_WRONLY) {
            flags |= LSF_O_WRONLY;
        }
        if (req->flags & O_RDWR) {
            flags |= LSF_O_RDWR;
        }
#ifdef O_NDELAY
        if (req->flags & O_NDELAY) {
            flags |= LSF_O_NDELAY;
        }
#endif
#ifdef O_NONBLOCK // FIXME FIXME FIXME add to configure.ac
        if (req->flags & O_NONBLOCK) {
            flags |= LSF_O_NONBLOCK;
        }
#endif
        if (req->flags & O_APPEND) {
            flags |= LSF_O_APPEND;
        }
        if (req->flags & O_CREAT) {
            flags |= LSF_O_CREAT;
        }
        if (req->flags & O_TRUNC) {
            flags |= LSF_O_TRUNC;
        }
        if (req->flags & O_EXCL) {
            flags |= LSF_O_EXCL;
        }
#ifdef O_NOCTTY // FIXME FIXME FIXME add to configure.ac
        if (req->flags & O_NOCTTY) {
            flags |= LSF_O_NOCTTY;
        }
#endif
        if (req->flags & LSF_O_CREAT_DIR) {
            flags |= LSF_O_CREAT_DIR;
        }
        if (!xdr_int (xdrs, &flags)) {
            return FALSE;
        }
    }
    else {
        if (!xdr_int (xdrs, &flags)) {
            return FALSE;
        }
        req->flags = 0;
        if (flags & LSF_O_WRONLY) {
            req->flags |= O_WRONLY;
        }
        if (flags & LSF_O_RDWR) {
            req->flags |= O_RDWR;
        }
#ifdef O_NDELAY // FIXME FIXME FIXME add to configure.ac
        if (flags & LSF_O_NDELAY) {
            req->flags |= O_NDELAY;
        }
#endif
#ifdef O_NONBLOCK // FIXME FIXME FIXME add to configure.ac
        if (flags & LSF_O_NONBLOCK) {
            req->flags |= O_NONBLOCK;
        }
#endif
        if (flags & LSF_O_APPEND) {
            req->flags |= O_APPEND;
        }
        if (flags & LSF_O_CREAT) {
            req->flags |= O_CREAT;
        }
        if (flags & LSF_O_TRUNC) {
            req->flags |= O_TRUNC;
        }
        if (flags & LSF_O_EXCL) {
            req->flags |= O_EXCL;
        }
#ifdef O_NOCTTY // FIXME FIXME FIXME add to configure.ac
        if (flags & LSF_O_NOCTTY) {
            req->flags |= O_NOCTTY;
        }
#endif
        if (flags & LSF_O_CREAT_DIR) {
            req->flags |= LSF_O_CREAT_DIR;
        }
    }
    
    if (!xdr_u_int (xdrs, &req->mode)) {
        return FALSE;
    }
    
    return TRUE;
}

bool_t
xdr_rrdwrReq (XDR *xdrs, struct rrdwrReq *req, struct LSFHeader *hdr)
{
    assert( hdr->length );
    assert( req->length <= INT_MAX );
    return xdr_int (xdrs, &req->fd) && xdr_u_long (xdrs, &req->length);
}

bool_t
xdr_rlseekReq (XDR *xdrs, struct rlseekReq *req, struct LSFHeader *hdr)
{
    int whence = 0;

    assert( hdr->length );
    
    if (xdrs->x_op == XDR_ENCODE) {
        if (req->whence & SEEK_SET) {
            whence |= LSF_SEEK_SET; // FIXME FIXME FIXME why will this never get executed?
        }
        if (req->whence & SEEK_CUR) {
            whence |= LSF_SEEK_CUR; // enum LSF_SEEK in lib/xdrres.h
        }
        if (req->whence & SEEK_END) {
            whence |= LSF_SEEK_END;
        }
    }

    assert( req->offset <= INT_MAX );
    if (!(xdr_int (xdrs, &req->fd) && xdr_int (xdrs, &whence) && xdr_long (xdrs, &req->offset))) {
        return FALSE;
    }
    
    if (xdrs->x_op == XDR_DECODE) {
        req->whence = 0;
        if (whence & LSF_SEEK_SET) {
            whence |= SEEK_SET;
        }
        if (whence & LSF_SEEK_CUR) {
            req->whence |= SEEK_CUR;
        }
        if (whence & LSF_SEEK_END) {
            req->whence |= SEEK_END;
        }
    }
    
    return TRUE;
}

bool_t
xdr_noxdr (XDR *xdrs, size_t size, struct LSFHeader *hdr)
{
    assert( hdr->length );
    assert( size + LSF_HEADER_LEN <= INT_MAX );
    // (*(xdrs)->x_ops->x_setpostn)(xdrs, (unsigned int)(size + LSF_HEADER_LEN));
    (*(xdrs)->x_ops->x_setpostn)(xdrs, (unsigned int) (size + LSF_HEADER_LEN)); // NOFIX, XDR wants to keep allocations small incase remote system runs out of memory
    return TRUE;
}
    