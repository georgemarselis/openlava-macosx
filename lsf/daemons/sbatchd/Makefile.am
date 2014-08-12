#
# Makefile.am: sources for slave batch daemon
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf/include

if !CYGWIN
sbatchd_LDADD += -lnsl
endif

sbin_PROGRAMS = sbatchd
sbatchd_SOURCES = sbd.comm.c sbd.file.c sbd.job.c sbd.main.c \
                  sbd.misc.c sbd.policy.c sbd.serv.c sbd.sig.c sbd.xdr.c \
                  elock.c mail.c misc.c daemons.c daemons.xdr.c \
                  sbd.h daemonout.h daemons.h 

sbatchd_LDADD = ../lib/liblsbatch.a \
                ../../lsf/lib/liblsf.a \
                ../../lsf/intlib/liblsfint.a \
				-lm
etags :
	etags *.[hc] ../*.h ../lib/*.[hc] ../../lsf/*.h \
  	../../lsf/lib/*.[hc] ../../lsf/intlib/*.[hc]
