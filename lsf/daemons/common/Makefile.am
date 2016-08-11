# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include

sbin_PROGRAMS = mail
mail_SOURCES  = daemons.c elock.c mail.c misc.c xdr.c

mail_LDADD = $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a -lm
