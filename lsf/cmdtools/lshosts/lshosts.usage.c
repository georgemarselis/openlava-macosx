#include <stdio.h>

#include "intlib/intlibout.h"

void lshosts_usage (char *);

void
lshosts_usage (char *cmd)
{
  fprintf (stderr,
	   "%s: %s [-h] [-V] [-w | -l] [-R res_req] [host_name ...]\n",
	   I18N_Usage, cmd);
  fprintf (stderr, "%s\n %s [-h] [-V] -s [static_resouce_name ...]\n",
	   I18N_or, cmd);
}
