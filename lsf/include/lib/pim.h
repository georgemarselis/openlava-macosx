// Added by George Marselis <george@marsel.is> Fri Apr 12 2019

#pragma once

size_t inNPGids;

/* pim.c */
struct jRusage *getJInfo_ (pid_t npgid, pid_t *pgid, unsigned short options, gid_t cpgid);
char *readPIMBuf( const char *pfile);
char *getNextString( const char *buf, char *string);
int readPIMFile( const char *pfile);
struct jRusage *readPIMInfo( pid_t inNPGids, pid_t *inPGid);
int inAddPList(struct lsPidInfo *pinfo);
int intoPidList(struct lsPidInfo *pinfo);
int pimPort(struct sockaddr_in *pimAddr, const char *pfile);
FILE *openPIMFile(const char *pfile);
