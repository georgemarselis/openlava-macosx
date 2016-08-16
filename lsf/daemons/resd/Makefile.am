AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include -I$(top_srcdir)/config.h

sbin_PROGRAMS = resd
resd_SOURCES  = res.c handler.c misc.c rf.c getproc.c init.c pty.c tasklog.c rescom.h res.h resout.h
resd_LDADD = $(top_builddir)/lsf/lib/liblsbatch/liblsbatch.a $(top_builddir)/lsf/lib/liblsf/liblsf.a $(top_builddir)/lsf/intlib/liblsfint.a 
#-lm -lnsl

# if !CYGWIN
# res_LDADD += -lnsl
# endif

#etags:
#	etags *.[hc] ../*.h ../lib/*.[hc] ../intlib/*.[hc]
