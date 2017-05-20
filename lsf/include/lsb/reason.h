
#pragma once

char *getMsg (struct msgMap *msgMap, int *msg_ID, int number);
char *lsb_suspreason ( unsigned int reasons, unsigned int subreasons, struct loadIndexLog *ld);
void userIndexReasons (char *msgline_, int resource, unsigned int reason, struct loadIndexLog *ld);
void getMsgByRes (int resource, unsigned int reason, char **sp, struct loadIndexLog *ld);
