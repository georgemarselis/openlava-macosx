
#pragma once

#include "daemons/liblimd/limout.h" // FIXME FIXME FIXME FIXME de-couple fhe structs used for limd from limd functionality


struct sharedResource **hostResources = NULL;


struct masterInfo masterInfo_;
int masterknown_ = 0; // FALSE

/* info.c */
char *ls_getclustername(void);
unsigned int expandList1_(char ***tolist, int num, int *bitmMaps, char **keys);
unsigned int expandList_(char ***tolist, int mask, char **keys);
int copyAdmins_(struct clusterInfo *clusPtr, struct shortCInfo *clusShort);
struct clusterInfo *expandSCinfo(struct clusterInfoReply *clusterInfoReply);
struct clusterInfo *ls_clusterinfo(char *resReq, unsigned int *numclusters, char **clusterList, int listsize, int options);
char               *ls_getmastername(void);
char               *ls_getmastername2(void);
int                 getname_(enum limReqCode limReqCode, char *name, size_t namesize);
char               *ls_gethosttype    ( const char *hostname  );
char               *ls_gethostmodel   ( const char *hostname  );
double             *ls_gethostfactor  ( const char *hostname  );
double             *ls_getmodelfactor ( const char *modelname );
struct    hostInfo *expandSHinfo(struct hostInfoReply *hostInfoReply);
struct    hostInfo *ls_gethostinfo( char *resReq, size_t *numhosts, const char **hostlist, size_t listsize, int options);
struct      lsInfo *ls_info(void);
char              **ls_indexnames(struct lsInfo *lsInfo);
int ls_isclustername( const char *name );
struct lsSharedResourceInfo *ls_sharedresourceinfo(char **resources, unsigned int *numResources, const char *hostName, int options);
