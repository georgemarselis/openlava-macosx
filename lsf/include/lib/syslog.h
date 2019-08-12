/* 
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

static char logfile[ MAXPATHLEN * sizeof(char) + 1];
static char *logident;
static int logmask;
static enum { LOGTO_SYS, LOGTO_FILE, LOGTO_STDERR } log_dest;

int logclass = 0; // also defined in lsf.h
int timinglevel = 0;

// #define LSF_LOG_MASK   7
// #define DEF_LOG_MASK   LOG_INFO
// #define DEF_LOG_MASK_NAME   "LOG_WARNING"

const char DEF_LOG_MASK[ ]      = "LOG_INFO";
const char DEF_LOG_MASK_NAME[ ] = "LOG_WARNING";
/*#ifndef HAS_VSNPRINTF FIXME FIXME
#define HAS_VSNPRINTF
#endif
*/


char *argvmsg_( int argc, char **argv);
void  ls_openlog( const char *ident, const char *path, int use_stderr, const char *logMask);
int   openLogFile( const char *ident, const char *myname );
void  ls_syslog( int level, const char *fmt, ... );
void  ls_closelog( void);
int   ls_setlogmask( int maskpri );
int   getLogMask ( char **msg, const char *logMask);
int   getLogClass_ ( const char *lsp, const char *tsp);
void  ls_closelog_ext (void);


