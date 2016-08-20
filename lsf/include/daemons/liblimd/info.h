
#pragma once

#include "daemons/liblimd/limd.h"

// #define NL_SETN 24

struct shortLsInfo oldShortInfo;


void pingReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr);
void clusInfoReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr);
void clusNameReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr);
void masterInfoReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr);
void hostInfoReq (XDR * xdrs, struct hostNode *fromHostP, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint chfd);
void infoReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint s);
struct shortLsInfo *getCShortInfo (struct LSFHeader *);
int checkResources (struct resourceInfoReq *, struct resourceInfoReply *, int *);
int copyResource (struct resourceInfoReply *, struct sharedResource *, int *, char *);
void freeResourceInfoReply (struct resourceInfoReply *);
void resourceInfoReq (XDR *xdr, struct sockaddr_in *clientMap, struct LSFHeader *hdr, uint chfd);


