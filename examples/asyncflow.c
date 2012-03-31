
/* Async flow simulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

static int numRes;
static char *resources[1024];
static int isWorkDone;
static int isResReady;

static void *makeResReady(void *);
static void *asResReady(void *);
static FILE *getResources(const char *, void *(f)(void *));

int
main(int argc, char **argv)
{
    pthread_attr_t attr;
    pthread_t t;
    int num;
    int s;
    char *file;
    FILE *fp;

    file = "resource.dat";

    num = 1;

znovu:
    s = pthread_attr_init(&attr);
    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    /* Start a thread that knows what to do when resources
     * becomes available.
     */
    pthread_create(&t, &attr, asResReady, NULL);
    printf("asResReady thread launched %lu\n", t);

    /* Get the resource file descriptor and
     * invoke makeResReady. This functions
     * open the connection fp to resource manager
     * and invokes a call back on the resources.
     */
    fp = getResources(file, makeResReady);

    while (!isWorkDone) {
        printf("\
%s: thread %lu waiting for run number %d\n", __func__,
               pthread_self(), num);
        sleep(1);
    }

    printf("\
%s: thread %lu run number %d done.\n", __func__,
           pthread_self(), num);

    /* Cleanup and reinitialize
     */
    fclose(fp);
    for (s = 0; s < numRes; s++)
        free(resources[s]);
    isWorkDone = 0;
    isResReady = 0;

    ++num;
    sleep(5);
    goto znovu;

    return 0;
}

static FILE *
getResources(const char *file, void *(f)(void *))
{
    FILE *fp;
    int s;
    pthread_attr_t attr;
    pthread_t t;

    /* opens the connection to the
     * resource manager
     */
    fp = fopen(file, "r");

    /* Launch the user callback which process the
     * resources and returns. This user callback
     * has to have a communication bus with a user
     * resource consuming function, in our model
     * is the resources array.
     */
    s = pthread_attr_init(&attr);
    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &attr, f, fp);
    printf("\
%s: resource allocation thread %lu launched\n", __func__, t);

    return fp;
}

static void *
makeResReady(void *v)
{
    FILE *fp;
    char buf[128];

    numRes = 0;
    fp = v;
    while (fgets(buf, sizeof(buf), fp)) {
        resources[numRes] = strdup(buf);
        ++numRes;
    }

    isResReady = 1;
    printf("\
%s: thread %lu resources are ready\n", __func__, pthread_self());

    return NULL;
}

static void *
asResReady(void *v)
{
    int i;

    while (1) {
        /* This is the user functiont that knows
         * what to do with the resources, for example
         * start an ssh session on each host.
         */
        if (isResReady == 0) {
            sleep(1);
            continue;
        }
        printf("\
%s: thread %lu resources are ready\n", __func__, pthread_self());
        for (i = 0; i < numRes; i++)
            printf(" %s", resources[i]);
        break;
    }

    isWorkDone = 1;

    return NULL;
}
