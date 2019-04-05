// Added by George Marselis <george@marsel.is> Fri Apr 5 2019

#pragma once

char *namestofilter_(char **indxs);
struct hostLoad *ls_loadinfo(char *resreq, size_t *numhosts, int options, const char *fromhost, char **hostlist, size_t listsize, char ***indxnamelist);
struct hostLoad *ls_load(char *resreq, size_t *numhosts, int options, const char *fromhost);
struct hostLoad *ls_loadofhosts(char *resreq, size_t *numhosts, int options, const char *fromhost, char **hostlist, size_t listsize);
struct hostLoad *loadinfo_(char *resReq, struct decisionReq *loadReqPtr, const char *fromhost, unsigned long *numHosts, char ***outnlist);
int ls_loadadj(char *resreq, struct placeInfo *placeinfo, size_t listsize);

