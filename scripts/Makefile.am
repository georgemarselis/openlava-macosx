#
# Copyright (C) openlava foundation
#

# include $(top_srcdir)/common.mk
# contencts of common.mk
AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\"


# Install scripts implementing integration with 
# mpi and openmpi systems.
 
bin_SCRIPTS = lam-mpirun mpich2-mpiexec mpich-mpirun openmpi-mpirun \
	esync

EXTRA_DIST = $(bin_SCRIPTS)
