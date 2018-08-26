// added by George Marselis <george@marsel.is>

#pragma once

// #define MAX_HOSTALIAS 64
enum MAX_HOSTALIAS  {
	MAX_HOSTALIAS = 64
};
/* #define MAX_HOSTIPS   32 */
	

char *ls_getmyhostname(void);
struct hostent *Gethostbyname_(char *hname);
struct hostent *Gethostbyaddr_(in_addr_t *addr, socklen_t len, int type);
int equalHost_(const char *host1, const char *host2);
char *sockAdd2Str_(struct sockaddr_in *from);
void stripDomain(char *name);
int mkHostTab(void);
void addHost2Tab(const char *hname, in_addr_t **addrs, char **aliases);
int getAskedHosts_(char *optarg_, char ***askedHosts, unsigned int *numAskedHosts, unsigned long *badIdx, int checkHost);
