/*
 * Copyright (C) 2011-2012 David Bigagli
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

#include <unistd.h>
#ifndef __USE_MISC
#define __USE_MISC // FIXME FIXME FIXME related to syslog.h via vsyslog(); not quite sure if portable
#endif
#include <syslog.h>
#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>

#include "lib/syslog.h"
#include "lib/lproto.h"
#include "lib/osal.h"
#include "lib/lib.h"
#include "lib/err.h"

// #define LSF_LOG_MASK   7
#define DEF_LOG_MASK   LOG_INFO
#define DEF_LOG_MASK_NAME   "LOG_WARNING"

/*#ifndef HAS_VSNPRINTF FIXME FIXME
#define HAS_VSNPRINTF
#endif
*/


char *
argvmsg_ (int argc, char **argv)
{
  static char avbuffer[128];
  char *bp = avbuffer;
  const char *ap;
  int i = 0;

  ap = argv[0];
  while ((i < argc) && (bp < (avbuffer + sizeof (avbuffer) - 1)))
	{
	  if (!*ap)
	{
	  i++;
	  if ((i < argc) && ((ap = argv[i]) != NULL))
		{
		  *bp = ' ';
		  bp++;
		}
	}
	  else
	{
	  *bp = *ap;
	  bp++;
	  ap++;
	}
	}
  *bp = '\0';
  return avbuffer;
}

void
ls_openlog (const char *ident, const char *path, int use_stderr, char *logMask)
{
  char *msg = NULL;

  strncpy (logident, ident, 9);
  logident[9] = '\0';

  logmask = getLogMask (&msg, logMask);

  if (use_stderr)
	{
	  log_dest = LOGTO_STDERR;
	  return;
	}

  if (path && *path)
	{
	  char *myname;
	  FILE *lfp;
	  struct stat st;

	  if ((myname = ls_getmyhostname ()) == NULL)
	goto syslog;

	  sprintf (logfile, "%s/%s.log.%s", path, ident, myname);

	  if (lstat (logfile, &st) < 0)
	{
	  if (errno == ENOENT)
		{
		  if (openLogFile (ident, myname) == 0)
		{
		  if (msg != NULL)
			ls_syslog (LOG_ERR, "%s", msg);
		  return;
		}
		}
	  else
		{
		  sprintf (logfile, "%s/%s.log.%s", LSTMPDIR, ident, myname);
		  if (lstat (logfile, &st) < 0)
		{
		  if (errno == ENOENT)
			{
			  if ((lfp = fopen (logfile, "a")) != NULL)
			{
			  fclose (lfp);
			  if (!strcmp (ident, "res")
				  || (logmask >= LOG_UPTO (LOG_DEBUG)
				  && logmask <= LOG_UPTO (LOG_DEBUG3)))
				chmod (logfile, 0666);
			  else
				chmod (logfile, 0644);
			  log_dest = LOGTO_FILE;
			  if (msg != NULL)
				ls_syslog (LOG_ERR, "%s", msg);
			  return;
			}
			}
		}
		  else if (S_ISREG (st.st_mode) && st.st_nlink == 1)
		{

		  if ((lfp = fopen (logfile, "a")) != NULL)
			{
			  fclose (lfp);
			  if (!strcmp (ident, "res")
			  || (logmask >= LOG_UPTO (LOG_DEBUG)
				  && logmask <= LOG_UPTO (LOG_DEBUG3)))
			chmod (logfile, 0666);
			  else
			chmod (logfile, 0644);
			  log_dest = LOGTO_FILE;
			  if (msg != NULL)
			ls_syslog (LOG_ERR, "%s", msg);
			  return;
			}
		}
		}
	}
	  else if (S_ISREG (st.st_mode) && st.st_nlink == 1)
	{
	  if (openLogFile (ident, myname) == 0)
		{
		  if (msg != NULL)
		ls_syslog (LOG_ERR, "%s", msg);
		  return;
		}
	}
	}

syslog: 	// FIXME FIXME FIXME FIXME substitute where appropriate and get rid of it

  log_dest = LOGTO_SYS;
  logfile[0] = '\0';

  openlog ((char *) ident, LOG_PID, LOG_DAEMON);
  setlogmask (logmask);

  if (msg != NULL)
	ls_syslog (LOG_ERR, "%s", msg);
}

int
openLogFile (const char *ident, char *myname)
{
  FILE *lfp;
  struct stat st;

  if ((lfp = fopen (logfile, "a")) == NULL)
	{

	  sprintf (logfile, "%s/%s.log.%s", LSTMPDIR, ident, myname);
	  if (lstat (logfile, &st) < 0)
	{
	  if (errno == ENOENT)
		{
		  if ((lfp = fopen (logfile, "a")) == NULL)
		{
		  return (-1);
		}
		}
	  else
		{
		  return (-1);
		}
	}
	  else if (S_ISREG (st.st_mode) && st.st_nlink == 1)
	{

	  if ((lfp = fopen (logfile, "a")) == NULL)
		{
		  return (-1);
		}
	}
	  else
	{
	  return (-1);
	}
	}

  if (lfp != NULL)
	{
	  fclose (lfp);

	  if (!strcmp (ident, "res")
	  || (logmask >= LOG_UPTO (LOG_DEBUG)
		  && logmask <= LOG_UPTO (LOG_DEBUG3)))
	{
	  chmod (logfile, 0666);
	}
	  else
	{
	  chmod (logfile, 0644);
	}
	  log_dest = LOGTO_FILE;
	  return (0);
	}
  return (-1);
}

void
ls_syslog (int level, const char *fmt, ...) // FIXME FIXME convert variable argument list to static
{
	int save_errno = errno;
	va_list ap; // FIXME FIXME full initilization
	// static char lastMsg[16384];
	// static int counter = 0;
	char *buf = malloc( sizeof( char ) * 1024 + 1 );

	va_start (ap, fmt);

	if (log_dest == LOGTO_STDERR)
	{

		if ((logmask & LOG_MASK (level)) != 0)
	   {
		   errno = save_errno;
		   verrlog_ (level, stderr, fmt, ap);
	   }

	}
	else if (logfile[0])
	{

		if ((logmask & LOG_MASK (level)) != 0)
		{
		   FILE *lfp = NULL;
		   struct stat st;

		   if (lstat (logfile, &st) < 0)
		   {
				if (errno == ENOENT)
				{
					if ((lfp = fopen (logfile, "a")) == NULL)
					{
						char *kot = malloc( sizeof( char ) * strlen( buf ) + 1 );

						if (log_dest == LOGTO_FILE)
						{
							log_dest = LOGTO_SYS;
							openlog (logident, LOG_PID, LOG_DAEMON);
							setlogmask (logmask);
						}
						
						if (level > LOG_DEBUG) {
							level = LOG_DEBUG;
						}

						sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
						kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
						strcat( kot, (char *) ap );
						syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throing errors. must revisit function
						vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
						closelog ();
						free( kot );
					}
				}
				else
				{
					err_str_ (save_errno, fmt, buf);
					char *kot = malloc( sizeof( char ) * strlen( buf ) + 1 );
					if (log_dest == LOGTO_FILE)
					{
						log_dest = LOGTO_SYS;
						openlog (logident, LOG_PID, LOG_DAEMON);
						setlogmask (logmask);
					}
					
					if (level > LOG_DEBUG) {
						level = LOG_DEBUG;
					}

					err_str_ (save_errno, fmt, buf); // this maybe wrong
					sprintf( kot, "%s", buf ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
					kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
					strcat( kot, (char *) ap );
					syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throwing errors. must revisit function
					vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
					closelog( );
					free( kot );
				}
			}
			else if (!(S_ISREG (st.st_mode) && st.st_nlink == 1))
			{

				char *kot = malloc( sizeof( char ) * strlen( err_str_ (save_errno, fmt, buf) ) + 1 );
				if (log_dest == LOGTO_FILE)
				{
					log_dest = LOGTO_SYS;
					openlog (logident, LOG_PID, LOG_DAEMON);
					setlogmask (logmask);
				}
				
				if (level > LOG_DEBUG) {
					level = LOG_DEBUG;
				}
					
				sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
				kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
				strcat( kot, (char *) ap );
				syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throing errors. must revisit function
				vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
				closelog ();
				free( kot );
			}
			else
			{
				if ((lfp = fopen (logfile, "a")) == NULL)
				{
					char *kot = malloc( sizeof( char ) * strlen( err_str_ (save_errno, fmt, buf) ) + 1 );
					if (log_dest == LOGTO_FILE)
					{
						log_dest = LOGTO_SYS;
						openlog (logident, LOG_PID, LOG_DAEMON);
						setlogmask (logmask);
					}
					
					if (level > LOG_DEBUG) {
						level = LOG_DEBUG;
					}

					sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
					kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
					strcat( kot, (char *) ap );
					syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throing errors. must revisit function
					vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
					closelog ();
					free( kot );
				}
			}

			if (log_dest == LOGTO_SYS)
			{

				closelog ();
				log_dest = LOGTO_FILE;
			}

			errno = save_errno;
			verrlog_ (level, lfp, fmt, ap);
			fclose (lfp);
		}
	}
	else if ((logmask & LOG_MASK (level)) != 0)
	{
		char *kot = malloc( sizeof( char ) * strlen( err_str_ (save_errno, fmt, buf) ) + 1 );
		if (level > LOG_DEBUG) {
			level = LOG_DEBUG;
		}
		sprintf( kot, "%s", err_str_ (save_errno, fmt, buf) ); // FIXME FIXME FIXME FIXME use debugger, unroll format string
		kot = realloc( kot, strlen( kot ) + strlen( (char *) ap ) + 1 );  // FIXME FIXME FIXME FIXME FIXME variable argument again, must fix
		strcat( kot, (char *) ap );
		syslog( level, "%s", kot ); // FIXME FIXME FIXME FIXME the "%s" is a hack to go around the problem of cc throing errors. must revisit function
		vsyslog( level, "%s", ap ); // FIXME FIXME FIXME FIXME use debugger, unroll format string 
		closelog ();
		free( kot );
}

	free( buf );
	va_end (ap);
}

void
ls_closelog (void)
{
  if (log_dest == LOGTO_SYS)
	closelog ();
}

int
ls_setlogmask (int maskpri)
{
  int oldmask = logmask;

  logmask = maskpri;
  oldmask = setlogmask (logmask);

  return oldmask;
}

int
getLogMask (char **msg, char *logMask)
{
  static char msgbuf[MAX_LINE_LEN];

  *msg = NULL;

  if (logMask == NULL)
	return (LOG_UPTO (DEF_LOG_MASK));
#ifdef LOG_ALERT
  if (strcmp (logMask, "LOG_ALERT") == 0)
	return (LOG_UPTO (LOG_ALERT));
#endif

#ifdef LOG_SALERT
  if (strcmp (logMask, "LOG_SALERT") == 0)
	return (LOG_UPTO (LOG_SALERT));
#endif

#ifdef LOG_EMERG
  if (strcmp (logMask, "LOG_EMERG") == 0)
	return (LOG_UPTO (LOG_EMERG));
#endif

  if (strcmp (logMask, "LOG_ERR") == 0)
	return (LOG_UPTO (LOG_ERR));

#ifdef LOG_CRIT
  if (strcmp (logMask, "LOG_CRIT") == 0)
	return (LOG_UPTO (LOG_CRIT));
#endif

  if (strcmp (logMask, "LOG_WARNING") == 0)
	return (LOG_UPTO (LOG_WARNING));

  if (strcmp (logMask, "LOG_NOTICE") == 0)
	return (LOG_UPTO (LOG_NOTICE));

  if (strcmp (logMask, "LOG_INFO") == 0)
	return (LOG_UPTO (LOG_INFO));

  if (strcmp (logMask, "LOG_DEBUG") == 0)
	return (LOG_UPTO (LOG_DEBUG));

  if (strcmp (logMask, "LOG_DEBUG1") == 0)
	return (LOG_UPTO (LOG_DEBUG1));

  if (strcmp (logMask, "LOG_DEBUG2") == 0)
	return (LOG_UPTO (LOG_DEBUG2));

  if (strcmp (logMask, "LOG_DEBUG3") == 0)
	return (LOG_UPTO (LOG_DEBUG3));

  sprintf (msgbuf, "Invalid log mask %s defined, default to %s", logMask, DEF_LOG_MASK_NAME);

  *msg = msgbuf;

  return LOG_UPTO (DEF_LOG_MASK);
}

int
getLogClass_ (char *lsp, char *tsp)
{
  char *word;
  int class = 0;

  timinglevel = 0;
  logclass = 0;

  if (tsp != NULL && isint_ (tsp))
	timinglevel = atoi (tsp);

  while (lsp != NULL && (word = getNextWord_ (&lsp)))
	{
	  if (strcmp (word, "LC_SCHED") == 0)
	class |= LC_SCHED;
	  if (strcmp (word, "LC_PEND") == 0)
	class |= LC_PEND;
	  if (strcmp (word, "LC_JLIMIT") == 0)
	class |= LC_JLIMIT;
	  if (strcmp (word, "LC_EXEC") == 0)
	class |= LC_EXEC;
	  if (strcmp (word, "LC_TRACE") == 0)
	class |= LC_TRACE;
	  if (strcmp (word, "LC_COMM") == 0)
	class |= LC_COMM;
	  if (strcmp (word, "LC_XDR") == 0)
	class |= LC_XDR;
	  if (strcmp (word, "LC_CHKPNT") == 0)
	class |= LC_CHKPNT;
	  if (strcmp (word, "LC_FILE") == 0)
	class |= LC_FILE;
	  if (strcmp (word, "LC_AUTH") == 0)
	class |= LC_AUTH;
	  if (strcmp (word, "LC_HANG") == 0)
	class |= LC_HANG;
	  if (strcmp (word, "LC_SIGNAL") == 0)
	class |= LC_SIGNAL;
	  if (strcmp (word, "LC_PIM") == 0)
	class |= LC_PIM;
	  if (strcmp (word, "LC_SYS") == 0)
	class |= LC_SYS;
	  if (strcmp (word, "LC_LOADINDX") == 0)
	class |= LC_LOADINDX;
	  if (strcmp (word, "LC_JGRP") == 0)
	class |= LC_JGRP;
	  if (strcmp (word, "LC_JARRAY") == 0)
	class |= LC_JARRAY;
	  if (strcmp (word, "LC_MPI") == 0)
	class |= LC_MPI;
	  if (strcmp (word, "LC_ELIM") == 0)
	class |= LC_ELIM;
	  if (strcmp (word, "LC_M_LOG") == 0)
	class |= LC_M_LOG;
	  if (strcmp (word, "LC_PERFM") == 0)
	class |= LC_PERFM;
	}
  logclass = class;

  return 0;
}

void
ls_closelog_ext (void)
{
	logfile[0] = '\0';
}
