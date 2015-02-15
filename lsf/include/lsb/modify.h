/* $Id: lsb.sub.c 397 2007-11-26 19:04:00Z mblack $
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



extern int getCommonParams (struct submit *, struct submitReq *, struct submitReply *);
extern int getOtherParams (struct submit *, struct submitReq *,  struct submitReply *, struct lsfAuth *, LSB_SUB_SPOOL_FILE_T *);
extern char *getSpoolHostBySpoolFile (const char *spoolFile);

static LS_LONG_INT sendModifyReq (struct modifyReq *, struct submitReply *, struct lsfAuth *);
static int esubValModify (struct submit *);
extern void makeCleanToRunEsub ();
extern void modifyJobInformation (struct submit *);

