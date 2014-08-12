/* $Id: lshosts.c 397 2007-11-26 19:04:00Z mblack $
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

#include <ctype.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "intlib/intlibout.h"
#include "lib/lproto.h"
#include "lsf.h"


#define NL_SETN 27

void lshosts_usage( char * );
void lshosts_printlong( struct hostInfo *hostInfo );
void lshosts_printwide( struct hostInfo *hostInfo );

int
main (int argc, char **argv)
{
  static char fname[] = "lshosts/main";
  char *namebufs[256];
  struct hostInfo *hostinfo;
  int numhosts = 0;
  struct hostent *hp;
  int i = 0, j = 0;
  char *resReq = NULL;
  char longformat = FALSE;
  char longname = FALSE;
  char staticResource = FALSE, otherOption = FALSE;
  int extView = FALSE;
  int achar = 0;
  int unknown = 0;
  int options = 0;
  int isClus = 0;
  /* int rc = 0;


   rc = _i18n_init (I18N_CAT_MIN); */

  if (ls_initdebug (argv[0]) < 0)
    {
      ls_perror ("ls_initdebug");
      exit (-1);
    }
  if (logclass & (LC_TRACE))
    ls_syslog (LOG_DEBUG, "%s: Entering this routine...", fname);

  for (i = 1; i < argc; i++) /* replace with GNU getopts() */
    {
      if (strcmp (argv[i], "-h") == 0)
	{
	  lshosts_usage (argv[0]);
	  exit (0);
	}
      else if (strcmp (argv[i], "-V") == 0)
	{
	  fputs (_LS_VERSION_, stderr);
	  exit (0);
	}
      else if (strcmp (argv[i], "-s") == 0)
	{
	  if (otherOption == TRUE)
	    {
	      lshosts_usage (argv[0]);
	      exit (-1);
	    }
	  staticResource = TRUE;
	  optind = i + 1;
	}
      else if (strcmp (argv[i], "-e") == 0)
	{
	  if (otherOption == TRUE || staticResource == FALSE)
	    {
	      lshosts_usage (argv[0]);
	      exit (-1);
	    }
	  optind = i + 1;
	  extView = TRUE;
	}
      else if (strcmp (argv[i], "-R") == 0 || strcmp (argv[i], "-l") == 0
	       || strcmp (argv[i], "-w") == 0)
	{
	  otherOption = TRUE;
	  if (staticResource == TRUE)
	    {
	      lshosts_usage (argv[0]);
	      exit (-1);
	    }
	}
    }

  if (staticResource == TRUE)
    {
      displayShareResource (argc, argv, optind, TRUE, extView);
    }
  else
    {
      while ((achar = getopt (argc, argv, "R:lw")) != EOF)
	{
	  switch (achar)
	    {
	    case 'R':
	      if (strlen (optarg) > MAXLINELEN) /* this should be a bit larger, and depedant on the OS compiled on */
		{
		  printf (" %s", I18N (1645, "The resource requirement string exceeds the maximum length of 512 characters. Specify a shorter resource requirement.\n"));	/* catgets  1645  */
		  exit (-1);
		}
	      resReq = optarg;
	      break;
	    case 'l':
	      longformat = TRUE;
	      break;
	    case 'w':
	      longname = TRUE;
	      break;
	    default:
	      lshosts_usage (argv[0]);
	      exit (-1);
	    }
	}

      i = 0;
      unknown = 0;
      for (; optind < argc; optind++)
	{
	  if (strcmp (argv[optind], "allclusters") == 0)
	    {
	      options = ALL_CLUSTERS;
	      i = 0;
	      break;
	    }
	  if ((isClus = ls_isclustername (argv[optind])) < 0)
	    {
	      fprintf (stderr, "lshosts: %s\n", ls_sysmsg ());
	      unknown = 1;
	      continue;
	    }
	  else if ((isClus == 0) &&
		   ((hp = Gethostbyname_ (argv[optind])) == NULL))
	    {
	      fprintf (stderr, "\
%s: gethostbyname() failed for host %s.\n", __func__, argv[optind]);
	      unknown = 1;
	      continue;
	    }
	  namebufs[i] = strdup (hp->h_name);
	  if (namebufs[i] == NULL)
	    {
	      perror ("strdup()");
	      exit (-1);
	    }
	  i++;
	}

      if (i == 0 && unknown == 1)
	exit (-1);

      if (i == 0)
	{
	  TIMEIT (0, (hostinfo = ls_gethostinfo (resReq, &numhosts, NULL, 0,
						 options)), "ls_gethostinfo");
	  if (hostinfo == NULL)
	    {
	      ls_perror ("ls_gethostinfo()");
	      exit (-1);
	    }
	}
      else
	{
	  TIMEIT (0, (hostinfo = ls_gethostinfo (resReq, &numhosts, namebufs,
						 i, 0)), "ls_gethostinfo");
	  if (hostinfo == NULL)
	    {
	      ls_perror ("ls_gethostinfo");
	      exit (-1);
	    }
	}

      if (!longformat && !longname)
	{
	  char *buf1, *buf2, *buf3, *buf4, *buf5;
	  char *buf6, *buf7, *buf8, *buf9;

	  buf1 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1628, "HOST_NAME"));	/* catgets 1628 */
	  buf2 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1602, "type"));	/* catgets  1602  */
	  buf3 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1603, "model"));	/* catgets  1603  */
	  buf4 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1604, "cpuf"));	/* catgets  1604 */
	  buf5 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1605, "ncpus"));	/* catgets  1605  */
	  buf6 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1607, "maxmem"));	/* catgets  1607  */
	  buf7 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1608, "maxswp"));	/* catgets  1608  */
	  buf8 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1611, "server"));	/* catgets  1611  */
	  buf9 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1614, "RESOURCES"));	/* catgets  1614  */

	  printf
	    ("%-11.11s %7.7s %8.8s %5.5s %5.5s %6.6s %6.6s %6.6s %9.9s\n",
	     buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9);

	  FREEUP (buf1);
	  FREEUP (buf2);
	  FREEUP (buf3);
	  FREEUP (buf4);
	  FREEUP (buf5);
	  FREEUP (buf6);
	  FREEUP (buf7);
	  FREEUP (buf8);
	  FREEUP (buf9);

	}
      else if (longname)
	{
	  char *buf1, *buf2, *buf3, *buf4, *buf5;
	  char *buf6, *buf7, *buf8, *buf9;

	  buf1 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1628, "HOST_NAME"));	/* catgets  1628 */
	  buf2 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1602, "type"));	/* catgets  1602  */
	  buf3 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1603, "model"));	/* catgets  1603  */
	  buf4 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1604, "cpuf"));	/* catgets  1604  */
	  buf5 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1605, "ncpus"));	/* catgets  1605  */
	  buf6 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1607, "maxmem"));	/* catgets  1607  */
	  buf7 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1608, "maxswp"));	/* catgets  1608  */
	  buf8 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1611, "server"));	/* catgets  1611  */
	  buf9 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1614, "RESOURCES"));	/* catgets  1614  */

	  printf
	    ("%-25.25s %10.10s %11.11s %5.5s %5.5s %6.6s %6.6s %6.6s %9.9s\n",
	     buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9);

	  FREEUP (buf1);
	  FREEUP (buf2);
	  FREEUP (buf3);
	  FREEUP (buf4);
	  FREEUP (buf5);
	  FREEUP (buf6);
	  FREEUP (buf7);
	  FREEUP (buf8);
	  FREEUP (buf9);
	}

      for (i = 0; i < numhosts; i++)
	{
	  char *server;
	  int first;

	  if (longformat)
	    {
	      lshosts_printlong (&hostinfo[i]);
	      continue;
	    }

	  if (hostinfo[i].isServer)
	    server = I18N_Yes;
	  else
	    server = I18N_No;


	  if (longname)
	    printf ("%-25s %10s %11s %5.1f ", hostinfo[i].hostName,
		    hostinfo[i].hostType, hostinfo[i].hostModel,
		    hostinfo[i].cpuFactor);
	  else
	    printf ("%-11.11s %7.7s %8.8s %5.1f ", hostinfo[i].hostName,
		    hostinfo[i].hostType, hostinfo[i].hostModel,
		    hostinfo[i].cpuFactor);

	  if (hostinfo[i].maxCpus > 0)
	    printf ("%5d", hostinfo[i].maxCpus);
	  else
	    printf ("%5.5s", "-");

	  if (hostinfo[i].maxMem > 0)
	    printf (" %5dM", hostinfo[i].maxMem);
	  else
	    printf (" %6.6s", "-");

	  if (hostinfo[i].maxSwap > 0)
	    printf (" %5dM", hostinfo[i].maxSwap);
	  else
	    printf (" %6.6s", "-");

	  printf (" %6.6s", server);
	  printf (" (");

	  first = TRUE;
	  for (j = 0; j < hostinfo[i].nRes; j++)
	    {
	      if (!first)
		printf (" ");
	      printf ("%s", hostinfo[i].resources[j]);
	      first = FALSE;
	    }

	  fputs (")\n", stdout);
	}


      _i18n_end (ls_catd);
      exit (0);
    }

  _i18n_end (ls_catd);
  return (0);
}
