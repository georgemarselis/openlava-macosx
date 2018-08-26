/* $Id: lib.dir.c 397 2007-11-26 19:04:00Z mblack $
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

 #pragma once

static struct hTab hashTab;

char chosenPath[MAX_PATH_LEN];

// int putin_ (unsigned long status, char *inkey, int inkeylen, char *inval, int invallen, void *indata);
int putin_( int instatus, char *inkey, int inkeylen, char *inval, int invallen, char *indata);
int getMap_( void );

static int tryPwd (char *path, char *pwdpath);
static int netHostChdir (char *, struct hostent *);
static char *mountNet_ (struct hostent *);
static char *usePath ( const char *);
