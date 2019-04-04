/* $Id: lib.sig.c 397 2007-11-26 19:04:00Z mblack $
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
 
#include "lib/lib.h"
#include "lib/sig.h"
// #include "lsb/sig.h" // deprecated


int
sig_encode (int sig)
{
	unsigned int i = 0;

	if (sig < 0) {
		return sig;
	}

	for( i = 0; i < NSIG_MAP; i++ ) {
		if (sig_map[i] == sig) {
			break;
		}
	}

	if (i == NSIG_MAP)  // FIXME FIXME FIXME the cast has to go; does sig below need to be negative at any point? if no, remove cast, change function param
	{
		if (sig >= (int) NSIG_MAP) {
			return sig;
		}
		else {
			return 0;
		}
	}
	else {
		return i;
	}
}

int
sig_decode (int sig)
{
	if (sig < 0) {
		return sig;
	}

	if (sig >= (int) NSIG_MAP)  // FIXME FIXME FIXME the cast has to go; does sig below need to be negative at any point? if no, remove cast, change function param
	{
		if (sig < SIGRTMAX) {  // NSIG is in <signal.h>
			return sig;
		}
		else
		{
			return 0;
		}
	}

	return sig_map[sig];
}

int
getSigVal (char *sigString)
{
	int sigVal = 0;
	char sigSig[16] = ""; // FIXME FIXME why 16?

	if (sigString == NULL) {
		return -1;
	}
	if (sigString[0] == '\0') {
		return -1;
	}

	if (isint_ (sigString) == TRUE)
	{
		if ((sigVal = atoi (sigString)) > SIGRTMAX) {
			return -1;
		}
		else {
			return sigVal;
		}
	}

	for ( unsigned int i = 0; i < NSIG_MAP; i++)
	{
		sprintf (sigSig, "%s%s", "SIG", sigSymbol[i]);
		if ((strcmp (sigSymbol[i], sigString) == 0) || (strcmp (sigSig, sigString) == 0)) {
			return sig_map[i];
		}
	}
	return -1;

}

char *
getSigSymbolList (void)
{
	char *list = malloc( sizeof( list ) * 512 + 1 );
	for( unsigned int i = 1; i < NSIG_MAP; i++ )
	{
		strcat (list, sigSymbol[i]);
		strcat (list, " ");
	}
	return list;

}

SIGFUNCTYPE Signal_ (int sig, void (*handler) (int))
{
	struct sigaction act, oact;

	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, sig);

	if (sigaction (sig, &act, &oact) == -1) {
		oact.sa_handler = (void (*)()) SIG_ERR;
	}

	return oact.sa_handler;
}

char *
getSigSymbol (int sig)
{
	char *symbol = malloc( sizeof(*symbol)* 30 + 1 );
	if (sig < 0 || sig >= (int) NSIG_MAP) { // FIXME FIXME FIXME find out if sig can ever take negative values, then alter code appropriatelly
		strcpy (symbol, "UNKNOWN");
	}
	else {
		strcpy (symbol, sigSymbol[sig]);
	}
	
	return symbol;
}

int
blockALL_SIGS_ (sigset_t *newMask, sigset_t *oldMask)
{
	sigfillset (newMask);
	sigdelset (newMask, SIGTRAP);
	sigdelset (newMask, SIGEMT);
	return sigprocmask (SIG_BLOCK, newMask, oldMask);
}
