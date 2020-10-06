/*
 * MIT License
 *
 * Copyright (c) 2017-2018 Ingenia-CAT S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "servo.h"

#include "ingenialink/err.h"

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <windows.h>
#include <inttypes.h>
/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_servo__retain(il_servo_t *servo)
{
	servo->ops->_retain(servo);
}

void il_servo__release(il_servo_t *servo)
{
	servo->ops->_release(servo);
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_servo_t *il_servo_create(il_net_t *net, uint16_t id, const char *dict)
{
	switch (il_net_prot_get(net)) {
#ifdef IL_HAS_PROT_EUSB
	case IL_NET_PROT_EUSB:
		return il_eusb_servo_ops.create(net, id, dict);
#endif
#ifdef IL_HAS_PROT_MCB
	case IL_NET_PROT_MCB:
		return il_mcb_servo_ops.create(net, id, dict);
#endif

#ifdef IL_HAS_PROT_ETH
	case IL_NET_PROT_ETH:
		return il_eth_servo_ops.create(net, id, dict);
#endif

#ifdef IL_HAS_PROT_ECAT
	case IL_NET_PROT_ECAT:
		return il_ecat_servo_ops.create(net, id, dict);
#endif


#ifdef IL_HAS_PROT_VIRTUAL
	case IL_NET_PROT_VIRTUAL:
		return il_virtual_servo_ops.create(net, id, dict);
#endif
	default:
		ilerr__set("Unsupported network protocol");
		return NULL;
	}
}

void il_servo_destroy(il_servo_t *servo)
{
	servo->ops->destroy(servo);
}

int il_servo_reset(il_servo_t *servo)
{
	return servo->ops->reset(servo);
}

void il_servo_state_get(il_servo_t *servo, il_servo_state_t *state, int *flags, uint8_t subnode)
{
	servo->ops->state_get(servo, state, flags, subnode);
}

int il_servo_state_subscribe(il_servo_t *servo,
			     il_servo_state_subscriber_cb_t cb, void *ctx)
{
	return servo->ops->state_subscribe(servo, cb, ctx);
}

void il_servo_state_unsubscribe(il_servo_t *servo, int slot)
{
	servo->ops->state_unsubscribe(servo, slot);
}

int il_servo_emcy_subscribe(il_servo_t *servo, il_servo_emcy_subscriber_cb_t cb,
			    void *ctx)
{
	return servo->ops->emcy_subscribe(servo, cb, ctx);
}

void il_servo_emcy_unsubscribe(il_servo_t *servo, int slot)
{
	servo->ops->emcy_unsubscribe(servo, slot);
}

il_dict_t *il_servo_dict_get(il_servo_t *servo)
{
	return servo->dict;
}

int il_servo_dict_load(il_servo_t *servo, const char *dict)
{
	if (servo->dict) {
		il_dict_destroy(servo->dict);
		servo->dict = NULL;
	}

	servo->dict = il_dict_create(dict);
	if (!servo->dict)
		return IL_EFAIL;

	return 0;
}

int il_servo_dict_storage_read(il_servo_t *servo)
{
	int r = 0;
	const char **ids;

	if (!servo->dict) {
		ilerr__set("No dictionary loaded");
		return IL_EFAIL;
	}

	// Subnodes = axis available at servo + 1 subnode of general parameters
	int subnodes = servo->subnodes + 1;
	for (int j = 0; j < subnodes; j++) {
		uint16_t actual_crc_value = 0x0000;
		ids = il_dict_reg_ids_get(servo->dict, j);
		if (!ids)
			return IL_EFAIL;
			
		for (size_t i = 0; i < ids[i]; i++) {
			const il_reg_t *reg;
			il_reg_value_t storage;
			(void)il_dict_reg_get(servo->dict, ids[i], &reg, j);

			if (reg->access != IL_REG_ACCESS_RW)
				continue;

			switch (reg->dtype) {
			case IL_REG_DTYPE_U8:
				r = il_servo_raw_read_u8(servo, reg, ids[i],
							&storage.u8);
				break;
			case IL_REG_DTYPE_S8:
				r = il_servo_raw_read_s8(servo, reg, ids[i],
							&storage.s8);
				break;
			case IL_REG_DTYPE_U16:
				r = il_servo_raw_read_u16(servo, reg, ids[i],
							&storage.u16);
				break;
			case IL_REG_DTYPE_S16:
				r = il_servo_raw_read_s16(servo, reg, ids[i],
							&storage.s16);
				break;
			case IL_REG_DTYPE_U32:
				r = il_servo_raw_read_u32(servo, reg, ids[i],
							&storage.u32);
				break;
			case IL_REG_DTYPE_STR:
				r = il_servo_raw_read_str(servo, reg, ids[i],
							&storage.u32);
				break;
			case IL_REG_DTYPE_S32:
				r = il_servo_raw_read_s32(servo, reg, ids[i],
							&storage.s32);
				break;
			case IL_REG_DTYPE_U64:
				r = il_servo_raw_read_u64(servo, reg, ids[i],
							&storage.u64);
				break;
			case IL_REG_DTYPE_S64:
				r = il_servo_raw_read_s64(servo, reg, ids[i],
							&storage.s64);
				break;
			case IL_REG_DTYPE_FLOAT:
				r = il_servo_raw_read_float(servo, reg, ids[i],
								&storage.flt);
				break;
			default:
				continue;
			}

			if (r < 0)
				continue;

			(void)il_dict_reg_storage_update(servo->dict, ids[i], storage, j);	
		}
	}
	printf("END\n");
	
cleanup_ids:
	il_dict_reg_ids_destroy(ids);

	return r;
}

int il_servo_dict_storage_write(il_servo_t *servo)
{
	int r = 0;
	const char **ids;

	if (!servo->dict) {
		ilerr__set("No dictionary loaded");
		return IL_EFAIL;
	}

	// Subnodes = axis available at servo + 1 subnode of general parameters
	int subnodes = servo->subnodes + 1;
	for (int j = 0; j < subnodes; j++) {
		ids = il_dict_reg_ids_get(servo->dict, j);
		if (!ids)
			return IL_EFAIL;

		uint16_t actual_crc_value = 0x0000;

		for (size_t i = 0; ids[i]; i++) {
			const il_reg_t *reg;

			(void)il_dict_reg_get(servo->dict, ids[i], &reg, j);
			
			if (reg->access != IL_REG_ACCESS_RW)
				continue;

			switch (reg->dtype) {
			case IL_REG_DTYPE_U8:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.u8, j);
				}			
				r = il_servo_raw_write_u8(servo, reg, ids[i],
							reg->storage.u8, 1, 0);
				break;
			case IL_REG_DTYPE_S8:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.s8, j);
				}
				r = il_servo_raw_write_s8(servo, reg, ids[i],
							reg->storage.s8, 1, 0);
				break;
			case IL_REG_DTYPE_U16:
				if ((strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0) && 
					(strcmp(ids[i], "DRV_CRC_COCO_IN") != 0))
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.u16, j);
				}
				r = il_servo_raw_write_u16(servo, reg, ids[i],
							reg->storage.u16, 1, 0);
				break;
			case IL_REG_DTYPE_S16:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.s16, j);
				}
				r = il_servo_raw_write_s16(servo, reg, ids[i],
							reg->storage.s16, 1, 0);
				break;
			case IL_REG_DTYPE_U32:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.u32, j);
				}
				r = il_servo_raw_write_u32(servo, reg, ids[i],
							reg->storage.u32, 1, 0);
				break;
			case IL_REG_DTYPE_S32:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.s32, j);
				}
				r = il_servo_raw_write_s32(servo, reg, ids[i],
							reg->storage.s32, 1, 0);
				break;
			case IL_REG_DTYPE_U64:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.u64, j);
				}
				r = il_servo_raw_write_u64(servo, reg, ids[i],
							reg->storage.u64, 1, 0);
				break;
			case IL_REG_DTYPE_S64:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage.s64, j);
				}
				r = il_servo_raw_write_s64(servo, reg, ids[i],
							reg->storage.s64, 1, 0);
				break;
			case IL_REG_DTYPE_FLOAT:
				if (strcmp(reg->address_type, "NVM_CFG") == 0 || strcmp(reg->address_type, "NVM") == 0)
				{
					actual_crc_value = il_dict_crc_update(servo->dict, ids[i], reg->storage, j);
				}
				r = il_servo_raw_write_float(servo, reg, ids[i], reg->storage.flt, 1, 0);
				break;
			default:
				continue;
			}
				
			if (r < 0)
				continue;

		}
		(void)il_dict_set_crc_input(servo->dict, j, actual_crc_value);
	}

cleanup_ids:
	il_dict_reg_ids_destroy(ids);

	return r;
}

int il_servo_name_get(il_servo_t *servo, char *name, size_t sz)
{
	return servo->ops->name_get(servo, name, sz);
}

int il_servo_name_set(il_servo_t *servo, const char *name)
{
	return servo->ops->name_set(servo, name);
}

int il_servo_info_get(il_servo_t *servo, il_servo_info_t *info)
{
	return servo->ops->info_get(servo, info);
}

int il_servo_store_all(il_servo_t *servo)
{
	return servo->ops->store_all(servo);
}

int il_servo_store_comm(il_servo_t *servo)
{
	return servo->ops->store_comm(servo);
}

int il_servo_store_app(il_servo_t *servo)
{
	return servo->ops->store_app(servo);
}

int il_servo_units_update(il_servo_t *servo)
{
	return servo->ops->units_update(servo);
}

double il_servo_units_factor(il_servo_t *servo, const il_reg_t *reg)
{
	return servo->ops->units_factor(servo, reg);
}

il_units_torque_t il_servo_units_torque_get(il_servo_t *servo)
{
	return servo->ops->units_torque_get(servo);
}

void il_servo_units_torque_set(il_servo_t *servo, il_units_torque_t units)
{
	servo->ops->units_torque_set(servo, units);
}

il_units_pos_t il_servo_units_pos_get(il_servo_t *servo)
{
	return servo->ops->units_pos_get(servo);
}

void il_servo_units_pos_set(il_servo_t *servo, il_units_pos_t units)
{
	servo->ops->units_pos_set(servo, units);
}

il_units_vel_t il_servo_units_vel_get(il_servo_t *servo)
{
	return servo->ops->units_vel_get(servo);
}

void il_servo_units_vel_set(il_servo_t *servo, il_units_vel_t units)
{
	servo->ops->units_vel_set(servo, units);
}

il_units_acc_t il_servo_units_acc_get(il_servo_t *servo)
{
	return servo->ops->units_acc_get(servo);
}

void il_servo_units_acc_set(il_servo_t *servo, il_units_acc_t units)
{
	servo->ops->units_acc_set(servo, units);
}

int il_servo_raw_read_u8(il_servo_t *servo, const il_reg_t *reg, const char *id,
			 uint8_t *buf)
{
	return servo->ops->raw_read_u8(servo, reg, id, buf);
}

int il_servo_raw_read_s8(il_servo_t *servo, const il_reg_t *reg, const char *id,
			 int8_t *buf)
{
	return servo->ops->raw_read_s8(servo, reg, id, buf);
}

int il_servo_raw_read_u16(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint16_t *buf)
{
	return servo->ops->raw_read_u16(servo, reg, id, buf);
}

int il_servo_raw_read_s16(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int16_t *buf)
{
	return servo->ops->raw_read_s16(servo, reg, id, buf);
}

int il_servo_raw_read_u32(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint32_t *buf)
{
	return servo->ops->raw_read_u32(servo, reg, id, buf);
}

int il_servo_raw_read_str(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint32_t *buf)
{
	return servo->ops->raw_read_str(servo, reg, id, buf);
}

int il_servo_raw_read_s32(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int32_t *buf)
{
	return servo->ops->raw_read_s32(servo, reg, id, buf);
}

int il_servo_raw_read_u64(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint64_t *buf)
{
	return servo->ops->raw_read_u64(servo, reg, id, buf);
}

int il_servo_raw_read_s64(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int64_t *buf)
{
	return servo->ops->raw_read_s64(servo, reg, id, buf);
}

int il_servo_raw_read_float(il_servo_t *servo, const il_reg_t *reg,
			    const char *id, float *buf)
{
	return servo->ops->raw_read_float(servo, reg, id, buf);
}

int il_servo_read(il_servo_t *servo, const il_reg_t *reg, const char *id,
		  double *buf)
{
	return servo->ops->read(servo, reg, id, buf);
}

int il_servo_raw_write_u8(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint8_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_u8(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_s8(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int8_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_s8(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_u16(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, uint16_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_u16(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_s16(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, int16_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_s16(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_u32(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, uint32_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_u32(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_wait_write_u32(il_servo_t *servo, const il_reg_t *reg,
	const char *id, uint32_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_wait_write_u32(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_s32(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, int32_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_s32(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_u64(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, uint64_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_u64(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_s64(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, int64_t val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_s64(servo, reg, id, val, confirm, extended);
}

int il_servo_raw_write_float(il_servo_t *servo, const il_reg_t *reg,
			     const char *id, float val, int confirm, uint16_t extended)
{
	return servo->ops->raw_write_float(servo, reg, id, val, confirm, extended);
}

int il_servo_write(il_servo_t *servo, const il_reg_t *reg, const char *id,
		   double val, int confirm, uint16_t extended)
{
	return servo->ops->write(servo, reg, id, val, confirm, extended);
}

int il_servo_disable(il_servo_t *servo, uint8_t subnode)
{
	return servo->ops->disable(servo, subnode);
}

int il_servo_switch_on(il_servo_t *servo, int timeout)
{
	return servo->ops->switch_on(servo, timeout);
}

int il_servo_enable(il_servo_t *servo, int timeout, uint8_t subnode)
{
	return servo->ops->enable(servo, timeout, subnode);
}

int il_servo_fault_reset(il_servo_t *servo, uint8_t subnode)
{
	return servo->ops->fault_reset(servo, subnode);
}

int il_servo_mode_get(il_servo_t *servo, il_servo_mode_t *mode)
{
	return servo->ops->mode_get(servo, mode);
}

int il_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode)
{
	return servo->ops->mode_set(servo, mode);
}

int il_servo_ol_voltage_get(il_servo_t *servo, double *voltage)
{
	return servo->ops->ol_voltage_get(servo, voltage);
}

int il_servo_ol_voltage_set(il_servo_t *servo, double voltage)
{
	return servo->ops->ol_voltage_set(servo, voltage);
}

int il_servo_ol_frequency_get(il_servo_t *servo, double *freq)
{
	return servo->ops->ol_frequency_get(servo, freq);
}

int il_servo_ol_frequency_set(il_servo_t *servo, double freq)
{
	return servo->ops->ol_frequency_set(servo, freq);
}

int il_servo_homing_start(il_servo_t *servo)
{
	return servo->ops->homing_start(servo);
}

int il_servo_homing_wait(il_servo_t *servo, int timeout)
{
	return servo->ops->homing_wait(servo, timeout);
}

int il_servo_torque_get(il_servo_t *servo, double *torque)
{
	return servo->ops->torque_get(servo, torque);
}

int il_servo_torque_set(il_servo_t *servo, double torque)
{
	return servo->ops->torque_set(servo, torque);
}

int il_servo_position_get(il_servo_t *servo, double *pos)
{
	return servo->ops->position_get(servo, pos);
}

int il_servo_position_set(il_servo_t *servo, double pos, int immediate,
			  int relative, int sp_timeout)
{
	return servo->ops->position_set(servo, pos, immediate, relative,
					sp_timeout);
}

int il_servo_position_res_get(il_servo_t *servo, uint32_t *res)
{
	return servo->ops->position_res_get(servo, res);
}

int il_servo_velocity_get(il_servo_t *servo, double *vel)
{
	return servo->ops->velocity_get(servo, vel);
}

int il_servo_velocity_set(il_servo_t *servo, double vel)
{
	return servo->ops->velocity_set(servo, vel);
}

int il_servo_velocity_res_get(il_servo_t *servo, uint32_t *res)
{
	return servo->ops->velocity_res_get(servo, res);
}

int il_servo_wait_reached(il_servo_t *servo, int timeout)
{
	return servo->ops->wait_reached(servo, timeout);
}

void il_servo_fake_destroy(il_servo_t *servo) {}

int il_servo_lucky(il_net_prot_t prot, il_net_t **net, il_servo_t **servo,
		   const char *dict)
{	

	il_net_dev_list_t *devs, *dev;
	il_net_servos_list_t *servo_ids, *servo_id;


	/* scan all available network devices */
	devs = il_net_dev_list_get(prot);
	il_net_dev_list_foreach(dev, devs) {
		il_net_opts_t opts;

		opts.port = dev->port;
		opts.timeout_rd = IL_NET_TIMEOUT_RD_DEF;
		opts.timeout_wr = IL_NET_TIMEOUT_WR_DEF;
		opts.connect_slave = 1;

		*net = il_net_create(prot, &opts);
		if (!*net)
			continue;

		/* try to connect to any available servo */
		servo_ids = il_net_servos_list_get(*net, NULL, NULL);
		il_net_servos_list_foreach(servo_id, servo_ids) {
			*servo = il_servo_create(*net, servo_id->id, dict);
			/* found */
			if (*servo) {
				il_net_servos_list_destroy(servo_ids);
				il_net_dev_list_destroy(devs);

				return 0;
			}
		}

		il_net_servos_list_destroy(servo_ids);
		il_net_destroy(*net);
	}

	il_net_dev_list_destroy(devs);

	ilerr__set("No connected servos found");
	return IL_EFAIL;
}

int il_servo_lucky_eth(il_net_prot_t prot, il_net_t **net, il_servo_t **servo,
		   const char *dict, const char *address_ip, int port_ip, int protocol)
{	
	il_eth_net_dev_list_t *dev;
	il_net_servos_list_t *servo_ids, *servo_id;

	/* scan all available network devices */
	dev = il_net_dev_list_get(prot);
	il_eth_net_opts_t opts;

	opts.address_ip = address_ip;
	opts.timeout_rd = IL_NET_TIMEOUT_RD_DEF;
	opts.timeout_wr = IL_NET_TIMEOUT_WR_DEF;
	opts.connect_slave = 1;
	opts.port_ip = port_ip;
	opts.port = "";
	opts.protocol = protocol;

	printf("before connect");
	*net = il_net_create(prot, &opts);
	if (!*net) {
		printf("FAIL");
		return IL_EFAIL;
	}
	/* try to connect to any available servo */
	servo_ids = il_net_servos_list_get(*net, NULL, NULL);
	


	il_net_servos_list_foreach(servo_id, servo_ids) {
		*servo = il_servo_create(*net, servo_id->id, dict);
		/* found */
		if (*servo) {
			il_net_servos_list_destroy(servo_ids);

			return 0;
		}
	}
	il_net_servos_list_destroy(servo_ids);
	il_net_destroy(*net);
	
	ilerr__set("No connected servos found");
	return IL_EFAIL;
}


int il_servo_is_connected(il_net_t **net, const char *address_ip, int port_ip, int protocol) 
{
	il_eth_net_opts_t opts;

	opts.address_ip = address_ip;
	opts.timeout_rd = IL_NET_TIMEOUT_RD_DEF;
	opts.timeout_wr = IL_NET_TIMEOUT_WR_DEF;
	opts.connect_slave = 0;
	opts.port_ip = port_ip;
	opts.port = "";
	opts.protocol = protocol;

	*net = il_net_create(IL_NET_PROT_ETH, &opts);
	if (!*net) {
		printf("FAIL");
		return IL_EFAIL;
	}
	int r = 0;
	il_net_close_socket(*net);
	r = il_net_is_slave_connected(*net, address_ip);
	il_net_destroy(*net);
	return r;
}

const uint16_t *il_servo_subnodes_get(il_servo_t *servo) 
{
	return servo->subnodes;
}

int il_servo_connect_ecat(il_net_prot_t prot, const char *ifname, const char *if_address_ip, il_net_t **net, il_servo_t **servo,
		   const char *dict, const char *address_ip, int port_ip)
{
	il_net_servos_list_t *servo_ids, *servo_id;

	il_ecat_net_opts_t opts;

	opts.address_ip = address_ip;
	opts.timeout_rd = IL_NET_TIMEOUT_RD_DEF;
	opts.timeout_wr = IL_NET_TIMEOUT_WR_DEF;
	opts.connect_slave = 1;
	opts.port_ip = port_ip;
	opts.port = "";
	
	// Initialization of the EtherCAT master
	int r = il_net_master_startup(net, ifname, if_address_ip);
	printf("master_startup result: %i\n", r);
	if (r > 0) {
		printf("Servos found!\n");
		// Wait until slaves are initialized
		Sleep(2000);

		*net = il_net_create(prot, &opts);
		if (!*net) {
			printf("FAIL");
			return IL_EFAIL;
		}

		/* Create as much servos as slaves found */
		servo_ids = il_net_servos_list_get(*net, NULL, NULL);

		/* Create servo */
		il_net_servos_list_foreach(servo_id, servo_ids) {
			*servo = il_servo_create(*net, servo_id->id, dict);
			/* found */
			if (servo) {
				il_net_servos_list_destroy(servo_ids);
				return r;
			}
		}
	}
	printf("No connected servos found\n");
	// ilerr__set("No connected servos found");
	return 0;
}
