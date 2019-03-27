// Added by George Marselis <george@marsel.is> March 27 2019

#pragma once


#if defined(DEBUG)
FILE *logfp;
char logfile[100]; // FIXME FIXME FIXME oddly specific
#endif

/* daemons/eauthd/eauth.c */
int main(int argc, char **argv);
int getAuth_lsb( char *inst );
int printUserName(void);
int vauth(char *lsfUserName, char *datBuf, int datLen)
