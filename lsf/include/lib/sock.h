// added by George Marselis <george@marsel.is>

#pragma once


int CreateSock_(int protocol);
int CreateSockEauth_(int protocol);
int get_nonstd_desc_(int desc);
int TcpCreate_(int service, int port);
int io_nonblock_(int s);
int io_block_(int s);
int setLSFChanSockOpt_(int newOpt);
int Socket_(int domain, int type, int protocol);
ls_svrsock_t *svrsockCreate_(u_short port, int backlog, struct sockaddr_in *addr, int options);
int svrsockAccept_(ls_svrsock_t *svrsock, int timeout);
char *svrsockToString_(ls_svrsock_t *svrsock);
void svrsockDestroy_(ls_svrsock_t *svrsock);
int TcpConnect_(char *hostname, u_short port, struct timeval *timeout);
char *getMsgBuffer_(int fd, size_t *bufferSize);
