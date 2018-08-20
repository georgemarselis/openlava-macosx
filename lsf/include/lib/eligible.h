/* $Id: lib.eligible.c 397 2007-11-26 19:04:00Z mblack $
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

char *ls_resreq      ( const char *task);
int   ls_eligible    ( const char *task, const char *resreqstr, const char mode);
long  inittasklists_ ( void );
int   readtaskfile_  ( const char *filename, struct hTab *minusListl, struct hTab *minusListr, struct hTab *localList, struct hTab *remoteList, const char useMinus);
int   writetaskfile_ ( const char *filename, struct hTab *minusListl, struct hTab *minusListr, struct hTab *localList, struct hTab *remoteList);
int   ls_insertrtask ( const char *task );
int   ls_insertltask ( const char *task );
void  inserttask_    ( const char *taskstr, struct hTab *tasktb);
int   ls_deletertask ( const char *task );
int   ls_deleteltask ( const char *task );
int   deletetask_    ( const char *taskstr, struct hTab *tasktb );
long  ls_listrtask   ( char ***taskList, int sortflag );
long  ls_listltask   ( char ***taskList, int sortflag );
long  listtask_      ( char ***taskList, struct hTab *tasktb, int sortflag );
int   tcomp_         ( const void *tlist1, const void *tlist2 );

