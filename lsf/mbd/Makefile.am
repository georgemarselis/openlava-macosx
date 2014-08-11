#
# Makefile.am: automake file for the master batch daemon
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf/include

if !CYGWIN
mbatchd_LDADD += -lnsl
endif

sbin_PROGRAMS = mbatchd 
mbatchd_SOURCES  = \
comm.c host.c jgrp.c main.c proxy.c resource.c \
dep.c init.c job.c misc.c queue.c serv.c \
policy.c grp.c jarray.c log.c requeue.c window.c \
elock.c misc.c mail.c daemons.c daemons.xdr.c \
h daemonout.h daemons.h jgrp.h proxy.h profcnt.def 

# Leave for future reference.
#if SCHED_EXPERIMENTAL
#mbatchd_SOURCES += epolicy.c
#else
#mbatchd_SOURCES += policy.c
#endif

mbatchd_LDADD = ../lib/liblsbatch.a \
                ../../lsf/lib/liblsf.a \
                ../../lsf/intlib/liblsfint.a -lm

