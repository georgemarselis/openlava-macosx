/*
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

#include "mbd.h"
#include "lib/lsi18n.h"
#include "daemons/init.h"

int
minit (int mbdInitFlags)
{
	struct hData *hPtr = NULL;
	int list = 0;
	char *master = NULL;

	ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);


	Signal_ (SIGTERM, (SIGFUNCTYPE) terminate_handler);
	Signal_ (SIGINT, (SIGFUNCTYPE) terminate_handler);
	Signal_ (SIGCHLD, (SIGFUNCTYPE) child_handler);
	Signal_ (SIGALRM, SIG_IGN);
	Signal_ (SIGHUP, SIG_IGN);
	Signal_ (SIGPIPE, SIG_IGN);
	Signal_ (SIGCHLD, (SIGFUNCTYPE) child_handler);

	if (!(debug || lsb_CheckMode)) {
			Signal_ (SIGTTOU, SIG_IGN);
			Signal_ (SIGTTIN, SIG_IGN);
			Signal_ (SIGTSTP, SIG_IGN);
	}

	clientList = (struct clientNode *) listCreate ("client list");

  for (list = 0; list < ALLJLIST; list++)
	{
	  char name[128];
	  sprintf (name, "Job Data List <%d>", list);
	  jDataList[list] = (struct jData *) listCreate (name);
	  listAllowObservers ((LIST_T *) jDataList[list]);
	}

  initTab (&jobIdHT);
  initTab (&jgrpIdHT);

  uDataPtrTb = uDataTableCreate ();

  qDataList = (struct qData *) listCreate ("Queue List");

  /* who am I...
   */
  batchId = getuid ();

  TIMEIT (0, masterHost = ls_getmyhostname (), "minit_ls_getmyhostname");
  if (masterHost == NULL)
	{

	  ls_syslog (LOG_ERR, "%s: Ohmygosh failed to get my own name, %M", __func__);
	  if (!lsb_CheckMode)
  mbdDie (MASTER_RESIGN);
	  else
  {
	lsb_CheckError = FATAL_ERR;
	return 0;
  }
	}

  master = ls_getmastername ();
  if (master == NULL)
	{
	  ls_syslog (LOG_ERR, "%s: ls_getmastername(): Failed to get master name, %M", __func__);
	  if (!lsb_CheckMode)
  {
	mbdDie (MASTER_RESIGN);
  }
	  else
  {
	lsb_CheckError = FATAL_ERR;
	return 0;
  }
	}

  if (strcmp (master, masterHost) != 0)
	{
	  ls_syslog (LOG_ERR, "%s: Current host %s is not master %s", __func__, masterHost, master);
	  if (!lsb_CheckMode)
  {
	mbdDie (MASTER_RESIGN);
  }
	  else
  {
	lsb_CheckError = FATAL_ERR;
	return 0;
  }
	}

  if (lsb_CheckMode)
	ls_syslog (LOG_INFO, "%s: Calling LIM to get cluster name ...", __func__);

  getClusterData ();

  TIMEIT (0, allLsInfo = ls_info (), "minit_ls_info");
  if (allLsInfo == NULL)
	{
	  ls_syslog (LOG_ERR, "%s: Ohmygosh ls_info() failed just now... %M", __func__);
	  if (lsb_CheckMode)
  {
	lsb_CheckError = FATAL_ERR;
	return 0;
  }
	  mbdDie (MASTER_FATAL);
	}

  for ( unsigned long i = allLsInfo->nModels; i < MAX_MODELS; i++) {
	allLsInfo->cpuFactor[i] = 1.0;
  }

  TIMEIT (0, getLsfHostInfo (TRUE), "minit_getLsfHostInfo");
  if (LIMhosts == NULL)
	{
	  ls_syslog (LOG_ERR, "%s: Ohmygosh ls_gethostinfo() failed just now... %M", __func__);
	  if (lsb_CheckMode)
  {
	lsb_CheckError = FATAL_ERR;
	return 0;
  }
	  else
  mbdDie (MASTER_FATAL);
	}

  initParse (allLsInfo);
  tclLsInfo = getTclLsInfo ();
  initTcl (tclLsInfo);

  allUsersSet = setCreate (MAX_GROUPS, getIndexByuData, getuDataByIndex, "");
  setAllowObservers (allUsersSet);

  uGrpAllSet = setCreate (MAX_GROUPS, getIndexByuData, getuDataByIndex, "");

  uGrpAllAncestorSet = setCreate (MAX_GROUPS, getIndexByuData, getuDataByIndex, "");

  TIMEIT (0, readParamConf (mbdInitFlags), "minit_readParamConf");
  TIMEIT (0, readHostConf (mbdInitFlags), "minit_readHostConf");
  getLsbResourceInfo ();

  /* Call LIM and update the load information
   * about the batch hosts we just built.
   */
  getLsbHostLoad ();
  updHostList ();
  copyGroups (TRUE);

  if ((hPtr = getHostData (masterHost)) == NULL)
	{
	  ls_syslog (LOG_ERR, "%s: Master host %s is not defined in the lsb.hosts file", __func__, masterHost);
	  mbdDie (MASTER_FATAL);
	}
  if (defaultHostSpec != NULL && !validHostSpec (defaultHostSpec))
	{
	  ls_syslog (LOG_ERR, "%s: Invalid system defined DEFAULT_HOST_SPEC %s; ignored", __func__, defaultHostSpec);
	  FREEUP (defaultHostSpec);
	  FREEUP (paramConf->param->defaultHostSpec);
	  lsb_CheckError = WARNING_ERR;
	}

  TIMEIT (0, readUserConf (mbdInitFlags), "minit_readUserConf");
  TIMEIT (0, readQueueConf (mbdInitFlags), "minit_readQueueConf");
  copyGroups (FALSE);

  updUserList (mbdInitFlags);
  updQueueList ();

  if (chanInit_ () < 0)
	{
	  ls_syslog (LOG_ERR, "%s: Ohmygosh chanInit() failed... %M", __func__);
	  mbdDie (MASTER_FATAL);
	}

  treeInit ();

  if (get_ports () < 0)
	{
	  if (!lsb_CheckMode)
  mbdDie (MASTER_FATAL);
	  else
  lsb_CheckError = FATAL_ERR;
	}

  if (getenv ("RECONFIG_CHECK") == NULL)
	{
	  batchSock = init_ServSock (mbd_port);
	  if (batchSock < 0)
  {
	ls_syslog (LOG_ERR, "%s: Cannot get batch server socket... %M", __func__);
	if (!lsb_CheckMode)
	  mbdDie (MASTER_FATAL);
	else
	  lsb_CheckError = FATAL_ERR;
  }
	}

  if (!(debug || lsb_CheckMode))
	{
	  nice (NICE_LEAST);
	  nice (NICE_MIDDLE);
	  nice (0);
	}

  if (!(debug || lsb_CheckMode))
	{

	  if (chdir (LSTMPDIR) < 0)
  {
	ls_syslog (LOG_ERR, "%s: Ohmygosh chdir(%s) failed %m", __func__, LSTMPDIR);
	if (!lsb_CheckMode)
	  mbdDie (MASTER_FATAL);
	else
	  lsb_CheckError = FATAL_ERR;
  }
	}

  if (!lsb_CheckMode)
	{
	  TIMEIT (0, init_log (), "init_log()");
	}

  getMaxCpufactor ();

  return 0;
}

int
readHostConf (int mbdInitFlags)
{
  char file[PATH_MAX];

  if (mbdInitFlags == FIRST_START || mbdInitFlags == RECONFIG_CONF)
	{

	  sprintf (file, "%s/lsb.hosts", daemonParams[LSB_CONFDIR].paramValue);

	  hostFileConf = getFileConf (file, HOST_FILE);
	  if (hostFileConf == NULL && lserrno == LSE_NO_FILE)
  {
	ls_syslog (LOG_ERR, "%s: lsb.hosts not found %M, all hosts known by LSF will be used", __func__);
	addDefaultHost ();
	return 0;
  }
	}
  else
	{
	  if (hostFileConf == NULL)
  {
	ls_syslog (LOG_ERR, "%s: lsb.hosts not found, all hosts known by LSF will be used", __func__);
	addDefaultHost ();
	return 0;
  }
	}

  fillClusterConf (&clusterConf);
  /* Invoke the library lsb.conf.c to read the
   * the lsb.hosts file.
   */
  if ((hostConf = lsb_readhost (hostFileConf,
		allLsInfo, CONF_CHECK, &clusterConf)) == NULL)
	{
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "lsb_readhost");
	  if (lsb_CheckMode)
  {
	lsb_CheckError = FATAL_ERR;
	return -1;
  }
	  else
  {
	mbdDie (MASTER_FATAL);
  }
	}
  addHostData (hostConf->numHosts, hostConf->hosts);
  createTmpGData (hostConf->hgroups,
	  hostConf->numHgroups, HOST_GRP, tempHGData, &nTempHGroups);
  if (lsberrno == LSBE_CONF_WARNING)
	lsb_CheckError = WARNING_ERR;

  /* make it...
   */
  mkLostAndFoundHost ();

  return 0;
}

struct hData *
initHData (struct hData *hData)
{
	if (hData == NULL) {
		hData = my_calloc (1, sizeof (struct hData), __func__ );
	}

	hData->host = NULL;
	hData->hostEnt.h_name = NULL;
	hData->hostEnt.h_aliases = NULL;
	hData->pollTime = 0;
	hData->acceptTime = 0;
	hData->numDispJobs = 0;
	hData->cpuFactor = 1.0;
	hData->numCPUs = 1;
	hData->hostType = NULL;
	hData->sbdFail = 0;
	hData->hStatus = HOST_STAT_OK;
	hData->uJobLimit = INFINIT_INT;
	hData->uAcct = NULL;
	hData->maxJobs = INFINIT_INT;
	hData->numJobs = 0;
	hData->numRUN = 0;
	hData->numSSUSP = 0;
	hData->numUSUSP = 0;
	hData->numRESERVE = 0;
	hData->mig = INFINIT_INT;
	hData->chkSig = SIG_CHKPNT;
	hData->hostModel = NULL;
	hData->maxMem = INFINIT_INT;
	hData->maxSwap = INFINIT_INT;
	hData->maxTmp = INFINIT_INT;
	hData->nDisks = INFINIT_INT;
	hData->limStatus = NULL;
	hData->resBitMaps = NULL;
	hData->lsfLoad = NULL;
	hData->lsbLoad = NULL;
	hData->busyStop = NULL;
	hData->busySched = NULL;
	hData->loadSched = NULL;
	hData->loadStop = NULL;
	hData->windows = NULL;
	hData->windEdge = 0;
	for ( unsigned short i = 0; i < 8; i++) {
		hData->week[i] = NULL;
	}
	for ( unsigned short i = 0; i < 3; i++) {
		hData->msgq[i] = NULL;
	}
	hData->flags = 0;
	hData->numInstances = 0;
	hData->instances = NULL;
	hData->pxySJL = NULL;
	hData->pxyRsvJL = NULL;
	hData->leftRusageMem = INFINIT_LOAD;

	return hData;
}

void
initThresholds (float loadSched[], float loadStop[])
{
	for ( unsigned int i = 0; i < allLsInfo->numIndx; i++) {
		if (allLsInfo->resTable[i].orderType == INCR) {
			loadSched[i] = INFINIT_LOAD;
			loadStop[i] = INFINIT_LOAD;
		}
		else {
			loadSched[i] = -INFINIT_LOAD;
			loadStop[i] = -INFINIT_LOAD;
		}
	}

	return;
}

void
readUserConf (int mbdInitFlags)
{
	char file[PATH_MAX];
	struct sharedConf sharedConf;
	struct hEnt *ent = NULL;

	memset (&sharedConf, 0, sizeof (struct sharedConf));

	if (mbdInitFlags == FIRST_START || mbdInitFlags == RECONFIG_CONF)
	{

		sprintf (file, "%s/lsb.users", daemonParams[LSB_CONFDIR].paramValue);
		userFileConf = getFileConf (file, USER_FILE);
		if (userFileConf == NULL && lserrno == LSE_NO_FILE)
		{

			ls_syslog (LOG_ERR, "%s: lsb.users not found %M, default user will be used", __func__);
			userConf = my_calloc (1, sizeof (struct userConf), __func__);
			// goto defaultUser;
			if (!defUser) {
				ent = h_getEnt_ (&uDataList, "default");
				if (ent == NULL) {
					addUserData ("default", INFINIT_INT, INFINIT_FLOAT, "readUserConf", FALSE, TRUE);
				}
				defUser = TRUE;
			}
			FREEUP (sharedConf.clusterName);
		}

	}
	else if (userFileConf == NULL)
	{

		ls_syslog (LOG_ERR, "%s: lsb.users not found %M, default user will be used", __func__);

		userConf = my_calloc (1, sizeof (struct userConf), __func__);
		// goto defaultUser;
		if (!defUser) {
			ent = h_getEnt_ (&uDataList, "default");
			if (ent == NULL) {
				addUserData ("default", INFINIT_INT, INFINIT_FLOAT, "readUserConf", FALSE, TRUE);
			}
			defUser = TRUE;
		}
		FREEUP (sharedConf.clusterName);
	}

	fillClusterConf (&clusterConf);
	fillSharedConf (&sharedConf);

	if ((userConf = lsb_readuser_ex (userFileConf,
		CONF_CHECK,
		&clusterConf, &sharedConf)) == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "lsb_readuser_ex");
		if (lsb_CheckMode)
		{
			lsb_CheckError = FATAL_ERR;
			FREEUP (sharedConf.clusterName);
			return;
		}
		else
		{
			mbdDie (MASTER_FATAL);
		}
	}

	createTmpGData (userConf->ugroups,
		userConf->numUgroups, USER_GRP, tempUGData, &nTempUGroups);

	if (lsberrno == LSBE_CONF_WARNING)
		lsb_CheckError = WARNING_ERR;

	addUData (userConf);

	// defaultUser:

	// if (!defUser)
	// {
	// 	ent = h_getEnt_ (&uDataList, "default");
	// 	if (ent == NULL)
	// 	{
	// 		addUserData ("default",
	// 			INFINIT_INT,
	// 			INFINIT_FLOAT, "readUserConf", FALSE, TRUE);
	// 	}
	// 	defUser = TRUE;
	// }

	// FREEUP (sharedConf.clusterName);

	return;
}

/* readQueueConf()
 * Copy the queue configuration as returned from
 * the library that processed the batch configuration.
 */
void
readQueueConf (int mbdInitFlags)
{
  char file[PATH_MAX];
  char *cp = NULL;
  char *word = NULL;
  struct qData *qp = NULL;
  int numDefQue = 0;
  int numQueues = 0;
  struct sharedConf sharedConf;

  if (mbdInitFlags == FIRST_START || mbdInitFlags == RECONFIG_CONF)
	{

	  sprintf (file, "%s/lsb.queues", daemonParams[LSB_CONFDIR].paramValue);
	  queueFileConf = getFileConf (file, QUEUE_FILE);
	  if (queueFileConf == NULL && lserrno == LSE_NO_FILE)
  {
	ls_syslog (LOG_ERR, "%s: lsb.queues not found %M, using default queue", __func__);
	createDefQueue ();
	return;
  }
	}
  else if (queueFileConf == NULL)
	{
	  ls_syslog (LOG_ERR, "%s: lsb.queues not be found, using default queue", __func__);
	  createDefQueue ();
	  return;
	}

  fillSharedConf (&sharedConf);
  if ((queueConf = lsb_readqueue (queueFileConf,
		  allLsInfo,
		  CONF_CHECK | CONF_RETURN_HOSTSPEC,
		  &sharedConf)) == NULL)
	{
	  if (lsberrno == LSBE_CONF_FATAL)
  {
	ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "lsb_readqueue");
	if (lsb_CheckMode)
	  {
		lsb_CheckError = FATAL_ERR;
		return;
	  }
	else
	  mbdDie (MASTER_FATAL);
  }
	  else
  {
	lsb_CheckError = WARNING_ERR;
	ls_syslog (LOG_ERR, "%s: lsb_readqueue() failed %M, using default queue", __func__);
	createDefQueue ();
	return;
  }
	}
  FREEUP (sharedConf.clusterName);
  if (lsberrno == LSBE_CONF_WARNING)
	lsb_CheckError = WARNING_ERR;

  addQData (queueConf, mbdInitFlags);

  numQueues = 0;
  for (qp = qDataList->forw; qp != qDataList; qp = qp->forw)
	if (qp->flags & QUEUE_UPDATE)
	  numQueues++;

  if (!numQueues)
	{
	  ls_syslog (LOG_WARNING, "%s: No valid queue defined", __func__);
	  lsb_CheckError = WARNING_ERR;
	}

  numDefQue = 0;
  if (numQueues && defaultQueues)
	{

	  cp = defaultQueues;

	  while ((word = getNextWord_ (&cp)))
  {
	if ((qp = getQueueData (word)) == NULL
		|| !(qp->flags & QUEUE_UPDATE))
	  {
		ls_syslog (LOG_WARNING, "%s: Invalid queue name specified by parameter DEFAULT_QUEUE; ignoring <%s>", __func__, word);
		lsb_CheckError = WARNING_ERR;
	  }
	else
	  {
		qp->qAttrib |= QUEUE_ATTRIB_DEFAULT;
		numDefQue++;
	  }
  }
	}

  if (numDefQue)
	return;

  ls_syslog (LOG_WARNING, "%s: lsb.queues: No valid default queue defined", __func__);

  lsb_CheckError = WARNING_ERR;

  if ((qp = getQueueData ("default")) != NULL)
	{
	  ls_syslog (LOG_WARNING, "%s: Using the default queue <default> provided by lsb.queues ", __func__);

	  qp->qAttrib |= QUEUE_ATTRIB_DEFAULT;
	  FREEUP (defaultQueues);
	  defaultQueues = safeSave ("default");
	  qp->flags |= QUEUE_UPDATE;
	  return;
	}

  createDefQueue ();

}

void
createDefQueue (void)
{
  struct qData *qp = NULL;

  ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 6136, "%s: Using the default queue <default> provided by the batch system"), __func__);  /* catgets 6136 */
  FREEUP (defaultQueues);
  defaultQueues = safeSave ("default");
  qp = initQData ();
  qp->qAttrib |= QUEUE_ATTRIB_DEFAULT;
  qp->description =
	safeSave
	("This is the default queue provided by the batch system. Jobs are scheduled with very loose control.");
  qp->queue = safeSave ("default");

  qp->hostSpec = safeSave (masterHost);
  qp->flags |= QUEUE_UPDATE;
  qp->numProcessors = numofprocs;
  qp->askedOthPrio = 0;
  qp->acceptIntvl = accept_intvl;
  inQueueList (qp);
}

/* addHost()
 * Configure the host to be part of MBD. Merge the
 * lsf base hostInfo information with the batch
 * configuration information, create a new host entry
 * in the host hash table.
 */
void
addHost (struct hostInfo *lsf, struct hData *thPtr, char *filename)
{
	static char first = TRUE;
	struct hEnt *ent = NULL;
	struct hData *hPtr = NULL;
	int new = 0;
	int i;
	char *word = NULL;

	if (first) {
		h_initTab_ (&hostTab, 101);
		first = FALSE;
	}

	ent = h_addEnt_ (&hostTab, thPtr->host, &new);
	if (!new) {
		assert (new);
	/* this cannot happen since lsb.conf.c
	* should have taken care of all checking.
	*/
		ls_syslog (LOG_WARNING, "%s: Host %s appears to be multiply defined nth instance ignored", __func__, thPtr->host);
		return;
	}
  /* This is a new host, hop la'
   * in the hostTab hash table.
   */
	hPtr = initHData (NULL);
	hPtr->flags |= HOST_NEEDPOLL;
	ent->hData = hPtr;

  /* In the case of lost and found host
   * we don't have lsf host data
   */
	if (lsf == NULL) {
		hPtr->host = safeSave (thPtr->host);
	}

  /* Let's inherit the lsf host base data.
   */
  if (lsf)
	{
	  hPtr->host = safeSave (lsf->hostName);
	  hPtr->cpuFactor = lsf->cpuFactor;
	  hPtr->hostType = safeSave (lsf->hostType);

	  /* Save the number of CPUs later on we will
	   * overwrite it if MXJ is set for this host.
	   */
	  if (lsf->maxCpus > 0)
  {
	hPtr->numCPUs = lsf->maxCpus;
  }
	  else
  {
	ls_syslog (LOG_DEBUG, "%s: numCPUs <%d> of host <%s> is not greater than 0; assuming as 1", __func__, lsf->maxCpus, lsf->hostName);
	hPtr->numCPUs = 1;
  }

	  hPtr->hostModel = safeSave (lsf->hostModel);
	  hPtr->maxMem = lsf->maxMem;

	  if (lsf->maxMem != 0)
  hPtr->leftRusageMem = (float) lsf->maxMem;

	  hPtr->maxSwap = lsf->maxSwap;
	  hPtr->maxTmp = lsf->maxTmp;
	  hPtr->nDisks = lsf->nDisks;
	  hPtr->resBitMaps = getResMaps (lsf->nRes, lsf->resources);

	  /* Fill up the hostent structure that is not used
	   * anywhere anyway...
	   */
	  hPtr->hostEnt.h_name = putstr_ (hPtr->host);
	  hPtr->hostEnt.h_aliases = NULL;
	}

  hPtr->uJobLimit = thPtr->uJobLimit;
  hPtr->maxJobs = thPtr->maxJobs;
  if (thPtr->maxJobs == -1)
	{
	  /* The MXJ was set as ! in lsb.hosts
	   */
	  hPtr->maxJobs = hPtr->numCPUs;
	  hPtr->flags |= HOST_AUTOCONF_MXJ;
	}

  hPtr->mig = thPtr->mig;
  hPtr->chkSig = thPtr->chkSig;

  hPtr->loadSched = my_calloc (allLsInfo->numIndx, sizeof (float), __func__);
  hPtr->loadStop = my_calloc (allLsInfo->numIndx, sizeof (float), __func__);
  hPtr->lsfLoad = my_calloc (allLsInfo->numIndx, sizeof (float), __func__);
  hPtr->lsbLoad = my_calloc (allLsInfo->numIndx, sizeof (float), __func__);
  hPtr->busySched = my_calloc (GET_INTNUM (allLsInfo->numIndx), sizeof (int), __func__);
  hPtr->busyStop = my_calloc (GET_INTNUM (allLsInfo->numIndx), sizeof (int), __func__);

  initThresholds (hPtr->loadSched, hPtr->loadStop);

		for ( unsigned int i = 0; i < allLsInfo->numIndx; i++) {

			if (thPtr->loadSched != NULL && thPtr->loadSched[i] != INFINIT_FLOAT) {
				hPtr->loadSched[i] = thPtr->loadSched[i];
			}

			if (thPtr->loadStop != NULL && thPtr->loadStop[i] != INFINIT_FLOAT) {
				hPtr->loadStop[i] = thPtr->loadStop[i];
			}
		}

  hPtr->flags |= HOST_UPDATE;

  if (thPtr->windows)
	{
	  char *sp = thPtr->windows;

	  hPtr->windows = safeSave (thPtr->windows);
	  *(hPtr->windows) = '\0';
	  while ((word = getNextWord_ (&sp)) != NULL)
  {
	char *save = safeSave (word);
	if (addWindow (word, hPtr->week, thPtr->host) < 0)
	  {
		ls_syslog (LOG_ERR, "%s: Bad time expression <%s>; ignored.", __func__, word);
		lsb_CheckError = WARNING_ERR;
		freeWeek (hPtr->week);
		free (save);
		continue;
	  }
	hPtr->windEdge = now;
	if (*(hPtr->windows) != '\0')
	  strcat (hPtr->windows, " ");
	strcat (hPtr->windows, save);
	free (save);
  }
	}
  else
	{
	  hPtr->windEdge = 0;
	  hPtr->hStatus = HOST_STAT_OK;
	}

  hPtr->limStatus = my_calloc((1 + GET_INTNUM (allLsInfo->numIndx)), sizeof (int), __func__);

	if (new) {
	/* openlava
	* If user did configure MXJ in their lsb.hosts
	* we do set the number of CPUs of this batch
	* host to be MXJ. This is for parallel job scheduling
	* as we believe that users know how many job slots
	* they have. However if MXJ is not set then we
	* use the physical number of CPUs we got from
	* the lsf base.
	*/
		if (hPtr->maxJobs > 0 && hPtr->maxJobs < INFINIT_INT)
			hPtr->numCPUs = hPtr->maxJobs;

		if (hPtr->numCPUs > 0)
			numofprocs += hPtr->numCPUs;
		else
			numofprocs++;
	}

	QUEUE_INIT (hPtr->msgq[MSG_STAT_QUEUED]);
	QUEUE_INIT (hPtr->msgq[MSG_STAT_SENT]);
	QUEUE_INIT (hPtr->msgq[MSG_STAT_RCVD]);

	return; // FIXME FIXME FIXME can a host fail to be added? check to see if return type needs to change
}       /* addHost() */

void
freeHData (struct hData *hPtr)
{
	struct hEnt *ent = NULL;
	int i;

	ent = h_getEnt_ (&hostTab, hPtr->host);
	assert (ent);

	/* Remove from the hostlist
	*/
	listRemoveEntry (hostList, (LIST_ENTRY_T *) hPtr);

	FREEUP (hPtr->host);
	FREEUP (hPtr->hostType);
	FREEUP (hPtr->hostModel);
	FREEUP (hPtr->hostEnt.h_name);

	if (hPtr->hostEnt.h_aliases) {
		for ( unsigned int i = 0; hPtr->hostEnt.h_aliases[i]; i++) {
			free (hPtr->hostEnt.h_aliases[i]);
		}
		free (hPtr->hostEnt.h_aliases);
	}
	FREEUP (hPtr->loadSched);
	FREEUP (hPtr->loadStop);
	FREEUP (hPtr->windows);
	freeWeek (hPtr->week);

	if (hPtr->uAcct) {
		if (hPtr->uAcct->slotPtr) {
			h_delTab_ (hPtr->uAcct);
		}
		else {
			FREEUP (hPtr->uAcct);
		}
	}

	FREEUP (hPtr->lsfLoad);
	FREEUP (hPtr->lsbLoad);
	FREEUP (hPtr->resBitMaps);

	QUEUE_DESTROY (hPtr->msgq[MSG_STAT_QUEUED]);
	QUEUE_DESTROY (hPtr->msgq[MSG_STAT_SENT]);
	QUEUE_DESTROY (hPtr->msgq[MSG_STAT_RCVD]);

	FREEUP (hPtr->instances);
	FREEUP (hPtr->limStatus);
	FREEUP (hPtr->busySched);
	FREEUP (hPtr->busyStop);

	h_rmEnt_ (&hostTab, ent);
	FREEUP (hPtr);

	return;
}

struct gData *
addGroup (struct gData **groups, char *gname, int *ngroups)
{

  groups[*ngroups] = my_calloc (1, sizeof (struct gData), __func__);
  groups[*ngroups]->group = safeSave (gname);
  h_initTab_ (&groups[*ngroups]->memberTab, 0);
  groups[*ngroups]->numGroups = 0;
  (*ngroups)++;

  return groups[*ngroups - 1];

}

void
addMember (struct gData *groupPtr, char *word, int grouptype, const char *filename, struct gData *groups[], int *ngroups)
{
  struct passwd *pw = NULL;
  char isgrp = FALSE;
  struct gData *subgrpPtr = NULL;
  char name[MAXHOSTNAMELEN];
  struct hostent *hp = NULL;

  if (grouptype == USER_GRP)
	{
	  subgrpPtr = getGrpData (tempUGData, word, nTempUGroups);
	  if (!subgrpPtr)
  TIMEIT (0, pw = getpwlsfuser_ (word), "addMemeber_getpwnam");

	  isgrp = TRUE;
	  if (pw != NULL)
  {
	strcpy (name, word);
	isgrp = FALSE;
  }
	  else if (!subgrpPtr)
  {
	char *grpSl = NULL;
	struct group *unixGrp = NULL;
	int lastChar = strlen (word) - 1;
	if (lastChar > 0 && word[lastChar] == '/')
	  {
		grpSl = safeSave (word);
		grpSl[lastChar] = '\0';
		TIMEIT (0, unixGrp = mygetgrnam (grpSl), "do_Users_getgrnam");
		FREEUP (grpSl);
	  }
	else
	  TIMEIT (0, unixGrp = mygetgrnam (word), "do_Users_getgrnam");

	if (unixGrp)
	  {


		subgrpPtr = addUnixGrp (unixGrp, word, filename,
			  groups, ngroups);
		if (!subgrpPtr)
	{
	  ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 6149, "%s: No valid users defined in Unix group <%s>; ignoring"), __func__, word); /* catgets 6149 */
	  lsb_CheckError = WARNING_ERR;
	  return;
	}

	  }
	else
	  {

		strcpy (name, word);
		isgrp = FALSE;
	  }
  }
	}

  if (grouptype == HOST_GRP)
	{
	  if ((subgrpPtr = getGrpData (tempHGData, word, nTempHGroups)) != NULL)
  {
	isgrp = TRUE;
  }
	  else
  {
	hp = Gethostbyname_ (word);
	if (hp == NULL)
	  {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6151, "%s: Bad host/group name <%s> in group <%s>; ignored"), __func__, word, groupPtr->group);  /* catgets 6151 */
		lsb_CheckError = WARNING_ERR;
		return;
	  }
	strcpy (name, hp->h_name);
	if (findHost (name) == NULL && numofhosts () != 0)
	  {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6152, "%s: Host <%s> is not used by the batch system; ignored"), __func__, name);  /* catgets 6152 */
		lsb_CheckError = WARNING_ERR;
		return;
	  }
	isgrp = FALSE;
  }
	}

  if (isInGrp (word, groupPtr))
	{
	  if (isgrp)
  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6153, "%s: Group <%s> is multiply defined in group <%s>; ignored"), __func__, word, groupPtr->group);  /* catgets 6153 */
	  else
  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6154, "%s: Member <%s> is multiply defined in group <%s>; ignored"), __func__, word, groupPtr->group); /* catgets 6154 */
	  lsb_CheckError = WARNING_ERR;
	  return;
	}

  if (isgrp)
	{
	  groupPtr->gPtr[groupPtr->numGroups] = subgrpPtr;
	  groupPtr->numGroups++;
	}
  else
	h_addEnt_ (&groupPtr->memberTab, name, NULL);

  return;

}

void
parseAUids (struct qData *qp, char *line)
{
	int numAds = 0;
	int callFail = FALSE;
	char *sp = NULL;
	char *word = NULL;
	char *tempStr = NULL;
	char *member = NULL;
	struct passwd *pw = NULL;
	struct group *unixGrp = NULL;
	struct gData *uGrp = NULL;
	struct admins admins;
	char forWhat[MAX_LINE_LEN];

	sprintf (forWhat, "for queue <%s> administrator", qp->queue);

	sp = line;

	while ((word = getNextWord_ (&sp)) != NULL)
		numAds++;
	if (numAds)
	{
		admins.adminIds = my_malloc ((numAds) * sizeof (int), __func__);
		admins.adminGIds = my_malloc ((numAds) * sizeof (int), __func__);
		admins.adminNames = my_malloc (numAds * sizeof (char *), __func__);
		admins.nAdmins = 0;
	}
	else
	{
		/* catgets 6155 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6155, "%s: No queue's administrators defined; ignored"), __func__);
		lsb_CheckError = WARNING_ERR;
		return;
	}
	sp = line;
	while ((word = getNextWord_ (&sp)) != NULL)
	{
		if (strcmp (word, "all") == 0)
		{
			setAllusers (qp, &admins);
			return;
		}
		else if ((pw = getpwlsfuser_ (word)))
		{
			if (putInLists (word, &admins, &numAds, forWhat) < 0)
			{
				callFail = TRUE;
				break;
			}
		}
		else if ((uGrp = getGrpData (tempUGData, word, nTempUGroups)) &&
			(tempStr = getGroupMembers (uGrp, TRUE)))
		{
			char *sp = tempStr;
			while ((member = getNextWord_ (&sp)) != NULL)
			{
				if (strcmp (word, "all") == 0)
				{
					setAllusers (qp, &admins);
					return;
				}
				if (putInLists (member, &admins, &numAds, forWhat) < 0)
				{
					callFail = TRUE;
					break;
				}
			}
			FREEUP (tempStr);
		}
		else if ((unixGrp = mygetgrnam (word)) != NULL)
		{
			i = 0;
			while (unixGrp->gr_mem[i] != NULL)
			{
				if (putInLists (unixGrp->gr_mem[i++], &admins, &numAds, forWhat)
					< 0)
				{
					callFail = TRUE;
					break;
				}
			}
		}
		else
		{

			if (putInLists (word, &admins, &numAds, forWhat) < 0)
			{
				callFail = TRUE;
				break;
			}
		}
		if (callFail == TRUE)
			break;
	}
	if (callFail == TRUE && !lsb_CheckMode)
		mbdDie (MASTER_RESIGN);

	if (!admins.nAdmins)
	{
		/* catgets 6157 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6157, "%s: No valid queue's administrators defined in <%s> for queue <%s>; ignored"), __func__, line, qp->queue);
		lsb_CheckError = WARNING_ERR;
		FREEUP (admins.adminIds);
		FREEUP (admins.adminGIds);
		FREEUP (admins.adminNames);
		return;
	}
	qp->nAdmins = admins.nAdmins;
	qp->adminIds = admins.adminIds;
	qp->admins = (char *) my_malloc (admins.nAdmins * MAX_LSB_NAME_LEN, __func__);
	qp->admins[0] = '\0';
	for (i = 0; i < admins.nAdmins; i++)
	{
		strcat (qp->admins, admins.adminNames[i]);
		strcat (qp->admins, " ");
		FREEUP (admins.adminNames[i]);
	}
	FREEUP (admins.adminNames);
	FREEUP (admins.adminGIds);
}

void
setAllusers (struct qData *qp, struct admins *admins)
{
	qp->nAdmins = 1;
	qp->adminIds = my_malloc (sizeof (int), __func__);
	qp->adminIds[0] = ALL_USERS_ADMINS;
	qp->admins = (char *) my_malloc (MAX_LSB_NAME_LEN, __func__);
	qp->admins[0] = '\0';
	strcat (qp->admins, "all users");

	FREEUP (admins->adminIds);
	FREEUP (admins->adminGIds);
	for( unsigned int i = 0; i < admins->nAdmins; i++) {
		FREEUP (admins->adminNames[i]);
	}
	FREEUP (admins->adminNames);

}

void
freeQData (struct qData *qp, int delete)
{
  FREEUP (qp->queue);
  FREEUP (qp->description);
  if (qp->uGPtr)
	{
	  freeGrp (qp->uGPtr);
	}

  FREEUP (qp->loadSched);
  FREEUP (qp->loadStop);
  FREEUP (qp->windows);
  FREEUP (qp->windowsD);
  freeWeek (qp->weekR);
  freeWeek (qp->week);
  FREEUP (qp->hostSpec);
  FREEUP (qp->defaultHostSpec);
  if (qp->nAdmins > 0)
	{
	  FREEUP (qp->adminIds);
	  FREEUP (qp->admins);
	}
  FREEUP (qp->preCmd);
  FREEUP (qp->postCmd);
  FREEUP (qp->prepostUsername);
  if (qp->requeueEValues)
	{
	  clean_requeue (qp);
	}
  FREEUP (qp->requeueEValues);
  FREEUP (qp->resReq);
  if (qp->resValPtr)
	{
	  FREEUP (qp->resValPtr->rusgBitMaps);
	  FREEUP (qp->resValPtr);
	}
  if (qp->uAcct)
	h_delTab_ (qp->uAcct);
  if (qp->hAcct)
	h_delTab_ (qp->hAcct);
  FREEUP (qp->resumeCond);
  lsbFreeResVal (&qp->resumeCondVal);
  FREEUP (qp->stopCond);
  FREEUP (qp->jobStarter);

  FREEUP (qp->suspendActCmd);
  FREEUP (qp->resumeActCmd);
  FREEUP (qp->terminateActCmd);

  FREEUP (qp->askedPtr);
  FREEUP (qp->hostList);

  if (delete == TRUE)
	{
	  offList ((struct listEntry *) qp);
	  numofqueues--;
	}
  if (qp->reasonTb)
	{
	  FREEUP (qp->reasonTb[0]);
	  FREEUP (qp->reasonTb[1]);
	  FREEUP (qp->reasonTb);
	}

  if (qp->hostInQueue)
	{
	  setDestroy (qp->hostInQueue);
	  qp->hostInQueue = NULL;
	}

  FREEUP (qp);
  return;
}

void
parseGroups (int groupType, struct gData **group, char *line, char *filename)
{
	char *word = NULL;
	char *groupName = NULL;
	char *grpSl = NULL;
	int lastChar = 0;
	struct group *unixGrp = NULL;
	struct gData *gp = NULL; 
	struct gData *mygp = NULL;
	struct passwd *pw = NULL;
	struct hostent *hp = NULL;

	mygp = my_malloc (sizeof (struct gData), __func__);
	*group = mygp;
	mygp->group = "";
	h_initTab_ (&mygp->memberTab, 16);
	mygp->numGroups = 0;
	for( unsigned int i = 0; i < MAX_GROUPS; i++)
		mygp->gPtr[i] = NULL;

	if (groupType == USER_GRP)
		groupName = "User/User";
	else
		groupName = "Host/Host";

  while ((word = getNextWord_ (&line)) != NULL)
	{
	  if (isInGrp (word, mygp))
  {
	ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6164, "%s/%s: Member <%s> is multiply defined; ignored"), filename, __func__, word); /* catgets 6164 */
	lsb_CheckError = WARNING_ERR;
	continue;
  }
	  if (groupType == USER_GRP)
  {
	TIMEIT (0, pw = getpwlsfuser_ (word), "parseGroups_getpwnam");
	if (pw != NULL)
	  {
		h_addEnt_ (&mygp->memberTab, word, NULL);
		continue;
	  }
	FREEUP (grpSl);
	lastChar = strlen (word) - 1;
	if (lastChar > 0 && word[lastChar] == '/')
	  {
		grpSl = safeSave (word);
		grpSl[lastChar] = '\0';
	  }


	gp = getGrpData (tempUGData, word, nTempUGroups);
	if (gp != NULL)
	  {
		mygp->gPtr[mygp->numGroups] = gp;
		mygp->numGroups++;
		continue;
	  }

	if (grpSl)
	  {
		TIMEIT (0, unixGrp =
		  mygetgrnam (grpSl), "parseGroups_getgrnam");

		grpSl[lastChar] = '/';
	  }
	else
	  TIMEIT (0, unixGrp = mygetgrnam (word), "parseGroups_getgrnam");
	if (unixGrp != NULL)
	  {

		gp =
	addUnixGrp (unixGrp, grpSl, filename, tempUGData,
		  &nTempUGroups);
		if (gp == NULL)
	{
	  ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 6165, "%s/%s: No valid users in Unix group <%s>; ignoring"), filename, __func__, word);  /* catgets 6165 */
	  lsb_CheckError = WARNING_ERR;
	  continue;
	}
		mygp->gPtr[mygp->numGroups] = gp;
		mygp->numGroups++;
	  }
	else
	  {

		h_addEnt_ (&mygp->memberTab, word, NULL);
		continue;
	  }
	continue;
  }
	  else
  {


	gp = getGrpData (tempHGData, word, nTempHGroups);
	if (gp != NULL)
	  {
		mygp->gPtr[mygp->numGroups] = gp;
		mygp->numGroups++;
		continue;
	  }

	hp = Gethostbyname_ (word);
	if (hp != NULL)
	  {
		word = hp->h_name;
		if (numofhosts ())
	{
	  if (!h_getEnt_ (&hostTab, word))
		{
		  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6170, "%s/%s: Host <%s> is not used by the batch system; ignored"), filename, __func__, word); /* catgets 6170 */
		  lsb_CheckError = WARNING_ERR;
		  continue;
		}
	}
		if (isInGrp (word, mygp))
	{
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6171, "%s/%s: Host name <%s> is multiply defined; ignored"), filename, __func__, word);  /* catgets 6171 */
	  lsb_CheckError = WARNING_ERR;
	  continue;
	}
		h_addEnt_ (&mygp->memberTab, word, NULL);
		continue;
	  }
	else
	  {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6172, "%s/%s: Unknown host or host group <%s>; ignored"), filename, __func__, word); /* catgets 6172 */
		lsb_CheckError = WARNING_ERR;
		continue;
	  }
  }
	}
  FREEUP (grpSl);

  return;
}

/* readParamConf()
 */
void
readParamConf (int mbdInitFlags)
{
  char file[PATH_MAX];

  setDefaultParams ();

  if (mbdInitFlags == FIRST_START || mbdInitFlags == RECONFIG_CONF)
	{

	  sprintf (file, "%s/lsb.params", daemonParams[LSB_CONFDIR].paramValue);

	  paramFileConf = getFileConf (file, PARAM_FILE);
	  if (paramFileConf == NULL && lserrno == LSE_NO_FILE)
  {
	ls_syslog (LOG_ERR, "%s: lsb.params can not be found %M, using default parameters", __func__);
	return;
  }
	}

  if ((paramConf = lsb_readparam (paramFileConf)) == NULL)
	{

	  if (lsberrno == LSBE_CONF_FATAL)
  {
	ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "lsb_readparam");
	if (lsb_CheckMode)
	  {
		lsb_CheckError = FATAL_ERR;
		return;
	  }

	mbdDie (MASTER_FATAL);
  }

	  if (lsberrno == LSBE_CONF_WARNING)
  lsb_CheckError = WARNING_ERR;
	  ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "lsb_readparam");

	  return;
	}

  if (lsberrno == LSBE_CONF_WARNING)
	lsb_CheckError = WARNING_ERR;

  setParams (paramConf);
}

int
my_atoi (char *arg, int upBound, int botBound)
{
  int num;

  if (!isint_ (arg))
	{
	  return INFINIT_INT;
	}
  num = atoi (arg);
  if (num >= upBound || num <= botBound)
	return INFINIT_INT;
  return num;

}

/* initQData()
 */
struct qData *
initQData (void)
{
  struct qData *qPtr;
  int i;

  qPtr = my_calloc (1, sizeof (struct qData), "initQData");
  qPtr->queue = NULL;
  qPtr->queueId = 0;
  qPtr->description = NULL;
  qPtr->numProcessors = 0;
  qPtr->priority = DEF_PRIO;
  qPtr->nice = DEF_NICE;
  qPtr->uJobLimit = INFINIT_INT;
  qPtr->uAcct = NULL;
  qPtr->pJobLimit = INFINIT_FLOAT;
  qPtr->hAcct = NULL;
  qPtr->windows = NULL;
  qPtr->windowsD = NULL;
  qPtr->windEdge = 0;
  qPtr->runWinCloseTime = 0;
  qPtr->numHUnAvail = 0;
  for (i = 0; i < 8; i++)
	{
	  qPtr->weekR[i] = NULL;
	  qPtr->week[i] = NULL;
	}
  for (i = 0; i < LSF_RLIM_NLIMITS; i++)
	{
	  qPtr->rLimits[i] = -1;
	  qPtr->defLimits[i] = -1;
	}
  qPtr->hostSpec = NULL;
  qPtr->defaultHostSpec = NULL;
  qPtr->qAttrib = 0;
  qPtr->qStatus = (QUEUE_STAT_OPEN | QUEUE_STAT_ACTIVE | QUEUE_STAT_RUN);
  qPtr->maxJobs = INFINIT_INT;
  qPtr->numJobs = 0;
  qPtr->numPEND = 0;
  qPtr->numRUN = 0;
  qPtr->numSSUSP = 0;
  qPtr->numUSUSP = 0;
  qPtr->mig = INFINIT_INT;
  qPtr->schedDelay = INFINIT_INT;
  qPtr->acceptIntvl = INFINIT_INT;

  qPtr->loadSched = my_calloc (allLsInfo->numIndx, sizeof (float), __func__);
  qPtr->loadStop = my_calloc (allLsInfo->numIndx, sizeof (float), __func__);

  initThresholds (qPtr->loadSched, qPtr->loadStop);
  qPtr->procLimit = -1;
  qPtr->minProcLimit = -1;
  qPtr->defProcLimit = -1;
  qPtr->adminIds = NULL;
  qPtr->admins = NULL;
  qPtr->nAdmins = 0;
  qPtr->preCmd = NULL;
  qPtr->postCmd = NULL;
  qPtr->prepostUsername = NULL;
  qPtr->requeueEValues = NULL;
  qPtr->requeEStruct = NULL;
  qPtr->hJobLimit = INFINIT_INT;
  qPtr->resValPtr = NULL;
  qPtr->resReq = NULL;
  qPtr->reasonTb = NULL;
  qPtr->numReasons = 0;
  qPtr->numUsable = 0;
  qPtr->numRESERVE = 0;
  qPtr->slotHoldTime = 0;
  qPtr->resumeCond = NULL;
  qPtr->resumeCondVal = NULL;
  qPtr->stopCond = NULL;
  qPtr->jobStarter = NULL;
  qPtr->suspendActCmd = NULL;
  qPtr->resumeActCmd = NULL;
  qPtr->terminateActCmd = NULL;
  for (i = 0; i < LSB_SIG_NUM; i++)
	qPtr->sigMap[i] = 0;

  qPtr->flags = 0;
  qPtr->uGPtr = NULL;
  qPtr->hostList = NULL;
  qPtr->hostInQueue = NULL;
  qPtr->askedPtr = NULL;
  qPtr->numAskedPtr = 0;
  qPtr->askedOthPrio = -1;
  FREEUP (qPtr->reasonTb);
  qPtr->reasonTb = my_calloc (2, sizeof (int *), __func__);
  qPtr->reasonTb[0] = my_calloc (numofhosts () + 1, sizeof (int), __func__);
  qPtr->reasonTb[1] = my_calloc (numofhosts () + 1, sizeof (int), __func__);
  qPtr->schedStage = 0;
  for (i = 0; i <= PJL; i++)
	{
	  qPtr->firstJob[i] = NULL;
	  qPtr->lastJob[i] = NULL;
	}
  qPtr->chkpntPeriod = -1;
  qPtr->chkpntDir = NULL;

  return qPtr;
}

int
searchAll (char *word)
{
	char *sp = NULL;
	char *cp = NULL;

	if (!word) {
		return FALSE;
	}
	cp = word;
	while ((sp = getNextWord_ (&cp))) {
		if (strcmp (sp, "all") == 0) {
			return TRUE;
		}
	}
	return FALSE;

}

void
freeKeyVal (struct keymap keylist[])
{
	for (unsigned int cc = 0; keylist[cc].key != NULL; cc++)
	{
		if (keylist[cc].val != NULL)
		{
			free (keylist[cc].val);
			keylist[cc].val = NULL;
		}
	}
}

int
isInGrp (char *word, struct gData *gp)
{
	if (h_getEnt_ (&gp->memberTab, word)) {
		return TRUE;
	}

	for ( unsigned int i = 0; i < gp->numGroups; i++) {
		if (strcmp (gp->gPtr[i]->group, word) == 0) {
			return TRUE;
		}
		if (isInGrp (word, gp->gPtr[i])) {
			return TRUE;
		}
	}

	return FALSE;
}

struct qData *
lostFoundQueue (void)
{
	static struct qData *qPtr;

	if (qPtr != NULL) {
		return qPtr;
	}

	qPtr = initQData ();
	qPtr->description = safeSave("This queue is created by the system to hold the jobs that were in the queues being removed from the configuration by the LSF Administrator.  Jobs in this queue will not be started unless they are switched to other queues.");
	qPtr->queue = safeSave (LOST_AND_FOUND);
	qPtr->uJobLimit = 0;
	qPtr->pJobLimit = 0.0;
	qPtr->acceptIntvl = DEF_ACCEPT_INTVL;
	qPtr->qStatus = (!QUEUE_STAT_OPEN | !QUEUE_STAT_ACTIVE);
	qPtr->uGPtr = my_malloc(sizeof (struct gData), __func__ );
	qPtr->uGPtr->group = "";
	h_initTab_ (&qPtr->uGPtr->memberTab, 16);
	qPtr->uGPtr->numGroups = 0;
	h_addEnt_ (&qPtr->uGPtr->memberTab, "nobody", 0);


	qPtr->hostSpec = safeSave (masterHost);
	inQueueList (qPtr);
	checkQWindow ();

	return qPtr;

}

/* mkLostAndFoundHost()
 * Always make this entry, it won't hurt and having
 * migrant hosts we will always need it.
 */
struct hData *
mkLostAndFoundHost (void)
{
	struct hData *lost = NULL;
	struct hData host = NULL;

	initHData (&host);
	host.host = LOST_AND_FOUND;
	host.cpuFactor = 1.0;
	host.uJobLimit = 0;
	host.maxJobs = 0;

	addHost (NULL, &host, "LostFoundHost");
	checkHWindow ();

	lost = getHostData (LOST_AND_FOUND);
	lost->flags = HOST_LOST_FOUND;

	return lost;
}

void
queueHostsPF (struct qData *qPtr, int *allPoll)
{
	struct hData *hData = NULL;

	qPtr->numProcessors = 0;

	if (qPtr->hostList == NULL || qPtr->askedOthPrio > 0)
	{

		qPtr->numProcessors = numofprocs;

		if ((qPtr->flags & QUEUE_NEEDPOLL) && *allPoll == FALSE)
		{
			struct sTab hashSearchPtr;
			struct hEnt *hashEntryPtr;
			hashEntryPtr = h_firstEnt_ (&hostTab, &hashSearchPtr);
			while (hashEntryPtr)
			{
				hData = (struct hData *) hashEntryPtr->hData;
				hashEntryPtr = h_nextEnt_ (&hashSearchPtr);
				if (hData->hStatus & HOST_STAT_REMOTE)
					continue;
				if (hData->flags & HOST_LOST_FOUND)
					continue;
				hData->flags |= HOST_NEEDPOLL;
				hData->pollTime = 0;
			}
			*allPoll = TRUE;
		}

	}
	else
	{
		for ( unsigned int i = 0; i < qPtr->numAskedPtr; i++)
		{
			if (qPtr->askedPtr[j].hData == NULL) {
				continue;
			}
			qPtr->numProcessors += qPtr->askedPtr[i].hData->numCPUs;
			hData = qPtr->askedPtr[i].hData;
			if ((qPtr->flags & QUEUE_NEEDPOLL) && *allPoll == FALSE)
			{
				hData->flags |= HOST_NEEDPOLL;
				hData->pollTime = 0;
			}
		}
	}

	return 0;
}

struct gData *
addUnixGrp (struct group *unixGrp, const char *grpName, const char *filename, struct gData *groups[], int *ngroups)
{
	int i = -1; // FIXME FIXME FIXME FIXME why start from -1?
	struct gData *gp = NULL;
	struct passwd *pw = NULL;
	char *grpTemp = NULL;

	if (grpName == NULL)
	{
		grpTemp = unixGrp->gr_name;
	}
	else
	{
		grpTemp = grpName;
	}
	gp = addGroup (groups, grpTemp, ngroups);
	while (unixGrp->gr_mem[++i] != NULL)
	{
		if ((pw = getpwlsfuser_ (unixGrp->gr_mem[i]))) {
			addMember (gp, unixGrp->gr_mem[i], USER_GRP, filename, groups, ngroups);
		}
		else{
			/* catgets 6179 */
			ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 6179, "%s: Unknown user <%s> in group <%s>"), filename, unixGrp->gr_mem[i], grpTemp);
			lsb_CheckError = WARNING_ERR;
		}
	}
	if (gp->memberTab.numEnts == 0 && gp->numGroups == 0)
	{
		freeGrp (gp);
		*ngroups -= 1;
		return NULL;
	}
	return gp;

}

void
getClusterData (void)
{
	int i;
	int num;

	TIMEIT (0, clusterName = ls_getclustername (), "minit_ls_getclustername");
	if (clusterName == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_getclustername");
		if (!lsb_CheckMode)
			mbdDie (MASTER_RESIGN);
		else
		{
			lsb_CheckError = FATAL_ERR;
			return;
		}
	}
	if (debug)
	{
		FREEUP (managerIds);
		if (lsbManagers) {
			FREEUP (lsbManagers[0]);
		}
		FREEUP (lsbManagers);
		nManagers = 1;
		lsbManagers = my_malloc (sizeof (char *), __func__);
		lsbManagers[0] = my_malloc (MAX_LSB_NAME_LEN, __func__);
		if (getLSFUser_ (lsbManagers[0], MAX_LSB_NAME_LEN) == 0)
		{

			managerIds = my_malloc (sizeof (uid_t), __func__);
			if (getOSUid_ (lsbManagers[0], &managerIds[0]) != 0)
			{
				ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "getOSUid_");
				if (!lsb_CheckMode)
					mbdDie (MASTER_RESIGN);
				else
				{
					lsb_CheckError = FATAL_ERR;
					return;
				}
				managerIds[0] = -1;
			}
		}
		else
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "getLSFUser_");
			mbdDie (MASTER_RESIGN);
		}
		/*catgets 6256 */
		if (!lsb_CheckMode) {
			ls_syslog (LOG_NOTICE, I18N (6256, "%s: The LSF administrator is the invoker in debug mode"), __func__); 
		}
		lsbManager = lsbManagers[0];
		managerId = managerIds[0];
	}
	num = 0;
	clusterInfo = ls_clusterinfo (NULL, &num, NULL, 0, 0);
	if (clusterInfo == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_clusterinfo");
		if (!lsb_CheckMode) {
			mbdDie (MASTER_RESIGN);
		}
		else {
			lsb_CheckError = FATAL_ERR;
			return;
		}
	}
	else
	{
		if (!debug)
		{
			if (nManagers > 0)
			{
				for ( unsigned int i = 0; i < nManagers; i++) {
					FREEUP (lsbManagers[i]);
				}
				FREEUP (lsbManagers);
				FREEUP (managerIds);
			}
		}

		for ( unsigned int i = 0; i < num; i++) {
			if (!debug && (strcmp (clusterName, clusterInfo[i].clusterName) == 0)) {
				setManagers (clusterInfo[i]);
			}
		}

		if (!nManagers && !debug){
			/* catgets 6182 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6182, "%s: Local cluster %s not returned by LIM"), __func__, clusterName);
			mbdDie (MASTER_FATAL);
		}
		numofclusters = num;
	}
}

void
setManagers (struct clusterInfo clusterInfo)
{
	struct passwd *pw = NULL;
	int numValid = 0
	gid_t gid = 999, 
	int temNum = 0;
	int tempId = 0;
	char *sp = NULL;
	char managerIdStr[MAX_LSB_NAME_LEN]
	char managerStr[MAX_LSB_NAME_LEN];

  temNum = (clusterInfo.nAdmins) ? clusterInfo.nAdmins : 1;
  managerIds = my_malloc (sizeof (uid_t) * temNum, __func__);
  lsbManagers = my_malloc (sizeof (char *) * temNum, __func__);

  for (i = 0; i < temNum; i++)
	{
	  sp = (clusterInfo.nAdmins) ? clusterInfo.admins[i] : clusterInfo.managerName;
	  tempId = (clusterInfo.nAdmins) ? clusterInfo.adminIds[i] : clusterInfo.managerId;

	  lsbManagers[i] = safeSave (sp);

	  if ((pw = getpwlsfuser_ (sp)) != NULL)
  {
	managerIds[i] = pw->pw_uid;
	if (numValid == 0)
	  {
		gid = pw->pw_gid;


		lsbManager = lsbManagers[i];
		managerId = managerIds[i];
	  }
	numValid++;
  }
	  else
  {
	if (logclass & LC_AUTH)
	  {
		ls_syslog (LOG_DEBUG, "%s: Non recognize LSF administrator name <%s> and userId <%d> detected. It may be a user from other realm", __func__, sp, tempId);
	  }
	managerIds[i] = -1;
  }
	}
  if (numValid == 0)
	{
		/* catgets 6184 */
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6184, "%s: No valid LSF administrators"), __func__);
	  mbdDie (MASTER_FATAL);
	}
  nManagers = temNum;

  setgid (gid);


  if (getenv ("LSB_MANAGERID") == NULL)
	{
	  sprintf (managerIdStr, "%d", managerId);
	  putEnv ("LSB_MANAGERID", managerIdStr);
	}

  if (getenv ("LSB_MANAGER") == NULL)
	{
	  sprintf (managerStr, "%s", lsbManager);
	  putEnv ("LSB_MANAGER", managerStr);
	}

	return;
}

void
setParams (struct paramConf *paramConf)
{
  struct parameterInfo *params = NULL;

	if (paramConf == NULL || paramConf->param == NULL) {
		/* catgets 6185 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6185, "%s: paramConf or param in paramConf is NULL; default  parameters will be used"), __func__);
		return;
	}
	params = paramConf->param;
	setString (defaultQueues, params->defaultQueues);
	setString (defaultHostSpec, params->defaultHostSpec);
	setString (lsfDefaultProject, params->defaultProject);

	setValue (msleeptime, params->mbatchdInterval);
	setValue (sbdSleepTime, params->sbatchdInterval);
	setValue (accept_intvl, params->jobAcceptInterval);
	setValue (max_retry, params->maxDispRetries);
	setValue (max_sbdFail, params->maxSbdRetries);
	setValue (clean_period, params->cleanPeriod);
	setValue (maxjobnum, params->maxNumJobs);
	setValue (pgSuspIdleT, params->pgSuspendIt);
	setValue (retryIntvl, params->retryIntvl);
	setValue (rusageUpdateRate, params->rusageUpdateRate);
	setValue (rusageUpdatePercent, params->rusageUpdatePercent);
	setValue (condCheckTime, params->condCheckTime);
	setValue (maxJobArraySize, params->maxJobArraySize);
	setValue (jobRunTimes, params->jobRunTimes);
	setValue (jobDepLastSub, params->jobDepLastSub);

	setValue (maxSbdConnections, params->maxSbdConnections);

	setValue (maxSchedStay, params->maxSchedStay);
	setValue (freshPeriod, params->freshPeriod);
	setValue (jobTerminateInterval, params->jobTerminateInterval);
	setString (pjobSpoolDir, params->pjobSpoolDir);

	setValue (maxUserPriority, params->maxUserPriority);
	setValue (jobPriorityValue, params->jobPriorityValue);
	setValue (jobPriorityTime, params->jobPriorityTime);
	setJobPriUpdIntvl ();


	setValue (sharedResourceUpdFactor, params->sharedResourceUpdFactor);

	setValue (scheRawLoad, params->scheRawLoad);

	setValue (preExecDelay, params->preExecDelay);
	setValue (slotResourceReserve, params->slotResourceReserve);

	setValue (maxJobId, params->maxJobId);

	setValue (maxAcctArchiveNum, params->maxAcctArchiveNum);
	setValue (acctArchiveInDays, params->acctArchiveInDays);
	setValue (acctArchiveInSize, params->acctArchiveInSize);

	return;
}

void
addUData (struct userConf *userConf)
{
	struct userInfoEnt *uPtr = NULL;

	removeFlags (&uDataList, USER_UPDATE, UDATA);
	for( unsigned int i = 0; i < userConf->numUsers; i++) {
		uPtr = &(userConf->users[i]);
		addUserData (uPtr->user, uPtr->maxJobs, uPtr->procJobLimit, __func__, TRUE, TRUE);
		if (strcmp ("default", uPtr->user) == 0)
			defUser = TRUE;
	}

	return
}

void
createTmpGData (struct groupInfoEnt *groups, size_t num, int groupType, struct gData *tempGData[], int *nTempGroups)
{
	struct groupInfoEnt *gPtr = NULL;
	char *HUgroups = NULL;
	char *sp = NULL;
	char *wp = NULL;
	struct gData *grpPtr;

	if (groupType == USER_GRP) {
		HUgroups = "usergroup";
	}
	else {
		HUgroups = "hostgroup";
	}

	for ( size_t i = 0; i < num; i++) {

		gPtr = &(groups[i]);
		if (groupType == HOST_GRP && gPtr && gPtr->group && isHostAlias (gPtr->group)) {
			ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 6186, "%s: Host group name <%s> conflicts with host name; ignoring"), __func__, gPtr->group);  /* catgets 6186 */
			lsb_CheckError = WARNING_ERR;
			continue;
		}

		if (getGrpData (tempGData, gPtr->group, *nTempGroups)) {
			ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 6187, "%s: Group <%s> is multiply defined in %s; ignoring"), __func__, gPtr->group, HUgroups); /* catgets 6187 */
			lsb_CheckError = WARNING_ERR;
		continue;
		}

		grpPtr = addGroup (tempGData, gPtr->group, nTempGroups);
		sp = gPtr->memberList;

		while (strcmp (gPtr->memberList, "all") != 0 && (wp = getNextWord_ (&sp)) != NULL) {
			addMember (grpPtr, wp, groupType, __func__, tempGData, nTempGroups);
		}

		if (grpPtr->memberTab.numEnts == 0 && grpPtr->numGroups == 0 && strcmp (gPtr->memberList, "all") != 0) {
			ls_syslog (LOG_WARNING, _i18n_msg_get (ls_catd, NL_SETN, 6188, "%s: No valid member in group <%s>; ignoring the group"), __func__, grpPtr->group);  /* catgets 6188 */
			lsb_CheckError = WARNING_ERR;
			freeGrp (grpPtr);
			*nTempGroups -= 1;
		}
	}

	return;
}

int
isHostAlias (char *grpName)
{
	struct hEnt *hashEntryPtr = NULL;
	struct sTab hashSearchPtr;
	struct hData *hData = NULL;

	hashEntryPtr = h_firstEnt_ (&hostTab, &hashSearchPtr);
	while (hashEntryPtr){
	// hData = (struct hData *) hashEntryPtr->hData;
		hData = hashEntryPtr->hData;
		hashEntryPtr = h_nextEnt_ (&hashSearchPtr);
	if (hData->hostEnt.h_aliases) {
		for ( unsigned int i = 0; hData->hostEnt.h_aliases[i]; i++) {
			if (equalHost_ (grpName, hData->hostEnt.h_aliases[i]))
				return TRUE;
			}
		}
	}

  return FALSE;
}

/* addHostData()
 * Add a host into MBD internal data structures.
 */
void
addHostData ( unsigned int numHosts, struct hostInfoEnt *hosts)
{
	struct hData hPtr;

	removeFlags (&hostTab, HOST_UPDATE | HOST_AUTOCONF_MXJ, HDATA);
	if (numHosts == 0 || hosts == NULL) {
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6189, "%s: No hosts specified in lsb.hosts; all hosts known by LSF will be used"), __func__);  /* catgets 6189 */
		addDefaultHost ();
		return;
	}

	for ( unsigned int i = 0; i < numHosts; i++) {

		for ( unsigned int j = 0; j < numLIMhosts; j++) {
			if (equalHost_ (hosts[i].host, LIMhosts[j].hostName))
				break;
		}
		if (j == numLIMhosts) {
			/* catgets 6190 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6190, "%s: Host <%s> is not used by the batch system; ignored"), __func__, hosts[i].host);
			continue;
		}
		if (LIMhosts[j].isServer != TRUE) {
			/* catgets 6191 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6191, "%s: Host <%s> is not a server; ignoring"), __func__, hosts[i].host);
			continue;
		}

		if ( !Gethostbyname_( LIMhosts[j].hostName ) ) {
			/* catgets 6242 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6242, "%s: Host <%s> is not a valid host; ignoring"), __func__, hosts[i].host);
			continue;
		}

		/* Initialize the temporary hData structure.
		*/
		initHData (&hPtr);

		/* Copy the host batch host configuration
		* into the temporary host data.
		*/
		hPtr.host = hostConf->hosts[i].host;
		hPtr.uJobLimit = hostConf->hosts[i].userJobLimit;
		hPtr.maxJobs = hostConf->hosts[i].maxJobs;

		if (hostConf->hosts[i].chkSig != INFINIT_INT) {
			hPtr.chkSig = hostConf->hosts[i].chkSig;
		}

		if (hostConf->hosts[i].mig == INFINIT_INT) {
			hPtr.mig = hostConf->hosts[i].mig;
		}
		else {
			hPtr.mig = hostConf->hosts[i].mig * 60;
		}

		hPtr.loadSched = hostConf->hosts[i].loadSched;
		hPtr.loadStop = hostConf->hosts[i].loadStop;
		hPtr.windows = hostConf->hosts[i].windows;

	  /* Add the host by merging the lsf base
	   * host information with the batch configuration.
	   */
		addHost (&LIMhosts[j], &hPtr, __func__);

	} // for (i = 0; i < numHosts; i++)

	return;
}

void
setDefaultParams (void)
{
	FREEUP (defaultQueues);
	FREEUP (defaultHostSpec);
	FREEUP (lsfDefaultProject);
	FREEUP (pjobSpoolDir);

	msleeptime = DEF_MSLEEPTIME;
	sbdSleepTime = DEF_SSLEEPTIME;
	accept_intvl = DEF_ACCEPT_INTVL;
	max_retry = DEF_MAX_RETRY;
	max_sbdFail = DEF_MAXSBD_FAIL;
	preemPeriod = DEF_PREEM_PERIOD;
	clean_period = DEF_CLEAN_PERIOD;
	maxjobnum = DEF_MAX_JOB_NUM;
	pgSuspIdleT = DEF_PG_SUSP_IT;
	retryIntvl = DEF_RETRY_INTVL;
	rusageUpdateRate = DEF_RUSAGE_UPDATE_RATE;
	rusageUpdatePercent = DEF_RUSAGE_UPDATE_PERCENT;
	condCheckTime = DEF_COND_CHECK_TIME;
	maxSbdConnections = DEF_MAX_SBD_CONNS;
	maxSchedStay = DEF_SCHED_STAY;
	freshPeriod = DEF_FRESH_PERIOD;
	maxJobArraySize = DEF_JOB_ARRAY_SIZE;
	jobTerminateInterval = DEF_JTERMINATE_INTERVAL;
	jobRunTimes = INFINIT_INT;
	jobDepLastSub = 0;
	scheRawLoad = 0;
	maxUserPriority = -1;
	jobPriorityValue = -1;
	jobPriorityTime = -1;
	preExecDelay = DEF_PRE_EXEC_DELAY;
	slotResourceReserve = FALSE;
	maxJobId = DEF_MAX_JOBID;
	maxAcctArchiveNum = -1;
	acctArchiveInDays = -1;
	acctArchiveInSize = -1;

	return;
}

void
addQData (struct queueConf *queueConf, int mbdInitFlags)
{
	// int i = 0;
	// int j = 0;
	unsigned int badqueue = 0;
	struct qData *qPtr = NULL;
	struct qData *oldQPtr = NULL;
	struct queueInfoEnt *queue = NULL;
	char *sp = NULL;
	char *word = NULL;
	int queueIndex = 0;

	for( qPtr = qDataList->forw; qPtr != qDataList; qPtr = qPtr->forw) {
		qPtr->flags &= ~QUEUE_UPDATE;
	}

	if( queueConf == NULL || queueConf->numQueues <= 0 || queueConf->queues == NULL) {
		ls_syslog (LOG_ERR, "%s: No valid queue in queueConf structure", __func__);
		lsb_CheckError = WARNING_ERR;
		return;
	}

	for( unsigned int i = 0; i < queueConf->numQueues; i++) {

		queue = &(queueConf->queues[i]);
		qPtr = initQData ();

		qPtr->queue = safeSave (queue->queue);
		if (queue->description) {
			qPtr->description = safeSave (queue->description);
		}
		else {
			qPtr->description = safeSave ("No description provided.");
		}

		setValue (qPtr->priority, queue->priority);

		if (queue->userList) {
			parseGroups (USER_GRP, &qPtr->uGPtr, queue->userList, __func__);
		}

		if (queue->hostList) {

			if (strcmp (queue->hostList, "none") == 0) {
				qPtr->hostList = safeSave (queue->hostList);

			}
			else
			{
				if (searchAll (queue->hostList) == FALSE)
				{
					if (parseQHosts (qPtr, queue->hostList) != 0)
					{
						qPtr->hostList = safeSave (queue->hostList);
					}
				}
			}
		}

		badqueue = FALSE;
		if (qPtr->uGPtr) {
			if (qPtr->uGPtr->memberTab.numEnts == 0 && qPtr->uGPtr->numGroups == 0) {

				ls_syslog (LOG_ERR, "%s: No valid value for key USERS in the queue <%s>; ignoring the queue", __func__, qPtr->queue);
				lsb_CheckError = WARNING_ERR;
				badqueue = TRUE;
			}
		}

		if (qPtr->hostList != NULL && strcmp (qPtr->hostList, "none") && qPtr->numAskedPtr <= 0 && qPtr->askedOthPrio <= 0) {
			ls_syslog (LOG_ERR, "%s: No valid value for key HOSTS in the queue <%s>; ignoring the queue", __func__, qPtr->queue);
			lsb_CheckError = WARNING_ERR;
			badqueue = TRUE;
		}

		if (badqueue) {
			freeQData (qPtr, FALSE);
			continue;
		}

		++queueIndex;
		qPtr->queueId = queueIndex;

		if ((oldQPtr = getQueueData (qPtr->queue)) == NULL) {
			inQueueList (qPtr);
			qPtr->flags |= QUEUE_UPDATE;
		}
		else {
			copyQData (qPtr, oldQPtr);
			oldQPtr->flags |= QUEUE_UPDATE;
			FREEUP (qPtr->queue);
			FREEUP (qPtr->reasonTb[0]);
			FREEUP (qPtr->reasonTb[1]);
			FREEUP (qPtr->reasonTb);
			FREEUP (qPtr);

			if (mbdInitFlags == RECONFIG_CONF || mbdInitFlags == WINDOW_CONF) {

				FREEUP (oldQPtr->reasonTb[0]);
				FREEUP (oldQPtr->reasonTb[1]);
				oldQPtr->reasonTb[0] = my_calloc (numLIMhosts + 2, sizeof (int), __func__);
				oldQPtr->reasonTb[1] = my_calloc (numLIMhosts + 2, sizeof (int), __func__);

				for( unsigned int j = 0; j < numLIMhosts + 2; j++ ) {
					oldQPtr->reasonTb[0][j] = 0;
					oldQPtr->reasonTb[1][j] = 0;
				}
			}
		}
	} // for( unsigned int i = 0; i < queueConf->numQueues; i++)

	for( unsigned int i = 0; i < queueConf->numQueues; i++) {

		queue = &(queueConf->queues[i]);
		qPtr = getQueueData (queue->queue);

		if (queue->nice != INFINIT_SHORT) {
			qPtr->nice = queue->nice;
		}

		setValue (qPtr->uJobLimit, queue->userJobLimit);
		setValue (qPtr->pJobLimit, queue->procJobLimit);
		setValue (qPtr->maxJobs, queue->maxJobs);
		setValue (qPtr->hJobLimit, queue->hostJobLimit);
		setValue (qPtr->procLimit, queue->procLimit);
		setValue (qPtr->minProcLimit, queue->minProcLimit);
		setValue (qPtr->defProcLimit, queue->defProcLimit);
		qPtr->windEdge = 0;

		if (queue->windows != NULL)	{

			qPtr->windows = safeSave (queue->windows);
			*(qPtr->windows) = '\0';
			sp = queue->windows;
			while ((word = getNextWord_ (&sp)) != NULL) {

				char *save = safeSave (word);
				if (addWindow (word, qPtr->weekR, qPtr->queue) < 0) {
					/* catgets NULL */
					ls_syslog (LOG_ERR, "%s: Bad time expression %s/%s in queue window %s; ignored", __func__, queue->windows, save, qPtr->queue);
					lsb_CheckError = WARNING_ERR;
					freeWeek (qPtr->weekR);
					free (save);
					continue;
				}
				qPtr->windEdge = now;
				if (*(qPtr->windows) != '\0')
					strcat (qPtr->windows, " ");
				strcat (qPtr->windows, save);
				free (save);
			}
		}

		if (queue->windowsD) {

			qPtr->windowsD = safeSave (queue->windowsD);
			*(qPtr->windowsD) = '\0';
			sp = queue->windowsD;

			while ((word = getNextWord_ (&sp)) != NULL)
			{
				char *save;
				save = safeSave (word);
				if (addWindow (word, qPtr->week, qPtr->queue) < 0) {
					/* catgets NULL */
					ls_syslog (LOG_ERR, "%s: Bad time expression %s/%s in queue dispatch windows %s; ignored", __func__, queue->windowsD, save, qPtr->queue);
					lsb_CheckError = WARNING_ERR;
					freeWeek (qPtr->week);
					free (save);
					continue;
				}
				qPtr->windEdge = now;
				if (*(qPtr->windowsD) != '\0') {
					strcat (qPtr->windowsD, " ");
				}

				strcat (qPtr->windowsD, save);
				free (save);
			}
		}

		if( !qPtr->windows && (qPtr->qStatus & QUEUE_STAT_RUNWIN_CLOSE)) {
			qPtr->qStatus &= ~QUEUE_STAT_RUNWIN_CLOSE;
		}

		if( !qPtr->windows && !qPtr->windowsD && !(qPtr->qStatus & QUEUE_STAT_RUN)) {
			qPtr->qStatus |= QUEUE_STAT_RUN;
		}

		if (queue->defaultHostSpec) {
			float *cpuFactor = 0.0;

			cpuFactor = getModelFactor (queue->defaultHostSpec);
			if (cpuFactor == NULL) {
				cpuFactor = getHostFactor (queue->defaultHostSpec);
			}
			if (cpuFactor == NULL) {
				/* catgets NULL */
				ls_syslog (LOG_ERR, "%s: Invalid hostspec %s for %s in queue <%s>; ignored", __func__, queue->defaultHostSpec, "DEFAULT_HOST_SPEC", qPtr->queue);
				lsb_CheckError = WARNING_ERR;
			}
			if (cpuFactor != NULL) {
				qPtr->defaultHostSpec = safeSave (queue->defaultHostSpec);
			}
		}

		if (queue->hostSpec) {
			qPtr->hostSpec = safeSave (queue->hostSpec);
		}

		for( unsigned int j = 0; j < LSF_RLIM_NLIMITS; j++)
		{
			if (queue->rLimits[j] == INFINIT_INT) {
				qPtr->rLimits[j] = -1;
			}
			else {
				qPtr->rLimits[j] = queue->rLimits[j];
			}

			if (queue->defLimits[j] == INFINIT_INT) {
				qPtr->defLimits[j] = -1;
			}
			else {
				qPtr->defLimits[j] = queue->defLimits[j];
			}
		}

		qPtr->qAttrib = (qPtr->qAttrib | queue->qAttrib);

		if (queue->mig != INFINIT_INT)
		{
			setValue (qPtr->mig, queue->mig);
			qPtr->mig *= 60;
		}

		if (queue->schedDelay != INFINIT_INT) {
			qPtr->schedDelay = queue->schedDelay;
		}

		if (queue->acceptIntvl != INFINIT_INT) {
			qPtr->acceptIntvl = queue->acceptIntvl;
		}
		else {
			qPtr->acceptIntvl = accept_intvl;
		}

		if (queue->admins) {
			parseAUids (qPtr, queue->admins);
		}

		if (queue->preCmd) {
			qPtr->preCmd = safeSave (queue->preCmd);
		}

		if (queue->prepostUsername) {
			qPtr->prepostUsername = safeSave (queue->prepostUsername);
		}

		if (queue->postCmd) {
			qPtr->postCmd = safeSave (queue->postCmd);
		}

		if (queue->requeueEValues) {
			qPtr->requeueEValues = safeSave (queue->requeueEValues);

			if (qPtr->requeEStruct) {
				clean_requeue (qPtr);
			}
			requeueEParse (&qPtr->requeEStruct, queue->requeueEValues, &j);

		}

		if (queue->resReq)
		{
			qPtr->resValPtr = checkResReq (queue->resReq, USE_LOCAL | PARSE_XOR | CHK_TCL_SYNTAX);
			if (qPtr->resValPtr == NULL) {
				ls_syslog (LOG_ERR, "%s: invalid RES_REQ %s in queues %s; ignoring", __func__, queue->resReq, qPtr->queue);
				lsb_CheckError = WARNING_ERR;
			}
			else
				qPtr->resReq = safeSave (queue->resReq);
		}

		if (queue->slotHoldTime != INFINIT_INT) {
			qPtr->slotHoldTime = queue->slotHoldTime * msleeptime;
		}

		if (queue->resumeCond) {

			struct resVal *resValPtr = NULL;
			resValPtr = checkResReq (queue->resumeCond,	USE_LOCAL | CHK_TCL_SYNTAX);
			if (resValPtr == NULL) {
				/* catgets NULL */
				ls_syslog (LOG_ERR, "%s: invalid RESUME_COND %s in queue %s; ignoring", __func__, queue->resumeCond, qPtr->queue);
				lsb_CheckError = WARNING_ERR;
			}
			else {
				qPtr->resumeCondVal = resValPtr;
				qPtr->resumeCond = safeSave (queue->resumeCond);
			}
		}

		if (queue->stopCond) {

			struct resVal *resValPtr = NULL;
			resValPtr = checkResReq (queue->stopCond, USE_LOCAL | CHK_TCL_SYNTAX);
			if (resValPtr == NULL) {
				/* catgets NULL */
				ls_syslog (LOG_ERR, "%s: invalid STOP_COND %s in queue %s; ignoring", __func__, queue->stopCond, qPtr->queue);
				lsb_CheckError = WARNING_ERR;
			}
			else {
				lsbFreeResVal (&resValPtr);
				qPtr->stopCond = safeSave (queue->stopCond);
			}
		}

		if (queue->jobStarter) {
			qPtr->jobStarter = safeSave (queue->jobStarter);
		}

		if (queue->suspendActCmd != NULL) {
			qPtr->suspendActCmd = safeSave (queue->suspendActCmd);
		}

		if (queue->resumeActCmd != NULL) {
			qPtr->resumeActCmd = safeSave (queue->resumeActCmd);
		}

		if (queue->terminateActCmd != NULL) {
			qPtr->terminateActCmd = safeSave (queue->terminateActCmd);
		}

		for (j = 0; j < LSB_SIG_NUM; j++) {
			qPtr->sigMap[j] = queue->sigMap[j];
		}

		initThresholds (qPtr->loadSched, qPtr->loadStop);
		for( unsigned int j = 0; j < queue->nIdx; j++ ) {
			if (queue->loadSched[j] != INFINIT_FLOAT) {
				qPtr->loadSched[j] = queue->loadSched[j];
			}
			if (queue->loadStop[j] != INFINIT_FLOAT) {
				qPtr->loadStop[j] = queue->loadStop[j];
			}
		}

		setValue (qPtr->chkpntPeriod, queue->chkpntPeriod);
		if (queue->chkpntDir)
		{
			if (qPtr->chkpntDir)
				FREEUP (qPtr->chkpntDir);
			qPtr->chkpntDir = safeSave (queue->chkpntDir);
		}

		if (queue->qAttrib & QUEUE_ATTRIB_CHKPNT) {
			qPtr->qAttrib |= QUEUE_ATTRIB_CHKPNT;
		}

		if (queue->qAttrib & QUEUE_ATTRIB_RERUNNABLE) {
			qPtr->qAttrib |= QUEUE_ATTRIB_RERUNNABLE;
		}

		if (qPtr->qAttrib & QUEUE_ATTRIB_BACKFILL) {
			qAttributes |= QUEUE_ATTRIB_BACKFILL;
		}
	}

	return 0;
}

/* copyGroups()
 */
void
copyGroups (int copyHGroups)
{

	if (copyHGroups == FALSE)
	{
		for ( unsigned int i = 0; i < numofugroups; i++) // FIXME FIXME FIXME wtf is numofugroups?
		{
			if (usergroups[i] == NULL) {
				continue;
			}
			freeGrp (usergroups[i]);
		}
		for ( unsigned int i = 0; i < nTempUGroups; i++) {
			usergroups[i] = tempUGData[i];
			tempUGData[i] = NULL;
		}
		numofugroups = nTempUGroups;
		nTempUGroups = 0;
		return;
	}
	for( unsigned int i = 0; i < numofhgroups; i++) {
		if (hostgroups[i] == NULL) {
			continue;
		}
		freeGrp (hostgroups[i]);
	}

	for( unsigned int i = 0; i < nTempHGroups; i++) {  // FIXME FIXME FIXME wtf is nTempHGroups?
		hostgroups[i] = tempHGData[i];
	}
	numofhgroups = nTempHGroups;
	nTempHGroups = 0;

	return;
} /* copyGroups */

void
createCondNodes (unsigned int numConds, char **conds, char *fileName, int flags)
{
	static int first = TRUE;
	int errcode = 0;
	int jFlags = 0;
	int new = 0;
	struct lsfAuth auth;
	struct condData *condNode = NULL;
	struct hEnt *ent = NULL;

	if (numConds <= 0 || conds == NULL)
	return;

	if (first == TRUE) {
		h_initTab_ (&condDataList, 20);
		first = FALSE;
	}

	auth.uid = batchId;
	strncpy (auth.lsfUserName, batchName, MAX_LSF_NAME_LEN);
	auth.lsfUserName[MAX_LSF_NAME_LEN - 1] = '\0';

	for (unsigned int i = 0; i < numConds; i++) {
		if (conds[i] == NULL || conds[i][0] == '\0') {
			continue;
		}
		if (h_getEnt_ (&condDataList, conds[i])) {
			/* catgets NULL */
			ls_syslog (LOG_DEBUG, "%s: Condition <%s> in file <%s> is multiply specified;retaining old definition", __func__, conds[i], fileName);
			continue;
		}
		condNode = initConfData ();
		condNode->name = safeSave (conds[i]);
		condNode->flags = flags;
		if ((condNode->rootNode = parseDepCond (conds[i], &auth, &errcode, NULL, &jFlags, 0)) == NULL) {
			/* catgets NULL */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6207, "%s: Bad condition <%s> in file <%s> :%M; ignored"), __func__, conds[i], fileName);  /* catgets 6207 */
			lsb_CheckError = WARNING_ERR;
		}

		ent = h_addEnt_ (&condDataList, condNode->name, &new);
		ent->hData = condNode;
		continue;
	}

	return;
}

int
updAllConfCond (void)
{
	int needReadAgain = FALSE;

	if (hostFileConf != NULL && updCondData (hostFileConf, HOST_FILE) == TRUE) {
		needReadAgain = TRUE;
	}
	if (userFileConf != NULL && updCondData (userFileConf, USER_FILE) == TRUE) {
		needReadAgain = TRUE;
	}
	if (queueFileConf != NULL && updCondData (queueFileConf, QUEUE_FILE) == TRUE) {
		needReadAgain = TRUE;
	}
	if (paramFileConf != NULL && updCondData (paramFileConf, PARAM_FILE) == TRUE) {
		needReadAgain = TRUE;
	}

	return needReadAgain;
}

int
updCondData (struct lsConf *conf, int fileType)
{
	struct hEnt *hashEntryPtr = NULL;
	struct condData *condition = NULL;
	int status = 0;
	int needReadAgain = FALSE;

	if (logclass & LC_TRACE)
		ls_syslog (LOG_DEBUG2, "%s: Entering this routine...", __func__);

	if (conf == NULL)
	{
	  ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6208, "%s: NULL Conf pointer for fileType <%x>"), __func__, fileType); /* catgets 6208 */
		return FALSE;
	}
	if (conf->numConds <= 0 || conf->conds == NULL)
		return FALSE;

	if (logclass & LC_TRACE)
		ls_syslog (LOG_DEBUG2, "%s: numConds <%d>, fileType <%x>", __func__,
			conf->numConds, fileType);

	for (unsigned int i = 0; i < conf->numConds; i++)
	{
		if ((hashEntryPtr = h_getEnt_ (&condDataList, conf->conds[i])) == NULL) {
			continue;
		}
		// condition = (struct condData *) hashEntryPtr->hData;
		condition = hashEntryPtr->hData;
		if (condition->rootNode == NULL) {
			continue;
		}
		status = evalDepCond (condition->rootNode, NULL);
		if (status == DP_TRUE) {
			status = TRUE;
		}
		else {
			status = FALSE;
		}

		if (status == TRUE) {
			conf->values[i] = 1;
		}
		else {
			conf->values[i] = 0;
		}

		if (status != condition->status)
		{
			condition->lastStatus = condition->status;
			condition->status = status;
			condition->lastTime = now;
			needReadAgain = TRUE;
		}
		if (logclass & LC_TRACE)
			/* catgets NULL */
			ls_syslog (LOG_DEBUG3, "%s: Condition name <%s> for %dth condition status <%d> lastStatus <%d>", __func__, conf->conds[i], i + 1, condition->status, condition->lastStatus);
	}

	return needReadAgain;
}

struct condData *
initConfData (void)
{
	struct condData *cData = NULL;

	cData = my_malloc( sizeof (struct condData), __func__ );
	cData->name = NULL;
	cData->status = FALSE;
	cData->lastStatus = FALSE;
	cData->lastTime = now;
	cData->flags = 0;
	cData->rootNode = NULL;
	
	return cData;
}

struct lsConf *
getFileConf (char *fileName, int fileType)
{

	struct lsConf *confPtr = NULL;

	confPtr = ls_getconf (fileName);
	if (confPtr == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "ls_getconf");
		if (lserrno != LSE_NO_FILE)
		{
			if (lsb_CheckMode)
			{
				lsb_CheckError = FATAL_ERR;
				return NULL;
			}
			else {
				mbdDie (MASTER_FATAL);
			}
		}

	}
	if (confPtr != NULL)
	{
		createCondNodes (confPtr->numConds, confPtr->conds, fileName, fileType);
		updCondData (confPtr, fileType);
	}
	return confPtr;

}

void
copyQData (struct qData *fromQp, struct qData *toQp)
{
#define copyString(string, newPointer) {        \
	FREEUP(string);                         \
	string = newPointer;}

	if (needPollQHost (fromQp, toQp)) {
		toQp->flags |= QUEUE_NEEDPOLL;
	}


	copyString (toQp->description, fromQp->description);
	copyString (toQp->resReq, fromQp->resReq);
	copyString (toQp->preCmd, fromQp->preCmd);
	copyString (toQp->postCmd, fromQp->postCmd);
	copyString (toQp->prepostUsername, fromQp->prepostUsername);
	copyString (toQp->requeueEValues, fromQp->requeueEValues);
	copyString (toQp->windowsD, fromQp->windowsD);
	copyString (toQp->windows, fromQp->windows);
	copyString (toQp->hostSpec, fromQp->hostSpec);
	copyString (toQp->defaultHostSpec, fromQp->defaultHostSpec);
	copyString (toQp->resumeCond, fromQp->resumeCond);
	copyString (toQp->stopCond, fromQp->stopCond);
	copyString (toQp->jobStarter, fromQp->jobStarter);


	copyString (toQp->suspendActCmd, fromQp->suspendActCmd);
	copyString (toQp->resumeActCmd, fromQp->resumeActCmd);
	copyString (toQp->terminateActCmd, fromQp->terminateActCmd);

	copyString (toQp->hostList, fromQp->hostList);


	toQp->numProcessors = fromQp->numProcessors;
	toQp->mig = fromQp->mig;
	toQp->priority = fromQp->priority;
	toQp->nice = fromQp->nice;
	toQp->procLimit = fromQp->procLimit;
	toQp->minProcLimit = fromQp->minProcLimit;
	toQp->defProcLimit = fromQp->defProcLimit;
	toQp->hJobLimit = fromQp->hJobLimit;
	toQp->pJobLimit = fromQp->pJobLimit;
	toQp->uJobLimit = fromQp->uJobLimit;
	toQp->maxJobs = fromQp->maxJobs;
	toQp->slotHoldTime = fromQp->slotHoldTime;
	toQp->askedOthPrio = fromQp->askedOthPrio;
	toQp->numAskedPtr = fromQp->numAskedPtr;
	toQp->queueId = fromQp->queueId;


	if (toQp->uGPtr) {
		freeGrp (toQp->uGPtr);
	}
	toQp->uGPtr = fromQp->uGPtr;

	FREEUP (toQp->askedPtr);
	toQp->askedPtr = fromQp->askedPtr;

	FREEUP (toQp->loadSched);
	toQp->loadSched = fromQp->loadSched;

	FREEUP (toQp->loadStop);
	toQp->loadStop = fromQp->loadStop;

	lsbFreeResVal (&toQp->resValPtr);
	toQp->resValPtr = fromQp->resValPtr;

	lsbFreeResVal (&toQp->resumeCondVal);
	toQp->resumeCondVal = fromQp->resumeCondVal;

	for ( unsigned int i = 0; i < LSF_RLIM_NLIMITS; i++) {
		toQp->rLimits[i] = fromQp->rLimits[i];
		toQp->defLimits[i] = fromQp->defLimits[i];
	}

	if (toQp->nAdmins > 0) {
		FREEUP (toQp->adminIds);
		FREEUP (toQp->admins);
	}

	toQp->nAdmins = fromQp->nAdmins;
	toQp->adminIds = fromQp->adminIds;
	toQp->admins = fromQp->admins;

	if (toQp->weekR) {
		freeWeek (toQp->weekR);
	}

	if (toQp->week) {
		freeWeek (toQp->week);
	}

	for( unsigned int i = 0; i < 8; i++) {
		toQp->weekR[i] = fromQp->weekR[i];
		toQp->week[i] = fromQp->week[i];
	}

	toQp->qAttrib = fromQp->qAttrib;

	return;
}

void
addDefaultHost (void)
{
	struct hData hData;

	for ( unsigned int i = 0; i < numLIMhosts; i++)
	{
		if (LIMhosts[i].isServer != TRUE) {
			continue;
		}
		initHData (&hData);
		hData.host = LIMhosts[i].hostName;
		addHost (&LIMhosts[i], &hData, "addDefaultHost");
	}

	return;
}

void
removeFlags (struct hTab *dataList, int flags, int listType)
{
	struct sTab sTabPtr;
	struct hEnt *hEntPtr = NULL;
	struct hData *hData  = NULL;
	struct uData *uData  = NULL;

	hEntPtr = h_firstEnt_ (dataList, &sTabPtr);
	while (hEntPtr) {
		switch (listType)
		{
			case HDATA: {
				hData = (struct hData *) hEntPtr->hData;
				hData->flags &= ~(flags);	
				break;
			}
			case UDATA: {
					uData = (struct uData *) hEntPtr->hData;
					uData->flags &= ~(flags);
				break;
			}
			default: {
				/* catgets 6210 */
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6210, "%s: Bad data list type <%d>"), __func__, listType);
				return;
			}
		}
		hEntPtr = h_nextEnt_ (&sTabPtr);
	}

	return;
}

int
needPollQHost (struct qData *newQp, struct qData *oldQp)
{

	if ((newQp->resumeCond == NULL && oldQp->resumeCond != NULL) ||
		(newQp->resumeCond != NULL && oldQp->resumeCond == NULL) ||
		(newQp->resumeCond != NULL && oldQp->resumeCond != NULL &&
			strcmp (newQp->resumeCond, oldQp->resumeCond)))
		return TRUE;
	if ((newQp->stopCond == NULL && oldQp->stopCond != NULL) ||
		(newQp->stopCond != NULL && oldQp->stopCond == NULL) ||
		(newQp->stopCond != NULL && oldQp->stopCond != NULL &&
			strcmp (newQp->stopCond, oldQp->stopCond)))
		return TRUE;

	if ((newQp->windows == NULL && oldQp->windows != NULL) ||
		(newQp->windows != NULL && oldQp->windows == NULL) ||
		(newQp->windows != NULL && oldQp->windows != NULL &&
			strcmp (newQp->windows, oldQp->windows)))
		return TRUE;

	if (newQp->nice != oldQp->nice || newQp->mig != oldQp->mig)
		return TRUE;

	if (newQp->rLimits[LSF_RLIMIT_RUN] != oldQp->rLimits[LSF_RLIMIT_RUN] ||
		newQp->rLimits[LSF_RLIMIT_CPU] != oldQp->rLimits[LSF_RLIMIT_CPU])
		return TRUE;

	return FALSE;

}

void
updHostList (void)
{
	struct qData *qPtr = NULL;
	struct uData *uPtr = NULL;
	struct hData *hPtr = NULL;
	struct sTab stab;
	struct hEnt *e = NULL;
	int cc = 0;

	ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);

	if (hostList) {
		FREEUP (hostList);
	}

	hostList = listCreate ("Host List");

	cc = 0;
	for(e = h_firstEnt_ (&hostTab, &stab); e != NULL; e = h_nextEnt_ (&stab)) {

		hPtr = (struct hData *) e->hData;

	  /* These two type of hosts arent' used
	   * for scheduling so don't include them in
	   * the working list.
	   */
		if (hPtr->hStatus & HOST_STAT_REMOTE) {
			continue;
		}

		if (hPtr->flags & HOST_LOST_FOUND) {
			continue;
		}

		hPtr->flags &= ~HOST_UPDATE;
		hPtr->hStatus &= ~HOST_STAT_EXCLUSIVE;

		hPtr->hostId = cc;
		++cc;

		/* Hopsa in da lista...
		*/
		listInsertEntryAtBack (hostList, (LIST_ENTRY_T *) hPtr);

		if (hPtr->maxJobs > 0 && hPtr->maxJobs < INFINIT_INT) {
			hPtr->numCPUs = hPtr->maxJobs;
		}

		if (hPtr->numCPUs > 0) {
			numofprocs += hPtr->numCPUs;
		}
		else {
			numofprocs++;
		}
	}

	if (hReasonTb != NULL) {
		FREEUP (hReasonTb[0]);
		FREEUP (hReasonTb[1]);
		FREEUP (hReasonTb);
	}
	/* Resize the reason table...
	 */
	hReasonTb = my_calloc (2, sizeof (int *), __func__);
	hReasonTb[0] = my_calloc (numofhosts () + 1, sizeof (int), __func__);
	hReasonTb[1] = my_calloc (numofhosts () + 1, sizeof (int), __func__);

	/* Resize the other reason tables...
     */
	for (qPtr = qDataList->forw; qPtr != qDataList; qPtr = qPtr->forw) {
		FREEUP (qPtr->reasonTb);
		qPtr->reasonTb = my_calloc (2, sizeof (int *), __func__);
		qPtr->reasonTb[0] = my_calloc (numofhosts () + 1, sizeof (int), __func__);
		qPtr->reasonTb[1] = my_calloc (numofhosts () + 1, sizeof (int), __func__);
	}

	e = h_firstEnt_ (&uDataList, &stab);
	while (e) {
		uPtr = e->hData;
		FREEUP (uPtr->reasonTb);
		uPtr->reasonTb = my_calloc (2, sizeof (int *), __func__);
		uPtr->reasonTb[0] = my_calloc (numofhosts () + 1, sizeof (int), __func__);
		uPtr->reasonTb[1] = my_calloc (numofhosts () + 1, sizeof (int), __func__);
		e = h_nextEnt_ (&stab);
	}

	checkHWindow ();

	return;
}

void
updQueueList (void)
{
	int list = 0; 
	int allHosts = FALSE;
	struct qData *qp = NULL;
	struct qData *lost_and_found = NULL;
	struct qData *next =NULL;
	struct jData *jpbw = NULL;
	struct qData *entry = NULL;
	struct qData *newqDataList = NULL;

	if ((newqDataList = (struct qData *) listCreate ("Queue List")) == NULL)	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "listCreate");
		mbdDie (MASTER_FATAL);
	}

	for (entry = qDataList->forw; entry != qDataList; entry = next) {
		next = entry->forw;
		listRemoveEntry ((LIST_T *) qDataList, (LIST_ENTRY_T *) entry);
		for (qp = newqDataList->forw; qp != newqDataList; qp = qp->forw) {
			if (entry->priority < qp->priority) {
				break;
			}
		}

		inList ((struct listEntry *) qp, (struct listEntry *) entry);
	}
	listDestroy ((LIST_T *) qDataList, NULL);
	qDataList = newqDataList;

	for (qp = qDataList->forw; qp != qDataList; qp = next) {
		next = qp->forw;
		if (strcmp (qp->queue, LOST_AND_FOUND) == 0) {
			continue;
		}
		if (qp->flags & QUEUE_UPDATE) {
			qp->flags &= ~QUEUE_UPDATE;
			continue;
		}

		if (lost_and_found == NULL && (lost_and_found = getQueueData (LOST_AND_FOUND)) == NULL) {
			lost_and_found = lostFoundQueue ();
		}

		for (list = SJL; list < FJL; list++) {
			for (jpbw = jDataList[list]->back; jpbw != jDataList[list]; jpbw = jpbw->back) {
				if (jpbw->qPtr != qp) {
					continue;
				}

				updSwitchJob (jpbw, qp, lost_and_found,
				jpbw->shared->jobBill.maxNumProcessors);
			}
		}

	  freeQData (qp, TRUE);
	}

	numofqueues = 0;
	for (qp = qDataList->forw; qp != qDataList; qp = qp->forw) {
		queueHostsPF (qp, &allHosts);
		createQueueHostSet (qp);
		numofqueues++;
	}

	checkQWindow ();

	return;
}

void
updUserList (int mbdInitFlags)
{
	struct uData *defUser = NULL;
	struct gData *gPtr = NULL;
	char *key = NULL;
	struct hEnt *hashTableEnt = NULL;
	// int i;

	defUser = getUserData ("default");

	if (defUser && !(defUser->flags & USER_UPDATE))
		defUser = NULL;

	FOR_EACH_HTAB_ENTRY (key, hashTableEnt, &uDataList)
	{
		struct uData *uData;

		uData = (struct uData *) hashTableEnt->hData;


		if (mbdInitFlags == RECONFIG_CONF || mbdInitFlags == WINDOW_CONF)
		{
			int j;
			FREEUP (uData->reasonTb[0]);
			FREEUP (uData->reasonTb[1]);
			uData->reasonTb[0] = my_calloc (numLIMhosts + 2, sizeof (int), __func__);
			uData->reasonTb[1] = my_calloc (numLIMhosts + 2, sizeof (int), __func__);

			for (j = 0; j < numLIMhosts + 2; j++)
			{
				uData->reasonTb[0][j] = 0;
				uData->reasonTb[1][j] = 0;
			}
		}

		if (uData->flags & USER_UPDATE)
		{

			if ((gPtr = getUGrpData (uData->user)) != NULL)
			{
				if (mbdInitFlags != FIRST_START &&
					uData->numJobs == 0 &&
					uData->numPEND == 0 &&
					uData->numRUN == 0 &&
					uData->numSSUSP == 0 &&
					uData->numUSUSP == 0 && uData->numRESERVE == 0)

					addUGDataValues (uData, gPtr);
			}

			if (uData != defUser)
				uData->flags &= ~USER_UPDATE;
			continue;

		}
		else if (getpwlsfuser_ (uData->user) != NULL)
		{

			if (defUser != NULL)
			{

				uData->pJobLimit = defUser->pJobLimit;
				uData->maxJobs = defUser->maxJobs;
			}
			else
			{
				uData->pJobLimit = INFINIT_FLOAT;
				uData->maxJobs = INFINIT_INT;
			}

		}
		else
		{

			if (mbdInitFlags != RECONFIG_CONF && mbdInitFlags != WINDOW_CONF)
			{

				FREEUP (uData->user);

				FREEUP (uData->reasonTb[0]);
				FREEUP (uData->reasonTb[1]);
				FREEUP (uData->reasonTb);
				setDestroy (uData->ancestors);
				uData->ancestors = NULL;
				setDestroy (uData->parents);
				uData->parents = NULL;
				setDestroy (uData->children);
				uData->children = NULL;
				setDestroy (uData->descendants);
				uData->descendants = NULL;

				if (uData->hAcct)
					h_delTab_ (uData->hAcct);
				h_delEnt_ (&uDataList, hashTableEnt);
			}
		}

	}
	END_FOR_EACH_HTAB_ENTRY;


	uDataGroupCreate ();


	setuDataCreate ();



	if (mbdInitFlags == RECONFIG_CONF || mbdInitFlags == WINDOW_CONF)
	{
		struct uData *uData;


		for (unsigned int i = 0; i < numofugroups; i++)
		{

			uData = getUserData (usergroups[i]->group);

			if (defUser != NULL)
			{

				if (uData->maxJobs == INFINIT_INT)
				{
					uData->maxJobs = defUser->maxJobs;
				}
				if (uData->pJobLimit == INFINIT_FLOAT)
				{
					uData->pJobLimit = defUser->pJobLimit;
				}


			}

		}
	}

	return;
}


void
addUGDataValues (struct uData *gUData, struct gData *gPtr)
{
	//int i, 
	unsigned int numMembers = NULL;
	char **groupMembers = NULL;
	struct uData *uPtr  = NULL;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG, "addUGDataValues: Enter the routine...");
	}

	groupMembers = expandGrp (gPtr, gUData->user, &numMembers);

	if (numMembers == 1 && strcmp (gUData->user, groupMembers[0]) == 0)
	{
		/* catgets 6215 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6215, "%s: User group <%s> with no members is ignored"), __func__, gUData->user);
	}
	else
	{
		for ( unsigned int i = 0; i < numMembers; i++) {
			if ((uPtr = getUserData (groupMembers[i])) == NULL) {
				continue;
			}
			addUGDataValues1 (gUData, uPtr);
		}
		FREEUP (groupMembers);
	}

	return;
}

void
addUGDataValues1 (struct uData *gUData, struct uData *uData)
{
	struct sTab hashSearchPtr;
	struct hEnt *hashEntryPtr = 0;
	struct hostAcct *hAcct = 0;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG, "addUGDataValues1: Enter the routine...");
	}

	gUData->numJobs += uData->numJobs;
	gUData->numPEND += uData->numPEND;
	gUData->numRUN += uData->numRUN;
	gUData->numSSUSP += uData->numSSUSP;
	gUData->numUSUSP += uData->numUSUSP;
	gUData->numRESERVE += uData->numRESERVE;

	if (uData->hAcct == NULL) {
		return;
	}

	hashEntryPtr = h_firstEnt_ (uData->hAcct, &hashSearchPtr);
	while (hashEntryPtr) {
		// hAcct = (struct hostAcct *) hashEntryPtr->hData;
		hAcct = hashEntryPtr->hData;
		hashEntryPtr = h_nextEnt_ (&hashSearchPtr);
		addHAcct (&gUData->hAcct, hAcct->hPtr, hAcct->numRUN, hAcct->numSSUSP,
		hAcct->numUSUSP, hAcct->numRESERVE);
	}

	return;
}

int
parseQHosts (struct qData *qp, char *hosts)
{

	// int i;
	int numAskedHosts = 0;
	struct askedHost *returnHosts = NULL;
	int returnErr = 0;
	int badReqIndx = 0;
	int others  = 0;
	int numReturnHosts = 0;
	char *sp = NULL;
	char *word = NULL;
	char  **askedHosts = NULL;

	if (qp == NULL) {
		/* catgets 6216 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6216, "%s: NULL pointer for queue"), __func__);
		return -1;
	}
	if (hosts == NULL)
	{
		/* catgets 6217 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6217, "%s: NULL pointer in HOSTS for queue <%s>"),  __func__, qp->queue);
		return -1;
	}

	sp = hosts;

	while ((word = getNextWord_ (&sp)) != NULL) {
		numAskedHosts++;
	}
	if (numAskedHosts == 0) {
		/* catgets 6218 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6218, "%s: No host specified in HOSTS for queue <%s>"), __func__, qp->queue);
		return -1;
	}
	askedHosts = (char **) my_calloc (numAskedHosts, sizeof (char *), __func__);
	sp = hosts;
	numAskedHosts = 0;
	while ((word = getNextWord_ (&sp)) != NULL)
	{
		askedHosts[numAskedHosts++] = safeSave (word);
	}

	returnErr = chkAskedHosts (numAskedHosts, askedHosts, numofprocs, &numReturnHosts, &returnHosts, &badReqIndx, &others, 0);
	if (returnErr == LSBE_NO_ERROR)
	{
		if (numReturnHosts > 0)
		{
			qp->askedPtr = my_calloc (numReturnHosts, sizeof (struct askedHost), __func__);
			for( unsigned int i = 0; i < numReturnHosts; i++)
			{
				qp->askedPtr[i].hData = returnHosts[i].hData;
				qp->askedPtr[i].priority = returnHosts[i].priority;
				if (qp->askedPtr[i].priority > 0)
					qp->qAttrib |= QUEUE_ATTRIB_HOST_PREFER;
			}

			qp->numAskedPtr = numReturnHosts;
			qp->askedOthPrio = others;
			FREEUP (returnHosts);
		}
	}
	else {
		if (parseFirstHostErr(returnErr, __func__, hosts, qp, returnHosts, numReturnHosts))
		{
			/* catgets 6219 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6219, "%s: No valid host used by the batch system is specified in HOSTS <%s> for queue <%s>"), __func__, hosts, qp->queue);
		}
	}

	for (i = 0; i < numAskedHosts; i++) {
		FREEUP (askedHosts[i]);
	}
	FREEUP (askedHosts);

	if (returnErr == LSBE_NO_ERROR)	{
		return numReturnHosts;
	}
	else {
		return -1;
	}

	return;
}

void
fillClusterConf (struct clusterConf *clusterConf)
{

	clusterConf->clinfo = clusterInfo;
	clusterConf->numHosts = numLIMhosts;
	clusterConf->hosts = LIMhosts;

	return;
}

void
fillSharedConf (struct sharedConf *sConf)
{
	sConf->lsinfo = allLsInfo;
	sConf->servers = NULL;

	sConf->clusterName = (char *) my_malloc (sizeof (char *), __func__);

	return;
}

void
freeGrp (struct gData *grpPtr)
{
	if (grpPtr == NULL) {
		return;
	}
	h_delTab_ (&grpPtr->memberTab);
	if (grpPtr->group && grpPtr->group[0] != '\0') {
		FREEUP (grpPtr->group);
	}
	FREEUP (grpPtr);

	return;
}

int
validHostSpec (char *hostSpec)
{

	if (hostSpec == NULL) {
		return FALSE;
	}

	if (getModelFactor (hostSpec) == NULL)
	{
		if (getHostFactor (hostSpec) == NULL) {
			return FALSE;
		}
	}

	return TRUE;
}

void
getMaxCpufactor (void)
{
	struct hData *hPtr = NULL;

	for (hPtr = (struct hData *) hostList->back; hPtr != (void *) hostList; hPtr = (struct hData *) hPtr->back) {
		if (hPtr->cpuFactor > maxCpuFactor) {
			maxCpuFactor = hPtr->cpuFactor;
		}
	}

	return;
}

int
parseFirstHostErr (int returnErr, char *__func__, char *hosts, struct qData *qp, struct askedHost *returnHosts, int numReturnHosts)
{

	if ((returnErr == LSBE_MULTI_FIRST_HOST)
		|| (returnErr == LSBE_HG_FIRST_HOST)
		|| (returnErr == LSBE_HP_FIRST_HOST)
		|| (returnErr == LSBE_OTHERS_FIRST_HOST))
	{

		if (returnErr == LSBE_MULTI_FIRST_HOST) {
			 /*catgets 6242 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6242, "%s: Multiple first execution hosts specified <%s> in HOSTS section for queue <%s>. Ignored."), __func__, hosts, qp->queue);
		}
		else if (returnErr == LSBE_HG_FIRST_HOST) {
			/* catgets 6243 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6243, "%s: host group <%s> specified as first execution host in HOSTS section for queue <%s>. Ignored."), __func__, hosts, qp->queue);
		}
		else if (returnErr == LSBE_HP_FIRST_HOST) {
			/* catgets 6244 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6244, "%s: host partition <%s> specified as first execution hosts specified <%s> in HOSTS section for queue <%s>. Ignored."), __func__, hosts, qp->queue);
		}
		else if (returnErr == LSBE_OTHERS_FIRST_HOST) {
			/* catgets 6245 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 6245, "%s: \"others\" specified as first execution host specified in HOSTS section for queue <%s>. Ignored."), __func__, qp->queue);
		}
		if (returnHosts) {
			FREEUP (returnHosts);
		}
		qp->numAskedPtr = numReturnHosts;
		if (numReturnHosts > 0)
		{
			qp->askedPtr = my_calloc (numReturnHosts, sizeof (struct askedHost), __func__);
			for ( unsigned int i = 0; i < numReturnHosts; i++) {

				qp->askedPtr[i].hData = returnHosts[i].hData;
				qp->askedPtr[i].priority = returnHosts[i].priority;
				if (qp->askedPtr[i].priority > 0) {
					qp->qAttrib |= QUEUE_ATTRIB_HOST_PREFER;
				}
			}
		}
		else {
			qp->askedPtr = NULL;
			qp->askedOthPrio = 0;
			hosts = NULL;
		}
		return 0;
	}
	else {
		return 1;
	}

	return 0;
}
