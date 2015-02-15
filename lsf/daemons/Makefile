
lsf_bindir :=../../../binary_dir/openlava/2014-01-06/lib/liblsf
CFLAGS :=-I ../include -arch x86_64 $(CFLAGS)
INCLUDE_DIR :=
GCC := /opt/local/bin/gcc
#CC := $(GCC)


vpath %.h ../include

##################################

vpath %.c common
common: common.o

common.o: daemons.o elock.o mail.o misc.o xdr.o

daemons.o: daemons.c

elock.o: elock.c

mail.o: mail.c




##################################

vpath %.c eauthd

eauthd: eauth.o

eauth.o: eauth.c




##################################

vpath %.c limd

limd: limd.o

limd.o: cluster.o conf.o control.o info.o internal.o lim.o load.o 
limd.o: main.o misc.o policy.o rload.o xdr.o


cluster.o: cluster.c


conf.o: conf.c


control.o: control.c


info.o: info.c


internal.o: internal.c


lim.o: lim.c


load.o: load.c


main.o: main.c


misc.o: misc.c


policy.o: policy.c


rload.o: rload.c


xdr.o: xdr.c




##################################

vpath %.c mbatchd

mbatchd: mbatchd.o

mbatch.o: comm.o dep.o grp.o host.o init.o jarray.o jgrp.o job.o log.o 
mbatch.o: main.o misc.o policy.o proxy.o queue.o requeue.o resource.o 
mbatch.o: serv.o window.o


comm.o: comm.c


dep.o: dep.c


grp.o: grp.c


host.o: host.c


init.o: init.c


jarray.o: jarray.c


jgrp.o: jgrp.c


job.o: job.c


log.o: log.c


main.o: main.c


misc.o: misc.c


policy.o: policy.c


proxy.o: proxy.c


queue.o: queue.c


requeue.o: requeue.c


resource.o: resource.c


serv.o: serv.c


window.o: window.c




##################################

vpath %.c niosd

niosd: nios.o handler.o

nios.o: nios.c

handler.o: handler.c




##################################

vpath %.c pimd

pimd: pimd.o

pimd.o: linux.o pim.o

linux.o: linux.c

pim.o: pim.c




##################################

vpath %.c resd

resd: resd.o

resd.o: getproc.o handler.o init.o misc.o pty.o res.o rf.o tasklog.o


getproc.o: getproc.c


handler.o: handler.c


init.o: init.c


misc.o: misc.c


pty.o: pty.c


res.o: res.c


rf.o: rf.c


tasklog.o: tasklog.c




##################################

vpath %.c sbatchd

sbatchd: sbatchd.o

sbatchd.o: comm.o file.o job.o main.o misc.o policy.o serv.o sig.o xdr.o

comm.o: comm.c


file.o: file.c


job.o: job.c


main.o: main.c


misc.o: misc.c


policy.o: policy.c


serv.o: serv.c


sig.o: sig.c


xdr.o: xdr.c



##################################

.PHONY: clean

clean:
	@rm -vf *.o


