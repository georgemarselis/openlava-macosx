// added by George Marselis <george@marsel.is> Tuesday March 26

#pragma once


char do_Hosts_lsf (          FILE * fp, const char *filename, size_t *lineNum, struct lsInfo *info );
char do_Hosts_lsb (struct lsConf *conf, const char *filename, size_t *lineNum, struct lsInfo *info, int options);
