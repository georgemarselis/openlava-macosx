/*
 * Copuright (C) 2011 David Bigagli
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

#include <stdlib.h>
#include <unistd.h>

#include "lib/lib.h"
// #include "lib/lproto.h"
#include "lib/osal.h"
#include "lib/initenv.h"
#include "lib/misc.h"
#include "lib/structs/genParams.h"
#include "lib/getnextline.h"
#include "lib/words.h"

int
doEnvParams_ (struct config_param *plp)
{
    char *sp = NULL;
    char *spp = NULL;

    if (!plp) {
        return 0;
    }

    for (; plp->paramName != NULL; plp++) // FIXME FIXME fix initial condition
    {
        if ((sp = getenv (plp->paramName)) != NULL)
        {
            if (NULL == (spp = putstr_ (sp)))
            {
                lserrno = LSE_MALLOC;
                return -1;
            }
            FREEUP (plp->paramValue);
            plp->paramValue = spp;
        }
    }
    return 0;
}

char *
getTempDir_ (void)
{
    char *sp = NULL;
    char *tmpSp = NULL;
    struct stat stb;

    // if (sp) {
    //     return sp;
    // }

    tmpSp = genParams_[LSF_TMPDIR].paramValue;
    if ((tmpSp != NULL) && (stat (tmpSp, &stb) == 0) && (S_ISDIR (stb.st_mode))) {
        sp = tmpSp;
    }
    else {
        tmpSp = getenv ("TMPDIR"); // FIXME FIXME FIXME FIXME move "TMPDIR" to configure.ac
        if ((tmpSp != NULL) && (stat (tmpSp, &stb) == 0) && (S_ISDIR (stb.st_mode))) {
            sp = putstr_ (tmpSp);
        }

    }

    if( NULL == sp ) {
        char tmpdir[ ] = "/tmp"; // FIXME FIXME FIXME FIXME move "TMPDIR" to configure.ac
        sp = malloc( sizeof(char) * strlen( tmpdir ) + 1 );
        strcpy( sp, tmpdir ); // FIXME FIXME FIXME FIXME memleak.
    }

    return sp;
}

/* initenv_()
 * Read and initialiaze the openlava environment.
 */
int
initenv_ (struct config_param *userEnv, const char *pathname)
{
    int Error = 0;
    char *envdir = NULL;
    static int lsfenvset = FALSE;

    if (osInit_ () < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace return value by appropriate *positive* return value
    }

    if ((envdir = getenv ("LSF_ENVDIR")) != NULL) {  // FIXME FIXME FIXME FIXME move "TMPDIR" to configure.ac
        pathname = envdir;
    }
    else if (pathname == NULL) {
        pathname = LSETCDIR;
    }

    if (lsfenvset) {
        if (userEnv == NULL) {
            return 0;
        }
        if (readconfenv_ (NULL, userEnv, pathname) < 0) {
            return -1; // FIXME FIXME FIXME FIXME replace return value by appropriate *positive* return value
        }
        else if (doEnvParams_ (userEnv) < 0) {
            return -1; // FIXME FIXME FIXME FIXME replace return value by appropriate *positive* return value
        }
        return 0;
    }

    if (readconfenv_ (genParams_, userEnv, pathname) < 0) {
        return -1; // FIXME FIXME FIXME FIXME replace return value by appropriate *positive* return value
    }
    else {
        if (doEnvParams_ (genParams_) < 0) {
            return -1; // FIXME FIXME FIXME FIXME replace return value by appropriate *positive* return value
        }
        lsfenvset = TRUE;
        if (doEnvParams_ (userEnv) < 0) {
            Error = 1;
        }
    }

    if (!genParams_[LSF_CONFDIR].paramValue || !genParams_[LSF_SERVERDIR].paramValue) {
        lserrno = LSE_BAD_ENV;
        return -1;
    }

    if (genParams_[LSF_SERVER_HOSTS].paramValue != NULL) {
        char *sp = NULL;
        for (sp = genParams_[LSF_SERVER_HOSTS].paramValue; *sp != '\0'; sp++) {
            if (*sp == '\"') {
                *sp = ' ';
            }
        }
    }

    LSTMPDIR = getTempDir_ ();

    if (Error) {
        return -1;
    }

    return 0;
}

// int
// ls_readconfenv (struct config_param *paramList, const char *confPath)
// {
//     return readconfenv_ (NULL, paramList, confPath);
// }

int
readconfenv_ (struct config_param *pList1, struct config_param *pList2, const char *confPath)
{
    char *key       = NULL;
    char *value     = NULL;
    char *line      = NULL;
    FILE *fp        = NULL;
    size_t lineNum  = 0;
    size_t errLineNum_ = 0;
    unsigned int saveErrNo = 0;
    char filename[MAX_FILENAME_LEN];
    struct config_param *plp = NULL;

    memset( filename, '\0', strlen( filename ) );

    if (pList1) {
        for (plp = pList1; plp->paramName != NULL; plp++) {
            if (plp->paramValue != NULL) {
                lserrno = LSE_BAD_ARGS;
                return -1; // FIXME FIXME FIXME FIXME replace return value by appropriate *positive* return value
            }
        }
    }

    if (pList2) {
        for (plp = pList2; plp->paramName != NULL; plp++) {
            if (plp->paramValue != NULL) {
                lserrno = LSE_BAD_ARGS;
                return -1; // FIXME FIXME FIXME FIXME replace return value by appropriate *positive* return value
            }
        }
    }
    if (confPath) {
        memset (filename, 0, sizeof (filename));
        ls_strcat (filename, sizeof (filename), confPath);
        ls_strcat (filename, sizeof (filename), "/lsf.conf");  // FIXME FIXME FIXME FIXME put in configure.ac
        fp = fopen (filename, "r");
    }
    else {
        char *ep = getenv ("LSF_ENVDIR"); // FIXME FIXME FIXME FIXME put in configure.ac
        char buf[MAX_FILENAME_LEN];

        if (ep == NULL) {
            sprintf (buf, "%s/lsf.conf", LSETCDIR); // FIXME FIXME FIXME FIXME put in configure.ac
            fp = fopen (buf, "r");
        }
        else {
            memset (buf, 0, sizeof (buf));
            ls_strcat (buf, sizeof (buf), ep);
            ls_strcat (buf, sizeof (buf), "/lsf.conf"); // FIXME FIXME FIXME FIXME put in configure.ac
            fp = fopen (buf, "r");
        }
    }

    if (!fp) {
        lserrno = LSE_LSFCONF;
        return -1;
    }

    while ((line = getNextLineC_ (fp, &lineNum, TRUE)) != NULL) {
        int cc;
        cc = parseLine (line, &key, &value);
        if (cc < 0 && errLineNum_ == 0) {
            errLineNum_ = lineNum;
            assert( lserrno > 0 );
            saveErrNo = (u_int) lserrno;
            continue;
        }
        if (!matchEnv (key, pList1) && !matchEnv (key, pList2)) {
            continue;
        }


        if (!setConfEnv (key, value, pList1) || !setConfEnv (key, value, pList2)) {
            fclose (fp);
            return -1;
        }
    }
    fclose (fp);
    if (errLineNum_ != 0) {
        assert( saveErrNo <= INT_MAX );
        lserrno = (int) saveErrNo;
        return -1;
    }

    return 0;
}


int
parseLine (char *line, char **keyPtr, char **valuePtr)
{

    char       *cp   = NULL;
    const char *sp   = line;
    const char *word = NULL;
   
    static char key[L_MAX_LINE_LEN_4ENV];
    static char value[L_MAX_LINE_LEN_4ENV];

    if (strlen (sp) >= L_MAX_LINE_LEN_4ENV - 1) {
        lserrno = LSE_BAD_ENV;
        return -1;
    }

    *keyPtr = key;
    *valuePtr = value;

    word = getNextWord_ (&sp);

    strcpy (key, word);
    cp = strchr (key, '=');

    if (cp == NULL)
        {
            lserrno = LSE_CONF_SYNTAX;
            return -1;
        }

    *cp = '\0';

    sp = strchr (line, '=');

    if (sp[1] == ' ' || sp[1] == '\t')
        {
            lserrno = LSE_CONF_SYNTAX;
            return -1;
        }

    if (sp[1] == '\0')
        {
            value[0] = '\0';
            return 0;
        }

    sp++;
    word = getNextValueQ_ (&sp, '\"', '\"');
    if (!word)
        return -1;

    strcpy (value, word);

    word = getNextValueQ_ (&sp, '\"', '\"');
    if (word != NULL || lserrno != LSE_NO_ERR)
        {
            lserrno = LSE_CONF_SYNTAX;
            return -1;
        }

    return 0;

}

int
matchEnv ( const char *name, struct config_param *paramList)
{
    if (paramList == NULL) {
        return FALSE;
    }

    for (; paramList->paramName; paramList++) {
        if (strcmp (paramList->paramName, name) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

int
setConfEnv ( const char *name, char *value, struct config_param *paramList)
{
    if (paramList == NULL) {
        return 1;
    }

    if (value == NULL) {
        *value = '\0';
    }

    for (; paramList->paramName; paramList++)
        {
            if (strcmp (paramList->paramName, name) == 0)
    {
        FREEUP (paramList->paramValue);
        paramList->paramValue = putstr_ (value);
        if (paramList->paramValue == NULL)
            {
                lserrno = LSE_MALLOC;
                return 0;
            }
    }
        }
    return 1;
}
