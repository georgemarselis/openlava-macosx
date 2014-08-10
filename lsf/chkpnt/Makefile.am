#
# Copyright (C) openlava foundation
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/include/chkpnt

sbin_PROGRAMS = lchkpnt lrestart

echkpnt_SOURCES = echkpnt.c echkpnt.env.c echkpnt.lib.c
echkpnt_LDADD = ../lsf/lib/liblsf.a

erestart_SOURCES = erestart.c echkpnt.env.c echkpnt.lib.c
erestart_LDADD = ../lsf/lib/liblsf.a
