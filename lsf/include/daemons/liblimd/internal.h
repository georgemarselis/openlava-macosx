
#pragma once

void sndConfInfo (struct sockaddr_in *);
void masterRegister (XDR * xdrs,	struct sockaddr_in *from, struct LSFHeader *reqHdr);
void announceElimInstance (struct clusterNode *clPtr);
void announceMaster (struct clusterNode *clPtr, char broadcast, char all);
void jobxferReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr);
void wrongMaster (struct sockaddr_in *from, char *buf, struct LSFHeader *reqHdr, int s);
void initNewMaster (void);
void rcvConfInfo (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *hdr);
void checkHostWd (void);
void announceMasterToHost (struct hostNode *hPtr, int infoType);
int probeMasterTcp (struct clusterNode *clPtr);
