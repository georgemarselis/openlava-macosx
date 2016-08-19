
#pragma once

#include "lib/lib.h"

void clientIO (struct Masks *chanmasks);
void processMsg (unsigned int chfd);
void clientReq (XDR *xdrs, struct LSFHeader *hdr, unsigned int chfd);
void shutDownChan (unsigned int chfd);
void processMsg (unsigned int chfd);
void shutDownChan (unsigned int chfd);

