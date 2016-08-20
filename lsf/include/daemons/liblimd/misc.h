
#pragma once

/* LIM events
 * Keep a global FILE pointer to the
 * events file. The events file is always open
 * to speed up the operations.
 */
FILE *logFp;

void lim_Exit (const char *fname);
int equivHostAddr (struct hostNode *hPtr, u_int from);
struct hostNode *findHost (char *hostName);
struct hostNode *findHostbyList (struct hostNode *hList, char *hostName);
struct hostNode *findHostbyNo (struct hostNode *hList, int hostNo);
struct hostNode *findHostbyAddr (struct sockaddr_in *from, char *fname);
struct hostNode *findHNbyAddr (in_addr_t from);
struct hostNode *rmHost (struct hostNode *r);
bool_t findHostInCluster (char *hostname);
int definedSharedResource (struct hostNode *host, struct lsInfo *allInfo);
struct shortLsInfo *shortLsInfoDup (struct shortLsInfo *src);
void shortLsInfoDestroy (struct shortLsInfo *shortLInfo);
int logInit (void);
int logLIMStart (void);
int logLIMDown (void);
int logAddHost (struct hostEntry *hPtr);
int logRmHost (struct hostEntry *hPtr);
int loadEvents (void);



