// Added by George Marselis <george@marsel.is> Mon Apr 15 2019

#pragma once

char **ls_placereq(char *resreq, size_t *numhosts, int options, char *fromhost);
char **ls_placeofhosts(char *resreq, size_t *numhosts, int options, char *fromhost, char **hostlist, size_t listsize);
char **placement_(char *resReq, struct decisionReq *placeReqPtr, char *fromhost, size_t *numhosts);
int ls_addhost(struct hostEntry *hPtr);
int ls_rmhost(const char *host);
