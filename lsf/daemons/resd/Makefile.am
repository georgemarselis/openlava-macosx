AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include -I$(top_srcdir)/config.h

sbin_PROGRAMS = res nios
res_SOURCES  = res.c res.handler.c res.misc.c res.rf.c res.getproc.c res.init.c \
res.pty.c res.tasklog.c rescom.h res.h resout.h
res_LDADD = $(top_builddir)/lsf/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf//intlib/liblsfint    .a -lm -lnsl

# if !CYGWIN
# res_LDADD += -lnsl
# endif

#etags:
#	etags *.[hc] ../*.h ../lib/*.[hc] ../intlib/*.[hc]
