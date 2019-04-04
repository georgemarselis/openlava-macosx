// Added by George Marselis <george@marsel.is> Thu Apr 4 2019

#pragma once

#include "libint/lsi18n.h"

/*static int I18nRunningFlag = 0;
static int I18nInitFlag = 0;
*/
int ls_catd;


/*static int i18n_ct_format_ID[] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7
};*/

/*#define NL_SETN         35*/

const char *i18n_ct_format[] = {
    "%Ec\n",              /* catgets 1 */
    "%a %b %d %T %Y",     /* catgets 2 */
    "%b %d %T %Y",        /* catgets 3 */
    "%a %b %d %T",        /* catgets 4 */
    "%b %d %H:%M",        /* catgets 5 */
    "%m/%d/%Y",           /* catgets 6 */
    "%H:%M:%S"            /* catgets 7 */
};

//#undef NL_SETN
const char *i18nCTFormatStr[MAX_CTIME_FORMATID + 1];


/* i18n.c */
int _i18n_init(int modId);
int _i18n_end(void);
void _i18n_ctime_init(int catID);
char *_i18n_ctime(int catID, int formatID, const time_t *timer);
char *_i18n_printf(const char *format, ...);
