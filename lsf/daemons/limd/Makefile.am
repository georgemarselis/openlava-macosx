AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include -I$(top_srcdir)/config.h

sbin_PROGRAMS = limd
limd_SOURCES  = \
cluster.c control.c internal.c main.c policy.c \
xdr.c conf.c info.c load.c misc.c  rload.c  linux.c
#common.h  conf.h  lim.h  linux.h  limout.h
limd_LDADD =  $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a
#etags:
#	etags ../*.h *.[hc] ../intlib/*.[hc] ../lib/*.[hc] \
#	../res/*.[hc] ../lstools/*.[hc]

