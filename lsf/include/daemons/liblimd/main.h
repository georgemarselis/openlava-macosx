
#pragma once

int getClusterConfig (void);
void initMiscLiStruct (void);
void printTypeModel (void);
void initLiStruct (void);
void startPIM (int argc, char **argv);
struct tclLsInfo *getTclLsInfo (void);
void errorBack (struct sockaddr_in *from, struct LSFHeader *reqHdr, enum limReplyCode replyCode, int chan);
int initSock (int checkMode);
void lim_child_handler (int sig);
void lim_term_handler (int signum);
void periodic (int kernelPerm);
int initAndConfig (int checkMode, int *kernelPerm);
void initSignals (void);
void doAcceptConn (void);
int processUDPMsg (void);
void usage (void);

struct clientNode **clientMap = NULL; // FIXME FIXME FIXME FIXME FIXME this needs memory management and/or debug
