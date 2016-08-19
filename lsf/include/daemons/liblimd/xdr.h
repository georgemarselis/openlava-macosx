

#pragma once

bool_t xdr_loadvector (XDR * xdrs,	struct loadVectorStruct *lvp, struct LSFHeader *hdr);
u_short encfloat16_ (float);
float decfloat16_ (u_short);
void freeResPairs (struct resPair *, uint num);
bool_t xdr_resPair (XDR *, struct resPair *, struct LSFHeader *);
bool_t xdr_loadmatrix (XDR * xdrs, int len, struct loadVectorStruct * lmp,	struct LSFHeader * hdr);
bool_t xdr_masterReg (XDR * xdrs, struct masterReg * masterRegPtr, struct LSFHeader * hdr);
bool_t xdr_statInfo (XDR * xdrs, struct statInfo * sip, struct LSFHeader * hdr);
u_short encfloat16_ (float f);
float decfloat16_ (u_short sf);
