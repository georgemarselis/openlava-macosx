/* $Id: userok.c 397 2007-11-26 19:04:00Z mblack $
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


#ifdef __linux__
#include <netdb.h>  // ruserok()
// FIXME FIXME FIXME FIXME alter configure.ac to specify that ruserok() function is mandatory
#else
	#error stop compilation if other operating systems found
#endif

#include "lib/lproto.h"
#include "libint/intlibout.h"
#include "libint/userok.h"

#define NL_SETN      22

#define LSF_AUTH            9     // FIXME FIXME replace defines with include resd.h
#define LSF_USE_HOSTEQUIV   10    //    FIXME FIXME but also re-adjust the entries from resdh.
#define LSF_ID_PORT         11    //        with the numbers from here
#define _USE_TCP_           0x04
extern struct config_param genParams_[];
#define LOOP_ADDR       0x7F000001


char *auth_user (u_long in, u_short local, u_short remote)
{
// #if 0
	const unsigned int TIMEOUT  = 60;
	const unsigned int SIZE     = 2048;
	static char *ruser          = NULL;
	static u_short id_port      = 0;
	char *readBuf               = NULL;
	char *authd_port            = NULL;
	char *rbuf                  = NULL;
	char *bp                    = NULL;
	u_short rlocal              = 0;
	u_short rremote             = 0;
	int s                       = 0;
	ssize_t n                   = 0;
	unsigned int oldTimer       = 0;
	sigset_t *newMask 			= NULL;
	sigset_t *oldMask			= NULL;
	size_t bufsize              = 0;
	struct sockaddr_in saddr;
	struct itimerval old_itimer;
	struct sigaction action;
	struct sigaction old_action;
	struct servent *svp         = NULL;

	if( !sigemptyset( newMask ) || !sigemptyset( oldMask ) ){
		ls_syslog( LOG_ERR, "%s: one of the two signal masks where not initalized:\noldMask: %s\nnewMask: %s\n", __PRETTY_FUNCTION__, (char *)oldMask, (char *) newMask ); 	// casts are fine
		fprintf(stderr, "%s: one of the two signal masks where not initalized:\noldMask: %s\nnewMask: %s\n", __PRETTY_FUNCTION__, (char *)oldMask, (char *) newMask );		// casts are fine

		// FIXME FIXME implement another dump mechanism

	}


	if (id_port == 0) {
		authd_port = genParams_[LSF_ID_PORT].paramValue;
		if (authd_port != NULL) {
			if ((id_port = atoi (authd_port)) == 0) {
			/* catgets 5801 */
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5801, "%s: LSF_ID_PORT in lsf.conf must be positive number"), __PRETTY_FUNCTION__);
				return NULL;
			}
			id_port = htons (id_port);
		}
		else {
			svp = getservbyname ("ident", "tcp");
			if (!svp) {
				/* catgets 5802 */
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5802, "%s: %s(%s/tcp) failed: %m"), __PRETTY_FUNCTION__, "getservbyname", "ident");
				return NULL;
			}
			id_port = svp->s_port;
		}
	}

	if ((s = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
		return 0;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = id_port;
	saddr.sin_addr.s_addr = in;

	ls_syslog (LOG_DEBUG, "%s: Calling for authentication at <%s>, port:<%d>",  __PRETTY_FUNCTION__, inet_ntoa (saddr.sin_addr), id_port);

	if (b_connect_ (s, (struct sockaddr *) &saddr, sizeof (struct sockaddr_in), TIMEOUT) == -1) {
		int realerrno = errno;
		close (s);
		ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __PRETTY_FUNCTION__, "b_connetc_", sockAdd2Str_ (&saddr));
		errno = realerrno;
		return 0;
	}

	if (getitimer (ITIMER_REAL, &old_itimer) < 0) {
		return 0;
	}


	action.sa_flags = 0;
	action.sa_handler = (SIGFUNCTYPE) alarmer_;

	sigfillset (&action.sa_mask);
	sigaction (SIGALRM, &action, &old_action);

	bp = rbuf;
	sprintf (bp, "%u , %u\r\n", remote, local);
	bufsize = strlen (bp);


	blockSigs_ (SIGALRM, newMask, oldMask);

	oldTimer = alarm (TIMEOUT);

	assert( bufsize < LONG_MAX );
	while ((n = write (s, bp, bufsize)) < (ssize_t) bufsize) {

		if (n <= 0) {
				// FIXME FIXME FIXME do a better job at checking errno (man 3 write)
			close (s);
			ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __PRETTY_FUNCTION__, "write", sockAdd2Str_ (&saddr));
			if (errno == EINTR) {
				errno = ETIMEDOUT;
			}
			alarm (oldTimer);
			setitimer (ITIMER_REAL, &old_itimer, NULL);
			sigaction (SIGALRM, &old_action, NULL);
			sigprocmask (SIG_SETMASK, oldMask, NULL);
			return 0;
		}
		else {
			bp += n;
			bufsize -= n;
		}
	}

	alarm (TIMEOUT);
	bp = rbuf;
	n = read (s, readBuf, SIZE);  // FIXME FIXME FIXME FIXME the size set might not be the optimal one
	if (n > 0) {
		int i;
		readBuf[n] = '\0';
		for (i = 0; i <= n; i++) {
			if ((readBuf[i] != ' ') && (readBuf[i] != '\t') && (readBuf[i] != '\r')) {
				*bp++ = readBuf[i];
			}
			if ((bp - rbuf == sizeof (rbuf) - 1) || (readBuf[i] == '\n')) {
				break;
			}
		}
		*bp = '\0';
	}
	close (s);

	if (n <= 0) {
		ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __PRETTY_FUNCTION__, "read", sockAdd2Str_ (&saddr));
		if (errno == EINTR) {
			errno = ETIMEDOUT;
		}
		alarm (oldTimer);
		setitimer (ITIMER_REAL, &old_itimer, NULL);
		sigaction (SIGALRM, &old_action, NULL);
		sigprocmask (SIG_SETMASK, oldMask, NULL);
		return 0;
	}

	alarm (oldTimer);
	setitimer (ITIMER_REAL, &old_itimer, NULL);
	sigaction (SIGALRM, &old_action, NULL);
	sigprocmask (SIG_SETMASK, oldMask, NULL);

	if (logclass & LC_AUTH) {
		ls_syslog (LOG_DEBUG, "%s Authentication buffer (rbuf=<%s>)", __PRETTY_FUNCTION__, rbuf);
	}

	if (sscanf (rbuf, "%hd,%hd: USERID :%*[^:]:%s", 	&rremote, &rlocal, ruser) < 3) {
		/* catgets 5806 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5806, "%s: Authentication data format error (rbuf=<%s>) from %s"), __PRETTY_FUNCTION__, rbuf, sockAdd2Str_ (&saddr));
		return 0;
	}
	if ((remote != rremote) || (local != rlocal)) {
		/* catgets 5807 */
		ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5807, "%s: Authentication port mismatch (remote=%d, local=%d, rremote=%d, rlocal=%d) from %s"),  __PRETTY_FUNCTION__, remote, local, rlocal, rremote, sockAdd2Str_ (&saddr));
		return 0;
	}

	return ruser;
// #endif
//   return NULL;
}


int
userok (int s, struct sockaddr_in *from, char *hostname, struct sockaddr_in *localAddr, struct lsfAuth *auth, int debug)
{
	unsigned short remote = 0;
	char savedUser[MAXHOSTNAMELEN];
	int user_ok = 0;
	char *authKind = NULL;

	if (debug)
	{
		char lsfUserName[MAXLSFNAMELEN];

		if (getLSFUser_ (lsfUserName, sizeof (lsfUserName)) < 0)
		{
			ls_syslog (LOG_ERR, I18N_FUNC_FAIL_MM, __PRETTY_FUNCTION__, "getLSFUser_");
			return FALSE;
		}
		if (strcmp (lsfUserName, auth->lsfUserName) != 0)
		{
			ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5809, "%s: %s rejected in single user mode"), __PRETTY_FUNCTION__, auth->lsfUserName);   /* catgets 5809 */
			return FALSE;
		}
	}

	authKind = genParams_[LSF_AUTH].paramValue;
	if (authKind != NULL)
	{
		if (strcmp (authKind, AUTH_PARAM_EAUTH)) {
			authKind = NULL;
		}
	}

	if (from->sin_family != AF_INET)
	{
		ls_syslog (LOG_ERR, "%s: sin_family != AF_INET", __PRETTY_FUNCTION__);
		return FALSE;
	}

	remote = ntohs (from->sin_port);

	authKind = genParams_[LSF_AUTH].paramValue;
	if (authKind == NULL)
	{
		if (!debug)
		{
			if (remote >= IPPORT_RESERVED || remote < IPPORT_RESERVED / 2)
			{
				ls_syslog (LOG_ERR, "%s: Request from bad port %d, denied", __func__, remote);
				return FALSE;
			}
		}
	}
	else
	{
		if (strcmp (authKind, AUTH_IDENT) == 0)
		{
			struct sockaddr_in saddr;
			socklen_t size;
			char *user = NULL;
			unsigned short local = 0;

			size = sizeof (struct sockaddr_in);
			if (remote >= IPPORT_RESERVED || remote < IPPORT_RESERVED / 2)
			{

				if (getsockname (s, (struct sockaddr *) &saddr, &size) == -1)
				{
					ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __PRETTY_FUNCTION__, "getsockname");
					return FALSE;
				}
				local = ntohs (saddr.sin_port);

				if (getpeername (s, (struct sockaddr *) &saddr, &size) == -1)
				{
					ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __PRETTY_FUNCTION__, "getpeername");
					return FALSE;
				}

				user = auth_user (saddr.sin_addr.s_addr, local, remote);
				if (!user)
				{
					if (errno == ETIMEDOUT)
					{
						/*catgets 5824 */
						ls_syslog (LOG_INFO, I18N (5824, "%s: auth_user %s returned NULL, retrying"), __PRETTY_FUNCTION__, sockAdd2Str_ (from));

						millisleep_ (500 + hostValue () * 5000 / 255);
						user = auth_user (from->sin_addr.s_addr, local, remote);
						if (!user)
						{
							/* catgets 5815 */
							ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5815, "%s: auth_user %s retry failed: %m"), __PRETTY_FUNCTION__, sockAdd2Str_ (from));
							return FALSE;
						}
						else
						{
				 			/*catgets 5825 */
							ls_syslog (LOG_INFO, I18N (5825, "%s: auth_user %s retry succeeded"), __PRETTY_FUNCTION__, sockAdd2Str_ (from));
						}
					}
					else
					{
						/*catgets 5826 */
						ls_syslog (LOG_INFO, I18N (5826, "%s: auth_user %s returned NULL"), __PRETTY_FUNCTION__, sockAdd2Str_ (from));
						return FALSE;
					}
				}
				else
				{
					if (strcmp (user, auth->lsfUserName) != 0)
					{
						/* catgets 5816 */
						ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5816, "%s: Forged username suspected from %s: %s/%s"),   __PRETTY_FUNCTION__, sockAdd2Str_ (from), auth->lsfUserName, user);
						return FALSE;
					}
				}
			}
		}
		else
		{
			if (!strcmp (authKind, AUTH_PARAM_EAUTH))
			{
				if (auth->kind != CLIENT_EAUTH)
				{
					/* catgets 5817 */
					ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5817, "%s: Client %s is not using <%d/eauth> authentication"), __PRETTY_FUNCTION__, sockAdd2Str_ (from), (int) auth->kind);
					return FALSE;
				}

				if (verifyEAuth_ (auth, from) == -1)
				{
					/* catgets 5818 */
					ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5818, "%s: %s authentication failed for %s/%s"), __PRETTY_FUNCTION__, "eauth", auth->lsfUserName, sockAdd2Str_ (from));
					return FALSE;
				}
			}
			else
			{
				/* catgets 5819 */
				ls_syslog (LOG_ERR, _i18n_msg_get (ls_catd, NL_SETN, 5819, "%s: Unkown authentication type <%d> from %s/%s; denied"), __PRETTY_FUNCTION__, auth->kind, auth->lsfUserName,	sockAdd2Str_ (from));
				return FALSE;
			}

		}
	}

	if (genParams_[LSF_USE_HOSTEQUIV].paramValue)
	{
		strcpy (savedUser, auth->lsfUserName);

		user_ok = ruserok (hostname, 0, savedUser, savedUser);
		if (user_ok == -1)
		{
			ls_syslog (LOG_INFO, I18N_FUNC_S_S_FAIL, __PRETTY_FUNCTION__, "ruserok", hostname, savedUser);
			return FALSE;
		}
	}

	return TRUE;

}


int shostOk (char *fromHost, int options)
{
	return 1;
}


int hostIsLocal (char *hname)
{
	return 1;
}
