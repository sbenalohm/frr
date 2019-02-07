#include "bgpmpd/bgpmpd.h"
#include "bgpmpd/bgpmp_external.h"

void bgpmp_add_to_wq(unsigned int msg)
{
    struct work_queue *wq = bgpmp_m->process_main_queue;
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
