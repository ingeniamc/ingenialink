/**
 * @example dict.c
 *
 * This example shows how to use a dictionary.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>
#include "external/log.c/src/log.h"

int main(int argc, const char **argv)
{
	int r = 0;
	il_net_t *net;
	il_servo_t *servo;
	il_dict_t *dict;

	if (argc < 3) {
		log_error("Usage: ./dict DICTIONARY.xml OUTPUT.xml");
		return -1;
	}

	log_info("Looking for servos...");
	r = il_servo_lucky(IL_NET_PROT_EUSB, &net, &servo, argv[1]);
	if (r < 0) {
		log_error("%s", ilerr_last());
		return r;
	}

	log_info("Reading servo registers...");
	r = il_servo_dict_storage_read(servo);
	if (r < 0) {
		log_error("%s", ilerr_last());
		goto cleanup_net_servo;
	}

	log_info("Storing...");
	dict = il_servo_dict_get(servo);
	r = il_dict_save(dict, argv[2]);
	if (r < 0)
		log_error("%s", ilerr_last());

cleanup_net_servo:
	il_servo_destroy(servo);
	il_net_destroy(net);

	return r;
}
