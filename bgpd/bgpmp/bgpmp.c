#include <zebra.h>

#include "log.h"
#include "memory.h"
#include "qobj.h"
#include "workqueue.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgpmp/bgpmp.h"
#include "bgpd/bgpmp/bgpmp_memory.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

static const char *inet_2a(uint32_t a, char *b)
{
	sprintf(b, "%u.%u.%u.%u", (a)&0xFF, (a >> 8) & 0xFF, (a >> 16) & 0xFF,
		(a >> 24) & 0xFF);
	return b;
}

static void send_message_to_bgpmp_server(char *message, uint16_t len)
{
    char buffer[1024];
    struct sockaddr_in serverAddr;

    int clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("172.17.5.222");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    socklen_t addr_size = sizeof serverAddr;

    connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

    strncpy(buffer, message, len);
    send(clientSocket, buffer, len, 0);

    close(clientSocket);
}

static struct bgpmp_node *bgpmp_node_create(unsigned int msg)
{
    struct bgpmp_node *node;
    node = XCALLOC(MTYPE_BGPMP_NODE, sizeof(struct bgpmp_node));

    return node;
}

static struct bgpmp_node *bgpmp_node_peer_up_create(uint32_t peer_address, uint16_t local_port, uint16_t peer_port)
{
    struct bgpmp_node *node;
    node = XCALLOC(MTYPE_BGPMP_NODE, sizeof(struct bgpmp_node));

    node->dummy = 20;
    node->peer_up_notification.peer_address = peer_address;
    node->peer_up_notification.local_port = local_port;
    node->peer_up_notification.peer_port = peer_port;

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

    // send_message_to_bgpmp_server("Hello from BGPMP process queue", 32);
    char ip_addr[20];
    inet_2a(node->peer_up_notification.peer_address, ip_addr);
    char buf[1024];
    buf[1023] = '\0';

    snprintf(buf, 1022, "BGP Peer: %s --- %d --- %d\n", ip_addr, node->peer_up_notification.local_port, node->peer_up_notification.peer_port);
    send_message_to_bgpmp_server(buf, 100);    
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

void bgpmp_add_peer_up_to_wq(uint32_t peer_address, uint16_t local_port, uint16_t peer_port)
{
    struct work_queue *wq = bm->bgpmp_process_queue;
    struct bgpmp_process_queue *pqnode;
    struct bgpmp_node *bgpmp_node;

    pqnode = bgpmp_processq_alloc(16);
    bgpmp_node = bgpmp_node_peer_up_create(peer_address, local_port, peer_port);

    STAILQ_INSERT_TAIL(&pqnode->pqueue, bgpmp_node, pq);
    work_queue_add(wq, pqnode);

    return;    
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