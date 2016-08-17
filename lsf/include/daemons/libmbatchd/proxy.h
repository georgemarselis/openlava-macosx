/* $Id: proxy.h 397 2007-11-26 19:04:00Z mblack $
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
 
typedef struct proxyListEntry PROXY_LIST_ENTRY_T;

struct proxyListEntry
{
  LIST_ENTRY_T *forw;
  LIST_ENTRY_T *back;
  void *subject;
};

PROXY_LIST_ENTRY_T *proxyListEntryCreate (void *subject);
void proxyListEntryDestroy (PROXY_LIST_ENTRY_T * pxy);


LIST_T *pxyRsvJL;



void proxyUSJLAttachObsvr (void);
void proxyUSJLAddEntry (struct jData *job);

void proxyHSJLAttachObsvr (void);
void proxyHSJLAddEntry (struct jData *job);

void proxyHRsvJLAddEntry (struct jData *job);
void proxyHRsvJLRemoveEntry (struct jData *job);

void proxyRsvJLAddEntry (struct jData *job);
void proxyRsvJLRemoveEntry (struct jData *job);


bool_t proxyListEntryEqual (PROXY_LIST_ENTRY_T * pxy, void *subject, int hint);

bool_t pendJobPrioEqual (PROXY_LIST_ENTRY_T * pxy, struct jData *subjectJob, int hint);

bool_t startJobPrioEqual (PROXY_LIST_ENTRY_T * pxy, struct jData *subjectJob, int hint);


#define JOB_PROXY_GET_JOB(pxy) ((struct jData *)(pxy)->subject)

struct jData *jobProxyGetPendJob (PROXY_LIST_ENTRY_T * pxy);
void jobProxySyslog (PROXY_LIST_ENTRY_T * pxy, void *hint);
char *jobProxySprintf (PROXY_LIST_ENTRY_T * pxy, void *hint);

