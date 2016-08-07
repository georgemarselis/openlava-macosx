/*
 * Copuright (C) 2011 David Bigagli
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

struct config_param genParams_[] = {
	{"LSF_CONFDIR", NULL},
	{"LSF_SERVERDIR", NULL},
	{"LSF_LIM_DEBUG", NULL},
	{"LSF_RES_DEBUG", NULL},
	{"LSF_STRIP_DOMAIN", NULL},
	{"LSF_LIM_PORT", NULL},
	{"LSF_RES_PORT", NULL},
	{"LSF_LOG_MASK", NULL},
	{"LSF_SERVER_HOSTS", NULL},
	{"LSF_AUTH", NULL},
	{"LSF_USE_HOSTEQUIV", NULL},
	{"LSF_ID_PORT", NULL},
	{"LSF_RES_TIMEOUT", NULL},
	{"LSF_API_CONNTIMEOUT", NULL},
	{"LSF_API_RECVTIMEOUT", NULL},
	{"LSF_AM_OPTIONS", NULL},
	{"LSF_TMPDIR", NULL},
	{"LSF_LOGDIR", NULL},
	{"LSF_SYMBOLIC_LINK", NULL},
	{"LSF_MASTER_LIST", NULL},
	{"LSF_MLS_LOG", NULL},
	{"LSF_INTERACTIVE_STDERR", NULL},
	{"HOSTS_FILE", NULL},
	{"LSB_SHAREDIR", NULL},
	{NULL, NULL}
};

char *getTempDir_ (void);
static int parseLine (char *line, char **keyPtr, char **valuePtr);
static int matchEnv (char *, struct config_param *);
static int setConfEnv (char *, char *, struct config_param *);
int initenv_ (struct config_param *userEnv, char *pathname);

