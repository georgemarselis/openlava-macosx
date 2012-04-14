
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
    struct lsbCores *cores;
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
    if (je == NULL) {
        lsb_perror("lsb_wait4event");
        return -1;
    }

    switch (je->event) {
        case EVENT_ADD_CORES:
            printf("Got EVENT_ADD_CORES:\n");
            cores = je->e;
            for (i = 0; i < cores->num; i++) {
                printf(" %s\n", cores->cores[i]);
            }
            break;
        case EVENT_RECALL_CORES:
        case EVENT_JOB_KILLED:
        case EVENT_CONNECTION_ERROR:
            return -1;
    }

    for (i = 0; i < 60; i++)
        sleep(1);

    printf("Done work %d\n", i);

    cc = lsb_signaljob(je->jobID, SIGKILL);
    if (cc < 0) {
        lsb_perror("lsb_signaljob()");
        return -1;
    }

    return 0;

} /* main() */
