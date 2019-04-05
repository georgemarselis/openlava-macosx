// added by George Marselis <george@marsel.is>

#pragma once

// #include "lib/lib.h"

// extern int totsockets_;
// extern int currentsocket_;

// static int mLSFChanSockOpt = 0; // defined, but not used


/* sock.c */
unsigned int    CreateSock_(int protocol);
int             CreateSockEauth_(int protocol);
int             get_nonstd_desc_(int desc);
int             TcpCreate_(int service, int port);
int             io_nonblock_      ( int s );
int             io_block_         ( int s );
int             setLSFChanSockOpt_( int newOpt );
int             Socket_           ( int domain, int type, int protocol);
struct svrsock *svrsockCreate_    ( u_short port, int backlog, struct sockaddr_in *addr, int options);
int             svrsockAccept_    ( struct svrsock *svrsock, int timeout);
char           *svrsockToString_  ( struct svrsock *svrsock);
void            svrsockDestroy_   ( struct svrsock *svrsock);
int             TcpConnect_       ( const char *hostname, u_short port, struct timeval *timeout);
char           *getMsgBuffer_     ( int fd, size_t *bufferSize);
