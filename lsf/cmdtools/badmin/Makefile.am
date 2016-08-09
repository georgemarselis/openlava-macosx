#
# badmin
#

AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include

bin_PROGRAMS = badmin 

COMMONSTUFF = $(top_builddir)/lib/liblsbatch.a $(top_builddir)/lsf/lib/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a -lm


badmin_LDADD   = -lnsl $(top_topbuilddir)/cmdtools/lsadm/startup.o $(COMMONSTUFF)