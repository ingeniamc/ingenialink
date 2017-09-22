/**
 * @example watcher.c
 *
 * Simple register watcher examples.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

/** Watcher base period (ms) */
#define WATCHER_BASE_PERIOD	100

/** Position watch period (ms) */
#define POS_WATCH_PERIOD	500

/** Velocity watch period (ms) */
#define VEL_WATCH_PERIOD	100

void on_pos_changed(void *ctx, double value)
{
	(void)ctx;

	printf("New position: %.2f\n", value);
}

void on_vel_changed(void *ctx, double value)
{
	(void)ctx;

	printf("New velocity: %.2f\n", value);
}

static int run(const char *port, uint8_t id)
{
	int r = 0;

	il_net_t *net;
	il_axis_t *axis;
	il_watcher_t *watcher;

	const il_reg_t POS_ACT = {
		.idx = 0x6064, .sidx = 0x00, .dtype = IL_REG_DTYPE_S32,
		.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_POS
	};

	const il_reg_t VEL_ACT = {
		.idx = 0x606c, .sidx = 0x00, .dtype = IL_REG_DTYPE_S32,
		.access = IL_REG_ACCESS_RW, .phy = IL_REG_PHY_VEL
	};

	/* create network */
	net = il_net_create(port);
	if (!net) {
		fprintf(stderr, "Could not create network: %s\n", ilerr_last());
		r = 1;
		goto out;
	}

	/* create axis */
	axis = il_axis_create(net, id, IL_AXIS_TIMEOUT_DEF);
	if (!axis) {
		fprintf(stderr, "Could not create axis: %s\n", ilerr_last());
		r = 1;
		goto cleanup_net;
	}

	/* create watcher */
	watcher = il_watcher_create(axis, WATCHER_BASE_PERIOD);
	if (!watcher) {
		fprintf(stderr, "Could not create watcher: %s\n", ilerr_last());
		r = 1;
		goto cleanup_axis;
	}

	/* subscribe to position and velocity changes */
	r = il_watcher_subscribe(watcher, &POS_ACT, POS_WATCH_PERIOD,
				 on_pos_changed, NULL);
	if (r < 0) {
		fprintf(stderr, "Could not subscribe: %s\n", ilerr_last());
		goto cleanup_watcher;
	}

	r = il_watcher_subscribe(watcher, &VEL_ACT, VEL_WATCH_PERIOD,
				 on_vel_changed, NULL);
	if (r < 0) {
		fprintf(stderr, "Could not subscribe: %s\n", ilerr_last());
		goto cleanup_watcher;
	}

	r = il_watcher_start(watcher);
	if (r < 0) {
		fprintf(stderr, "Could not start watcher: %s\n", ilerr_last());
		goto cleanup_watcher;
	}

	printf("Press ENTER to terminate...\n");
	getchar();

	(void)il_watcher_stop(watcher);

cleanup_watcher:
	il_watcher_destroy(watcher);

cleanup_axis:
	il_axis_destroy(axis);

cleanup_net:
	il_net_destroy(net);

out:
	return r;
}

int main(int argc, char **argv)
{
	const char *port;
	uint8_t id;

	if (argc < 3) {
		fprintf(stderr,
			"Usage: watcher PORT AXIS_ID\n");
		return 1;
	}

	port = argv[1];
	id = (uint8_t)strtoul(argv[2], NULL, 0);

	return run(port, id);
}
