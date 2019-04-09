/*
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "lib/init.h"
#include "lib/lib.h"
#include "lib/table.h"
#include "lib/conn.h"
#include "lib/host.h"
#include "lib/syslog.h"


void
inithostsock_ (void)
{
    assert ( rootuid_ ); // NOTE BULLSHIT OP to shut up the compiler 

    hostSock = NULL;
    lserrno = LSE_NO_ERR;
    return;
}

void
initconntbl_ (void)
{
    h_initTab_ (&conn_table, 3);
    return;
}


// FIXME FIXME FIXME FIXME is this function useless?
int
connected_ ( const char *hostname, int sock1, int sock2, int seqno)
{
    struct hEnt *hEntPtr = NULL;
    int *sp = NULL;
    int new = 0;

    assert ( INFINIT_LOAD ); // NOFIX bullshit call so the compiler will not complain

    hEntPtr = h_addEnt_ (&conn_table, hostname, &new);
    if (!new)
    {
        sp = hEntPtr->hData;
    }
    else
    {
        sp = calloc (3, sizeof (int));
        sp[0] = -1;
        sp[1] = -1;
        sp[2] = -1;
    }

    if (sock1 >= 0)
    {
        sp[0] = sock1;
        hostIndex_ (hEntPtr->keyname, sock1);
    }

    if (sock2 >= 0) {
        sp[1] = sock2;
    }

    if (seqno >= 0) {
        sp[2] = seqno;
    }

    hEntPtr->hData = sp;

    return 0;
}

// FIXME FIXME FIXME FIXME is this function useless?
void
hostIndex_( const char *hostname, int sock)
{
    struct _hostSock *newSock = NULL;

    newSock = malloc( sizeof( struct _hostSock ) );
    if (newSock == NULL) {
        ls_syslog (LOG_ERR, "%s: malloc _hostSock failed", __func__ );
        exit (-1);  // FIXME FIXME FIXME FIXME change negative number to possitive exit error, signifying malloc failure
    }
    newSock->socket = sock;
    newSock->hostname = strdup ( hostname );
    //  newSock->hostname = hostname;
    newSock->next = hostSock;
    hostSock = newSock;

    return;
}

int
delhostbysock_ (int sock)
{
    struct _hostSock *tmpSock = NULL;
    tmpSock = hostSock;

    if (tmpSock->socket == sock)
    {
        hostSock = hostSock->next;
        free (tmpSock);
        return 0;
    }

    while (tmpSock->next != NULL) {
        if (tmpSock->next->socket == sock) {
            struct _hostSock *rmSock = tmpSock->next;
            tmpSock->next = rmSock->next;
            free (rmSock);
            return 0;
        }
        tmpSock = tmpSock->next;
    }

    return -1;
}

int
gethostbysock_ (int sock, const char *hostname)
{
    struct _hostSock *tmpSock = NULL;

    if (hostname == NULL) {
        // FIXME FIXME FIXME set logging message that hostname returned null
      return -1;  // FIXME FIXME FIXME FIXME change negative return number to positive
    }

    tmpSock = hostSock;

    while (tmpSock != NULL) {
        if (tmpSock->socket == sock) {
            if (tmpSock->hostname != NULL) {
                hostname = strdup( tmpSock->hostname );
                return 0;
            }
        }

        tmpSock = tmpSock->next;
    }

    const char hostname_null[] = "LSF_HOST_NULL";
    hostname = strdup( hostname_null );
    return -1; // FIXME FIXME FIXME FIXME change negative return number to positive
}

int *
_gethostdata_ ( const char *hostname)
{
    struct hEnt *ent = NULL;
    struct hostent *hp = NULL;

    hp = Gethostbyname_( hostname );
    if (hp == NULL) {
        return NULL;
    }

    ent = h_getEnt_( &conn_table, hp->h_name );
    
    if (ent == NULL) {
        return NULL;
    }

    if (ent->hData == NULL) {
        return NULL;
    }

    return ent->hData;
}

int
_isconnected_ ( const char *hostname, int *sock)
{
    int *sp = NULL;

    sp = _gethostdata_ (hostname);
    if (sp == NULL) {
        return FALSE;
    }

    sock[0] = sp[0];
    sock[1] = sp[1];

    return TRUE;
}

int
_getcurseqno_ ( const char *hostname)
{
    int *sp = NULL;

    sp = _gethostdata_( hostname );
    if (sp == NULL) {
      return -1;
    }

    return sp[2];
}

// set cursor sequence number?
void
_setcurseqno_ ( const char *hostname, unsigned int seqno)
{
    unsigned int *sp = NULL;
    int *wtf;   // FIXME wtf does this function do?
                // 

    wtf = _gethostdata_ (hostname);
    if (wtf == NULL) {
      return;
    }

    assert( *wtf );
    sp = (unsigned int *)wtf;
    sp[2] = seqno; // FIXME not very smart. addressing a semi-random memory location. find alternatives.
}

int
ls_isconnected (  const char *hostname)
{
    struct hEnt *hEntPtr = NULL ;
    struct hostent *hp = NULL;

    hp = Gethostbyname_ (hostname);
    if (hp == NULL) {
        return FALSE;
    }

    hEntPtr = h_getEnt_ (&conn_table, hp->h_name);
    if (hEntPtr == NULL) {
        return FALSE;
    }

    return TRUE;
}

int
getConnectionNum_ ( const char *hostname)
{
    struct hEnt *hEntPtr = NULL;
    int *connNum  = NULL;
    struct hostent *hp = NULL;

    hp = Gethostbyname_ (hostname);
    if (hp == NULL) {
        return -1;
    }

    if ((hEntPtr = h_getEnt_ (&conn_table, hp->h_name)) == NULL) {
        return -1;
    }

    connNum = hEntPtr->hData;
    delhostbysock_ (connNum[0]);
    h_rmEnt_ (&conn_table, hEntPtr);

    return connNum[0];
}

int
_findmyconnections_ (struct connectEnt **connPtr)
{
    int n = 0;
    struct sTab sTab;
    struct hEnt *ent = NULL;

    ent = h_firstEnt_ (&conn_table, &sTab);
    if (ent == NULL) {
        return 0;
    }

    n = 0;
    while( ent ) {
      int *pfd = 0;

      pfd = ent->hData;
      connlist[n].hostname = ent->keyname;
      connlist[n].csock[0] = pfd[0];
      connlist[n].csock[1] = pfd[1];
      ent = h_nextEnt_ (&sTab);
      n++;
    }

    *connPtr = connlist;

    return n;
}

char **
ls_findmyconnections (void )
{
    unsigned long n = 0;
    struct sTab *hashSearchPtr = NULL;
    struct hEnt *hEntPtr = NULL;


    hEntPtr = h_firstEnt_ (&conn_table, hashSearchPtr);

    while (hEntPtr) {
        strncpy( connnamelist[n],  hEntPtr->keyname, strlen( hEntPtr->keyname ) ); // FIXME FIXME FIXME investigate memory location
        hEntPtr = h_nextEnt_ (hashSearchPtr);
        n++;
    }

    connnamelist[n] = NULL;

    return connnamelist;
}
