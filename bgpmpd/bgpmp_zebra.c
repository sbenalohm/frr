#include <zebra.h>

#include "log.h"
#include "zclient.h"

#include "bgpmp_zebra.h"

/* Zebra structure to hold current status. */
struct zclient *zclient = NULL;

/* For registering threads. */
// extern struct thread_master *master;

static void bgpmp_zebra_connected(struct zclient *zclient)
{
    zlog_err("Connected BGPMP to Zebra.");
}

static int bgpmp_zebra_read_route(int command, struct zclient *zclient,
			    zebra_size_t length, vrf_id_t vrf_id)
{
    zlog_err("In BGPMP_ZEBRA_READ_ROUTE.");
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