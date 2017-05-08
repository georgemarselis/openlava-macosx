#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([openlava-macosx], [0], [george@marsel.is], [openlava], [https://marsel.is/openlava-macosx])

# Checks for programs.
AC_CONFIG_SRCDIR([lsf/include/lsf.h])
AC_CONFIG_HEADERS([config.h])

# Init configure aux dir
AC_CONFIG_AUX_DIR([build-aux])

# make autoconfiguration look for an external libintl
AM_GNU_GETTEXT([external])

# Init m4 macro dir
AC_CONFIG_MACRO_DIR([m4])

# AC_PROG_RANLIB

AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET
                
AC_PROG_LEX
# turn on full diagnostics for bison/yacc
AC_PROG_YACC 
AM_PROG_AR
# AX_PROG_TCL

# must also check to see if specific data structures are available, and un/set
# TCL_NO_DEPRECATED, USE_INTERP_RESULT, USE_INTERP_ERRORLINE
# HAVE_TCL_TCL_H = no 
# HAVE_TCL_H = no
# Checks for headers
AC_CHECK_HEADERS([tcl.h])
AC_CHECK_HEADERS([tcl.h], [HAVE_TCL_TCL_H=yes], [HAVE_TCL_H=y]) # FIXME FIXME FIXME FIXME FIXME 

# features
# enable/disable enviromental value support
# enable/disable TLS
# set mbatchd port
# set sbatchd port
# set resd port
# set limd port
# set pimd port
# set __LP64__ or check for same for rpc/xdr.h header
# check if macosx or other
# location of /tmp should be configurable; might go on a network, rather than local
# use gmp.h for floating point calculations?
# get kern.msgbuf from sysctl()


# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lncurses':
AC_CHECK_LIB([ncurses], [main])
# FIXME: Replace `main' with a function in `-lnsl':
AC_CHECK_LIB([nsl], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h limits.h malloc.h mntent.h netdb.h netinet/in.h stdint.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/mntent.h sys/mount.h sys/param.h sys/socket.h sys/statvfs.h sys/time.h syslog.h termios.h unistd.h utmp.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_GETGROUPS
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_FUNC_WAIT3
AC_CHECK_FUNCS([alarm atexit dup2 endgrent getcwd gethostbyaddr gethostbyname gethostname getpagesize gettimeofday inet_ntoa isascii memmove memset mkdir pow putenv rmdir select socket stime strcasecmp strchr strdup strerror strncasecmp strpbrk strrchr strstr strtol utime])

AC_CONFIG_FILES([Makefile po/Makefile.in
                config/Makefile
                docs/examples/Makefile
                docs/man/Makefile
                docs/man/man1/Makefile
                docs/man/man5/Makefile
                docs/man/man8/Makefile
                lsf/Makefile
                lsf/cmdtools/Makefile
                lsf/cmdtools/badmin/Makefile
                lsf/cmdtools/bbot/Makefile
                lsf/cmdtools/bbottom/Makefile
                lsf/cmdtools/bcheckpoint/Makefile
                lsf/cmdtools/beligible/Makefile
                lsf/cmdtools/bhc/Makefile
                lsf/cmdtools/bhist/Makefile
                lsf/cmdtools/bhosts/Makefile
                lsf/cmdtools/bjobs/Makefile
                lsf/cmdtools/bkill/Makefile
                lsf/cmdtools/bmgroup/Makefile
                lsf/cmdtools/bmig/Makefile
                lsf/cmdtools/bmigrate/Makefile
                lsf/cmdtools/bmigrategroup/Makefile
                lsf/cmdtools/bmod/Makefile
                lsf/cmdtools/bmodify/Makefile
                lsf/cmdtools/bparams/Makefile
                lsf/cmdtools/bpeek/Makefile
                lsf/cmdtools/bqc/Makefile
                lsf/cmdtools/bqueues/Makefile
                lsf/cmdtools/brequeue/Makefile
                lsf/cmdtools/brestart/Makefile
                lsf/cmdtools/bresume/Makefile
                lsf/cmdtools/brun/Makefile
                lsf/cmdtools/bstop/Makefile
                lsf/cmdtools/bsub/Makefile
                lsf/cmdtools/bswitch/Makefile
                lsf/cmdtools/btop/Makefile
                lsf/cmdtools/bugroup/Makefile
                lsf/cmdtools/busers/Makefile
                lsf/cmdtools/lib/Makefile
                lsf/cmdtools/lsacct/Makefile
                lsf/cmdtools/lsaddhost/Makefile
                lsf/cmdtools/lsadm/Makefile
                lsf/cmdtools/lseligible/Makefile
                lsf/cmdtools/lshosts/Makefile
                lsf/cmdtools/lsid/Makefile
                lsf/cmdtools/lsinfo/Makefile
                lsf/cmdtools/lsload/Makefile
                lsf/cmdtools/lsloadadj/Makefile
                lsf/cmdtools/lsmon/Makefile
                lsf/cmdtools/lsplace/Makefile
                lsf/cmdtools/lsrcp/Makefile
                lsf/cmdtools/lsrmhost/Makefile
                lsf/cmdtools/lsrun/Makefile
                lsf/daemons/Makefile
                lsf/daemons/common/Makefile
                lsf/daemons/eauthd/Makefile
                lsf/daemons/limd/Makefile
                lsf/daemons/mbatchd/Makefile
                lsf/daemons/niosd/Makefile
                lsf/daemons/pimd/Makefile
                lsf/daemons/resd/Makefile
                lsf/daemons/sbatchd/Makefile
                lsf/intlib/Makefile
                lsf/lib/Makefile
                lsf/lib/liblsf/Makefile
                lsf/lib/liblsbatch/Makefile
                scripts/Makefile])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])
LT_INIT
AC_OUTPUT
