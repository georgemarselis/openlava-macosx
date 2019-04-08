/* $Id: cmd.err.c 397 2007-11-26 19:04:00Z mblack $
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

#include <pwd.h>
#include <netdb.h>
#include <ctype.h>

#include "lsb/lsb.h"
#include "cmdtools/cmdtools.h"


#define NL_SETN 8


void
jobInfoErr (size_t jobId, char *jobName, char *user, char *queue, char *host, int options)
{
    char errMsg[ MAX_LINE_LEN ];
    char hostOrQueue[ MAX_LINE_LEN ];

    memset( errMsg, '\0', MAX_LINE_LEN );
    memset( hostOrQueue, '\0', MAX_LINE_LEN );

    if (user && lsberrno == LSBE_BAD_USER) {
        lsb_perror (user);
        return;
    }
    if (queue && lsberrno == LSBE_BAD_QUEUE) {
        lsb_perror (queue);
        return;
    }
    if (host && lsberrno == LSBE_BAD_HOST) {
        lsb_perror (host);
        return;
    }

    if (lsberrno == LSBE_NO_JOB) {

        if (queue)
        {
            strcpy (hostOrQueue, (_i18n_msg_get (ls_catd, NL_SETN, 801, " in queue <")));   /* catgets  801  */
            strcat (hostOrQueue, queue);
        }
        if (host)
        {
            if (ls_isclustername (host) <= 0)
            {
                if (hostOrQueue[0] == '\0')
        strcpy (hostOrQueue, (_i18n_msg_get (ls_catd, NL_SETN, 802, " on host/group <")));  /* catgets  802  */
                    else
        strcat (hostOrQueue, (_i18n_msg_get (ls_catd, NL_SETN, 803, "> and on host/group <"))); /* catgets  803  */
                }
            else
            {
                if (hostOrQueue[0] == '\0')
        strcpy (hostOrQueue, (_i18n_msg_get (ls_catd, NL_SETN, 804, " in cluster <"))); /* catgets  804  */
                    else
        strcat (hostOrQueue, (_i18n_msg_get (ls_catd, NL_SETN, 805, "> and in cluster <")));    /* catgets  805  */
                }
            strcat (hostOrQueue, host);
        }
        if (hostOrQueue[0] != '\0') {
            strcat (hostOrQueue, ">");
        }

        if (jobId) {
            if (options & JGRP_ARRAY_INFO) {
                if (LSB_ARRAY_IDX (jobId)) {
                    sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 806, "Job <%s> is not a job array")), lsb_jobid2str (jobId));  /* catgets  806  */
                }
                else  {
                    sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 807, "Job array <%s> is not found%s")), lsb_jobid2str (jobId), hostOrQueue); /* catgets  807  */
                }
            }
            else  {
                sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 808, "Job <%s> is not found%s")), lsb_jobid2str (jobId), hostOrQueue); /* catgets  808  */
            }
        }
        else if (jobName && !strcmp (jobName, "/") && (options & JGRP_ARRAY_INFO)) {
            /* catgets  810  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 810, "Job array is not found%s")), hostOrQueue);
        }
        else if (jobName)
        {
            if (options & JGRP_ARRAY_INFO) {
                if (strchr (jobName, '[')) {
                    sprintf (errMsg,(_i18n_msg_get(ls_catd, NL_SETN, 806, "Job <%s> is not a job array")), jobName);
                }
                else {
                    sprintf (errMsg,(_i18n_msg_get(ls_catd, NL_SETN, 807, "Job array <%s> is not found%s")), jobName, hostOrQueue);
                }
            }
            else {
                sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 808, "Job <%s> is not found%s")), jobName, hostOrQueue);
            }
        }
        else if (options & ALL_JOB) {
            /* catgets  814  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 814, "No job found%s")), hostOrQueue);
        }
        else if (options & (CUR_JOB | LAST_JOB)) {
            /* catgets  815  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 815, "No unfinished job found%s")), hostOrQueue);
        }
        else if (options & DONE_JOB) {
            /* catgets  816  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 816, "No DONE/EXIT job found%s")), hostOrQueue);
        }
        else if (options & PEND_JOB) {
            /* catgets  817  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 817, "No pending job found%s")), hostOrQueue);
        }
        else if (options & SUSP_JOB) {
            /* catgets  818  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 818, "No suspended job found%s")), hostOrQueue);
        }
        else if (options & RUN_JOB) {
            /* catgets  819  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 819, "No running job found%s")), hostOrQueue);
        }
        else {
            /* catgets  820  */
            sprintf (errMsg, (_i18n_msg_get (ls_catd, NL_SETN, 820, "No job found")));
        }

        fprintf (stderr, "%s\n", errMsg);
        return;
    }

    lsb_perror (NULL);

    return;
}
