// added by George Marselis <george@marsel.is>

#pragma once

// #define NL_SETN		10


void addSharedResource (struct lsSharedResourceInfo *);
void addInstances (struct lsSharedResourceInfo *, struct sharedResource *);
void freeHostInstances (void);
void initHostInstances (int);
int copyResource (struct lsbShareResourceInfoReply *, struct sharedResource *, char *);

void initQPRValues (struct qPRValues *, struct qData *);
void freePreemptResourceInstance (struct preemptResourceInstance *);
void freePreemptResource (struct preemptResource *);
int addPreemptableResource (int);
struct preemptResourceInstance *findPRInstance (int, struct hData *);
struct qPRValues *findQPRValues (int, struct hData *, struct qData *);
struct qPRValues *addQPRValues (int, struct hData *, struct qData *);
float roundFloatValue (float);

float checkOrTakeAvailableByPreemptPRHQValue (int index, float value, struct hData *hPtr, struct qData *qPtr, int update);

struct objPRMO *pRMOPtr = NULL;