// Added by George Marselis <george@marsel.is>

#include <stdio.h>

#include "struct-config_param.h"

enum LSB_PARAMS {
	LSB_DEBUG                   = 0,
	LSB_CONFDIR                 = 1,
	LSB_SHAREDIR                = 4,
	LSB_MAILTO                  = 5,
	LSB_MAILPROG                = 6,
	LSB_MBD_PORT                = 8,
	LSB_SBD_PORT                = 7,

	LSB_CRDIR                   = 11,
	LSB_DEBUG_MBD               = 14,
	LSB_DEBUG_SBD               = 15,
	LSB_TIME_MBD                = 16,
	LSB_TIME_SBD                = 17,
	LSB_SIGSTOP                 = 18,

	LSB_MBD_CONNTIMEOUT         = 21,
	LSB_SBD_CONNTIMEOUT         = 22,
	LSB_MBD_MAILREPLAY          = 24,
	LSB_MBD_MIGTOPEND           = 25,
	LSB_SBD_READTIMEOUT         = 26,
	LSB_MBD_BLOCK_SEND          = 27,
	LSB_MEMLIMIT_ENFORCE        = 29,

	LSB_BSUBI_OLD               = 30,
	LSB_STOP_IGNORE_IT          = 31,
	LSB_HJOB_PER_SESSION        = 32,
	LSB_REQUEUE_HOLD            = 34,
	LSB_SMTP_SERVER             = 35,
	LSB_MAILSERVER              = 36,
	LSB_MAILSIZE_LIMIT          = 37,
	LSB_REQUEUE_TO_BOTTOM       = 38,
	LSB_ARRAY_SCHED_ORDER       = 39,

	LSB_QPOST_EXEC_ENFORCE      = 41,
	LSB_MIG2PEND                = 42,
	LSB_UTMP                    = 43,
	LSB_JOB_CPULIMIT            = 44,
	LSB_RENICE_NEVER_AT_RESTART = 45,
	LSB_JOB_MEMLIMIT            = 47,
	LSB_MOD_ALL_JOBS            = 48,
	LSB_SET_TMPDIR              = 49,

	LSB_PTILE_PACK              = 50,
	LSB_SBD_FINISH_SLEEP        = 51,
	LSB_VIRTUAL_SLOT            = 52,
	LSB_STDOUT_DIRECT           = 53,

	// found around the code base
	LSB_DEBUG_CMD               = 4,
	LSB_CMD_LOG_MASK            = 7,
	LSB_API_CONNTIMEOUT         = 9,
	LSB_API_RECVTIMEOUT         = 10,
	LSB_SERVERDIR               = 11,
	LSB_MODE                    = 12,
	LSB_SHORT_HOSTLIST          = 13,
	LSB_INTERACTIVE_STDERR      = 14,
	LSB_32_PAREN_ESC            = 15,
	LSB_API_QUOTE_CMD           = 14, // or 16

	LSB_NULL = 999

} LSB_PARAMS;


struct config_param lsbParams[ ] = {
	{ "LSB_DEBUG",                   NULL }, // 0
	{ "LSB_CONFDIR",                 NULL }, // 1
	{ "LSB_SHAREDIR",                NULL }, // 4
	{ "LSB_MAILTO",                  NULL }, // 5
	{ "LSB_MAILPROG",                NULL }, // 6
	{ "LSB_MBD_PORT",                NULL }, // 8
	{ "LSB_SBD_PORT",                NULL }, // 7

	{ "LSB_CRDIR",                   NULL }, // 11
	{ "LSB_DEBUG_MBD",               NULL }, // 14
	{ "LSB_DEBUG_SBD",               NULL }, // 15
	{ "LSB_TIME_MBD",                NULL }, // 16
	{ "LSB_TIME_SBD",                NULL }, // 17
	{ "LSB_SIGSTOP",                 NULL }, // 18

	{ "LSB_MBD_CONNTIMEOUT",         NULL }, // 21
	{ "LSB_SBD_CONNTIMEOUT",         NULL }, // 22
	{ "LSB_MBD_MAILREPLAY",          NULL }, // 24
	{ "LSB_MBD_MIGTOPEND",           NULL }, // 25
	{ "LSB_SBD_READTIMEOUT",         NULL }, // 26
	{ "LSB_MBD_BLOCK_SEND",          NULL }, // 27
	{ "LSB_MEMLIMIT_ENFORCE",        NULL }, // 29

	{ "LSB_BSUBI_OLD",               NULL }, // 30
	{ "LSB_STOP_IGNORE_IT",          NULL }, // 31
	{ "LSB_HJOB_PER_SESSION",        NULL }, // 32
	{ "LSB_REQUEUE_HOLD",            NULL }, // 34
	{ "LSB_SMTP_SERVER",             NULL }, // 35
	{ "LSB_MAILSERVER",              NULL }, // 36
	{ "LSB_MAILSIZE_LIMIT",          NULL }, // 37
	{ "LSB_REQUEUE_TO_BOTTOM",       NULL }, // 38
	{ "LSB_ARRAY_SCHED_ORDER",       NULL }, // 39

	{ "LSB_QPOST_EXEC_ENFORCE",      NULL }, // 41
	{ "LSB_MIG2PEND",                NULL }, // 42
	{ "LSB_UTMP",                    NULL }, // 43
	{ "LSB_JOB_CPULIMIT",            NULL }, // 44
	{ "LSB_RENICE_NEVER_AT_RESTART", NULL }, // 45
	{ "LSB_JOB_MEMLIMIT",            NULL }, // 47
	{ "LSB_MOD_ALL_JOBS",            NULL }, // 48
	{ "LSB_SET_TMPDIR",              NULL }, // 49

	{ "LSB_PTILE_PACK",              NULL }, // 50
	{ "LSB_SBD_FINISH_SLEEP",        NULL }, // 51
	{ "LSB_VIRTUAL_SLOT",            NULL }, // 52
	{ "LSB_STDOUT_DIRECT",           NULL }, // 53

	// doubly-assigned values or newly found values
	{ "LSB_DEBUG_CMD",               NULL }, // 4  
	{ "LSB_TIME_CMD",                NULL }, // 5
	{ "LSB_CMD_LOGDIR",              NULL }, // 6
	{ "LSB_LOG_MASK",                NULL }, // ? FIXME FIXME FIXME FIXME
	{ "LSB_CMD_LOG_MASK",            NULL }, // 7 
	{ "LSB_API_CONNTIMEOUT",         NULL }, // 9
	{ "LSB_API_RECVTIMEOUT",         NULL }, // 10
	{ "LSB_SERVERDIR",               NULL }, // 11
	{ "LSB_MODE",                    NULL }, // 12
	{ "LSB_SHORT_HOSTLIST",          NULL }, // 13
	{ "LSB_API_QUOTE_CMD",           NULL }, // 14 // or 16
	{ "LSB_32_PAREN_ESC",            NULL }, // 15

	{ NULL,                          NULL }
};
