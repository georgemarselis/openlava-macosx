/* $Id: lib.conf.h 397 2007-11-26 19:04:00Z mblack $
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

#pragma once

#include "lib/conf.h"


// #define NL_SETN   23
/* confmisc.c */
char *getNextValue      (        char           **line );
int  keyMatch           (        struct keymap   *keyList   , const char     *line,             int         exact);
int  isSectionEnd       (        char            *linep     , const char     *lsfile,           size_t     *lineNum, const char *sectionName );
char *getBeginLine      (        FILE            *fp        ,       size_t   *lineNum );
int  readHvalues        (        struct keymap   *keyList   ,       char     *linep   ,         FILE       *fp,      const char *lsfile, size_t *lineNum, int exact, const char *section );
void doSkipSection      (        FILE            *fp        ,       size_t   *lineNum,    const char       *lsfile,  const char *sectionName );
int  mapValues          (        struct keymap   *keyList   ,       char     *line     );
int  isInlist           ( const  char           **adminNames, const char     *userName,         unsigned  int actAds );
char *getBeginLine_conf ( const  struct lsConf   *conf      ,       size_t   *lineNum  );
void doSkipSection_conf ( const  struct lsConf   *conf      ,       size_t   *lineNum,    const char    *lsfile,     const char *sectionName );
