# recursive Makefile


.PHONY: all

all:

.PHONY: clean

clean:

# lib/liblsf.mk has the complete expansion of the liblsf target
include lib/liblsf.mk

lib: liblsf

# daemons/daemons-sbin.mk has the complete expansion of the daemons-sbin target
include daemons/daemons-sbin.mk

sbin: daemons-sbin
