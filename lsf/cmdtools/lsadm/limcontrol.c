/* $Id: cmd.limcontrol.c 397 2007-11-26 19:04:00Z mblack $
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

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#include "daemons/liblimd/limcontrol.h"
#include "libint/admin.h"
#include "libint/intlibout.h"
#include "libint/lsi18n.h"
#include "lsf.h"



int limCtrl (int argc, char **argv, int opCode)
{
	char *optName   = NULL;
	char *localHost = NULL;
	int vFlag       = 0;
	int config      = 0;
	int checkReply  = 0;

	if (strcmp (argv[optind - 1], "reconfig") == 0) {
		config = 1;
	}

	while ((optName = myGetOpt (argc, argv, "f|v|")) != NULL) { // FIXME FIXME FIXME FIXME replace with gnu getopt()
		switch (optName[0]) {
			case 'v':
				if (opCode == LIM_CMD_SHUTDOWN) {
					return -2;
				}
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
	exitrc = 0;
	if (config && optind != argc) {
		return -2;
	}

	switch (checkReply)
	{
		case EXIT_FATAL_ERROR:
			return -1;
		break;
		case EXIT_WARNING_ERROR:
			if (fFlag) {
				break;
			}
			/* catgets 250 */
			if (!getConfirm (I18N (250, "Do you want to reconfigure? [y/n] "))) {
				/* catgets 251 */
				fprintf (stderr, "%s", I18N (251, "Reconfiguration aborted.\n"));
				return -1;
			}
		break;
		default:
			;
		break;
	}

	if (config) {
		doAllHosts (opCode);
		return exitrc;
	}

	if (optind == argc) {
		if ((localHost = ls_getmyhostname ()) == NULL) {
			ls_perror ("ls_getmyhostname");
			return -1;
		}
		operateHost (localHost, opCode, 0);
	}
	else {
		doHosts (argc, argv, opCode);
	}

	return exitrc;
}

void doHosts (int argc, char **argv, int opCode)
{
	if (optind == argc - 1 && strcmp (argv[optind], "all") == 0) {
		doAllHosts (opCode);
		return;
	}

	for (; optind < argc; optind++) {
		operateHost (argv[optind], opCode, 0);
	}

	return;
}

void doAllHosts (int opCode)
{
	size_t numHosts = 0;
	struct hostInfo *hostinfo = NULL;
	int ask = FALSE;
	int try = FALSE;
	char msg[100];

	memset( msg, 0, strlen( msg ) );

	hostinfo = ls_gethostinfo ("-:server", &numHosts, NULL, 0, LOCAL_ONLY);
	if (hostinfo == NULL) {
		ls_perror ("ls_gethostinfo");
		/* catgets 252 */
		fprintf (stderr, "%s", I18N (252, "Operation aborted\n"));
		exitrc = -1;
		return;
	}

	if (!fFlag) {

		if (opCode == LIM_CMD_REBOOT) {
			/* catgets 253 */
			sprintf (msg, "%s", I18N (253, "Do you really want to restart LIMs on all hosts? [y/n] "));
		}
		else {
			/* catgets 254 */
			sprintf (msg, "%s", I18N (254, "Do you really want to shut down LIMs on all hosts? [y/n] "));
		}

		ask = (!getConfirm (msg));
	}

	for ( unsigned int i = 0; i < numHosts; i++) {
		if (hostinfo[i].maxCpus > 0) {
			operateHost (hostinfo[i].hostName, opCode, ask);
		}
		else {
			try = 1;
		}
	}
	
	if( try ) {
		fprintf (stderr, "\n%s :\n\n", I18N (255, "Trying unavailable hosts")); /* catgets 255 */
		for ( unsigned int i = 0; i < numHosts; i++) {
			if (hostinfo[i].maxCpus <= 0) {
				operateHost (hostinfo[i].hostName, opCode, ask);
			}
		}
	}

	return;
}

void operateHost (char *host, int opCode, int confirm)
{
	char msg1[MAXLINELEN];
	char msg[MAXLINELEN];

	memset( msg1, 0, strlen( msg1 ) );
	memset( msg, 0 , strlen( msg  ) );
	if( !host || strcmp( host, "" ) ) {
		exitrc = -2;
		fprintf( stderr, "Hostname entered %s NULL or empty.", __func__ );
		return;
	}

	if (opCode == LIM_CMD_REBOOT) {
		/* catgets 256 */
		sprintf (msg1, "%s", I18N (256, "Restart LIM on <%s>"), host);
	}
	else {
		/* catgets 257 */
		sprintf (msg1, "%s", I18N (257, "Shut down LIM on <%s>"), host);
	}

	if (confirm) {
		sprintf (msg, "%s ? [y/n] ", msg1);
		if (!getConfirm (msg))
				return;
	}

	fprintf (stderr, "%s ...... ", msg1);
	fflush (stderr);
	if (ls_limcontrol (host, opCode) == -1) {
		ls_perror ("ls_limcontrol");
		exitrc = -1;
	}
	else {
		char *delay = getenv ("LSF_RESTART_DELAY"); // FIXME FIXME FIXME FIXME remove environmental variable
		unsigned int delay_time = 0;
		if ( NULL == delay ) {
			delay_time = 500; // FIXME FIXME FIXME possible cause of delay for all LIM nodes
		}
		else {
			delay_time = atoi (delay) * 1000;  // FIXME FIXME FIXME possible cause of delay for all LIM nodes
		}

		millisleep_ (delay_time);
		fprintf (stderr, "%s\n", I18N_done);
	}
	
	fflush (stderr);
	return;
}

int limLock (int argc, char **argv)
{
	size_t duration = 0;
	char *optName = NULL;

	while ((optName = myGetOpt (argc, argv, "l:")) != NULL) {
		switch (optName[0]) {
			case 'l':
				duration = atoi (optarg);
				if (!isint_ (optarg) || atoi (optarg) <= 0)
				{
					/* catgets 258 */
					fprintf (stderr, "%s", I18N (258, "The host locking duration <%s> should be a positive integer\n"), optarg);
					return -2;
				}
			break;
			default:
				return -2;
			break;
		}
	}

	if (argc > optind) {
		return -2;
	}

	if (ls_lockhost (duration) < 0) {
		char *ls_perrroBuffer = malloc( 100 ); // FIXME FIXME FIXME 100 chars is rather random, yes, but I put that in
		sprintf( ls_perrroBuffer, "%s", "%s: ls_lockhost failed", __func__  );
		ls_perror( ls_perrroBuffer );
		free( ls_perrroBuffer ); 
		return -1;
	}

	if( duration ) {
		/* catgets 259 */
		fprintf( stdout, "%s", I18N (259, "Host is locked for %lu seconds\n") , duration);
	}
	else {
		/* catgets 260 */
		fprintf( stdout, "%s", I18N (260, "Host is locked\n"));
	}

	fflush (stdout);
	return 0;
}

int
limUnlock (int argc, char **argv)
{

	assert( argv );

	if (argc > optind)
	{
		/* catgets 261 */
		fprintf (stderr, "%s", I18N (261, "Syntax error: too many arguments.\n"));
		return -2;
	}

	if( ls_unlockhost () < 0) {
		char *ls_perrroBuffer = malloc( 100 ); // FIXME FIXME FIXME 100 chars is rather random, yes, but I put that in
		sprintf( ls_perrroBuffer, "%s", "%s: ls_unlockhost failed", __func__  );
		ls_perror( ls_perrroBuffer );
		free( ls_perrroBuffer ); 
		return -1;
	}

 	/* catgets 262 */
	fprintf ( stdout, "%s", I18N (262, "Host is unlocked\n"));
	fflush (stdout);

	return 0;
}
