/* $Id: lsb.reason.c 397 2007-11-26 19:04:00Z mblack $
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

 * You should have received a copy of the../../../lsf/include/lsb/lsbatch.h GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include <unistd.h>
#include <pwd.h>

#include "lsb/lsb.h"
#include "lsb/reason.h"

// #define   NL_SETN     13

static char msgbuf[MSGSIZE];
static char msgline[MAXLINELEN];

char *getMsg (struct msgMap *msgMap, int *msg_ID, int number)
{
	for ( unsigned int i = 0; msgMap[i].message != NULL; i++) {
		if (msgMap[i].number == number) {
			return _i18n_msg_get (ls_catd, NL_SETN, msg_ID[i], msgMap[i].message);
		}
	}

	return "";  // FIXME FIXME FIXME the fuck?
}

char *lsb_suspreason ( unsigned int reasons, unsigned int subreasons, struct loadIndexLog *ld)
{
	memset( msgbuf, 0, strlen( msgbuf ) );
	// msgbuf[0] = '\0';	// FIXME FIXME FIXME FIXME why is set to \0 ? 

	if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC))  {
		ls_syslog (LOG_DEBUG1, "%s: reasons=%x, subreasons=%d", __func__, reasons, subreasons);
	}


	if (reasons & SUSP_USER_STOP) {
		/* catgets 500 */
		sprintf (msgbuf, "catgets 500:  The job was suspended by user;\n");   
	}
	else if (reasons & SUSP_ADMIN_STOP) {
		/* catgets 501 */
		sprintf (msgbuf, "catgets 501:  The job was suspended by LSF admin or root;\n");  
	}
	else if (reasons & SUSP_QUEUE_WINDOW) {
		/* catgets 502 */
		sprintf (msgbuf, "catgets 502:  The run windows of the queue are closed;\n"); 
	}
	else if (reasons & SUSP_HOST_LOCK) {
		/* catgets 506 */
		sprintf (msgbuf, "catgets 506:  The execution host is locked by LSF administrator now;\n");   
	}
	else if (reasons & SUSP_HOST_LOCK_MASTER) {
		/* catgets 531 */
		sprintf (msgbuf, "catgets 531: The execution host is locked by master LIM now;\n");   
	}
	else if (reasons & SUSP_USER_RESUME) {
		/* catgets 507 */
		sprintf (msgbuf, "catgets 507:  Waiting for re-scheduling after being resumed by user;\n");   
	}
	else if (reasons & SUSP_QUEUE_STOP_COND) {
		/* catgets 508 */
		sprintf (msgbuf, "catgets 508:  STOP_COND is true with current host load;\n");    
	}
	else if (reasons & SUSP_QUEUE_RESUME_COND) {
		/* catgets 509 */
		sprintf (msgbuf, "catgets 509:  RESUME_COND is false with current host load;\n"); 
	}
	else if (reasons & SUSP_RES_RESERVE) {
		/* catgets 510 */
		sprintf (msgbuf, "catgets 510:  Job's requirements for resource reservation not satisfied;\n");   
	}
	else if (reasons & SUSP_PG_IT) {
		/* catgets 511 */
		sprintf (msgbuf, "catgets 511:  Job was suspended due to paging rate (pg) and the host is not idle yet\n");   
	}
	else if (reasons & SUSP_LOAD_UNAVAIL) {
		/* catgets 512 */
		sprintf (msgbuf, "catgets 512:  Load information on execution host(s) is unavailable\n"); 
	}
	else if (reasons & SUSP_LOAD_REASON) {

		/* catgets 513 */
		strcpy (msgbuf, "catgets 513:  Host load exceeded threshold: ");    
		if (subreasons == R15S) {
			/* catgets 514 */
			sprintf (msgbuf, "catgets 514: %s 15-second CPU run queue length (r15s)\n", msgbuf);  
		}
		else if (subreasons == R1M) {
			/* catgets 515 */
			sprintf (msgbuf, "catgets 515: %s 1-minute CPU run queue length (r1m)\n", msgbuf);    
		}
		else if (subreasons == R15M) {
			/* catgets 516 */
			sprintf (msgbuf, "catgets 516: %s 15-minute CPU run queue length (r15m)\n", msgbuf);  
		}
		else if (subreasons == UT) {
			/* catgets 517 */
			sprintf (msgbuf, "catgets 517: %s 1-minute CPU utilization (ut)\n", msgbuf);  
		}
		else if (subreasons == IO) {
			/* catgets 518 */
			sprintf (msgbuf, "catgets 518: %s Disk IO rate (io)\n", msgbuf);  
		}
		else if (subreasons == PG) {
			/* catgets 519 */
			sprintf (msgbuf, "catgets 519: %s Paging rate (pg)\n", msgbuf);   
		}
		else if (subreasons == IT) {
			/* catgets 520 */
			sprintf (msgbuf, "catgets 520: %s Idle time (it)\n", msgbuf); 
		}
		else if (subreasons == MEM) {
			/* catgets 521 */
			sprintf (msgbuf, "catgets 521: %s Available memory (mem)\n", msgbuf); 
		}
		else if (subreasons == SWP) {
			/* catgets 522 */
			sprintf (msgbuf, "catgets 522: %s Available swap space (swp)\n", msgbuf); 
		}
		else if (subreasons == TMP) {
			/* catgets 523 */
			sprintf (msgbuf, "catgets 523: %s Available /tmp space (tmp)\n", msgbuf); 
		}
		else if (subreasons == LS) {
			/* catgets 524 */
			sprintf (msgbuf, "catgets 524: %s Number of login sessions (ls)\n", msgbuf);  
		}
		else {
			userIndexReasons (msgline, 0, subreasons, ld);
			strcat (msgbuf, "  ");
			strcat (msgbuf, msgline);
			strcat (msgbuf, "\n");
		}
	}
	else if (reasons & SUSP_RES_LIMIT) {

		if (subreasons & SUB_REASON_RUNLIMIT) {
			/* catgets 525 */
			sprintf (msgbuf, "catgets 525:  RUNLIMIT was reached;\n");    
		}
		else if (subreasons & SUB_REASON_DEADLINE) {
			/* catgets 526 */
			sprintf (msgbuf, "catgets 526:  DEADLINE was reached;\n");    
		}
		else if (subreasons & SUB_REASON_PROCESSLIMIT) {
			/* catgets 527 */
			sprintf (msgbuf, "catgets 527:  PROCESSLIMIT was reached;\n");    
		}
		else if (subreasons & SUB_REASON_CPULIMIT) {
			/* catgets 529 */
			sprintf (msgbuf, "catgets 529: CPULIMIT was reached;\n");   
		}
		else if (subreasons & SUB_REASON_MEMLIMIT) {
			/* catgets 530 */
			sprintf (msgbuf, "catgets 530: MEMLIMIT was reached;\n");
		}
	}
	else {
		/* catgets 528 */
		sprintf (msgbuf, "catgets 528:  Unknown suspending reason code: %d\n", reasons);
	}

	return msgbuf;
}

char *lsb_pendreason ( unsigned int numReasons, int *rsTb, struct jobInfoHead *jInfoH, struct loadIndexLog *ld)
{
	int num = 0;
	int hostId = 0;
	int reason = 0;
	int hostIdJ = 0;
	int reasonJ = 0;
	int *reasonTb = NULL;
	static unsigned int memSize = 0;
	char *hostList = NULL;
	char *retMsg = NULL;
	char *sp = NULL;

	if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC)) {
		ls_syslog (LOG_DEBUG1, "%s: numReasons=%d", __func__, numReasons);
	}

	if (!numReasons || !rsTb)
	{
		lsberrno = LSBE_BAD_ARG;
		return "";
	}
	if (memSize < numReasons)
	{
		FREEUP (reasonTb);
		assert( numReasons >= 0);
		reasonTb = calloc ( numReasons, sizeof (int));
		if ( NULL == reasonTb && ENOMEM == errno )
		{
			memSize = 0;
			lsberrno = LSBE_NO_MEM;
			return "";
		}
		memSize = numReasons;
	}
	for ( unsigned int i = 0; i < numReasons; i++) {
		reasonTb[i] = rsTb[i];
	}

	FREEUP (hostList);
	FREEUP (retMsg);
	if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL)
	{
		assert( jInfoH->numHosts >= 0 );
		hostList = malloc( jInfoH->numHosts * MAXHOSTNAMELEN );
		retMsg   = malloc( jInfoH->numHosts * MAXHOSTNAMELEN + MSGSIZE);
		if ( ( NULL == hostList && ENOMEM == errno) || ( NULL == retMsg   && ENOMEM == errno ) )
		{
			lsberrno = LSBE_NO_MEM;
			return "";
		}
	}
	else
	{
		retMsg = malloc (MSGSIZE);
		if (retMsg == NULL)
		{
			lsberrno = LSBE_NO_MEM;
			return "";
		}
	}

	// retMsg[0] = '\0';
	memset( retMsg, 0, strlen(retMsg ) );
	for ( unsigned int i = 0; i < numReasons; i++)
	{
		if (!reasonTb[i]) {
			continue;
		}
		GET_LOW (reason, reasonTb[i]);
		if (!reason) {
			continue;
		}
		GET_HIGH (hostId, reasonTb[i]);
		if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC)) {
			ls_syslog (LOG_DEBUG2, "%s: hostId=%d, reason=%d reasonTb[%d]=%d", __func__, hostId, reason, i, reasonTb[i]);
		}

		if (!hostId) {
			sprintf (msgline, " %s;\n", getMsg (pendMsg, pendMsg_ID, reason));
			strcat (retMsg, msgline);
			continue;
		}

		if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL) {
			strcpy (hostList, jInfoH->hostNames[hostId - 1]);
		}
		else {
			num = 1;
		}

		for (unsigned int j = i + 1; j < numReasons; j++) {
			if (reasonTb[j] == 0) {
				continue;
			}
			GET_LOW (reasonJ, reasonTb[j]);
			if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC)) {
				ls_syslog (LOG_DEBUG2, "%s: reasonJ=%d reasonTb[j]=%d",	__func__, reasonJ, reasonTb[j]);
			}
			if (reasonJ != reason) {
				continue;
			}
			GET_HIGH (hostIdJ, reasonTb[j]);
			if (logclass & (LC_TRACE | LC_SCHED | LC_EXEC)) {
				ls_syslog (LOG_DEBUG2, "%s: j=%d, hostIdJ=%d", __func__, j, hostIdJ);
			}
			reasonTb[j] = 0;
			if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL) {
				sprintf (hostList, "%s, %s", hostList, jInfoH->hostNames[hostIdJ - 1]);
			}
			else
				num++;
		}

		if (reason >= PEND_HOST_LOAD && reason < PEND_HOST_QUEUE_RUSAGE) {
			getMsgByRes (reason - PEND_HOST_LOAD, PEND_HOST_LOAD, &sp, ld);
		}
		else if (reason >= PEND_HOST_QUEUE_RUSAGE && reason < PEND_HOST_JOB_RUSAGE) {
			getMsgByRes (reason - PEND_HOST_QUEUE_RUSAGE, PEND_HOST_QUEUE_RUSAGE, &sp, ld);

		}
		else if (reason >= PEND_HOST_JOB_RUSAGE) {
			getMsgByRes (reason - PEND_HOST_JOB_RUSAGE, PEND_HOST_JOB_RUSAGE, &sp, ld);

		}
		else {
			sp = getMsg (pendMsg, pendMsg_ID, reason);
		}

		if (jInfoH && jInfoH->numHosts != 0 && jInfoH->hostNames != NULL) {
			sprintf (retMsg, "%s %s: %s;\n", retMsg, sp, hostList);
		}
		else if (num == 1) {
			/* catgets 713 */
			sprintf (retMsg, "catgets 713: %s %s: 1 host;\n", retMsg, sp);
		}
		else {
			/* catgets 714 */
			sprintf (retMsg, "catgets 714: %s %s: %d hosts;\n", retMsg, sp, num);
		}

		return retMsg;
	}

	return NULL;
}


void userIndexReasons (char *msgline_, int resource, unsigned int reason, struct loadIndexLog *ld)
{

	if (ld == NULL || reason <= MEM || resource >= ld->nIdx)
	{
	  /* catgets 711 */
		sprintf (msgline_, "catgets 711: External load index is beyond threshold");
		return;
	}

	if (reason == PEND_HOST_LOAD)
	{
	  /* catgets 712 */
		sprintf (msgline_, "catgets 712: External load index (%s) is beyond threshold", ld->name[resource]);
	}
	else if (reason == PEND_HOST_QUEUE_RUSAGE)
	{
	  /* catgets 737 */
		sprintf (msgline_, "catgets 737: Queue requirements for reserving resource (%s) not satisfied", ld->name[resource]);
	}
	else if (reason == PEND_HOST_JOB_RUSAGE)
	{
	  /* catgets 738 */
		sprintf (msgline_, "catgets 738: Job's requirements for reserving resource (%s) not satisfied", ld->name[resource]);
	}

	return;

}

void getMsgByRes (int resource, unsigned int reason, char **sp, struct loadIndexLog *ld)
{
	switch (resource)
	{
		case R15S:

		if (reason == PEND_HOST_LOAD)
		{
	/* catgets 700 */
			sprintf (msgline, "catgets 700: The 15s effective CPU queue length (r15s) is beyond threshold");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 715 */
			sprintf (msgline, "catgets 715: Queue requirements for reserving resource (r15s) not satisfied ");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 716 */
			sprintf (msgline, "catgets 716: Job requirements for reserving resource (r15s) not satisfied");
		}
		break;

		case R1M:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 701 */
			sprintf (msgline, "catgets 701: The 1 min effective CPU queue length (r1m) is beyond threshold");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 717 */
			sprintf (msgline, "catgets 717: Queue requirements for reserving resource (r1m) not satisfied ");   
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 718 */
			sprintf (msgline, "catgets 718: Job requirements for reserving resource (r1m) not satisfied");
		}
		break;

		case R15M:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 702 */
			sprintf (msgline, "catgets 702: The 15 min effective CPU queue length (r15m) is beyond threshold");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 719 */
			sprintf (msgline, "catgets 719: Queue requirements for reserving resource (r15m) not satisfied ");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 720 */
			sprintf (msgline, "catgets 720: Job requirements for reserving resource (r15m) not satisfied");
		}
		break;

		case UT:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 703 */
			sprintf (msgline, "catgets 703: The CPU utilization (ut) is beyond threshold");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 721 */
			sprintf (msgline, "catgets 721: Queue requirements for reserving resource (ut) not satisfied ");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 722 */
			sprintf (msgline, "catgets 722: Job requirements for reserving resource (ut) not satisfied");
		}
		break;

		case PG:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 704 */
			sprintf (msgline, "catgets 704: The paging rate (pg) is beyond threshold");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 723 */
			sprintf (msgline, "catgets 723: Queue requirements for reserving resource (pg) not satisfied");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 724 */
			sprintf (msgline, "catgets 724: Job requirements for reserving resource (pg) not satisfied");
		}
		break;

		case IO:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 705 */
			sprintf (msgline, "catgets 705: The disk IO rate (io) is beyond threshold");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 725 */
			sprintf (msgline, "catgets 725: Queue requirements for reserving resource (io) not satisfied");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 726 */
			sprintf (msgline, "catgets 726: Job requirements for reserving resource (io) not satisfied");   
		}
		break;

		case LS:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 706 */
			sprintf (msgline,"catges 706: There are too many login users (ls)");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 727 */
			sprintf (msgline, "catgets 727: Queue requirements for reserving resource (ls) not satisfied");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 728 */
			sprintf (msgline, "catgets 728: Job requirements for reserving resource (ls) not satisfied");
		}
		break;

		case IT:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 707 */
			sprintf (msgline, "catges 707: The idle time (it) is not long enough");    
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 729 */
			sprintf (msgline, "catgets 729: Queue requirements for reserving resource (it) not satisfied"); 
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 730 */
			sprintf (msgline, "catgets 730: Job requirements for reserving resource (it) not satisfied");
		}
		break;

		case TMP:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 708 */
			sprintf (msgline, "catgets 708: The available /tmp space (tmp) is low");    
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 731 */
			sprintf (msgline, "catges 731: Queue requirements for reserving resource (tmp) not satisfied");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 732 */
			sprintf (msgline, "catgets 732: Job requirements for reserving resource (tmp) not satisfied");
		}
		break;

		case SWP:

		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 709 */
			sprintf (msgline, "catges 709: The available swap space (swp) is low");
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 733 */
			sprintf (msgline, "catges 733: Queue requirements for reserving resource (swp) not satisfied");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 734 */
			sprintf (msgline, "catgets 734: Job requirements for reserving resource (swp) not satisfied");
		}
		break;

		case MEM:
		if (reason == PEND_HOST_LOAD)
		{
	  /* catgets 710 */
			sprintf (msgline, "catgets 710: The available memory (mem) is low");    
		}
		else if (reason == PEND_HOST_QUEUE_RUSAGE)
		{
	  /* catgets 735 */
			sprintf (msgline, "catgets 735: Queue requirements for reserving resource (mem) not satisfied");
		}
		else if (reason == PEND_HOST_JOB_RUSAGE)
		{
	  /* catgets 736 */
			sprintf (msgline, "catgets 736: Job requirements for reserving resource (mem) not satisfied"); 
		}
		break;

		default:
		userIndexReasons (msgline, resource, reason, ld);
		break;
	}

	*sp = msgline;

	return;
}
