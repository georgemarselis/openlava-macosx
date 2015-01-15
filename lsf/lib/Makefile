
lsf_bindir := ../../../binary_dir/openlava/2014-01-06/lib/liblsf
CFLAGS := -I ../include
INCLUDE_DIR := 
GCC := /opt/local/bin/gcc
#CC := $(GCC)

vpath %.c liblsf
vpath %.h ../include


liblsf.so: lim


liblsf.so: channel.o comm.o conf.o confmisc.o conn.o control.o cwd.o debug.o dir.o eauth.o eligible.o err.o
liblsf.so: errno.o esub.o host.o i18n.o id.o info.o init.o initenv.o limits.o load.o lsf.o misc.o
liblsf.so: nioback.o nios.o osal.o pim.o place.o priority.o queue.o rcp.o rdwr.o res.o resctrl.o reslog.o
liblsf.so: rex.o rf.o rtask.o rwait.o sig.o so.o sock.o syntax.o syslog.o table.o term.o tid.o tty.o usleep.o
liblsf.so: utmp.o wconf.o whathost.o words.o xdr.o xdrlim.o xdrmisc.o xdrnio.o xdrres.o xdrrf.o

# liblsf: channel.h conf.h hdr.h lib.h lproto.h lsi18n.h mls.h osal.h
# liblsf: osux.h pim.h queue.h rcp.h rf.h so.h table.h words.h xdr.h
# liblsf: xdrlim.h xdrres.h


# these go before all others, in the order stated
.PHONY:lim
lim: lim.o

lim.o: ../daemons/limd/lim.c ../include/daemons/liblimd/limd.h





channel.o: channel.c ../include/lib/channel.h

comm.o: comm.c

conf.o: conf.c ../include/lib/conf.h

confmisc.o: confmisc.c

conn.o: conn.c

control.o: control.c

cwd.o: cwd.c

debug.o: debug.c

dir.o: dir.c

eauth.o: eauth.c

eligible.o: eligible.c

err.o: err.c

errno.o: errno.c

esub.o: esub.c

host.o: host.c

i18n.o: i18n.c ../include/libint/lsi18n.h

id.o: id.c

info.o: info.c

init.o: init.c

initenv.o: initenv.c

limits.o: limits.c

load.o: load.c

lsf.o: lsf.c ../include/lsf.h

misc.o: misc.c

nioback.o: nioback.c

nios.o: nios.c

osal.o: osal.c ../include/lib/osal.h

pim.o: pim.c ../include/daemons/libpimd/pimd.h

place.o: place.c

priority.o: priority.c

queue.o: queue.c ../include/lib/queue.h

rcp.o: rcp.c ../include/lib/rcp.h

rdwr.o: rdwr.c

res.o: res.c ../include/daemons/libresd/resd.h

resctrl.o: resctrl.c 

reslog.o: reslog.c

rex.o: rex.c

rf.o: rf.c ../include/lib/rf.h

rtask.o: rtask.c

rwait.o: rwait.c

sig.o: sig.c ../include/lsb/sig.h

so.o: so.c

sock.o: sock.c ../include/lib/sock.h

syntax.o: syntax.c ../include/lib/syntax.h

syslog.o: syslog.c ../include/lib/syslog.h

table.o: table.c ../include/lib/table.h

term.o: term.c ../include/lib/term.h

tid.o: tid.c ../include/lib/tid.h

tty.o: tty.c ../include/lib/tty.h

usleep.o: usleep.c ../include/lib/usleep.h

utmp.o: utmp.c ../include/lib/utmp.h

wconf.o: wconf.c ../include/lib/wconf.h

whathost.o: whathost.c ../include/lib/whathost.h

words.o: words.c ../include/lib/words.h

xdr.o: xdr.c ../include/lib/xdr.h

xdrlim.o: xdrlim.c ../include/lib/xdrlim.h

xdrmisc.o: xdrmisc.c ../include/lib/xdrmisc.h

xdrnio.o: xdrnio.c ../include/lib/xdrnio.h

xdrres.o: xdrres.c ../include/lib/xdrres.h

xdrrf.o: xdrrf.c ../include/lib/xdrrf.h



.PHONY: clean

clean: 
	@rm -fv channel comm conf confmisc conn control cwd debug dir *.o
