#include <ctype.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "intlib/intlibout.h"
// #include "lib/lproto.h"
#include "lsf.h"

char *stripSpaces (char *);

/***
 * lshosts_printlong: lshosts -l
 *	prints a detailed index of the load-sharing status of all hosts
 *  in particular, it prints the
 *  r15s, r1m, r15m, ut, pg, io, ls, t, tmp, swp, mem, dflt, and NULL
 *  In particular, see struct indexFormat above
 */
void
lshosts_printlong( struct hostInfo *hostInfo )
{
	int i;
	float *li;
	char *sp;
	static int first = TRUE;
	static char line[132];
	static char newFormat[10];
	int newIndexLen, retVal;
	char **indxnames;
	char **shareNames, **shareValues, **formats;
	char strbuf1[30], strbuf2[30], strbuf3[30];

struct indexFormat {
  char *name;
  char *hdr;
  char *busy;
  char *ok;
  float scale;
};

  struct indexFormat format1[13] = {
    {"r15s", "%6s", "*%4.1f", "%5.1f", 1.0},
    {"r1m", "%6s", "*%4.1f", "%5.1f", 1.0},
    {"r15m", "%6s", "*%4.1f", "%5.1f", 1.0},
    {"ut", "%5s", "*%3.0f%%", "%3.0f%%", 100.0},
    {"pg", "%6s", "*%4.1f", "%4.1f", 1.0},
    {"io", "%6s", "*%4.0f", "%4.0f", 1.0},
    {"ls", "%5s", "*%2.0f", "%2.0f", 1.0},
    {"it", "%5s", "*%3.0f", "%4.0f", 1.0},
    {"tmp", "%6s", "*%4.0fM", "%4.0fM", 1.0},
    {"swp", "%6s", "*%3.0fM", "%4.0fM", 1.0},
    {"mem", "%6s", "*%3.0fM", "%4.0fM", 1.0},
    {"dflt", "%7s", "*%6.1f", "%6.1f", 1.0},
    {NULL, "%7s", "*%6.1f", " %6.1f", 1.0}
  }; 
  
  struct indexFormat *format;

  if( first ) 
  {
      char tmpbuf[MAX_LSF_NAME_LEN];
      int fmtid;


      if (!(format = (struct indexFormat *)
	    malloc ((hostInfo->numIndx + 2) * sizeof (struct indexFormat))))
	{
	  lserrno = LSE_MALLOC;
	  ls_perror ("print_long");
	  exit (-1);
	}
      for (i = 0; i < NBUILTINDEX + 2; i++)
	format[i] = format1[i];

      TIMEIT (0, (indxnames = ls_indexnames (NULL)), "ls_indexnames");
      if (indxnames == NULL)
	{
	  ls_perror ("ls_indexnames");
	  exit (-1);
	}
      for (i = 0; indxnames[i]; i++)
	{
	  if (i > MEM)
	    fmtid = MEM + 1;
	  else
	    fmtid = i;

	  if ((fmtid == MEM + 1)
	      && (newIndexLen = strlen (indxnames[i])) >= 7)
	    {
	      sprintf (newFormat, "%s%d%s", "%", newIndexLen + 1, "s");
	      /* sprintf (tmpbuf, newFormat, indxnames[i]); */
        sprintf( tmpbuf, "%s", indxnames[i] );
	    }
	  else {
	    /* sprintf (tmpbuf, format[fmtid].hdr, indxnames[i]); */
       sprintf (tmpbuf, "%s", indxnames[i]);
    }
	  strcat (line, tmpbuf);
	}
      first = FALSE;
    }

  printf ("\n%s:  %s\n", _i18n_msg_get (ls_catd, NL_SETN, 1601, "HOST_NAME"),	/* catgets 1601 */
	  hostInfo->hostName);
  {
    char *buf1, *buf2, *buf3, *buf4, *buf5, *buf6, *buf7, *buf8, *buf9,
      *buf10;

    buf1 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1602, "type"));	/* catgets 1602 */
    buf2 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1603, "model"));	/* catgets 1603 */
    buf3 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1604, "cpuf"));	/* catgets 1604 */
    buf4 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1605, "ncpus"));	/* catgets 1605 */
    buf5 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1606, "ndisks"));	/* catgets 1606 */
    buf6 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1607, "maxmem"));	/* catgets 1607 */
    buf7 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1608, "maxswp"));	/* catgets 1608 */
    buf8 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1609, "maxtmp"));	/* catgets 1609 */
    buf9 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1610, "rexpri"));	/* catgets 1610 */
    buf10 = putstr_ (_i18n_msg_get (ls_catd, NL_SETN, 1611, "server"));	/* catgets 1611 */

    printf
      ("%-10.10s %11.11s %5.5s %5.5s %6.6s %6.6s %6.6s %6.6s %6.6s %6.6s\n",
       buf1, buf2, buf3, buf4, buf5, buf6, buf7, buf8, buf9, buf10);

    FREEUP (buf1);
    FREEUP (buf2);
    FREEUP (buf3);
    FREEUP (buf4);
    FREEUP (buf5);
    FREEUP (buf6);
    FREEUP (buf7);
    FREEUP (buf8);
    FREEUP (buf9);
    FREEUP (buf10);

  }
  if (hostInfo->isServer)
    {
      sprintf (strbuf1, "%-10s", hostInfo->hostType);
      strbuf1[10] = '\0';
      sprintf (strbuf2, "%11s", hostInfo->hostModel);
      strbuf2[11] = '\0';
      sprintf (strbuf3, "%5.1f", hostInfo->cpuFactor);
      strbuf3[5] = '\0';
      printf ("%-10s %11s %5s ", strbuf1, strbuf2, strbuf3);
      if (hostInfo->maxCpus > 0)
	printf ("%5d %6d %5dM %5dM %5dM %6d %6s\n",
		hostInfo->maxCpus, hostInfo->nDisks, hostInfo->maxMem,
		hostInfo->maxSwap, hostInfo->maxTmp,
		hostInfo->rexPriority, I18N_Yes);
      else
	printf ("%5s %6s %6s %6s %6s %6d %6s\n", "-", "-", "-", "-", "-", hostInfo->rexPriority, I18N_Yes);	/* catgets 1612  */
    }
  else
    {
      sprintf (strbuf1, "%-10s", hostInfo->hostType);
      strbuf1[10] = '\0';
      sprintf (strbuf2, "%11s", hostInfo->hostModel);
      strbuf2[11] = '\0';
      sprintf (strbuf3, "%5.1f", hostInfo->cpuFactor);
      strbuf3[5] = '\0';
      printf ("%-10s %11s %5s ", strbuf1, strbuf2, strbuf3);
      printf ("%5s %6s %6s %6s %6s %6s %6s\n", "-", "-", "-", "-", "-", "-", I18N_No);	/* catgets 1613 */
    }


  if (sharedResConfigured_ == TRUE)
    {
      if ((retVal = makeShareField (hostInfo->hostName, TRUE, &shareNames,
				    &shareValues, &formats)) > 0)
	{


	  for (i = 0; i < retVal; i++)
	    {
	      /* printf (formats[i], shareNames[i]); */
        printf( "%s", shareNames[i] );
	    }
	  printf ("\n");
	  for (i = 0; i < retVal; i++)
	    {
	      /* printf (formats[i], shareValues[i]); */
        printf( "%s" , shareValues[i]);
	    }

	  printf ("\n");
	}
    }

  printf ("\n");
  printf ("%s: ", _i18n_msg_get (ls_catd, NL_SETN, 1614, "RESOURCES"));	/* catgets 1614 */
  if (hostInfo->nRes)
    {
      int first = TRUE;
      for (i = 0; i < hostInfo->nRes; i++)
	{
	  if (!first)
	    printf (" ");
	  else
	    printf ("(");
	  printf ("%s", hostInfo->resources[i]);
	  first = FALSE;
	}
      printf (")\n");
    }
  else
    {
      printf ("%s\n", _i18n_msg_get (ls_catd, NL_SETN, 1615, "Not defined"));	/* catgets 1615 */
    }

  printf ("%s: ", _i18n_msg_get (ls_catd, NL_SETN, 1616, "RUN_WINDOWS"));	/* catgets 1616  */
  if (hostInfo->isServer)
    {
      if (strcmp (hostInfo->windows, "-") == 0)
	fputs (_i18n_msg_get (ls_catd, NL_SETN, 1617, " (always open)\n"),	/* catgets 1617 */
	       stdout);
      else
	printf ("%s\n", hostInfo->windows);
    }
  else
    {
      printf (_i18n_msg_get (ls_catd, NL_SETN, 1618, "Not applicable for client-only host\n"));	/* catgets 1618 */
    }

  if (!hostInfo->isServer)
    {
      printf ("\n");
      return;
    }


  printf ("\n");
  printf (_i18n_msg_get (ls_catd, NL_SETN, 1626, "LOAD_THRESHOLDS:"));	/* catgets 1626 */
  printf ("\n%s\n", line);
  li = hostInfo->busyThreshold;
  for (i = 0; indxnames[i]; i++)
    {
      char tmpfield[MAX_LSF_NAME_LEN];
      int id;

      if (i > MEM)
	id = MEM + 1;
      else
	id = i;
      if (fabs (li[i]) >= (double) INFINIT_LOAD)
	sp = "-";
      else
	{
	  /* sprintf (tmpfield, format[id].ok, li[i] * format[id].scale); */
    sprintf (tmpfield, "%f", li[i] * format[id].scale);
	  sp = stripSpaces (tmpfield);
	}
      if ((id == MEM + 1) && (newIndexLen = strlen (indxnames[i])) >= 7)
	{
	  sprintf (newFormat, "%s%d%s", "%", newIndexLen + 1, "s");
	  /* printf (newFormat, sp); */
    printf ( "%s", sp );
	}
      else {
	     /* printf (format[id].hdr, sp); */ 
       printf( "%s", sp );
     }
  }

  printf ("\n");
}

char *
stripSpaces (char *field)
{
  char *cp;
  int len, i;

  cp = field;
  while (*cp == ' ')
    cp++;

  len = strlen (field);
  i = len - 1;
  while ((i > 0) && (field[i] == ' '))
    i--;
  if (i < len - 1)
    field[i] = '\0';
  return (cp);
}
