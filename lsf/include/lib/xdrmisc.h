// Added by George Marselis <george@marsel.is> Thu Apr 4 2019

#pragma once

/* xdrmisc.c */
bool_t xdr_LSFlong(XDR *xdrs, long *l);
bool_t xdr_stat(XDR *xdrs, struct stat *st, struct LSFHeader *hdr);
bool_t xdr_lsfRusage(XDR *xdrs, struct lsfRusage *lsfRu);
bool_t xdr_var_string(XDR *xdrs, char **astring);
bool_t xdr_lenData(XDR *xdrs, struct lenData *ld);
bool_t xdr_lsfAuth(XDR *xdrs, struct lsfAuth *auth, struct LSFHeader *hdr);
bool_t my_xdr_float(XDR *xdrs, float *fp);
int xdr_lsfAuthSize(struct lsfAuth *auth);
bool_t xdr_pidInfo(XDR *xdrs, struct pidInfo *pidInfo, struct LSFHeader *hdr);
bool_t xdr_jRusage(XDR *xdrs, struct jRusage *runRusage, struct LSFHeader *hdr);
