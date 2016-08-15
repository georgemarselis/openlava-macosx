
#pragma once 

#include "lib/rdwr.h"
	// needed for static void alarmer_ (void);

int userok (int s, struct sockaddr_in *from, char *hostname, struct sockaddr_in *localAddr, struct lsfAuth *auth, int debug);
char *auth_user (u_long in, u_short local, u_short remote);
int hostOk (char *fromHost, int options);
int hostIsLocal (char *hname);
