// Added by George Marselis <george@marsel.is> Wed Mar 27 2019

#pragma once

#include "lib/common_structs.h"

/* control.c */
int ls_limcontrol(const char *hname, int opCode);
int ls_lockhost(time_t duration);
int ls_unlockhost(void);
int lockHost_(time_t duration, const char *hname);
int unlockHost_( const char *hname);
int setLockOnOff_(int on_, time_t duration, const char *hname);
int oneLimDebug(struct debugReq *pdebug, const char *hostname);
int ls_servavail(int servId, int nonblock);
