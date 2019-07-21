// Created by George Marselis <george@marsel.is> Sun July 21 2019 21:35 

#pragma once

/* reslog.c */
int ls_putacctrec(FILE *log_fp, struct lsfAcctRec *acctRec);
struct lsfAcctRec *ls_getacctrec(FILE *log_fp, size_t *lineNum);
