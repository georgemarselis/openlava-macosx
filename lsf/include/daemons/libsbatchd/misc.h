// Added by George Marselis <george@marsel.is> March 2019

#pragma once


// #ifndef BSIZE
// #define BSIZE 1024 // FIXME FIXME FIXME FIXME FIXME 
// #endif

static const unsigned int BSIZE = 1024;

// #define NL_SETN         11

/* daemons/sbatchd/misc.c */
void milliSleep(int msec);
char window_ok(struct jobCard *jobPtr);
void shout_err(struct jobCard *jobPtr, char *msg);
void child_handler(int sig);
int fcp(char *file1, char *file2, struct hostent *hp);
int rmDir(char *dir);
void closeBatchSocket(void);
void getManagerId(struct sbdPackage *sbdPackage);
