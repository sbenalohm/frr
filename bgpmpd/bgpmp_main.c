#include <zebra.h>

#include <lib/version.h>
#include "memory.h"
#include "privs.h"
#include "zclient.h"
#include "libfrr.h"
#include "log.h"
#include "config.h"
#include "getopt.h"

// For socket server
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "bgpmpd/bgpmp_zebra.h"
#include "bgpmpd/bgpmpd.h"

char *collector_address;

static zebra_capabilities_t _caps_p[] = {ZCAP_BIND, ZCAP_NET_RAW,
					 ZCAP_NET_ADMIN, ZCAP_SYS_ADMIN};
					 
struct zebra_privs_t bgpmp_privs = {
#if defined(FRR_USER) && defined(FRR_GROUP)
	.user = FRR_USER,
	.group = FRR_GROUP,
#endif
#if defined(VTY_GROUP)
	.vty_group = VTY_GROUP,
#endif
	.caps_p = _caps_p,
	.cap_num_p = array_size(_caps_p),
	.cap_num_i = 0};

// /* Master of threads. */
// struct thread_master *master;

/* SIGHUP handler. */
static void sighup(void)
{
	zlog_info("SIGHUP received");
}

static void send_message_to_bgpmp_server(char *message, uint16_t len);
static void send_hello_signal_to_bgpmp_server(void);

/* SIGINT / SIGTERM handler. */
static void sigint(void)
{
	zlog_notice("Terminating on signal");

	send_hello_signal_to_bgpmp_server();

	exit(0);
}

/* SIGUSR1 handler. */
static void sigusr1(void)
{
	zlog_rotate();
}

static void send_message_to_bgpmp_server(char *message, uint16_t len)
{
	char buffer[1024];
	struct sockaddr_in serverAddr;

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(7891);
	serverAddr.sin_addr.s_addr = inet_addr(collector_address);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	socklen_t addr_size = sizeof serverAddr;

	connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

	strncpy(buffer, message, len);
	send(clientSocket, buffer, len, 0);

	close(clientSocket);	
}

static void send_hello_signal_to_bgpmp_server()
{
	send_message_to_bgpmp_server("Hello World\n", 13);	
}

struct quagga_signal_t bgpmp_signals[] = {
	{
		.signal = SIGHUP,
		.handler = &sighup
	},
	{
		.signal = SIGUSR1,
		.handler = &sigusr1,
	},
	{
		.signal = SIGINT,
		.handler = &sigint,
	},
	{
		.signal = SIGTERM,
		.handler = &sigint,
	},
};

FRR_DAEMON_INFO(bgpmpd, BGPMP,
	.proghelp = "Implementation of BGP Monitoring Protocol.",
	.signals = bgpmp_signals,
	.n_signals = array_size(bgpmp_signals),
	.privs = &bgpmp_privs)

/* bgpmpd options, we use GNU getopt library. */
static const struct option longopts[] = {
	{"collector", required_argument, NULL, 'c'},
	{0}};

int main(int argc, char **argv, char **envp)
{
	frr_preinit(&bgpmpd_di, argc, argv);

	frr_opt_add("c", longopts,
		"  -c, --collector		Set address to forward BGP messages to.");

	while (1) {
		int opt;

		opt = frr_getopt(argc, argv, NULL);

		if (opt == EOF)
			break;

		switch (opt) {
		case 0:
			break;
		case 'c':
			collector_address = optarg;
			break;
		default:
			frr_help_exit(1);
			break;
		}
	}
	
	if (!collector_address)
	{
		collector_address = "172.17.23.93";
	}

	bgpmp_master_init(frr_init());
	// master = frr_init();

	bgpmp_zebra_init(bm->master);

	frr_config_fork();
	frr_run(bm->master);

	/* Not reached. */
	return (0);
}