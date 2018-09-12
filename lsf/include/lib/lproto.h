/* $Id: lproto.h 397 2007-11-26 19:04:00Z mblack $
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

// #include "lsf.h"
// #include "lib/conf.h"
// #include "lib/hdr.h"
// #include "lib/table.h"
// #include "lib/xdrrf.h"
// #include "libint/lsi18n.h"
// #include "daemons/libpimd/pimd.h"
// #include "daemons/libresd/resd.h"
// #include "daemons/libresd/resout.h"

#define VALID_IO_ERR(x) ((x) == EWOULDBLOCK || (x) == EINTR || (x) == EAGAIN)
#define BAD_IO_ERR(x)   ( ! VALID_IO_ERR(x))


#define FREEUP(pointer)   if (pointer != NULL) {  \
							  free(pointer);      \
							  pointer = NULL;     \
						  } // FIXME FIXME FIXME FIXME this macro has to go

#define STRNCPY(str1, str2, len)  { strncpy(str1, str2, len); \
									str1[len -1] = '\0';  \
								  }

#define IS_UNC(a) \
		((a!=NULL) && (*a == '\\') && (*(a+1) == '\\') ? TRUE : FALSE)

#define TRIM_LEFT(sp) if (sp != NULL) { \
						  while (isspace(*(sp))) (sp)++; \
					  }
#define TRIM_RIGHT(sp)     while (isspace(*(sp+strlen(sp)-1))) *(sp+strlen(sp)-1)='\0';

#define ALIGNWORD_(s)    (((s)&0xfffffffc) + 4)
// #define NET_INTADDR_(a) ((char *) (a))

// #define NET_INTSIZE_ 4

#define XDR_DECODE_SIZE_(a) (a)

#define GET_INTNUM(i) ((i)/INTEGER_BITS + 1)
#define SET_BIT(bitNo, integers)           \
	integers[(bitNo)/INTEGER_BITS] |= (1<< (bitNo)%INTEGER_BITS);
#define CLEAR_BIT(bitNo, integers)           \
	integers[(bitNo)/INTEGER_BITS] &= ~(1<< (bitNo)%INTEGER_BITS);
#define TEST_BIT(bitNo, integers, isSet)  \
   {  \
	  if (integers[(bitNo)/INTEGER_BITS] & (1<<(bitNo)%INTEGER_BITS))  \
		  isSet = 1;         \
	  else                   \
		  isSet = 0;         \
   }

#define FOR_EACH_WORD_IN_SPACE_DELIMITED_STRING(String, Word) \
	if ((String) != NULL) { \
		char *Word; \
		while (((Word) = getNextWord_(&String)) != NULL) { \

#define END_FOR_EACH_WORD_IN_SPACE_DELIMITED_STRING }}

enum {
	LSF_O_RDONLY    = 00000,
	LSF_O_WRONLY    = 00001,
	LSF_O_RDWR      = 00002,
	LSF_O_NDELAY    = 00004,
	LSF_O_NONBLOCK  = 00010,
	LSF_O_APPEND    = 00020,
	LSF_O_CREAT     = 00040,
	LSF_O_TRUNC     = 00100,
	LSF_O_EXCL      = 00200,
	LSF_O_NOCTTY    = 00400,
	LSF_O_CREAT_DIR = 04000
} LSF_O;

struct admins
{
	char padding[4];
	unsigned int nAdmins;
	uid_t *adminIds;
	gid_t *adminGIds;
	char **adminNames; // FIXME FIXME FIXME FIXME locate all instances of struct, see how char **adminNames gets malloc'ed
};


struct resPair
{
	const char *name;
	const char *value;
};

struct sharedResource
{
	const char *resourceName;
	unsigned int numInstances;
	const char padding1[4];
	struct resourceInstance **instances;
};

struct resourceInfoReq
{
	unsigned int numResourceNames;
	int options;
	const char *hostname;
	char **resourceNames;
};

struct resourceInfoReply
{
	unsigned int numResources;
	unsigned int badResource;
	struct lsSharedResourceInfo *resources;
};


// #define HOST_ATTR_SERVER        (0x00000001)
// #define HOST_ATTR_CLIENT        (0x00000002)
// #define HOST_ATTR_NOT_LOCAL     (0x00000004)
// #define HOST_ATTR_NOT_READY     (0xffffffff)
const unsigned long HOST_ATTR_SERVER    = 0x00000001;
const unsigned long HOST_ATTR_CLIENT    = 0x00000002;
const unsigned long HOST_ATTR_NOT_LOCAL = 0x00000004;
const unsigned long HOST_ATTR_NOT_READY = 0xffffffff;

// #define AUTH_IDENT          "ident"
// #define AUTH_PARAM_DCE      "dce"
// #define AUTH_PARAM_EAUTH    "eauth"
// #define AUTOMOUNT_LAST_STR  "AMFIRST"
// #define AUTOMOUNT_NEVER_STR "AMNEVER"
const char AUTH_IDENT[]          = "ident";
const char AUTH_PARAM_DCE[]      = "dce";
const char AUTH_PARAM_EAUTH[]    = "eauth";
const char AUTOMOUNT_LAST_STR[]  = "AMFIRST";
const char AUTOMOUNT_NEVER_STR[] = "AMNEVER";


const char LSF_LIM_ERESOURCE_OBJECT[]   =  "liblimvcl.so";
const char LSF_LIM_ERESOURCE_VERSION[]  = "lim_vcl_get_eres_version";
const char LSF_LIM_ERESOURCE_DEFINE[]   = "lim_vcl_get_eres_def";
const char LSF_LIM_ERESOURCE_LOCATION[] = "lim_vcl_get_eres_loc";
const char LSF_LIM_ERESOURCE_VALUE[]    = "lim_vcl_get_eres_val";
const char LSF_LIM_ERES_TYPE[]          = "!";



const char LS_EXEC_T[] = "LS_EXEC_T";

char *lsTmpDir_ = NULL;

const unsigned short BIND_RETRY_TIMES = 100;
int sharedResConfigured_;
