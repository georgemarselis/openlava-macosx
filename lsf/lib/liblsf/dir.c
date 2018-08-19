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

#include <rpcsvc/ypclnt.h>
#include <sys/ioctl.h>
#include <limits.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/dir.h"
#include "lib/misc.h"



char *
usePath( const char *path)
{
	strcpy( chosenPath, path ); 
	return chosenPath;
}

int
mychdir_( char *path, struct hostent *hp )
{
	struct sTab hashSearchPtr;
	struct hEnt *hashEntryPtr = NULL;
	static char first = TRUE;
	char *goodpath = path;
	char *sp = NULL;
	char *filename = malloc( sizeof( char ) * MAXPATHLEN + 1);

	if (path == NULL || strlen (path) == 0 || path[0] != '/' || AM_NEVER) {
		return chdir (usePath (path));
	}

	if (hp != NULL) {
		if (netHostChdir (path, hp) == 0) {
			return 0;
		}
	}

	if (strstr (path, "/tmp_mnt") == path) { // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
		sp = path + strlen ("/tmp_mnt");	 // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
		if (chdir (usePath (sp)) == 0) {
			return 0;
		}
	}
	else {
		if (chdir (usePath (path)) == 0) {
			return 0;
		}
	}

	if (errno != ENOENT && errno != ENOTDIR) {
		return -1;
	}

	if (getcwd (filename, sizeof (filename)) == NULL) {
		goto try;
	}

	sp = getenv ("HOME"); // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
	if (sp != NULL) {
		chdir (sp);
	}

	chdir (filename);

try:  // FIXME FIXME FIXME FIXME FIXME remove label:
	if (path[0] != '/') {
		return -1;
	}

	if ((goodpath = strstr (path, "/exp/lsf")) != NULL) { // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
		if (chdir (usePath (goodpath)) == 0) {
			return 0;
		}
	}

	if (strstr (path, "/tmp_mnt") == path) { // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
		goodpath = path + strlen ("/tmp_mnt"); // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
	}
	else {
		if (chdir (usePath (path)) == 0) {
			return 0;
		}
		sp = getenv ("PWD"); 
		if (tryPwd (path, sp) == 0) { // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
			return 0;
		}
	}

	if (goodpath == NULL) {
		goodpath = strchr (path + 1, '/');
	}
	else {
		goodpath = strchr (goodpath + 1, '/');
	}

	if (goodpath != NULL) {
		if (chdir (usePath (goodpath)) == 0) {
			return 0;
		}
	}
	else {
		return -1;
	}

	if (first) {
		first = FALSE;
		if (getMap_ () != 0) {
			return -1;
		}
	}

	hashEntryPtr = h_firstEnt_ (&hashTab, &hashSearchPtr);
	if (hashEntryPtr == NULL) {
		errno = ENOENT;
		return -1;
	}

	while (hashEntryPtr != NULL) {
		sprintf (filename, "%s%s", hashEntryPtr->keyname, goodpath);
		if (chdir (usePath (filename)) == 0) {
			return 0;
		}
		hashEntryPtr = h_nextEnt_ (&hashSearchPtr);
	}

	goodpath = strchr (goodpath + 1, '/');
	if (goodpath == NULL) {
		return -1;
	}

	hashEntryPtr = h_firstEnt_ (&hashTab, &hashSearchPtr);
	while (hashEntryPtr != NULL)
	{
		sprintf (filename, "%s%s", hashEntryPtr->keyname, goodpath);
		if (chdir (usePath (filename)) == 0) {
			return 0;
		}
		hashEntryPtr = h_nextEnt_ (&hashSearchPtr);
	}

	if (chdir (usePath (goodpath)) == 0)  {
		return 0;
	}

	if (strstr (path, "/tmp_mnt") != path) { // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
		return -1;
	}

	goodpath = path + strlen ("/tmp_mnt"); // FIXME FIXME FIXME FIXME replace fixed strings with autoconf settings
	if (*goodpath == '\0') {
		return -1;
	}

	strcpy (filename, goodpath);
	sp = strchr (filename + 1, '/');
	if (sp == NULL) {
		return -1;
	}

	goodpath = strchr (sp + 1, '/');
	if (goodpath == NULL) {
		return -1;
	}

	if ((sp = strchr (goodpath + 1, '/')) == NULL) {
		return -1;
	}

	*goodpath = '\0'; // FIXME FIXME FIXME FIXME FIXME this outa be a free or something.
	strcat (filename, sp);

	if (chdir (usePath (filename)) == 0) {
		return 0;
	}

	if ((sp = strchr (goodpath + 1, '/')) == NULL) {
		return -1;
	}

	*goodpath = '\0'; // FIXME FIXME FIXME FIXME FIXME this outa be a free or something.
	strcat (filename, sp);

	if (chdir (usePath (filename)) == 0) {
		return 0;
	}

	if (chdir (usePath (path)) == 0) {
		return 0;
	}

// FIXME FIXME FIXME FIXME FIXME fix memory management
	return -1;
}

int
tryPwd (char *path, char *pwdpath)
{
	char *PA   = NULL;
	char *PAPB = NULL;
	char *pa   = NULL;
	char *pb   = NULL;
	char *pc   = NULL;
	char *sp   = NULL;
	char *filename = malloc( strlen( pwdpath ) + 1 ); // FIXME FIXME FIXME FIXME replace  with OS dependent settings @ configure.ac

	if (pwdpath == NULL) {
		return -1;
	}

	if( !strcmp (pwdpath, "/") ) {
		return -1;
	}

//#error  // FIXME FIXME FIXME FIXME FIXME this code is kinda crazy
	strcpy (filename, pwdpath);
	sp = strchr (filename + 1, '/');
	if (sp != NULL) {
		*sp = '\0';// FIXME FIXME FIXME FIXME FIXME this outa be a free or something.
	}
	PA = putstr_ (filename);
	strcpy (filename, pwdpath);
	if (sp != NULL)
	{
		sp = strchr (sp + 1, '/');
		if (sp != NULL) {
			*sp = '\0';  // FIXME FIXME FIXME FIXME FIXME this outa be a free or something.
		}
	}
	PAPB = putstr_ (filename);

	pa = path;
	pb = strchr (path + 1, '/');
  	if (pb == NULL) {
		pb = pa;
	}
	pc = strchr (pb + 1, '/');
	if (pc == NULL) {
		pc = pb;
	}

	strcpy (filename, PA);
	strcat (filename, pa);
	if (chdir (usePath (filename)) == 0) {
		free (PA);
		free (PAPB);
		return 0;
	}

	strcpy (filename, PA);
	strcat (filename, pb);
	if (chdir (usePath (filename)) == 0)
	{
		free (PA);
		free (PAPB);
		return 0;
	}

	  strcpy (filename, PAPB);
	  strcat (filename, pc);
	if (chdir (usePath (filename)) == 0)
	{
		free( PA );
		free( PAPB );
		free( pa );
		free( pb );
		free( pc );
		free( pc );
		free( sp );
		return 0;
	}

  strcpy (filename, PAPB);
  strcat (filename, pb);
  if (chdir (usePath (filename)) == 0)
	{
		free( PA );
		free( PAPB );
		free( pa );
		free( pb );
		free( pc );
		free( pc );
		free( sp );
	  return 0;
	}

	// FIXME FIXME FIXME FIXME FIXME mem management in this function is nto complete
	free( PA );
	free( PAPB );
	free( pa );
	free( pb );
	free( pc );
	free( pc );
	free( sp );

	return -1;
}

int
// FIXME FIXME one below is probably for MacOS
// see more:	http://www-personal.umich.edu/~saarela/programming/2005/06/dumping-nis-database-programmatically.html
// putin_ (unsigned long instatus, char *inkey, int inkeylen, char *inval, int invallen, void *indata)
putin_ ( int instatus, char *inkey, int inkeylen, char *inval, int invallen, char *indata)

{

/* do nothing with this, but gets rid of the compilier complaining 
   void *indata is mandatory cuz the header must match the
	  incallback.foreach( unsigned long, char *, int, char *, int, void * ) function  

	  this is part of NIS/NIS+

	  FIXME FIXME
*/
	assert( inval != NULL );
	assert( invallen != 0);
	assert( indata != NULL); 

#ifndef __CYGWIN__

	if( 0 != ypprot_err( (int) instatus ) ) { // FIXME FIXME FIXME cast has to go
		return TRUE;
	}
#endif
	// FIXME FIXME FIXME FIXME FIXME string comes in non-null teriminated
	assert( inkey != NULL );
	assert( inkey[inkeylen] == '\0'); // Recommended comp.lang.c F.A.Q. way of null-ternimating a string.
	if( NULL == inkey ) {
		return FALSE;
	}

	h_addEnt_( &hashTab, inkey, 0 );

	return FALSE;
}


int
getMap_ (void)
{
#ifndef __CYGWIN__

	char *domain = NULL;
	struct ypall_callback incallback;  // NIS/YP programming guide
	int i = 0;

	h_initTab_ (&hashTab, 64);		// FIXME FIXME FIXME FIXME wtf is hashTab from?
									// FIXME FIXME FIXME FIXME also why is the value of 64 is important here?
	incallback.foreach = &putin_;

	if ((i = yp_get_default_domain (&domain)) != 0) {
		return i;
	}

	return yp_all( domain, "auto.master", &incallback );
#else
	return 0;
#endif
}

int
netHostChdir (char *path, struct hostent *hp)
{
	char *filename = malloc( sizeof( char ) * MAXFILENAMELEN + 1 ); // FIXME FIXME FIXME FIXME FIXME FIXME replace MAXFILENAMELEN to OS equivallent.
	char *mp       = NULL;

	if (AM_LAST || AM_NEVER)
	{
		if ( chdir( usePath( path ) ) == 0 ) {
			return 0;
		}
	}

	if (strstr (path, "/net/") == path) { // FIXME FIXME FIXME FIXME replace fixed strings with autoconf constants, if needed.
		return -1;
	}

	if (strstr (path, "/tmp_mnt/") == path) { // FIXME FIXME FIXME FIXME replace fixed strings with autoconf constants, if needed.
		return -1;
	}

	if (hp == NULL) {
		return -1;
	}

	if ((mp = mountNet_ (hp)) == NULL) {
		return -1;
	}

	sprintf( filename, "%s%s", mp, path);
	
	free( mp );
	free( filename );
	return chdir( usePath( filename ) );
}

char *
mountNet_ (struct hostent *hp)
{
	static char *filename = NULL;
	char *hostnamebuf     = malloc( sizeof( char ) * MAXHOSTNAMELEN + 1 ); // FIXME FIXME FIXME FIXME replace MAXFILENAMELEN with appropriate OS constant
	char *cwd             = malloc( sizeof( char ) * MAXPATHLEN + 1 );     // FIXME FIXME FIXME FIXME replace MAXFILENAMELEN with appropriate OS constant
	char *sp              = NULL;
	filename = malloc( sizeof( char ) * MAXFILENAMELEN + 1);  // FIXME FIXME FIXME FIXME replace MAXFILENAMELEN with appropriate OS constant

	if (getcwd (cwd, sizeof (cwd)) == NULL) {
		return NULL;
	}

  	strcpy (hostnamebuf, hp->h_name);
  	sp = hostnamebuf;
  	if ((sp = strchr (sp, '.')) != NULL)
	{
		sp = NULL;   // FIXME FIXME FIXME FIXME sp and its value needs debugging
		sprintf (filename, "/net/%s", hostnamebuf);// FIXME FIXME FIXME FIXME replace fixed string with appropriate const char *
		if (chdir( filename ) == 0)
		{
			chdir (cwd);
			return filename;
		}
		strcpy( sp, ".");
	}

	sprintf (filename, "/net/%s", hostnamebuf);
	if (chdir (filename) == 0)
	{
		chdir (cwd);
		free (cwd );
		free( hostnamebuf ); 
		free( sp );

		return filename;
	}

	free( hostnamebuf ); 
	free( filename );
	free( cwd );
	free( sp );
	return NULL;
}

int
myopen_ (char *filename, int flags, int mode, struct hostent *hp)
{
	char *filenamebuf = malloc( sizeof( char ) * MAXFILENAMELEN + 1 );
	char *mp = NULL;
	int i = 0;

	// if we do not have a host entity struct not a path starting with a /
	if ( NULL == hp || filename[0] != '/' || AM_NEVER) {
		assert( NULL != filename );
		return open( usePath( filename ), flags, mode );
	}

	if (AM_LAST) {
		if ((i = open (usePath (filename), flags, mode)) != -1) {
			return i;
		}
	}

	if (strstr (filename, "/net/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return open (usePath (filename), flags, mode);
	}

	if (strstr (filename, "/tmp_mnt/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return open (usePath (filename), flags, mode);
	}

	if ((mp = mountNet_ (hp)) == NULL) {
		return open (usePath (filename), flags, mode);
	}

	sprintf (filenamebuf, "%s%s", mp, filename); // FIXME FIXME FIXME FIXME FIXME this neeeds to go under the debugger and see what is the difference with the return at the end of the function
	i = open (usePath (filenamebuf), flags, mode);
	if (i >= 0) {
		return i;
	}

  return open (usePath (filename), flags, mode);

}

FILE *
myfopen_ (const char *filename, const char *type, struct hostent *hp)
{
	char *filenamebuf = malloc( MAXFILENAMELEN*sizeof(char) + 1 ) ;
	FILE *fp = NULL;
	char *mp = NULL;

	memset( filenamebuf, ' ', MAXFILENAMELEN*sizeof(char) + 1 );

	if (!hp || filename[0] != '/' || AM_NEVER) {
		return fopen (usePath ( filename), type); // FIXME FIXME FIXME fix cast or call to function
	}

	if (AM_LAST) {
		if ((fp = fopen (usePath ( filename), type)) != NULL) {  // FIXME FIXME FIXME fix cast or call to function
			return fp;
		}
	}

	if (strstr (filename, "/net/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return fopen (usePath (filename), type); // FIXME FIXME FIXME fix cast or call to function
	}

	if (strstr (filename, "/tmp_mnt/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return fopen (usePath ( filename), type); // FIXME FIXME FIXME fix cast or call to function
	}

	if ((mp = mountNet_ (hp)) == NULL) {
		return fopen (usePath ( filename), type); // FIXME FIXME FIXME fix cast or call to function
	}

	sprintf (filenamebuf, "%s%s", mp, filename);
	fp = fopen (usePath (filenamebuf), type);
	if (fp != NULL) {
		return fp;
	}

	return fopen (usePath (filename), type);

}

int
mystat_ (char *filename, struct stat *sbuf, struct hostent *hp)
{
	char filenamebuf[MAXFILENAMELEN];
	char *mp = NULL;
	int i;

	if (!hp || filename[0] != '/' || AM_NEVER) {
		return stat (usePath (filename), sbuf);
	}

	if (AM_LAST) {
		if ((i = stat (usePath (filename), sbuf)) != -1) {
			return i;
		}
	}

	if (strstr (filename, "/net/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return stat (usePath (filename), sbuf);
	}

	if (strstr (filename, "/tmp_mnt/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return stat (usePath (filename), sbuf);
	}

	if ((mp = mountNet_ (hp)) == NULL) {
		return stat (usePath (filename), sbuf);
	}

	sprintf (filenamebuf, "%s%s", mp, filename);
	i = stat (usePath (filenamebuf), sbuf);
	if (i >= 0) {
		return i;
	}

	return stat (usePath (filename), sbuf);

}

int
mychmod_ (char *filename, mode_t mode, struct hostent *hp)
{
	char fnamebuf[MAXFILENAMELEN];
	int i  = 0;
	char *mp = NULL;

	if (!hp || filename[0] != '/' || AM_NEVER) {
		return chmod (usePath (filename), mode);
	}

	if (AM_LAST) {
		if ((i = chmod (usePath (filename), mode)) != -1) {
			return i;
		}
	}

	if (strstr (filename, "/net/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return chmod (usePath (filename), mode);
	}

	if (strstr (filename, "/tmp_mnt/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return chmod (usePath (filename), mode);
	}

	if ((mp = mountNet_ (hp)) == NULL) {
		return chmod (usePath (filename), mode);
	}

	sprintf (fnamebuf, "%s%s", mp, filename);
	i = chmod (usePath (fnamebuf), mode);
	if (i >= 0) {
		return i;
	}

	return chmod (usePath (filename), mode);

}

void
myexecv_ (char *filename, char **argv, struct hostent *hp)
{
  char fnamebuf[MAXFILENAMELEN];
  char *mp;

  if (!hp || filename[0] != '/' || AM_NEVER)
	{
	  lsfExecX( usePath (filename), argv , execv );
	  return;
	}

  if (AM_LAST)
	{
	  lsfExecX( usePath (filename), argv , execv );
	  return;
	}

  if (strstr (filename, "/net/") == filename)
	{
	  lsfExecX( usePath (filename), argv , execv );
	  return;
	}

  if (strstr (filename, "/tmp_mnt/") == filename)
	{
	  lsfExecX( usePath (filename), argv , execv );
	  return;
	}

  if ((mp = mountNet_ (hp)) == NULL)
	{
	  lsfExecX( usePath (filename), argv , execv );
	  return;
	}

  sprintf (fnamebuf, "%s%s", mp, filename);
  lsfExecX( usePath (fnamebuf), argv , execv );

  lsfExecX( usePath (filename), argv , execv );

}


int
myunlink_ (char *filename, struct hostent *hp, int doMount)
{
	char fnamebuf[MAXFILENAMELEN];
	char *mp = NULL;
	int i    = 0;

	if (!hp || filename[0] != '/' || AM_NEVER) {
		return unlink (usePath (filename));
	}

	if (AM_LAST) {
		if ((i = unlink (usePath (filename))) != -1) {
			return 1;
		}
	}

	if (doMount) {
		if (strstr (filename, "/net/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
			return unlink (usePath (filename));
		}

		if (strstr (filename, "/tmp_mnt/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
			return unlink (usePath (filename));
		}

		if ((mp = mountNet_ (hp)) == NULL) {
			return 1;
		}

		sprintf (fnamebuf, "%s%s", mp, filename);
		i = unlink (usePath (fnamebuf));
		if (i >= 0)
			return i;
	}

	return unlink (usePath (filename));
}


int
mymkdir_ (char *filename, mode_t mode, struct hostent *hp)
{
	char fnamebuf[MAXFILENAMELEN];
	char *mp = NULL;
	int i    = 0;

	if (!hp || filename[0] != '/' || AM_NEVER) {
		return mkdir (usePath (filename), mode);
	}

	if (AM_LAST)
		if ((i = mkdir (usePath (filename), mode)) != -1) {
			return i;
		}

	if (strstr (filename, "/net/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return mkdir (usePath (filename), mode);
	}

	if (strstr (filename, "/tmp_mnt/") == filename) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return mkdir (usePath (filename), mode);
	}

	if ((mp = mountNet_ (hp)) == NULL) {
		return mkdir (usePath (filename), mode);
	}

	sprintf (fnamebuf, "%s%s", mp, filename);
	i = mkdir (usePath (fnamebuf), mode);
	if (i >= 0) {
		return i;
	}

	return mkdir (usePath (filename), mode);
}


int
myrename_ (char *from, char *to, struct hostent *hp)
{
	char fnamebuf[MAXFILENAMELEN];
	char tnamebuf[MAXFILENAMELEN];
	char *mp = NULL;
	int i    = 0;

	if (!hp || AM_NEVER) {
		return rename (from, to);
	}

	if (AM_LAST) {
		if ((i = rename (from, to)) != -1) {
			return i;
		}
	}

	if ((strstr (from, "/net/") == from) && (strstr (to, "/net") == to)) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return rename (from, to);
	}

	if ( (strstr (from, "/tmp_mnt/") == from) && (strstr (to, "/tmp_mnt/") == to) ) { // FIXME FIXME FIXME FIXME FIXME const UNIX file path reference: change it to variable, investigate use
		return rename (from, to);
	}

	if ((mp = mountNet_ (hp)) == NULL) {
		return rename (from, to);
	}

	if (from[0] == '/')
		sprintf (fnamebuf, "%s%s", mp, from);
	else {
		strcpy (fnamebuf, from);
	}

	if (to[0] == '/') {
		sprintf (tnamebuf, "%s%s", mp, to);
	}
	else {
		strcpy (tnamebuf, to);
	}

	i = rename (fnamebuf, tnamebuf);
	if (i >= 0) {
		return i;
	}

	return rename (from, to);

}
