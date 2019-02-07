#ifndef __BGPMP_H__
#define __BGPMP_H__

extern void bgpmp_add_to_wq(unsigned int msg);

extern void bgpmp_process_queue_init(void);

struct bgpmp_node {
    unsigned int dummy;
    STAILQ_ENTRY(bgpmp_node) pq;
};

struct bgpmp_process_queue {
    unsigned int dummy;
    STAILQ_HEAD(, bgpmp_node) pqueue;
};

#endif /* __BGPMP_H__ */