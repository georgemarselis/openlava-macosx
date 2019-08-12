/*
 * Copyright (C) 2011 David Bigagli
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

#include "libint/lsftcl.h"
#include "libint/resreq.h"
// #include "lib/lproto.h"
#include "assert.h"
#include "lsf.h"

// #if defined(HAVE_TCL_TCL_H) // FIXME FIXME FIXME FIXME FIXME this must be set by configure.ac
// #include <tcl/tcl.h>
// #elif defined(HAVE_TCL_H)
#include <tcl.h>
// #endif

/* numericValue()
 * Evaluate host or shared resource numerica value.
 */
int
numericValue( void *clientData, Tcl_Interp *interp, Tcl_Value * args, Tcl_Value * resultPtr )
{
	int *indx   = NULL;
	char *value = NULL;
	float cpuf  = 0.0;
	
	assert( clientData );
	assert( interp );
	assert( args );

	indx = clientData; 			// FIXME ClientData is void *, but it is a bit of wtf why.
								// google "ClientData void *)says it has to do with TCL

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG3, "numericValue: *indx = %d", *indx);
	}

	cpuf = hPtr->cpuFactor;
	resultPtr->type = TCL_INT;
	runTimeDataQueried = TRUE;

	if (*indx < numIndx)
	{

		resultPtr->type = TCL_DOUBLE;
		if (*indx <= R15M)
		{
			resultPtr->doubleValue = hPtr->loadIndex[*indx] * cpuf - 1;
		}
		else
		{
			resultPtr->doubleValue = hPtr->loadIndex[*indx];
			if (hPtr->loadIndex[*indx] >= (INFINIT_LOAD - 10.0) && hPtr->flag != TCL_CHECK_SYNTAX)
			{
				return (TCL_ERROR);
			}
		}

		return TCL_OK;
	}

  if (*indx == CPUFACTOR_)
	{
	  runTimeDataQueried = FALSE;
	  resultPtr->type = TCL_DOUBLE;
	  resultPtr->doubleValue = cpuf;
	}
  else if (*indx == NDISK)
	{

	  runTimeDataQueried = FALSE;
	  resultPtr->intValue = hPtr->nDisks;

	}
  else if (*indx == REXPRI)
	{

	  runTimeDataQueried = FALSE;
	  resultPtr->intValue = hPtr->rexPriority;

	}
  else if (*indx == MAXCPUS_)
	{
	  resultPtr->intValue = hPtr->maxCpus;

	}
  else if (*indx == MAXMEM)
	{

	  resultPtr->intValue = hPtr->maxMem;

	}
  else if (*indx == MAXSWAP)
	{

	  resultPtr->intValue = hPtr->maxSwap;

	}
  else if (*indx == MAXTMP)
	{

	  resultPtr->intValue = hPtr->maxTmp;

	}
  else if (*indx == SERVER)
	{

	  runTimeDataQueried = FALSE;
	  resultPtr->intValue = (hPtr->hostInactivityCount == -1) ? 0 : 1;

	}
  else
	{

	  value = getResValue (*indx - myTclLsInfo->numIndx);
	  if (value == NULL || !strcmp (value, "-"))
	{
	  resultPtr->intValue = 0;
	  return (TCL_OK);
	}

	  resultPtr->doubleValue = atof (value);
	  resultPtr->type = TCL_DOUBLE;

		if (logclass & LC_TRACE) {
			ls_syslog (LOG_DEBUG3, "numericValue():value = %s, clientData =%d", value, *indx);
		}
	}

  return TCL_OK;
}

/* booleanValue()
 * Evaluate host based on shared resource bool.
 */
int
booleanValue (void *clientData, Tcl_Interp *interp, Tcl_Value *args, Tcl_Value *resultPtr)
{
	char *value = NULL;
	int  *idx   = NULL ;
	int isSet   = 0;
  
	assert( NULL != clientData );
	assert( interp );
	assert( args );

	idx = clientData;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG3, "booleanValue: *idx = %d", *idx);
	}
	if (*idx < 0) {
		return TCL_ERROR;
	}

  overRideFromType = TRUE;

  resultPtr->type = TCL_INT;
  if (hPtr->resBitMaps == NULL)
	{
	  resultPtr->intValue = 0;
	  return TCL_OK;
	}

  /* Is a host based resource.
   */
  TEST_BIT (*idx, hPtr->resBitMaps, isSet);
  if (isSet == 1)
	{
	  resultPtr->intValue = isSet;
	  return TCL_OK;
	}

  value = getResValue (*idx);
  if (value == NULL || value[0] == '-')
	{
		if (hPtr->flag == TCL_CHECK_SYNTAX) {
			resultPtr->intValue = 1;
		}
		else {
			resultPtr->intValue = 0;
		}
	}
  else
	{
	  resultPtr->intValue = atoi (value);
	}

  return TCL_OK;
}

/* stringValue()
 * Evaluate host based on shared resource string value.
 */
//int
int
stringValue( void *clientData, Tcl_Interp *interp, int argc, const char *argv[] )
{
	// int code     = 0;
	int *indx    = NULL;
	char *value  = NULL; // FIXME FIXME FIXME FIXME no allocation for char *value, possible buffer overflow
	char *status = NULL; // FIXME FIXME FIXME FIXME this should be malloc'ed
	char *sp     = NULL;
	const char *sp2    = NULL;
	// const char *result = NULL;
	// Tcl_Obj *strObject = NULL;
	// Tcl_Obj *intObject = NULL;
	Tcl_Obj *objv[3]   = { NULL, NULL, NULL };

	struct hostent *hp = NULL;

	assert( interp );
	if (argc != 3)
	{
		Tcl_WrongNumArgs(interp, 3, objv, "Wrong number of arguments; should be exactly 3"); // FIXME FIXME must see if 3 is the correct number to enter
		//Tcl_SetResult( interp, "wrong number of arguments; should be exactly 3", TCL_VOLATILE );
		return TCL_ERROR;
	}

	assert( NULL != clientData );
	indx = clientData;

	if (logclass & LC_TRACE) {
		char buffer[4096] = "";		// FIXME FIXME FIXME use GNU %us extension, in order to not have buffer overflows
		sprintf( buffer, "stringValue: arg0 %s arg1 %s arg2 %s indx %d hostname %s", argv[0], argv[1], argv[2], *indx, hPtr->hostName );
		ls_syslog( LOG_DEBUG3, buffer );
	}

	switch( *indx )
	{
		case HOSTNAME: 
		{
			char *argvSecondItem = malloc( sizeof( char ) * strlen( argv[2] ) + 1 ) ;
			strcpy( argvSecondItem, argv[2] );
			overRideFromType = TRUE;
			sp = hPtr->hostName;
			hp = Gethostbyname_( argvSecondItem );
			if( hp ) {
				sp2 = hp->h_name;
			}
			else {
				sp2 = argvSecondItem;
			}
			free( argvSecondItem );
		}
		break;

		case HOSTTYPE:
		{
			sp = hPtr->hostType;
			if (strcmp (argv[2], LOCAL_STR) == 0)
			{
				sp2 = hPtr->fromHostType;
				if (strcmp (argv[1], "eq") != 0) {
					overRideFromType = TRUE;
				}
			}
			else
			{
				overRideFromType = TRUE;
				sp2 = argv[2];
			}
		}
		break;

		case HOSTMODEL: 
		{
			overRideFromType = TRUE;
			sp = hPtr->hostModel;
			if (strcmp (argv[2], LOCAL_STR) == 0) {
				sp2 = hPtr->fromHostModel;
			}
			else {
				sp2 = argv[2];
			}
		}
		break;

	case HOSTSTATUS:
	  status = NULL;
	  overRideFromType = TRUE;
	  if (LS_ISUNAVAIL (hPtr->status))
	{
	  strcpy (status, "unavail");
	}
	  else if (LS_ISLOCKED (hPtr->status))
	{
	  strcpy (status, "lock");
	  if (LS_ISLOCKEDU (hPtr->status))
		{
		  strcat (status, "U");
		}
	  if (LS_ISLOCKEDW (hPtr->status))
		{
		  strcat (status, "W");
		}
	  if (LS_ISLOCKEDM (hPtr->status))
		{
		  strcat (status, "M");
		}
	}
	  else if (LS_ISBUSY (hPtr->status))
	{
	  strcpy (status, "busy");
	}
	  else
	{
	  strcpy (status, "ok");
	}
	  sp = status;
	  sp2 = argv[2];
	  break;
	default:

	  value = getResValue (*indx - LAST_STRING);
	  if (value == NULL || value[0] == '-')
	{
	  if (hPtr->flag == TCL_CHECK_SYNTAX)
		{
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
			return (TCL_OK);
		}
	  else
		{
		  return (TCL_ERROR);
		}
	}
	  overRideFromType = TRUE;
	  sp = value;
	  sp2 = argv[2];
	  break;
	}

  if (logclass & LC_TRACE)
	{
	  ls_syslog (LOG_DEBUG3, "stringValue: sp = %s, sp2 = %s", sp, sp2);
	}

  if (strcmp (sp2, WILDCARD_STR) == 0)
	{
	  Tcl_SetResult( interp, "1", TCL_VOLATILE );
	  return TCL_OK;
	}

  if (strcmp (argv[1], "eq") == 0)
	{
		if (strcmp (sp2, sp) == 0) {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
		}
		else {
			Tcl_SetResult( interp, "0", TCL_VOLATILE );
		}
	}
  else if (strcmp (argv[1], "ne") == 0)
	{
		if (strcmp (sp2, sp) != 0) {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
		}
		else {
			Tcl_SetResult( interp, "0", TCL_VOLATILE );
		}
	}
  else if (strcmp (argv[1], "ge") == 0)
	{
		if (strcmp (sp2, sp) <= 0) {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
		}
		else {
			Tcl_SetResult( interp, "0", TCL_VOLATILE );
		}
	}
  else if (strcmp (argv[1], "le") == 0)
	{
		if (strcmp (sp2, sp) >= 0) {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
		}
		else {
			Tcl_SetResult( interp, "0", TCL_VOLATILE );
		}
	}
  else if (strcmp (argv[1], "gt") == 0)
	{
		if (strcmp (sp2, sp) < 0) {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
		}
		else {
			Tcl_SetResult( interp, "0", TCL_VOLATILE );
		}
	}
  else if (strcmp (argv[1], "lt") == 0)
	{
		if (strcmp (sp2, sp) > 0) {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
		}
		else {
			Tcl_SetResult( interp, "0", TCL_VOLATILE );
		}
	}
  else
	{
	  return TCL_ERROR;
	}

	return TCL_OK;
}

int
// definedCmd (void *clientData, Tcl_Interp *interp, int argc, const char *argv[])
definedCmd ( void *clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
	unsigned int resNo  = 0;
	int hasRes = FALSE;
	int isSet  = 0;
	int *indx  = NULL ;
	char *value = NULL;
	Tcl_Obj *objv[2] = { NULL, NULL };

	if (argc != 2) {

		Tcl_WrongNumArgs(interp, 2, objv, "Wrong number of arguments; should be exactly 2"); // FIXME FIXME is 2 the right number here?
		//Tcl_SetResult( interp, "Wrong number of args; Should be 2", TCL_VOLATILE );
		return TCL_ERROR;
	}

	assert( clientData );
	indx = clientData; // FIXME FIXME wtf is clientData?

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG3, "%s: argv[0]: %s; argv[1]: %s; indx: %d", __func__, argv[0], argv[1], *indx);
	}

	overRideFromType = TRUE;
	for( resNo = 0; resNo < myTclLsInfo->nRes; resNo++ ) {

	  	if( strcmp( myTclLsInfo->resName[ resNo ], argv[1] ) == 0 )
		{
	  		hasRes = TRUE;
	  		break;
		}
	}

	if (hasRes == FALSE) {
		return TCL_ERROR;
	}

	if (hPtr->resBitMaps == NULL) {

		Tcl_SetResult( interp, "0", TCL_VOLATILE );
		return TCL_OK;
	}

  	TEST_BIT (resNo, hPtr->resBitMaps, isSet);

	if (isSet == 1) {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
	}
	else {
		value = getResValue (resNo);
		if ( !value ) {

			if (hPtr->flag == TCL_CHECK_SYNTAX) {
				Tcl_SetResult( interp, "1", TCL_VOLATILE );
			}
			else {
				Tcl_SetResult( interp, "0", TCL_VOLATILE );
			}
		}
	  	else {
			Tcl_SetResult( interp, "1", TCL_VOLATILE );
	  	}
	}

	return TCL_OK;
}

/* initTcl()
 * Initialize the tcl interpreter for the evaluation
 * of resource requirement expressions.
 */
int
initTcl( struct tclLsInfo *tclLsInfo )
{
	unsigned int i    = 0;
	int isSet = 0; 	// FIXME int or unsigned int?
	static int ar3[5] = { 0, 0, 0, 0, 0 }; // FIXME FIXME initialise values
	attribFunc *funcPtr = NULL;

	static attribFunc attrFuncTable[] = {
		{ "cpu",    R1M },
		{ "login",  LS  },
		{ "idle",   IT  },
		{ "swap",   SWP },
		{ "cpuf",   0   },
		{ "ndisks", 0   },
		{ "rexpri", 0   },
		{ "ncpus",  0   },
		{ "maxmem", 0   },
		{ "maxswp", 0   },
		{ "maxtmp", 0   },
		{ "server", 0   },
		{ NULL,    -1   }
	};

	if (myTclLsInfo)
	{
		freeTclLsInfo (myTclLsInfo, 1);
	}
	if (globinterp)
	{
		Tcl_DeleteInterp (globinterp);
	}
	if (copyTclLsInfo (tclLsInfo) < 0) {
		return -1;
	}

	numIndx = tclLsInfo->numIndx;
	nRes = tclLsInfo->nRes;

	attrFuncTable[4].clientData = CPUFACTOR_;
	attrFuncTable[5].clientData = NDISK;
	attrFuncTable[6].clientData = REXPRI;
	attrFuncTable[7].clientData = MAXCPUS_;
	attrFuncTable[8].clientData = MAXMEM;
	attrFuncTable[9].clientData = MAXSWAP;
	attrFuncTable[10].clientData = MAXTMP;
	attrFuncTable[11].clientData = SERVER;

	globinterp = Tcl_CreateInterp ();

	/* The math functions are invoked by the interpreter
	* while evaluating the expression. The expression itself
	* is made of symbols R15S, R1M etc which we define
	* her as functions together with input (void *)
	* that tcl has to pass them.
	*/
	ar = calloc (tclLsInfo->numIndx + tclLsInfo->nRes, sizeof (int));

	for( i = 0; i < tclLsInfo->numIndx; i++ )
	{
		if (logclass & LC_TRACE) {
			ls_syslog (LOG_DEBUG3, "initTcl:indexNames=%s, i =%d", tclLsInfo->indexNames[i], i);
		}

		ar[i] = i;
		// Tcl_CreateMathFunc( globinterp, tclLsInfo->indexNames[i], 0, NULL, numericValue, (void *) & ar[i] ); // FIXME FIXME FIXME FIXME where on earth did void *come from? what is its value
		Tcl_CreateMathFunc( globinterp, tclLsInfo->indexNames[i], 0, NULL, numericValue, (void *) & ar[i] );
	}

	for( unsigned int resNo = 0; resNo < tclLsInfo->nRes; resNo++ )
	{

		TEST_BIT (resNo, tclLsInfo->numericResBitMaps, isSet);
		if (isSet == 0) {
			continue;
		}

		if (logclass & LC_TRACE) {
			ls_syslog (LOG_DEBUG3, "initTcl:Name=%s, i =%d", tclLsInfo->resName[resNo], i);
		}

		ar[i] = resNo + tclLsInfo->numIndx; // FIXME FIXME FIXME FIXME FIXME this is a subscript overflow.
		Tcl_CreateMathFunc (globinterp, tclLsInfo->resName[resNo], 0, NULL, numericValue, (void *) & ar[i]);
		i++;
	}

	for (funcPtr = attrFuncTable; funcPtr->name != NULL; funcPtr++) // FIXME FIXME FIXME FIXME FIXME erh, where the fuck this is pointing to?
																	// 		why is this not a linked list?
	{
		if (logclass & LC_TRACE)  {
			ls_syslog (LOG_DEBUG3, "initTcl:indexNames=%s, i =%d", funcPtr->name, funcPtr->clientData);
		}

		Tcl_CreateMathFunc (globinterp, funcPtr->name, 0, NULL, numericValue, (void *) & funcPtr->clientData);
	}

	i = 0;
	ar2 = calloc (tclLsInfo->nRes, sizeof (int));
	for( unsigned int resNo = 0; resNo < tclLsInfo->nRes; resNo++, i++)
	{
		TEST_BIT (resNo, tclLsInfo->numericResBitMaps, isSet);
		if (isSet == TRUE) {
			continue;
		}

		TEST_BIT (resNo, tclLsInfo->stringResBitMaps, isSet);
		if (isSet == TRUE) {
			continue;
		}

		ar2[i] = resNo; // FIXME FIXME possible array subscript over-run. check with debugger, to see the 
						// 		values i takes; remove from global and substitute with a for loop declaring
						// 		multiple local variables
		Tcl_CreateMathFunc (globinterp, tclLsInfo->resName[resNo], 0, NULL, booleanValue, (void *) & ar2[i]);
		++i;
	}

	ar3[0] = HOSTTYPE;
	Tcl_CreateCommand (globinterp, "type", stringValue, (void *) & ar3[0], NULL);
	ar3[1] = HOSTMODEL;
	Tcl_CreateCommand (globinterp, "model", stringValue, (void *) & ar3[1], NULL);
	ar3[2] = HOSTSTATUS;
	Tcl_CreateCommand (globinterp, "status", stringValue, (void *) & ar3[2], NULL);
	ar3[3] = HOSTNAME;
	Tcl_CreateCommand (globinterp, "hname", stringValue, (void *) & ar3[3], NULL);

	ar3[4] = DEFINEDFUNCTION;
	Tcl_CreateCommand (globinterp, "defined", definedCmd, (void *) & ar3[4], NULL);

	i = 0;
	ar4 = calloc (tclLsInfo->nRes, sizeof( int ) );  // suspect for overflow

	for ( unsigned int resNo = 0; resNo < tclLsInfo->nRes; resNo++)
	{

		TEST_BIT (resNo, tclLsInfo->stringResBitMaps, isSet);
		if (isSet == FALSE) {
			continue;
		}

		ar4[i] = resNo + LAST_STRING;
		Tcl_CreateCommand (globinterp, tclLsInfo->resName[resNo], stringValue, (void *) & ar4[i], NULL);
		++i;
	}

  return 0;
}

/* evalResReq()
 */
int
evalResReq (char *resReq, struct tclHostData *hPtr2, char useFromType)
{
	int resBits = 0;
	int code    = 0;
	int result  = 0;
	int sscanfResult = 0;

	hPtr = hPtr2;

	overRideFromType = FALSE;
	runTimeDataQueried = FALSE;

	hPtr->overRideFromType = FALSE;

	if (logclass & LC_TRACE) {
		ls_syslog (LOG_DEBUG3, "evalResReq: resReq=%s, host = %s", resReq, hPtr->hostName);
	}

	code = Tcl_Eval( globinterp, resReq );
	sscanfResult = sscanf( 
		(const char *)Tcl_GetObjResult( globinterp ), "%d", &result ); // FIXME is cast here correct?


	if( !sscanfResult || code != TCL_OK || !result )
	{
  		return -1;
	}

	hPtr->overRideFromType = overRideFromType;

	resBits = 0;
	if (!hPtr->ignDedicatedResource && hPtr->DResBitMaps != NULL)
	{
		for ( int i = 0; i < GET_INTNUM (nRes); i++) {
			resBits += hPtr->DResBitMaps[i];
		}

		if (resBits != 0 && hPtr->resBitMaps)
		{
			resBits = 0;
			for ( int i = 0; i < GET_INTNUM (nRes); i++) {
				resBits += hPtr->resBitMaps[i] & hPtr->DResBitMaps[i];
			}

			if (resBits == 0) {
				return 0;
			}
		}
	}

	if (!overRideFromType && useFromType)
	{
		if( strcmp( hPtr->hostType, hPtr->fromHostType ) != 0 ) {
			return 0;
		}
	}

	if (runTimeDataQueried && LS_ISUNAVAIL (hPtr->status)) {
		return 0;
	}

	sscanfResult = sscanf( (const char *)Tcl_GetObjResult( globinterp ), "%d", &result ); // FIXME is cast here correct?

	if( !sscanfResult || !result ) {
	// if( strcmp( itoa( result ), "0" ) == 0 ) { // cute
		return 0;
	}

  return 1;
}

/* getResValue()
 */
char *
getResValue ( unsigned int resNo)
{

	if( logclass & LC_TRACE ) {
		ls_syslog( LOG_DEBUG3, "getResValue:resNo=%d, resName =%s, nRes=%d, hPtr->numResPairs=%d", 
			resNo, myTclLsInfo->resName[ resNo ], myTclLsInfo->nRes, hPtr->numResPairs );
	}

	if( resNo > myTclLsInfo->nRes || hPtr->numResPairs <= 0) {
		return NULL;
	}

	for( int i = 0; i < hPtr->numResPairs; i++ ) // FIXME FIXME FIXME look into hPtr and see if numRespairs member should be unsigned or not
	{
		if( strcmp( hPtr->resPairs[ i ].name, myTclLsInfo->resName[ resNo ] ) == 0 )
		{
			return hPtr->resPairs[ i ].value;
		}
	}

  return NULL;
}

/* copyTcllsInfo()
 */
int
copyTclLsInfo( struct tclLsInfo *tclLsInfo )
{

	myTclLsInfo = calloc( 1, sizeof( struct tclLsInfo ) );

	myTclLsInfo->nRes = tclLsInfo->nRes;
	myTclLsInfo->numIndx = tclLsInfo->numIndx;

	myTclLsInfo->resName           = calloc( myTclLsInfo->nRes, sizeof( char * ) );
	myTclLsInfo->indexNames        = calloc( myTclLsInfo->numIndx, sizeof( char * ) );
	myTclLsInfo->stringResBitMaps  = calloc( GET_INTNUM( myTclLsInfo->nRes ), sizeof( int ) );
	myTclLsInfo->numericResBitMaps = calloc( GET_INTNUM( myTclLsInfo->nRes ), sizeof( int ) );

	for( unsigned int i = 0; i < myTclLsInfo->nRes; i++)
	{
		myTclLsInfo->resName[ i ] = strdup( tclLsInfo->resName[ i ] );
	}

	for( unsigned int i = 0; i < myTclLsInfo->numIndx; i++)
	{
		myTclLsInfo->indexNames[ i ] = strdup( tclLsInfo->indexNames[ i ] );
	}

	for( unsigned int i = 0; i < GET_INTNUM( myTclLsInfo->nRes ); i++ )
	{
		myTclLsInfo->stringResBitMaps[ i ]  = tclLsInfo->stringResBitMaps[ i ];
		myTclLsInfo->numericResBitMaps[ i ] = tclLsInfo->numericResBitMaps[ i ];
	}

  return 0;

}

/* freeTclLsInfo()
 */
void
freeTclLsInfo( struct tclLsInfo *tclLsInfo, int mode )
{

	if (tclLsInfo && 1 == mode )
	{
		// if (mode == 1)
		// {
			for( unsigned int i = 0; i < tclLsInfo->numIndx; i++)
			{
				FREEUP( tclLsInfo->indexNames[ i ] )
			}
			for( unsigned int i = 0; i < tclLsInfo->nRes; i++ )
			{
				FREEUP (tclLsInfo->resName[ i ] )
			}
			FREEUP(tclLsInfo->indexNames );
			FREEUP(tclLsInfo->resName );
			FREEUP(tclLsInfo->stringResBitMaps );
			FREEUP(tclLsInfo->numericResBitMaps );
			FREEUP(tclLsInfo );
		// }			
	}

}
