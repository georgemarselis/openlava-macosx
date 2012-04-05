#
# Copyright (C) 2012 David Bigagli
#
INCLUDES = -I$(top_srcdir)/lsf -I. -I..

noinst_PROGRAMS = asyncflow gethostbyaddr hdr  qstring server \
	client gethostbyname msg readevent sobufsiz absub

asyncflow_SOURCS = asyncflow.c
asyncflow_LDADD = -lpthread

gethostbyaddr_SOURCES = gethostbyaddr.c

gethostbyname_SOURCES = gethostbyname.c

hdr_SOURCES = hdr.c

qstring_SOURCES = qstring.c

server_SOURCES = server.c proto.h

client_SOURCES = client.c proto.h

msg_SOURCES = msg.c

absub_SOURCES = absub.c
absub_LDADD = ../lsbatch/lib/liblsbatch.a ../lsf/lib/liblsf.a ../lsf/intlib/liblsfint.a -lnsl

readevent_SOURCES = readevent.c
readevent_LDADD =../lsbatch/lib/liblsbatch.a ../lsf/lib/liblsf.a ../lsf/intlib/liblsfint.a -lnsl 

sobufsiz_SOURCES = sobufsiz.c

