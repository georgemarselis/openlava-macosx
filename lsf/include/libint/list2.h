#pragma once

struct list_
{
	struct list_ *forw;
	struct list_ *back;
	unsigned int num;
	char padding[4];
	char *name;
};

#define LIST_NUM_ENTS(L) ((L)->num)
typedef void (*LIST_DATA_DESTROY_FUNC_T) (void *);

struct list_ *listpop (struct list_ *);

