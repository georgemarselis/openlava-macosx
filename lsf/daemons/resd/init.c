/* $Id: res.init.c 397 2007-11-26 19:04:00Z mblack $
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

#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>


#include "lib/mls.h"
#include "lib/lib.h"
// #include "lib/lproto.h"
#include "daemons/libresd/init.h"
#include "daemons/libresd/resd.h"

void
initConn2NIOS (void)
{
	conn2NIOS.task_duped = calloc (sysconf (_SC_OPEN_MAX), sizeof (int));

	if ( NULL == conn2NIOS.task_duped )
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
		resExit_ (-1);
	}

	conn2NIOS.sock->rbuf = malloc (sizeof (struct relaybuf));
	if ( NULL == conn2NIOS.sock->rbuf)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
		resExit_ (-1);
	}

	conn2NIOS.sock->wbuf = malloc (sizeof ( struct relaylinebuf));
	if ( NULL == conn2NIOS.sock->wbuf)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
		resExit_ (-1);
	}

	conn2NIOS.sock->fd          = -1;
	conn2NIOS.wtag              = -1;
	conn2NIOS.rtag              = -1;
	conn2NIOS.num_duped         = 0;
	conn2NIOS.sock->rcount      = 0;
	conn2NIOS.sock->wcount      = 0;
	conn2NIOS.sock->rbuf->bcount = 0;
	conn2NIOS.sock->wbuf->bcount = 0;

	return;
}


void
init_res( void )
{
	int i = 0;
	int maxfds = 0;

	if (logclass & (LC_TRACE | LC_HANG)) {
		ls_syslog (LOG_DEBUG, "%s: Entering this routine...", __func__);
	}

	if (!sbdMode)  // FIXME FIXME FIXME FIXME this control code can be made more brief
	{
		if (!debug)
		{

			if (geteuid () || getuid ())
			{
				fprintf (stderr, "RES should be run as root.\n");
				// FIXME FIXME FIXME add error message to syslog, as well.
				fflush (stderr);
				resExit_ (1);
			}

			chdir ("/tmp"); // FIXME FIXME FIXME FIXME remove fixed string, replace with OS-dependent constant
		}

		if (debug <= 1)
		{
			daemonize_ ();
			ls_openlog ("res", resParams[LSF_LOGDIR].paramValue, 0, resParams[LSF_LOG_MASK].paramValue);
			umask (0);
			nice (NICE_LEAST);
		}
	}

	if ((Myhost = ls_getmyhostname ()) == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __func__, "ls_getmyhostname");
		resExit_ (-1);
	}


	if (isatty (0))
	{
		tcgetattr (0, &defaultTty.attr);

		defaultTty.ws.ws_row = 24;
		defaultTty.ws.ws_col = 80;
		defaultTty.ws.ws_xpixel = defaultTty.ws.ws_ypixel = 0;
	}
	else
	{
		defaultTty.ws.ws_row = 24;
		defaultTty.ws.ws_col = 80;
		defaultTty.ws.ws_xpixel = defaultTty.ws.ws_ypixel = 0;
	}

	if (!sbdMode)
	{
		init_AcceptSock ();
	}

	client_cnt = child_cnt = 0;
	for( uint i = 0; i < MAXCLIENTS_HIGHWATER_MARK + 1; i++) // FIXME FIXME FIXME FIXME FIXME MAXCLIENTS_HIGHWATER_MARK should be confiruable in configure.ac, or dependant on the OS
	{
		clients[i] = NULL;
	}

	children = calloc (sysconf (_SC_OPEN_MAX), sizeof (struct children *));
	if( NULL == children )
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
		resExit_ (-1);
	}

	maxfds = sysconf (_SC_OPEN_MAX);
	for ( uint i = 0; i < maxfds; i++)
	{
		children[i] = NULL;
	}

	initConn2NIOS ();
	resNotifyList = listCreate ("resNotifyList");
	if (!resNotifyList)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "listCreate");
		resExit_ (-1);
	}

	/* catgets 5346 */
	ls_syslog (LOG_INFO, (_i18n_msg_get (ls_catd, NL_SETN, 5346, "Daemon started")));

	initResLog ();

	return;
}

/* init_AcceptSock()
 */

// FIXME FIXME FIXME FIXME FIXME memory management needs work
void
init_AcceptSock (void)
{

	struct sockaddr_in svaddr = { };
	struct servent *sv = NULL ;
	struct hostent *hp = NULL;
	socklen_t len = 0;;
	int one = 1;

	memset( &svaddr, 0, sizeof (svaddr)); // FIXME FIXME FIXME FIXME why is memsetting here requried? 
	if ((accept_sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "socket", "RES");
		resExit_ (1);
	}

	setsockopt (accept_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (int));

	if (io_nonblock_ (accept_sock) < 0) {
		ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "io_nonblock_", accept_sock);
	}

	fcntl (accept_sock, F_SETFD, (fcntl (accept_sock, F_GETFD) | FD_CLOEXEC));
	if (resParams[LSF_RES_PORT].paramValue)
	{
		if ((svaddr.sin_port = atoi (resParams[LSF_RES_PORT].paramValue)) == 0)
		{
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5307, "%s: LSF_RES_PORT in lsf.conf (%s) must be positive integer; exiting"), __func__, resParams[LSF_RES_PORT].paramValue);    /* catgets 5307 */
			resExit_ (1);
		}
		svaddr.sin_port = htons (svaddr.sin_port);
	}
	else if (debug)
	{
		svaddr.sin_port = htons (RES_PORT);
	}
	else
	{
		if ((sv = getservbyname ("res", "tcp")) == NULL)
		{
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5309, "%s: res/tcp: unknown service, exiting"), __func__);  /* catgets 5309 */
			resExit_ (1);
		}
		svaddr.sin_port = sv->s_port;
	}

	svaddr.sin_family = AF_INET;
	svaddr.sin_addr.s_addr = INADDR_ANY;
	if (Bind_ (accept_sock, (struct sockaddr *) &svaddr, sizeof (svaddr)) < 0)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "accept_sock", ntohs (svaddr.sin_port));
		resExit_ (1);
	}

	if (listen (accept_sock, 1024) < 0) // FIXME FiXME FIXME fixed number here should be replaced appropriatelly
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "listen");
		resExit_ (1);
	}

	if ((ctrlSock = TcpCreate_ (TRUE, 0)) < 0)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "TcpCreate_");
		resExit_ (1);
	}

	len = sizeof (ctrlAddr); // FIXME FIXME verify sizeof( is correct )
	memset( &ctrlAddr, 0, sizeof (ctrlAddr ) );
	if (getsockname (ctrlSock, (struct sockaddr *) &ctrlAddr, &len) < 0) // FIXME FIXME verify cast
	{
		ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "getsockname", ctrlSock);
		resExit_ (-1);
	}

	if ((hp = Gethostbyname_ (Myhost))) {
		memcpy ( &ctrlAddr.sin_addr, hp->h_addr, hp->h_length);
	}

	return;
}

void
initChildRes (char *envdir)
{
	int i, maxfds;

	getLogClass_ (resParams[LSF_DEBUG_RES].paramValue,resParams[LSF_TIME_RES].paramValue);

	openChildLog ("res", resParams[LSF_LOGDIR].paramValue, (debug > 1), &(resParams[LSF_LOG_MASK].paramValue));

	if ((Myhost = ls_getmyhostname ()) == NULL)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "ls_getmyhostname");
		resExit_ (-1);
	}
	client_cnt = child_cnt = 0;

	for (i = 0; i < MAXCLIENTS_HIGHWATER_MARK + 1; i++)
	{
		clients[i] = NULL;
	}
	children = calloc (sysconf (_SC_OPEN_MAX), sizeof (struct children *));
	if (!children)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "calloc");
		resExit_ (-1);
	}
	maxfds = sysconf (_SC_OPEN_MAX);

	for (i = 0; i < maxfds; i++)
	{
		children[i] = NULL;
	}

	initConn2NIOS ();
	resNotifyList = listCreate ("resNotifyList");
	if (!resNotifyList)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "listCreate");
		resExit_ (-1);
	}
	return;
}


int
resParent (int s, struct passwd *pw, struct lsfAuth *auth, struct resConnect *connReq, struct hostent *hostp)
{
	struct resChildInfo childInfo = { };
	struct LSFHeader hdr          = { };
	XDR xdrs      = { };
	char **argv   = NULL;
	char *hndlbuf = malloc( sizeof(char) * 64 + 1 ); // FIXME FIXME FIXME FIXME FIXME wtf is 64?
	char *buf     = malloc( sizeof( char ) * (2 * MSGSIZE ) + 1 ) ; // FIXME FIXME FIXME FIXME FIXME  what is MSGSIZE and why allocate twise the size?
	pid_t pid     = 0;
	size_t len    = 0;
	int *hpipe    = malloc( sizeof( int ) * 2 );
	int *wrapPipe = malloc( sizeof( int ) * 2 );
	int cc        = 0;

	if (resParams[LSF_SERVERDIR].paramValue != NULL) {
		argv[0] = getDaemonPath_ ("/res", resParams[LSF_SERVERDIR].paramValue); // FIXME FIXME FIXME FIXME FIXME replace fixed string with autotools value
	}
	else {
		argv[0] = "res";  // FIXME FIXME FIXME FIXME FIXME replace fixed string with autotools value
	}

	childInfo.resConnect = connReq;
	childInfo.lsfAuth = auth;
	childInfo.pw = pw;
	childInfo.host = hostp;
	childInfo.parentPort = ctrlAddr.sin_port;
	childInfo.currentRESSN = currentRESSN;


	if (resLogOn == 1) {
		char *strLogCpuTime = malloc( sizeof( char )* 32 + 1 ); // FIXME FIXME FIXME FIXME FIXME replace fixed number with constant. why 32? autoconf const?

		sprintf (strLogCpuTime, "%d", resLogcpuTime);
		putEnv ("LSF_RES_LOGON", "1"); 				// FIXME FIXME FIXME get appropriate gParams variable in.
		putEnv ("LSF_RES_CPUTIME", strLogCpuTime);    // FIXME FIXME FIXME get appropriate gParams variable in.
		putEnv ("LSF_RES_ACCTPATH", resAcctFN);       // FIXME FIXME FIXME get appropriate gParams variable in.
	}
	else if (resLogOn == 0)
	{
		putEnv ("LSF_RES_LOGON", "0");                // FIXME FIXME FIXME get appropriate gParams variable in.
	}
	else if (resLogOn == -1)
	{
		putEnv ("LSF_RES_LOGON", "-1");               // FIXME FIXME FIXME get appropriate gParams variable in.

		xdrmem_create (&xdrs, buf, 2 * MSGSIZE, XDR_ENCODE);
		memset ( &hdr, 0, sizeof (struct LSFHeader));
		hdr.version = OPENLAVA_VERSION; // FIXME FIXME FIXME FIXME FIXME set in configure.ac
		if (!xdr_resChildInfo (&xdrs, &childInfo, &hdr))
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL, __func__, "xdr_resChildInfo");
			return -1;
		}
		len = XDR_GETPOS (&xdrs);

		if (socketpair (AF_UNIX, SOCK_STREAM, 0, hpipe) < 0)
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "socketpair");
			return -1;
		}
		if (socketpair (AF_UNIX, SOCK_STREAM, 0, wrapPipe) < 0)
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "socketpair");
			return -1;
		}
		sprintf (hndlbuf, "%d:%d", hpipe[1], s);

		// FIXME FIXME FIXME FIXME replace with getopts() ;
		// FIXME FIXME FIXME FIXME explain what the otpions do.
		cc = 1;
		argv[cc++] = "-d";
		argv[cc++] = env_dir;
		if (debug)
		{
			if (debug == 1) {
				argv[cc++] = "-1";
			}
			else {
				argv[cc++] = "-2";
			}
			argv[cc++] = "-s";
			argv[cc++] = hndlbuf;
			argv[cc++] = NULL;
		}
		else
		{
			argv[cc++] = "-s";
			argv[cc++] = hndlbuf;
			argv[cc++] = NULL;
		}


if (getenv ("LSF_SETDCEPAG") == NULL) { // FIXME FIXME FIXME get appropriate gParams variable in.
	putEnv ("LSF_SETDCEPAG", "Y");      // FIXME FIXME FIXME get appropriate gParams variable in.
}

	pid = fork ();
	if (pid < 0)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "fork");
		close (hpipe[0]);
		close (hpipe[1]);
		close (wrapPipe[0]);
		close (wrapPipe[1]);
		return -1;
	}

	if (pid == 0)
	{
		if (logclass & LC_TRACE)
		{
			if (debug)
			{
				ls_syslog (LOG_DEBUG2, "%s: executing %s %s %s %s %s %s ",
					__func__, argv[0], argv[1], argv[2], argv[3], argv[4],
					argv[5]);
			}
			else
			{
				ls_syslog (LOG_DEBUG2, "%s: executing %s %s %s %s %s ",
					__func__, argv[0], argv[1], argv[2], argv[3], argv[4]);
			}
		}
		close (hpipe[0]);
		close (wrapPipe[0]);


		if (dup2 (wrapPipe[1], 0) == -1)
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "dup2");
			exit (-1);
		}
		close (wrapPipe[1]);
		lsfExecX(argv[0], argv, execvp); // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "execv");
		exit (-1);
	}

	close (hpipe[1]);
	close (wrapPipe[1]);

	if (connReq->eexec.len > 0)
	{
		int cc1;
		if ((cc1 = b_write_fix (wrapPipe[0], connReq->eexec.data,
			connReq->eexec.len)) != connReq->eexec.len)
		{
			/* catgets 5333 */
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5333, "%s: Falied in sending data to wrap for user <%s>, length = %d, cc=1%d: %m"), __func__, pw->pw_name, connReq->eexec.len, cc1);
			close (wrapPipe[0]);
			close (hpipe[0]);
			return -1;
		}
	}
	close (wrapPipe[0]);

	if (write (hpipe[0], (char *) &len, sizeof (len)) != sizeof (len))
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "write");
		xdr_destroy (&xdrs);
		close (hpipe[0]);
		return -1;
	}

	if (write (hpipe[0], buf, len) != len)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "write");
		xdr_destroy (&xdrs);
		close (hpipe[0]);
		return -1;
	}
	xdr_destroy (&xdrs);
	close (hpipe[0]);

	//loop over *argv size )
	free( *argv  );
	free( hndlbuf );
	free( buf );
	free( hpipe );
	free (wrapPipe );

	return 0;
}


void
resChild (char *arg, char *envdir)
{
	struct passwd pw;
	struct hostent hp;
	struct resConnect connReq;
	struct lsfAuth auth;
	struct resChildInfo childInfo;
	XDR xdrs;
	int clientHandle, resHandle;
	char *sp, *buf;
	struct LSFHeader hdr;
	int len;
	char *nullist[1];

	initChildRes (envdir);

	sp = strchr (arg, ':');
	sp[0] = '\0';
	sp++;
	resHandle = atoi (arg);
	clientHandle = atoi (sp);

	if (b_read_fix (resHandle, (char *) &len, sizeof (len)) != sizeof (len))
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "b_read_fix");
		resExit_ (-1);
	}
	buf = malloc (len);
	if (!buf)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
		resExit_ (-1);
	}
	if (b_read_fix (resHandle, buf, len) != len)
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "b_read_fix");
		resExit_ (-1);
	}

	CLOSE_IT (resHandle);

	childInfo.pw = &pw;
	childInfo.host = &hp;
	childInfo.resConnect = &connReq;
	childInfo.lsfAuth = &auth;

	xdrmem_create (&xdrs, buf, len, XDR_DECODE);
	hdr.version = OPENLAVA_VERSION;
	if (!xdr_resChildInfo (&xdrs, &childInfo, &hdr))
	{
		ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "xdr_resChildInfo");
		resExit_ (-1);
	}

	ctrlAddr.sin_addr.s_addr = htonl (LOOP_ADDR);
	ctrlAddr.sin_family = AF_INET;
	ctrlAddr.sin_port = childInfo.parentPort;
	currentRESSN = childInfo.currentRESSN;

	if (getenv ("LSF_RES_LOGON")) // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
	{
		if (strcmp (getenv ("LSF_RES_LOGON"), "1") == 0) // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
		{
			resLogOn = 1;
			if (getenv ("LSF_RES_CPUTIME")) // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
			{
				resLogcpuTime = atoi (getenv ("LSF_RES_CPUTIME")); // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
			}
			if (getenv ("LSF_RES_ACCTPATH")) // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
			{
				strcpy (resAcctFN, getenv ("LSF_RES_ACCTPATH")); // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
			}
		}
		else if (strcmp (getenv ("LSF_RES_LOGON"), "0") == 0) // FIXME FIXME FIXME FIXME FIXME sanitize argv[0] 
		{
			resLogOn = 0;
		}
		else if (strcmp (getenv ("LSF_RES_LOGON"), "-1") == 0) // FIXME FIXME FIXME FIXME FIXME sanitize argv[0]
		{
			resLogOn = -1;
		}
	}

	nullist[0] = NULL;
	hp.h_aliases = nullist;

	childAcceptConn (clientHandle, &pw, &auth, &connReq, &hp);

	free (buf);
	return;
}
