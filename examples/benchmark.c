/**
 * @example benchmark.c
 *
 * This example performs a transfer benchmark.
 */

#include <stdio.h>
#include <ingenialink/ingenialink.h>

/** Register address. */
#define ADDRESS	0x0060FF

#ifdef _WIN32
#include <Windows.h>

#define benchmark_init(loops) \
	LARGE_INTEGER frequency; \
	LARGE_INTEGER start, end; \
	QueryPerformanceCounter(&start); \
	for (int i = 0; i < (loops); i++) \

#define benchmark_end(elapsed) \
	QueryPerformanceCounter(&end); \
	QueryPerformanceFrequency(&frequency); \
	elapsed = (end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
#else
#include <sys/time.h>

#define benchmark_init(loops) \
	struct timeval start, end; \
	gettimeofday(&start, NULL); \
	for (int i = 0; i < (loops); i++)

#define benchmark_end(elapsed) \
	gettimeofday(&end, NULL); \
	elapsed = (end.tv_sec - start.tv_sec) * 1000.0; \
	elapsed += (end.tv_usec - start.tv_usec) / 1000.0
#endif

static int run(int loops, const char *port, uint8_t id)
{
	int32_t r = 0;

	il_net_t *net;
	il_servo_t *servo;
	il_reg_t reg;

	double elapsed;

	/* create network */
	net = il_net_create(port);
	if (!net) {
		fprintf(stderr, "Could not create network: %s\n", ilerr_last());
		r = 1;
		goto out;
	}

	/* create servo */
	servo = il_servo_create(net, id, NULL, IL_SERVO_TIMEOUT_DEF);
	if (!servo) {
		fprintf(stderr, "Could not create servo: %s\n", ilerr_last());
		goto cleanup_net;
	}

	/* run benchmark */
	reg.address = ADDRESS;
	reg.dtype = IL_REG_DTYPE_S32;
	reg.access = IL_REG_ACCESS_RO;
	reg.phy = IL_REG_PHY_NONE;

	benchmark_init(loops)
	{
		int32_t buf;
		/*double buf;*/

		r = il_servo_raw_read_s32(servo, &reg, NULL, &buf);
		/*r = il_servo_read(servo, &reg, NULL, &buf);*/
		if (r < 0) {
			fprintf(stderr, "Error while reading: %s\n",
				ilerr_last());
			break;
		}
	}
	benchmark_end(elapsed);

	if (r == 0) {
		printf("%d messages read in %.2f ms (%.2f msgs/s).\n", loops,
		       elapsed, ((double)loops / elapsed) * 1000.0);
	}

	il_servo_destroy(servo);

cleanup_net:
	il_net_destroy(net);

out:
	return (int)r;
}

int main(int argc, char **argv)
{
	int loops;
	const char *port;
	uint8_t id;

	if (argc < 4) {
		fprintf(stderr,
			"Usage: benchmark LOOPS PORT SERVO_ID\n");
		return 1;
	}

	loops = (int)strtoul(argv[1], NULL, 0);
	port = argv[2];
	id = (uint8_t)strtoul(argv[3], NULL, 0);

	return run(loops, port, id);
}
