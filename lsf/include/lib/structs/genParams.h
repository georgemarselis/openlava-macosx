// Added by George Marselis <george@marsel.is> Apr 1st 2019


#pragma once 

#include "struct-config_param.h"

//*****************************************************************
// LSF

enum LSF_PARAMS {
    LSF_DEBUG                   = 0,
    LSF_SERVERDIR               = 1,
    LSF_ENVDIR                  = 2, // FIXME FIXME newly inserted variable
    LSF_LOGDIR                  = 3,
    LSF_RES_PORT                = 6,
    LSF_LOG_MASK                = 8, // or 19
    LSF_ID_PORT                 = 9,
    LSF_AUTH                    = 10,
    LSF_DEBUG_RES               = 11,
    LSF_TIME_RES                = 12,
    LSF_ROOT_REX                = 13,
    LSF_RES_RLIMIT_UNLIM        = 14,
    LSF_CMD_SHELL               = 15,
    LSF_ENABLE_PTY              = 16,
    LSF_TMPDIR                  = 17,
    LSF_BINDIR                  = 18, // or 20
    // LSF_LOG_MASK                = 19,
    LSF_RES_NO_LINEBUF          = 21,
    LSF_CONFDIR                 = 23,
    LSF_GETPWNAM_RETRY          = 28,
    LSF_AUTH_DAEMONS            = 33,
    LSF_LIBDIR                  = 40,
    LSF_MLS_LOG                 = 46,
    // LSF_LIM_DEBUG               = 2
    LSF_NO_HOSTS_FILE           = 124,
    LSF_RES_ACCTDIR             = 9,
    LSF_RES_ACCT                = 10,
    LSF_USE_HOSTEQUIV           = 122,
    LSF_AM_OPTIONS              = 123,
    LSF_RES_DEBUG               = 124,
    LSF_NULL
} LSF_STATUS;

static struct config_param genParams_[ ] = { // FIXME FIXME FIXME FIXME genParams order has to match with above enum LSF_LSB
    { "LSF_DEBUG",              NULL }, // 0
    { "LSF_SERVERDIR",          NULL }, // 1
    { "LSF_ENVDIR",             NULL }, // 2
    { "LSF_LOGDIR",             NULL }, // 3
    { "LSF_LIM_PORT",           NULL }, // 5
    { "LSF_RES_PORT",           NULL }, // 6
    { "LSF_LOG_MASK",           NULL }, // 8 // or 19
    { "LSF_ID_PORT",            NULL }, // 9
    { "LSF_AUTH",               NULL }, // 10
    { "LSF_USE_HOSTEQUIV",      NULL }, // 12
    { "LSF_ROOT_REX",           NULL }, // 13
    { "LSF_TMPDIR",             NULL }, // 17
    { "LSF_BINDIR",             NULL }, // 18 // or 20
    { "LSF_CONFDIR",            NULL }, // 23
    { "LSF_GETPWNAM_RETRY",     NULL }, // 28
    { "LSF_AUTH_DAEMONS",       NULL }, // 33
    { "LSF_LIBDIR",             NULL }, // 40
    { "LSF_MLS_LOG",            NULL }, // 46
    { "LSF_STRIP_DOMAIN",       NULL }, // N+1
    { "LSF_SERVER_HOSTS",       NULL }, // N+2
    { "LSF_API_CONNTIMEOUT",    NULL }, // N+3
    { "LSF_API_RECVTIMEOUT",    NULL }, // N+4
    { "LSF_AM_OPTIONS",         NULL }, // N+5
    { "LSF_SYMBOLIC_LINK",      NULL }, // N+6
    { "LSF_MASTER_LIST",        NULL }, // N+7
    { "LSF_INTERACTIVE_STDERR", NULL }, // N+8
    { "LSF_SHAREDIR",           NULL }, // N+9
    { "LSF_CONF_RETRY_INT",     NULL }, // N+10
    { "LSF_CONF_RETRY_MAX",     NULL }, // N+11
    { "LSF_HOSTS_FILE",         NULL }, // N+12 // renamed from HOSTS_FILE
    { "LSF_NO_HOSTS_FILE",      NULL }, // N+13 // renamed from NO_HOSTS_FILE
    { "LSF_RES_DEBUG",          NULL }, // N+14
    { NULL,                     NULL }
};

// static enum LSFDebugParams_t {
//     LSF_DEBUG_CMD,
//     LSF_TIME_CMD,
//     LSF_CMD_LOGDIR,
//     LSF_CMD_LOG_MASK,
//     LSF_LOG_MASK_DEBUG
// } debugParams_t;


// static struct config_param LSFDebugParams[] = {
//     { "LSF_DEBUG_CMD",      NULL },
//     { "LSF_TIME_CMD",       NULL },
//     { "LSF_CMD_LOGDIR",     NULL },
//     { "LSF_CMD_LOG_MASK",   NULL },
//     { "LSF_LOG_MASK_DEBUG", NULL },
//     { NULL,                 NULL }
// };

