#
# b* parts
#

#include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include

#INCLUDES = -I$(top_srcdir)/lsf/include

SUBDIRS = 

bin_PROGRAMS = badmin bkill bparams brestart btop bbot bmgroup \
bpeek brun busers bhosts bmig bqueues bsub bjobs bmod \
brequeue bswitch bhist

COMMONSTUFF = ../lib/liblsbatch.a ../../lsf/lib/liblsf.a ../../lsf/intlib/liblsfint.a -lm


# ifndef cygwin
badmin_LDADD   = -lnsl ../../lsf/lsadm/startup.o $(COMMONSTUFF)
bkill_LDADD    = -lnsl $(COMMONSTUFF)
bparams_LDADD  = -lnsl $(COMMONSTUFF)
brestart_LDADD = -lnsl $(COMMONSTUFF)
btop_LDADD     = -lnsl $(COMMONSTUFF)
bbot_LDADD     = -lnsl $(COMMONSTUFF)
bmgroup_LDADD  = -lnsl $(COMMONSTUFF)
bpeek_LDADD    = -lnsl $(COMMONSTUFF)
brun_LDADD     = -lnsl $(COMMONSTUFF)
busers_LDADD   = -lnsl $(COMMONSTUFF)
bhosts_LDADD   = -lnsl $(COMMONSTUFF)
bmig_LDADD     = -lnsl $(COMMONSTUFF)
bqueues_LDADD  = -lnsl $(COMMONSTUFF)
bsub_LDADD     = -lnsl $(COMMONSTUFF)
bjobs_LDADD    = -lnsl $(COMMONSTUFF)
bmod_LDADD     = -lnsl $(COMMONSTUFF)
bhist_LDADD    = -lnsl cmd.job.o cmd.misc.o cmd.jobid.o cmd.prt.o cmd.err.o cmd.hist.o $(COMMONSTUFF)
brequeue_LDADD = -lnsl $(COMMONSTUFF)
bswitch_LDADD  = -lnsl $(COMMONSTUFF)
# endif

badmin_SOURCES   = badmin.c cmd.bqc.c cmd.hist.c cmd.bhc.c cmd.misc.c cmd.job.c cmd.prt.c badmin.h cmd.h
bkill_SOURCES    = bkill.c cmd.sig.c cmd.jobid.c cmd.err.c
bparams_SOURCES  = bparams.c cmd.h
brestart_SOURCES = brestart.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
btop_SOURCES     = btop.c cmd.move.c cmd.jobid.c cmd.misc.c cmd.prt.c cmd.err.c cmd.h
bbot_SOURCES     = bbot.c cmd.move.c cmd.jobid.c cmd.misc.c cmd.prt.c cmd.err.c cmd.h
bmgroup_SOURCES  = bmgroup.c cmd.misc.c cmd.h
bpeek_SOURCES    = bpeek.c cmd.err.c cmd.jobid.c cmd.misc.c cmd.prt.c cmd.h
brun_SOURCES     = brun.c cmd.jobid.c cmd.err.c cmd.h
busers_SOURCES   = busers.c cmd.misc.c
bhosts_SOURCES   = bhosts.c cmd.prt.c cmd.misc.c cmd.h
bmig_SOURCES     = bmig.c cmd.jobid.c cmd.err.c cmd.h
bqueues_SOURCES  = bqueues.c cmd.prt.c cmd.misc.c cmd.h
bsub_SOURCES     = bsub.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
bjobs_SOURCES    = bjobs.c cmd.prt.c cmd.err.c cmd.job.c cmd.jobid.c cmd.misc.c cmd.h
bmod_SOURCES     = bmod.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
brequeue_SOURCES = brequeue.c cmd.jobid.c cmd.err.c cmd.h
bswitch_SOURCES  = bswitch.c cmd.jobid.c cmd.err.c cmd.h
bhist_SOURCES    = bhist.c read.event.c bhist.h

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

bin_PROGRAMS += lsacct lseligible lshosts lsid lsinfo lsloadadj \
 		lsload lsmon lsplace lsrcp lsrun lsaddhost lsrmhost

lsacct_SOURCES = lsacct.c  
lsacct_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lseligible_SOURCES = lseligible.c  
lseligible_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lshosts_SOURCES = lshosts.c  
lshosts_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lsid_SOURCES = lsid.c  
lsid_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lsinfo_SOURCES = lsinfo.c  
lsinfo_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lsloadadj_SOURCES = lsloadadj.c  
lsloadadj_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lsload_SOURCES = lsload.c load.c
lsload_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lsmon_SOURCES = lsmon.c  
lsmon_LDADD = load.o ../lib/liblsf.a ../intlib/liblsfint.a -lncurses

lsplace_SOURCES = lsplace.c  
lsplace_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a

lsrcp_SOURCES = lsrcp.c
lsrcp_LDADD = -lnsl ../lib/liblsf.a ../intlib/liblsfint.a 

lsrun_SOURCES = lsrun.c
lsrun_LDADD = ../lib/liblsf.a 

lsaddhost_SOURCES = lsaddhost.c
lsaddhost_LDADD = ../lib/liblsf.a 

lsrmhost_SOURCES = lsrmhost.c
lsrmhost_LDADD = ../lib/liblsf.a

