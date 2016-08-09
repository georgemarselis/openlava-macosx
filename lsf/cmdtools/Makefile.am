#
# b* and l* tools
#

AM_CPPFLAGS = -DSYSCONFDIR=\"${sysconfdir}\" -I$(top_srcdir)/lsf/include

#INCLUDES = -I$(top_srcdir)/lsf/include

SUBDIRS = badmin bbot bbottom bcheckpoint beligible bhc bhist bhosts bjobs bkill \
	bmgroup bmig bmigrate bmigrategroup bmod bmodify bmove bparams bpeek bqc bqueues \
	brequeue brestart bresume brun bstop bsub bswitch btop bugroup busers lib lsacct \
	lsaddhost lsadm lseligible lshosts lsid lsinfo lsload lsloadadj lsmon lsplace \
	lsrcp lsrmhost lsrun

install-data-local:
	cd "$(DESTDIR)/$(bindir)" && ln -sf bkill bstop
	cd "$(DESTDIR)/$(bindir)" && ln -sf bkill bresume
	cd "$(DESTDIR)/$(bindir)" && ln -sf bkill bchkpnt
	cd "$(DESTDIR)/$(bindir)" && ln -sf bmgroup bugroup