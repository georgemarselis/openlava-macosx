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


#define EAUTH_ENV_BUF_LEN       (MAXPATHLEN+32)

int putEauthClientEnvVar( char *client );
int putEauthServerEnvVar( char *server );
int verifyEAuth_        ( struct lsfAuth *auth, struct sockaddr_in *from );
static int getEAuth     ( struct eauth *, char * );
static int putEnvVar    ( char *buf, const char *envVar, const char *envValue );
static char *getLSFAdmin( void );
static char *getLSFAdmin( void );
