AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include -I$(top_srcdir)/config.h

sbin_PROGRAMS = niosd
niosd_SOURCES = nios.c nios.handler.c nios.h
#
# yep.. true
#
#nios_LDADD = $(top_builddir)/lsf/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf//intlib/liblsfint.a -lm -lnsl


#etags:
#	etags *.[hc] ../*.h ../lib/*.[hc] ../intlib/*.[hc]
