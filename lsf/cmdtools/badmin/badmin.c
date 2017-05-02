/* $Id: badmin.c 397 2007-11-26 19:04:00Z mblack $
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
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "lib/common_structs.h"
#include "lib/debug.h"
#include "cmdtools/badmin/badmin.h"
#include "cmdtools/lsadmin/lsadmin.h"

// char *myGetOpt (int nargc, char **nargv, char *ostr);
// int checkConf (int, int);
// int getConfirm (char *msg);
// int lsb_debugReq (struct debugReq *pdebug, char *host);
int linux_optind = 0;
int linux_opterr = 0;
static int doBatchCmd (int argc, char *argv[]);
static int badminDebug (int nargc, char *nargv[], int opCode);

// #define NL_SETN 8
static int cmdInfo_ID[] = {
	3101, 3102, 3103, 3104, 3105, 3106, 3107, 3108, 3109, 3110,
	3111, 3112, 3113, 3114, 3115, 3116, 3117, 3122,
	3118, 3119, 3120, 3120, 3121
};


int
main (int argc, char **argv)
{
	int cc = 0; 
	int myIndex = 0;
	const char prompt[] = "badmin> ";
	static char line[MAXLINELEN];
	// int rc = 0;

	// rc = 
	_i18n_init (I18N_CAT_MIN);

	if (lsb_init (argv[0]) < 0)
	{
		lsb_perror ("lsb_init");
		// _i18n_end (ls_catd);
		_i18n_end ( );
		exit (-1);
	}

	while ((cc = getopt (argc, argv, "Vh")) != EOF)
	{
		switch (cc)
		{
			case 'V':
	  			fputs (_LS_VERSION_, stderr);
				exit (0);
				break;
			case 'h':
			;
			default:
			;
			cmdsUsage ("badmin", cmdList, _i18n_msgArray_get (ls_catd, NL_SETN, cmdInfo_ID, cmdInfo));
			break;
		}
	}

	if (argc > optind) {
		int rc;

		if ((myIndex = adminCmdIndex (argv[optind], cmdList)) == -1) {
			//	fprintf (stderr, (_i18n_msg_get (ls_catd, NL_SETN, 2552, "Invalid command <%s> \n")), argv[optind]);  /* catgets  2552  */
			/* catgets  2552  */
			fprintf (stderr, "catgets 2552: Invalid command <%s> \n", argv[optind] );
			cmdsUsage ("badmin", cmdList, _i18n_msgArray_get (ls_catd, NL_SETN, cmdInfo_ID, cmdInfo));
		}

		optind++;
		rc = doBatchCmd (argc, argv);
		// _i18n_end (ls_catd);
		_i18n_end( );
		exit (rc);
	}

	for (;;) {
		printf ("%s", prompt);
		fflush (stdout);

		if (fgets (line, MAXLINELEN, stdin) == NULL) {
			printf ("\n");
			// _i18n_end (ls_catd);
			_i18n_end( );
			exit (-1);
		}
 		parseAndDo (line, doBatchCmd);
	}

	return 0;
}

static int
doBatchCmd (int argc, char *argv[])
{

	int cmdRet = 0;
	int myIndex = 0;

 	if ((myIndex = adminCmdIndex (argv[optind - 1], cmdList)) == -1) {
		/* catgets  2554  */
		// fprintf (stderr, _i18n_msg_get (ls_catd, NL_SETN, 2554, "Invalid command <%s>. Try help\n"), argv[optind - 1]);
		fprintf (stderr, "catgs 2554: Invalid command <%s>. Try help\n", argv[optind - 1] );
		return -1;
	}

	switch (myIndex) {
		case BADMIN_MBDRESTART:
			cmdRet = breconfig (argc, argv, MBD_RESTART);
		break;
		case BADMIN_RECONFIG:
			cmdRet = breconfig (argc, argv, MBD_RECONFIG);
		break;
		case BADMIN_CKCONFIG:
			cmdRet = breconfig (argc, argv, MBD_CKCONFIG);
		break;
		case BADMIN_QOPEN:
		case BADMIN_QCLOSE:
		case BADMIN_QACT:
		case BADMIN_QINACT:
			cmdRet = bqc (argc, argv, opCodeList[myIndex]);
		break;
		case BADMIN_HOPEN:
		case BADMIN_HCLOSE:
		case BADMIN_HREBOOT:
		case BADMIN_HSHUT:
			cmdRet = bhc (argc, argv, opCodeList[myIndex]);
		break;
		case BADMIN_HSTARTUP:
			startup (argc, argv, myIndex);
		break;
		case BADMIN_QHIST:
		case BADMIN_HHIST:
		case BADMIN_MBDHIST:
		case BADMIN_HIST:
			cmdRet = sysHist (argc, argv, opCodeList[myIndex]);
		break;
		case BADMIN_MBDDEBUG:
		case BADMIN_MBDTIME:
		case BADMIN_SBDDEBUG:
		case BADMIN_SBDTIME:
			cmdRet = badminDebug (argc, argv, opCodeList[myIndex]);
		break;

		case BADMIN_HELP:
		case BADMIN_QES:
			cmdHelp (argc, argv, cmdList, _i18n_msgArray_get (ls_catd, NL_SETN, cmdInfo_ID, cmdInfo), cmdSyntax);
		break;
		case BADMIN_QUIT:
			exit (0);
		default:
			// fprintf (stderr, I18N_FUNC_S_ERROR, "adminCmdIndex()");
			fprintf (stderr, "Failure at adminCmdIndex()");
			exit (-1);
		break;
	}

	if (cmdRet == -2) {
		oneCmdUsage (myIndex, cmdList, cmdSyntax);
	}

	fflush (stderr);
	return cmdRet;

}

int
breconfig (int argc, char **argv, int configFlag)
{
	char *optName;
	int vFlag = 0;
	int fFlag = 0;
	int checkReply = 0;
	int stdoutsave = 0;
	int fd = 0;
	FILE *fp = NULL;
	char *linep = NULL;
	char tmpfile[256];
	char *tmpname = "breconfig_XXXXXX";

	while ((optName = myGetOpt (argc, argv, "f|v|")) != NULL) {
		switch (optName[0]) {
			case 'v':
				vFlag = 1;
			break;
			case 'f':
				fFlag = 1;
			break;
			default:
				return -2;
			break;
		}	
	}

	if (optind < argc) {
		return -2;
	}

	if (!vFlag && !fFlag) {

		fprintf (stderr, "\nChecking configuration files ...\n\n");

		stdoutsave = dup (1);
		// sprintf (tmpfile, "/tmp/%s", tmpname);
		fd = mkstemp (tmpname);
		// fd = open (tmpfile, O_RDWR | O_CREAT | O_TRUNC, 0666); // FIXME FIXME FIXME 666 WTF
		if (fd > 0) {

			dup2 (fd, 1);
			dup2 (fd, 2);
			checkReply = checkConf (1, 2);
			fflush (stderr);
			close (fd);
			dup2 (stdoutsave, 1);
			dup2 (stdoutsave, 2);
			fp = fopen (tmpfile, "r");
			if (fp != 0) {
				if (checkReply == EXIT_FATAL_ERROR || checkReply == EXIT_WARNING_ERROR) {
					if (checkReply == EXIT_FATAL_ERROR) {
						fprintf (stderr, "There are fatal errors.\n\n");
					}
					else {
						fprintf (stderr, "There are warning errors.\n\n");
					}

		  			fflush (stderr);

					if (getConfirm ((_i18n_msg_get (ls_catd, NL_SETN, 2563, "Do you want to see detailed messages? [y/n] "))))    /* catgets  2563  */
						while ((linep = getNextLine_ (fp, 0))) {
							fprintf (stderr, "%s\n", linep);
						}
					}
					else {
						// fprintf (stderr, I18N (2586, "No errors found.\n\n"));  /* catgets 2586 */
						/* catgets 2586 */
						fprintf (stderr, "catgets 2586: No errors found.\n\n");
					}
				fflush (stderr);
			}
			fclose (fp);
			unlink (tmpfile);
		}
		else  {
			checkReply = checkConf (0, 2);
		}
	}
	else {
		checkReply = checkConf (vFlag, 2);
	}

	if (configFlag == MBD_CKCONFIG) {
		return 0;
	}

	switch (checkReply) {
		case EXIT_FATAL_ERROR:
			return -1;
		break;
		case EXIT_WARNING_ERROR:
			if (fFlag) {
				break;
			}

			if (configFlag == MBD_RECONFIG) {

				/* catgets  2564  */
				if (!getConfirm((_i18n_msg_get(ls_catd, NL_SETN, 2564, "\nDo you want to reconfigure? [y/n] ")))) {
					// fprintf (stderr, (_i18n_msg_get (ls_catd, NL_SETN, 2565, "Reconfiguration aborted.\n"))); /* catgets  2565  */
					/* catgets  2565  */
					fprintf (stderr, "catgets 2565: Reconfiguration aborted.\n");
					return -1;
				}
			}
			else {
				/* catgets  2570  */
				if (!getConfirm(I18N (2570, "\nDo you want to restart MBD? [y/n] "))) {
					/* catgets  2571  */
					// fprintf (stderr, (I18N (2571, "MBD restart aborted.\n")));
					fprintf (stderr, "catgets 2571: MBD restart aborted.\n" );
					return -1;
				}
			}
		break;
		default:
			;
		break;
	}

	if (lsb_reconfig (configFlag) < 0) {
		lsb_perror ((_i18n_msg_get (ls_catd, NL_SETN, 2566, "Failed")));
		return -1;
	}

	if (configFlag == MBD_RECONFIG) {
		printf ("%s\n", _i18n_msg_get (ls_catd, NL_SETN, 2567, "Reconfiguration initiated")); /* catgets  2567  */
	}
	else {
		/* catgets  2569  */
		printf ("%s\n", I18N (2569, "MBD restart initiated"));
	}

	return 0;
}



static int
badminDebug (int nargc, char *nargv[], int opCode)
{

	struct hostInfoEnt *hostInfo;

	char opt[10]; // FIXME FIXME FIXME FIXME awfully particular
	char **hostPoint = NULL;
	char *word = NULL;

	char **hosts = NULL;
	int c = 0;
	int send = 0;
	int retCode = 0;
	int all = FALSE;
	unsigned int numHosts = 0;
	struct debugReq debug = {
		opCode, 0, 0, 0, NULL, 0 };

	// debug.opCode = opCode;
	// debug.logClass = 0;
	// debug.level = 0;
	// debug.hostName = NULL;

	// debug.logFileName[0] = '\0';
	// debug.options = 0;

	if (opCode == MBD_DEBUG || opCode == SBD_DEBUG) {
		strcpy (opt, "oc:l:f:");
	}
	else if (opCode == MBD_TIMING || opCode == SBD_TIMING) {
		strcpy (opt, "ol:f:");
	}
	else {
		return -2;
	}

	linux_optind = 1;
	linux_opterr = 1;

	if( strstr (nargv[0], "badmin" ) ) {
	  linux_optind++;
	}

	while ((c = getopt (nargc, nargv, opt)) != EOF) {

		switch (c) {
			case 'c':
				while (optarg != NULL && (word = getNextWord_ (&optarg))) {
					if (strcmp (word, "LC_SCHED") == 0)
						debug.logClass |= LC_SCHED;

					if (strcmp (word, "LC_EXEC") == 0)
						debug.logClass |= LC_EXEC;

					if (strcmp (word, "LC_TRACE") == 0)
						debug.logClass |= LC_TRACE;

					if (strcmp (word, "LC_COMM") == 0)
						debug.logClass |= LC_COMM;

					if (strcmp (word, "LC_XDR") == 0)
						debug.logClass |= LC_XDR;

					if (strcmp (word, "LC_CHKPNT") == 0)
						debug.logClass |= LC_CHKPNT;

					if (strcmp (word, "LC_FILE") == 0)
						debug.logClass |= LC_FILE;

					if (strcmp (word, "LC_AUTH") == 0)
						debug.logClass |= LC_AUTH;

					if (strcmp (word, "LC_HANG") == 0)
						debug.logClass |= LC_HANG;

					if (strcmp (word, "LC_SIGNAL") == 0)
						debug.logClass |= LC_SIGNAL;

					if (strcmp (word, "LC_PIM") == 0)
						debug.logClass |= LC_PIM;

					if (strcmp (word, "LC_SYS") == 0)
						debug.logClass |= LC_SYS;

					if (strcmp (word, "LC_JLIMIT") == 0)
						debug.logClass |= LC_JLIMIT;

					if (strcmp (word, "LC_PEND") == 0)
						debug.logClass |= LC_PEND;

					if (strcmp (word, "LC_LOADINDX") == 0)
						debug.logClass |= LC_LOADINDX;

					if (strcmp (word, "LC_M_LOG") == 0) {
						debug.logClass |= LC_M_LOG;
					}

					if (strcmp (word, "LC_PERFM") == 0) {
						debug.logClass |= LC_PERFM;
					}

					if (strcmp (word, "LC_MPI") == 0) {
						debug.logClass |= LC_MPI;
					}

					if (strcmp (word, "LC_JGRP") == 0) {
						debug.logClass |= LC_JGRP;
					}

				}
				if (debug.logClass == 0) {
					/* catgets 2572 */
					// fprintf (stderr, I18N (2572, "Command denied. Invalid class name\n"));
					fprintf (stderr, "catgets 2572: Command denied. Invalid logging class name; valid are:\n",
							"LC_SCHED, LC_EXEC, LC_TRACE, LC_COMM, LC_XDR, LC_CHKPNT, LC_FILE, LC_AUTH,\n"
							"LC_HANG, LC_SIGNAL, LC_PIM, LC_SYS, LC_JLIMIT, LC_PEND, LC_LOADINDX, LC_M_LOG\n",
							"LC_PERFM, LC_MPI and LC_JGRP\n" );
					return -1;
				}
			break;

			case 'l':
				for ( unsigned int i = 0; i < strlen (optarg); i++) {
					if (!isdigit (optarg[i])) {
						// fprintf (stderr, I18N (2573, "Command denied. Invalid level value\n"));   /* catgets 2573 */
						/* catgets 2573 */
						fprintf (stderr, "catgets 2573: Command denied. Invalid level value\n");
						return -1;
					}
				}
				debug.level = atoi (optarg);
				if (opCode == MBD_DEBUG || opCode == SBD_DEBUG) {
					if (debug.level < 0 || debug.level > 3) {
						// fprintf(stderr, I18N (2574, "Command denied. Valid debug level is [0-3] \n"));   /* catgets 2574 */
						/* catgets 2574 */
						fprintf(stderr, "catgets 2574: Command denied. Valid debug level is [0-3] \n" );
						return -1;
					}
				}
				else if (debug.level < 1 || debug.level > 5) {
					// fprintf (stderr, I18N (2575, "Command denied. Valid timing level is [1-5]\n"));   /* catgets 2575 */
					/* catgets 2575 */
					fprintf (stderr, "catgets 2575: Command denied. Valid timing level is [1-5]\n");
					return -1;
				}
				break;

			case 'f':
				if (strstr (optarg, "/") && strstr (optarg, "\\")) {
					/*  catgets 2576 */
					// fprintf(stderr, I18N (2576, "Command denied. Invalid file name\n"));
					fprintf(stderr, "catgets 2576: Command denied. Invalid file name\n" );
					return -1;
				}
				memset (debug.logFileName, 0, strlen (debug.logFileName));
				ls_strcat (debug.logFileName, strlen (debug.logFileName), optarg);
				if (debug.logFileName[strlen (debug.logFileName) - 1] == '/' || debug.logFileName[strlen (debug.logFileName) - 1] == '\\') {
					/*  catgets 2577 */
					// fprintf (stderr, I18N (2577, "Command denied. File name is needed after the path\n"));
					fprintf (stderr, "catgets 2577: Command denied. File name is needed after the path\n" );
					return -1;
				}
			break;
			case 'o':
				debug.options = 1;
			break;

			default:
				return -2;
			break;
		}
	}


	if (opCode == SBD_DEBUG || opCode == SBD_TIMING) {

		numHosts = getNames (nargc, nargv, optind, &hosts, &all, "hostC");
		hostPoint = NULL;
		if (!numHosts && !all) {
			numHosts = 1;
		}
		else if (numHosts) {
			hostPoint = hosts;
		}

		if ((hostInfo = lsb_hostinfo (hostPoint, &numHosts)) == NULL) {
			lsb_perror (NULL);
			return -1;
		}

		for ( unsigned int i = 0; i < numHosts; i++) {
			if (strcmp (hostInfo[i].host, "lost_and_found") == 0) {
				if (!all) {
				/* catgets  2568  */
					fprintf (stderr, "%s.\n", _i18n_msg_get (ls_catd, NL_SETN, 2568, "<lost_and_found> is not a real host, ignored"));
				}
		  		continue;
			}

			fflush (stderr);
			if (hostInfo[i].hStatus & (HOST_STAT_UNAVAIL | HOST_STAT_UNREACH))
			{
				if (hostInfo[i].hStatus & HOST_STAT_UNAVAIL) {
					/* catgets 2578 */
					// fprintf (stderr, I18N (2578, "failed : LSF daemon (LIM) is unavailable on host %s\n"), hostInfo[i].host);
					fprintf (stderr, "catgets 2578: failed : LSF daemon (LIM) is unavailable on host %s\n", hostInfo[i].host);
				}
				else {
					/* catgets 2579 */
					// fprintf (stderr, I18N (2579, "failed : Slave batch daemon (sbatchd) is unreachable now on host %s\n"), hostInfo[i].host);
					fprintf (stderr, "catgets 2579: failed : Slave batch daemon (sbatchd) is unreachable now on host %s\n", hostInfo[i].host);
				}
				continue;
			}

			if ((send = lsb_debugReq (&debug, hostInfo[i].host)) < 0) {
				char msg[100];  // FIXME FIXME FIXME awfully specific
				/* catgets 2580 */
				// sprintf (msg, I18N (2580, "Operation denied by SBD on <%s>"), hostInfo[i].host);
				sprintf (msg, "catgets 2580: Operation denied by SBD on <%s>", hostInfo[i].host);
				lsb_perror (msg);
				retCode = -1;
			}
		}
		return retCode;
	}
	else {
		numHosts = getNames (nargc, nargv, optind, &hosts, &all, "hostC");
		if (numHosts > 0) {
			/* catgets 2581 */
			// fprintf (stderr, I18N (2581, "Host name does not need to be specified, set debug to the host which runs MBD\n"));
			fprintf (stderr, "catgets 2581: Host name does not need to be specified, set debug to the host which runs MBD\n");
		}
		if ((send = lsb_debugReq (&debug, NULL)) < 0) {

			char msg[100];
			/* catgets 2582 */
			sprintf (msg, "catgets 2582: Operation denied by MBD"); // FIXME FIXME probably needs a \n at the end of the string here
			lsb_perror (msg);
			return -1;
		}
	}

	return 0;
}
