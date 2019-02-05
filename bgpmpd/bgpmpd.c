#include <zebra.h>

#include "log.h"
#include "thread.h"
#include "qobj.h"
#include "workqueue.h"

#include "bgpmpd/bgpmpd.h"
#include "bgpmpd/bgpmp_memory.h"

DEFINE_QOBJ_TYPE(bgpmp_master)

/* BGPMP process wide configuration. */
static struct bgpmp_master bgpmp_master;

/* BGPMP process wide configuration pointer to export.  */
struct bgpmp_master *bm;

static void bgpmp_process_main_one(struct bgpmp_node *node)
{
    zlog_err("In BGPMP_PROCESS_MAIN_ONE.");
}

static struct bgpmp_process_queue *bgpmp_processq_alloc(unsigned int msg)
{
    struct bgpmp_process_queue *pqnode;

    pqnode = XCALLOC(MTYPE_BGPMP_PROCESS_QUEUE,
             sizeof(struct bgpmp_process_queue));

    pqnode->dummy = msg;

    STAILQ_INIT(&pqnode->pqueue);

    return pqnode;
}

static wq_item_status bgpmp_process_wq(struct work_queue *wq, void *data)
{
    struct bgpmp_process_queue *pqnode = data;
    struct bgpmp_node *node;

    while (!STAILQ_EMPTY(&pqnode->pqueue))
    {
        node = STAILQ_FIRST(&pqnode->pqueue);
        STAILQ_REMOVE_HEAD(&pqnode->pqueue, pq);
        STAILQ_NEXT(node, pq) = NULL; /* complete unlink */

        bgpmp_process_main_one(node);
    }

    return WQ_SUCCESS;
}

static void bgpmp_processq_del(struct work_queue *wq, void *data)
{
    zlog_err("In BGPMP_PROCESSQ_DEL.");
    // XFREE(MTYPE_BGPMP_PROCESS_QUEUE, pqnode);
}

static struct bgpmp_node *bgpmp_node_create(unsigned int msg)
{
    struct bgpmp_node *node;
    node = XCALLOC(MTYPE_BGPMP_NODE, sizeof(struct bgpmp_node));

    return node;
}

void bgpmp_add_to_wq(unsigned int msg)
{
    struct work_queue *wq = bm->process_main_queue;
    struct bgpmp_process_queue *pqnode;
    struct bgpmp_node *bgpmp_node;

    // Missing some specific logic from BGP, but should work for testing
    pqnode = bgpmp_processq_alloc(msg);
    bgpmp_node = bgpmp_node_create(msg + 1);

    // assert(STAILQ_NEXT())
    STAILQ_INSERT_TAIL(&pqnode->pqueue, bgpmp_node, pq);
    work_queue_add(wq, pqnode);

    return;
}

void bgpmp_process_queue_init(void)
{
    if (!bm->process_main_queue)
    {
        bm->process_main_queue = 
            work_queue_new(bm->master, "process_main_queue");
    }

    bm->process_main_queue->spec.workfunc = &bgpmp_process_wq;
    bm->process_main_queue->spec.del_item_data = &bgpmp_processq_del;
    bm->process_main_queue->spec.max_retries = 0;
    bm->process_main_queue->spec.hold = 50;
    bm->process_main_queue->spec.yield = 50 * 1000L;
}

void bgpmp_master_init(struct thread_master *master)
{
    qobj_init();

    memset(&bgpmp_master, 0, sizeof(struct bgpmp_master));

    bm = &bgpmp_master;
    bm->master = master;

    bgpmp_process_queue_init();

    QOBJ_REG(bm, bgpmp_master);
}