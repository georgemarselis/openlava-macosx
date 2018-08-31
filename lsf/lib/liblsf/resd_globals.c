// Added by George Marselis <george@marsel.is>

#include <string.h>

#include "struct-config_param.h"
#include "lib/resd_globals.h"

// RESD

enum RES_PARAMS {
	RES_DEBUG                     = 0,
	RES_PORT                      = 6,    // default port: 36002
	RES_TIMEOUT                   = 20,   // default timeout: 
	RES_TIMING                    = 666,  // made up value
	RES_NULL
};

struct config_param resDaemonParams[ ] = {
	{ "RES_DEBUG",                NULL }, // 0
	{ "RES_PORT",              "36002" }, // 6   // default port: 36002 // FIXME FIXME FIXME FIXME insert into configure.ac
	{ "RES_TIMEOUT",              NULL }, // 20  // default timeout: 
	{ "RES_TIMING",               NULL }, // 666 // made up value 
	{ NULL,                       NULL }
};


enum resdParams_t {
	RES_PORT_DEFAULT = 36002 // FIXME FIXME FIXME FIXME FIXME make it configurable in configure.ac; 36002 by default
};
