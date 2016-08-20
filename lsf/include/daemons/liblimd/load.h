
#pragma once

#include "daemons/liblimd/limd.h"

// #define NL_SETN 24

enum loadstruct
{
	e_vec, 
	e_mat 
};

float exchIntvl             = EXCHINTVL;
float sampleIntvl           = SAMPLINTVL;
short hostInactivityLimit   = HOSTINACTIVITYLIMIT;
short masterInactivityLimit = MASTERINACTIVITYLIMIT;
short resInactivityLimit    = RESINACTIVITYLIMIT;
short retryLimit            = RETRYLIMIT;
short keepTime = KEEPTIME;

time_t lastSbdActiveTime = 0;

char mustSendLoad = TRUE;

void sendLoad (void);
struct resPair *getResPairs (struct hostNode *hPtr);
void rcvLoad (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *hdr);
void rcvLoadVector (XDR * xdrs, struct sockaddr_in *from, struct LSFHeader *hdr);
void copyResValues (struct loadVectorStruct loadVector, struct hostNode *hPtr);
void copyIndices (float *lindx, int numIndx, int numUsrIndx, struct hostNode *hPtr);
float normalizeRq (float rawql, float cpuFactor, int nprocs);

