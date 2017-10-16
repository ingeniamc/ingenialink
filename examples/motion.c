/**
 * @example motion.c
 *
 * Simple motion example (Homing + PP). You can use the motion_plot.py script
 * to plot the resulting CSV.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

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

	printf("Emergency occurred (0x%04x)\n", code);
}

static int run(const char *port, uint8_t id, const char *log_fname)
{
	int r = 0;

	size_t i;

	il_net_t *net;
	il_servo_t *servo;

	il_poller_t *poller;
	il_poller_acq_t *acq;
	FILE *log_f;

	/* create network */
	net = il_net_create(port);
	if (!net) {
		fprintf(stderr, "Could not create network: %s\n", ilerr_last());
		r = 1;
		goto out;
	}

	/* create servo */
	servo = il_servo_create(net, id, IL_SERVO_TIMEOUT_DEF);
	if (!servo) {
		fprintf(stderr, "Could not create servo: %s\n", ilerr_last());
		r = 1;
		goto cleanup_net;
	}

	r = il_servo_emcy_subscribe(servo, on_emcy, NULL);
	if (r < 0) {
		fprintf(stderr, "Could not subscribe to emergencies: %s\n",
			ilerr_last());
		goto cleanup_servo;
	}

	il_servo_units_pos_set(servo, IL_UNITS_POS_DEG);
	il_servo_units_vel_set(servo, IL_UNITS_VEL_DEG_S);

	/* create poller */
	poller = il_poller_create(servo, 2);
	if (!poller) {
		fprintf(stderr, "Could not create poller: %s\n", ilerr_last());
		r = 1;
		goto cleanup_servo;
	}

	r = il_poller_configure(poller, T_S, POLLER_SZ);
	if (r < 0) {
		fprintf(stderr, "Could not configure poller: %s\n",
			ilerr_last());
		goto cleanup_poller;
	}

	r = il_poller_ch_configure(poller, 0, &IL_REG_POS_ACT);
	if (r < 0) {
		fprintf(stderr, "Could not configure poller channel: %s\n",
			ilerr_last());
		goto cleanup_poller;
	}

	r = il_poller_ch_configure(poller, 1, &IL_REG_VEL_ACT);
	if (r < 0) {
		fprintf(stderr, "Could not configure poller channel: %s\n",
			ilerr_last());
		goto cleanup_poller;
	}

	/* reset faults, disable */
	il_servo_fault_reset(servo);
	if (r < 0) {
		fprintf(stderr, "Could not reset fault: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_disable(servo);
	if (r < 0) {
		fprintf(stderr, "Could not disable servo: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	/* perform homing */
	r = il_servo_mode_set(servo, IL_SERVO_MODE_HOMING);
	if (r < 0) {
		fprintf(stderr, "Could not set mode: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_enable(servo, ENABLE_TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Could not enable servo: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_homing_start(servo);
	if (r < 0) {
		fprintf(stderr, "Could not start homing: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_homing_wait(servo, HOMING_TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Homing did not succeed: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	/* perform PP movements */
	r = il_servo_disable(servo);
	if (r < 0) {
		fprintf(stderr, "Could not disable servo: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_mode_set(servo, IL_SERVO_MODE_PP);
	if (r < 0) {
		fprintf(stderr, "Could not set mode: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_servo_enable(servo, ENABLE_TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Could not enable servo: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	r = il_poller_start(poller);
	if (r < 0) {
		fprintf(stderr, "Could not start poller: %s\n", ilerr_last());
		goto cleanup_poller;
	}

	for (i = 1; i < 5; i++) {
		r = il_servo_position_set(servo, 90 * i, 0, 0);
		if (r < 0) {
			fprintf(stderr, "Could not set position: %s\n",
				ilerr_last());
			goto cleanup_poller;
		}

		r = il_servo_position_wait_ack(servo, 1000);
		if (r < 0) {
			fprintf(stderr, "Position not acknowledged: %s\n",
				ilerr_last());
			goto cleanup_poller;
		}
	}

	r = il_servo_wait_reached(servo, POS_TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Could not reach target: %s\n", ilerr_last());
	}

	(void)il_servo_disable(servo);
	(void)il_poller_stop(poller);

	/* obtain poller results and log to CSV file. */
	il_poller_data_get(poller, &acq);

	if (acq->lost)
		fprintf(stderr, "Warning: poller data was lost\n");

	log_f = fopen(log_fname, "w");
	if (!log_f) {
		fprintf(stderr, "Could not open log file\n");
		goto cleanup_poller;
	}

	for (i = 0; i < acq->cnt; i++)
		fprintf(log_f, "%f, %f, %f\n",
			acq->t[i], acq->d[0][i], acq->d[1][i]);

	fclose(log_f);

cleanup_poller:
	il_poller_destroy(poller);

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
		fprintf(stderr,
			"Usage: motion PORT SERVO_ID LOG_FILE\n");
		return 1;
	}

	port = argv[1];
	id = (uint8_t)strtoul(argv[2], NULL, 0);
	log_fname = argv[3];

	return run(port, id, log_fname);
}
