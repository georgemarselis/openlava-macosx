// added by George Marselis <george@marsel.is> Wed Marc 27 2019

#pragma once

#define ISBOUNDARY(h1, h2, len)  ( (h1[len]=='.' || h1[len]=='\0') && (h2[len]=='.' || h2[len]=='\0') )

// #define MAX_HOSTALIAS 64
enum MAX_HOSTALIAS  {
	MAX_HOSTALIAS = 64
};
/* #define MAX_HOSTIPS   32 */
	
/* host.c */
char           *ls_getmyhostname( void ) ;
struct hostent *Gethostbyname_  ( const char *hname);
struct hostent *Gethostbyaddr_  ( in_addr_t *addr, socklen_t len, int type);
int             equalHost_      ( const char *host1, const char *host2);
char           *sockAdd2Str_    ( struct sockaddr_in *from);
void            stripDomain     ( char *name );
int             mkHostTab       ( void );
void            addHost2Tab     ( const char *hostname, in_addr_t **addrs, char **aliases);
int             getAskedHosts_  ( const char *optarg_, char ***askedHosts, unsigned int *numAskedHosts, unsigned long *badIdx, int checkHost);
