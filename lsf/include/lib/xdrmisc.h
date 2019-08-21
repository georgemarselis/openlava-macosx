// Added by George Marselis <george@marsel.is> Thu Apr 4 2019

#pragma once

#include "lib/header.h"

/* xdrmisc.c */
bool_t xdr_LSFlong(XDR *xdrs, long *l);
bool_t xdr_stat(XDR *xdrs, struct stat *st, struct LSFHeader *hdr);
bool_t xdr_lsfRusage(XDR *xdrs, struct lsfRusage *lsfRusage);
bool_t xdr_var_string(XDR *xdrs, char **astring);
bool_t xdr_lenData(XDR *xdrs, struct lenData *ld);
bool_t xdr_lsfAuth(XDR *xdrs, struct lsfAuth *auth, struct LSFHeader *hdr);
size_t xdr_lsfAuthSize(struct lsfAuth *auth);
bool_t xdr_pidInfo(XDR *xdrs, struct pidInfo *pidInfo, struct LSFHeader *hdr);
bool_t xdr_jRusage(XDR *xdrs, struct jRusage *runRusage, struct LSFHeader *hdr);
bool_t xdr_enum_kind( XDR *xdrs, enum CLIENT_AUTH *auth );
bool_t xdr_struct_timeval( XDR *xdrs, struct timeval *timeval );
