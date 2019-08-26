/* $Id: lsb.sig.h 397 2007-11-26 19:04:00Z mblack $
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


// #define SIG_NULL             -65535
// #define SIG_CHKPNT               -1
// #define SIG_CHKPNT_COPY          -2
// #define SIG_DELETE_JOB           -3

// #define  SIG_SUSP_USER           -4
// #define  SIG_SUSP_LOAD           -5
// #define  SIG_SUSP_WINDOW         -6
// #define  SIG_SUSP_OTHER          -7

// #define  SIG_RESUME_USER         -8
// #define  SIG_RESUME_LOAD         -9
// #define  SIG_RESUME_WINDOW       -10
// #define  SIG_RESUME_OTHER        -11

// #define  SIG_TERM_USER           -12
// #define  SIG_TERM_LOAD           -13
// #define  SIG_TERM_WINDOW         -14
// #define  SIG_TERM_OTHER          -15
// #define  SIG_TERM_RUNLIMIT       -16
// #define  SIG_TERM_DEADLINE       -17
// #define  SIG_TERM_PROCESSLIMIT   -18
// #define  SIG_TERM_FORCE          -19
// #define  SIG_KILL_REQUEUE        -20
// #define  SIG_TERM_CPULIMIT       -21
// #define  SIG_TERM_MEMLIMIT       -22
// #define  SIG_ARRAY_REQUEUE       -23

// int sigNameToValue_ (char *sigString);
// char *getLsbSigSymbol (int sigValue);
// int getDefSigValue_ (int sigValue, char *actCmd);
// int isSigTerm (int sigValue);
// int isSigSusp (int sigValue);
// int terminateWhen_ (int *sigMap, char *name);



// int lsbSig_map[] = {
// enum lsbSig_map {
//     SIG_NULL,
//     SIG_CHKPNT,
//     SIG_CHKPNT_COPY,
//     SIG_DELETE_JOB,

//     SIG_SUSP_USER,
//     SIG_SUSP_LOAD,
//     SIG_SUSP_WINDOW,
//     SIG_SUSP_OTHER,

//     SIG_RESUME_USER,
//     SIG_RESUME_LOAD,
//     SIG_RESUME_WINDOW,
//     SIG_RESUME_OTHER,

//     SIG_TERM_USER,
//     SIG_TERM_LOAD,
//     SIG_TERM_WINDOW,
//     SIG_TERM_OTHER,
//     SIG_TERM_RUNLIMIT,
//     SIG_TERM_DEADLINE,
//     SIG_TERM_PROCESSLIMIT,
//     SIG_TERM_FORCE,
//     SIG_KILL_REQUEUE,
//     SIG_TERM_CPULIMIT,
//     SIG_TERM_MEMLIMIT
// };

enum SIG_LSF {
    SIG_NULL              = 0,
    SIG_CHKPNT            = 1,
    SIG_CHKPNT_COPY       = 2,
    SIG_DELETE_JOB        = 3,

    SIG_SUSP_USER         = 4,
    SIG_SUSP_LOAD         = 5,
    SIG_SUSP_WINDOW       = 6,
    SIG_SUSP_OTHER        = 7,

    SIG_RESUME_USER       = 8,
    SIG_RESUME_LOAD       = 9,
    SIG_RESUME_WINDOW     = 10,
    SIG_RESUME_OTHER      = 11,

    SIG_TERM_USER         = 12,
    SIG_TERM_LOAD         = 13,
    SIG_TERM_WINDOW       = 14,
    SIG_TERM_OTHER        = 15,
    SIG_TERM_RUNLIMIT     = 16,
    SIG_TERM_DEADLINE     = 17,
    SIG_TERM_PROCESSLIMIT = 18,
    SIG_TERM_FORCE        = 19,
    SIG_KILL_REQUEUE      = 20,
    SIG_TERM_CPULIMIT     = 21,
    SIG_TERM_MEMLIMIT     = 22,
    SIG_ARRAY_REQUEUE     = 23
};


const char *lsbSigSymbol[] = {
    "SIG_NULL",
    "SIG_CHKPNT",
    "SIG_CHKPNT_COPY",
    "SIG_DELETE_JOB",

    "SIG_SUSP_USER",
    "SIG_SUSP_LOAD",
    "SIG_SUSP_WINDOW",
    "SIG_SUSP_OTHER",

    "SIG_RESUME_USER",
    "SIG_RESUME_LOAD",
    "SIG_RESUME_WINDOW",
    "SIG_RESUME_OTHER",

    "SIG_TERM_USER",
    "SIG_TERM_LOAD",
    "SIG_TERM_WINDOW",
    "SIG_TERM_OTHER",
    "SIG_TERM_RUNLIMIT",
    "SIG_TERM_DEADLINE",
    "SIG_TERM_PROCESSLIMIT",
    "SIG_TERM_FORCE",
    "SIG_KILL_REQUEUE",
    "SIG_TERM_CPULIMIT",
    "SIG_TERM_MEMLIMIT"
};


int defaultSigValue[] = {
    SIG_NULL,
    SIG_CHKPNT,
    SIG_CHKPNT_COPY,
    SIG_DELETE_JOB,

    SIGSTOP,
    SIGSTOP,
    SIGSTOP,
    SIGSTOP,

    SIGCONT,
    SIGCONT,
    SIGCONT,
    SIGCONT,

    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
    SIGKILL,
};

/* lib/liblsbatch/signals.c */
int sigNameToValue_( const char *sigString);
const char *getLsbSigSymbol(int sigValue);
int getDefSigValue_(int sigValue, const char *actCmd);
int isSigTerm(int sigValue);
int isSigSusp(int sigValue);
int terminateWhen_(int *sigMap, const char *name);
