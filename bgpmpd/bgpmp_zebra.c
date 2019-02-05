#include <zebra.h>
#include "string.h"

#include "log.h"
#include "zclient.h"

#include "bgpmp_zebra.h"

/* Zebra structure to hold current status. */
struct zclient *zclient = NULL;

/* For registering threads. */
// extern struct thread_master *master;

static void send_message_to_bgpmp_server(char *message, uint16_t len)
{
	char buffer[1024];
	struct sockaddr_in serverAddr;

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(7891);
	serverAddr.sin_addr.s_addr = inet_addr("172.17.23.93");
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	socklen_t addr_size = sizeof serverAddr;

	connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

	strncpy(buffer, message, len);
	send(clientSocket, buffer, len, 0);

	close(clientSocket);	
}

static void bgpmp_zebra_connected(struct zclient *zclient)
{
	// /* We need router-id information. */
	// zebra_message_send(zclient, ZEBRA_ROUTER_ID_ADD, VRF_DEFAULT);

	// /* We need interface information. */
	// zebra_message_send(zclient, ZEBRA_INTERFACE_ADD, VRF_DEFAULT);



	// zebra_message_send(zclient, ZEBRA_ROUTE_ADD, VRF_DEFAULT);
	// zebra_message_send(zclient, ZEBRA_NEXTHOP_UPDATE, VRF_DEFAULT);
	// zebra_message_send(zclient, ZEBRA_HELLO, VRF_DEFAULT);
	// zebra_message_send(zclient, ZEBRA_REDISTRIBUTE_ROUTE_ADD, VRF_DEFAULT);
	// zebra_message_send(zclient, ZEBRA_REDISTRIBUTE_ADD, VRF_DEFAULT);

	zclient_send_reg_requests(zclient, VRF_DEFAULT);
	// zclient_send_rnh(zclient, ZEBRA_IMPORT_ROUTE_REGISTER);
	// zclient_send_rnh(zclient, ZEBRA_NEXTHOP_REGISTER);

    zlog_err("Connected BGPMP to Zebra.");
}

static int bgpmp_zebra_read_route(int command, struct zclient *zclient,
			    zebra_size_t length, vrf_id_t vrf_id)
{
    zlog_err("In BGPMP_ZEBRA_READ_ROUTE.");

	char msg[1024];
	int slen = sprintf(msg, "In BGPMP_ZEBRA_READ_ROUTE, %hu", length);

	send_message_to_bgpmp_server(msg, slen);
	return 0;
}

static int bgpmp_zebra_read_nexthop_update(int command, struct zclient *zclient,
			    zebra_size_t length, vrf_id_t vrf_id)
{
    zlog_err("IN_BGPMP_ZEBRA_READ_NEXTHOP_UPDATE.");
	return 0;
}

static int bgpmp_zebra_fec_update(int command, struct zclient *zclient, zebra_size_t length)
{
	zlog_err("IN_BGPMP_ZEBRA_FEC_UPDATE.");
	return 0;
}

static void bgpmp_zebra_read_local_prefix(int command, struct zclient *zclient,
			    zebra_size_t length, vrf_id_t vrf_id)
{
    zlog_err("In BGPMP_ZEBRA_READ_LOCAL_PREFIX.");
}

extern struct zebra_privs_t bgpmp_privs;

void bgpmp_zebra_init(struct thread_master *master)
{
    struct zclient_options opt = {.receive_notify = true};
    
    zclient = zclient_new(master, &opt);

    zclient_init(zclient, ZEBRA_ROUTE_BGP, 0, &bgpmp_privs);

    zclient->zebra_connected = bgpmp_zebra_connected;
    zclient->redistribute_route_add = bgpmp_zebra_read_route;
	zclient->redistribute_route_del = bgpmp_zebra_read_route;
	zclient->nexthop_update = bgpmp_zebra_read_nexthop_update;

    // Following aren't logically correct, but registered for experimentation
	zclient->import_check_update = bgpmp_zebra_read_route;
	zclient->fec_update = bgpmp_zebra_fec_update;
	zclient->local_es_add = bgpmp_zebra_read_route;
	zclient->local_es_del = bgpmp_zebra_read_route;
	zclient->local_vni_add = bgpmp_zebra_read_route;
	zclient->local_vni_del = bgpmp_zebra_read_route;
	zclient->local_macip_add = bgpmp_zebra_read_route;
	zclient->local_macip_del = bgpmp_zebra_read_route;
	zclient->local_l3vni_add = bgpmp_zebra_read_route;
	zclient->local_l3vni_del = bgpmp_zebra_read_route;
	zclient->local_ip_prefix_add = bgpmp_zebra_read_local_prefix;
	zclient->local_ip_prefix_del = bgpmp_zebra_read_local_prefix;
	zclient->label_chunk = bgpmp_zebra_read_local_prefix;
	zclient->rule_notify_owner = bgpmp_zebra_read_route;
	zclient->ipset_notify_owner = bgpmp_zebra_read_route;
	zclient->ipset_entry_notify_owner = bgpmp_zebra_read_route;
	zclient->iptable_notify_owner = bgpmp_zebra_read_route;
	zclient->interface_up = bgpmp_zebra_read_route;
	zclient->interface_down = bgpmp_zebra_read_route;
	zclient->router_id_update = bgpmp_zebra_read_route;
	zclient->interface_add = bgpmp_zebra_read_route;
	zclient->interface_delete = bgpmp_zebra_read_route;
	zclient->interface_address_add = bgpmp_zebra_read_route;
	zclient->interface_address_delete = bgpmp_zebra_read_route;
	zclient->interface_nbr_address_add = bgpmp_zebra_read_route;
	zclient->interface_nbr_address_delete = bgpmp_zebra_read_route;
	zclient->interface_vrf_update = bgpmp_zebra_read_route;
}