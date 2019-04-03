// Added by George Marselis <george@marsel.is> Wed Apri 3rd 2019

#pragma once



// #define ESUBNAME "esub"
// #define EEXECNAME "eexec"
// #define EGROUPNAME "egroup"

// #define NL_SETN   23 

/* esub.c */
int   runEsub_   (struct lenData *ed, char *path);
int   runEClient_(struct lenData *ed, char **argv);
int   getEData   (struct lenData *ed, char **argv, const char *lsfUserName);
int   runEexec_  (char *option, int job, struct lenData *eexec, const char *path);
char *runEGroup_ (char *type, char *gname);
