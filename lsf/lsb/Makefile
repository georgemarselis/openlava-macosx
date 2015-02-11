
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
#lsb.so: conf_queues.o conf_queuegroups.o conf_users.o conf_usergroups.o conf_hosts.o conf_hostgroups.o conf_misc.o conf_cluster.o


comm: comm.o


conf: conf.o

# conf_queues conf_queuegroups conf_users conf_usergroups conf_hosts conf_hostgroups conf_cluster conf_misc 

# conf_queues: conf_queues.o

# conf_queuegroups: conf_queuegroups.o

# conf_users: conf_users.o

# conf_usergroups: conf_usergroups.o

# conf_hosts: conf_hosts.o

# conf_hostgroups: conf_hostgroups.o

# conf_cluster: conf_cluster.o

# conf_misc: conf_misc.o



debug: debug.o


err: err.o


groups: groups.o


hc: hc.o


hosts: hosts.o


init: init.o


jobs: jobs.o


log: log.o


mig: mig.o


misc: misc.o


modify: modify.o


move: move.o


msg: msg.o


params: params.o


peek: peek.o


qc: qc.o


queues: queues.o


reason: reason.o


reconfig: reconfig.o


resource: resource.o


rexecv: rexecv.o


sig: sig.o


signals: signals.o


spool: spool.o


sub: sub.o


switch: switch.o


users: users.o


xdr: xdr.o





comm.o: comm.c


conf.o: conf.c


# conf_queues.o conf_queuegroups.o conf_users.o conf_usergroups.o conf_hosts.o conf_hostgroups.o conf_misc.o conf_cluster.o

# conf_queues.o: conf_queues.c


# conf_queuegroups.o: conf_queuegroups.c


# conf_users.o: conf_users.c


# conf_usergroups.o: conf_usergroups.c


# conf_hosts.o: conf_hosts.c


# conf_hostgroups.o: conf_hostgroups.c


# conf_cluster.o: conf_cluster.c


# conf_misc.o: conf_misc.c




debug.o: debug.c


err.o: err.c


groups.o: groups.c


hc.o: hc.c


hosts.o: hosts.c


init.o: init.c


jobs.o: jobs.c


log.o: log.c


mig.o: mig.c


misc.o: misc.c


modify.o: modify.c


move.o: move.c


msg.o: msg.c


params.o: params.c


peek.o: peek.c


qc.o: qc.c


queues.o: queues.c


reason.o: reason.c


reconfig.o: reconfig.c


resource.o: resource.c


rexecv.o: rexecv.c


sig.o: sig.c


signals.o: signals.c


spool.o: spool.c


sub.o: sub.c


switch.o: switch.c


users.o: users.c


xdr.o: xdr.c






.PHONY: clean

clean:
	@rm -vf *.o
