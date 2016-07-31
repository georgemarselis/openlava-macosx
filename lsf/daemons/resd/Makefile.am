#
# Copyright (C) 2011-2012 David Bigagli
#

# include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf

# INCLUDES = -I$(top_srcdir)/lsf

sbin_PROGRAMS = res nios
res_SOURCES  = \
res.c res.handler.c res.misc.c res.rf.c res.getproc.c res.init.c \
res.pty.c res.tasklog.c rescom.h res.h resout.h
res_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a -lm -lnsl
# if !CYGWIN
# res_LDADD += -lnsl
# endif

nios_SOURCES = nios.c nios.handler.c nios.h
#
# yep.. true
#
nios_LDADD = ../../lsbatch/lib/liblsbatch.a \
	../lib/liblsf.a ../intlib/liblsfint.a -lm -lnsl
# if !CYGWIN
# nios_LDADD += -lnsl
# endif
#
etags:
	etags *.[hc] ../*.h ../lib/*.[hc] ../intlib/*.[hc]
