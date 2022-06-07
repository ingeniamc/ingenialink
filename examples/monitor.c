/**
 * @example monitor.c
 *
 * Simple monitor example which uses the monitor to capture the velocity curve
 * when the motion is started.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>
#include "external/log.c/src/log.h"

/** Enable timeout. */
#define ENABLE_TIMEOUT	2000

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
	il_net_opts_t opts;
	il_servo_t *servo;
	il_monitor_t *monitor;

	il_monitor_acq_t *acq;
	size_t i;
	FILE *log_f;

	const il_reg_t IL_REG_VEL_ACT = {
		.address = 0x00606C,
		.dtype = IL_REG_DTYPE_S32,
		.access = IL_REG_ACCESS_RW,
		.phy = IL_REG_PHY_VEL,
		.range = {
			.min.s32 = INT32_MIN,
			.max.s32 = INT32_MAX
		},
		.labels = NULL
	};

	/* create network */
	opts.port = port;
	opts.timeout_rd = IL_NET_TIMEOUT_RD_DEF;
	opts.timeout_wr = IL_NET_TIMEOUT_WR_DEF;

	net = il_net_create(IL_NET_PROT_EUSB, &opts);
	if (!net) {
		log_error("Could not create network: %s", ilerr_last());
		r = 1;
		goto out;
	}

	/* create servo */
	servo = il_servo_create(net, id, NULL);
	if (!servo) {
		log_error("Could not create servo: %s", ilerr_last());
		r = 1;
		goto cleanup_net;
	}


	il_servo_units_vel_set(servo, IL_UNITS_VEL_RPS);

	/* create monitor, configure to sample velocity @1ms after reaching a
	 * 90 % of the target */
	monitor = il_monitor_create(servo);
	if (!monitor) {
		log_error("Could not create monitor: %s", ilerr_last());
		r = 1;
		goto cleanup_servo;
	}

	r = il_monitor_configure(monitor, T_S, 0, MAX_SAMPLES);
	if (r < 0) {
		log_error("Could not configure monitor: %s",
			ilerr_last());
		goto cleanup_monitor;
	}

	r = il_monitor_ch_configure(monitor, 0, &IL_REG_VEL_ACT, NULL);
	if (r < 0) {
		log_error("Could not configure channel: %s",
			ilerr_last());
		goto cleanup_monitor;
	}

	r = il_monitor_trigger_configure(monitor, IL_MONITOR_TRIGGER_POS,
					 0, &IL_REG_VEL_ACT, NULL,
					 TARGET_VEL * 0.9, 0, 0);
	if (r < 0) {
		log_error("Could not configure trigger: %s",
			ilerr_last());
		goto cleanup_monitor;
	}

	/* enable servo in PV mode */
	r = il_servo_disable(servo);
	if (r < 0) {
		log_error("Could not disable servo: %s", ilerr_last());
		goto cleanup_monitor;
	}

	r = il_servo_mode_set(servo, IL_SERVO_MODE_PV);
	if (r < 0) {
		log_error("Could not set mode: %s", ilerr_last());
		goto cleanup_monitor;
	}

	r = il_servo_enable(servo, ENABLE_TIMEOUT);
	if (r < 0) {
		log_error("Could not enable servo: %s", ilerr_last());
		goto cleanup_monitor;
	}

	/* enable monitor, set velocity */
	r = il_monitor_start(monitor);
	if (r < 0) {
		log_error("Could not start monitor: %s", ilerr_last());
		goto servo_disable;
	}

	r = il_servo_velocity_set(servo, TARGET_VEL);
	if (r < 0) {
		log_error("Could not set velocity: %s", ilerr_last());
		goto servo_disable;
	}

	/* wait for monitor to capture all samples, then store results */
	log_info("Waiting for monitor to complete...\n");
	r = il_monitor_wait(monitor, MONITOR_TIMEOUT);
	if (r < 0) {
		log_error("Monitor acquisition failed: %s",
			ilerr_last());
		goto servo_disable;
	}

	il_monitor_data_get(monitor, &acq);

	if (acq->sz != acq->cnt)
		log_error("WARNING: Acquisition did not complete!");

	log_info("Writing samples (%zu) to file...", acq->cnt);
	log_f = fopen(log_fname, "w");
	if (!log_f) {
		log_error("Could not open log file");
		goto servo_disable;
	}

	for (i = 0; i < acq->cnt; i++)
		fprintf(log_f, "%f, %f\n",
			acq->t[i], acq->d[0][i]);

	fclose(log_f);

servo_disable:
	(void)il_servo_disable(servo);

cleanup_monitor:
	il_monitor_destroy(monitor);

cleanup_servo:
	il_servo_destroy(servo);

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
		log_error("Usage: monitor PORT SERVO_ID LOG_FILE");
		return 1;
	}

	port = argv[1];
	id = (uint8_t)strtoul(argv[2], NULL, 0);
	log_fname = argv[3];

	return run(port, id, log_fname);
}
