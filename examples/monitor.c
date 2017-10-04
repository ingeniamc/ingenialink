/**
 * @example monitor.c
 *
 * Simple monitor example which uses the monitor to capture the velocity curve
 * when the motion is started.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

/** Sampling period (us). */
#define T_S		1000

/** Maximum number of samples. */
#define MAX_SAMPLES	200

/** Target velocity (rps). */
#define TARGET_VEL	20

/** Monitor acquisition timeout. */
#define MONITOR_TIMEOUT	10000

static int run(const char *port, uint8_t id, const char *log_fname)
{
	int r = 0;

	il_net_t *net;
	il_axis_t *axis;
	il_monitor_t *monitor;

	const il_reg_t IL_REG_VEL_ACT = {
		0x606c, 0x00, IL_REG_DTYPE_S32, IL_REG_ACCESS_RW, IL_REG_PHY_VEL
	};

	il_monitor_acq_t *acq;
	size_t i;
	double t;
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


	il_axis_units_vel_set(axis, IL_UNITS_VEL_RPS);

	/* create monitor, configure to sample velocity @1ms after reaching a
	 * 90 % of the target */
	monitor = il_monitor_create(axis);
	if (!monitor) {
		fprintf(stderr, "Could not create monitor: %s\n", ilerr_last());
		r = 1;
		goto cleanup_axis;
	}

	r = il_monitor_configure(monitor, T_S, 0, MAX_SAMPLES);
	if (r < 0) {
		fprintf(stderr, "Could not configure monitor: %s\n",
			ilerr_last());
		goto cleanup_monitor;
	}

	r = il_monitor_ch_disable_all(monitor);
	if (r < 0) {
		fprintf(stderr, "Could not disable all channels: %s\n",
			ilerr_last());
		goto cleanup_monitor;
	}

	r = il_monitor_ch_configure(monitor, IL_MONITOR_CH_1, &IL_REG_VEL_ACT);
	if (r < 0) {
		fprintf(stderr, "Could not configure channel: %s\n",
			ilerr_last());
		goto cleanup_monitor;
	}

	r = il_monitor_trigger_configure(monitor, IL_MONITOR_TRIGGER_POS,
					 0, &IL_REG_VEL_ACT, TARGET_VEL * 0.9,
					 0, 0);
	if (r < 0) {
		fprintf(stderr, "Could not configure trigger: %s\n",
			ilerr_last());
		goto cleanup_monitor;
	}

	/* enable axis in PV mode */
	r = il_axis_disable(axis);
	if (r < 0) {
		fprintf(stderr, "Could not disable axis: %s\n", ilerr_last());
		goto cleanup_monitor;
	}

	r = il_axis_mode_set(axis, IL_AXIS_MODE_PV);
	if (r < 0) {
		fprintf(stderr, "Could not set mode: %s\n", ilerr_last());
		goto cleanup_monitor;
	}

	r = il_axis_enable(axis, IL_AXIS_PDS_TIMEOUT_DEF);
	if (r < 0) {
		fprintf(stderr, "Could not enable axis: %s\n", ilerr_last());
		goto cleanup_monitor;
	}

	/* enable monitor, set velocity */
	r = il_monitor_start(monitor);
	if (r < 0) {
		fprintf(stderr, "Could not start monitor: %s\n", ilerr_last());
		goto axis_disable;
	}

	r = il_axis_velocity_set(axis, TARGET_VEL);
	if (r < 0) {
		fprintf(stderr, "Could not set velocity: %s\n", ilerr_last());
		goto axis_disable;
	}

	/* wait for monitor to capture all samples, then store results */
	printf("Waiting for monitor to complete...\n");
	r = il_monitor_wait(monitor, MONITOR_TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Monitor acquisition failed: %s\n",
			ilerr_last());
		goto axis_disable;
	}

	il_monitor_data_get(monitor, &acq);

	if (acq->sz != acq->n_samples)
		fprintf(stderr, "WARNING: Acquisition is not complete!\n");

	printf("Writing samples (%zu) to file...\n", acq->n_samples);
	log_f = fopen(log_fname, "w");
	if (!log_f) {
		fprintf(stderr, "Could not open log file");
		goto axis_disable;
	}

	for (i = 0, t = 0.; i < acq->n_samples; i++, t += T_S / 1000000.)
		fprintf(log_f, "%f, %f\n",
			t, acq->samples[IL_MONITOR_CH_1][i]);

	fclose(log_f);

axis_disable:
	(void)il_axis_disable(axis);

cleanup_monitor:
	il_monitor_destroy(monitor);

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
			"Usage: monitor PORT AXIS_ID LOG_FILE\n");
		return 1;
	}

	port = argv[1];
	id = (uint8_t)strtoul(argv[2], NULL, 0);
	log_fname = argv[3];

	return run(port, id, log_fname);
}
