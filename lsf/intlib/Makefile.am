#
# Copyright (C) 2011-2012 David Bigagli
#

# include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include -I$(top_srcdir)


noinst_LIBRARIES = liblsfint.a

liblsfint_a_SOURCES = admin.c bitset.c callex.c cmdtime.c conf.c \
	daemon.c idx.y link.c list.c list2.c listset.c lsftcl.c misc.c \
	resourcecmd.c resreq.c set.c testbitset.c userok.c window.c
