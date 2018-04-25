/**
 * @example dict.c
 *
 * This example shows how to use a dictionary.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

int main(int argc, const char **argv)
{
	int r = 0;
	il_net_t *net;
	il_servo_t *servo;
	il_dict_t *dict;

	if (argc < 3) {
		fprintf(stderr, "Usage: ./dict DICTIONARY.xml OUTPUT.xml\n");
		return -1;
	}

	printf("Looking for servos...\n");
	r = il_servo_lucky(IL_NET_PROT_EUSB, &net, &servo, argv[1]);
	if (r < 0) {
		fprintf(stderr, "%s\n", ilerr_last());
		return r;
	}

	printf("Reading servo registers...\n");
	r = il_servo_dict_storage_read(servo);
	if (r < 0) {
		fprintf(stderr, "%s\n", ilerr_last());
		goto cleanup_net_servo;
	}

	printf("Storing...\n");
	dict = il_servo_dict_get(servo);
	r = il_dict_save(dict, argv[2]);
	if (r < 0)
		fprintf(stderr, "%s\n", ilerr_last());

cleanup_net_servo:
	il_servo_destroy(servo);
	il_net_destroy(net);

	return r;
}
