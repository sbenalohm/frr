#include <zebra.h>

#include "log.h"
#include "memory.h"
#include "qobj.h"
#include "workqueue.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgpmp/bgpmp.h"
#include "bgpd/bgpmp/bgpmp_memory.h"

static struct bgpmp_node *bgpmp_node_create(unsigned int msg)
{
    struct bgpmp_node *node;
    node = XCALLOC(MTYPE_BGPMP_NODE, sizeof(struct bgpmp_node));

    return node;
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

static void bgpmp_process_main_one(struct bgpmp_node *node)
{
    zlog_err("In BGPMP_PROCESS_MAIN_ONE.");
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

void bgpmp_add_to_wq(unsigned int msg)
{
    struct work_queue *wq = bm->bgpmp_process_queue;
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
    if (!bm->bgpmp_process_queue)
    {
        bm->bgpmp_process_queue =
            work_queue_new(bgp_pth_bgpmp->master, "bgpmp_process_queue");
    }

    bm->bgpmp_process_queue->spec.workfunc = &bgpmp_process_wq;
    bm->bgpmp_process_queue->spec.del_item_data = &bgpmp_processq_del;
    bm->bgpmp_process_queue->spec.max_retries = 0;
    bm->bgpmp_process_queue->spec.hold = 50;
    bm->bgpmp_process_queue->spec.yield = 50 * 1000L;
}