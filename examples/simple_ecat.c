#include <ingenialink/ingenialink.h>
#include <ingenialink/registers.h>
#include <time.h>
#include <stdio.h>


int main(int argc, const char *argv[])
{
	il_net_t *net = NULL;
	il_servo_t *servo = NULL;
	char* ifname = "\\Device\\NPF_{XXX}";
	char* dict_path = "XXX";
	int port = 1061;
	int slave = 1;
	int use_eoe = 1;
	int r;
	r = il_servo_connect_ecat(IL_NET_PROT_ECAT, ifname, &net, &servo, dict_path, port, slave, use_eoe);

	if (r < 0) {
		fprintf(stderr, "CONNECTION FAIL %d\n", r);
		return -1;
	}

	int32_t actual_position_buf;
    const il_reg_t IL_REG_POS_ACT = {
		.subnode = 1,
        .address = 0x0030,
        .dtype = IL_REG_DTYPE_S32,
        .access = IL_REG_ACCESS_RO,
        .phy = IL_REG_PHY_NONE,
        .range = {
            .min.s32 = INT32_MIN,
            .max.s32 = INT32_MAX
        }
    };


	float bus_voltage_value_buf;
    const il_reg_t IL_REG_BUS_VOLT = {
		.subnode = 1,
        .address = 0x0060,
        .dtype = IL_REG_DTYPE_FLOAT,
        .access = IL_REG_ACCESS_RO,
        .phy = IL_REG_PHY_NONE,
        .range = {
            .min.s32 = -100000000,
            .max.s32 = 100000000
        }
    };

	int n_iter = 1000;
	time_t init_t;
	init_t = time(0);

	for (int i = 0; i<n_iter; i++) {
		int r;
		r = il_servo_raw_read_s32(servo, &IL_REG_POS_ACT, NULL, &actual_position_buf);
		if (r < 0) {
			fprintf(stderr, "ACTUAL POSITION READ ERROR: %d\n", r);
		}
		r = il_servo_raw_read_float(servo, &IL_REG_BUS_VOLT, NULL, &bus_voltage_value_buf);
		if (r < 0) {
			fprintf(stderr, "BUS VOLTAGE READ ERROR: %d\n", r);
		}

		time_t now;
		now = time(0);
		unsigned diff_t = (unsigned)now - (unsigned)init_t;
		printf("%d -- ", diff_t);
		printf("Actual position value: %d -- ", actual_position_buf);
		printf("Bus voltage value: %f -- ", bus_voltage_value_buf);
		printf("%d\n", i);
	}

	return 0;
}
