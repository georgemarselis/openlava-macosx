// Added by George Marselis <george@marsel.is> Thu Apri 11 2019

#pragma once

#include "lib/lib.h"

/* #define SIGEMT SIGBUS */

char rexcwd_[1024]; // FIXME FIXME FIXME FIXME FIXME I have no idea what rxexcwd does

// static int short nios_ok_;
int short nios_ok_;
// extern int currentSN;


/* rtask.c */
int ls_rtask(const char *host, char **argv, int options);
int ls_rtaske(const char *host, char **argv, int options, char **envp);
int rgetpidCompletionHandler_(struct lsRequest *request);
void *lsRGetpidAsync_(pid_t taskid, pid_t *pid);
pid_t lsRGetpid_(pid_t taskid, int options);
int lsRGetpgrp_(int socket, pid_t taskid, pid_t pid);
void initSigHandler(int sig);
void default_tstp_(int signo);
unsigned short getTaskPort(int socket);
// void setRexWd_(const char *wd);