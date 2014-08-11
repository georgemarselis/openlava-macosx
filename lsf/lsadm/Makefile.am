#
# Copyright (C) 2011-2012 David Bigagli
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf/include

bin_PROGRAMS = lsadmin

lsadmin_SOURCES = lsadmin.c startup.c limcontrol.c rescontrol.c lsadmin.h
lsadmin_LDADD =  ../lib/liblsf.a ../intlib/liblsfint.a

