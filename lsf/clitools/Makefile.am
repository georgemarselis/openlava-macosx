#
# b* parts
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf/include

bin_PROGRAMS = badmin bkill bparams brestart btop bbot bmgroup \
bpeek brun busers bhosts bmig bqueues bsub bjobs bmod \
brequeue bswitch bhist

ifndef CYGWIN
badmin_LDADD += -lnsl
bkill_LDADD += -lnsl
lsrcp_LDADD += -lnsl
bparams_LDADD += -lnsl
brestart_LDADD += -lnsl
btop_LDADD += -lnsl
bbot_LDADD += -lnsl
bmgroup_LDADD += -lnsl
bpeek_LDADD += -lnsl
brun_LDADD += -lnsl
busers_LDADD += -lnsl
bhosts_LDADD += -lnsl
bmig_LDADD += -lnsl
bqueues_LDADD += -lnsl
bsub_LDADD += -lnsl
bjobs_LDADD += -lnsl
bmod_LDADD += -lnsl
bhist_LDADD += -lnsl
brequeue_LDADD += -lnsl
bswitch_LDADD += -lnsl
endif

badmin_SOURCES = badmin.c cmd.bqc.c cmd.hist.c \
	cmd.bhc.c cmd.misc.c cmd.job.c cmd.prt.c \
	badmin.h cmd.h
badmin_LDADD = ../../lsf/lsadm/startup.o \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bkill_SOURCES = bkill.c cmd.sig.c cmd.jobid.c cmd.err.c
bkill_LDADD = \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bparams_SOURCES = bparams.c cmd.h
bparams_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

brestart_SOURCES = brestart.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
brestart_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \-lm

btop_SOURCES = btop.c cmd.move.c cmd.jobid.c cmd.misc.c cmd.prt.c cmd.err.c cmd.h
btop_LDADD = ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bbot_SOURCES = bbot.c cmd.move.c cmd.jobid.c cmd.misc.c cmd.prt.c cmd.err.c cmd.h
bbot_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bmgroup_SOURCES = bmgroup.c cmd.misc.c cmd.h
bmgroup_LDADD = ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bpeek_SOURCES = bpeek.c cmd.err.c cmd.jobid.c cmd.misc.c cmd.prt.c cmd.h
bpeek_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm

brun_SOURCES = brun.c cmd.jobid.c cmd.err.c cmd.h
brun_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

busers_SOURCES = busers.c cmd.misc.c
busers_LDADD = ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bhosts_SOURCES = bhosts.c cmd.prt.c cmd.misc.c cmd.h
bhosts_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bmig_SOURCES = bmig.c cmd.jobid.c cmd.err.c cmd.h
bmig_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bqueues_SOURCES = bqueues.c cmd.prt.c cmd.misc.c cmd.h
bqueues_LDADD = ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bsub_SOURCES = bsub.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
bsub_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bjobs_SOURCES = bjobs.c cmd.prt.c cmd.err.c cmd.job.c \
	cmd.jobid.c cmd.misc.c cmd.h
bjobs_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bmod_SOURCES = bmod.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
bmod_LDADD = ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  \
	-lm

brequeue_SOURCES = brequeue.c cmd.jobid.c cmd.err.c cmd.h
brequeue_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  \
	-lm

bswitch_SOURCES = bswitch.c cmd.jobid.c cmd.err.c cmd.h
bswitch_LDADD =  ../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a \
	-lm

bhist_SOURCES  = bhist.c read.event.c bhist.h
bhist_LDADD = cmd.job.o cmd.misc.o cmd.jobid.o cmd.prt.o cmd.err.o cmd.hist.o \
	lib/liblsbatch.a lib/liblsf.a \
	intlib/liblsfint.a \
	-lm

install-data-local:
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bstop
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bresume
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bchkpnt
	cd "$(DESTDIR)$(bindir)" && ln -sf bmgroup bugroup

etags :
	etags *.[hc] ../*.h ../lib/*.[hc] ../../lsf/*.h ../../lsf/lib/*.[hc] \
	../../lsf/intlib/*.[hc]
etags :
	etags *.[hc] ../*.h ../lib/*.[hc]

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
lsrcp_LDADD = ../lib/liblsf.a ../intlib/liblsfint.a 

lsrun_SOURCES = lsrun.c
lsrun_LDADD = ../lib/liblsf.a 

lsaddhost_SOURCES = lsaddhost.c
lsaddhost_LDADD = ../lib/liblsf.a 

lsrmhost_SOURCES = lsrmhost.c
lsrmhost_LDADD = ../lib/liblsf.a

