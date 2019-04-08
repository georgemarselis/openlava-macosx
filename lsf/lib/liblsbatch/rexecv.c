/* $Id: lsb.rexecv.c 397 2007-11-26 19:04:00Z mblack $
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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cmdtools/cmd.h"
#include "lsb/rexecv.h"


void
prtBETime2 (struct submit req)
{
  char sp[60];

  if (logclass & (LC_TRACE | LC_EXEC | LC_SCHED))
    ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);


  if (req.beginTime)
    {
      strcpy (sp, _i18n_ctime (ls_catd, CTIME_FORMAT_a_b_d_T_Y, &req.beginTime));
      /* catgets 800 */
      fprintf (stderr, "catgets 800: Job will be scheduled after %s\n", sp);
    }
  if (req.termTime)
    {
      strcpy (sp, _i18n_ctime (ls_catd, CTIME_FORMAT_a_b_d_T_Y, &req.termTime));
      /* catgets 801 */
      fprintf (stderr, "catgets 801: Job will be terminated by %s\n", sp);
    }
}

static int
CopyCommand2 (char **from, int len)
{
  char *arg;
  char *temP; 
  char *endArg;
  char *endArg1;
  char endChar = '\0';
  unsigned long size = 0;

  assert( len );

  for (int i = 0; from[i]; i++)
    {
      size += strlen (from[i]) + 1 + 4;
    }

  size += 1 + 1;

  commandline = (char *) malloc (size);
  if ( NULL == commandline && ENOMEM == errno )
    {
      /* catgets 802 */
      fprintf (stderr, "catgets 802: Unable to allocate memory for commands");
      return (FALSE);
    }

  strcpy (commandline, from[0]);

  for (int i = 1; from[i] != NULL; i++)
    {
      strcat (commandline, " ");

      if (strchr (from[i], ' '))
    {
      if (strchr (from[i], '"'))
        {
          strcat (commandline, "'");
          strcat (commandline, from[i]);
          strcat (commandline, "'");
        }
      else
        {
          strcat (commandline, "\"");
          strcat (commandline, from[i]);
          strcat (commandline, "\"");
        }
    }
      else
    {
      arg = putstr_ (from[i]);
      temP = arg;
      while (*arg)
        {
          endArg = strchr (arg, '"');
          endArg1 = strchr (arg, '\'');
          if (!endArg || (endArg1 && endArg > endArg1))
        endArg = endArg1;

          endArg1 = strchr (arg, '\\');
          if (!endArg || (endArg1 && endArg > endArg1))
        endArg = endArg1;
          if (endArg)
        {
          endChar = *endArg;
          *endArg = '\0';
        }
          strcat (commandline, arg);
          if (endArg)
        {
          arg += strlen (arg) + 1;
          if (endChar == '\\')
            strcat (commandline, "\\\\");
          else if (endChar == '"')
            strcat (commandline, "\\\"");
          else if (endChar == '\'')
            strcat (commandline, "\\\'");
        }
          else
        arg += strlen (arg);
        }
      free (temP);
    }
    }

  return TRUE;
}


void
prtErrMsg2 (struct submit *req, struct submitReply *reply)
{
  static char rNames[10][12] = {
    "CPULIMIT",
    "FILELIMIT",
    "DATALIMIT",
    "STACKLIMIT",
    "CORELIMIT",
    "MEMLIMIT",
    "",
    "",
    "",
    "RUNLIMIT"
  };
  switch (lsberrno)
    {
    case LSBE_BAD_QUEUE:
    case LSBE_QUEUE_USE:
    case LSBE_QUEUE_CLOSED:
    case LSBE_EXCLUSIVE:
      if (req->options & SUB_QUEUE)
    sub_perror (req->queue);
      else
    sub_perror (reply->queue);
      break;

    case LSBE_DEPEND_SYNTAX:
      sub_perror (req->dependCond);
      break;

    case LSBE_NO_JOB:
    case LSBE_JGRP_NULL:
    case LSBE_ARRAY_NULL:
      if (reply->badJobId)
    {
      char idStr[30];
      sprintf (idStr, "%s", lsb_jobidinstr (reply->badJobId));
      sub_perror (idStr);
    }
      else
    {
      if (strlen (reply->badJobName) == 0)
        {
          sub_perror (req->jobName);
        }
      else
        sub_perror (reply->badJobName);
    }
      break;
    case LSBE_QUEUE_HOST:
    case LSBE_BAD_HOST:
      sub_perror (req->askedHosts[reply->badReqIndx]);
      break;
    case LSBE_OVER_LIMIT:
      sub_perror (rNames[reply->badReqIndx]);
      break;

    case LSBE_BAD_HOST_SPEC:
      sub_perror (req->hostSpec);
      break;
    default:
      sub_perror (NULL);
      break;
    }

  return;

}

static int
parseLine2 (char *line, int *embedArgc, char ***embedArgv, int option)
{
#define INCREASE 40

  static int parsing = TRUE;
  static char **argBuf = NULL, *key;
  static int bufSize = 0;

  char *sp, *sQuote, *dQuote, quoteMark;

  if (argBuf == NULL)
    {
      if ((argBuf = (char **) malloc (INCREASE * sizeof (char *))) == NULL)
    {
      fprintf (stderr, "catgets 803: Unable to allocate memory for options");   /* catgets  803   */
      return (-1);
    }
      bufSize = INCREASE;
      *embedArgc = 1;
      *embedArgv = argBuf;

      if (option & EMBED_BSUB)
    {
      argBuf[0] = "bsub";
      key = "BSUB";
    }
      else if (option & EMBED_RESTART)
    {
      argBuf[0] = "brestart";
      key = "BRESTART";
    }
      else if (option & EMBED_QSUB)
    {
      argBuf[0] = "qsub";
      key = "QSUB";
    }
      else
    {
      fprintf (stderr, "catgets 804: Invalid option");  /* catgets  804   */
      return (-1);
    }
      argBuf[1] = NULL;
    }

  if (!parsing && !emptyCmd)
    return (0);

  SKIPSPACE (line);
  if (*line == '\0')
    return (0);
  if (*line != '#')
    {
      emptyCmd = FALSE;
      return (0);
    }

  if (!parsing)
    return (0);

  ++line;
  SKIPSPACE (line);
  if (strncmp (line, key, strlen (key)) == 0)
    {
      line += strlen (key);
      SKIPSPACE (line);
      if (*line != '-')
    {
      parsing = FALSE;
      return (0);
    }
      while (TRUE)
    {
      quoteMark = '"';
      if ((sQuote = strchr (line, '\'')) != NULL)
        if ((dQuote = strchr (line, '"')) == NULL || sQuote < dQuote)

          quoteMark = '\'';

      if ((sp = getNextValueQ_ (&line, quoteMark, quoteMark)) == NULL)
        return (0);

      if (*sp == '#')
        return (0);

      if (*embedArgc + 2 > bufSize)
        {
          char **tmp;
          bufSize += INCREASE;
        assert( bufSize >= 0 );
          tmp = (char **) realloc (argBuf, (unsigned long)bufSize * sizeof (char *));
        if (NULL == tmp && ENOMEM == errno )
        {
      /* catgets 803 */
          fprintf (stderr, "catgets 803: Unable to allocate memory for options\n");
          return (-1);
        }
          argBuf = tmp;
        }
      argBuf[*embedArgc] = putstr_ (sp);
      (*embedArgc)++;
      argBuf[*embedArgc] = NULL;
    }
    }
  return (0);

}

static int
parseScript2 (FILE * from, int *embedArgc, char ***embedArgv, int option)
{

  register int ttyin  = 0;
  // char *prompt  = NULL;
  char firstLine[MAX_LINE_LEN * 10];
  char line[MAX_LINE_LEN * 10];
  int notBourne         = FALSE;
  unsigned int length           = 0;
  unsigned long lineLen = 0;
  unsigned long size    = 10 * MAX_LINE_LEN;
  char *buf             = NULL;
  char *sp              = NULL;
  # define szTmpShellCommands "%s\n) > $LSB_CHKFILENAME.shell\nchmod u+x $LSB_CHKFILENAME.shell\n$LSB_JOBFILENAME.shell\nsaveExit=$?\n/bin/rm -f $LSB_JOBFILENAME.shell\n(exit $saveExit)\n" // FIXME FIXME FIXME this shit has to go

  if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC)) {
    ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
  }

/*  if (option & EMBED_BSUB) // FIXME FIXME what is this supposed to be ?
    prompt = "bsub> ";
*/
#ifdef QSUB
  else if (option & EMBED_QSUB)
    prompt = "qsub> ";
#endif

  if (option & EMBED_INTERACT)
    {
      firstLine[0] = '\0';
      if ((buf = malloc (size)) == NULL)
    {
    /* catgets 802 */
      fprintf (stderr, "catgets 802: Unable to allocate memory for commands\n");
      return (-1);
    }
      ttyin = isatty (fileno (from));
      if (ttyin)
    {
      printf("bsub> ");
      fflush(stdout);
    }
    }

  sp = line;
  lineLen = 0;
  while (fgets (sp, 10 * MAX_LINE_LEN - (int)lineLen - 1, from) != NULL)
    {
      lineLen = strlen (line);

      if (line[lineLen - 2] == '\\' && line[lineLen - 1] == '\n')
    {
      lineLen -= 2;
      sp = line + lineLen;
      continue;
    }
      if (parseLine2 (line, embedArgc, embedArgv, option) == -1)
    return (-1);

      if (option & EMBED_INTERACT)
    {
      if (!firstLine[0])
        {

          sprintf (firstLine, "( cat <<%s\n%s", SCRIPT_WORD, line);
          strcpy (line, firstLine);
          notBourne = TRUE;
        }
      lineLen = strlen (line);


      if (length + lineLen + 20 >= size)
        {
          size = size * 2;
          if ((sp = (char *) realloc (buf, size)) == NULL)
        {
          free (buf);
          /* catgets 807 */
      fprintf (stderr, "catgets 807: Unable to reallocate memory for commands\n");
          return (-1);
        }
          buf = sp;
        }
      for (unsigned int i = length, j = 0; j < lineLen; i++, j++) {
        buf[i] = line[j];
    }
      length += lineLen;


      if (ttyin)
        {
          printf ("bsub> ");
          fflush (stdout);
        }
    }
      sp = line;
      lineLen = 0;
    }

  if (option & EMBED_INTERACT)
    {
      buf[length] = '\0';
      if (firstLine[0] != '\n' && firstLine[0] != '\0')
    {


      if (notBourne == TRUE)
        {

          if (length + strlen (szTmpShellCommands) + 1 >= size)
        {
          size = size + strlen (szTmpShellCommands) + 1;
          if ((sp = (char *) realloc (buf, size)) == NULL)
            {
              free (buf);
              /* catgets 808 */
          fprintf (stderr, "catgets 808: Unable to reallocate memory for temp shell file"); 
              return (-1);
            }
          buf = sp;
        }
          sprintf (&buf[length], szTmpShellCommands, SCRIPT_WORD_END);
        }
    }
      commandline = buf;
    }
  return (0);

}

int
fillReq2 (int argc, char **argv, int operate, struct submit *req)
{

  struct stat statBuf;
  char *template = NULL, **embedArgv = NULL;
  int i = 0, embedArgc = 0, redirect = 0;
  int myArgc = 0;
  char *myArgv0 = NULL;

  if (logclass & (LC_TRACE | LC_EXEC | LC_SCHED)) {
    ls_syslog (LOG_DEBUG1, "%s: Entering this routine...", __func__);
  }


  if (operate == CMD_BRESTART)
    {
      req->options = SUB_RESTART;
      template = "E:w:B|f|x|N|h|m:q:b:t:c:W:F:D:S:C:M:V|G:";
    }
  else if (operate == CMD_BMODIFY)
    {
      req->options = SUB_MODIFY;
      template = "h|V|O|Tn|T:xn|x|rn|r|Bn|B|Nn|N|En|E:wn|w:fn|f:kn|k:Rn|R:mn|m:Jn|J:in|i:en|e:qn|q:bn|b:tn|t:sn|s:cn|c:Wn|W:Fn|F:Dn|D:Sn|S:Cn|C:Mn|M:on|o:nn|n:un|u:Pn|P:Ln|L:Gn|G:Xn|X:Z:";
    }
  else
    {
      req->options = 0;
      template = "E:T:w:f:k:R:m:J:L:u:i:o:e:n:q:b:t:s:c:v:p:W:F:D:S:C:M:O:P:Ip|Is|I|r|H|x|N|B|h|V|G:X:K|";
    }

  req->options2 = 0;
  commandline = "";

  myArgc = 0;
  myArgv0 = (char *) NULL;

  req->beginTime = 0;
  req->termTime = 0;
  req->command = NULL;
  req->nxf = 0;
  req->numProcessors = 0;
  req->maxNumProcessors = 0;
  for (i = 0; i < LSF_RLIM_NLIMITS; i++)
    req->rLimits[i] = DEFAULT_RLIMIT;
  req->hostSpec = NULL;
  req->resReq = NULL;
  req->loginShell = NULL;
  req->delOptions = 0;

  if ((req->projectName = getenv ("LSB_DEFAULTPROJECT")) != NULL)
    req->options |= SUB_PROJECT_NAME;


  optind = 1;
  if (setOption_ (argc, argv, template, req, ~0, NULL) == -1)
    return (-1);

  if (operate == CMD_BSUB && (req->options & SUB_INTERACTIVE)
      && (req->options & SUB_PTY))
    {
      if (req->options & SUB_PTY_SHELL)
    putenv (putstr_ ("LSB_SHMODE=y"));
      else
    putenv (putstr_ ("LSB_USEPTY=y"));
    }

  if (fstat (0, &statBuf) == 0)

    if ((statBuf.st_mode & S_IFREG) == S_IFREG || (statBuf.st_mode & S_IFLNK) == S_IFLNK)
      {

    redirect = (ftell (stdin) == 0) ? 1 : 0;
      }

  if (myArgc > 0 && myArgv0 != NULL)
    {
      emptyCmd = FALSE;
      argv[argc] = myArgv0;
      if (!CopyCommand2 (&argv[argc], myArgc))
    return (-1);
    }
  else if (argc >= optind + 1)
    {

      emptyCmd = FALSE;
      if (!CopyCommand2 (argv + optind, argc - optind - 1))
    return (-1);
    }
  else
    if (parseScript2 (stdin, &embedArgc, &embedArgv,
              EMBED_INTERACT | EMBED_BSUB) == -1)
    return (-1);

  req->command = commandline;
  SKIPSPACE (req->command);
  if (emptyCmd)
    {
      if (redirect)
    fprintf (stderr, "catgets 809: No command is specified in the script file");    /* catgets  809   */
      else
    fprintf (stderr, "catgets 810: No command is specified");   /* catgets  810   */
      return (-1);
    }

  if (embedArgc > 1 && operate == CMD_BSUB)
    {

      optind = 1;
      if (setOption_
      (embedArgc, embedArgv, template, req, ~req->options, NULL) == -1)
    return (-1);
    }

  if (optionFlag)
    {
      if (parseOptFile_ (optionFileName, req, NULL) == NULL)
    return (-1);
      optionFlag = FALSE;
    }
  return 0;
}

size_t
lsb_rexecv (int argc, char **argv, char **env, int *fds, int options)
{
    struct submit req;
    struct submitReply reply;
    pid_t pid;
    size_t jobId;
    int jobIdFd[2];

    assert( **env );
    assert( options );

    fflush (stdout);
    setbuf (stdout, NULL);

    memset (&req, 0, sizeof (struct submit));
    if (fillReq2 (argc, argv, CMD_BSUB, &req) < 0)  {
        lsberrno = LSBE_BAD_ARG;
        return (-1);
    }

    if (!(req.options & SUB_INTERACTIVE)) {
        lsberrno = LSBE_ONLY_INTERACTIVE;
        return (-1);
    }

    if (socketpair (AF_UNIX, SOCK_STREAM, 0, jobIdFd) < 0) {
        lsberrno = LSBE_SYS_CALL;
        return (-1);
    }

    if ((pid = fork ()) < 0) {
        close (jobIdFd[0]);
        close (jobIdFd[1]);
        lsberrno = LSBE_NO_FORK;
        return (-1);
    }
    
    if (pid == 0) {

        if ((pid = fork ()) < 0) {

            close (jobIdFd[0]);
            close (jobIdFd[1]);
            lsberrno = LSBE_NO_FORK;
            return (-1);
        }

        if (pid != 0)  {
            _exit (0);
        }
    }

    if (pid == 0) {

        char envBuf[128];
        close (jobIdFd[0]);
        if (fds) {
            for (int i = 0; i < 3; i++) {
                if (FD_IS_VALID (fds[i])) {
                    dup2 (fds[i], i);
                }
            }
        }

        sprintf (envBuf, "JOBID_FD=%d", jobIdFd[1]);
        putenv (envBuf);

        memset (&reply, 0, sizeof (struct submitReply));
        //TIMEIT (0, (
        jobId = lsb_submit (&req, &reply);
        // ), "lsb_submit");

        if( jobId < 0 ) {
            jobId = -lsberrno;
        }

        b_write_fix (jobIdFd[1], (char *) &jobId, sizeof (jobId));
        close (jobIdFd[1]);
        _exit (1);
    }
    else {
        long temp  = 0;
        unsigned long len = 0;

        close (jobIdFd[1]);

        temp = b_read_fix(jobIdFd[0], (char *) &jobId, sizeof (jobId));
        assert( temp >= 0 );
        len = (unsigned long) temp;

        if (len != sizeof (jobId)) {

            if (pid == waitpid (pid, NULL, 0)) {
               lsberrno = LSBE_PREMATURE;
            }
            else {
               lsberrno = LSBE_LSLIB;
               kill (pid, SIGTERM);
            }

           close (jobIdFd[0]);
           return (-1);
        }

        if (jobId < 0) {
            assert( jobId >= INT_MIN );
            lsberrno = (int)-jobId;
            close (jobIdFd[0]);
            return jobId;
        }
        else if (!(req.options & SUB_INTERACTIVE)) {
            lsberrno = LSBE_NO_ERROR;
            close (jobIdFd[0]);
            return jobId;
        }

        if (b_write_fix (jobIdFd[0], (char *) &pid, sizeof (pid)) != sizeof (pid))  {
            lsberrno = LSBE_SYS_CALL;
            kill (pid, SIGTERM);
            close (jobIdFd[0]);
            return (-1);
        }
    }

    lsberrno = LSBE_NO_ERROR;
    close (jobIdFd[0]);
    waitpid (pid, NULL, 0);

    return jobId;
}
