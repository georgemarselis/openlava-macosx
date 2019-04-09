/* $Id: lib.lsf.c 397 2007-11-26 19:04:00Z mblack $
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

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include "lib/lib.h"
#include "lsf.h"

// #ifndef MIN
// #define MIN(a,b) ((a) < (b) ? (a) : (b))
// #endif

// #ifndef MAX
// #define MAX(a,b) ((a) > (b) ? (a) : (b))
// #endif

// #ifndef LSFRU_FIELD_ADD
// #define LSFRU_FIELD_ADD(a,b) 
// { 
//     if ((a) < 0 || (b) < 0) { 
//         (a) = MAX((a), (b)); 
//     } else { 
//         (a) += (b); 
//     } 
// }
// #endif


void lsf_c_bullshit( ) 
{
    assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain

    return;
}

long int 
tosec( long int tv_usec )
{
    return tv_usec / 1000000;
}

void
ls_ruunix2lsf (struct rusage *rusage, struct lsfRusage *lsfRusage)
{
    // struct timeval ru_utime: user time
    lsfRusage->ru_utime.tv_sec = rusage->ru_utime.tv_sec + tosec( rusage->ru_utime.tv_usec ); // tv_sec: seconds / tv_usec: microseconds
    // struct timeval ru_utime: syste mtime
    lsfRusage->ru_stime.tv_sec = rusage->ru_stime.tv_sec + tosec( rusage->ru_stime.tv_usec ); // tv_sec: seconds / tv_usec: microseconds


    lsfRusage->ru_maxrss = rusage->ru_maxrss;
    lsfRusage->ru_ixrss  = rusage->ru_ixrss;

    lsfRusage->ru_ismrss = -1;

    lsfRusage->ru_isrss   = rusage->ru_isrss;
    lsfRusage->ru_minflt  = rusage->ru_minflt;
    lsfRusage->ru_majflt  = rusage->ru_majflt;
    lsfRusage->ru_nswap   = rusage->ru_nswap;
    lsfRusage->ru_inblock = rusage->ru_inblock;
    lsfRusage->ru_oublock = rusage->ru_oublock;

    lsfRusage->ru_ioch = -1;

    lsfRusage->ru_idrss    = rusage->ru_idrss;
    lsfRusage->ru_msgsnd   = rusage->ru_msgsnd;
    lsfRusage->ru_msgrcv   = rusage->ru_msgrcv;
    lsfRusage->ru_nsignals = rusage->ru_nsignals;
    lsfRusage->ru_nvcsw    = rusage->ru_nvcsw;
    lsfRusage->ru_nivcsw   = rusage->ru_nivcsw;
    lsfRusage->ru_exutime  = -1;


}

void
ls_rulsf2unix (struct lsfRusage *lsfRusage, struct rusage *rusage)
{
//    rusage->ru_utime.tv_sec   = MIN (lsfRusage->ru_utime, LONG_MAX);
    rusage->ru_utime.tv_sec   = lsfRusage->ru_utime.tv_sec;
    
//    rusage->ru_stime.tv_sec   = MIN (lsfRusage->ru_stime, LONG_MAX);
    rusage->ru_stime.tv_sec   = lsfRusage->ru_stime.tv_sec;

//    rusage->ru_utime.tv_usec  = MIN(( lsfRusage->ru_utime - rusage->ru_utime.tv_sec ) * 1000000, LONG_MAX );
//    rusage->ru_stime.tv_usec  = MIN(( lsfRusage->ru_stime - rusage->ru_stime.tv_sec ) * 1000000,  LONG_MAX );
#ifdef __APPLE__ 
    rusage->ru_utime.tv_usec  = MIN((__darwin_suseconds_t)( lsfRusage->ru_utime - rusage->ru_utime.tv_sec), INT_MAX );
    rusage->ru_stime.tv_usec  = MIN((__darwin_suseconds_t)( lsfRusage->ru_stime - rusage->ru_stime.tv_sec ), INT_MAX );
#endif
#ifdef __linux__
    rusage->ru_utime.tv_usec  = MIN( (suseconds_t) ( lsfRusage->ru_utime.tv_sec - rusage->ru_utime.tv_sec), INT_MAX );
    rusage->ru_stime.tv_usec  = MIN( (suseconds_t) ( lsfRusage->ru_stime.tv_sec - rusage->ru_stime.tv_sec ), INT_MAX );
#endif
    rusage->ru_maxrss = MIN (lsfRusage->ru_maxrss, LONG_MAX);
    rusage->ru_ixrss  = MIN (lsfRusage->ru_ixrss,  LONG_MAX);
    rusage->ru_isrss  = MIN (lsfRusage->ru_isrss,  LONG_MAX);

    rusage->ru_idrss   = MIN (lsfRusage->ru_idrss,   LONG_MAX);
    rusage->ru_isrss   = MIN (lsfRusage->ru_isrss,   LONG_MAX);
    rusage->ru_minflt  = MIN (lsfRusage->ru_minflt,  LONG_MAX);
    rusage->ru_majflt  = MIN (lsfRusage->ru_majflt,  LONG_MAX);
    rusage->ru_nswap   = MIN (lsfRusage->ru_nswap,   LONG_MAX);
    rusage->ru_inblock = MIN (lsfRusage->ru_inblock, LONG_MAX);
    rusage->ru_oublock = MIN (lsfRusage->ru_oublock, LONG_MAX);

    rusage->ru_msgsnd   = MIN (lsfRusage->ru_msgsnd,   LONG_MAX);
    rusage->ru_msgrcv   = MIN (lsfRusage->ru_msgrcv,   LONG_MAX);
    rusage->ru_nsignals = MIN (lsfRusage->ru_nsignals, LONG_MAX);
    rusage->ru_nvcsw    = MIN (lsfRusage->ru_nvcsw,    LONG_MAX);
    rusage->ru_nivcsw   = MIN (lsfRusage->ru_nivcsw,   LONG_MAX);

    return;
}

int
lsfRu2Str (FILE * log_fp, struct lsfRusage *lsfRu)
{
    // FIXME FIXME this output must be re-written to be more descriptive of what each varialbe represents
    // FIXME FIXME since the original format specifier was altered, this function should be altered as well to
    //                  perform the proper arithmetics
    time_t now;
    struct tm* now_tm;
    char out[80];
    
    time(&now);
    now_tm = localtime(&now);
    strftime (out, 80, "%Y-%m-%d %H:%M:%S.", now_tm);

    fprintf( log_fp, "\n\n>>>>>>>>>>>> %s >>>>>>>>>>>>\n", out );
    int cc =  fprintf (log_fp, "%1.0ld %1.0ld %1.0ld %1.0ld %1.0f  ", lsfRu->ru_utime.tv_sec, lsfRu->ru_stime.tv_sec, lsfRu->ru_maxrss, lsfRu->ru_ixrss,  lsfRu->ru_ismrss );
        cc += fprintf (log_fp, "%1.0ld %1.0ld %1.0ld %1.0ld %1.0ld ", lsfRu->ru_idrss,        lsfRu->ru_isrss,        lsfRu->ru_minflt, lsfRu->ru_majflt, lsfRu->ru_nswap  );
        cc += fprintf (log_fp, "%1.0ld %1.0ld %1.0f  %1.0ld %1.0ld ", lsfRu->ru_inblock,      lsfRu->ru_oublock,      lsfRu->ru_ioch,   lsfRu->ru_msgsnd, lsfRu->ru_msgrcv );
        cc += fprintf (log_fp, "%1.0ld %1.0ld %1.0ld %1.0f\n",        lsfRu->ru_nsignals,     lsfRu->ru_nvcsw,        lsfRu->ru_nivcsw, lsfRu->ru_exutime );
    return cc;

}

int
str2lsfRu (char *line, struct lsfRusage *lsfRu, int *ccount)
{
    // FIXME FIXME this output must be re-written to be more descriptive of what each varialbe represents
    // FIXME FIXME since the original format specifier was altered, this function should be altered as well to
    //                  perform the proper arithmetics
    // FIXME FIXME
    //      alter the scanf to include a timestamp
    time_t now;
    struct tm* now_tm;
    char out[80];
    
    time(&now);
    now_tm = localtime(&now);
    strftime (out, 80, "%Y-%m-%d %H:%M:%S.", now_tm);

//  int cc = sscanf (line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf%n", 
    int cc = sscanf (line, 
"%ld %ld  %ld %ld %lf %ld %ld %ld %ld %ld %ld %ld %lf %ld %ld %ld %ld %ld %lf %d", 
                &(lsfRu->ru_utime.tv_sec),    &(lsfRu->ru_stime.tv_sec), &(lsfRu->ru_maxrss),
                &(lsfRu->ru_ixrss),           &(lsfRu->ru_ismrss),       &(lsfRu->ru_idrss),   &(lsfRu->ru_isrss),   &(lsfRu->ru_minflt), &(lsfRu->ru_majflt),
                &(lsfRu->ru_nswap),           &(lsfRu->ru_inblock),      &(lsfRu->ru_oublock), &(lsfRu->ru_ioch),    &(lsfRu->ru_msgsnd), &(lsfRu->ru_msgrcv),
                &(lsfRu->ru_nsignals),        &(lsfRu->ru_nvcsw),        &(lsfRu->ru_nivcsw),  &(lsfRu->ru_exutime), ccount);
  return cc;
}


void
lsfRusageAdd_ (struct lsfRusage *lsfRusage1, struct lsfRusage *lsfRusage2)
{
    LSFRU_FIELD_ADD ( lsfRusage1->ru_utime.tv_sec,    lsfRusage2->ru_utime.tv_sec );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_stime.tv_sec,    lsfRusage2->ru_stime.tv_sec );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_maxrss,          lsfRusage2->ru_maxrss       );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_ixrss,           lsfRusage2->ru_ixrss        );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_ismrss,          lsfRusage2->ru_ismrss       );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_idrss,           lsfRusage2->ru_idrss        );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_isrss,           lsfRusage2->ru_isrss        );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_minflt,          lsfRusage2->ru_minflt       );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_majflt,          lsfRusage2->ru_majflt       );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_nswap,           lsfRusage2->ru_nswap        );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_inblock,         lsfRusage2->ru_inblock      );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_oublock,         lsfRusage2->ru_oublock      );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_ioch,            lsfRusage2->ru_ioch         );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_msgsnd,          lsfRusage2->ru_msgsnd       );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_msgrcv,          lsfRusage2->ru_msgrcv       );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_nsignals,        lsfRusage2->ru_nsignals     );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_nvcsw,           lsfRusage2->ru_nvcsw        );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_nivcsw,          lsfRusage2->ru_nivcsw       );
    LSFRU_FIELD_ADD ( lsfRusage1->ru_exutime,         lsfRusage2->ru_exutime      );

    return;
}

void
cleanLsfRusage (struct lsfRusage *lsfRusage)
{
    lsfRusage->ru_utime.tv_sec = -1;
    lsfRusage->ru_stime.tv_sec = -1;
    lsfRusage->ru_maxrss       = -1;
    lsfRusage->ru_ixrss        = -1;
    lsfRusage->ru_ismrss       = -1;
    lsfRusage->ru_idrss        = -1;
    lsfRusage->ru_isrss        = -1;
    lsfRusage->ru_minflt       = -1;
    lsfRusage->ru_majflt       = -1;
    lsfRusage->ru_nswap        = -1;
    lsfRusage->ru_inblock      = -1;
    lsfRusage->ru_oublock      = -1;
    lsfRusage->ru_ioch         = -1;
    lsfRusage->ru_msgsnd       = -1;
    lsfRusage->ru_msgrcv       = -1;
    lsfRusage->ru_nsignals     = -1;
    lsfRusage->ru_nvcsw        = -1;
    lsfRusage->ru_nivcsw       = -1;
    lsfRusage->ru_exutime      = -1;

    assert( lserrno ); // NOFIX bullshit code to get rid of compiler warning

    return;
}

void
cleanRusage (struct rusage *rusage)
{
    rusage->ru_utime.tv_sec  = -1;
    rusage->ru_utime.tv_usec = -1;

    rusage->ru_stime.tv_sec  = -1;
    rusage->ru_stime.tv_usec = -1;

    rusage->ru_maxrss = -1;
    rusage->ru_ixrss  = -1;

    rusage->ru_isrss   = -1;
    rusage->ru_minflt  = -1;
    rusage->ru_majflt  = -1;
    rusage->ru_nswap   = -1;
    rusage->ru_inblock = -1;
    rusage->ru_oublock = -1;

    rusage->ru_idrss    = -1;
    rusage->ru_msgsnd   = -1;
    rusage->ru_msgrcv   = -1;
    rusage->ru_nsignals = -1;
    rusage->ru_nvcsw    = -1;
    rusage->ru_nivcsw   = -1;

}
