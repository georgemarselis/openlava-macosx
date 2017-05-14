
#pragma once

#include "daemons/liblimd/limout.h"

struct masterInfo masterInfo_;
int masterknown_ = 0; // FALSE

struct hostInfo *expandSHinfo (struct hostInfoReply *);
struct clusterInfo *expandSCinfo (struct clusterInfoReply *);
int copyAdmins_ (struct clusterInfo *, struct shortCInfo *);
int getname_ (enum limReqCode limReqCode, char *name, size_t namesize);
