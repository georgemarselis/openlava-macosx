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

static size_t errLineNum_ = 0;

char *getTempDir_(void);
int initenv_(struct config_param *userEnv, char *pathname);
int ls_readconfenv(struct config_param *paramList, char *confPath);
int readconfenv_(struct config_param *pList1, struct config_param *pList2, char *confPath);
int parseLine(char *line, char **keyPtr, char **valuePtr);
int matchEnv(char *name, struct config_param *paramList);
int setConfEnv(char *name, char *value, struct config_param *paramList);

