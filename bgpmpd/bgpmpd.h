#ifndef __BGPMPD_H__
#define __BGPMPD_H__

#include "thread.h"
#include "workqueue.h"
#include "qobj.h"

struct bgpmp_master {
    struct thread_master *master;
    struct work_queue *process_main_queue;
    QOBJ_FIELDS
};
DECLARE_QOBJ_TYPE(bgpmp_master)

extern struct bgpmp_master *bm;

struct bgpmp_node {
    unsigned int dummy;
    STAILQ_ENTRY(bgpmp_node) pq;
};

struct bgpmp_process_queue {
    unsigned int dummy;
    STAILQ_HEAD(, bgpmp_node) pqueue;
};

extern void bgpmp_master_init(struct thread_master *master);

extern void bgpmp_process_queue_init(void);

#endif  /* __BGPMPD_H__ */