#
# Copyright (C) 2011-2012 David Bigagli
#

# include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(srcdir)/lsf/include

#INCLUDES = -I$(top_srcdir)/lsf/include

#ifndef CYGWIN
#nios_LDADD += -lnsl
#endif

sbin_PROGRAMS = nios
nios_SOURCES = nios.c nios.handler.c nios.h
#
# yep.. true
#
nios_LDADD = ../../lsbatch/lib/liblsbatch.a \
	../lib/liblsf.a ../intlib/liblsfint.a -lm -lnsl

#etags:
#	etags *.[hc] ../*.h ../lib/*.[hc] ../intlib/*.[hc]
