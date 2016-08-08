# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include

sbin_PROGRAMS = mail
mail_SOURCES  = daemons.c elock.c mail.c misc.c xdr.c

mail_LDADD = ../../lib/liblsbatch/liblsbatch.a ../../lsf/lib/liblsf.a ../../lsf/intlib/liblsfint.a -lm
