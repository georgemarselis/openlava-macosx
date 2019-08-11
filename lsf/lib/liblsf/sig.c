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
#include "lib/misc.h"
// #include "lsb/sig.h" // deprecated


unsigned int
sig_encode (unsigned int sig)
{
    unsigned int i = 0;

    // if (sig < 0) {
    //     return sig;
    // }

    for( i = 0; i < NSIG_MAP; i++ ) {
        if (sig_map[i] == sig) {
            break;
        }
    }

    if (i == NSIG_MAP)  // FIXME FIXME FIXME the cast has to go; does sig below need to be negative at any point? if no, remove cast, change function param
    {
        if (sig >= NSIG_MAP) {
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

unsigned int
sig_decode (unsigned int sig)
{
    // if (sig < 0) {
    //     return sig;
    // }

    if (sig >= NSIG_MAP) { // FIXME FIXME FIXME the cast has to go; does sig below need to be negative at any point? if no, remove cast, change function param
        if ( (int)sig < SIGRTMAX) {  // NSIG is in <signal.h>
            return sig;
        }
        else {
            return 0;
        }
    }

    return sig_map[sig];
}

unsigned int
getSigVal ( const char *sigString)
{
    unsigned int sigVal = 0;
    char sigSig[16] = ""; // FIXME FIXME why 16?

    if (sigString == NULL) {
        return UINT_MAX;
    }
    if (sigString[0] == '\0') { // FIXME FIXME why did i put this here?
        return UINT_MAX;
    }

    if (isint_ (sigString) == TRUE)
    {
        sigVal = (unsigned int) atoi (sigString);
        if ( (int) sigVal > SIGRTMAX) {
            return UINT_MAX;
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

    return UINT_MAX;
}

char *
getSigSymbolList (void)
{
    char *list = malloc( sizeof( char ) * 512 + 1 ); // FIXME FIXME FIXME why 521?
    for( unsigned int i = 1; i < NSIG_MAP; i++ ) {
        strcat (list, sigSymbol[i]);
        strcat (list, " ");
    }
    return list;

}

SIGFUNCTYPE Signal_ ( unsigned int sig, void (*handler) (int))
{
    struct sigaction act;
    struct sigaction oact;

    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset (&act.sa_mask);
    sigaddset (&act.sa_mask,  (int)sig);

    if (sigaction ((int)sig, &act, &oact) == -1) {
        oact.sa_handler = (void (*)()) SIG_ERR;
    }

    return oact.sa_handler;
}

char *
getSigSymbol ( unsigned int sig)
{
    char *symbol = malloc( sizeof(*symbol)* 30 + 1 );
    // if (sig < 0 || sig >= (int) NSIG_MAP) { // FIXME FIXME FIXME find out if sig can ever take negative values, then alter code appropriatelly
    if ( (int)sig >= (int) NSIG_MAP) { // FIXME FIXME FIXME find out if sig can ever take negative values, then alter code appropriatelly
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
#ifdef __NO_INTEL__
    sigdelset (newMask, SIGEMT);
#endif
    return sigprocmask (SIG_BLOCK, newMask, oldMask);
}
