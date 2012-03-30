#
# Copyright (C) 2011-2012 David Bigagli
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf

sbin_PROGRAMS = pim
pim_SOURCES  = pim.main.c pim.linux.h
pim_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a
