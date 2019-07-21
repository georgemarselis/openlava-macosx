// Created by George Marselis <george@marsel.is> Sun July 21 2019 21:43

#pragma once

// #include "lsf.h"

/* lsf.c */
void lsf_c_bullshit ( void );
long int tosec      ( long int tv_usec );
void ls_ruunix2lsf  ( struct rusage *rusage, struct lsfRusage *lsfRusage );
void ls_rulsf2unix  ( struct lsfRusage *lsfRusage, struct rusage *rusage );
int  lsfRu2Str      ( FILE *log_fp, struct lsfRusage *lsfRu );
int  str2lsfRu      ( char *line, struct lsfRusage *lsfRu, int *ccount );
void lsfRusageAdd_  ( struct lsfRusage *lsfRusage1, struct lsfRusage *lsfRusage2 );
void cleanLsfRusage ( struct lsfRusage *lsfRusage );
void cleanRusage    ( struct rusage *rusage);
