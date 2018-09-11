/*
 * Copyright (C) 2011 David Bigagli
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

#include <arpa/inet.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "lib/lib.h"
#include "lib/lproto.h"
#include "lib/table.h"
#include "lib/host.h"
#include "lib/initenv.h"
#include "lib/words.h"


/**************************************************************
 * this file must be compiled together with channel.c cuz there is
 * sockAdd2Str_() to resolve
 *
 */

static struct hTab *nameTab;
static struct hTab *addrTab;

/* ls_getmyhostname()
 */
char *
ls_getmyhostname (void)
{
	char hostname[MAXHOSTNAMELEN];
	struct hostent *hp = NULL;

	memset( hostname, '\0', MAXHOSTNAMELEN );

// YOUR CACHING MECHANISM IS BAD AND YOU SHOULD FEEL BAD
	// if (hostname[0] != 0){
	//     return hostname;
	// }

	gethostname (hostname, MAXHOSTNAMELEN);
	hp = Gethostbyname_ (hostname);
	if (hp == NULL)  {
			return NULL;
	}

	// strcpy (hostname, hp->h_name);

	return hp->h_name;
}

/* Gethostbyname_()
 */
struct hostent *
Gethostbyname_ (char *hostname)
{
	// int cc;
	struct hEnt *e = NULL;
	struct hostent *hp = NULL;
	char lsfHname[MAXHOSTNAMELEN];

	memset( lsfHname, '\0', MAXHOSTNAMELEN );

	if (strlen (hostname) >= MAXHOSTNAMELEN) {
		lserrno = LSE_BAD_HOST;
		return NULL;
	}

	strcpy (lsfHname, hostname);
		/* openlava strips all hostnames
		 * of their domain names.
		 */
	stripDomain (lsfHname);

		/* This is always somewhat controversial
		 * should we have an explicit libray init
		 * call host_cache_init() or doing the
		 * initialization as part of the first call...
		 */
	if (nameTab == NULL) {
		mkHostTab ();
	}

	e = h_getEnt_ (nameTab, lsfHname);
	if (e) {
		hp = e->hData;
		return hp;
	}

	hp = gethostbyname (lsfHname);
	if ( NULL == hp ) {
		lserrno = LSE_BAD_HOST;
		return NULL;
	}
	stripDomain (hp->h_name);
	/* add the new host to the host hash table
	 */
	addHost2Tab (hp->h_name, (in_addr_t **) hp->h_addr_list, hp->h_aliases);

/*    if (0) {
			dybag should we write a command
			 * to dump the cache sooner or later.
			 
			cc = 0;
			while (hp->h_addr_list[cc]) {
					char *p;
					struct in_addr a;
					memcpy (&a, hp->h_addr_list[cc], sizeof (a));
					p = inet_ntoa (a);
					fprintf (stderr, "%s\n", p);  could be closed 
					++cc;
				}
		}
*/

		return hp;
}

/* Gethostbyaddr_()
 */
struct hostent *
Gethostbyaddr_ (in_addr_t * addr, socklen_t len, int type)
{
	struct hostent *hp = NULL;
	struct hEnt *e = NULL;
	char ipbuf[32]; // FIXME FIXME FIXME FIXME get rid of the 32; why 32 chars?

	memset( ipbuf, '\0', 32 ); // FIXME FIXME FIXME FIXME get rid of the 32; why 32 chars?s

	/* addrTab is built together with
	 * nameTab.
	 */
	if (nameTab == NULL) {
		mkHostTab ();
	}

	sprintf (ipbuf, "%u", *addr);

	e = h_getEnt_ (addrTab, ipbuf);
	if (e)
		{
			hp = e->hData;
			return hp;
		}

	hp = gethostbyaddr (addr, len, type);
	if (hp == NULL)
		{
			lserrno = LSE_BAD_HOST;
			return NULL;
		}
	stripDomain (hp->h_name);

	addHost2Tab (hp->h_name, (in_addr_t **) hp->h_addr_list, hp->h_aliases);
	return hp;
}

#define ISBOUNDARY(h1, h2, len)  ( (h1[len]=='.' || h1[len]=='\0') && (h2[len]=='.' || h2[len]=='\0') )

int
equalHost_ (const char *host1, const char *host2)
{
		size_t len = 0;

		if (strlen (host1) > strlen (host2)) {
			len = strlen (host2);
		}
		else {
			len = strlen (host1);
		}

		if ((strncasecmp (host1, host2, len) == 0) && ISBOUNDARY (host1, host2, len)) {
				return TRUE;
		}

		return FALSE;
}

/* sockAdd2Str_()
 */
char *
sockAdd2Str_ (struct sockaddr_in *from)
{
	static char adbuf[24]; // FIXME FIXME FIXME not having the "static" qualifier 

	sprintf (adbuf, "%s:%hu", inet_ntoa (from->sin_addr), ntohs (from->sin_port));
	return adbuf;
}

/* stripDomain()
 */
void
stripDomain (char *name)
{
	char *p = NULL;

	if ((p = strchr (name, '.'))) {
		*p = 0;
	}

	return;
}

/* mkHostTab()
 */
int
mkHostTab (void)
{
	FILE *fp = NULL;
	char *buf = NULL;
	char fbuf[BUFSIZ];

	memset( fbuf, '\0', BUFSIZ ); // BUFSIZ is a GNU C lib macro https://www.gnu.org/software/libc/manual/html_node/Controlling-Buffering.html

	if (nameTab){
		assert (addrTab);
		return -1;
	}

	nameTab = calloc (1, sizeof (struct hTab));
	addrTab = calloc (1, sizeof (struct hTab));

	h_initTab_ (nameTab, 101);
	h_initTab_ (addrTab, 101);

	if (initenv_ (NULL, NULL) < 0) {
		return -1;
	}

	if (genParams_[NO_HOSTS_FILE].paramValue) {
		return -1;
	}

	sprintf (fbuf, "%s/hosts", genParams_[LSF_CONFDIR].paramValue); // FIXME FIXME FIXME FIXME move "hosts" configure.ac

	if ((fp = fopen (fbuf, "r")) == NULL)
		return -1;

	while ((buf = nextline_ (fp))) {
		char *addrstr = NULL;
		char *name = NULL;
		char *p = NULL;
		char *alias[MAX_HOSTALIAS];
		in_addr_t *addr[2];
		in_addr_t x;
		int cc = 0;;

		memset (alias, 0, sizeof (char *) * MAX_HOSTALIAS);

		addrstr = getNextWord_ (&buf);
		if (addrstr == NULL) {
			continue;
		}

		x = inet_addr (addrstr);
		addr[0] = &x;
		addr[1] = NULL;

		name = getNextWord_ (&buf);
		if (name == NULL) {
			continue;
		}

		cc = 0;
		while ((p = getNextWord_ (&buf)) && cc < MAX_HOSTALIAS) {
			alias[cc] = strdup (p);
			++cc;
		}
		/* multihomed hosts are
		 * listed multiple times
		 * in the host file each time
		 * with the same name but different
		 * addr.
		 *
		 * 192.168.7.1 jumbo
		 * 192.168.7.4 jumbo
		 *     ...
		 */
		addHost2Tab (name, addr, alias);

		cc = 0;
		while (alias[cc]) {
			FREEUP (alias[cc]);
			++cc;
		}

	}               /* while() */

	fclose (fp);

	return 0;
}

/* addHost2Tab()
 */
void
addHost2Tab (const char *hostname, in_addr_t ** addrs, char **aliases)
{
	struct hostent *hp = NULL;
	char ipbuf[32] = NULL;
	struct hEnt *e = NULL;
	struct hEnt *e2 = NULL;
	int new = 0;
	unsigned long cc = 0;;

	/* add the host to the table by its name
	 * if it exists already we must be processing
	 * another ipaddr for it.
	 */
	e = h_addEnt_ (nameTab, hostname, &new);
	if (new)
		{
			hp = calloc (1, sizeof (struct hostent));
			hp->h_name = strdup (hostname);
			hp->h_addrtype = AF_INET;
			hp->h_length = 4;
			e->hData = hp;
		}
	else
		{
			hp = (struct hostent *) e->hData;
		}

		cc = 0;
		while (aliases[cc]) {
				++cc;
		}

		hp->h_aliases = calloc(cc + 1, sizeof (char *));
		cc = 0;
		
		while (aliases[cc]) {
			hp->h_aliases[cc] = strdup (aliases[cc]);
			++cc;
		}

		cc = 0;
		while (addrs[cc]) {
				++cc;
		}

		hp->h_addr_list = calloc (cc + 1, sizeof (char *));
		cc = 0;
		
		while (addrs[cc]) {
			hp->h_addr_list[cc] = calloc (1, sizeof (in_addr_t));
			memcpy (hp->h_addr_list[cc], addrs[cc], sizeof (in_addr_t));
			/* now hash the host by its addr,
			 * there can be N addrs but each
			 * must be unique...
			 */
			sprintf (ipbuf, "%u", *(addrs[cc]));
			e2 = h_addEnt_ (addrTab, ipbuf, &new);
			/* If new is false it means this IP
			 * is configured for another host already,
			 * confusion is waiting down the road as
			 * Gethostbyadrr_() will always return the
			 * first configured host.
			 * 192.168.1.4 joe
			 * 192.168.1.4 banana
			 * when banana will call the library will
			 * always tell you joe called.
			 */
			if (new)
		e2->hData = hp;

			++cc;         /* nexte */
		}
}

/* getAskedHosts_()
 */

// *optarg_ was originally named optarg, but there is a variable named optarg in lsf.h
//      so, rename.
int
getAskedHosts_ (char *optarg_, char ***askedHosts, unsigned int *numAskedHosts, unsigned long *badIdx, int checkHost)
{
	unsigned long  num = 64;
	char *word;
	char *hostname;
	char **tmp;
	int foundBadHost = FALSE;
	static char **hlist = NULL;
	static unsigned long nhlist = 0;
	char host[MAXHOSTNAMELEN];

		if (hlist)
		{
				for ( unsigned long i = 0; i < nhlist; i++) {
						free (hlist[i]);
				}
				free (hlist);
				hlist = NULL;
		}

		nhlist = 0;
		hlist = calloc (num, sizeof (char *) );
		if ( NULL == hlist )   {
			lserrno = LSE_MALLOC;
			return (-1);
		}

	*badIdx = 0;

	while ((word = getNextWord_ (&optarg_)) != NULL)
		{
			strncpy (host, word, sizeof (host));
			if (ls_isclustername (host) <= 0)
		{
			if (checkHost == FALSE)
				{
					hostname = host;
				}
			else
				{
					if (Gethostbyname_ (host) == NULL)
				{
					if (!foundBadHost)
						{
							foundBadHost = TRUE;
							*badIdx = nhlist;
						}
					hostname = host;
				}
					else
				{
					hostname = host;
				}
				}
		}
			else
		hostname = host;

			if ((hlist[nhlist] = putstr_ (hostname)) == NULL)
		{
			lserrno = LSE_MALLOC;
			goto Error;
		}

			nhlist++;
			if (nhlist == num)
		{
			if ((tmp = realloc( hlist, 2 * num * sizeof( tmp ) ) ) == NULL)
				{
					lserrno = LSE_MALLOC;
					goto Error;
				}
			hlist = tmp;
			num = 2 * num;
		}
		}

		assert( nhlist <= UINT_MAX );
	*numAskedHosts = nhlist;
	*askedHosts = hlist;

	if (foundBadHost)
		{
			lserrno = LSE_BAD_HOST;
			return (-1);
		}

	return (0);

// FIXME FIXME FIXME FIXME 
// GOTOS GOT TO GO
Error:

		for (unsigned long i = 0; i < nhlist; i++) {
				free (hlist[i]);
		}

		free (hlist);
		hlist = NULL;
		nhlist = 0;
		return (-1);
}
