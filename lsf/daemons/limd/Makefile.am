#
# Copyright (C) 2011-2012 David Bigagli
#

#include $(top_srcdir)/common.mk
# contencts of common.mk
HOSTTYPE = LINUX64
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include -D$(HOSTTYPE) -DHOST_TYPE_STRING=\"$(HOSTTYPE)\"

# INCLUDES = -I$(top_srcdir)/lsf/include


sbin_PROGRAMS = lim
lim_SOURCES  = \
cluster.c control.c internal.c main.c policy.c \
xdr.c conf.c info.c load.c misc.c  rload.c  \
common.h  conf.h  lim.h  linux.h  limout.h
lim_LDADD =  ../lib/liblsf.a ../intlib/liblsfint.a

etags:
	etags ../*.h *.[hc] ../intlib/*.[hc] ../lib/*.[hc] \
	../res/*.[hc] ../lstools/*.[hc]

