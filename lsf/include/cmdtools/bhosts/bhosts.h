// added by George Marselis <george@marsel.is

#pragma once


// #define HOST_NAME_LENGTH    18
// #define HOST_STATUS_LENGTH  14
// #define HOST_STATUS_SHORT   11
// #define HOST_JL_U_LENGTH    5
// #define HOST_MAX_LENGTH     6
// #define HOST_NJOBS_LENGTH   6
// #define HOST_RUN_LENGTH     6
// #define HOST_SSUSP_LENGTH   6
// #define HOST_USUSP_LENGTH   6
// #define HOST_RSV_LENGTH     6
// #define HOST_CPUF_LENGTH    6

enum HOSTLENGTHS {
	HOST_NAME_LENGTH   = 18,
	HOST_STATUS_LENGTH = 14,
	HOST_STATUS_SHORT  = 11,
	HOST_JL_U_LENGTH   = 5,
	HOST_MAX_LENGTH    = 6,
	HOST_NJOBS_LENGTH  = 6,
	HOST_RUN_LENGTH    = 6,
	HOST_SSUSP_LENGTH  = 6,
	HOST_USUSP_LENGTH  = 6,
	HOST_RSV_LENGTH    = 6,
	HOST_CPUF_LENGTH   = 6
};

// extern int _lsb_recvtimeout;

// #define ALL_HOSTS "ALLHOSTS"
const char ALL_HOSTS[ ] = "ALLHOSTS";

// extern int lsbSharedResConfigured_;


// #define NL_SETN 8



struct indexFmt
{
	char *name;
	char *hdr;
	char *busy;
	char *ok;
	float scale;
	// unsigned int dispLen;
	unsigned short dispLen;
	char *normFmt;
	char *expFmt;
};


// #define  DEFAULT_FMT 11
const unsigned int DEFAULT_FMT = 11;


struct indexFmt fmt1[] = {
	{ "r15s", "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g"   },
	{ "r1m",  "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g"   },
	{ "r15m", "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g"   },
	{ "ut",   "%6s", "*%4.0", "%6.0", 100.0, 6, "f%%", "g%%" },
	{ "pg",   "%6s", "*%4.1", "%5.1", 1.0,   6, "f",   "g"   },
	{ "io",   "%6s", "*%4.0", "%4.0", 1.0,   4, "f",   "g"   },
	{ "ls",   "%5s", "*%2.0", "%3.0", 1.0,   3, "f",   "g"   },
	{ "it",   "%6s", "*%4.0", "%5.0", 1.0,   5, "f",   "g"   },
	{ "tmp",  "%6s", "*%3.0", "%4.0", 1.0,   5, "fM",  "fG"  },
	{ "swp",  "%6s", "*%3.0", "%4.0", 1.0,   5, "fM",  "fG"  },
	{ "mem",  "%6s", "*%4.0", "%5.0", 1.0,   5, "fM",  "fG"  },
	{ "dflt", "%7s", "*%6.1", "%6.1", 1.0,   7, "f",   "g"   },
	{ NULL,   "%7s", "*%6.1", "%6.1", 1.0,   7, "f",   "g"   }
};

struct indexFmt *fmt;

char *defaultindex[] = { 
	"r15s", "r1m", "r15m", "ut", "pg", "ls", "it", "tmp", "swp", "mem", NULL // FIXME FIXME FIXME FIXME "dflt" is missing here
};

struct lsInfo *lsInfoPtr;

// #define MAXFIELDSIZE 80
const unsigned int MAXFIELDSIZE = 80;

char wflag = FALSE;
char fomt[200];                    // FIXME FIXME FIXME FIXME 200 chars is awfully specific
int nameToFmt (char *indx);


/* cmdtools/bhosts/bhosts.c */
void    usage            ( const char *cmd );
void    prtHostsLong     ( int numReply,              struct hostInfoEnt *hInfo );
void    prtHostsShort    ( int numReply,              struct hostInfoEnt *hInfo );
void    sort_host        ( int replyNumHosts,         struct hostInfoEnt *hInfo );
int     repeatHost       ( int currentNum,            struct hostInfoEnt *hInfo );
int     getDispLastItem  ( char **dispIndex,          int start, int last );
void    prtLoad          ( struct hostInfoEnt *hPtrs, struct lsInfo *lsInfo );
int     nameToFmt        ( char *indx );
char   *formatHeader     ( char **dispindex,          int start, int end );
char   *stripSpaces      ( char *field );
int     makeFields       ( struct hostInfoEnt *host,  char *loadval[], char **dispindex, int option);
char  **formLINamesList  ( struct lsInfo *lsInfo );
float   getLoad          ( char *dispindex,           float *loads, int *index );
void    getCloseString   ( int hStatus,               char **status );
void    displayShareRes  ( int argc,                  char **argv, int index );
void    prtResourcesShort( int num,                   struct lsbSharedResourceInfo *info );
void    prtOneInstance   ( char *resName,             struct lsbSharedResourceInstance *instance );
int     makeShareFields  ( char *hostname,            struct lsInfo *lsInfo, char ***nameTable, char ***totalValues, char ***rsvValues, char ***formatTable);
