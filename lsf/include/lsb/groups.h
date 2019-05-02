// Added by George Marselis <george@marsel.is> May 2nd 2019

#pragma once

/* lib/liblsbatch/groups.c */
struct groupInfoEnt *lsb_usergrpinfo(char **groups, unsigned int *numGroups, int options);
struct groupInfoEnt *lsb_hostgrpinfo(char **groups, unsigned int *numGroups, int options);
struct groupInfoEnt *getGrpInfo(char **groups, unsigned int *numGroups, int options);
int sendGrpReq(char *clusterName, int options, struct infoReq *groupInfo, struct groupInfoReply *reply);
void freeGroupInfoReply(struct groupInfoReply *reply);
