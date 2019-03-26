// Added by George Marselis <george@marsel.is> Tue March 26 2019

#pragma once

/* lib/liblsf/getnextline.c */
char *getNextLine_conf(struct lsConf *conf, int confFormat);
char *getNextLineD_(FILE *fp, size_t *LineCount, int confFormat);
char *getNextLineC_(FILE *fp, size_t *lineCount, int confFormat);
char *getNextLineC_conf(struct lsConf *conf, size_t *LineCount, int confFormat);
