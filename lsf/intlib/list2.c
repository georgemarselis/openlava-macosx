
/* Simple double linked list.
 * The idea is very simple we have 2 insertion methods
 * enqueue which adds at the end of the queue and push
 * which adds at the front. Then we simply pick the first
 * element in the queue. If you have inserted by enqueue
 * you get a FCFS policy if you pushed you get a stack policy.
 *
 *
 * FCFS
 *
 *   H->1->2->3->4
 *
 * you retrive elements as 1, 2 etc
 *
 * Stack:
 *
 * H->4->3->2->1
 *
 * you retrieve elements as 4,3, etc
 *
 *
 */

#include "list2.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

struct list_ *
listmake(const char *name)
{
    struct list_   *L;

    L = calloc(1, sizeof(struct list_));
    assert(L);
    L->forw = L->back = L;

    L->name = strdup(name);
    assert(L->name);

    return L;

} /* listmake() */

/* listinisert()
 *
 * Using cartesian coordinates the head h is at
 * zero and elemets are pushed along x axes.
 *
 *       <-- back ---
 *      /             \
 *     h <--> e2 <--> e
 *      \             /
 *        <-- forw --
 *
 * The h points the front, the first element of the list,
 * elements can be pushed in front or enqueued at the back.
 *
 */
int
listinsert(struct list_ *h,
           struct list_ *e,
           struct list_ *e2)
{
    assert(h && e && e2);

    /*  before: h->e
     */

    e->back->forw = e2;
    e2->back = e->back;
    e->back = e2;
    e2->forw = e;

    /* after h->e2->e
     */

    h->num++;

    return h->num;

} /* listinsert() */

/*
 * listenqueue()
 *
 * Enqueue a new element at the end
 * of the list.
 *
 * listenque()/listdeque()
 * implements FCFS policy.
 *
 */
int
listenque(struct list_ *h,
          struct list_ *e2)
{
    assert(h && e2);

    /* before: h->e
     */
    listinsert(h, h, e2);
    /* after: h->e->e2
     */
    return 0;
}

/* listdeque()
 */
struct list_ *
listdeque(struct list_ *h)
{
    struct list_   *e;

    if (h->forw == h) {
        assert(h->back == h);
        return(NULL);
    }

    /* before: h->e->e2
     */

    e = listrm(h, h->forw);

    /* after: h->e2
     */

    return e;
}

/*
 * listpush()
 *
 * Push e at the front of the list
 *
 * H --> e --> e2
 *
 */
int
listpush(struct list_ *h,
         struct list_ *e2)
{
    /* before: h->e
     */
    listinsert(h, h->forw, e2);

    /* after: h->e2->e
     */

    return 0;
}

/* listpop()
 */
struct list_ *
listpop(struct list_ *h)
{
    struct list_ *e;

    e = listdeque(h);

    return e;
}

struct list_ *
listrm(struct list_ *h,
       struct list_ *e)
{
    if (h->num == 0)
        return(NULL);

    e->back->forw = e->forw;
    e->forw->back = e->back;
    h->num--;

    return e;

} /* listrm() */


/* listfree()
 */
void
listfree(struct list_ *L,
         void (*f)(void *))
{
    struct list_   *l;

    while ((l = listpop(L))) {
        if (f == NULL)
            free(l);
        else
            (*f)(l);
    }

    free(L->name);
    free(L);
}
