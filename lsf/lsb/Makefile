
lsf_bindir :=../../../binary_dir/openlava/2014-01-06/lib/liblsf
CFLAGS :=-I ../include -arch x86_64 $(CFLAGS)
INCLUDE_DIR :=
GCC := /opt/local/bin/gcc
#CC := $(GCC)

vpath %.c liblsf
vpath %.h ../include

lsb.so: comm.o conf.o debug.o err.o groups.o hc.o hosts.o init.o jobs.o 
lsb.so:	log.o mig.o misc.o modify.o move.o msg.o params.o peek.o qc.o
lsb.so: queues.o reason.o reconfig.o resource.o rexecv.o sig.o signals.o 
lsb.so: spool.o sub.o switch.o users.o xdr.o 


comm : comm.c


conf : conf.c


debug : debug.c


err : err.c


groups : groups.c


hc : hc.c


hosts : hosts.c


init : init.c


jobs : jobs.c


log : log.c


mig : mig.c


misc : misc.c


modify : modify.c


move : move.c


msg : msg.c


params : params.c


peek : peek.c


qc : qc.c


queues : queues.c


reason : reason.c


reconfig : reconfig.c


resource : resource.c


rexecv : rexecv.c


sig : sig.c


signals : signals.c


spool : spool.c


sub : sub.c


switch : switch.c


users : users.c


xdr : xdr.c


comm.o : comm.c


conf.o : conf.c


debug.o : debug.c


err.o : err.c


groups.o : groups.c


hc.o : hc.c


hosts.o : hosts.c


init.o : init.c


jobs.o : jobs.c


log.o : log.c


mig.o : mig.c


misc.o : misc.c


modify.o : modify.c


move.o : move.c


msg.o : msg.c


params.o : params.c


peek.o : peek.c


qc.o : qc.c


queues.o : queues.c


reason.o : reason.c


reconfig.o : reconfig.c


resource.o : resource.c


rexecv.o : rexecv.c


sig.o : sig.c


signals.o : signals.c


spool.o : spool.c


sub.o : sub.c


switch.o : switch.c


users.o : users.c


xdr.o : xdr.c


.PHONY: clean

clean:
	@rm -vf *.o
