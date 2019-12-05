// Added by George Marselis <george@marsel.is> Wed Apri 3rd 2019

#pragma once

#include "lib/header.h"

// #define ESUBNAME "esub"
// #define EEXECNAME "eexec"
// #define EGROUPNAME "egroup"

// #define NL_SETN   23 

/* esub.c */
unsigned int runEsub_(struct lenData *ed, const char *path);
unsigned int runEClient_(struct lenData *ed, char **argv);
bool getEData(struct lenData *ed, char **argv, const char *lsfUserName);
int runEexec_(char *option, unsigned int job, struct lenData *eexec, const char *path);
char *runEGroup_(const char *type, const char *gname);

