/* $Id: lim.rload.c 397 2007-11-26 19:04:00Z mblack $
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


#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include <utmpx.h>

#include "daemons/liblimd/rload.h"
#include "daemons/liblimd/limd.h"
#include "lib/lproto.h"
#include "lib/mls.h"
#include "lsf.h"


void
satIndex (void)
{
    for ( uint i = 0; i < allInfo.numIndx; i++) {
       li[i].satvalue = myHostPtr->busyThreshold[i];
    }

    return;
}

void
loadIndex (void)
{
    li[R15S].exchthreshold += 0.05 * (myHostPtr->statInfo.maxCpus - 1);
    li[R1M].exchthreshold += 0.04 * (myHostPtr->statInfo.maxCpus - 1);
    li[R15M].exchthreshold += 0.03 * (myHostPtr->statInfo.maxCpus - 1);
}

void
smooth (float *val, float instant, float factor)
{
  (*val) = ((*val) * factor) + (instant * (1 - factor));

}

time_t
getutime (char *usert)
{
    struct stat ttystatus = { };
    char *buffer = malloc( sizeof( char ) * MAXPATHLEN + 1);
    time_t lastinputtime = 0;
    time_t t = 0;


    if (strchr (usert, ':') != NULL) {
        return (MAXIDLETIME);
    }
  
    strcpy (buffer, "/dev/"); // FIXME FIXME FIXME FIXME remove fixed strings
    strcat (buffer, usert);

    if (stat (buffer, &ttystatus) < 0)
    {
        ls_syslog (LOG_DEBUG, "getutime: stat(%s) failed: %m", buffer);
        return MAXIDLETIME;
    }
    lastinputtime = ttystatus.st_atime;

    time (&t);
    if (t < lastinputtime) {
        return (time_t) 0;
    }
    else {
        return (t - lastinputtime);
    }
}

void
putLastActiveTime ()
{
  char lsfLastActiveTime[MAXLINELEN];

  sprintf (lsfLastActiveTime, "%ld", lastActiveTime);
  if (putEnv (ENV_LAST_ACTIVE_TIME, lsfLastActiveTime) != 0)
    {
      /* catgets 5902 */
      ls_syslog (LOG_WARNING, "5902: putLastActiveTime: %s, failed.", lsfLastActiveTime);
    }
}

void
getLastActiveTime ()
{
  char *lsfLastActiveTime = NULL;

  lsfLastActiveTime = getenv (ENV_LAST_ACTIVE_TIME);


  if (lsfLastActiveTime != NULL && lsfLastActiveTime[0] != '\0')
    {
      lastActiveTime = (time_t) atol (lsfLastActiveTime);


      if (lastActiveTime < 0)
	{
	  time (&lastActiveTime);
	}


      putEnv (ENV_LAST_ACTIVE_TIME, "");
    }
  else
    {

      time (&lastActiveTime);
    }
}

float
idletime (int *logins)
{
  time_t itime;
  time_t idleSeconds;
  int idcount;
  int last_logins;
  time_t currentTime;
  int numusers;
  int ufd;
// FIXME FIXME FIXME future compatibility point
#ifndef __APPLE__
  struct utmp user;   // FIXME FIXME FIXME FIXME FIXME set feature automatically, according to OS
#else
  struct utmpx user;
#endif
  char **users;
  char excused_ls = FALSE;
  size_t listsize = GUESS_NUM; // 30, in rload.h
  char *thisHostname;
  int i;
  bool_t firstLoop;

  if ((thisHostname = ls_getmyhostname ()) == NULL)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_getmyhostname");
      thisHostname = "localhost";
    }


  time (&currentTime);
  idleSeconds = currentTime - lastActiveTime;
  if (idleSeconds < 0)
    {
      idleSeconds = 0;
    }

  if (idcount >= (IDLE_INTVL / exchIntvl))
    idcount = 0;

  idcount++;
  if (idcount != 1)
    {
      *logins = last_logins;
      return (idleSeconds / 60.0);
    }

    // FIXME FIXME FIXME future compatibility point
    if ((ufd = open (UTMPX_FILE, O_RDONLY)) < 0)
    {
        ls_syslog (LOG_WARNING, I18N_FUNC_S_FAIL_M, __func__, "open", UTMPX_FILE);
        *logins = last_logins;
        return (MAXIDLETIME / 60.0);
    }

  numusers = 0;
  users = calloc (listsize, sizeof (char *));
  if (users == NULL)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
      excused_ls = TRUE;
    }

  firstLoop = TRUE;

    while (read (ufd, (char *) &user, sizeof user) == sizeof user)
    {
        // FIXME FIXME FIXME future compatibility point; ut_user in macosx vs ut_name
        if (user.ut_user[0] == 0 || nonuser (user)) { // FIXME FIXME FIXME FIXME test case to fail; what is nonuser(user)? 
            continue;
        }
        else
        {
        // FIXME FIXME FIXME future compatibility point
            char *ut_user;
            if (! (ut_user = malloc ((sizeof (user.ut_user) + 1) * sizeof (char))))
            {
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
                lim_Exit (__func__);
            }

            // FIXME FIXME FIXME future compatibility point
            memset (ut_user, '\0', (sizeof (user.ut_user) + 1));
            strncpy (ut_user, user.ut_user, sizeof (user.ut_user));
            ut_user[sizeof (user.ut_user)] = '\0';

	  if (!excused_ls)
	    {
	      for (i = 0; i < numusers; i++)
		{
		  if (strcmp (ut_user, users[i]) == 0)
		    break;
		}
	      if (i >= numusers)
		{

		  users[numusers] = putstr_ (ut_user);
		  if (users[numusers] == NULL)
		    {
		      ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "putstr_", ut_user);
                excused_ls = TRUE;
                for (i = 0; i < numusers; i++) {
                    FREEUP (users[i]);
                }
		      FREEUP (users);
		    }
		  else
		    {
		      numusers++;
		      if (numusers >= listsize)
			{

			  char **sp;
			  listsize = 2 * listsize;
			  sp = realloc (users, listsize * sizeof (char *));
			  if (sp == NULL)
			    {
			      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "realloc");
                    for (i = 0; i < numusers; i++) {
                        FREEUP (users[i]);
                    }
			      FREEUP (users);
			      excused_ls = TRUE;
			    }
			  else
			    {
			      users = sp;
			    }
			}
		    }
		}
	    }

	  user.ut_line[ strlen(user.ut_line) - 1 ] = '\0'; // FIXME FIXME FIXME FIXME alternatively use const _UTX_LINESIZE

        if (idleSeconds > 0)
	    {
            itime = getutime (user.ut_line);
            if (firstLoop == TRUE)
            {
                idleSeconds = itime;
                firstLoop = FALSE;
            }
            else
            {
                if (itime < idleSeconds) {
                    idleSeconds = itime;
                }
            }
	    }
	  FREEUP (ut_user);
	}
    }
  close (ufd);

    if (idleSeconds > 0 && (itime = getXIdle ()) < idleSeconds) {
        idleSeconds = itime;
    }

    if (excused_ls)
        *logins = last_logins;
    else
    {
        *logins = numusers;
        last_logins = numusers;
        for (i = 0; i < numusers; i++) {
            FREEUP (users[i]);
        }
        FREEUP (users);
    }

  time (&currentTime);
  if ((currentTime - idleSeconds) > lastActiveTime)
    {
      lastActiveTime = currentTime - idleSeconds;
    }
  else
    {
      idleSeconds = currentTime - lastActiveTime;
    }
  return (idleSeconds / 60.0);
}

time_t
getXIdle ()
{
    time_t lastTime = 0;
  time_t t;
  struct stat st;

  if (stat ("/dev/kbd", &st) == 0) // FIXME FIXME FIXME FIXME remove fixed string
    {
      if (lastTime < st.st_atime)
	lastTime = st.st_atime;
    }
  else
    {
      if (errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "stat", "/dev/kbd");
    }

  if (stat ("/dev/mouse", &st) == 0)
    {
      if (lastTime < st.st_atime)
	lastTime = st.st_atime;
    }
  else
    {
      if (errno != ENOENT)
	ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "stat", "/dev/mouse");
    }

  time (&t);
  if (t < lastTime)
    return 0;
  else
    return (t - lastTime);
}

void
readLoad (int kernelPerm)
{
  int i;
  int busyBits = 0;
  double etime;
  double itime;
  int readCount0 = 10000;
  int readCount1 = 5;
  int readCount2 = 1500;
  float avrun15 = 0.0;
  float ql;
  float avrun1m = 0.0;
  float avrun15m = 0.0;
  float smpages;
  float smkbps;
  float extrafactor;
  float swap;
  float instant_ut;
  int loginses;

  TIMEIT (0, getusr (), "getusr()");

  if (kernelPerm < 0)
    goto checkOverRide;

  if (++readCount0 < (5.0 / sampleIntvl))
    {
      goto checkExchange;
    }

  readCount0 = 0;

  if (queueLengthEx (&avrun15, &avrun1m, &avrun15m) < 0)
    {

      ql = queueLength ();
      smooth (&avrun15, ql, EXP3);
      smooth (&avrun1m, ql, EXP12);
      smooth (&avrun15m, ql, EXP180);
    }

  if (++readCount1 < 3)
    goto checkExchange;

  readCount1 = 0;

  cpuTime (&itime, &etime);

  instant_ut = 1.0 - itime / etime;
  smooth (&cpu_usage, instant_ut, EXP4);
  etime /= k_hz;
  etime = etime / ncpus;

  TIMEIT (0, smpages = getpaging (etime), "getpaging");
  TIMEIT (0, smkbps = getIoRate (etime), "getIoRate");
  TIMEIT (0, myHostPtr->loadIndex[IT] = idletime (&loginses), "idletime");
  TIMEIT (0, swap = getswap (), "getswap");
  TIMEIT (0, myHostPtr->loadIndex[TMP] = tmpspace (), "tmpspace");
  TIMEIT (0, myHostPtr->loadIndex[MEM] = realMem (0.0), "realMem");

checkOverRide:
  if (overRide[UT] < INFINIT_LOAD)
    cpu_usage = overRide[UT];

  if (overRide[PG] < INFINIT_LOAD)
    smpages = overRide[PG];

  if (overRide[IO] < INFINIT_LOAD)
    smkbps = overRide[IO];

  if (overRide[IT] < INFINIT_LOAD)
    myHostPtr->loadIndex[IT] = overRide[IT];

  if (overRide[SWP] < INFINIT_LOAD)
    swap = overRide[SWP];

  if (overRide[TMP] < INFINIT_LOAD)
    myHostPtr->loadIndex[TMP] = overRide[TMP];

  if (overRide[R15S] < INFINIT_LOAD)
    avrun15 = overRide[R15S];

  if (overRide[R15S] < INFINIT_LOAD)
    avrun15 = overRide[R15S];

  if (overRide[R1M] < INFINIT_LOAD)
    avrun1m = overRide[R1M];

  if (overRide[R15M] < INFINIT_LOAD)
    avrun15m = overRide[R15M];

  if (overRide[LS] < INFINIT_LOAD)
    loginses = overRide[LS];

  if (overRide[MEM] < INFINIT_LOAD)
    myHostPtr->loadIndex[MEM] = overRide[MEM];

checkExchange:

  if (++readCount2 < (exchIntvl / sampleIntvl))
    return;
  readCount2 = 0;

  extrafactor = 0;
  if (jobxfer)
    {
      extrafactor = jobxfer / keepTime;
      jobxfer--;
    }
  myHostPtr->loadIndex[R15S] = avrun15 + extraload[R15S] * extrafactor;
  myHostPtr->loadIndex[R1M] = avrun1m + extraload[R1M] * extrafactor;
  myHostPtr->loadIndex[R15M] = avrun15m + extraload[R15M] * extrafactor;

  myHostPtr->loadIndex[UT] = cpu_usage + extraload[UT] * extrafactor;
  if (myHostPtr->loadIndex[UT] > 1.0)
    myHostPtr->loadIndex[UT] = 1.0;
  myHostPtr->loadIndex[PG] = smpages + extraload[PG] * extrafactor;
  if (myHostPtr->statInfo.nDisks)
    myHostPtr->loadIndex[IO] = smkbps + extraload[IO] * extrafactor;
  else
    myHostPtr->loadIndex[IO] = smkbps;
  myHostPtr->loadIndex[LS] = loginses + extraload[LS] * extrafactor;
  myHostPtr->loadIndex[IT] += extraload[IT] * extrafactor;
  if (myHostPtr->loadIndex[IT] < 0)
    myHostPtr->loadIndex[IT] = 0;
  myHostPtr->loadIndex[SWP] = swap + extraload[SWP] * extrafactor;
  if (myHostPtr->loadIndex[SWP] < 0)
    myHostPtr->loadIndex[SWP] = 0;
  myHostPtr->loadIndex[TMP] += extraload[TMP] * extrafactor;
  if (myHostPtr->loadIndex[TMP] < 0)
    myHostPtr->loadIndex[TMP] = 0;

  myHostPtr->loadIndex[MEM] += extraload[MEM] * extrafactor;
  if (myHostPtr->loadIndex[MEM] < 0)
    myHostPtr->loadIndex[MEM] = 0;

  for (i = 0; i < allInfo.numIndx; i++)
    {
      if (i == R15S || i == R1M || i == R15M)
	{

	  li[i].value = normalizeRq (myHostPtr->loadIndex[i], 1, ncpus) - 1;
	}
      else
	{
	  li[i].value = myHostPtr->loadIndex[i];
	}
    }

  for (i = 0; i < allInfo.numIndx; i++)
    {

      if ((li[i].increasing && fabs (li[i].value - INFINIT_LOAD) < 1.0)
	  || (!li[i].increasing && fabs (li[i].value + INFINIT_LOAD) < 1.0))
	{
	  continue;
	}

      if (!THRLDOK (li[i].increasing, li[i].value, li[i].satvalue))
	{
	  SET_BIT (i + INTEGER_BITS, myHostPtr->status);
	  myHostPtr->status[0] |= LIM_BUSY;
	}
      else
	CLEAR_BIT (i + INTEGER_BITS, myHostPtr->status);

    }
  for (i = 0; i < GET_INTNUM (allInfo.numIndx); i++)
    busyBits += myHostPtr->status[i + 1];
  if (!busyBits)
    myHostPtr->status[0] &= ~LIM_BUSY;

  if (LOCK_BY_USER (limLock.on))
    {
      if (time (0) > limLock.time)
	{
	  limLock.on &= ~LIM_LOCK_STAT_USER;
	  limLock.time = 0;
	  mustSendLoad = TRUE;
	  myHostPtr->status[0] = myHostPtr->status[0] & ~LIM_LOCKEDU;
	}
      else
	{
	  myHostPtr->status[0] = myHostPtr->status[0] | LIM_LOCKEDU;
	}
    }

  myHostPtr->loadMask = 0;

  TIMEIT (0, sendLoad (), "sendLoad()");

  for (i = 0; i < allInfo.numIndx; i++)
    {

      if (myHostPtr->loadIndex[i] < MIN_FLOAT16 && i < NBUILTINDEX)
	{
	  myHostPtr->loadIndex[i] = 0.0;
	}

      if (i == R15S || i == R1M || i == R15M)
	{
	  float rawql;

	  rawql = myHostPtr->loadIndex[i];
	  myHostPtr->loadIndex[i]
	    = normalizeRq (rawql,
			   (myHostPtr->hModelNo >= 0) ?
			   shortInfo.cpuFactors[myHostPtr->hModelNo] : 1.0,
			   ncpus);
	  myHostPtr->uloadIndex[i] = rawql;
	}
      else
	{
	  myHostPtr->uloadIndex[i] = myHostPtr->loadIndex[i];
	}
    }

  return;
}

FILE *
lim_popen (char **argv, char *mode)
{
    int p[2], pid, i;

  if (mode[0] != 'r')
    return (NULL);

    if (pipe (p) < 0) {
        return (NULL);
    }

    if ((pid = fork ()) == 0)
    {
        char *resEnv;
        resEnv = getElimRes ();
        if (resEnv != NULL)
        {
            if (logclass & LC_TRACE) {
               ls_syslog (LOG_DEBUG, "lim_popen: LS_ELIM_RESOURCES <%s>", resEnv);
            }
            putEnv ("LS_ELIM_RESOURCES", resEnv);
        }
        close (p[0]);
        dup2 (p[1], 1);

        alarm (0);

        for (i = 2; i < sysconf (_SC_OPEN_MAX); i++) {
            close (i);
        }
        for (i = 1; i < NSIG; i++) {
            Signal_ (i, SIG_DFL);
        }
        lsfExecX(argv[0], argv, execvp); // FIXME FIXME FIXME FIXME sanitize argv[0]; name argv[0]
        // lsfExecvp (argv[0], argv);
        ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "execvp", argv[0]);
        exit (127);
    }

    if (pid == -1)
    {
      close (p[0]);
      close (p[1]);
      return NULL;
    }

    elim_pid = pid;
    close (p[1]);

    return fdopen (p[0], mode);
}

int
lim_pclose (FILE * ptr)
{
  sigset_t omask;
  sigset_t newmask;
  pid_t child;

  child = elim_pid;
  elim_pid = -1;

  if (ptr)
    fclose (ptr);

  if (child == -1)
    return (-1);

  kill (child, SIGTERM);

  sigemptyset (&newmask);
  sigaddset (&newmask, SIGINT);
  sigaddset (&newmask, SIGQUIT);
  sigaddset (&newmask, SIGHUP);
  sigprocmask (SIG_BLOCK, &newmask, &omask);

  sigprocmask (SIG_SETMASK, &omask, NULL);

  return (0);
}

int
saveIndx (char *name, float value)
{
    char **names;
  int indx, i;

  if (!names)
    {
      if (!(names = malloc ((allInfo.numIndx + 1) * sizeof (char *))))
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
	  lim_Exit (__func__);
        }
        memset (names, 0, (allInfo.numIndx + 1) * sizeof (char *));
    }
    indx = getResEntry (name);

    if (indx < 0)
    {

        for (i = NBUILTINDEX; names[i] && i < allInfo.numIndx; i++)
        {
            if (strcmp (name, names[i]) == 0) {
                return 0;
            }
        }
        /* catgets 5920 */
        ls_syslog (LOG_ERR, "5920: %s: Unknown index name %s from ELIM", __func__, name);
        if (names[i])
        {
            FREEUP (names[i]);
        }
        names[i] = putstr_ (name);
        return 0;
    }

    if (allInfo.resTable[indx].valueType != LS_NUMERIC  || indx >= allInfo.numIndx)
    {
      return (0);
    }

    if (indx < NBUILTINDEX)
    {
        if (!names[indx])
        {
            names[indx] = allInfo.resTable[indx].name;
            /* catgets 5921 */
            ls_syslog (LOG_WARNING, "5921: %s: ELIM over-riding value of index %s", __func__, name);
        }
        overRide[indx] = value;
    }
    else {
        myHostPtr->loadIndex[indx] = value;
    }

    return 0;
}

int
getSharedResBitPos (char *resName)
{
    struct sharedResourceInstance *tmpSharedRes = NULL;
    int bitPos = 0;

    if (resName == NULL) {
        return -1;
    }

    for (tmpSharedRes = sharedResourceHead, bitPos = 0; tmpSharedRes; tmpSharedRes = tmpSharedRes->nextPtr, bitPos++)
    {
        if (!strcmp (resName, tmpSharedRes->resName))
        {
            return bitPos;
        }
    }

    return -1;
}

void
getExtResourcesLoad (void)
{
    int bitPos     = 0;
    char *resName  = NULL;
    char *resValue = NULL;
    float fValue   = 0.0;

    for (uint i = 0; i < allInfo.nRes; i++)
    {
        if (allInfo.resTable[i].flags & RESF_DYNAMIC && allInfo.resTable[i].flags & RESF_EXTERNAL)
        {
            resName = allInfo.resTable[i].name;

            if (!defaultRunElim)
            {
                if ((bitPos = getSharedResBitPos (resName)) == -1) {
                    continue;
                }
            }
            if ((resValue = getExtResourcesVal (resName)) == NULL) {
                continue;
            }

            if (saveSBValue (resName, resValue) == 0) {
                continue;
            }
            fValue = atof (resValue);

            saveIndx (resName, fValue);
        }
    }
    return;
}

int
isResourceSharedByHost (struct hostNode *host, char *resName)
{
    for( uint i = 0; i < host->numInstances; i++)
    {
        if (strcmp (host->instances[i]->resName, resName) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void
getusr (void)
{
    static char first = TRUE; // FIXME FIXME FIXME there has to be a better way to mark a re-entrant function
    struct sharedResourceInstance *tmpSharedRes = NULL;
    struct timeval timeout = { 0, 0 };
    struct timeval expire  = { 0, 0 };
    struct timeval time0   = { 0, 0 };
    struct timeval t       = { 0, 0 };
    time_t lastStart = 0;
    FILE *fp = NULL;
    char *masterResStr = "Y";
    char *hostNamePtr  = NULL;
    char *resStr       = NULL;
    int nfds = 0;
    int size = 0;
    int scc  = 0;
    int bw   = 0;
    int i    = 0;

    if (first)
    {
        for (i = 0; i < NBUILTINDEX; i++) {
            overRide[i] = INFINIT_LOAD;
        }
        first = FALSE;
    }

    if (!callElim ())
    {
      return;
    }

  getExtResourcesLoad ();

  if (!startElim ())
    {
      return;
    }

  if ((elim_pid < 0) && (time (0) - lastStart > 90))
    {

      if (ELIMrestarts < 0 || ELIMrestarts > 0)
	{

	  if (ELIMrestarts > 0)
	    {
	      ELIMrestarts--;
	    }

	  if (!myClusterPtr->eLimArgv)
	    {
	      char *path =
		malloc (strlen (limParams[LSF_SERVERDIR].paramValue) +
			strlen (ELIMNAME) + 8);
	      if (!path)
		{
		  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
		  setUnkwnValues ();
		  return;
		}
	      strcpy (path, limParams[LSF_SERVERDIR].paramValue);
	      strcat (path, "/");
	      strcat (path, ELIMNAME);

	      if (logclass & LC_EXEC)
		{
		  ls_syslog (LOG_DEBUG, "%s : the elim's name is <%s>\n",
			     __func__, path);
		}


	      myClusterPtr->eLimArgv =
		parseCommandArgs (path, myClusterPtr->eLimArgs);
	      if (!myClusterPtr->eLimArgv)
		{
		  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
		  setUnkwnValues ();
		  return;
		}
	    }

	  if (fp)
	    {
	      fclose (fp);
	      fp = NULL;
	    }

	  lastStart = time (0);

	  if (masterMe)
	    putEnv ("LSF_MASTER", masterResStr);
	  else
	    putEnv ("LSF_MASTER", "N");

	  if (resStr != NULL)
	    free (resStr);

	  hostNamePtr = ls_getmyhostname ();

	  size = 0;
	  for (tmpSharedRes = sharedResourceHead;
	       tmpSharedRes; tmpSharedRes = tmpSharedRes->nextPtr)
	    {
	      size += strlen (tmpSharedRes->resName) + sizeof (char);
	    }
	  for (i = NBUILTINDEX; i < allInfo.nRes; i++)
	    {
            if (allInfo.resTable[i].flags & RESF_EXTERNAL) {
                continue;
            }
	      if ((allInfo.resTable[i].flags & RESF_DYNAMIC)
		  && !(allInfo.resTable[i].flags & RESF_BUILTIN))
		{

		  size += strlen (allInfo.resTable[i].name) + sizeof (char);
		}
	    }
	  resStr = calloc (size + 1, sizeof (char));
	  if (!resStr)
	    {
	      ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
	      setUnkwnValues ();
	      return;
	    }
	  resStr[0] = '\0';

	  for (i = NBUILTINDEX; i < allInfo.nRes; i++)
	    {
	      if (allInfo.resTable[i].flags & RESF_EXTERNAL)
		continue;
	      if ((allInfo.resTable[i].flags & RESF_DYNAMIC)
		  && !(allInfo.resTable[i].flags & RESF_BUILTIN))
		{

		  if ((allInfo.resTable[i].flags & RESF_SHARED)
		      && (!masterMe)
		      &&
		      (isResourceSharedInAllHosts (allInfo.resTable[i].name)))
		    {
		      continue;
		    }

		  if ((allInfo.resTable[i].flags & RESF_SHARED)
		      &&
		      (!isResourceSharedByHost
		       (myHostPtr, allInfo.resTable[i].name)))
		    continue;

		  if (resStr[0] == '\0')
		    sprintf (resStr, "%s", allInfo.resTable[i].name);
		  else
		    {
		      sprintf (resStr, "%s %s", resStr,
			       allInfo.resTable[i].name);
		    }
		}
	    }
	  putEnv ("LSF_RESOURCES", resStr);

	  if ((fp = lim_popen (myClusterPtr->eLimArgv, "r")) == NULL)
	    {
	      ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL, __func__, "lim_popen",
			 myClusterPtr->eLimArgv[0]);
	      setUnkwnValues ();

	      return;
	    }
	  ls_syslog (LOG_INFO, (_i18n_msg_get (ls_catd, NL_SETN, 5930, "%s: Started ELIM %s pid %d")), __func__,	/* catgets 5930 */
		     myClusterPtr->eLimArgv[0], (int) elim_pid);
	  mustSendLoad = TRUE;

	}

    }

  if (elim_pid < 0)
    {
      setUnkwnValues ();
      if (fp)
	{
	  fclose (fp);
	  fp = NULL;
	}

      return;
    }

  timeout.tv_sec = 0;
  timeout.tv_usec = 5;

  if ((nfds = rd_select_ (fileno (fp), &timeout)) < 0)
    {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "rd_select_");
      lim_pclose (fp);
      fp = NULL;

      return;
    }

  if (nfds == 1)
    {
      int numIndx, cc;
      char name[MAXLSFNAMELEN], valueString[MAXEXTRESLEN];
      float value;
      sigset_t oldMask;
      sigset_t newMask;

      char *fromELIM = NULL;
      int sizeOfFromELIM = MAXLINELEN;
      char *elimPos;
      int spaceLeft, spaceRequired;

    if (!fromELIM)
    {
        fromELIM = malloc( sizeOfFromELIM );
        if (!fromELIM)
	    {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
            /* catgets 5935 */
            ls_syslog (LOG_ERR, "5935: %s:Received from ELIM: <out of memory to record contents>", __func__ );
            setUnkwnValues ();
            lim_pclose (fp);
            fp = NULL;
            return;
        }
    }

    elimPos = fromELIM;
    // elimPos = '\0';  // FIXME FIXME FIXME FIXME the fuck?


    blockSigs_ (0, &newMask, &oldMask);

    if (logclass & LC_TRACE)
    {
        ls_syslog (LOG_DEBUG, "%s: Signal mask has been changed, all are signals blocked now", __func__);
    }

    if (ELIMblocktime >= 0)
    {
        io_nonblock_ (fileno (fp));
	}

    cc = fscanf (fp, "%d", &numIndx);
    if (cc != 1)
    {
        /* catgets 5924 */
        ls_syslog (LOG_ERR, "5924: %s: Protocol error numIndx not read (cc=%d): %m" , __func__, cc);
        lim_pclose (fp);
        fp = NULL;
        unblockSigs_ (&oldMask);
        return;
    }

    bw = sprintf (elimPos, "%d ", numIndx);
    elimPos += bw;

    if (numIndx < 0)
	{
        /* catgets 5925 */
        ls_syslog (LOG_ERR, "5925: %s: Protocol error numIndx=%d", __func__, numIndx);
        setUnkwnValues ();
        lim_pclose (fp);
        free(fp);
        unblockSigs_ (&oldMask);
        return;
	}


    if (ELIMblocktime >= 0)
	{
        gettimeofday (&t, NULL);
        expire.tv_sec = t.tv_sec + ELIMblocktime;
        expire.tv_usec = t.tv_usec;
	}

    i = numIndx * 2;
    while (i)
    {
        if (i % 2)
        {
            cc = fscanf (fp, "%40s", valueString);
            valueString[MAXEXTRESLEN - 1] = '\0'; // FIXME FIXME FIXME FIXME throw at debugger; what on earth was I thinking?
        }
	  else
	    {
	      cc = fscanf (fp, "%40s", name);
	      name[MAXLSFNAMELEN - 1] = '\0';
	    }

	  if (cc == -1)
	    {
	      int scanerrno = errno;
	      if (scanerrno == EAGAIN)
		{


		  gettimeofday (&t, NULL);
		  timersub (&expire, &t, &timeout);
		  if (timercmp (&timeout, &time0, <))
		    {
		      timerclear (&timeout);
		    }
		  scc = rd_select_ (fileno (fp), &timeout);
		}

	      if (scanerrno != EAGAIN || scc <= 0)
		{

            /* catgets 5926 */
            ls_syslog (LOG_ERR, "5926: %s: Protocol error, expected %d more tokens (cc=%d): %m", __func__, i, cc);


            /* catgets 5904 */
            ls_syslog (LOG_ERR, "5904: Received from ELIM: %s", fromELIM);

            setUnkwnValues ();
            lim_pclose (fp);
            fp = NULL; // this is probably not the right way..
            unblockSigs_ (&oldMask);
            return;
		}


	      continue;
	    }



	  spaceLeft = sizeOfFromELIM - (elimPos - fromELIM) - 1;
	  spaceRequired = strlen ((i % 2) ? valueString : name) + 1;

	  if (spaceLeft < spaceRequired)
	    {

	      char *oldFromElim = fromELIM;
	      int oldSizeOfFromELIM = sizeOfFromELIM;


	      sizeOfFromELIM += (spaceRequired - spaceLeft);

	      fromELIM = realloc (fromELIM, sizeOfFromELIM);
	      if (!fromELIM)
		{
		  ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "malloc");
	    /* catgets 5935 */
      ls_syslog (LOG_ERR, "5935: %s:Received from ELIM: <out of memory to record contents>", __func__);


		  sizeOfFromELIM = oldSizeOfFromELIM;
		  fromELIM = oldFromElim;

		  setUnkwnValues ();
		  lim_pclose (fp);
		  fp = NULL;
		  unblockSigs_ (&oldMask);
		  return;
		}
	      elimPos = fromELIM + strlen (fromELIM);
	    }

	  bw = sprintf (elimPos, "%s ", (i % 2) ? valueString : name);
	  elimPos += bw;

	  if (i % 2)
	    {

	      if (saveSBValue (name, valueString) == 0)
		{
		  i--;
		  continue;
		}


	      value = atof (valueString);
	      saveIndx (name, value);
	    }
	  i--;
	}

      unblockSigs_ (&oldMask);



      if (ELIMdebug)
	{

	 /* catgets 5903 */
    ls_syslog (LOG_WARNING, "5903: ELIM: %s", fromELIM);
	}
    }
}

void
unblockSigs_ (sigset_t * mask)
{
  
  sigprocmask (SIG_SETMASK, mask, NULL);

  if (logclass & LC_TRACE)
    {
      ls_syslog (LOG_DEBUG, "%s: The original signal mask has been restored", __func__);
    }

}

void
setUnkwnValues (void)
{
  int i;

  for (i = 0; i < allInfo.numUsrIndx; i++)
    myHostPtr->loadIndex[NBUILTINDEX + i] = INFINIT_LOAD;
  for (i = 0; i < NBUILTINDEX; i++)
    overRide[i] = INFINIT_LOAD;

  for (i = 0; i < myHostPtr->numInstances; i++)
    {
      if (myHostPtr->instances[i]->updateTime == 0
	  || myHostPtr->instances[i]->updHost == NULL)

	continue;
      if (myHostPtr->instances[i]->updHost == myHostPtr)
	{
	  strcpy (myHostPtr->instances[i]->value, "-");
	  myHostPtr->instances[i]->updHost = NULL;
	  myHostPtr->instances[i]->updateTime = 0;
	}
    }
}

int
saveSBValue (char *name, char *value)
{
    int i, indx, j, myHostNo = -1, updHostNo = -1;
  char *temp = NULL;
  time_t currentTime = 0;

  if ((indx = getResEntry (name)) < 0)
    return (-1);

  if (!(allInfo.resTable[indx].flags & RESF_DYNAMIC))
    return -1;

  if (allInfo.resTable[indx].valueType == LS_NUMERIC)
    {
      if (!isanumber_ (value))
	{
	  return -1;
	}
    }

  if (myHostPtr->numInstances <= 0)
    return (-1);

  for (i = 0; i < myHostPtr->numInstances; i++)
    {
      if (strcmp (myHostPtr->instances[i]->resName, name))
	continue;
      if (currentTime == 0)
	currentTime = time (0);
      if (masterMe)
	{

	  for (j = 0; j < myHostPtr->instances[i]->nHosts; j++)
	    {
	      if (myHostPtr->instances[i]->updHost
		  && (myHostPtr->instances[i]->updHost
		      == myHostPtr->instances[i]->hosts[j]))
		updHostNo = j;
	      if (myHostPtr->instances[i]->hosts[j] == myHostPtr)
		myHostNo = j;
	      if (myHostNo >= 0
		  && (updHostNo >= 0
		      || myHostPtr->instances[i]->updHost == NULL))
		break;
	    }
	  if (updHostNo >= 0
	      && (myHostNo < 0
		  || ((updHostNo < myHostNo)
		      && strcmp (myHostPtr->instances[i]->value, "-"))))
	    return (0);
	}

      if ((temp = (char *) malloc (strlen (value) + 1)) == NULL)
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
	  FREEUP (temp);
	  return (0);
	}
      strcpy (temp, value);
      FREEUP (myHostPtr->instances[i]->value);
      myHostPtr->instances[i]->value = temp;
      myHostPtr->instances[i]->updateTime = currentTime;
      myHostPtr->instances[i]->updHost = myHostPtr;
      if (logclass & LC_LOADINDX)
	ls_syslog (LOG_DEBUG3, "saveSBValue: i = %d, resName=%s, value=%s, newValue=%s, updHost=%s",
		   i, myHostPtr->instances[i]->resName,
		   myHostPtr->instances[i]->value, temp,
		   myHostPtr->instances[i]->updHost->hostName);
      return (0);
    }
  return (-1);

}

void
initConfInfo (void)
{
    char *sp;

  if ((sp = getenv ("LSF_NCPUS")) != NULL)
    myHostPtr->statInfo.maxCpus = atoi (sp);
  else
    myHostPtr->statInfo.maxCpus = numCpus ();
  if (myHostPtr->statInfo.maxCpus <= 0)
    {
      /* catgets 5928 */
      ls_syslog (LOG_ERR, "5928: %s: Invalid num of CPUs %d. Default to 1", __func__, myHostPtr->statInfo.maxCpus);
      myHostPtr->statInfo.maxCpus = 1;
    }

  ncpus = myHostPtr->statInfo.maxCpus;

  myHostPtr->statInfo.portno = lim_tcp_port;
  myHostPtr->statInfo.hostNo = myHostPtr->hostNo;
  myHostPtr->infoValid = TRUE;

}

char *
getElimRes (void)
{

  int i;
  int numEnv;
  int resNo;
  char *resNameString;

  resNameString = malloc ((allInfo.nRes) * MAXLSFNAMELEN);
  if (resNameString == NULL)
    {
      ls_syslog (LOG_ERR, "%s: failed allocate %d bytes %m", __func__, allInfo.nRes * MAXLSFNAMELEN);
      lim_Exit ("getElimRes");
    }

  numEnv = 0;
  resNameString[0] = '\0';
  for (i = 0; i < allInfo.numIndx; i++)
    {
      if (allInfo.resTable[i].flags & RESF_EXTERNAL)
	continue;
      if (numEnv != 0)
	strcat (resNameString, " ");
      strcat (resNameString, allInfo.resTable[i].name);
      numEnv++;
    }

  for (i = 0; i < myHostPtr->numInstances; i++)
    {
      resNo = resNameDefined (myHostPtr->instances[i]->resName);
      if (allInfo.resTable[resNo].flags & RESF_EXTERNAL)
	continue;
      if (allInfo.resTable[resNo].interval > 0)
	{
	  if (numEnv != 0)
	    strcat (resNameString, " ");
	  strcat (resNameString, myHostPtr->instances[i]->resName);
	  numEnv++;
	}
    }

  if (numEnv == 0)
    return NULL;

  return resNameString;
}

int
callElim (void)
{
  int runit = FALSE;
  int lastTimeMasterMe = FALSE;

  if (masterMe && !lastTimeMasterMe)
    {
      lastTimeMasterMe = TRUE;
      if (runit)
	{
	  termElim ();
	  if (myHostPtr->callElim || defaultRunElim)
	    {
	      return TRUE;
	    }
	  else
	    {
	      runit = FALSE;
	      return FALSE;
	    }
	}
    }

  if (!masterMe && lastTimeMasterMe)
    {
      lastTimeMasterMe = FALSE;
      if (runit)
	{
	  termElim ();
	  if (myHostPtr->callElim || defaultRunElim)
	    {
	      return TRUE;
	    }
	  else
	    {
	      runit = FALSE;
	      return FALSE;
	    }
	}
    }

  if (masterMe)
    lastTimeMasterMe = TRUE;
  else
    lastTimeMasterMe = FALSE;

  if (runit)
    {
      if (!myHostPtr->callElim && !defaultRunElim)
	{
	  termElim ();
	  runit = FALSE;
	  return FALSE;
	}
    }


  if (defaultRunElim)
    {
      runit = TRUE;
      return TRUE;
    }

  if (myHostPtr->callElim)
    {
      runit = TRUE;
      return TRUE;
    }
  else
    {
      runit = FALSE;
      return FALSE;
    }


}

int
startElim (void)
{
  int notFirst = FALSE, startElim = FALSE;
  int i;

  if (!notFirst)
    {

      for (i = 0; i < allInfo.nRes; i++)
	{
	  if (allInfo.resTable[i].flags & RESF_EXTERNAL)
	    continue;
	  if ((allInfo.resTable[i].flags & RESF_DYNAMIC)
	      && !(allInfo.resTable[i].flags & RESF_BUILTIN))
	    {
	      startElim = TRUE;
	      break;
	    }
	}
      notFirst = TRUE;
    }

  return startElim;
}


void
termElim (void)
{
  if (elim_pid == -1)
    return;

  kill (elim_pid, SIGTERM);
  elim_pid = -1;

}

int
isResourceSharedInAllHosts (char *resName)
{
  struct sharedResourceInstance *tmpSharedRes = NULL;

    for (tmpSharedRes = sharedResourceHead; tmpSharedRes; tmpSharedRes = tmpSharedRes->nextPtr)
    {
        if (strcmp (tmpSharedRes->resName, resName))
        {
            continue;
        }
        if (tmpSharedRes->nHosts == myClusterPtr->numHosts)
        {
            return (1);
        }
    }

  return (0);
}

int
queueLengthEx (float *r15s, float *r1m, float *r15m)
{
    char *ldavgbuf = malloc( sizeof( char ) * 40 + 1);
    double loadave[3] = { 0.0, 0.0, 0.0 };
    int count = 0;
    int fd    = 0;
//#define LINUX_LDAV_FILE "/proc/loadavg"
// FIXME FIXME FIXME FIXME FIXME major inflaction point to 
    fd = open (LINUX_LDAV_FILE, O_RDONLY);
    if (fd < 0)
    {
        ls_syslog (LOG_ERR, "%s: %m", __FUNCTION__);
        return -1;
    }

    count = read (fd, ldavgbuf, sizeof (ldavgbuf));
    if (count < 0)
    {
        ls_syslog (LOG_ERR, "%s:%m", __FUNCTION__);
        close (fd);
        return -1;
    }

    close (fd);
    count = sscanf (ldavgbuf, "%lf %lf %lf", &loadave[0], &loadave[1], &loadave[2]);
    if (count != 3)
    {
        ls_syslog (LOG_ERR, "%s: %m", __FUNCTION__);
        return -1;
    }

    *r15s = queueLength ();
    *r1m  = loadave[0];
    *r15m = loadave[2];

    return 0;
}

float
queueLength (void)
{
    DIR *dir_proc_fd       = NULL;
    struct dirent *process = NULL;
    char *filename = malloc( sizeof( char ) * MAXLSFNAMELEN + 1 );
    char *buffer   = malloc( sizeof( char ) * MAXLSFNAMELEN + 1 );
    char status    = ' ';
    size_t size    = 0;
    float ql       = 0.0;
    uint running   = 0;
    int fd         = 0;

    dir_proc_fd = opendir ("/proc"); // FIXME FIXME FIXME FIXME FIXME remove fixed string
    if (dir_proc_fd == (DIR *) 0)
    {
        ls_syslog (LOG_ERR, "%s: opendir() /proc failed: %m", __FUNCTION__);
        return (0.0);
    }

    while ((process = readdir (dir_proc_fd)))
    {
        if (isdigit (process->d_name[0]))
        {

            sprintf (filename, "/proc/%s/stat", process->d_name);
            fd = open (filename, O_RDONLY, 0);
            if (fd == -1)
            {
                ls_syslog (LOG_DEBUG, "%s: cannot open [%s], %m", __FUNCTION__, filename);
                continue;
            }
            if( read (fd, buffer, strlen (buffer) ) <= 0 )  // FIXME FIXME FIXME FIXME test case to crash; might crash here, or read garbage, then set lenght to read to - 1
      {
        ls_syslog (LOG_DEBUG, "%s: cannot read [%s], %m", __FUNCTION__, filename);
        close (fd);
        continue;
      }
    close (fd);
    sscanf (buffer, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %*u %*d %*u %lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n", &status, &size);
    if (status == 'R' && size > 0)
      running++;
  }
    }
  closedir (dir_proc_fd);

  if (running > 0)
    {
      ql = running - 1;
      if (ql < 0)
  ql = 0;
    }
  else
    {
      ql = 0;
    }

  prevRQ = ql;

  return ql;
}

void
cpuTime ( time_t *itime, time_t *etime)
{
  time_t ttime = 0.0;
  int stat_fd  = 0;
  time_t cpu_user_time = 0.0;
  time_t cpu_nice_time = 0.0;
  time_t cpu_sys_time  = 0.0;
  time_t cpu_idle      = 0.0;

  stat_fd = open ("/proc/stat", O_RDONLY, 0); // FIXME FIXME FIXME FIXME FIXME remove fixed string
  if (stat_fd == -1)
    {
      ls_syslog (LOG_ERR, "%s: open() /proc/stat failed: %m:", __FUNCTION__);
      return;
    }

  if (read (stat_fd, buffer, sizeof (buffer) - 1) <= 0)
    {
      ls_syslog (LOG_ERR, "0%s: read() /proc/stat failed: %m", __FUNCTION__);
      close (stat_fd);
      return;
    }
  close (stat_fd);

  sscanf (buffer, "cpu  %lf %lf %lf %lf", &cpu_user, &cpu_nice, &cpu_sys, &cpu_idle);


  *itime = (cpu_idle - prev_idle);
  prev_idle = cpu_idle;

  ttime = cpu_user + cpu_nice + cpu_sys + cpu_idle;
  *etime = ttime - prev_time;

  prev_time = ttime;

  if (*etime == 0)
    *etime = 1;

  return;
}


float
getpaging (float etime)
{
  float smoothpg = 0.0;
  char first = TRUE;
  double prev_pages;
  double page, page_in, page_out;

  if (getPage (&page_in, &page_out, TRUE) == -1)
    {
      return (0.0);
    }

  page = page_in + page_out;
  if (first)
    {
      first = FALSE;
    }
  else
    {
      if (page < prev_pages)
  smooth (&smoothpg, (prev_pages - page) / etime, EXP4);
      else
  smooth (&smoothpg, (page - prev_pages) / etime, EXP4);
    }

  prev_pages = page;

  return smoothpg;
}

float
getIoRate (float etime)
{
  float kbps;
  char first = TRUE;
  double prev_blocks = 0;
  float smoothio = 0;
  double page_in, page_out;

  if (getPage (&page_in, &page_out, FALSE) == -1)
    {
      return (0.0);
    }

  if (first)
    {
      kbps = 0;
      first = FALSE;

      if (myHostPtr->statInfo.nDisks == 0)
  myHostPtr->statInfo.nDisks = 1;
    }
  else
    kbps = page_in + page_out - prev_blocks;

  if (kbps > 100000.0)
    {
      ls_syslog (LOG_DEBUG, "%s:: IO rate=%f bread=%d bwrite=%d", __FUNCTION__, kbps, page_in, page_out);
    }

  prev_blocks = page_in + page_out;
  smooth (&smoothio, kbps, EXP4);

  return smoothio;
}

float
getswap (void)
{
  short tmpcnt;
  float swap;

  if (tmpcnt >= SWP_INTVL_CNT)
    tmpcnt = 0;

  tmpcnt++;
  if (tmpcnt != 1) {
    return swap;
  }

  if (readMeminfo () == -1) {
    return (0);
  }
  swap = free_swap / 1024.0; // FIXME FIXME FIXME FIXME use probler library to format; i didn't write this, did I ?

  return swap;
}

int
readMeminfo (void)
{
    FILE *f = NULL;
    char *lineBuffer = malloc( sizeof( char ) * MAXLSFNAMELEN + 1 );
    char *tag        = malloc( sizeof( char ) * MAXLSFNAMELEN + 1 );
    u_long value = 0;

    if( ( f = fopen ("/proc/meminfo", "r")) == NULL)
    {
        ls_syslog (LOG_ERR, "%s: open() failed /proc/meminfo: %m", __FUNCTION__);
        return -1;
    }

    while (fgets (lineBuffer, sizeof (lineBuffer), f))
    {

        if (sscanf (lineBuffer, "%s %lld kB", tag, &value) != 2) {
            continue;
        }

        if( strcmp( tag, "MemTotal:" ) == 0 ) {
            main_mem = value;
        }
        if( strcmp( tag, "MemFree:" ) == 0 ) {
            free_mem = value;
        }
        if( strcmp( tag, "MemShared:" ) == 0 ) {
            shared_mem = value;
        }
        if( strcmp( tag, "Buffers:" ) == 0 ) {
            buf_mem = value;
        }
        if( strcmp( tag, "Cached:" ) == 0 ) {
            cashed_mem = value;
        }
        if( strcmp( tag, "SwapTotal:" ) == 0 ) {
            swap_mem = value;
        }
        if( strcmp( tag, "SwapFree:" ) == 0 ) {
            free_swap = value;
        }
    }

    fclose (f);
    free(tag);
    free(lineBuffer);

    return 0;
}

int
getPage (double *page_in, double *page_out, bool_t isPaging)
{
    FILE *f = NULL;
    double value = 0.0;
    char *lineBuffer = malloc( sizeof( char ) * MAXLSFNAMELEN + 1 );
    char *tag        = malloc( sizeof( char ) * MAXLSFNAMELEN + 1 );

    if ((f = fopen ("/proc/vmstat", "r")) == NULL) // FIXME FIXME FIXME FIXME FIXME set macosx appropriate path
    {
        ls_syslog (LOG_ERR, "%s: fopen() failed /proc/vmstat: %m", __FUNCTION__);
        return -1;
    }

    while (fgets (lineBuffer, sizeof (lineBuffer), f))
    {

        if (sscanf (lineBuffer, "%s %lf", tag, &value) != 2) {
            continue;
        }

        if( isPaging )
        {
            if (strcmp (tag, "pswpin") == 0) {
                *page_in = value;
            }
            if (strcmp (tag, "pswpout") == 0) {
                *page_out = value;
            }
        }
        else
        {
            if (strcmp (tag, "pgpgin") == 0) {
                *page_in = value;
            }
            if (strcmp (tag, "pgpgout") == 0) {
                *page_out = value;
            }
        
        }
    }

    fclose (f);
    free( tag );
    free( lineBuffer );

    return 0;
}

float
tmpspace (void)
{
    struct statvfs fs = { };
    float tmps = 0.0;
    int tmpcnt = 0;

    if (tmpcnt >= TMP_INTVL_CNT) {
        tmpcnt = 0;
    }

    tmpcnt++;
    if (tmpcnt != 1) {
        return tmps;
    }

    if (statvfs ("/tmp", &fs) < 0)  // FIXME FIXME FIXME FIXME /tmp should be set in configure.ac
    {
        ls_syslog (LOG_ERR, "%s: statfs() /tmp failed: %m", __FUNCTION__);
        return (tmps);
    }

    if (fs.f_bavail > 0) {
        tmps = (float) fs.f_bavail / ((float) (1024 * 1024) / fs.f_bsize); // FIXME FIXME FIXME FIXME read up on float division in C; i'm a dumbass;
    }
    else {
        tmps = 0.0;
    }

    return tmps;

}

