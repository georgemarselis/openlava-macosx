// Added by George Marselis <george@marsel.is> 

// PIMD
#include <stddef.h>
#include <struct-config_param.h>


enum PIM_API {
	PIM_API_TREAT_JID_AS_PGID = 0x1,
	PIM_API_UPDATE_NOW        = 0x2,
	PIM_API_NULL
} PIM_API;

// #define PIM_SLEEP_TIME 3
// #define PIM_UPDATE_INTERVAL 30
enum PIM_PARAMS {
	PIM_DEBUG             = -1,
	PIM_INFODIR           =  0,
	PIM_SLEEP_TIME        =  1, // or 3
	PIM_PORT              =  3, // made up value
	PIM_SLEEPTIME_UPDATE  =  4, // what is the difference between this and PIM_UPDATE_INTERVAL
	PIM_UPDATE_INTERVAL   =  30,
	PIM_NULL
} PIM_PARAMS;

struct config_param pimDaemonParams[ ] = {
	{ "PIM_DEBUG",                NULL }, // -1
	{ "PIM_INFODIR",              NULL }, //  0
	{ "PIM_SLEEP_TIME",           NULL }, //  1
	{ NULL,                       NULL },
	{ "PIM_PORT",                 NULL }, //  3 // made up value
	{ "PIM_SLEEPTIME_UPDATE",     NULL }, //  4
	{ "PIM_UPDATE_INTERVAL",      NULL }, //  30
	{ NULL,                       NULL }
};
