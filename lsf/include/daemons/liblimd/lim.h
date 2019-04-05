// Added by George Marselis <george@marsel.is>  Mar 31st 2019

#pragma once

// #define MAXMSGLEN  32*MSGSIZE
// #define CONNECT_TIMEOUT 3
// #define RECV_TIMEOUT    3

const size_t MAXMSGLEN             = 32 * MSGSIZE; // FIXME FIXME FIXME FIXME why 32 times?
const unsigned int CONNECT_TIMEOUT = 3;
const unsigned int RECV_TIMEOUT    = 3;

// static int conntimeout_ = CONNECT_TIMEOUT;
// static int recvtimeout_ = RECV_TIMEOUT;

// static unsigned int localAddr = 0;

int lsf_lim_version = -1;

struct sockaddr_in sockIds_[4]; // FIXME FIXME FIXME FIXME 4 is very particular
unsigned int limchans_[4];      // FIXME FIXME FIXME FIXME 4 is very particular: 
								// limchans_[TCP], limchans_[PRIMARY], limchans_[MASTER], limchans_[UNBOUND]

/* daemons/limd/lim.c */
int callLim_      ( enum limReqCode reqCode, void *dsend, bool_t (*xdr_sfunc )(), void *drecv, bool_t (*xdr_rfunc )(), const char *host, int options, struct LSFHeader *hdr );
int callLimTcp_   ( char *reqbuf, char **rep_buf, int req_size, struct LSFHeader *replyHdr, int options );
int callLimUdp_   ( char *reqbuf, char *repbuf, int len, struct LSFHeader *reqHdr, const char *host, int options );
int createLimSock_( struct sockaddr_in *connaddr );
int initLimSock_  ( void );
int rcvreply_     ( int sock, char *rep );
void err_return_  ( enum limReplyCode limReplyCode );
