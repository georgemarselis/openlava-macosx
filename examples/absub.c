
#include "../lsf/lsf.h"
#include "../lsbatch/lsbatch.h"

extern char *getNextWord_(char **);

int
main(int argc,
     char **argv)
{
    struct submit req;
    struct submitReply rep;
    LS_LONG_INT jobID;
    struct lsbJobEvent *je;
    int timeout;
    int cc;
    int i;

    cc = lsb_init((char *)__func__);
    if (cc < 0) {
        fprintf(stderr, "%s\n", lsb_sysmsg());
        return -1;
    }

    memset(&req, 0, sizeof(struct submit));
    for (i = 0; i < LSF_RLIM_NLIMITS; i++)
        req.rLimits[i] = DEFAULT_RLIMIT;
    req.userPriority = -1;
    req.options2 = SUB2_KEEP_CONNECT;
    req.command = "/bin/sleep 86400";
    req.numProcessors = 1;
    req.maxNumProcessors = 1;
    memset(&rep, 0, sizeof(struct submitReply));

    jobID = lsb_asyncsubmit(&req, &rep, NULL, NULL);
    if (jobID < 0) {
        lsb_perror("lsb_asyncsubmit()");
        return -1;
    }

    timeout = -1;
    je = lsb_wait4event(timeout);

    return 0;

} /* main() */
