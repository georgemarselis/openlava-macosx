/* $Id: echkpnt.c 397 2007-11-26 19:04:00Z mblack $
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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>

#include "echkpnt.lib.h"
#include "echkpnt.env.h"

#define LSTMPDIR        lsTmpDir_

void echkpnt_usage (const char *);

static char logMesgBuf[MAX_LINE_LEN];
extern char *lsTmpDir_;

static void
echkpnt_usage (const char *pCmd)
{
  fprintf (stderr, "Usage: %s [-c] [-f] [-k | -s] [-x] [-d chkdir] [-h] [-V] pid\n", Cmd);
}


int
main (int argc, char **argv)
{

  static char __func__] = "main()";


  char *pMethodName = NULL;
  char *pMethodDir = NULL;
  char *pIsOutput = NULL;
  char *pJobID = NULL;

  char *presAcctFN = NULL;
  char resAcctFN[MAX_FILENAME_LEN];

  char echkpntProgPath[MAX_PATH_LEN];
  char *pChkpntDir = NULL;
  pid_t iChildPid;
  LS_WAIT_T iStatus;
  char *cargv[MAX_ARGS];
  int argIndex = 0;
  int cc;
  int iReValue;
  char *jf, *jfn;


  while ((cc = getopt (argc, argv, "cfksd:Vhx")) != EOF)
    {
      switch (cc)
	{
	case 'c':
	case 'f':
	case 'k':
	case 'x':
	case 's':
	  break;
	case 'V':
	  fputs (_LS_VERSION_, stderr);
	  exit (0);
	case 'd':
	  pChkpntDir = optarg;
	  break;
	case 'h':
	  usage (argv[0]);
	  exit (0);
	default:
	  usage (argv[0]);
	  exit (-1);
	}
    }


  if (pChkpntDir == NULL)
    {
      usage (argv[0]);
      exit (-1);
    }

  if (access (pChkpntDir, W_OK | X_OK) < 0)
    {
      fprintf (stderr, "%s : the chkpnt dir %s can not be accessed\n",
	       __func__, pChkpntDir);
      exit (-1);
    }


  iReValue = fileIsExist (pChkpntDir);

  if (iReValue == 1)
    {


      pMethodName = getEchkpntVar (ECHKPNT_METHOD);

      if ((pMethodName == NULL) || (strcmp (pMethodName, "") == 0))
	{
	  pMethodName = ECHKPNT_DEFAULT_METHOD;
	}

      pMethodDir = getEchkpntVar (ECHKPNT_METHOD_DIR);
      pIsOutput = getEchkpntVar (ECHKPNT_KEEP_OUTPUT);

    }
  else if (iReValue == 0)
    {


      initenv_ (NULL, NULL);
      pMethodName = getenv (ECHKPNT_METHOD);

      if ((pMethodName == NULL) || (strcmp (pMethodName, "") == 0))
	{
	  pMethodName = ECHKPNT_DEFAULT_METHOD;
	}
      pMethodDir = getenv (ECHKPNT_METHOD_DIR);
      pIsOutput = getenv (ECHKPNT_KEEP_OUTPUT);

      {

	jf = getenv ("LSB_JOBFILENAME");
	if (jf == NULL)
	  {
	    fprintf (stderr, "%s : getenv of LSB_JOBFILENAME failed", __func__);
	  }
	else
	  {
	    jfn = strchr (jf, '/');
	    if (jfn)
	      {
		sprintf (resAcctFN, "%s/.%s.acct", LSTMPDIR, jfn + 1);
		presAcctFN = (char *) resAcctFN;
	      }
	  }
      }

      writeEchkpntVar (ECHKPNT_METHOD, pMethodName);
      writeEchkpntVar (ECHKPNT_METHOD_DIR, pMethodDir);
      writeEchkpntVar (ECHKPNT_KEEP_OUTPUT, pIsOutput);
      writeEchkpntVar (ECHKPNT_ACCT_FILE, presAcctFN);
    }
  else
    {
      fprintf (stderr, "%s : the .echkpnt file content occurs error: %s\n",
	       __func__, strerror (errno));
      exit (-1);
    }


  if ((pIsOutput != NULL) && ((strcmp (pIsOutput, ECHKPNT_OPEN_OUTPUT) == 0)
			      || (strcmp (pIsOutput, ECHKPNT_OPEN_OUTPUT_L) ==
				  0)))
    {
      initLog ("Echkpnt");
    }

#ifdef DEBUG
  sprintf (logMesgBuf, "%s : the LSB_ECHKPNT_METHOD = %s\n", __func__,
	   pMethodName);
  logMesg (logMesgBuf);
  sprintf (logMesgBuf, "%s : the LSB_ECHKPNT_METHOD_DIR = %s\n", __func__,
	   pMethodDir != NULL ? pMethodDir : "");
  logMesg (logMesgBuf);
#endif


  if (getEchkpntMethodDir
      (echkpntProgPath, pMethodDir, ECHKPNT_PROGRAM, pMethodName) == NULL)
    {
      sprintf (logMesgBuf,
	       "%s : the echkpnt method(%s) path is not correct\n", __func__,
	       pMethodName);
      goto Error;
    }

#ifdef DEBUG
  sprintf (logMesgBuf, "%s : the echkpntProgPath is : %s\n", __func__,
	   echkpntProgPath);
  logMesg (logMesgBuf);
#endif

  for (argIndex = 0; argIndex < argc; argIndex++)
    {
      cargv[argIndex] = argv[argIndex];
    }
  cargv[argIndex] = NULL;


  if (strcmp (pMethodName, ECHKPNT_DEFAULT_METHOD) == 0)
    {
      cargv[0] = "echkpnt.default";
      freeTable_ ();

#ifdef DEBUG
      logMesg ("the echkpnt.default will be executed\n");
#endif
      closeLog ();
      execv (echkpntProgPath, cargv);
      sprintf (logMesgBuf, "%s : execute the echkpnt.default fail\n%s\n",
	       __func__, (errno ? strerror (errno) : ""));
      fprintf (stderr, "%s", logMesgBuf);
      exit (-1);
    }


  iChildPid = fork ();
  if (iChildPid < 0)
    {
      sprintf (logMesgBuf, "%s : fork() fork a child process fail...\n",
	       __func__);
      goto Error;


    }
  else if (iChildPid == 0)
    {
      long lMaxfds;
      int ii;
      char progName[MAX_FILENAME_LEN];


      sprintf (logMesgBuf, "erestart.%s", pMethodName);
      setMesgHeader (logMesgBuf);


      if ((pIsOutput == NULL)
	  || ((strcmp (pIsOutput, ECHKPNT_OPEN_OUTPUT) != 0)
	      && (strcmp (pIsOutput, ECHKPNT_OPEN_OUTPUT_L) != 0)))
	{
	  if (redirectFd (ECHKPNT_DEFAULT_OUTPUT_FILE, 1) == -1)
	    {
	      sprintf (logMesgBuf, "%s : redirect stdout to %s file\n%s\n",
		       __func__, ECHKPNT_DEFAULT_OUTPUT_FILE,
		       errno ? strerror (errno) : "");
	      goto Error;
	    }
	  if (redirectFd (ECHKPNT_DEFAULT_OUTPUT_FILE, 2) == -1)
	    {
	      sprintf (logMesgBuf, "%s : redirect stderr to %s file\n%s\n",
		       __func__, ECHKPNT_DEFAULT_OUTPUT_FILE,
		       errno ? strerror (errno) : "");
	      goto Error;
	    }

	}
      else
	{
	  char aFileName[MAX_PATH_LEN];



	  if ((getChkpntDirFile (aFileName, ECHKPNT_STDOUT_FILE) == -1)
	      || (redirectFd (aFileName, 1) == -1))
	    {
	      sprintf (logMesgBuf, "%s : redirect the stdout to %s fail\n",
		       __func__, ECHKPNT_STDOUT_FILE);
	      logMesg (logMesgBuf);

	      if (redirectFd (ECHKPNT_DEFAULT_OUTPUT_FILE, 1) == -1)
		{
		  sprintf (logMesgBuf,
			   "%s : redirect stdout to %s file fail\n%s\n",
			   __func__, ECHKPNT_DEFAULT_OUTPUT_FILE,
			   errno ? strerror (errno) : "");
		  goto Error;
		}
	    }

	  if ((getChkpntDirFile (aFileName, ECHKPNT_STDERR_FILE) == -1)
	      || (redirectFd (aFileName, 2) == -1))
	    {
	      sprintf (logMesgBuf, "%s : redirect the stderr to %s fail\n",
		       __func__, ERESTART_STDERR_FILE);
	      logMesg (logMesgBuf);

	      if (redirectFd (ECHKPNT_DEFAULT_OUTPUT_FILE, 2) == -1)
		{
		  sprintf (logMesgBuf,
			   "%s : redirect stderr to %s file fail\n%s\n",
			   __func__, ECHKPNT_DEFAULT_OUTPUT_FILE,
			   errno ? strerror (errno) : "");
		  goto Error;
		}
	    }
	}

      lMaxfds = sysconf (_SC_OPEN_MAX);
      for (ii = 3; ii < lMaxfds; ii++)
	{
	  close (ii);
	}

      sprintf (progName, "%s.%s", ECHKPNT_PROGRAM, pMethodName);
      cargv[0] = progName;
      freeTable_ ();

      execv (echkpntProgPath, cargv);
      sprintf (logMesgBuf, "%s : the child process execute the %s fail\n",
	       __func__, progName);
      fprintf (stderr, "%s", logMesgBuf);
      logMesg (logMesgBuf);
      closeLog ();
      exit (-1);
    }




  while ((iChildPid = waitpid (iChildPid, &iStatus, 0)) < 0
	 && errno == EINTR);

  if (iChildPid < 0)
    {
      sprintf (logMesgBuf, "%s : %s fail, \n%s\n",
	       __func__, "waitpid", errno ? strerror (errno) : "");
      goto Error;
    }
  else
    {
      if (WEXITSTATUS (iStatus) != 0)
	{
	  sprintf (logMesgBuf,
		   "%s : the echkpnt.%s fail,the exit value is %d\n", __func__,
		   pMethodName, WEXITSTATUS (iStatus));
	  fprintf (stderr, "%s", logMesgBuf);
	  logMesg (logMesgBuf);
	  freeTable_ ();
	  closeLog ();
	  exit (WEXITSTATUS (iStatus));
	}
    }


  pJobID = getenv (ECHKPNT_JOBID);
  if (pJobID == NULL)
    {
      writeEchkpntVar (ECHKPNT_OLD_JOBID, "");
      sprintf (logMesgBuf,
	       "%s : getenv() can not get the env variable LSB_JOBID\n",
	       __func__);
      logMesg (logMesgBuf);
    }
  else
    {
      writeEchkpntVar (ECHKPNT_OLD_JOBID, pJobID);
    }
  freeTable_ ();
  closeLog ();
  exit (0);

Error:

  fprintf (stderr, "%s", logMesgBuf);
  logMesg (logMesgBuf);
  freeTable_ ();
  closeLog ();
  exit (-1);

}
