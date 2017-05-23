

#pragma once


int changeUserEUId (void);
void parseAndDo (char *cmdBuf, int (*func)() );
int adminCmdIndex (char *cmd, char *cmdList[]);
void cmdsUsage (char *cmd, char *cmdList[], char *cmdInfo[]);
void oneCmdUsage (int i, char *cmdList[], char *cmdSyntax[]);
void cmdHelp (int argc, char **argv, char *cmdList[], char *cmdInfo[],  char *cmdSyntax[]);
char *myGetOpt (int nargc, char **nargv, char *ostr);
int getConfirm (char *msg);
int checkConf (int verbose, int who);
int changeUserEUId (void);
