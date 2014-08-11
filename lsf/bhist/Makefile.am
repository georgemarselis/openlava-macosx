#
# Copyright (C) 2011-2012 David Bigagli
#

include $(top_srcdir)/common.mk

INCLUDES = -I$(top_srcdir)/lsf/include

bin_PROGRAMS = bhist
bhist_SOURCES  = bhist.c read.event.c bhist.h
bhist_LDADD = ../cmd/cmd.job.o \
	../cmd/cmd.misc.o ../cmd/cmd.jobid.o ../cmd/cmd.prt.o \
	../cmd/cmd.err.o ../cmd/cmd.hist.o \
	../lib/liblsbatch.a \
	../../lsf/lib/liblsf.a \
	../../lsf/intlib/liblsfint.a -lm

if !CYGWIN
bhist_LDADD += -lnsl
endif
