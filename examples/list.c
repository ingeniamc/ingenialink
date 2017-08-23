/**
 * @example list.c
 *
 * This example scans for all available IngeniaLink devices.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

void on_found(void *ctx, uint8_t id)
{
	(void)ctx;

	printf("Found node with id 0x%02x\n", id);
}

int main(void)
{
	il_net_dev_list_t *devs, *dev;
	il_net_nodes_list_t *nodes;

	devs = il_net_dev_list_get();
	il_net_dev_list_foreach(dev, devs) {
		/* create network */
		il_net_t *net = il_net_create(dev->port, IL_NET_TIMEOUT_DEF);
		if (!net)
			continue;

		/* scan (use callback to print on the go) */
		printf("Scanning %s...\n", dev->port);

		nodes = il_net_nodes_list_get(net, on_found, NULL);

		/* free resources */
		il_net_nodes_list_destroy(nodes);
		il_net_destroy(net);
	}

	il_net_dev_list_destroy(devs);

	return 0;
}
