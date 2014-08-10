
INCLUDES = -I$(top_srcdir)/lsf -I$(top_srcdir)/lsbatch

noinst_PROGRAMS = client gethostbyaddr gethostbyname \
                  hdr msg pimme qstring readevent server sobufsiz

client_SOURCES = client.c proto.h
gethostbyaddr_SOURCES = gethostbyaddr.c
gethostbyname_SOURCES = gethostbyname.c
hdr_SOURCES = hdr.c
msg_SOURCES = msg.c
pimme_SOURCES = pimme.c
qstring_SOURCES = qstring.c
readevent_SOURCES = readevent.c
readevent_LDADD = ../lsbatch/lib/liblsbatch.a ../lsf/lib/liblsf.a -lnsl
server_SOURCES = server.c proto.h
sobufsiz_SOURCES = sobufsiz.c

etags :
	etags *.[hc]

