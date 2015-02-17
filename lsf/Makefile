
lsf_bindir :=../../binary_dir/openlava/2014-01-06/lib/liblsf
CFLAGS :=-I ../include -arch x86_64 $(CFLAGS)
INCLUDE_DIR :=
GCC := /opt/local/bin/gcc
#CC := $(GCC)

vpath %.c liblsf
vpath %.h ../include

clean:
	@cd lib && make $@
	@cd lsb && make $@
	@cd daemons && make $@
