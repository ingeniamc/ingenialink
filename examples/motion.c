/**
 * @example motion.c
 *
 * Simple motion example (Homing + PP). You can use the motion_plot.py script
 * to plot the resulting CSV.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>
#include "external/log.c/src/log.h"

/** Enable timeout. */
#define ENABLE_TIMEOUT  2000

/** Homing timeout. */
#define HOMING_TIMEOUT	15000

/** Position reach timeout (ms). */
#define POS_TIMEOUT	5000

/** Sampling period (ms). */
#define T_S		5

/** Poller size. */
#define POLLER_SZ	2000

void on_emcy(void *ctx, uint32_t code)
{
	(void)ctx;

	log_info("Emergency occurred (0x%04x)", code);
}

static int run(const char *log_fname)
{
	int r = 0;

	size_t i;

	il_net_t *net;
	il_servo_t *servo;

	il_poller_t *poller;
	il_poller_acq_t *acq;
	FILE *log_f;

	const il_reg_t IL_REG_POS_ACT = {
		.address = 0x006064,
		.dtype = IL_REG_DTYPE_S32,
		.access = IL_REG_ACCESS_RW,
		.phy = IL_REG_PHY_VEL,
		.range = {
			.min.s32 = INT32_MIN,
			.max.s32 = INT32_MAX
		},
		.labels = NULL
	};

	const il_reg_t IL_REG_VEL_ACT = {
		.address = 0x00606c,
		.dtype = IL_REG_DTYPE_S32,
		.access = IL_REG_ACCESS_RW,
		.phy = IL_REG_PHY_VEL,
		.range = {
			.min.s32 = INT32_MIN,
			.max.s32 = INT32_MAX
		},
		.labels = NULL
	};

	r = il_servo_lucky(IL_NET_PROT_EUSB, &net, &servo, NULL);
	if (r < 0) {
		log_error("%s", ilerr_last());
		return r;
	}

	r = il_servo_emcy_subscribe(servo, on_emcy, NULL);
	if (r < 0) {
		log_error("Could not subscribe to emergencies: %s",
			ilerr_last());
		goto cleanup_net_servo;
	}

	il_servo_units_pos_set(servo, IL_UNITS_POS_DEG);
	il_servo_units_vel_set(servo, IL_UNITS_VEL_DEG_S);

	/* create poller */
	poller = il_poller_create(servo, 2);
	if (!poller) {
		log_error("Could not create poller: %s", ilerr_last());
		r = 1;
		goto cleanup_net_servo;
	}

	r = il_poller_configure(poller, T_S, POLLER_SZ);
	if (r < 0) {
		log_error("Could not configure poller: %s",
			ilerr_last());
		goto cleanup_poller;
	}

	r = il_poller_ch_configure(poller, 0, &IL_REG_POS_ACT, NULL);
	if (r < 0) {
		log_error("Could not configure poller channel: %s",
			ilerr_last());
		goto cleanup_poller;
	}

	r = il_poller_ch_configure(poller, 1, &IL_REG_VEL_ACT, NULL);
	if (r < 0) {
		log_error("Could not configure poller channel: %s",
			ilerr_last());
		goto cleanup_poller;
	}

	/* disable */
	r = il_servo_disable(servo);
	if (r < 0) {
		log_error("Could not disable servo: %s", ilerr_last());
		goto cleanup_poller;
	}

	/* perform homing */
	r = il_servo_mode_set(servo, IL_SERVO_MODE_HOMING);
	if (r < 0) {
		log_error("Could not set mode: %s", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_enable(servo, ENABLE_TIMEOUT);
	if (r < 0) {
		log_error("Could not enable servo: %s", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_homing_start(servo);
	if (r < 0) {
		log_error("Could not start homing: %s", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_homing_wait(servo, HOMING_TIMEOUT);
	if (r < 0) {
		log_error("Homing did not succeed: %s", ilerr_last());
		goto cleanup_poller;
	}

	/* perform PP movements */
	r = il_servo_disable(servo);
	if (r < 0) {
		log_error("Could not disable servo: %s", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_mode_set(servo, IL_SERVO_MODE_PP);
	if (r < 0) {
		log_error("Could not set mode: %s", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_enable(servo, ENABLE_TIMEOUT);
	if (r < 0) {
		log_error("Could not enable servo: %s", ilerr_last());
		goto cleanup_poller;
	}

	r = il_poller_start(poller);
	if (r < 0) {
		log_error("Could not start poller: %s", ilerr_last());
		goto cleanup_poller;
	}

	for (i = 1; i < 5; i++) {
		r = il_servo_position_set(servo, 90 * i, 0, 0,
					  IL_SERVO_SP_TIMEOUT_DEF);
		if (r < 0) {
			flog_error("Could not set position: %s",
				ilerr_last());
			goto cleanup_poller;
		}
	}

	r = il_servo_wait_reached(servo, POS_TIMEOUT);
	if (r < 0) {
		log_error("Could not reach target: %s", ilerr_last());
	}

	(void)il_servo_disable(servo);
	(void)il_poller_stop(poller);

	/* obtain poller results and log to CSV file. */
	il_poller_data_get(poller, &acq);

	if (acq->lost)
		log_error("Warning: poller data was lost");

	log_f = fopen(log_fname, "w");
	if (!log_f) {
		log_error("Could not open log file");
		goto cleanup_poller;
	}

	for (i = 0; i < acq->cnt; i++)
		fprintf(log_f, "%f, %f, %f\n",
			acq->t[i], acq->d[0][i], acq->d[1][i]);

	fclose(log_f);

cleanup_poller:
	il_poller_destroy(poller);

cleanup_net_servo:
	il_servo_destroy(servo);

	il_net_destroy(net);

	return r;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		log_error("Usage: motion LOG_FILE");
		return 1;
	}

	return run(argv[1]);
}
