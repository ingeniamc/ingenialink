/**
 * @example servo_monitor.c
 *
 * This example monitors the connection of devices/servos.
 */

#include "utils.h"

#include <stdio.h>

void on_found(void *ctx, uint8_t id)
{
	(void)ctx;

	printf("Found servo with id 0x%02x\n", id);
}

void on_evt(void *ctx, il_net_dev_evt_t evt, const char *port)
{
	il_net_prot_t *prot = ctx;

	if (evt == IL_NET_DEV_EVT_ADDED) {
		il_net_t *net;
		il_net_opts_t opts;
		il_net_servos_list_t *servos;

		printf("Plugged device %s\n", port);

		/* create network */
		opts.port = port;
		opts.timeout_rd = IL_NET_TIMEOUT_RD_DEF;
		opts.timeout_wr = IL_NET_TIMEOUT_WR_DEF;

		net = il_net_create(*prot, &opts);
		if (!net)
			return;

		/* scan */
		printf("Scanning...\n");
		servos = il_net_servos_list_get(net, on_found, NULL);
		printf("Scanning finished\n");

		/* free resources */
		il_net_servos_list_destroy(servos);
		il_net_destroy(net);
	} else {
		printf("Unplugged device %s\n", port);
	}
}

int main(int argc, const char *argv[])
{
	int r;
	il_net_prot_t prot;
	il_net_dev_mon_t *mon;

	if (argc < 2) {
		fprintf(stderr, "Usage: ./servo_monitor PROT\n");
		return -1;
	}

	prot = str2prot(argv[1]);

	mon = il_net_dev_mon_create(prot);
	if (!mon) {
		fprintf(stderr, "Could not create monitor: %s\n", ilerr_last());
		return 1;
	}

	r = il_net_dev_mon_start(mon, on_evt, &prot);
	if (r < 0) {
		goto cleanup;
	}

	printf("Press ENTER to stop monitoring\n");
	getchar();
	printf("Stopping...\n");

	il_net_dev_mon_stop(mon);

cleanup:
	il_net_dev_mon_destroy(mon);

	return r;
}
