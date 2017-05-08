/* $Id: eauth.c 397 2007-11-26 19:04:00Z mblack $
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

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libint/intlibout.h"
#include "lib/lib.h"
#include "lib/lproto.h"
#include "lsf.h"


#if defined(DEBUG)
FILE *logfp;
char logfile[100]; // FIXME FIXME FIXME oddly specific
#endif

int getAuth (char *);
int printUserName (void);
int vauth (char *, char *, int);

int main (int argc, char **argv)
{

  if (initenv_ (NULL, NULL) < 0)
    exit (-1);

#if defined(DEBUG)
  sprintf (logfile, "%s/eauth.log", LSTMPDIR);

  if ((logfp = fopen (logfile, "a+")) == NULL)
    {
      perror ("fopen failed!");
      exit (-1);
    }
#endif

  if (argc < 2)
    {
#if defined(DEBUG)
      fprintf (logfp, "Missing argument.\n");
      fclose (logfp);
#endif
      exit (-1);
    }

  if (!strcmp (argv[1], "-c"))
    {

      if (setuid (getuid ()) < 0)
	{
#if defined(DEBUG)
	  fclose (logfp);
#endif
	  exit (-1);
	}

      if (getAuth (argv[2]))
	{
#if defined(DEBUG)
	  fclose (logfp);
#endif
	  exit (-1);
	}
    }
  else if (!strcmp (argv[1], "-s"))
    {
      char datBuf[1024];
      char lsfUserName[128];
      char client_addr[64];
      char lsfUserNameTmp[1024];
      char client_addrTmp[1024];
      uid_t uid = 0;
      gid_t gid = 0;
      ushort client_port = 0;
      unsigned long datLen = 0;
      unsigned long cc = 0;

      for (;;)
	{
	  fflush (stderr);

	  memset (datBuf, 0, sizeof (datBuf));
	  memset (lsfUserName, 0, sizeof (lsfUserName));
	  memset (client_addr, 0, sizeof (client_addr));
	  memset (lsfUserNameTmp, 0, sizeof (lsfUserNameTmp));
	  memset (client_addrTmp, 0, sizeof (client_addrTmp));
	  fgets (datBuf, sizeof (datBuf), stdin);
	  sscanf (datBuf, "%d %d %s %s %hd %ld", &uid, &gid, lsfUserNameTmp, client_addrTmp, &client_port, &datLen);

	  ls_strcat (lsfUserName, sizeof (lsfUserName), lsfUserNameTmp);
	  ls_strcat (client_addr, sizeof (client_addr), client_addrTmp);

	  memset (datBuf, 0, sizeof (datBuf));
	  if ((cc = fread (datBuf, 1, datLen, stdin)) != datLen)
	    {
#if defined(DEBUG)
	      fprintf (logfp, "fread (%d) failed\n", datLen);
	      fprintf (logfp, "uid=%d, gid=%d, username=%s, client_addr=%s, client_port=%d, datLen=%d, cc=%d, dataBuf=%s\n", uid, gid, lsfUserName, client_addr, client_port, datLen, cc, datBuf);
	      fclose (logfp);
#endif
	      exit (-1);
	    }

      assert( datLen <= INT_MAX );
	  if (vauth (lsfUserName, datBuf, (int)datLen) == -1)
	    {
	      putchar ('0');
	    }
	  else
	    {
	      putchar ('1');
	    }
	  fflush (stdout);
	}
    }
#if defined(DEBUG)
  fclose (logfp);
#endif

  return 0;
}


int
getAuth (char *inst)
{

assert( *inst );
#if defined(DEBUG)
  fprintf (logfp, "======Call by client=====\n");
  fprintf (logfp, "LSF_EAUTH_KEY=NULL\n");
#endif

  return printUserName ();
}


int
printUserName (void)
{
  char lsfUserName[MAXLSFNAMELEN];
  char *encUsername;
  char dataBuff[1024];

  if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) < 0)
    {
#if defined(DEBUG)
      fprintf (logfp, "getLSFUser_ failed: %s!\n", ls_sysmsg ());
#endif
      return -1;
    }

  if ((encUsername = encryptByKey_ (NULL, lsfUserName)) == NULL)
    {
#if defined(DEBUG)
      fprintf (logfp, "encryptByKey_ (NULL, %s) failed!\n", pw->pw_name);

      return -1;
#endif
    }
  memset (dataBuff, 0, sizeof (dataBuff));
  ls_strcat (dataBuff, sizeof (dataBuff), encUsername);

  free (encUsername);

#if defined(DEBUG)
  fprintf (logfp, "username is %s, encrypted username is %s, len=%d\n",
	   lsfUserName, dataBuff, strlen (dataBuff));
#endif

  fwrite (dataBuff, strlen (dataBuff), 1, stdout);
  return 0;
}

int
vauth (char *lsfUserName, char *datBuf, int datLen)
{
  char *authName;
  char *authPass;
  char *deUserName;

  assert( datLen );


#if defined(DEBUG)
  fprintf (logfp, "==========Call by server=========\n");
  fprintf (logfp, "LSF_EAUTH_KEY=NULL\n");
#endif


  authName = datBuf;
  if ((authPass = (char *) strchr (authName, (int) ' ')) != NULL)
    *authPass++ = '\0';

  if ((deUserName = decryptByKey_ (NULL, datBuf)) == NULL)
    {
#if defined(DEBUG)
      fprintf (logfp, "decryptByKey_(NULL,  %s) failed!\n", datBuf);
#endif
      return -1;
    }
  if (strcmp (deUserName, lsfUserName) != 0)
    {
#if defined(DEBUG)
      fprintf (logfp, "decrypt username %s doesn't equal username %s\n",
	       deUserName, lsfUserName);
      fflush (logfp);
#endif
      return -1;
    }
#if defined(DEBUG)
  fprintf (logfp, "decrypt username success, dataBuf is %s, username is %s\n",
	   datBuf, lsfUserName);
  fflush (logfp);
#endif
  return 0;

}
