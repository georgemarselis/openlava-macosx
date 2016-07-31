#
# Copyright (C) 2011-2012 David Bigagli
#

#include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf

#INCLUDES = -I$(top_srcdir)/lsf

sbin_PROGRAMS = eauth
eauth_SOURCES = eauth.c
eauth_LDADD = ../lsf/lib/liblsf.a ../lsf/intlib/liblsfint.a
