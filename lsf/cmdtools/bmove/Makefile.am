#
# b* parts
#

#include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include

#INCLUDES = -I$(top_srcdir)/lsf/include

SUBDIRS = 

bin_PROGRAMS = bmove

COMMONSTUFF = ../lib/liblsbatch.a ../../lsf/lib/liblsf.a ../../lsf/intlib/liblsfint.a -lm


# ifndef cygwin
bmove_LDADD  = -lnsl $(COMMONSTUFF)
# endif

bmove_SOURCES   = bmove.c

install-data-local:
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bstop
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bresume
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bchkpnt
	cd "$(DESTDIR)$(bindir)" && ln -sf bmgroup bugroup

#etags :
#	etags *.[hc] ../*.h ../lib/*.[hc] ../../lsf/*.h ../../lsf/lib/*.[hc] \
#	../../lsf/intlib/*.[hc]
#etags :
#	etags *.[hc] ../*.h ../lib/*.[hc]

#
# ls* parts
#

bmove_LDADD += ../lib/liblsf.a

