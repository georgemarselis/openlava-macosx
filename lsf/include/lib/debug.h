/*
 * wonderfully and fully automated heading goes here
 */

#pragma once

#include "struct-config_param.h"

enum debparams {
    LSF_CMD_LOG_MASK,
    LSF_LOG_MASK,
    LSF_CMD_LOGDIR,
    LSF_DEBUG_CMD,
    LSF_TIME_CMD,
    DEBPARAMS_NULL
};

struct config_param debParams[ ] = {
    { "LSF_CMD_LOG_MASK",  NULL },
    { "LSF_LOG_MASK",      NULL },
    { "LSF_CMD_LOGDIR",    NULL },
    { "LSF_DEBUG_CMD",     NULL },
    { "LSF_TIME_CMD",      NULL },
    { NULL,    NULL }
};

int lsb_debugReq( struct debugReq *pdebug, char *host );
