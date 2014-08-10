#
# Copyright (C) 2011-2012 David Bigagli
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf/include

bin_PROGRAMS = badmin bkill bparams brestart btop bbot bmgroup \
bpeek brun busers bhosts bmig bqueues bsub bjobs bmod \
brequeue bswitch 

badmin_SOURCES = badmin.c cmd.bqc.c cmd.hist.c \
	cmd.bhc.c cmd.misc.c cmd.job.c cmd.prt.c \
	badmin.h cmd.h
badmin_LDADD = ../../lsf/lsadm/startup.o \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a -lm
if !CYGWIN
badmin_LDADD += -lnsl
endif

bkill_SOURCES = bkill.c cmd.sig.c cmd.jobid.c cmd.err.c
bkill_LDADD = \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm

if !CYGWIN
bkill_LDADD += -lnsl
endif

bparams_SOURCES = bparams.c cmd.h
bparams_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bparams_LDADD += -lnsl
endif


brestart_SOURCES = brestart.c cmd.sub.c cmd.jobid.c \
	cmd.err.c cmd.h
brestart_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
brestart_LDADD += -lnsl
endif


btop_SOURCES = btop.c cmd.move.c cmd.jobid.c cmd.misc.c \
	 cmd.prt.c cmd.err.c cmd.h
btop_LDADD =  \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
btop_LDADD += -lnsl
endif


bbot_SOURCES = bbot.c cmd.move.c cmd.jobid.c cmd.misc.c \
	 cmd.prt.c cmd.err.c cmd.h
bbot_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bbot_LDADD += -lnsl
endif


bmgroup_SOURCES = bmgroup.c cmd.misc.c cmd.h
bmgroup_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bmgroup_LDADD += -lnsl
endif


bpeek_SOURCES = bpeek.c cmd.err.c cmd.jobid.c cmd.misc.c cmd.prt.c cmd.h
bpeek_LDADD =  \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bpeek_LDADD += -lnsl
endif


brun_SOURCES = brun.c cmd.jobid.c cmd.err.c cmd.h
brun_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
brun_LDADD += -lnsl
endif


busers_SOURCES = busers.c cmd.misc.c
busers_LDADD =  \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
busers_LDADD += -lnsl
endif


bhosts_SOURCES = bhosts.c cmd.prt.c cmd.misc.c cmd.h
bhosts_LDADD =  \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bhosts_LDADD += -lnsl
endif

bmig_SOURCES = bmig.c cmd.jobid.c cmd.err.c cmd.h
bmig_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bmig_LDADD += -lnsl
endif

bqueues_SOURCES = bqueues.c cmd.prt.c cmd.misc.c cmd.h
bqueues_LDADD = \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bqueues_LDADD += -lnsl
endif

bsub_SOURCES = bsub.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
bsub_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bsub_LDADD += -lnsl
endif

bjobs_SOURCES = bjobs.c cmd.prt.c cmd.err.c cmd.job.c \
	cmd.jobid.c cmd.misc.c cmd.h
bjobs_LDADD =  \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bjobs_LDADD += -lnsl
endif

bmod_SOURCES = bmod.c cmd.sub.c cmd.jobid.c cmd.err.c cmd.h
bmod_LDADD =  \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bmod_LDADD += -lnsl
endif

brequeue_SOURCES = brequeue.c cmd.jobid.c cmd.err.c cmd.h
brequeue_LDADD =   \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
brequeue_LDADD += -lnsl
endif

bswitch_SOURCES = bswitch.c cmd.jobid.c cmd.err.c cmd.h
bswitch_LDADD =  \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a  -lm
if !CYGWIN
bswitch_LDADD += -lnsl
endif

install-data-local:
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bstop
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bresume
	cd "$(DESTDIR)$(bindir)" && ln -sf bkill bchkpnt
	cd "$(DESTDIR)$(bindir)" && ln -sf bmgroup bugroup

etags :
	etags *.[hc] ../*.h ../lib/*.[hc] ../../lsf/*.h ../../lsf/lib/*.[hc] \
	../../lsf/intlib/*.[hc]

#
# Copyright (C) 2011-2012 David Bigagli
#

include $(top_srcdir)/common.mk

INCLUDES = -I..

bin_PROGRAMS = lsacct lseligible lshosts lsid lsinfo lsloadadj \
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
if !CYGWIN
lsrcp_LDADD += -lnsl
endif

lsrun_SOURCES = lsrun.c
lsrun_LDADD = ../lib/liblsf.a 

lsaddhost_SOURCES = lsaddhost.c
lsaddhost_LDADD = ../lib/liblsf.a 

lsrmhost_SOURCES = lsrmhost.c
lsrmhost_LDADD = ../lib/liblsf.a 

etags :
	etags *.[hc] ../*.h ../lib/*.[hc] 
