
#pragma once

#include "daemons/liblimd/limd.h"
#include "libint/resreq.h"

// #define NL_SETN 24
#define effectiveRq(nrq, factor) ((nrq) * (factor) -1)

#define ELIG_ALL     0x01
#define ELIG_NOLIMIT 0x02

#define SORT_CUT    0x01
#define SORT_FINAL  0x02
#define SORT_SINDX  0x04
#define SORT_INCR   0x08

#define P_(s) s

void placeReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint s);
int findBestHost P_ ((register struct resVal *, int, int, char **, int, char, int, int));
void jackup P_ ((int, struct hostNode *, float));
void loadAdj P_ ((struct jobXfer *, struct hostNode **, int, char));
void potentialOfCandidates P_ ((int, struct resVal *));
void potentialOfHost P_ ((struct hostNode *, struct resVal *));
void selectBestInstances P_ ((int, int, char, int));
int getNumInstances P_ ((int));
int getOkSites (int, int, int);
int findNPref (int, int, char **);
int bsort (int, int, int, int, float, char, int, int);
void mkexld (struct hostNode *, struct hostNode *, int, float *, float *, float);
int orderByStatus (int, int);
int grabHosts (struct hostNode *, struct resVal *, struct decisionReq *, int, char *, int);
void loadadjReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint s);

int addCandList (struct hostNode *, int);
int initCandList (void);

void loadReq (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *reqHdr, uint s);

void setBusyIndex (int, struct hostNode *);
float loadIndexValue (int, int, int);

float *extraload;
char jobxfer = 0;
struct hostNode **candidates = NULL;
int candListSize = 0;
struct hostNode *fromHostPtr;
