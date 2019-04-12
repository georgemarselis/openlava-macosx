/* $Id: lib.pim.h 397 2007-11-26 19:04:00Z mblack $
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#pragma once
 
#include <limits.h>
#include <time.h>

#include "lsf.h"
#include "lib/syslog.h"
#include "struct-config_param.h"

// Added by George Marselis <george@marsel.is

// PIMD

static enum PIM_API {
    PIM_API_TREAT_JID_AS_PGID = 0x1,
    PIM_API_UPDATE_NOW        = 0x2,
    PIM_API_NULL
} PIM_API;

// #define PIM_SLEEP_TIME 3
// #define PIM_UPDATE_INTERVAL 30
static enum PIM_PARAMS {
    PIM_DEBUG             = -1,
    PIM_INFODIR           =  0,
    PIM_SLEEP_TIME        =  1, // or 3
    PIM_PORT              =  3, // made up value
    PIM_SLEEPTIME_UPDATE  =  4, // what is the difference between this and PIM_UPDATE_INTERVAL
    PIM_UPDATE_INTERVAL   =  30,
    PIM_NULL
} PIM_PARAMS;

static struct config_param pimDaemonParams[ ] = {
    { "PIM_DEBUG",                NULL }, // -1
    { "PIM_INFODIR",              NULL }, //  0
    { "PIM_SLEEP_TIME",           NULL }, //  1
    { NULL,                       NULL },
    { "PIM_PORT",                 NULL }, //  3 // made up value
    { "PIM_SLEEPTIME_UPDATE",     NULL }, //  4
    { "PIM_UPDATE_INTERVAL",      NULL }, //  30
    { NULL,                       NULL }
};


enum lsPStatType
{
    LS_PSTAT_RUNNING,
    LS_PSTAT_INTERRUPTIBLE,
    LS_PSTAT_UNINTERRUPTIBLE,
    LS_PSTAT_ZOMBI,
    LS_PSTAT_STOPPED,
    LS_PSTAT_SWAPPED,
    LS_PSTAT_SLEEP,
    LS_PSTAT_EXITING
};

struct lsPidInfo
{
    // char padding[4];
    enum lsPStatType status;
    char command[PATH_MAX]; // PATH_MAX is declared in limits.h
    pid_t pid;
    pid_t ppid;
    pid_t pgid;
    size_t proc_size;
    size_t resident_size;
    size_t stack_size;
    size_t jobid;
    time_t utime;
    time_t stime;
    time_t cutime;
    time_t cstime;
};

// const unsigned int NL_SETN = 32;     // FIXME FIXME remove at earliest convience

struct config_param pimParams[ ] = {
    { "LSF_DEBUG_PIM",           NULL },
    { "LSF_LIM_DEBUG",           NULL },
    { "LSF_LOGDIR",              NULL },
    { "LSF_LOG_MASK",            NULL },
    { "LSF_PIM_INFODIR",         NULL },
    { "LSF_PIM_NPROC",           NULL },
    { "PIM_SLEEP_TIME",          NULL },
    { "LSF_PIM_TRACE",           NULL },
    { "LSF_PIM_UPDATE_INTERVAL", NULL },
    { "LSF_TIME_PIM",            NULL },
    {  NULL,                     NULL }
};

enum
{
    LSF_LIM_DEBUG,
    LSF_LOGDIR,
    LSF_DEBUG_PIM,
    LSF_LOG_MASK,
    LSF_TIME_PIM,
    LSF_PIM_SLEEP_TIME, // FIXME FIME FIXME FIXME must reconsile and move out of here
    LSF_PIM_INFODIR,
    LSF_PIM_NPROC,
    LSF_PIM_TRACE,
    LSF_PIM_UPDATE_INTERVAL,
    LSF_PIM_NULL
} pimStatus;


int      gothup;
char     *pimInfoBuf =                    NULL;
int       argOptions =                    0;
int       numprocs   =                    0;
pid_t    *pgidList   =                    NULL;
pid_t     npgidList  =                    0;
pid_t     npidList   =                    0;
unsigned int         hitPGid             = 0;
unsigned int         npinfoList          = 0;
unsigned long        pimInfoLen          = 0;
unsigned short      *pimPortNo           = NULL;
struct   lsPidInfo  *pinfoList           = NULL;
struct   pidInfo    *pidList             = NULL;
struct   lsPidInfo   pbase[MAX_PROC_ENT];  // FIXME FIXME FIXME FIXME MAX_PROC_ENT, 
                                           // what is it depended on? 
                                           // Can this declaration be turned into a pointer?

int pim_debug = 0;
int sleepTime = PIM_SLEEP_TIME;
int updInterval = PIM_UPDATE_INTERVAL;


/*   pim.c  */
int  doServ         ( void   );
void logProcessInfo ( void   );
int  scan_procs     ( void   );
void hup            ( int    sig    );
void pimd_usage     ( const  char   *cmd         );
void updateProcs    ( const  time_t  lastUpdate );
int  ls_pidinfo     ( int     pid,   struct lsPidInfo *rec   );
int  parse_stat     ( char   *buf,   struct lsPidInfo *pinfo );
