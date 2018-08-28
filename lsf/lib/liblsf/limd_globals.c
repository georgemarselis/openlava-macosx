// added by George Marselis <george@marsel.is>

// LIMD

enum LIM_PARAMS {
	LIM_DEBUG                         = 0,
	LIM_PORT                          = 5,
	LIM_TIMING                        = 6,
	LIM_NO_MIGRANT_HOSTS              = 55,
	LIM_NULL
};

static struct config_param limDaemonParams[ ] = {
	{ "LIM_DEBUG",                    NULL }, // 0
	{ "LIM_PORT",                     NULL }, // 5   // default port: 
	{ "LIM_TIMING",                   NULL }, // 6 
	{ "LIM_NO_MIGRANT_HOSTS",         NULL }, // 55
	{ NULL,                           NULL }
};

// static enum limParams_t {
// 	LIM_DEBUG,
// // #define LIM_PORT        36000 // FIXME FIXME FIXME FIXME FIXME set appropriate configuration variable in configure.ac
// // #define RES_PORT        36002 // FIXME FIXME FIXME FIXME FIXME set appropriate configuration variable in configure.ac 
// 	LIM_PORT, // FIXME FIXME FIXME FIXME FIXME set appropriate configuration variable in configure.ac; 3600 by default
// 	LIM_TIME,
// 	LIM_IGNORE_CHECKSUM,
// 	LIM_JACKUP_BUSY,
// 	LIM_COMPUTE_ONLY,
// 	// LIM_NO_MIGRANT_HOSTS,
// 	LIM_NO_FORK,
// 	LSF_DEBUG_LIM,
// 	LSF_TIME_LIM,
// 	LIM_RSYNC_CONFIG
// } limParams_t;


// static struct config_param limParams[] = {
// 	{ "LIM_DEBUG",            NULL },
// 	{ "LIM_PORT",             NULL },
// 	{ "LIM_TIME",             NULL },
// 	{ "LIM_IGNORE_CHECKSUM",  NULL },
// 	{ "LIM_JACKUP_BUSY",      NULL },
// 	{ "LIM_COMPUTE_ONLY",     NULL },
// 	{ "LIM_NO_MIGRANT_HOSTS", NULL },
// 	{ "LIM_NO_FORK",          NULL },
// 	{ "LSF_DEBUG_LIM",        NULL },
// 	{ "LSF_TIME_LIM",         NULL },
// 	{ "LIM_RSYNC_CONFIG",     NULL },
// 	{ NULL,                   NULL },
// };
