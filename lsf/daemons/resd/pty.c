/* $Id: res.pty.c 397 2007-11-26 19:04:00Z mblack $
 * Copyright (C) 2007 Platform Computing Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "daemons/libresd/resd.h"
#include "libint/lsi18n.h"

// FIXME FIXME FIXME FIXME FIXME http://www.iakovlev.org/index.html?p=1169&m=1
#define TIOCGPTN  _IOR('T',0x30, unsigned int) /* Get Pty Number (of pty-mux device) */

// #define NL_SETN      29  // FIXME FIXME FIXME FIXME take NL_SETN out

 int grantpt (int);
 int unlockpt (int);
 char *ptsname (int);
 int ptymaster (char *line);
char *pty_translate (char *pty_name); // FIXME FIXME why pty_translate?
int ptyslave (char *tty_name);
int check_valid_tty (char *tty_name);

static int letterInd = 0;  // FIXME FIXME FIXME FIXME FIXME adjust type of globas
static int digitInd = 0;

void
ptyreset (void)
{
  letterInd = 0;
  digitInd = 0;
}


int
ptymaster (char *line)
{
  static char __func__] = "ptymaster()";
  int master_fd = 0;
  int ptyno = 0;
  char *slave = NULL;


  master_fd = open ("/dev/ptmx", O_RDWR);
  if (master_fd < 0)
  {
      ls_syslog (LOG_ERR, I18N_FUNC_S_FAIL_M, __func__, "open", "/dev/ptmx");
      return (-1);
  }
  if (grantpt (master_fd) < 0)
  {
      ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "grantpt", master_fd);
      close (master_fd);
      return (-1);
  }

  if (unlockpt (master_fd) < 0)
  {
      ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "unlockpt", master_fd);
      close (master_fd);
  }
#ifdef __CYGWIN__
  slave = ptsname (master_fd);
  if (slave == NULL)
  {
      ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "ptsname");
      close (master_fd);
      return (-1);
  }
  strcpy (line, slave);
#else

    #if !defined(__sun__)
      if (ioctl (master_fd, TIOCGPTN, &ptyno) != 0)
      {
    #endif
      ls_syslog (LOG_DEBUG, I18N_FUNC_FAIL_M, __func__, "ioctl(TIOCGPTN)");
      slave = ptsname (master_fd);
      if (slave == NULL)
      {
          ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "ptsname");
          close (master_fd);
          return (-1);
      }
      strcpy (line, slave);
#if !defined(__sun__)
  }
  else
  {
      sprintf (line, "/dev/pts/%d", ptyno);
  }
#endif
#endif
  return (master_fd);
}

int
ptyslave (char *tty_name)
{
  int slave;


  slave = open (tty_name, O_RDWR);

  if (slave < 0)
  {
      return (-1);
  }

  return (slave);
}



char *
pty_translate (char *pty_name)
{
  static char tmp[11] = "/dev/ttyXX";
  int n = strlen (pty_name);

  tmp[8] = pty_name[n - 2];
  tmp[9] = pty_name[n - 1];

  if (debug > 1) {
    printf ("%s -> %s\n", pty_name, tmp);
}


return tmp;
}



int
check_valid_tty (char *tty_name)
{
  int i;
  char valid_name[9] = "/dev/tty"; // FIXME FIXME FIXME FIXME remote fixed string
                                    // FIXME FIXME FIXME FIXME FIXME what the heck is the fixed string doing here??

  for (i = 0; i < 8; i++) {
    if (tty_name[i] != valid_name[i]) {
      return 0;
  }
}
return 1;
}
