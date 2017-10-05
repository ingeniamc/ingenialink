/**
 * @example motion.c
 *
 * Simple motion example (Homing + PP). You can use the motion_plot.py script
 * to plot the resulting CSV.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

/** Homing timeout. */
#define HOMING_TIMEOUT	15000

/** Position reach timeout (ms). */
#define POS_TIMEOUT	5000

void on_emcy(void *ctx, uint32_t code)
{
	(void)ctx;

	printf("Emergency occurred (0x%04x)\n", code);
}

static int run(const char *port, uint8_t id, const char *log_fname)
{
	int r = 0;

	size_t i;

	il_net_t *net;
	il_axis_t *axis;
	il_poller_t *poller;

	double *t, *d;
	size_t cnt;
	int lost;

	FILE *log_f;

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


	r = il_axis_emcy_subscribe(axis, on_emcy, NULL);
	if (r < 0) {
		fprintf(stderr, "Could not subscribe to emergencies: %s\n",
			ilerr_last());
		goto cleanup_axis;
	}

	il_axis_units_pos_set(axis, IL_UNITS_POS_DEG);

	/* create poller */
	poller = il_poller_create(axis, &IL_REG_POS_ACT, 2, 2000);
	if (!poller) {
		fprintf(stderr, "Could not create poller: %s\n", ilerr_last());
		r = 1;
		goto cleanup_axis;
	}

	/* reset faults, disable */
	il_axis_fault_reset(axis);
	if (r < 0) {
		fprintf(stderr, "Could not reset fault: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_axis_disable(axis);
	if (r < 0) {
		fprintf(stderr, "Could not disable axis: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	/* perform homing */
	r = il_axis_mode_set(axis, IL_AXIS_MODE_HOMING);
	if (r < 0) {
		fprintf(stderr, "Could not set mode: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_axis_enable(axis, IL_AXIS_PDS_TIMEOUT_DEF);
	if (r < 0) {
		fprintf(stderr, "Could not enable axis: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_axis_homing_start(axis);
	if (r < 0) {
		fprintf(stderr, "Could not start homing: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_axis_homing_wait(axis, HOMING_TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Homing did not succeed: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	/* perform PP movements */
	r = il_axis_disable(axis);
	if (r < 0) {
		fprintf(stderr, "Could not disable axis: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_axis_mode_set(axis, IL_AXIS_MODE_PP);
	if (r < 0) {
		fprintf(stderr, "Could not set mode: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_axis_enable(axis, IL_AXIS_PDS_TIMEOUT_DEF);
	if (r < 0) {
		fprintf(stderr, "Could not enable axis: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_poller_start(poller);
	if (r < 0) {
		fprintf(stderr, "Could not start poller: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	for (i = 1; i < 5; i++) {
		r = il_axis_position_set(axis, 90 * i, 0, 0);
		if (r < 0) {
			fprintf(stderr, "Could not set position: %s\n",
				ilerr_last());
			goto cleanup_poller;
		}
	}

	r = il_axis_wait_reached(axis, POS_TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Could not reach target: %s\n", ilerr_last());
	}

	sleep(5);

	(void)il_axis_disable(axis);
	(void)il_poller_stop(poller);

	/* obtain poller results and log to CSV file. */
	il_poller_data_get(poller, &t, &d, &cnt, &lost);

	if (lost)
		fprintf(stderr, "Warning: poller data was lost\n");

	log_f = fopen(log_fname, "w");
	if (!log_f) {
		fprintf(stderr, "Could not open log file\n");
		goto cleanup_poller;
	}

	for (i = 0; i < cnt; i++)
		fprintf(log_f, "%f, %f\n", t[i], d[i]);

	fclose(log_f);

cleanup_poller:
	il_poller_destroy(poller);

cleanup_axis:
	il_axis_destroy(axis);

cleanup_net:
	il_net_destroy(net);

out:
	return r;
}

int main(int argc, char **argv)
{
	const char *port, *log_fname;
	uint8_t id;

	if (argc < 4) {
		fprintf(stderr,
			"Usage: motion PORT AXIS_ID LOG_FILE\n");
		return 1;
	}

	port = argv[1];
	id = (uint8_t)strtoul(argv[2], NULL, 0);
	log_fname = argv[3];

	return run(port, id, log_fname);
}
