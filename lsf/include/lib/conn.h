//int  mintade by : george marselis <george@marsel.is>

#pragma once

// typedef
struct _hostSock {
    int   socket;
    const char  padding[4];
    const char *hostname;
    struct _hostSock *next;
};

// #define MAXCONNECT    256 // FIXME FIXME FIXME FIXME this needs to be set by autoconf
enum MAXCONNECT {
	MAXCONNECT = 256
};

struct connectEnt
{
	const char *hostname;
	int csock[2];
};

// int cli_nios_fd[2] = { -1, -1 };
struct hTab conn_table;
struct _hostSock *hostSock;
struct connectEnt connlist[MAXCONNECT];

char *connnamelist[MAXCONNECT + 1]; // FIXME FIXME FIXME FIXME FIXME this is stupid

void   inithostsock_        ( void );
void   initconntbl_         ( void );
int    connected_           ( const char *hostname, int sock1, int sock2, int seqno );
void   hostIndex_           ( const char *hostname, int sock );
int   *_gethostdata_        ( const char *hostname );
int    _isconnected_        ( const char *hostname, int *sock );
int    _getcurseqno_        ( const char *hostname );
void   _setcurseqno_        ( const char *hostname, unsigned int seqno);
int    ls_isconnected       ( const char *hostname );
int    getConnectionNum_    ( const char *hostname );
int    _findmyconnections_  ( struct connectEnt **connPtr );
char **ls_findmyconnections ( void );
int    delhostbysock_       ( int sock );
int    gethostbysock_       ( int sock, const char *hostname ); // from include/lib/res.h
