// Added by George Marselis <george@marsel.is> Wed Apr 10 2019

#pragma once

/* #define SIGEMT SIGBUS */

/* nios.c */
void nios_c_bullshit(void);
int ls_stdinmode(int onoff);
int ls_donerex(void);
int ls_stoprex(void);
int ls_niossync(long numTasks);
int ls_setstdout(int on_, char *format);
int ls_setstdin(int on_, int *rpidlist, size_t len);
size_t ls_getstdin(int on_, unsigned long *rtaskidlist, long maxlen);

