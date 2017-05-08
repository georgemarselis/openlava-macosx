/* $Id: admin.c 397 2007-11-26 19:04:00Z mblack $
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


#include "libint/intlibout.h"
#include "libint/lsi18n.h"
#include "lib/initenv.h"
#include "lib/init.h"
#include "lib/lib.h"
#include "lib/mls.h"


#define BADCH   ":"

// extern int errLineNum_;

static int changeUserEUId (void);


void
parseAndDo (char *cmdBuf, int (*func)() ) // FIXME FIXME FIXME replace with parser
{

	const unsigned int MAX_ARG = 100;

	unsigned int i = 0;
	int argc = 0;
	char *argv[MAX_ARG];

	int see_string = 0;

	for (i = 0; i < MAX_ARG; i++)
	{

		while( isspace( *cmdBuf ) && !see_string ) {
			cmdBuf++;
		}

		if (*cmdBuf == '"') {
			cmdBuf++;
			see_string = 1;
		}

		if (*cmdBuf == '\0') {
			break;
		}

		argv[i] = cmdBuf;

		while( *cmdBuf != '\0' && !isspace( *cmdBuf ) && *cmdBuf != '"' )
		{
			cmdBuf++;
		}

		while (see_string && *cmdBuf != '"')
		{
			cmdBuf++;
			if (*cmdBuf == '\0')
			{
				see_string = 0;
				/* catgets 100 */
				ls_perror( "100: Syntax Error of line parameter! \n" );
				exit (-1);
			}

		}

		if (see_string)  {
			see_string = 0;
		}

		if( *cmdBuf != '\0' ) {
			*cmdBuf = '\0';
			cmdBuf++;
		}
	}

	if (i == 0) {
		return;
	}

	argv[i] = NULL;
	argc = i;
	optind = 1;

	(void) (*func) (argc, argv);

	return;
}

int
adminCmdIndex (char *cmd, char *cmdList[])
{
	static char quit[] = "quit";

	if (strcmp ("q", cmd) == 0) {
		cmd = quit;
	}

	for( unsigned int i = 0; NULL != cmdList[ i ]; i++ ) {
		if( strcmp( cmdList[ i ], cmd ) == 0 ) {
			return i;
		}
	}

	return -1;
}

void
cmdsUsage (char *cmd, char *cmdList[], char *cmdInfo[])
{

	static char intCmds[] = " ";
//	int i;

	fprintf (stderr, "\n");
	fprintf (stderr, "5000 Usage" );
	fprintf (stderr, ": %s [-h] [-V] [command] [command_options] [command_args]\n\n", cmd);
	/* catgets 102 */
	fprintf( stderr, ",     where 'command' is:\n\n");

	for ( unsigned int i = 0; cmdList[i] != NULL; i++) {
		if (strstr (intCmds, cmdList[i]) == NULL) {
			fprintf (stderr, "    %-12.12s%s\n", cmdList[i], cmdInfo[i]);
		}
	}

	exit (-1); // FIXME FIXME : POSIX demands positive exit statusess
	return;
}


void
oneCmdUsage (int i, char *cmdList[], char *cmdSyntax[])
{
	fprintf (stderr, "5000 Usage");
	fprintf (stderr, ":    %-12.12s%s\n", cmdList[i], cmdSyntax[i]);
	fflush (stderr);

	return;
}


void
cmdHelp (int argc, char **argv, char *cmdList[], char *cmdInfo[],  char *cmdSyntax[])
{

	static char intCmds[] = " ";

	if (argc <= optind)
	{

		/* catgets 104  */
		fprintf (stderr, "\n%s\n\n", "104: Commands are : ");

		for ( unsigned int i = 0, j = 0; cmdList[i] != NULL; i++)
		{

			if (strstr (intCmds, cmdList[i]) == NULL)
			{
				j++;
				fprintf (stderr, "%-12.12s", cmdList[i]);
				if (j % 6 == 0) {
					fprintf (stderr, "\n");
				}
			}
		}

		/* catgets 105 */
		fprintf (stderr, "\n\n%s\n\n", "105: Try help command... to get details. ");
		fflush (stderr);
		return;
	}

	for (; argc > optind; optind++) {
		int i = 0;
		if ((i = adminCmdIndex (argv[optind], cmdList)) != -1)
		{
			oneCmdUsage (i, cmdList, cmdSyntax);
			/* catgets 106 */
			fprintf (stderr, "106: Function: %s\n\n", cmdInfo[i]);
		}
		else {
			/* catgets 107 */
			fprintf (stderr, "107: Invalid command <%s>\n\n", argv[optind]);
		}
	}

	fflush (stderr);

	return;
}

char *
myGetOpt (int nargc, char **nargv, char *ostr)
{
	// char svstr[256];
	char *svstr   = malloc( sizeof( char ) * 256 + 1 ); // FIXME FIXME FIXME FIXME throw at debugger, might need larger allocation
	char *cp1     = svstr;
	char *cp2     = svstr;
	char *optName = NULL;
	int num_arg   = 0;

	if ((optName = nargv[optind]) == NULL) {
		return NULL;
	}
	if (optind >= nargc || *optName != '-') {
		return NULL;
	}
	if (optName[1] && *++optName == '-')
	{
		++optind;
		return NULL;
	}
	if (ostr == NULL) {
		return NULL;
	}
	strcpy (svstr, ostr);
	num_arg = 0;
	optarg  = NULL;

	while (*cp2)  // FIXME FIXME FIXME FIXME FIXME  replace with getopts()
	{
		unsigned int cp2len = strlen (cp2);
		unsigned int i = 0;
		for ( i = 0; i < cp2len; i++)
		{
			if (cp2[i] == '|')
			{
				num_arg = 0;
				cp2[i] = '\0';
				break;
			}
			else if (cp2[i] == ':')
			{
				num_arg = 1;
				cp2[i] = '\0';
				break;
			}
		}
		if (i >= cp2len) {
			return BADCH;
		}

		if (!strcmp (optName, cp1))
		{
			if (num_arg)
			{
				if (nargc <= optind + 1)
				{
					/* catgets 108 */
					fprintf (stderr, "108: %s: option requires an argument -- %s\n", nargv[0], optName);
					return BADCH;
				}
				optarg = nargv[++optind];
			}
			++optind;
			return optName;
		}
		cp1 = &cp2[i];
		cp2 = ++cp1;
	}

	/* catgets 109 */
	fprintf (stderr, "109: %s: illegal option -- %s\n", nargv[0], optName );
	return BADCH;
}

int
getConfirm (char *msg)
{
	char answer[MAXLINELEN]; // convert to dynamic allocation
	int i = 0;

	while (1)
	{
		fputs (msg, stdout);
		fflush (stdout);
		if (fgets (answer, MAXLINELEN, stdin) == NULL)
		{
			return FALSE;
		}

		i = 0;
		while (answer[i] == ' ') {
			i++;
		}

		if ((answer[i] == 'y' || answer[i] == 'n' || answer[i] == 'Y' || answer[i] == 'N') && answer[i + 1] == '\n') {
			break;
		}
	}

	return (answer[i] == 'Y' || answer[i] == 'y');
}

int
checkConf (int verbose, int who)
{
	char confCheckBuf[] = "RECONFIG_CHECK=TRUE";
	struct config_param *plp = NULL;
	char *lsfEnvDir  = NULL;
	char *daemon     = NULL;
	LS_WAIT_T status = 0;
	int fatalErr = FALSE;
	pid_t pid    = 0;
	int cc       = 0;
	int fd       = 0;

	if (lsfParams[LSF_ENVDIR].paramValue == NULL)
	{
		lsfEnvDir = getenv ("LSF_ENVDIR");
		cc = initenv_ (lsfParams, lsfEnvDir);
	}

	if (cc < 0)
	{
		if (lserrno == LSE_CONF_SYNTAX)
		{
			char lno[20];   // FIXME FIXME FIXME FIXME convert to dynamic allocation
			/* catgets 110 */
			sprintf (lno, "110: Line %ld", errLineNum_);
			ls_perror (lno);
		}
		else {
			ls_perror ("initenv_");
		}
	}
	plp = lsfParams;
	for (; plp->paramName != NULL; plp++) 
	{
		if (plp->paramValue == NULL)
		{
			/* catgets 111 */
			fprintf (stderr, "111: %s is missing or has a syntax error in lsf.conf file\n", plp->paramName);
			fatalErr = TRUE;
		}
	}

	if (fatalErr) {
		return EXIT_FATAL_ERROR;
	}
	if (cc < 0) {
		return EXIT_WARNING_ERROR;
	}

	if ((daemon = calloc (strlen (lsfParams[LSF_ENVDIR].paramValue) + 15, sizeof (char))) == NULL)
	{
		perror ("calloc");
		return EXIT_FATAL_ERROR;
	}

	strcpy (daemon, lsfParams[LSF_ENVDIR].paramValue);
	strcat (daemon, ((who == 1) ? "/lim" : "/mbatchd")); // FIXME FIXME FIXME FIXME replace fixed strings

	if (access (daemon, X_OK) < 0)
	{
		perror (daemon);
		free (daemon);
		return EXIT_FATAL_ERROR;
	}

	if (putenv (confCheckBuf))
	{
		/* catgets 112 */
		fprintf (stderr, "112: Failed to set environment variable RECONFIG_CHECK\n");
		free (daemon);
		return EXIT_FATAL_ERROR;
	}


	if ((pid = fork ()) < 0)
	{
		perror ("fork");
		free (daemon);
		return EXIT_FATAL_ERROR;
	}

	if (pid == 0)
	{
		if (!verbose)
		{
			fd = open (LSDEVNULL, O_RDWR);
			dup2 (fd, 1);
			dup2 (fd, 2);
		}

		if (changeUserEUId () < 0)
		{
			exit (EXIT_FATAL_ERROR);
		}

		execlp (daemon, daemon, "-C", (char *) 0);
		perror ("execlp");

		exit (EXIT_RUN_ERROR);
	}


	free (daemon);
	/* catgets 115 */
	fprintf (stderr, "\n115: Checking configuration files ...\n") ;

	if (waitpid (pid, &status, 0) < 0)
	{
		perror ("waitpid");
		return EXIT_FATAL_ERROR;
	}

	if( WIFEXITED( status ) != 0 && WEXITSTATUS( status ) != 0xf8 ) {
		if (verbose) {
			fprintf (stderr, "---------------------------------------------------------\n");
		}
	}


	if (WIFEXITED (status) == 0)
	{
		/* catgets 116 */
		fprintf( stderr, "116: Child process killed by signal.\n\n" );
		return EXIT_FATAL_ERROR;
	}

	switch (WEXITSTATUS (status)) // FIXME FIXME FIXMEFIXME must break and set the appropriate value. single return status at end.
	{
		case 0:
		 	/* catgets 117 */
			fprintf (stderr, "117: No errors found.\n\n");
			return EXIT_NO_ERROR;

		case 0xff:
			/* catgets 118 */
			fprintf (stderr, "118: There are fatal errors.\n\n");
			return EXIT_FATAL_ERROR;

		case 0xf8:
			/* catgets 119 */
			fprintf (stderr, "119: Fail to run checking program \n\n");
			return EXIT_FATAL_ERROR;

		case 0xfe:
			/* catgets 120, 121, 122 */
			fprintf (stderr, "120: No fatal errors found.\n\n" );
			fprintf (stderr, "121: Warning: Some configuration parameters may be incorrect.\n");
			fprintf (stderr, "122:         They are either ignored or replaced by default values.\n\n");
			return EXIT_WARNING_ERROR;

		default:
			/* catgets 123 */
			fprintf (stderr, "123: Errors found.\n\n");
			return EXIT_FATAL_ERROR;
	}

	return 0;
}

static int
changeUserEUId (void)
{
	static char fname[] = "changeUserEUId";
	uid_t uid;

	uid = getuid ();

	if (uid == 0) {
		return 0;
	}

	if( lsfSetXUid(0, -1, uid, -1, seteuid) < 0 ) {
		ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, fname, "setresuid/seteuid", uid);
		return -1;
	}

	return 0;
}
