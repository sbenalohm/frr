#ifndef __BGPMP_H__
#define __BGPMP_H__

extern void bgpmp_add_peer_up_to_wq(uint32_t peer_address, uint16_t local_port, uint16_t peer_port);
extern void bgpmp_add_to_wq(unsigned int msg);

extern void bgpmp_process_queue_init(void);

typedef struct bgpmp_peer_up_notification {
    uint32_t peer_address; // net.IP?
    uint16_t local_port;
    uint16_t peer_port;
} bgpmp_peer_up_notification;

struct bgpmp_node {
    unsigned int dummy;
    bgpmp_peer_up_notification peer_up_notification;
    STAILQ_ENTRY(bgpmp_node) pq;
};

struct bgpmp_process_queue {
    unsigned int dummy;
    STAILQ_HEAD(, bgpmp_node) pqueue;
};

#endif /* __BGPMP_H__ */