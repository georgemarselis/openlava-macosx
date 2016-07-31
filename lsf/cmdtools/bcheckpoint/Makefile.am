#
# Copyright (C) openlava foundation
#

#include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\"

#INCLUDES = -I$(top_srcdir)/include/chkpnt

sbin_PROGRAMS  = lchkpnt lrestart

lchkpnt_SOURCES = echkpnt.c echkpnt.env.c echkpnt.lib.c
lchkpnt_LDADD = ../lsf/lib/liblsf.a

lrestart_SOURCES = erestart.c echkpnt.env.c echkpnt.lib.c
lrestart_LDADD = ../lsf/lib/liblsf.a
