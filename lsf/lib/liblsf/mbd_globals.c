// added by George Marselis <george@marsel.is>

// / MDB

#include <stdio.h>

#include "struct-config_param.h"


enum MDB_PARAMS {
	MBD_DEBUG                         = 0,
	MBD_PORT                          = 3,
	MBD_DONT_FORK                     = 54,
	MBD_TIMING                        = 666, // made up value
	MBD_NULL
};

struct config_param mbdDaemonParams[ ] = {
	{ "MBD_DEBUG",                    NULL }, // 0	
	{ "MBD_PORT",                     NULL }, // 3   // default port: 
	{ "MBD_DONT_FORK",                NULL }, // 54
	{ "MBD_TIMING",                   NULL }, // 666 // made up value
	{ NULL,                           NULL }
};
