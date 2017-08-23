/**
 * @example monitor.c
 *
 * This example monitors the connection of IngeniaLink devices/nodes.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

void on_found(void *ctx, uint8_t id)
{
	(void)ctx;

	printf("Found node with id 0x%02x\n", id);
}

void on_added(void *ctx, const char *port)
{
	il_net_t *net;
	il_net_nodes_list_t *nodes;

	(void)ctx;

	printf("Plugged device %s\n", port);

	/* create network */
	net = il_net_create(port, IL_NET_TIMEOUT_DEF);
	if (!net)
		return;

	/* scan */
	printf("Scanning...\n");
	nodes = il_net_nodes_list_get(net, on_found, NULL);
	printf("Scanning finished\n");

	/* free resources */
	il_net_nodes_list_destroy(nodes);
	il_net_destroy(net);
}

int main(void)
{
	il_net_dev_mon_t *mon;

	mon = il_net_dev_mon_create(on_added, NULL);
	if (!mon) {
		fprintf(stderr, "Could not create monitor: %s\n", ilerr_last());
		return 1;
	}

	printf("Press ENTER to stop monitoring\n");
	getchar();
	printf("Stopping...\n");

	il_net_dev_mon_destroy(mon);

	return 0;
}
