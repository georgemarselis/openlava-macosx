
#pragma once 

#include "lib/rdwr.h"
	// needed for static void alarmer_ (void);

#define LSF_AUTH            9     // FIXME FIXME replace defines with include resd.h
#define LSF_USE_HOSTEQUIV   10    //    FIXME FIXME but also re-adjust the entries from resdh.
#define LSF_ID_PORT         11    //        with the numbers from here
#define _USE_TCP_           0x04
//#define LOOP_ADDR           0x7F000001


int userok (int s, struct sockaddr_in *from, char *hostname, struct sockaddr_in *localAddr, struct lsfAuth *auth, int debug);
char *auth_user (u_long in, u_short local, u_short remote);
int hostOk (char *fromHost, int options);
int hostIsLocal (char *hname);
