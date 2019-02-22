#ifndef __BGPMP_H__
#define __BGPMP_H__

void bgpmp_add_peer_to_wq(struct peer *peer);
void bgpmp_add_peer_up_to_wq(uint32_t peer_address, uint16_t local_port, uint16_t peer_port, uint32_t peer_as);
extern void bgpmp_add_to_wq(unsigned int msg);

extern void bgpmp_process_queue_init(void);

typedef struct bgpmp_peer_up_notification {
    uint32_t peer_address; // net.IP?
    uint16_t local_port;
    uint16_t peer_port;
} bgpmp_peer_up_notification;

typedef struct _bgpmp_header {
    uint32_t length;
    uint8_t version;
    uint8_t type;
} bgpmp_header;

typedef struct _bgpmp_peer_header {
    uint64_t peer_distinguisher;
    uint64_t timestamp;
    uint32_t peer_address;
    uint32_t peer_as;
    uint32_t peer_bgp_id;
    uint8_t peer_type;
    uint8_t flags;
} bgpmp_peer_header;

typedef struct _bgpmp_message {
    bgpmp_peer_header peer_header;
    bgpmp_header header;
    unsigned int dummy;
} bgpmp_message;

struct bgpmp_node {
    // bgpmp_peer_up_notification peer_up_notification;
    bgpmp_message message;
    STAILQ_ENTRY(bgpmp_node) pq;
};

struct bgpmp_process_queue {
    STAILQ_HEAD(, bgpmp_node) pqueue;
};

#endif /* __BGPMP_H__ */