
#pragma once

// #define NL_SETN 25
int exitrc;
int fFlag;

int limCtrl     (int argc, char **argv, int opCode );
int limLock     (int argc, char **argv);
void doHosts    (int argc, char **argv, int opCode );
void doAllHosts (int opCode );
void operateHost(char *host, int opCode, int confirm );

