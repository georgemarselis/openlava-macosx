/* $Id: jgrp.h 397 2007-11-26 19:04:00Z mblack $
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

#include "intlib/jidx.h"


#define DESTROY_REF(x, y) { \
     y(x);\
     x = NULL;\
}

#define    JGRP_VOID         -1

typedef void (*FREE_JGARRAY_FUNC_T) (void *);

#define JG_ARRAY_BASE \
    int      numRef;\
    int      status;\
    time_t   changeTime;\
    int      oldStatus;\
    int      userId;\
    char    *userName;\
    int      fromPlatform;\
    void    (*freeJgArray)(void *);\
    int      counts[NUM_JGRP_COUNTERS+1];

struct jgArrayBase
{
JG_ARRAY_BASE};

struct jarray
{
  JG_ARRAY_BASE struct jData *jobArray;
  int maxJLimit;
};

struct jgrpData
{
JG_ARRAY_BASE};


#define JOB_DATA(node)   ((struct jData *)node->ndInfo)
#define JGRP_DATA(node)   ((struct jgrpData *)node->ndInfo)
#define ARRAY_DATA(node)   ((struct jarray *)node->ndInfo)

struct jgTreeNode
{
  struct jgTreeNode *parent, *child, *left, *right;
  int nodeType;
  char *name;
  void *ndInfo;
};


typedef enum treeEventType
{
  TREE_EVENT_ADD,
  TREE_EVENT_CLIP,
  TREE_EVENT_CTRL,
  TREE_EVENT_NULL
} TREE_EVENT_TYPE_T;


struct treeObserver;
typedef void (*TREE_EVENT_OP_T) (struct treeObserver *, void *,
				 TREE_EVENT_TYPE_T);
struct treeObserver
{
  struct treeObserver *forw;
  struct treeObserver *back;
  char *name;
  void *entry;
  TREE_EVENT_OP_T eventOp;
};

typedef struct treeObserver TREE_OBSERVER_T;



char treeFile[];


struct nodeList
{
  int isJData;
  void *info;
};

struct jgTreeNode *groupRoot;

void treeInit ();
struct jgTreeNode *treeLexNext (struct jgTreeNode *);
struct jgTreeNode *treeLinkSibling (struct jgTreeNode *,
					   struct jgTreeNode *);
struct jgTreeNode *treeInsertChild (struct jgTreeNode *,
					   struct jgTreeNode *);
void treeInsertLeft (struct jgTreeNode *, struct jgTreeNode *);
void treeInsertRight (struct jgTreeNode *, struct jgTreeNode *);
struct jgTreeNode *treeClip (struct jgTreeNode *);
struct jgTreeNode *treeNewNode (int);
void treeFree (struct jgTreeNode *);
int isAncestor (struct jgTreeNode *, struct jgTreeNode *);
int isChild (struct jgTreeNode *, struct jgTreeNode *);
char *parentGroup (char *);
char *parentOfJob (char *);
void initObj (char *, int);
char *parentGroup (char *);
struct jgArrayBase *createJgArrayBaseRef (struct jgArrayBase *);
void destroyJgArrayBaseRef (struct jgArrayBase *);
int getIndexOfJStatus (int);
void updJgrpCountByJStatus (struct jData *, int, int);
void putOntoTree (struct jData *, int jobType);
void printTreeStruct (char *);
int jgrpPermitOk (struct lsfAuth *, struct jgTreeNode *);
int selectJgrps (struct jobInfoReq *, void **, int *);
void updJgrpCountByOp (struct jgTreeNode *, int);
char *myName (char *);
void freeJarray (struct jarray *);
void checkJgrpDep (void);
int inIdxList (size_t, struct idxList *);
int matchName (char *, char *);
struct jgTreeNode *treeNextSib (struct jgTreeNode *);
char *jgrpNodeParentPath (struct jgTreeNode *);
char *fullJobName (struct jData *);
int jgrpNodeParentPath_r (struct jgTreeNode *, char *);
void fullJobName_r (struct jData *, char *);
int updLocalJData (struct jData *, struct jData *);
int localizeJobElement (struct jData *);
int localizeJobArray (struct jData *);
