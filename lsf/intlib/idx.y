/* $Id: idx.y,v 1.1 2007/07/18 23:17:45 cchen Exp $
 ***********************************************************************
 *              Job Array indicies  YYACC Parser
 *                    -- IndParser --
 ***********************************************************************
 */
%token ICON     1
%token SZZZZ    40
%define parse.error verbose

%{
int yylex();
int yyerror(const char *);
%}

%{
# include <stdio.h>
# include <stdlib.h>
#   include "libint/yparse.h"
# include "libint/jidx.h"
# include "libint/lsi18n.h"
// MOD_LSF_INTLIB  22  MsgId 550-599
# define NL_SETN 22

#define YYALLOC(x) yyalloc(&idxAllocHead, (x))
struct mallocList  *idxAllocHead = NULL;
int idxerrno = IDX_NOERR;

struct idxList **idxList = NULL;
int *maxJLimit = 0;

%}

%union  {
		int ival ;
		struct idxList  idxType;
		struct idxList  *idxPtr;
		}

// /opt/local/bin/bison --warnings=all --feature=all --verbose --locations --token-table
%left ','         // FIXME FIXME FIXME FIXME bison warning here
%nonassoc '!'     // FIXME FIXME FIXME FIXME bison warning here
%right '='        // FIXME FIXME FIXME FIXME bison warning here
%left  OR         // FIXME FIXME FIXME FIXME bison warning here
%left '+' '-' '/' '*'  // FIXME FIXME FIXME FIXME bison warning here
%right AND        // FIXME FIXME FIXME FIXME bison warning here

%type <idxType> index region
%type <idxPtr> indicies arrayInd
%type <ival> icon
%%

arrayIndSpec   : arrayInd
			   {
				   *idxList = $1;
				   yparseSucc(&idxAllocHead);
				}
				| arrayInd '%' icon
				{
				   *idxList = $1;
				   *maxJLimit = $3;
					yparseSucc(&idxAllocHead);
				}
arrayInd    : '[' indicies ']'
		{
				   $$ = $2;
		}
indicies     : index
				{
				   if (($$ = (struct idxList *)
							 YYALLOC(sizeof(struct idxList))) == NULL) {
					   idxerror(_i18n_msg_get(ls_catd, NL_SETN, 550,
						  "No Memory")); /* catgets 550 */
					   idxerrno = IDX_MEM;
					   YYABORT;
				   }
				   $$->start = $1.start;
				   $$->end = $1.end;
				   $$->step = $1.step;
				   $$->next = NULL;
				}
				| index ',' indicies
				{
				   if (($$ = (struct idxList *) YYALLOC(sizeof(struct idxList))) == NULL) {
					   idxerror(_i18n_msg_get(ls_catd, NL_SETN, 550, "No Memory"));
					   idxerrno = IDX_MEM;
					   YYABORT;
				   }
				   $$->start = $1.start;
				   $$->end = $1.end;
				   $$->step = $1.step;
				   $$->next = $3;
				}
index           : region
				{
			$$.start = $1.start;
					$$.end   = $1.end;
					$$.step  = 1;
					if ($$.start < 1 || $$.start > $$.end || $$.step <= 0) {
						/* catgets 551 */
						idxerror( "551: boundary error" );
						idxerrno = IDX_BOUND;
						YYABORT;
					}
				}
				| region ':' icon
				{
			$$.start = $1.start;
					$$.end   = $1.end;
					$$.step  = $3;
					if ($$.start < 1 || $$.start > $$.end || $$.step <= 0) {
						idxerror( "551: boundary error" );
						idxerrno = IDX_BOUND;
						YYABORT;
					}
				}
region          : icon
				{
					$$.start = $1;
					$$.end  = $$.start;
					$$.step = 1;
				};
				| '-' icon
				{
					$$.start = 1;
					$$.end  = $2;
					$$.step = 1;
				};
				| icon '-'
				{
					$$.start = $1;
					$$.end  = INFINIT_INT;
					$$.step = 1;
				};
				| icon '-' icon
				{
					$$.start = $1;
					$$.end  = $3;
					$$.step = 1;
				}
icon        : ICON
		{
			$$ = atoi(token);
		}
%%
