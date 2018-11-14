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
		  size_t sz, int confirmed)
{
	return net->ops->_write(net, id, subnode, address, buf, sz, confirmed);
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

il_net_state_t il_net_state_get(il_net_t *net)
{
	return net->ops->state_get(net);
}

const char *il_net_port_get(il_net_t *net)
{
	return (const char *)net->port;
}

uint16_t *il_net_monitornig_data_get(il_net_t *net) 
{
	return net->monitoring_data;
}

uint16_t il_net_monitornig_data_size_get(il_net_t *net) 
{
	return net->monitoring_data_size;
}

uint16_t *il_net_disturbance_data_get(il_net_t *net) 
{
	return net->disturbance_data;
}

uint16_t il_net_disturbance_data_size_get(il_net_t *net) 
{
	return net->disturbance_data_size;
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
	// case IL_NET_PROT_ETH:
	// 	return il_eth_net_dev_mon_ops.create(opts);
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
