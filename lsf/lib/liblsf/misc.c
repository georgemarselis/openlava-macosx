 /* $Id: lib.misc.c 397 2007-11-26 19:04:00Z mblack $
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
#include "lib/syslog.h"
#include "lib/misc.h"
#include "lib/host.h"
#include "lib/info.h"
#include "libint/lsi18n.h"

char
isanumber_ ( char *word )
{
    char **eptr = NULL;
    double number = 0.0F;

    if (!word || *word == '\0') {
        return FALSE;
    }

    if (errno == ERANGE) {
        errno = 0;
    }

    eptr = &word;
    number = strtod (word, eptr);
    if (**eptr == '\0' && errno != ERANGE) {
        if (number <= LONG_MAX && number > LONG_MIN ) {
            return TRUE;
        }
    }

    return FALSE;
}

char
islongint_ ( const char *word)
{
    long number = 0;

    if (!word || *word == '\0') {
        return FALSE;
    }

    if (!isdigitstr_ (word)) {
        return FALSE;
    }

    if (errno == ERANGE) {
        errno = 0;
    }

    sscanf (word, "%ld", &number);
    if (errno != ERANGE) {
        if (number <= INFINIT_LONG_INT && number > -INFINIT_LONG_INT) {
            return TRUE;
        }
    }

    return FALSE;
}

int
isdigitstr_ (  const char *string)
{
    for( size_t i = 0; i < strlen( string ); i++ ) {

        if(!isdigit (string[i])) {
            return FALSE;
        }
    }

    return TRUE;
}

ssize_t
atoi64_( const char *word )
{
    ssize_t number = 0;
    char *tempword = NULL;

    if (!word || *word == '\0') {
        return 0;
    }

    if (errno == ERANGE) {
        errno = 0;
    }

    tempword = strdup( word );
    sscanf (tempword, "%ld", &number);
    if (errno != ERANGE) {
        if (number <= INFINIT_LONG_INT && number > -INFINIT_LONG_INT) {
            return number;
        }
    }

    return 0;
}

unsigned int
isint_ ( const char *word)
{
    char **eptr = NULL;
    long number = 0;
    char *tempword = NULL;

    if (!word || *word == '\0') {
        return FALSE;
    }

    if (errno == ERANGE) {
        errno = 0;
    }

    tempword = strdup( word );
    eptr = &tempword;
    number = strtol (word, eptr, 10);
    if (**eptr == '\0' && errno != ERANGE) {
        if (number <= INFINIT_INT && number > -INFINIT_INT) {
            return TRUE;
        }
    }

    return FALSE;
}

char *
putstr_ (const char *s)
{
    register char *p = NULL;

    if( NULL == s) {
        s = "";
    }

    p = malloc (strlen (s) + 1);
    if (!p) {
        return NULL;
    }

    strcpy (p, s);

    return p;
}

short
getRefNum_ (void)
{
    static short reqRefNum = MIN_REF_NUM;

    reqRefNum++;
    if (reqRefNum >= MAX_REF_NUM) {
        reqRefNum = MIN_REF_NUM;
    }

    return reqRefNum;
}

char *
chDisplay_ (char *disp)
{
    char *sp = NULL;
    char *hostName = NULL;
    static char dspbuf[MAXHOSTNAMELEN + 10];

    sp = disp + 8;
    if (strncmp ("unix:", sp, 5) == 0) {
        sp += 4;
    }
    else if (strncmp ("localhost:", sp, 10) == 0) {
        sp += 9;
    }

    if (sp[0] == ':') {
        
        if ((hostName = ls_getmyhostname ()) == NULL){
            return disp;
        }
        
        sprintf (dspbuf, "%s=%s%s", "DISPLAY", hostName, sp);
        
        return dspbuf;
    }

  return disp;
}

// void
// strToLower_ (const char *name)
// {
//     while (*name != '\0') {
//         assert( tolower( *name ) > 0 );
//         *name = (char) tolower (*name);
//         name++;
//     }

// }

char *
getNextToken (char **sp)
{
    static char word[MAX_LINE_LEN];
    char *cp;

    if (!*sp) {
        return NULL;
    }

    cp = *sp;
    if (cp[0] == '\0') {
        return NULL;
    }

    if (cp[0] == ':' || cp[0] == '=' || cp[0] == ' ') {
        *sp += 1;
    }

    cp = *sp;
    if (cp[0] == '\0') {
        return NULL;
    }

    strcpy (word, cp);
    if ((cp = strchr (word, ':')) != NULL) {
        *cp = '\0';
    }

    if ((cp = strchr (word, '=')) != NULL) {
        *cp = '\0';
    }

    *sp += strlen (word);
    
    return word;

}

int
getValPair (char **resReq, int *val1, int *val2)
{
    char *token = NULL;
    char *cp    = NULL;
    char *wd1   = NULL;
    char *wd2   = NULL;
    size_t len  = 0;

    *val1 = INFINIT_INT;
    *val2 = INFINIT_INT;

    token = getNextToken (resReq);
    if (!token) {
        return 0;
    }
    
    len = strlen (token);
    if (len == 0) {
        return 0;
    }
    
    cp = token;
    while (*cp != '\0' && *cp != ',' && *cp != '/') {
        cp++;
    }
    
    if (*cp != '\0') {
        
        *cp = '\0';        
        if (cp - token > 0) {
            wd1 = token;
        }
        
        assert( cp - token >= 0);
        if ( (size_t)(cp - token) < len - 1) {
            wd2 = ++cp;
        }
    }
    else {
        wd1 = token;
    }

    if (wd1 && !isint_ (wd1)) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (wd2 && !isint_ (wd2)) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (!wd1 && !wd2) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (wd1) {
        *val1 = atoi (wd1);
    }

    if (wd2) {
        *val2 = atoi (wd2);
    }

    return 0;
}


const char *
my_getopt (int nargc, char **nargv, char *ostr, char **errMsg)
{
    char svstr[256];
    char *cp1 = svstr;
    char *cp2 = svstr;
    char *optName;
    size_t i = 0; 
    int num_arg = 0;
    const char *BADCH = ":";

    if ((optName = nargv[optind]) == NULL) {
        return NULL;
    }

    if (optind >= nargc || *optName != '-') {
        return NULL;
    }

    if (optName[1] && *++optName == '-') { // FIXME FIXME find what optName[1] is and replace 1 with label

        ++optind;
        return NULL;
    }

    if (ostr == NULL) {
        return NULL;
    }

    strcpy (svstr, ostr);
    num_arg = 0;
    optarg = NULL;
    while (*cp2) {

        size_t cp2len = strlen (cp2);
        for (i = 0; i < cp2len; i++) {
            if (cp2[i] == '|') {
                num_arg = 0;
                cp2[i] = '\0';
                break;
            }
            else if (cp2[i] == ':') {
                num_arg = 1;
                cp2[i] = '\0';
                break;
            }
        }
      
        if (i >= cp2len) {  // FIXME FIXME refactoring. 'i' should not be seen outside the loop
            return BADCH;
        }

        if (!strcmp (optName, cp1)) {

            if (num_arg) {

                if (nargc <= optind + 1) {
                    /* catgets 650 */
                    if (errMsg == NULL) {                   
                        fprintf(stderr, "catgets 650: %s: option requires an argument -- %s\n", nargv[0], optName);   
                    } 
                    else { 
                        sprintf(*errMsg, "catgets 650: %s: option requires an argument -- %s\n", nargv[0], optName);   
                    }
                    return BADCH;
                }

                optarg = nargv[++optind];
            }

            ++optind;
            
            return optName;
        }
        else if (!strncmp (optName, cp1, strlen (cp1))) {
        
            if (num_arg == 0) {
                     /* catgets 651 */
                    if (errMsg == NULL) {                   
                        fprintf(stderr, "catgets 651: %s: option cannot have an argument -- %s\n", nargv[0], cp1);   
                    } 
                    else { 
                        sprintf(*errMsg, "catgets 651: %s: option cannot have an argument -- %s\n", nargv[0], cp1);   
                    }
                    return BADCH;
            }

            optarg = optName + strlen (cp1);
            ++optind;
            
            return cp1;
        }

        cp1 = &cp2[i];
        cp2 = ++cp1;
    }
    /* catgets 652 */
    if (errMsg == NULL) {                   
        fprintf(stderr, "catgets 652: %s: illegal option -- %s\n", nargv[0], optName);   
    } 
    else { 
        sprintf(*errMsg, "catgets 652: %s: illegal option -- %s\n", nargv[0], optName);   
    }
    
    return BADCH;
}

int
putEnv (char *env, char *val)
{
    char *buf = malloc ( (strlen (env) + strlen (val))*sizeof(char) + 4);
    if (buf == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    sprintf (buf, "%s=%s", env, val);

    return putenv (buf);
}

void
initLSFHeader_ (struct LSFHeader *hdr)
{
    memset (hdr, 0, sizeof (struct LSFHeader));
    hdr->version = OPENLAVA_VERSION;

    return;
}

void *
myrealloc (void *ptr, size_t size)
{
    if (ptr == NULL) {
        return malloc (size);
    }
    else {
        return realloc (ptr, size);
    }

    return NULL;
}

int
Bind_ (int sockfd, struct sockaddr *myaddr, socklen_t addrlen)
{
    struct sockaddr_in *cliaddr = NULL;
    unsigned int port = 0;


    cliaddr = (struct sockaddr_in *) myaddr;  // FIXME FIXME FIXME FIXME
                                              //    enroll in union, present appropiate part of unon
    if (cliaddr->sin_port != 0)
        return bind (sockfd, myaddr, addrlen);
    else {
        for ( unsigned int i = 1; i <= BIND_RETRY_TIMES; i++) {
            if (bind (sockfd, (struct sockaddr *) cliaddr, addrlen) == 0) {
                return 0;
            }
            else {
                if (errno == EADDRINUSE) {
                    if (i == 1) {
                        port = (unsigned short) (time (0) | getpid ());
                        port = ((port < 1024) ? (port + 1024) : port);
                    }
                    else {
                        port++;
                        port = ((port < 1024) ? (port + 1024) : port);
                    }
                    /* catgets 5650 */
                    ls_syslog (LOG_ERR, "catgets 5650: %s: retry <%d> times, port <%d> will be bound" , "Bind_", i, port);
                    cliaddr->sin_port = htons ( (uint16_t) port);
                }
                else {
                    return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
                }
            }
        }
        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, "Bind_", "bind", BIND_RETRY_TIMES);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    return INT_MAX;
}

char *
getCmdPathName_ (const char *cmdStr, size_t *cmdLen)
{
    char *pRealCmd = NULL;
    char *sp1      = NULL;
    char *sp2      = NULL;
    unsigned int i = 0;

    for(  pRealCmd = strdup(cmdStr); *pRealCmd == ' ' ||  *pRealCmd == '\t' || *pRealCmd == '\n'; pRealCmd++ ) 
        {;}

    if (pRealCmd[0] == '\'' || pRealCmd[0] == '"') {
        sp1 = &pRealCmd[1];
        sp2 = strchr (sp1, pRealCmd[0]);
    }
    else {
        sp1 = pRealCmd;
        for ( i = 0; sp1[i] != '\0'; i++) {
            if( sp1[i] == ';' || sp1[i] == ' ' || sp1[i] == '&' || sp1[i] == '>' || sp1[i] == '<' || sp1[i] == '|' || sp1[i] == '\t' || sp1[i] == '\n')  {
                break; 
            }
        }

        sp2 = &sp1[i];
    }

    if (sp2) {
        assert( sp2 - sp1 > 0 );
        *cmdLen = (size_t)(sp2 - sp1);
    }
    else {
        assert( strlen (sp1) <= INT_MAX );
        *cmdLen = strlen (sp1);
    }

    return sp1;
}

int replace1stCmd_ (const char *oldCmdArgs, const char *newCmdArgs, char *outCmdArgs, size_t outLen)
{
    const char *sp1     = NULL;
    const char *sp2     = NULL;
    const char *sp3     = NULL;
    char *newSp   = NULL;
    char *curSp   = NULL;
    size_t len    = 0;
    size_t len2   = 0;
    size_t newLen = 0;
  
    sp1 = oldCmdArgs;
    sp2 = getCmdPathName_ (sp1, &len2);
    newSp = getCmdPathName_ (newCmdArgs, &newLen);

    if ( newLen - len2 + strlen (sp1) >= outLen) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    sp3 = sp2 + len2;

    assert( sp2 - sp1 > 0 );
    len = (size_t)(sp2 - sp1);
    curSp = memcpy (outCmdArgs, sp1, len);
    curSp = memcpy (curSp + len, newSp, newLen);
    strcpy (curSp + newLen, sp3);

    return 0;
}

char *
getLowestDir_ (const char *filePath)
{
    const char *sp1 = NULL;
    const char *sp2 = NULL;
    char *dirName = malloc( sizeof( char ) * MAX_FILENAME_LEN + 1 );
    size_t len = 0;

    sp1 = strrchr (filePath, '/');
    if (sp1 == NULL) {
        sp1 = filePath;
    }
    
    sp2 = strrchr (filePath, '\\');
    if (sp2 == NULL) {
        sp2 = filePath;
    }
    
    assert(sp2 - sp1 > 0);
    len = (size_t) (sp2 > sp1) ? (size_t) (sp2 - filePath) : (size_t)(sp1 - filePath);

    if (len) {
        memcpy (dirName, filePath, len);
        if( '\0' != dirName[len - 1] ) {
          dirName[len - 1 ] = '\0';       // FIXME FIXME FIXME FIXME FIXME bug waiting to happen!
        }

    }
    else {
      return NULL;
    }

    return dirName;
}

void
getLSFAdmins_ (void)
{
    struct clusterInfo *clusterInfo = NULL;


    clusterInfo = ls_clusterinfo (NULL, NULL, NULL, 0, 0);
    if (clusterInfo == NULL) {
      return;
    }

    if (LSFAdmins.numAdmins != 0) {
        FREEUP (LSFAdmins.names);
    }

    LSFAdmins.numAdmins = clusterInfo->nAdmins;
    LSFAdmins.names = calloc (LSFAdmins.numAdmins, sizeof (char *));
    if ( NULL == LSFAdmins.names) {
        LSFAdmins.numAdmins = 0;
        return;
    }

    for ( unsigned int i = 0; i < LSFAdmins.numAdmins; i++) {
        LSFAdmins.names[i] = putstr_ (clusterInfo->admins[i]);
        
        if (LSFAdmins.names[i] == NULL) {

            for ( unsigned  int j = 0; j < i; j++) {
                FREEUP (LSFAdmins.names[j]);
            }
            
            FREEUP (LSFAdmins.names);
            LSFAdmins.numAdmins = 0;

            return;
        }
    }
}

bool_t
isLSFAdmin_ (const char *name)
{
    for ( unsigned int i = 0; i < LSFAdmins.numAdmins; i++) {

        if (strcmp (name, LSFAdmins.names[i]) == 0) {
            return TRUE;
        }
    }

  return FALSE;
}

int
ls_strcat ( char *trustedBuffer, size_t bufferLength, const char *strToAdd)
{
    size_t start = strlen (trustedBuffer);
    // size_t remainder = bufferLength - start;
    // size_t i = 0;

    if ((start > bufferLength) || strToAdd == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    strcpy( trustedBuffer, strToAdd);
    // for (i = 0; i < remainder; i++) {
    //     trustedBuffer[start + i] = strToAdd[i];
    //     if (strToAdd[i] == '\0') {
    //         break;
    //     }
    // }

    // if (i == remainder) {
    //     trustedBuffer[bufferLength - 1] = '\0';
    //     return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    // }

    return 0;
}

/* Every even has a header like that:
 * EVENT_TYPE openlavaversion unixtime
 */


/* ls_readeventrec()
 */
struct lsEventRec *
ls_readeventrec (FILE * fp)
{
    static struct lsEventRec ev;
    static char ebuf[BUFSIZ];
    int cc = 0;
    char *p = NULL;

    if (fp == NULL) {
        lserrno = LSE_BAD_ARGS;
        return NULL;
    }

    if (!fgets (ebuf, BUFSIZ, fp)) {
        lserrno = LSE_EOF;
        return NULL;
    }

    p = ebuf;
    cc = readEventHeader (ebuf, &ev);
    if (cc < 0) {
        lserrno = LSE_FILE_CLOSE;
        return NULL;
    }
    /* move ahead since we consumed
     * the header.
     */
    p = p + cc;

    switch (ev.event)
    {
        case EV_LIM_START: 
        {
            ev.record = NULL;
        }
        break;
        case EV_LIM_SHUTDOWN:
        {
            ev.record = NULL;
        }
        break;
        case EV_ADD_HOST:
        {
            readAddHost (p, &ev);
        }
        break;
        case EV_REMOVE_HOST:
        {
            readRmHost (p, &ev);
        }
        break;
        case EV_EVENT_LAST:
        {
            ;
        }
        break;
        default:
        {
            ;
        }
        break;
    }

    return &ev;
}

/* ls_writeeventrec()
 */
int
ls_writeeventrec (FILE * fp, struct lsEventRec *ev)
{
    if (fp == NULL || ev == NULL)
    {
        lserrno = LSE_BAD_ARGS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    writeEventHeader (fp, ev);

    switch (ev->event)
    {
        case EV_LIM_START:
        {
            fputc ('\n', fp);
        }
        break;
        case EV_LIM_SHUTDOWN:
        {
            fputc ('\n', fp);
        }
        break;
        case EV_ADD_HOST:
        {
            writeAddHost (fp, ev);
        }
        break;
        case EV_REMOVE_HOST:
        {
            writeRmHost (fp, ev);
        }
        break;
        case EV_EVENT_LAST:
        {
            ;
        }
        break;
        default:
        {
            ;
        }
        break;
    }

    fflush (fp);

    return 0;
}

/* writeEventHeader()
 */
int
writeEventHeader (FILE * fp, struct lsEventRec *ev)
{
    if (ev->event < EV_LIM_START || ev->event >= EV_EVENT_LAST) {
        lserrno = LSE_BAD_ARGS;
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    fprintf (fp, "%s %hu %ld", emap[ev->event], ev->version, ev->etime);

    fflush (fp);

    return 0;
}

/* writeAddHost()
 */
int
writeAddHost (FILE * fp, struct lsEventRec *ev)
{
    struct hostEntryLog *hPtr = NULL;

    hPtr = ev->record;

    fprintf (fp, "\"%s\" \"%s\" \"%s\" %d %u %4.2f %u ", hPtr->hostName, hPtr->hostModel, hPtr->hostType, hPtr->rcv, hPtr->nDisks, hPtr->cpuFactor, hPtr->numIndx);

    for( size_t cc = 0; cc < hPtr->numIndx; cc++) {
        fprintf (fp, "%4.2lf",  hPtr->busyThreshold[cc] );
    }

    fprintf (fp, "%u ", hPtr->nRes);

    for( size_t cc = 0; cc < hPtr->nRes; cc++) {
        fprintf (fp, "\"%s\" ", hPtr->resList[cc]);
    }

    /* the window is really the only string that needs
     * to have quotes around as there can be spaces in it
     * or it may not be configured and does not have
     * a counter.
     */
    fprintf (fp, "%d \"%s\" ", hPtr->rexPriority, hPtr->window);

    /* new line and flush
     */
    fprintf (fp, "\n");
    fflush (fp);

    return 0;
}

/* writermHost
 */
int
writeRmHost (FILE * fp, struct lsEventRec *ev)
{
    struct hostEntryLog *hLog = ev->record;

    fprintf (fp, "\"%s\" \n", hLog->hostName);
    fflush (fp);

    return 0;
}

/* readEventHeader()
 */
int
readEventHeader (char *buf, struct lsEventRec *ev)
{
    int n = 0;
    int cc = 0;
    char event[32];

    memset( event, '\0', strlen( event ) );

    cc = sscanf (buf, "%s %hu %ld %n", event, &ev->version, &ev->etime, &n);
    if (cc != 3) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (strcmp ("LIM_START", event) == 0) {
        ev->event = EV_LIM_START;
    }
    else if (strcmp ("LIM_SHUTDOWN", event) == 0) {
        ev->event = EV_LIM_SHUTDOWN;
    }
    else if (strcmp ("ADD_HOST", event) == 0) {
        ev->event = EV_ADD_HOST;
    }
    else if (strcmp ("REMOVE_HOST", event) == 0) {
        ev->event = EV_REMOVE_HOST;
    }
    else {
        abort( );
    }

    return n;
}

/* readAddHost()
 */
int
readAddHost ( const char *buf, struct lsEventRec *ev)
{
    int cc = 0;
    int n = 0;
    // int i;
    struct hostEntryLog *hPtr = NULL;
    static char name[MAX_LSF_NAME_LEN + 1];
    static char model[MAX_LSF_NAME_LEN + 1];
    static char type[MAX_LSF_NAME_LEN + 1];
    char *p = NULL;
    char *window = NULL;

    hPtr = calloc (1, sizeof (struct hostEntryLog));
    p = strdup( buf );
    /* name, model, type, number of disks
     * and cpu factor.
     */
    cc = sscanf (p, "%s%s%s%d%u%lf%u%n", name, model, type, &hPtr->rcv, &hPtr->nDisks, &hPtr->cpuFactor, &hPtr->numIndx, &n);
    if (cc != 7) {
        free (hPtr);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    p = p + n;

    strcpy (hPtr->hostName, getstr_ (name));
    strcpy (hPtr->hostModel, getstr_ (model));
    strcpy (hPtr->hostType, getstr_ (type));

    /* load indexes
     */

    hPtr->busyThreshold = calloc ( (unsigned long) hPtr->numIndx, sizeof (float));
    for( unsigned int i = 0; i < hPtr->numIndx; i++) {
        cc = sscanf (p, "%lf%n", &hPtr->busyThreshold[i], &n);
        if (cc != 1) {
            // goto out
            FREEUP (hPtr->busyThreshold);
            FREEUP (hPtr);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        p = p + n;
    }

    /* resources
    */
    cc = sscanf (p, "%u%n", &hPtr->nRes, &n);
    if (cc != 1) {
        // goto out
        FREEUP (hPtr->busyThreshold);
        FREEUP (hPtr);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    if (hPtr->nRes > 0) {
        hPtr->resList = calloc ( hPtr->nRes, sizeof (char *));
    }
    p = p + n;

    for( unsigned int i = 0; i < hPtr->nRes; i++ ) {
        cc = sscanf (p, "%s%n", name, &n);
        if (cc != 1) {
            // goto out
            FREEUP (hPtr->busyThreshold);
            FREEUP (hPtr);
            return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
        }
        hPtr->resList[i] = strdup (getstr_ (name));
        p = p + n;
    }

    cc = sscanf (p, "%d%s%n", &hPtr->rexPriority, name, &n);
    if (cc != 2) {
        // goto out
        FREEUP (hPtr->busyThreshold);
        FREEUP (hPtr);
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }
    
    p = p + n;

    /* windows are coded bit funny, addHost()
     * will handle the NULL pointer all right.
     */
    window = getstr_ (name);
    if (window[0] != 0) {
        hPtr->window = strdup (window);
    }
    else {
        hPtr->window = NULL;
    }

    /* at last my baby is comin' home
    */
    ev->record = hPtr;

    return 0;

// out:
//   FREEUP (hPtr->busyThreshold);
//   FREEUP (hPtr);

//   return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
}

/* readRmHost()
 */
int
readRmHost (char *buf, struct lsEventRec *ev)
{
    struct hostEntryLog *hPtr = NULL;
    static char name[MAX_LSF_NAME_LEN + 1];

    hPtr = calloc (1, sizeof (struct hostEntryLog));

    sscanf (buf, "%s", name);
    strcpy (hPtr->hostName, getstr_ (name));

    ev->record = hPtr;

    return 0;
}


/* getstr_()
 * Strip the quotes around the string
 */
char *
getstr_ (const char *s)
{
    char *p = NULL;
    static char buf[MAX_LSF_NAME_LEN + 1];
    const char *kot = "";
    char *buffer = malloc( strlen( kot ) );
    strcpy( buffer, kot );

    p = strdup( buf );
    if (s[0] == '"' && s[1] == '"') { // FIXME FIXME FIXME identify what is s[0] and s[1] and use labels instead of numbers
        return buffer;
    }

    ++s;
    while (*s != '"') {
        *p++ = *s++;
    }

    *p = 0;

    return buf;
}

/* freeHostEntryLog()
 */
int
freeHostEntryLog (struct hostEntryLog **hPtr)
{
    if (hPtr == NULL || *hPtr == NULL) {
        return -1; // FIXME FIXME FIXME FIXME replace with meaningful, *positive* return value
    }

    FREEUP ((*hPtr)->busyThreshold);
    for( size_t cc = 0; cc < (*hPtr)->nRes; cc++) {
        FREEUP ((*hPtr)->resList[cc]);
    }
    FREEUP ((*hPtr)->resList);
    FREEUP ((*hPtr)->window);
    FREEUP (*hPtr);

    return 0;
}
