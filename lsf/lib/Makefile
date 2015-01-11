
lsf_bindir := ../../../binary_dir/openlava/2014-01-06/lib/liblsf
CFLAGS := -g -I ../include -c
INCLUDE_DIR := 
GCC := /opt/local/bin/gcc
#CC := $(GCC)

vpath %.c liblsf
vpath %.h ../include


liblsf: channel comm conf confmisc conn control cwd debug dir eauth eligible err
liblsf: errno esub host i18n id info init initenv lim limits load lsf misc
liblsf: nioback nios osal pim place priority queue rcp rdwr res resctrl reslog
liblsf: rex rf rtask rwait sig so sock syntax syslog table term tid tty usleep
liblsf: utmp wconf whathost words xdr xdrlim xdrmisc xdrnio xdrres xdrrf


.PHONY: clean

clean: 
	@rm -fv channel comm conf confmisc conn control cwd debug dir