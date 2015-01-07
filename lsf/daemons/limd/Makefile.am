#
# Copyright (C) 2011-2012 David Bigagli
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf/include
AM_CPPFLAGS += -D$(HOSTTYPE) -DHOST_TYPE_STRING=\"$(HOSTTYPE)\"

HOSTTYPE=LINUX64

sbin_PROGRAMS = lim
lim_SOURCES  = \
cluster.c control.c internal.c main.c policy.c \
xdr.c conf.c info.c load.c misc.c  rload.c  \
common.h  conf.h  lim.h  linux.h  limout.h
lim_LDADD =  ../lib/liblsf.a ../intlib/liblsfint.a

etags:
	etags ../*.h *.[hc] ../intlib/*.[hc] ../lib/*.[hc] \
	../res/*.[hc] ../lstools/*.[hc]

