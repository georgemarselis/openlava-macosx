AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include -I$(top_srcdir)/config.h

lib_LIBRARIES = libdaemons.a libelock.a libmail.a libmisc.a libxdr.a
libdaemons_a_SOURCES = daemons.c
libdaemons_a_LIBADD   = $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a 

libelock_a_SOURCES = elock.c
libelock_a_LIBADD   = $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a 

libmail_a_SOURCES  = mail.c
libmail_a_LIBADD   = $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a 

libmisc_a_SOURCES = misc.c
libmisc_a_LIBADD   = $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a 

libxdr_a_SOURCES = xdr.c
libxdr_a_LIBADD   = $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a 

