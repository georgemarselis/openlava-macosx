///////////////////////////////////////////////////
//
// lsf/daemons/liblimd/linux.c
// 


#pragma once

#include <dirent.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/sysmacros.h>

#include "daemons/liblimd/common.h"
#include "daemons/liblimd/linux.h"
#include "daemons/liblimd/rload.c"




void
initReadLoad (int checkMode, int *kernelPerm)
{
  float maxmem;
  unsigned long maxSwap;
  struct statvfs fs;
  int stat_fd;

  k_hz = (float) sysconf (_SC_CLK_TCK);

  myHostPtr->loadIndex[R15S] = 0.0;
  myHostPtr->loadIndex[R1M] = 0.0;
  myHostPtr->loadIndex[R15M] = 0.0;

  if (checkMode)
    return;

  if (statvfs ("/tmp", &fs) < 0)
    {
      ls_syslog (LOG_ERR, "%s: statvfs() failed /tmp: %m", __func__);
      myHostPtr->statInfo.maxTmp = 0;
    }
  else
    myHostPtr->statInfo.maxTmp =
      (float) fs.f_blocks / ((float) (1024 * 1024) / fs.f_bsize);

  stat_fd = open ("/proc/stat", O_RDONLY, 0);
  if (stat_fd == -1)
    {
      ls_syslog (LOG_ERR, "%s: open() on /proc/stat failed: %m", __FUNCTION__);
      *kernelPerm = -1;
      return;
    }

  if (read (stat_fd, buffer, sizeof (buffer) - 1) <= 0)
    {
      ls_syslog (LOG_ERR, "%s: read() /proc/stat failed: %m", __FUNCTION__);
      close (stat_fd);
      *kernelPerm = -1;
      return;
    }
  close (stat_fd);
  sscanf (buffer, "cpu  %lf %lf %lf %lf", &prev_cpu_user, &prev_cpu_nice, &prev_cpu_sys, &prev_cpu_idle);

  prev_idle = prev_cpu_idle;
  prev_time = prev_cpu_user + prev_cpu_nice + prev_cpu_sys + prev_cpu_idle;

  if (readMeminfo () == -1)
    return;

  maxmem = main_mem / 1024;
  maxSwap = swap_mem / 1024;

  if (maxmem < 0.0)
    maxmem = 0.0;

  myHostPtr->statInfo.maxMem = maxmem;
  myHostPtr->statInfo.maxSwap = maxSwap;
}

const char *
getHostModel (void)
{
  char model[MAXLSFNAMELEN];
  char buf[128], b1[128], b2[128]; // FIXME FIXME FIXME FIXME FIXME
  int pos = 0;
  int bmips = 0;
  FILE *fp;

  model[pos] = '\0'; // FIXME FIXME FIXME FIXME FIXME
  b1[0] = '\0';      // FIXME FIXME FIXME FIXME FIXME
  b2[0] = '\0';      // FIXME FIXME FIXME FIXME FIXME

  if ((fp = fopen ("/proc/cpuinfo", "r")) == NULL)
    return model;

  while (fgets (buf, sizeof (buf) - 1, fp))
    {

      if (strncasecmp (buf, "cpu\t", 4) == 0
    || strncasecmp (buf, "cpu family", 10) == 0)
  {
    char *p = strchr (buf, ':');
    if (p)
      strcpy (b1, stripIllegalChars (p + 2));
  }
      if (strstr (buf, "model") != 0)
  {
    char *p = strchr (buf, ':');
    if (p)
      strcpy (b2, stripIllegalChars (p + 2));
  }
      if (strncasecmp (buf, "bogomips", 8) == 0)
  {
    char *p = strchr (buf, ':');
    if (p)
      bmips = atoi (p + 2);
  }
    }

  fclose (fp);

  if (!b1[0])
    return model;

  if (isdigit (b1[0]))
    model[pos++] = 'x';

  strncpy (&model[pos], b1, MAXLSFNAMELEN - 15);
  model[MAXLSFNAMELEN - 15] = '\0';
  pos = strlen (model);
  if (bmips)
    {
      pos += sprintf (&model[pos], "_%d", bmips);
      if (b2[0])
  {
    model[pos++] = '_';
    strncpy (&model[pos], b2, MAXLSFNAMELEN - pos - 1);
  }
      model[MAXLSFNAMELEN - 1] = '\0';
    }

  return model;
}



