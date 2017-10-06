/**
 * @example list.c
 *
 * This example scans for all availe servo on every IngeniaLink network device.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

void on_found(void *ctx, uint8_t id)
{
	(void)ctx;

	printf("Found servo with id 0x%02x\n", id);
}

int main(void)
{
	il_net_dev_list_t *devs, *dev;
	il_net_servos_list_t *servos;

	devs = il_net_dev_list_get();
	il_net_dev_list_foreach(dev, devs) {
		/* create network */
		il_net_t *net = il_net_create(dev->port);
		if (!net)
			continue;

		/* scan (use callback to print on the go) */
		printf("Scanning %s...\n", dev->port);

		servos = il_net_servos_list_get(net, on_found, NULL);

		/* free resources */
		il_net_servos_list_destroy(servos);
		il_net_destroy(net);
	}

	il_net_dev_list_destroy(devs);

	return 0;
}
