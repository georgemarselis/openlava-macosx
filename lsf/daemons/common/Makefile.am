# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include

sbin_PROGRAMS = mail
mail_SOURCES  = daemons.c elock.c mail.c misc.c xdr.c

mail_LDADD = $(top_builddir)/lib/liblsbatch/liblsbatch.a $(top_builddir)/lib/liblsf.a $(top_build)/intlib/liblsfint.a -lm
