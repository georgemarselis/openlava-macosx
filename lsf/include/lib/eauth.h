/* $Id: lib.eauth.c 397 2007-11-26 19:04:00Z mblack $
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


// FIXME FIXME FIXME
// WTF is exit() redefined?!
// must investigate 
/* #define exit(a)         _exit(a) 
*/

/* #define NL_SETN   23 */

#pragma once

// #define EAUTH_ENV_BUF_LEN       (MAX_PATH_LEN+32)

const size_t EAUTH_ENV_BUF_LEN = MAX_PATH_LEN + 32;

/* eauth.c */
int   getAuth_lsf(struct lsfAuth *auth, const char *host);
int   getEAuth(struct eauth *eauth, const char *host);
int   verifyEAuth_(struct lsfAuth *auth, struct sockaddr_in *from);
char *getLSFAdmin( void );
int   putEnvVar(char *buf, const char *envVar, const char *envValue);
int   putEauthClientEnvVar( const char *client );
int   putEauthServerEnvVar( const char *server );
