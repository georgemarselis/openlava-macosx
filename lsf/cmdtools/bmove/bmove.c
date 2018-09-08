/* $Id: cmd.move.c 397 2007-11-26 19:04:00Z mblack $
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

#include "cmdtools/cmdtools.h"
#include "cmdtools/cmd.h"
#include "libint/lsi18n.h"

// #define NL_SETN 8


void usage (char *cmd)
{
	fprintf (stderr, "%s\n", _i18n_msg_get(ls_catd, 33, 5000, "Usage") );
	fprintf (stderr, ": %s [-h] [-V] jobId | \"jobId[index]\" [position]\n", cmd);
	exit (-1);
}

void
bmove (int argc, char **argv, int opCode)
{
	int position = 0;
	int reqPos   = 0;
	LS_LONG_INT jobId = 0;
	int achar    = 0;

	if (lsb_init (argv[0]) < 0)
	{
		lsb_perror ("lsb_init");
		exit (-1);
	}

	opterr = 0;
	while ((achar = getopt (argc, argv, "hV")) != EOF) {
		switch (achar)
		{
			case 'V':
			fputs (_LS_VERSION_, stderr);
			exit (0);
			break;
			case 'h':
			default:
			usage (argv[0]);
			break;
		}
	}
	if (argc == optind)
	{
		/* catgets  852  */
		fprintf (stderr, "%s.\n", (_i18n_msg_get (ls_catd, NL_SETN, 852, "Job ID must be specified")));
		usage (argv[0]);
	}
	if (optind < argc - 2)
	{
		/* catgets  853  */
		fprintf (stderr, "%s.\n", (_i18n_msg_get (ls_catd, NL_SETN, 853, "Command syntax error: too many arguments")));
		usage (argv[0]);
	}

	if (getOneJobId (argv[optind], &jobId, 0))
	{
		usage (argv[0]);
	}

	position = 1;
	if (optind == argc - 2)
	{
		if (!isint_ (argv[++optind]) || atoi (argv[optind]) <= 0)
		{
		/* catgets854 */
			fprintf (stderr, "%s: %s.\n", argv[optind], I18N (854, "Position value must be a positive integer"));
			usage (argv[0]);
		}
		position = atoi (argv[optind]);
	}

	reqPos = position;
	if (lsb_movejob (jobId, &position, opCode) < 0)
	{
		lsb_perror (lsb_jobid2str (jobId));
		exit (-1);
	}

	if (position != reqPos) {
		/* catgets  855  */
		fprintf (stderr, "%s\n", (_i18n_msg_get (ls_catd, NL_SETN, 855, "Warning: position value <%d> is beyond movable range.\n")), reqPos);
	}
	if (opCode == TO_TOP) {
		/* catgets  856  */
		fprintf (stderr, "%s\n", (_i18n_msg_get (ls_catd, NL_SETN, 856, "Job <%s> has been moved to position %d from top.\n")), lsb_jobid2str (jobId), position);
	}
	else {
		/* catgets  857  */
		fprintf (stderr, "%s\n", (_i18n_msg_get (ls_catd, NL_SETN, 857, "Job <%s> has been moved to position %d from bottom.\n")), lsb_jobid2str (jobId), position);
	}
	exit (0);
}
