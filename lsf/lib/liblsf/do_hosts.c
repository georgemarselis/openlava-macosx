// added by George Marselis <george@marsel.is> Tuesday March 26

#include <stdio.h>
#include <strings.h> // required for strcasecmp();
#include <stdbool.h> // required for bool type
#include <float.h>

#include "lsf.h"
#include "lib/confmisc.h"
#include "lib/getnextline.h"
#include "lib/syslog.h"
#include "lsb/lsbatch.h"
#include "lib/words.h"
#include "lib/misc.h"
#include "lib/host.h"
#include "libint/lsi18n.h"
#include "lib/lsb_params.h"

/*************************************************************************************************************************
 * FIXME FIXME FIXME FIXME FIXME explain the difference between the two
*/

/*************************************************************************************************************************
 * Function: do_Hosts_lsf (FILE * fp, const char *filename, size_t *lineNum, struct lsInfo *info)
 * 
 *  purpose: 
 *
 * Input:
 * Output: 
*/
char
do_Hosts_lsf (FILE *fp, const char *filename, size_t *lineNum, struct lsInfo *info)
{
    const char *sp        = NULL;
    char *word      = NULL;
    char **resList  = NULL;
    char *linep     = NULL;
    int ignoreR     = FALSE;
    unsigned int n  = 0 ; /* FIXME FIXME
                           * WARNING DANGER WARNING SHITTY CODE
                           * n is shared among many blocks of code.
                           * MUST MUST MUST disentagle
                           */

    unsigned int numAllocatedResources = 0;
    struct hostInfo host;

    enum {
        HOSTNAME, // = info->numIndx,
        MODEL,
        TYPE,
        ND,
        RESOURCES,
        RUNWINDOW,
        REXPRI0,
        SERVER0,
        R,
        S,
        NUM_ALLOCATED_RESOURCES = 64
    };

    const char *keylist[] = {
        "HOSTNAME",
        "MODEL",
        "TYPE",
        "ND",
        "RESOURCES",
        "RUNWINDOW",
        "REXPRI0",
        "SERVER0",
        "R",
        "S",
        "NUM_ALLOCATED_RESOURCES",
        NULL
    };

    struct keymap keyList[] = {
        { HOSTNAME,                keylist[HOSTNAME],                NULL },
        { MODEL,                   keylist[MODEL],                   NULL },
        { TYPE,                    keylist[TYPE],                    NULL },
        { ND,                      keylist[ND],                      NULL },
        { RESOURCES,               keylist[RESOURCES],               NULL },
        { RUNWINDOW,               keylist[RUNWINDOW],               NULL },
        { REXPRI0,                 keylist[REXPRI0],                 NULL },
        { SERVER0,                 keylist[SERVER0],                 NULL },
        { R,                       keylist[R],                       NULL },
        { S,                       keylist[S],                       NULL },
        { NUM_ALLOCATED_RESOURCES, keylist[NUM_ALLOCATED_RESOURCES], NULL },
        { 255,                     NULL,                             NULL }  // FIXME FIXME FIXME replace all similar 255 with label
    };

    const char hostString[ ]   = "host";
    const char mallocString[ ] = "malloc";

    initHostInfo (&host);

    linep = getNextLineC_ (fp, lineNum, TRUE);
    if (!linep) {
        ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, hostString); /*catgets33 */
        return FALSE;
    }

    if (isSectionEnd (linep, filename, lineNum, hostString)) {
        /* catgets 5135 */
        ls_syslog (LOG_ERR, "catgets 5135: %s: %s(%d): empty host section", __func__, filename, *lineNum);
        return FALSE;
    }

    if (strchr (linep, '=') == NULL) {

        if (!keyMatch (keyList, linep, FALSE)) {
            /* catgets 5136 */
            ls_syslog (LOG_ERR, "catgets 5136: %s: %s(%d): keyword line format error for section host, ignoring section", __func__, filename, *lineNum);
            doSkipSection (fp, lineNum, filename, hostString);
            return FALSE;
        }

        for( unsigned int i = 0; keyList[i].key != NULL; i++) {
            // if (keyList[i].position != -1) {
            //  continue;
            // }
            if (keyList[i].position == 255) { // FIXME FIXME FIXME FIXME FIXME what the fuck is -1 supposed to represent and why continue on it
                break;
            }

            if( ( strcasecmp ( "hostname", keyList[i].key ) == 0 ) || ( strcasecmp( "model", keyList[i].key) == 0 ) || ( strcasecmp ( "type", keyList[i].key ) == 0 ) || ( strcasecmp( "resources", keyList[i].key) == 0 ) ) {
                /* catgets 5137 */
                ls_syslog (LOG_ERR, "catgets 5137: %s: %s(%d): keyword line: key %s is missing in section host, ignoring section", __func__, filename, *lineNum, keyList[i].key);
                doSkipSection (fp, lineNum, filename, hostString);

                for( unsigned int j = 0; keyList[j].key != NULL; j++) {
                        // if (keyList[j].position != -1) {
                    if (keyList[j].position != 255 ) {
                        FREEUP (keyList[j].value);
                    }
                }

                return FALSE;
            }
        }
    }

    // if (keyList[R].position != -1 && keyList[SERVER0].position != -1) {
    if( keyList[R].position != 255 && keyList[SERVER0].position != 255 ) {
        /* catgets 5138 */
        ls_syslog (LOG_WARNING, "catgets 5138: %s: %s(%d): keyword line: conflicting keyword definition: you cannot define both 'R' and 'SERVER'. 'R' ignored", __func__, filename, *lineNum);
        ignoreR = TRUE;
    }

    while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)  {

        freekeyval (keyList);
        initHostInfo (&host);

        if (isSectionEnd (linep, filename, lineNum, hostString )) {
            return TRUE;
        }

        if (mapValues (keyList, linep) < 0)
        {
            /* catgets 5139  */
            ls_syslog (LOG_ERR, "catgets 5139: %s: %s(%d): values do not match keys for section host, ignoring line", __func__, filename, *lineNum);
            continue;
        }

        if (strlen (keyList[HOSTNAME].value) > MAXHOSTNAMELEN) {
            /* catgets 5140 */
            ls_syslog (LOG_ERR, "catgets 5140: %s: %s(%d): too long host name, ignored.", __func__, filename, *lineNum);
            continue;
        }

        if (Gethostbyname_ (keyList[HOSTNAME].value) == NULL) {
            /* catgets 5141 */
            ls_syslog (LOG_ERR, "catgets 5141: %s: %s(%d): Invalid hostname %s in section host. Ignoring host", __func__, filename, *lineNum, keyList[HOSTNAME].value);
            continue;
        }

        strcpy (host.hostName, keyList[HOSTNAME].value);

        if ((host.hostModel = putstr_ (keyList[MODEL].value)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
            lserrno = LSE_MALLOC;
            freeHostInfo (&host);
            freekeyval (keyList);
            doSkipSection (fp, lineNum, filename, hostString);
            return FALSE;
        }

        if ((host.hostType = putstr_ (keyList[TYPE].value)) == NULL) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, mallocString);
            lserrno = LSE_MALLOC;
            freeHostInfo (&host);
            freekeyval (keyList);
            doSkipSection (fp, lineNum, filename, hostString);
            return FALSE;
        }

        // if (keyList[ND].position != -1) {
        if (keyList[ND].position != 255) {
            const unsigned short BASEZERO = 0;
            errno = 0;
            assert( strtoul( keyList[ND].value, NULL , BASEZERO ) );
            if( !errno ) {
                    host.nDisks = (unsigned int) strtoul( keyList[ND].value, NULL , BASEZERO ); // FIXME FIXME FIXME i'm sure nobody will get a couple of billion of disks in a single system any time soon, but please take care of this and change nDisks to size_t or devise a plan to have a proper conversion to int.
            }
            else {
                fprintf( stdout, "%s: errno was: %d; the value of keyList[ND].value was %s; too big for strtoul(); ignoring, no disks set\n", __func__, errno, keyList[ND].value );
                ls_syslog( LOG_ERR, "%s: errno was: %d; the value of keyList[ND].value was %s; too big for strtoul(); ignoring, no disks set", __func__, errno, keyList[ND].value );
                errno = 0;
                continue;
            }

        }
        else {
            host.nDisks = UINT_MAX;
        }

        host.busyThreshold = malloc( info->numIndx * sizeof ( host.busyThreshold ) );
        if ( NULL == host.busyThreshold && ENOMEM == errno ) {
            /* FIXME
             this lserrno and logging to syslog about memory allocation must either
             go or get a lot more serious somehow
             */
            assert( info->numIndx );
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, mallocString );
            lserrno = LSE_MALLOC;
            freeHostInfo (&host);
            freekeyval (keyList);
            doSkipSection (fp, lineNum, filename, hostString );

            return FALSE;
        }

        liblsf_putThreshold( R15S, &host, keyList[R15S].position, keyList[R15S].value, FLT_MAX );
        liblsf_putThreshold( R1M,  &host, keyList[R1M].position,  keyList[R1M].value,  FLT_MAX );
        liblsf_putThreshold( R15M, &host, keyList[R15M].position, keyList[R15M].value, FLT_MAX );
        liblsf_putThreshold( UT,   &host, keyList[UT].position,   keyList[UT].value,   FLT_MAX );

        if (host.busyThreshold[UT] > 1.0l  && host.busyThreshold[UT] < DBL_MAX ) {
            ls_syslog (LOG_INFO, "catgets 5145: %s: %s(%d): value for threshold ut <%2.2e> is greater than 1, assumming <%5.1e%%>", __func__, filename, *lineNum, (double) host.busyThreshold[UT], (double) host.busyThreshold[UT]); // FIXME FIXME the (double) cast s probably correct
            host.busyThreshold[UT] /= 100;
        }
        liblsf_putThreshold( PG,   &host, keyList[PG].position,  keyList[PG].value,  FLT_MAX );
        liblsf_putThreshold( IO,   &host, keyList[IO].position,  keyList[IO].value,  FLT_MAX );
        liblsf_putThreshold( LS,   &host, keyList[LS].position,  keyList[LS].value,  FLT_MAX );
        liblsf_putThreshold( IT,   &host, keyList[IT].position,  keyList[IT].value,  FLT_MAX );
        // liblsf_putThreshold( TMP,  &host, keyList[TMP].position, keyList[TMP].value, -INFINIT_LOAD );
        liblsf_putThreshold( TMP,  &host, keyList[TMP].position, keyList[TMP].value, FLT_MAX );
        // liblsf_putThreshold( SWP,  &host, keyList[SWP].position, keyList[SWP].value, -INFINIT_LOAD );
        liblsf_putThreshold( SWP,  &host, keyList[SWP].position, keyList[SWP].value, FLT_MAX );
        // liblsf_putThreshold( MEM,  &host, keyList[MEM].position, keyList[MEM].value, -INFINIT_LOAD );
        liblsf_putThreshold( MEM,  &host, keyList[MEM].position, keyList[MEM].value, FLT_MAX );

        for (unsigned int i = NBUILTINDEX; i < NBUILTINDEX + info->numUsrIndx; i++) {
            if (info->resTable[i]->orderType == INCR) {
                assert( i <= INT_MAX );
                liblsf_putThreshold ( i, &host, keyList[i].position, keyList[i].value, FLT_MAX);
            }
            else {
                assert( i <= INT_MAX );
                liblsf_putThreshold ( i, &host, keyList[i].position, keyList[i].value, FLT_MAX);
            }
        }

        for (unsigned int i = NBUILTINDEX + info->numUsrIndx; i < info->numIndx; i++) {

            host.busyThreshold[i] = FLT_MAX;
            host.numIndx = info->numIndx;
            numAllocatedResources = NUM_ALLOCATED_RESOURCES;
            resList = calloc (numAllocatedResources, sizeof( *resList ) );

            if (resList == NULL) {
                const char calloc[] = "calloc";
                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, calloc );;
            }

            n = 0;
            sp = keyList[RESOURCES].value;
            while ((word = getNextWord_ (&sp)) != NULL) {

                // FIXME FIXME FIXME
                // WARNING DANGER WARNING DANGER
                // (WAS A) SHITTY CODE ASSIGNS TO LOOP VARIABLE
                // FIXME FIXME FIXME
                // MUST INVESTIGATE WHY
                for ( unsigned int j = 0; j < n; j++) {
                    if (!strcmp (word, resList[j])) {
                        break;
                    }
                }

                // casts of n to int must be thrown out, for all instanses in this function
                if (i < n) {
                    /* catgets 5146 */
                    ls_syslog (LOG_ERR, "catgets 5146: %s: %s(%d): Resource <%s> multiply specified for host %s in section host. Ignored.", __func__, filename, *lineNum, word, host.hostName);
                    continue;
                }
                else {

                    /* FIXME FIXME WTF IS THIS SHIT? why isn't this an else if() condition?!? */
                    if (n >= numAllocatedResources) {

                        numAllocatedResources *= 2;
                        resList = realloc (resList, numAllocatedResources * (sizeof (char *)));

                        if (resList == NULL) {
                            const char calloc[] = "calloc";
                            lserrno = LSE_MALLOC;
                            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, calloc);
                            freeHostInfo (&host);
                            freekeyval (keyList);
                            doSkipSection (fp, lineNum, filename, hostString);
                            return FALSE;
                        }
                    }

                    if ((resList[n] = putstr_ (word)) == NULL) {
                        ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
                        lserrno = LSE_MALLOC;

                        for ( unsigned int j = 0; j < n; j++) {
                            FREEUP (resList[j]);
                        }
                        FREEUP (resList);
                        freeHostInfo (&host);
                        freekeyval (keyList);
                        doSkipSection (fp, lineNum, filename, hostString);
                        return FALSE;
                    }

                    n++; /* FIXME FIXME ... the fuck is this increment doing down here? why isn't it
                          * in the increment section of the for loop?
                          */

                } // end f (i < n)/else

            } // end while ((word = getNextWord_ (&sp)) != NULL)

        } // end for (int i = NBUILTINDEX + info->numUsrIndx; i < info->numIndx; i++)

        resList[n] = NULL;

        /* FIXME
         * the hostInfo struct needs a bit of looking over. The following code example
         *      host.nRest = n;
         * n lives at the top of this function (ffs). It used to be int, i changed it
         * to unsigned int and kept any assert( n >= 0 ) laying around; redundant, yes.
         *
         * hostInfo.nRest is of type int. n is of type unsigned int.
         *
         * the casted assignment below is legal, and will not create any issues, but
         * this looks like a bigger problem with the code base as there are a number
         * of assignments here needing a cast for no good reason.
         *
         * when coming back to this piece of code:
         *      take a look at the following clues
         *
         * 1. what are the possible value ranges for the hostInfo fields?
         *      if >= 0, then please redeclare them all as unsigned int.
         *          insert assert()s for upper limits
         *
         * 2. why is n shared among multiple code blocks? can this code be refactored
         *      withtout breaking it.
         *
         *  In order to proceed with the above, function tests must be implemented.
         *
         *  which means, we just bumped the tests up in priority before coding
         *  anything more. the editing in this current run must be kept to a cool minimum
         *      - gmarselis
         *
         */
        host.nRes = n;

        host.resources = malloc( n * sizeof( host.resources ) + 1 );
        if ( ( NULL == host.resources) && ENOMEM == errno ) {

            assert( n );
            ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString );
            lserrno = LSE_MALLOC;

            for ( unsigned int j = 0; j < n; j++) {
                FREEUP (resList[j]);
            }

            FREEUP (resList);
            freeHostInfo (&host);
            freekeyval (keyList);
            doSkipSection (fp, lineNum, filename, hostString);
            return FALSE;
        }

        for( unsigned int i = 0; i < n; i++) {
            if ((host.resources[i] = putstr_ (resList[i])) == NULL) {

                ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString );
                lserrno = LSE_MALLOC;

                for ( unsigned int j = 0; j < n; j++) {
                    FREEUP (resList[j]);
                }

                FREEUP (resList);
                freeHostInfo (&host);
                freekeyval (keyList);
                doSkipSection (fp, lineNum, filename, hostString);
                return FALSE;
            }
        }

        for ( unsigned int j = 0; j < n; j++) {
            FREEUP (resList[j]);
        }
        FREEUP (resList);

        host.rexPriority = (int) DEF_REXPRIORITY; // Cast is fine here: we want DEF_REXPRIORITY to be a constand and rexPriority to be renice-abled
        if ( keyList[REXPRI0].position != 255 ) { // FIXME FIXME FIXME FIXME 255 must be set to a label and set global
            host.rexPriority = atoi( keyList[REXPRI0].value );
        }

        host.isServer = 1;
        if ( keyList[R].position != 255 ) { // FIXME FIXME FIXME FIXME 255 must be set to a label and set global
            if (!ignoreR) {     // FIXME shitty code. isServer and value should be the same type
                // host.isServer = (char) atoi (keyList[R].value);
                host.isServer = *keyList[R].value;
            }
        }

        if ( keyList[SERVER0].position != 255 ) { // FIXME FIXME FIXME FIXME 255 must be set to a label and set global
            // FIXME shitty code. isServer and value should be the same type
            // host.isServer = (char) atoi (keyList[SERVER0].value); // FIXME FIXME is cast here justified?
            host.isServer = *keyList[SERVER0].value; // FIXME FIXME is cast here justified?
        }

        host.windows = NULL;
        if (keyList[RUNWINDOW].position != 255) { // FIXME FIXME FIXME FIXME 255 must be set to a label and set global

            if (strcmp (keyList[RUNWINDOW].value, "") == 0) {
                host.windows = NULL;
            }
            else {
                host.windows = parsewindow (keyList[RUNWINDOW].value, filename, lineNum, hostString); // FIXME FIXME FIXME FIXME make conf case-insensitive

                if (host.windows == NULL) {

                    ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, mallocString);
                    lserrno = LSE_MALLOC;
                    freeHostInfo (&host);
                    freekeyval (keyList);
                    doSkipSection (fp, lineNum, filename, hostString);
                    return FALSE;
                }
            }
        }

        addHost (&host, filename, lineNum);
    } // end while ((linep = getNextLineC_ (fp, lineNum, TRUE)) != NULL)

    // FIXME FIXME FIXME
    //      this chunk came out of somewhere. after manual indentation and adding of brackets,
    //      it could not sick anywhere particular. be causious with it, find out where this is supposed to go.
    /*    else
     {
     ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, "host");
     doSkipSection (fp, lineNum, __func__, "host");
     return FALSE;
     }*/

    ls_syslog (LOG_ERR, "catgets 33: %s: %s(%d): Premature EOF in section %s", __func__, filename, *lineNum, hostString); /*catgets33 */
    return TRUE;
}


bool
do_Hosts_lsb (struct lsConf *conf, const char *filename, size_t *lineNum, struct lsInfo *info, int options)
{

// #define HKEY_HNAME info->numIndx
// #define HKEY_MXJ info->numIndx+1
// #define HKEY_RUN_WINDOW info->numIndx+2
// #define HKEY_MIG info->numIndx+3
// #define HKEY_UJOB_LIMIT info->numIndx+4
// #define HKEY_DISPATCH_WINDOW info->numIndx+5
//   
    int num                 = 0;
    int new                 = 0;
    int numSelectedHosts    = 0;
    bool isTypeOrModel      = false;
    bool returnCode         = false;
    bool copyCPUFactor      = false;
    size_t *override        = 0;
    char   *linep           = NULL;
    char hostname[MAXHOSTNAMELEN];
    struct hostInfoEnt host;
    struct hostent  *hp                  = NULL;
    struct hTab     *tmpHosts            = NULL;
    struct hostInfo *hostInfo            = NULL;
    struct hostInfo *hostList            = NULL;
    struct hTab     *nonOverridableHosts = NULL;

    enum state { 
        HKEY_HNAME, 
        HKEY_MXJ, 
        HKEY_RUN_WINDOW, 
        HKEY_MIG, 
        HKEY_UJOB_LIMIT,
        HKEY_DISPATCH_WINDOW
    };

    const char *keylist[ ] = {
        "HKEY_HNAME", 
        "HKEY_MXJ", 
        "HKEY_RUN_WINDOW", 
        "HKEY_MIG", 
        "HKEY_UJOB_LIMIT",
        "HKEY_DISPATCH_WINDOW",
        NULL
    };

    struct keymap keyList[] = {
        { HKEY_HNAME,           keylist[ HKEY_HNAME ],           NULL },
        { HKEY_MXJ,             keylist[ HKEY_MXJ ],             NULL },
        { HKEY_RUN_WINDOW,      keylist[ HKEY_RUN_WINDOW ],      NULL },
        { HKEY_MIG,             keylist[ HKEY_MIG ],             NULL },
        { HKEY_UJOB_LIMIT,      keylist[ HKEY_UJOB_LIMIT ],      NULL },
        { HKEY_DISPATCH_WINDOW, keylist[ HKEY_DISPATCH_WINDOW ], NULL },
        { UINT_MAX,             NULL, NULL }
    };

    const char host_string[] = "host";
    const char default_[] = "default"; // FIXME FIXME FIXME default could actually be defaultLabel or something else, look it up 

    memset( hostname,'\0', strlen( hostname ) );

    if (conf == NULL) {
        return false;
    }

    // FREEUP (keyList);
    // keyList = malloc ((unsigned long)(HKEY_DISPATCH_WINDOW + 2) * sizeof (struct keymap));
    // if ( NULL == keyList && ENOMEM == errno ){
    //  ls_syslog (LOG_ERR, I18N_FUNC_FAIL_M, __func__, "malloc");
    //  return FALSE;
    // }

    // assert( HKEY_HNAME + 1 <= INT_MAX );
    // assert( HKEY_DISPATCH_WINDOW + 2 <= INT_MAX );
    // initkeyList (keyList, (int)HKEY_HNAME + 1, (int)(HKEY_DISPATCH_WINDOW + 2), info);
    // keyList[HKEY_HNAME].key               = "HOST_NAME";
    // keyList[HKEY_MXJ].key                 = "MXJ";
    // keyList[HKEY_RUN_WINDOW].key          = "RUN_WINDOW";
    // keyList[HKEY_MIG].key                 = "MIG";
    // keyList[HKEY_UJOB_LIMIT].key          = "JL/U";
    // keyList[HKEY_DISPATCH_WINDOW].key     = "DISPATCH_WINDOW";
    // keyList[HKEY_DISPATCH_WINDOW + 1].key = NULL;

    // initHostInfoEnt ((struct hostInfoEnt *)&host);
    initHostInfoEnt ( &host);
    linep = getNextLineC_conf (conf, lineNum, true);
    if (!linep) {
        // ls_syslog (LOG_ERR, I18N_FILE_PREMATURE, __func__, filename, *lineNum);
        ls_syslog (LOG_ERR, LSB_FILE_PREMATUR_STR, __func__, filename, *lineNum); /*catgets 5051 */
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (isSectionEnd (linep, filename, lineNum, host_string)) {
        // ls_syslog (LOG_WARNING, I18N_EMPTY_SECTION, __func__, filename, *lineNum, host_string);
        ls_syslog (LOG_WARNING, LSB_EMPTY_SECTION_STR, __func__, filename, *lineNum, host_string); /*catgets 5052 */
        lsberrno = LSBE_CONF_WARNING;
        return false;
    }

    if (strchr (linep, '=') != NULL) {
        ls_syslog (LOG_ERR, I18N_HORI_NOT_IMPLE, __func__, filename, *lineNum, host_string);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, host_string);
        return false;
    }

    if (!keyMatch (keyList, linep, false ) ) {
        /* catgets 5174 */
        ls_syslog (LOG_ERR, "catgets 5174: %s: File %s at line %d: Keyword line format error for Host section; ignoring section", __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, host_string);
        return false;
    }

    if ( UINT_MAX == keyList[HKEY_HNAME].position ) {
        /* catgets 5175 */
        ls_syslog (LOG_ERR, "catgets 5175: %s: File %s at line %d: Hostname required for Host section; ignoring section", __func__, filename, *lineNum);
        lsberrno = LSBE_CONF_WARNING;
        doSkipSection_conf (conf, lineNum, filename, host_string);
        return false;
    }

    nonOverridableHosts = malloc( sizeof( struct hTab ) );
    tmpHosts            = malloc( sizeof( struct hTab ) );
    hostList            = malloc( cConf->numHosts *  sizeof( struct hostInfo ) );
    if ( ( NULL == nonOverridableHosts && ENOMEM == errno ) || 
         ( NULL == tmpHosts            && ENOMEM == errno ) ||
         ( NULL == hostList            && ENOMEM == errno )
        )
    {

        ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc",  cConf->numHosts * sizeof (struct hostInfo));
        lsberrno = LSBE_NO_MEM;
        FREEUP (nonOverridableHosts);
        FREEUP (tmpHosts);
        FREEUP (hostList);
        return false;
    }
    h_initTab_ (tmpHosts, 32);                      // FIXME FIXME 32 seems awfully specific
    // h_initTab_ ((hTab *)nonOverridableHosts, 32);   // FIXME FIXME 32 seems awfully specific
    h_initTab_ ( nonOverridableHosts, 32);   // FIXME FIXME 32 seems awfully specific
    while ((linep = getNextLineC_conf (conf, lineNum, TRUE)) != NULL) {

        freekeyval (keyList);
        initHostInfoEnt( host );
        isTypeOrModel    = false;
        numSelectedHosts = 0;

        if (isSectionEnd (linep, filename, lineNum, host_string)) {
            FREEUP (hostList);
            h_delTab_ (( struct hTab *)nonOverridableHosts);
            FREEUP (nonOverridableHosts);
            h_delTab_ (tmpHosts);
            FREEUP (tmpHosts);
            // freeHostInfoEnt ((struct hostInfoEnt *)&host);
            freeHostInfoEnt ( &host );
            // assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
            return (char)returnCode;
        }

        if (mapValues (keyList, linep) < 0) {
            /* catgets 5177 */
            ls_syslog (LOG_ERR, "catgets 5177: %s: File %s at line %d: Values do not match keys in Host section; ignoring line", __func__, filename, *lineNum);
            lsberrno = LSBE_CONF_WARNING;
            continue;
        }

        if (strcmp (keyList[HKEY_HNAME].value, default_ ) != 0) {

            hp = Gethostbyname_ (keyList[HKEY_HNAME].value);
            if (!hp && options != CONF_NO_CHECK) {

                unsigned int total1 = 0;
                for ( unsigned int i = 0; i < info->nModels; i++) {
                    total1 = i;
                    if (strcmp (keyList[HKEY_HNAME].value, info->hostModels[i]) == 0) {
                        break;
                    }
                }

                if ( total1 == info->nModels) {

                    unsigned int total2 = 0;
                    for ( unsigned int i = 0; i < info->nTypes; i++) {

                        total2 = i;
                        if( strcmp( keyList[HKEY_HNAME].value, info->hostTypes[i]) == 0 ) {
                            break;
                        }
                    }

                    if( total2 == (unsigned int) info->nTypes ) {
                        /* catgets 5178 */
                        ls_syslog (LOG_ERR, "catgets 5178: %s: File %s at line %d: Invalid host/type/model name <%s>; ignoring line", __func__, filename, *lineNum, keyList[HKEY_HNAME].value);
                        lsberrno = LSBE_CONF_WARNING;
                        continue;
                    }
                }

                numSelectedHosts = 0;
                for ( unsigned int i = 0; i < cConf->numHosts; i++) {
                    hostInfo = &(cConf->hosts[i]);
                    if( ( strcmp (hostInfo->hostType,  keyList[HKEY_HNAME].value) == 0 || 
                          strcmp (hostInfo->hostModel, keyList[HKEY_HNAME].value) == 0
                        ) && hostInfo->isServer) 
                    {
                        hostList[numSelectedHosts] = cConf->hosts[i];
                        numSelectedHosts++;
                    }
                }

                if (!numSelectedHosts) {
                    /* catgets 5179 */
                    ls_syslog (LOG_ERR, "catgets 5179: %s: File %s at line %d: no server hosts of type/model <%s> are known by LSF; ignoring line", __func__, filename, *lineNum, keyList[HKEY_HNAME].value);
                    lsberrno = LSBE_CONF_WARNING;
                    continue;
                }
                strcpy (hostname, keyList[HKEY_HNAME].value);
                isTypeOrModel = true;
            }
            else {
                if (hp) {
                    strcpy (hostname, hp->h_name);
                }
                else if (options == CONF_NO_CHECK) {
                    strcpy (hostname, keyList[HKEY_HNAME].value);
                }
            }
        }
        else {
            strcpy (hostname, default_ );
        }
        h_addEnt_ (tmpHosts, hostname, &new);
        if (!new) {
            /* catgets 5180 */
            ls_syslog (LOG_ERR, "catgets 5180: %s: File %s at line %d: Host name <%s> multiply specified; ignoring line", __func__, filename, *lineNum, keyList[HKEY_HNAME].value );
            lsberrno = LSBE_CONF_WARNING;
            continue;
        }

        if (keyList[HKEY_MXJ].value != NULL && strcmp (keyList[HKEY_MXJ].value, "!") == 0) {
            host.maxJobs = 0;
        }
        // else if( keyList[HKEY_MXJ].position >= 0 && 
        else if( keyList[HKEY_MXJ].value != NULL   && 
                 strcmp (keyList[HKEY_MXJ].value, "") ) 
        {
            // assert( my_atoi (keyList[HKEY_MXJ].value, INFINIT_INT, -1) >= 0);
            host.maxJobs = my_atoi (keyList[HKEY_MXJ].value, INT_MAX, -1);
            // if ( fabs( INFINIT_INT - host.maxJobs) < 0.00001 ) {
            if ( ( UINT_MAX - host.maxJobs) < 0.00001F ) {
                /* catgets 5183 */
                ls_syslog (LOG_ERR, "catgets 5183: %s: File %s at line %d: Invalid value <%s> for key <%s>; %d is assumed", __func__, filename, *lineNum, keyList[HKEY_MXJ].value, keyList[HKEY_MXJ].key, UINT_MAX);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        // if( keyList[HKEY_UJOB_LIMIT].position >= 0 && keyList[HKEY_UJOB_LIMIT].value != NULL && strcmp (keyList[HKEY_UJOB_LIMIT].value, "")) {
        if( keyList[HKEY_UJOB_LIMIT].value != NULL && strcmp (keyList[HKEY_UJOB_LIMIT].value, "" ) ) {

            // assert( my_atoi( keyList[HKEY_UJOB_LIMIT].value, INFINIT_INT, -1 ) >=0 );
            host.userJobLimit = my_atoi( keyList[HKEY_UJOB_LIMIT].value, INT_MAX, -1 );
            // if ( fabs( INFINIT_INT - host.userJobLimit ) < 0.00001 ) {
            if ( ( UINT_MAX - host.userJobLimit ) < 0.00001F ) {
                /* catgets 5183 */
                ls_syslog (LOG_ERR, "catgets 5183: %s: File %s at line %d: Invalid value <%s> for key <%s>; %d is assumed", __func__, filename, *lineNum, keyList[HKEY_UJOB_LIMIT].value, keyList[HKEY_UJOB_LIMIT].key, UINT_MAX);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        // if (keyList[HKEY_RUN_WINDOW].position >= 0) {

        if (strcmp (keyList[HKEY_RUN_WINDOW].value, "")) {
            host.windows = parsewindow (keyList[HKEY_RUN_WINDOW].value, filename, lineNum, "Host");

            if(  lserrno == LSE_CONF_SYNTAX) {
                 lserrno  = LSE_NO_ERR;
                 lsberrno = LSBE_CONF_WARNING;
            }
        }
        // }

        // if (keyList[HKEY_DISPATCH_WINDOW].position >= 0) {

        if (strcmp (keyList[HKEY_DISPATCH_WINDOW].value, "")) {
            FREEUP (host.windows);
            host.windows = parsewindow (keyList[HKEY_DISPATCH_WINDOW].value, filename, lineNum, "Host");

            if (lserrno == LSE_CONF_SYNTAX) {
                lserrno = LSE_NO_ERR;
                lsberrno = LSBE_CONF_WARNING;
            }
        }
        // }

        // if (keyList[HKEY_MIG].position >= 0 keyList[HKEY_MIG].value != NULL && strcmp (keyList[HKEY_MIG].value, "")) {
        if( keyList[HKEY_MIG].value != NULL && strcmp (keyList[HKEY_MIG].value, "" ) ) {

            host.mig = strtoul( keyList[HKEY_MIG].value, NULL, 10 ); // FIXME FIXME FIXME check the strtoul() call was successful
            // if ( fabs( INFINIT_INT - host.mig ) < 0.00001 ) {
            if ( ( UINT_MAX - host.mig ) < 0.00001F ) {
                /* catgets 5186 */
                ls_syslog (LOG_ERR, "catgets 5186: %s: File %s at line %d: Invalid value <%s> for key <%s>; no MIG threshold is assumed", __func__, filename, *lineNum, keyList[HKEY_MIG].value, keyList[HKEY_MIG].key);
                lsberrno = LSBE_CONF_WARNING;
            }
        }

        host.loadSched = malloc( info->numIndx * sizeof ( host.loadSched ));
        if (info->numIndx && ( NULL == host.loadSched && ENOMEM == errno ) ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", info->numIndx * sizeof (float *));
            lsberrno = LSBE_NO_MEM;
            freekeyval (keyList);
            FREEUP (hostList);
            h_delTab_ ( ( struct hTab *)nonOverridableHosts);
            FREEUP (nonOverridableHosts);
            h_delTab_ (tmpHosts);
            FREEUP (tmpHosts);
            FREEUP (host.windows);
            FREEUP (host.loadSched);
            FREEUP (host.loadStop);
            assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
            return returnCode;
        }

        host.loadStop = malloc( info->numIndx * sizeof ( host.loadStop ));
        if (info->numIndx && ( NULL == host.loadStop && ENOMEM == errno ) ) {
            ls_syslog (LOG_ERR, I18N_FUNC_D_FAIL_M, __func__, "malloc", info->numIndx * sizeof (float *));
            lsberrno = LSBE_NO_MEM;
            freekeyval (keyList);
            FREEUP (hostList);
            h_delTab_ ( ( struct hTab *)nonOverridableHosts);
            FREEUP (nonOverridableHosts);
            h_delTab_ (tmpHosts);
            FREEUP (tmpHosts);
            FREEUP (host.windows);
            FREEUP (host.loadSched);
            FREEUP (host.loadStop);
            assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
            return returnCode;
        }

        getThresh (info, keyList, host.loadSched, host.loadStop, filename, lineNum, " in section Host ending");
        host.nIdx = info->numIndx;
        if (options == CONF_NO_CHECK) {
            host.host = hostname;
            num = 1;
            *override = true;
            copyCPUFactor = false;
            h_addEnt_( ( struct hTab * )nonOverridableHosts, hostname, &new );
        }
        else if (strcmp (hostname, default_ ) == 0) {
            num = 0;
            for ( unsigned int i = 0; i < cConf->numHosts; i++) {
                if (cConf->hosts[i].isServer != true) {
                    continue;
                }
                hostList[num++] = cConf->hosts[i];
            }

            override = false;
            copyCPUFactor = true;
        }
        else if (isTypeOrModel) {
            num           = numSelectedHosts;
            *override     = true;  // FIXME cast here is probably correct
            copyCPUFactor = true;
        }
        else {
            unsigned int total = 0;
            for ( unsigned int i = 0; i < cConf->numHosts; i++) {
                if (equalHost_ (hostname, cConf->hosts[i].hostName)) {
                    hostList[0] = cConf->hosts[i]; // FIXME FIXME FIXME label [0]
                    break;
                }
                total = i;
            }
            if (total == cConf->numHosts) {
                num = 0;
                /* catgets 5189 */
                ls_syslog (LOG_ERR, "catgets 5189: %s: File %s at line %d: Can't find the information of host <%s>", __func__, filename, *lineNum, hostname);
                lsberrno = LSBE_CONF_WARNING;
                freeHostInfoEnt (&host);
            }
            else if (cConf->hosts[total].isServer != true ) {
                num = 0;
                /* catgets 5190 */
                ls_syslog (LOG_ERR, "catgets 5190: %s: File %s at line %d: Host <%s> is not a server;ignoring", __func__, filename, *lineNum, hostname);
                lsberrno = LSBE_CONF_WARNING;
                freeHostInfoEnt ( &host);
            }
            else {
                num = 1;
                *override = true ;
                // h_addEnt_( (hTab *)nonOverridableHosts, hostList[0].hostName, &new);
                h_addEnt_( nonOverridableHosts, hostList[0].hostName, &new);
                copyCPUFactor = true;
            }
        }
        for( int i = 0; i < num; i++) {

            if (isTypeOrModel && h_getEnt_ (( struct hTab *)nonOverridableHosts, hostList[i].hostName)) {
                continue;
            }

            if (copyCPUFactor == true) {
                host.host = hostList[i].hostName;

                for( unsigned int j = 0; j < info->nModels; j++) {
                    if (strcmp (hostList[i].hostModel, info->hostModels[j]) == 0) {
                        host.cpuFactor = info->cpuFactor[j];
                        break;
                    }
                }
            }

            if (addHostEnt (&host, &hostList[i], override) == false) {
                    freekeyval (keyList);
                    FREEUP (hostList);
                    // h_delTab_ ( (struct hTab *) nonOverridableHosts );
                    h_delTab_ ( nonOverridableHosts );
                    FREEUP (nonOverridableHosts);
                    h_delTab_ (tmpHosts);
                    FREEUP (tmpHosts);
                    FREEUP (host.windows);
                    FREEUP (host.loadSched);
                    FREEUP (host.loadStop);
                    assert( returnCode <= CHAR_MAX && returnCode >= CHAR_MIN );
                    return returnCode;

            }
        }
        FREEUP (host.windows);
        FREEUP (host.loadSched);
        FREEUP (host.loadStop);

    }

    ls_syslog (LOG_ERR, LSB_FILE_PREMATUR_STR, __func__, filename, *lineNum);
    lsberrno = LSBE_CONF_WARNING;
    returnCode = true;

    return returnCode; // FIXME FIXME FIXME why a cast here?
}
