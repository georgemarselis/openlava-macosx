//int  mintade by : george marselis <george@marsel.is>

#pragma once

// #define MAXCONNECT    256 // FIXME FIXME FIXME FIXME this needs to be set by autoconf
enum MAXCONNECT {
	MAXCONNECT = 256
};

struct connectEnt
{
	const char *hostname;
	int csock[2];
};

void   inithostsock_        ( void );
void   initconntbl_         ( void );
int    connected_           ( const char *hostname, int sock1, int sock2, int seqno );
void   hostIndex_           ( const char *hostname, int sock );
int   *_gethostdata_        ( const char *hostname );
int    _isconnected_        ( const char *hostname, int *sock );
int    connected_           ( const char *hostname, int sock1, int sock2, int seqno );
int    _getcurseqno_        ( const char *hostname );
void   _setcurseqno_        ( const char *hostname, unsigned int seqno);
int    ls_isconnected       ( const char *hostname );
int    getConnectionNum_    ( const char *hostname );
int    _findmyconnections_  ( struct connectEnt **connPtr );
char **ls_findmyconnections ( void );
void   hostIndex_           ( const char *hostname, int sock );
int    delhostbysock_       ( int sock );
int    gethostbysock_       ( int sock, const char *hostname );
