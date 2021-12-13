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

#include "net.h"


#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Internal
 ******************************************************************************/
void il_net__retain(il_net_t *net)
{
	net->ops->_retain(net);
}

void il_net__release(il_net_t *net)
{
	net->ops->_release(net);
}

void il_net__state_set(il_net_t *net, il_net_state_t state)
{
	net->ops->_state_set(net, state);
}

int il_net__write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address, const void *buf,
		  size_t sz, int confirmed, uint16_t extended)
{
	return net->ops->_write(net, id, subnode, address, buf, sz, confirmed, extended);
}

int il_net__wait_write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address, const void *buf,
		  size_t sz, int confirmed, uint16_t extended)
{
	return net->ops->_wait_write(net, id, subnode, address, buf, sz, confirmed, extended);
}

int il_net__read(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address, void *buf,
		 size_t sz)
{
	return net->ops->_read(net, id, subnode, address, buf, sz);
}

int il_net__sw_subscribe(il_net_t *net, uint16_t id,
			 il_net_sw_subscriber_cb_t cb, void *ctx)
{
	return net->ops->_sw_subscribe(net, id, cb, ctx);
}

void il_net__sw_unsubscribe(il_net_t *net, int slot)
{
	net->ops->_sw_unsubscribe(net, slot);
}

int il_net__emcy_subscribe(il_net_t *net, uint16_t id,
			   il_net_emcy_subscriber_cb_t cb, void *ctx)
{
	return net->ops->_emcy_subscribe(net, id, cb, ctx);
}

void il_net__emcy_unsubscribe(il_net_t *net, int slot)
{
	net->ops->_emcy_unsubscribe(net, slot);
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_net_t *il_net_create(il_net_prot_t prot, const il_net_opts_t *opts)
{
	switch (prot) {
#ifdef IL_HAS_PROT_EUSB
	case IL_NET_PROT_EUSB:
		return il_eusb_net_ops.create(opts);
#endif
#ifdef IL_HAS_PROT_MCB
	case IL_NET_PROT_MCB:
		return il_mcb_net_ops.create(opts);
	case IL_NET_PROT_ETH:
		return il_eth_net_ops.create(opts);
	case IL_NET_PROT_ECAT:
		return il_ecat_net_ops.create(opts);
#endif
#ifdef IL_HAS_PROT_VIRTUAL
	case IL_NET_PROT_VIRTUAL:
		return il_virtual_net_ops.create(opts);
#endif
	default:
		ilerr__set("Unsupported network protocol");
		return NULL;
	}
}

void il_net_destroy(il_net_t *net)
{
	net->ops->destroy(net);
}

int il_net_connect(il_net_t *net)
{
	return net->ops->connect(net);
}

void il_net_disconnect(il_net_t *net)
{
	net->ops->disconnect(net);
}

il_net_prot_t il_net_prot_get(il_net_t *net)
{
	return net->prot;
}

int il_net_status_get(il_net_t *net)
{
	return net->ops->status_get(net);
}

int *il_net_is_slave_connected(il_net_t *net, const char *ip)
{
	return il_eth_net_ops.is_slave_connected(net);
}

int il_net_mon_stop(il_net_t *net)
{
	return net->ops->mon_stop(net);
}

il_net_state_t il_net_state_get(il_net_t *net)
{
	return net->ops->state_get(net);
}

const char *il_net_port_get(il_net_t *net)
{
	return (const char *)net->port;
}

char *il_net_extended_buffer_get(il_net_t *net)
{
	return net->extended_buff;
}

uint16_t *il_net_monitornig_data_get(il_net_t *net)
{
	return net->monitoring_raw_data;
}

uint32_t il_net_monitornig_data_size_get(il_net_t *net)
{
	return net->monitoring_data_size;
}

uint16_t il_net_monitornig_bytes_per_block_get(il_net_t *net)
{
	return net->monitoring_bytes_per_block;
}

uint16_t *il_net_disturbance_data_get(il_net_t *net)
{
	return net->disturbance_data;
}

uint32_t il_net_disturbance_data_size_get(il_net_t *net)
{
	return net->disturbance_data_size;
}

void il_net_disturbance_data_set(il_net_t *net, uint16_t disturbance_data[2048])
{
	for (int i = 0; i < 2048; net->disturbance_data[i] = disturbance_data[i], i++);
}

void il_net_disturbance_data_size_set(il_net_t *net, uint16_t disturbance_data_size)
{
	net->disturbance_data_size = disturbance_data_size;
}

int *il_net_remove_all_mapped_registers(il_net_t *net)
{
	return net->ops->remove_all_mapped_registers(net);
}

int *il_net_set_mapped_register(il_net_t *net, int channel, uint32_t address, uint8_t subnode, il_reg_dtype_t dtype, uint8_t size)
{
	return net->ops->set_mapped_register(net, channel, address, subnode, dtype, size);
}

uint16_t il_net_num_mapped_registers_get(il_net_t *net)
{
	return net->monitoring_number_mapped_registers;
}

int *il_net_enable_monitoring(il_net_t *net)
{
	return net->ops->enable_monitoring(net);
}

int *il_net_disable_monitoring(il_net_t *net)
{
	return net->ops->disable_monitoring(net);
}

int *il_net_enable_disturbance(il_net_t *net)
{
	return net->ops->enable_disturbance(net);
}

int *il_net_disable_disturbance(il_net_t *net)
{
	return net->ops->disable_disturbance(net);
}

int *il_net_monitoring_remove_data(il_net_t *net)
{
	return net->ops->monitoring_remove_data(net);
}

int *il_net_disturbance_remove_data(il_net_t *net)
{
	return net->ops->disturbance_remove_data(net);
}

int *il_net_read_monitoring_data(il_net_t *net)
{
	return net->ops->read_monitoring_data(net);
}

uint16_t *il_net_monitoring_channel_u16(il_net_t *net, int channel)
{
	return net->monitoring_data_channels[channel].value.monitoring_data_u16;
}

int16_t *il_net_monitoring_channel_s16(il_net_t *net, int channel)
{
	return net->monitoring_data_channels[channel].value.monitoring_data_s16;
}

uint32_t *il_net_monitoring_channel_u32(il_net_t *net, int channel)
{
	return net->monitoring_data_channels[channel].value.monitoring_data_u32;
}

int32_t *il_net_monitoring_channel_s32(il_net_t *net, int channel)
{
	return net->monitoring_data_channels[channel].value.monitoring_data_s32;
}

float *il_net_monitoring_channel_flt(il_net_t *net, int channel)
{
	return net->monitoring_data_channels[channel].value.monitoring_data_flt;
}

int *il_net_disturbance_remove_all_mapped_registers(il_net_t *net)
{
	return net->ops->disturbance_remove_all_mapped_registers(net);
}



int *il_net_disturbance_set_mapped_register(il_net_t *net,
	int channel, uint32_t address,
	uint8_t subnode, il_reg_dtype_t dtype, uint8_t size)
{
	net->last_channel = net->last_channel > channel ? net->last_channel : channel;
	return net->ops->disturbance_set_mapped_register(net, channel, address, subnode, dtype, size);
}

void il_net_disturbance_data_u16_set(il_net_t *net, int channel, uint16_t disturbance_data[2048])
{
	for (int i = 0; i < (1010/sizeof(uint16_t)); net->disturbance_data_channels[channel].value.disturbance_data_u16[i] = disturbance_data[i], i++);
}

void il_net_disturbance_data_s16_set(il_net_t *net, int channel, int16_t disturbance_data[2048])
{
	for (int i = 0; i < (1010/sizeof(int16_t)); net->disturbance_data_channels[channel].value.disturbance_data_s16[i] = disturbance_data[i], i++);
}

void il_net_disturbance_data_u32_set(il_net_t *net, int channel, uint32_t disturbance_data[2048])
{
	for (int i = 0; i < (1010/sizeof(uint32_t)); net->disturbance_data_channels[channel].value.disturbance_data_u32[i] = disturbance_data[i], i++);
}

void il_net_disturbance_data_s32_set(il_net_t *net, int channel, int32_t disturbance_data[2048])
{
	for (int i = 0; i < (1010/sizeof(int32_t)); net->disturbance_data_channels[channel].value.disturbance_data_s32[i] = disturbance_data[i], i++);
}

void il_net_disturbance_data_flt_set(il_net_t *net, int channel, float disturbance_data[2048], uint32_t size)
{
	for (int i = 0; i < (1010/sizeof(float)); net->disturbance_data_channels[channel].value.disturbance_data_flt[i] = disturbance_data[i], i++);
}

int il_net_close_socket(il_net_t *net)
{
	return il_eth_net_ops.close_socket(net);
}

int il_net_ecat_close_socket(il_net_t *net)
{
	return il_ecat_net_ops.close_socket(net);
}

int il_net_master_startup(il_net_t *net, char *ifname, uint16_t slave, uint8_t use_eoe_comms)
{
	return il_ecat_net_ops.master_startup(net, ifname, slave, use_eoe_comms);
}

int il_net_num_slaves_get(char *ifname)
{
	return il_ecat_net_ops.num_slaves_get(ifname);
}

int il_net_master_stop(il_net_t *net)
{
	return il_ecat_net_ops.master_stop(net);
}

int il_net_set_if_params(il_net_t *net, char *ifname, char *if_address_ip)
{
	return il_ecat_net_ops.set_if_params(net, ifname, if_address_ip);
}

int il_net_update_firmware(il_net_t **net, char *ifname, uint16_t slave, char *filename, bool is_summit)
{
	return il_ecat_net_ops.update_firmware(net, ifname, slave, filename, is_summit);
}

int il_net_eeprom_tool(il_net_t **net, char *ifname, int slave, int mode, char *fname)
{
	return il_ecat_net_ops.eeprom_tool(net, ifname, slave, mode, fname);
}

int il_net_force_error(il_net_t **net, char *ifname, char *if_address_ip)
{
	return il_ecat_net_ops.force_error(net, ifname, if_address_ip);
}

int il_net_set_reconnection_retries(il_net_t *net, uint8_t num_reconnections)
{
	switch(net->prot)
	{
		case IL_NET_PROT_ETH:
			return il_eth_net_ops.set_reconnection_retries(net, num_reconnections);
		case IL_NET_PROT_ECAT:
			return il_ecat_net_ops.set_reconnection_retries(net, num_reconnections);
	}
}

int il_net_set_recv_timeout(il_net_t *net, uint32_t timeout)
{
	switch(net->prot)
	{
		case IL_NET_PROT_ETH:
			return il_eth_net_ops.set_recv_timeout(net, timeout);
		case IL_NET_PROT_ECAT:
			return il_ecat_net_ops.set_recv_timeout(net, timeout);
	}
}

int il_net_set_status_check_stop(il_net_t *net, int stop)
{
	switch(net->prot)
	{
		case IL_NET_PROT_ETH:
			return il_eth_net_ops.set_status_check_stop(net, stop);
		case IL_NET_PROT_ECAT:
			return il_ecat_net_ops.set_status_check_stop(net, stop);
	}
}

int il_net_test(il_net_t *net) {
	switch(net->prot) {
		case IL_NET_PROT_ECAT:
			return il_ecat_net_ops.net_test(net);
	}
}

int il_net_SDO_read(il_net_t *net, uint8_t slave, uint16_t index, uint8_t subindex, il_reg_dtype_t dtype, double *buf)
{
	switch(net->prot)
	{
		case IL_NET_PROT_ECAT:

			uint8_t u8_v;
			uint16_t u16_v;
			uint32_t u32_v;
			uint32_t u32_str_v;
			uint64_t u64_v;
			int8_t s8_v;
			int16_t s16_v;
			int32_t s32_v;
			int64_t s64_v;
			float float_v;

			double buf_;

			int r = 0;

			switch(dtype)
			{
				case IL_REG_DTYPE_U8:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(uint8_t), &u8_v);
					buf_ = (double)u8_v;
					break;
				case IL_REG_DTYPE_S8:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(int8_t), &s8_v);
					buf_ = (double)s8_v;
					break;
				case IL_REG_DTYPE_U16:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(uint16_t), &u16_v);
					buf_ = (double)u16_v;
					break;
				case IL_REG_DTYPE_S16:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(int16_t), &s16_v);
					buf_ = (float)s16_v;
					break;
				case IL_REG_DTYPE_U32:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(uint32_t), &u32_v);
					buf_ = (double)u32_v;
					break;
				case IL_REG_DTYPE_S32:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(int32_t), &s32_v);
					buf_ = (double)s32_v;
					break;
				case IL_REG_DTYPE_U64:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(uint64_t), &u64_v);
					buf_ = (double)u64_v;
					break;
				case IL_REG_DTYPE_S64:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(int64_t), &s64_v);
					buf_ = (double)s64_v;
					break;
				case IL_REG_DTYPE_FLOAT:
					il_ecat_net_ops.SDO_read(net, slave, index, subindex, sizeof(float), &float_v);
					buf_ = (double)float_v;
					break;
				default:
					ilerr__set("Unsupported register data type");
					return IL_EINVAL;
			}
			*buf = buf_;
			return r;
	}
}

int il_net_SDO_read_array(il_net_t *net, uint8_t slave, uint16_t index, uint8_t subindex, int size, void *buf)
	{
	switch(net->prot)
	{
		case IL_NET_PROT_ECAT:
			return il_ecat_net_ops.SDO_read(net, slave, index, subindex, size, buf);
	}
}

int il_net_SDO_read_complete_access(il_net_t *net, uint8_t slave, uint16_t index, int size, void *buf) 
	{
	switch(net->prot)
	{
		case IL_NET_PROT_ECAT:
			return il_ecat_net_ops.SDO_read_complete_access(net, slave, index, size, buf);
	}
}

int il_net_SDO_write(il_net_t *net, uint8_t slave, uint16_t index, uint8_t subindex, il_reg_dtype_t dtype, double buf)
{
	switch(net->prot)
	{
		case IL_NET_PROT_ECAT:
			uint8_t u8_v;
			uint16_t u16_v;
			uint32_t u32_v;
			uint32_t u32_str_v;
			uint64_t u64_v;
			int8_t s8_v;
			int16_t s16_v;
			int32_t s32_v;
			int64_t s64_v;
			float float_v;
			double val_;

			// val_ = buf;

			switch (dtype) {
				case IL_REG_DTYPE_U8:
					u8_v = (uint8_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &u8_v);
				case IL_REG_DTYPE_S8:
					s8_v = (int8_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &s8_v);
				case IL_REG_DTYPE_U16:
					u16_v = (uint16_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &u16_v);
				case IL_REG_DTYPE_S16:
					s16_v = (int16_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &s16_v);
				case IL_REG_DTYPE_U32:
					u32_v = (uint32_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &u32_v);
				case IL_REG_DTYPE_S32:
					s32_v = (int32_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &s32_v);
				case IL_REG_DTYPE_U64:
					u64_v = (uint64_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &u64_v);
				case IL_REG_DTYPE_S64:
					s64_v = (int64_t)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &s64_v);
				case IL_REG_DTYPE_FLOAT:
					float_v = (double)buf;
					return il_ecat_net_ops.SDO_write(net, slave, index, subindex, dtype, &float_v);
				default:
					ilerr__set("Unsupported register data type");
					return IL_EINVAL;
			}


	}
}

il_net_servos_list_t *il_net_servos_list_get(il_net_t *net,
					     il_net_servos_on_found_t on_found,
					     void *ctx)
{
	return net->ops->servos_list_get(net, on_found, ctx);
}

void il_net_servos_list_destroy(il_net_servos_list_t *lst)
{
	il_net_servos_list_t *curr;

	curr = lst;
	while (curr) {
		il_net_servos_list_t *tmp;

		tmp = curr->next;
		free(curr);
		curr = tmp;
	}
}

void il_net_dev_list_destroy(il_net_dev_list_t *lst)
{
	il_net_dev_list_t *curr;

	curr = lst;
	while (curr) {
		il_net_dev_list_t *tmp;

		tmp = curr->next;
		free(curr);
		curr = tmp;
	}
}

il_net_dev_mon_t *il_net_dev_mon_create(il_net_prot_t prot)
{
	switch (prot) {
#ifdef IL_HAS_PROT_EUSB
	case IL_NET_PROT_EUSB:
		return il_eusb_net_dev_mon_ops.create();
#endif
#ifdef IL_HAS_PROT_MCB
	case IL_NET_PROT_MCB:
		return il_mcb_net_dev_mon_ops.create();
	 case IL_NET_PROT_ETH:
	 	return il_eth_net_dev_mon_ops.create();
	 case IL_NET_PROT_ECAT:
		 return il_ecat_net_dev_mon_ops.create();
#endif
	default:
		ilerr__set("Unsupported network protocol");
		return NULL;
	}
}

void il_net_dev_mon_destroy(il_net_dev_mon_t *mon)
{
	mon->ops->destroy(mon);
}

int il_net_dev_mon_start(il_net_dev_mon_t *mon, il_net_dev_on_evt_t on_evt,
			 void *ctx)
{
	return mon->ops->start(mon, on_evt, ctx);
}

void il_net_dev_mon_stop(il_net_dev_mon_t *mon)
{
	mon->ops->stop(mon);
}

void il_net_fake_destroy(il_net_t *net) {}

il_net_dev_list_t *il_net_dev_list_get(il_net_prot_t prot)
{
	switch (prot) {
#ifdef IL_HAS_PROT_EUSB
	case IL_NET_PROT_EUSB:
		return il_eusb_net_dev_list_get();
#endif
#ifdef IL_HAS_PROT_MCB
	case IL_NET_PROT_MCB:
		return il_mcb_net_dev_list_get();
	case IL_NET_PROT_ETH:
	 	return il_eth_net_dev_list_get();
#endif
	default:
		ilerr__set("Unsupported network protocol");
		return NULL;
	}
}
