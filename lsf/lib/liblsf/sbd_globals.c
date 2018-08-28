// Added by George Marselis <george@marsel.is>

// SDB

enum SDB_PARAMS {
	SBD_DEBUG                         = 0,
	SBD_PORT                          = 3,
	SBD_DONT_FORK                     = 54,
	SBD_TIMING                        = 666,
	SBD_NULL
};

static struct config_param sbdDaemonParams[ ] = {
	{ "SBD_DEBUG",                    NULL }, // 0
	{ NULL,                           NULL },
	{ NULL,                           NULL },
	{ "SBD_PORT",                     NULL }, // 3
	{ "SBD_DONT_FORK",                NULL }, // 54
	{ "SBD_TIMING",                   NULL }, // 666 // made up value
	{ NULL,                           NULL }
};
