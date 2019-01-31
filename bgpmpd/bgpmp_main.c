#include <zebra.h>

#include <lib/version.h>
#include "memory.h"
#include "privs.h"
#include "zclient.h"
#include "libfrr.h"
#include "log.h"
#include "config.h"
#include "getopt.h"

#include "bgpmp_zebra.h"

zebra_capabilities_t _caps_p[] = {};

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

/* Master of threads. */
struct thread_master *master;

/* SIGHUP handler. */
static void sighup(void)
{
	zlog_info("SIGHUP received");
}

/* SIGINT / SIGTERM handler. */
static void sigint(void)
{
	zlog_notice("Terminating on signal");

	exit(0);
}

/* SIGUSR1 handler. */
static void sigusr1(void)
{
	zlog_rotate();
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
	zlog_err("Starting BGPMP...");

	char *collector_address = NULL;

	frr_preinit(&bgpmpd_di, argc, argv);

	frr_opt_add("c", longopts,
		"  -c, --collector		Set address to forward BGP messages to.");

	// zlog_set_level(ZLOG_DEST_FILE, LOG_DEBUG);

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
	
	master = frr_init();

	bgpmp_zebra_init(master);

	frr_config_fork();
	frr_run(master);

	/* Not reached. */
	return 0;
}