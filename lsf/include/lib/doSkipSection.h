// Added by George Marselis <george@marsel.is> Sun April 7 2019

#pragma once

#include <stdio.h>

/* doSkipSection.c */
void doSkipSection_    ( FILE *fp,                  size_t *lineNum, const char *lsfFileName, const char *sectionName );
void doSkipSection_conf( const struct lsConf *conf, size_t *lineNum, const char *lsfFileName, const char *sectionName );
void doSkipSection     ( FILE *fp,                  size_t *lineNum, const char *lsfFileName, const char *sectionName );
