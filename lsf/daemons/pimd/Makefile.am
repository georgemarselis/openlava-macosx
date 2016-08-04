#
# Copyright (C) 2011-2012 David Bigagli
#

#include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\"  -I$(srcdir)/lsf/include

#INCLUDES = -I$(top_srcdir)/lsf

sbin_PROGRAMS = pim
pim_SOURCES  = pim.main.c 
pim_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

#etags:
#	etags ../*.h *.[hc] ../intlib/*.[hc] ../lib/*.[hc] \
#	../res/*.[hc] ../lstools/*.[hc]

