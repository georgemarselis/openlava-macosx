
lsf_bindir :=../../binary_dir/openlava/2014-01-06/lib/liblsf
CFLAGS :=-I ../include -arch x86_64 $(CFLAGS)
INCLUDE_DIR :=
GCC := /opt/local/bin/gcc
#CC := $(GCC)

vpath %.c liblsf
vpath %.h ../include

.PHONY: all

all: liblsf.so lsb.so common.o

liblsf.so:
	cd lib && echo -e "\n\n\n\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n\n\n"; make -j2 clean; CFLAGS="-g -Weverything -Wextra -Wformat=2 -Wno-format-extra-args -Wno-format-zero-length -Winit-self -Wimplicit-int -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -pedantic-errors -ansi -std=c99 -Wall -ferror-limit=0" make liblsf.so

lsb.so:
	cd lsb && echo -e "\n\n\n\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n\n\n"; make -j2 clean; CFLAGS="-g -Weverything -Wextra -Wformat=2 -Wno-format-extra-args -Wno-format-zero-length -Winit-self -Wimplicit-int -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -pedantic-errors -ansi -std=c99 -Wall -ferror-limit=0" make lsb.so

common.o:
	cd daemons && echo -e "\n\n\n\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n\n\n"; make -j2 clean; CFLAGS="-g -Weverything -Wextra -Wformat=2 -Wno-format-extra-args -Wno-format-zero-length -Winit-self -Wimplicit-int -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -pedantic-errors -ansi -std=c99 -Wall -ferror-limit=0" make common.o

.PHONY:
clean:
	@cd lib && make $@
	@cd lsb && make $@
	@cd daemons && make $@
