// Added by George Marselis <george@marsel.is> Thu Apri 11 2019

#pragma once

#include "lib/lib.h"

/* #define SIGEMT SIGBUS */

static int short nios_ok_;
// extern int currentSN;


/* rtask.c */
int ls_rtaske(char *host, char **argv, int options, char **envp);
int rgetpidCompletionHandler_(struct lsRequest *request);
void *lsRGetpidAsync_(int taskid, int *pid);
int lsRGetpid_(int taskid, int options);
int lsRGetpgrp_(int sock, int taskid, pid_t pid);
void initSigHandler(int sig);
int ls_rtask(char *host, char **argv, int options);
void default_tstp_(int signo);
unsigned short getTaskPort(int s);
void setRexWd_(char *wd);
