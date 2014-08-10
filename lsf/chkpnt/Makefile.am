#
# Copyright (C) openlava foundation
#

include $(top_srcdir)/common.mk

INCLUDES = -I../lsf

sbin_PROGRAMS = echkpnt erestart

echkpnt_SOURCES = echkpnt.c echkpnt.env.c echkpnt.lib.c echkpnt.lib.h echkpnt.env.h
echkpnt_LDADD = ../lsf/lib/liblsf.a

erestart_SOURCES = erestart.c echkpnt.env.c echkpnt.lib.c echkpnt.lib.h echkpnt.env.h
erestart_LDADD = ../lsf/lib/liblsf.a
