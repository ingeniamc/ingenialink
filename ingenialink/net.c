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

#include "ingenialink/err.h"

/*******************************************************************************
 * Base implementation
 ******************************************************************************/

int il_net_base__sw_subscribe(il_net_t *net, uint16_t id,
			      il_net_sw_subscriber_cb_t cb, void *ctx)
{
	int r = 0;
	int slot;

	osal_mutex_lock(net->sw_subs.lock);

	/* check if already subscribed */
	for (slot = 0; slot < net->sw_subs.sz; slot++) {
		if (net->sw_subs.subs[slot].id == id) {
			ilerr__set("Node already subscribed");
			r = IL_EALREADY;
			goto unlock;
		}
	}

	/* look for the first empty slot */
	for (slot = 0; slot < net->sw_subs.sz; slot++) {
		if (!net->sw_subs.subs[slot].cb)
			break;
	}

	/* increase array if no space left */
	if (slot == net->sw_subs.sz) {
		size_t sz;
		il_net_sw_subscriber_t *subs;

		/* double in size on each realloc */
		sz = 2 * net->sw_subs.sz * sizeof(*subs);
		subs = realloc(net->sw_subs.subs, sz);
		if (!subs) {
			ilerr__set("Subscribers re-allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		net->sw_subs.subs = subs;
		net->sw_subs.sz = sz;
	}

	net->sw_subs.subs[slot].id = id;
	net->sw_subs.subs[slot].cb = cb;
	net->sw_subs.subs[slot].ctx = ctx;

	r = slot;

unlock:
	osal_mutex_unlock(net->sw_subs.lock);

	return r;
}

void il_net_base__sw_unsubscribe(il_net_t *net, int slot)
{
	osal_mutex_lock(net->sw_subs.lock);

	/* skip out of range slot */
	if (slot >= net->sw_subs.sz)
		goto unlock;

	net->sw_subs.subs[slot].id = 0;
	net->sw_subs.subs[slot].cb = NULL;
	net->sw_subs.subs[slot].ctx = NULL;

unlock:
	osal_mutex_unlock(net->sw_subs.lock);
}

int il_net_base__emcy_subscribe(il_net_t *net, uint16_t id,
				il_net_emcy_subscriber_cb_t cb, void *ctx)
{
	int r;
	int slot;

	/* check if already subscribed */
	for (slot = 0; slot < net->emcy_subs.sz; slot++) {
		if (net->emcy_subs.subs[slot].id == id) {
			ilerr__set("Node already subscribed");
			r = IL_EALREADY;
			goto unlock;
		}
	}

	/* look for the first empty slot */
	for (slot = 0; slot < net->emcy_subs.sz; slot++) {
		if (!net->emcy_subs.subs[slot].cb)
			break;
	}

	/* increase array if no space left */
	if (slot == net->emcy_subs.sz) {
		size_t sz;
		il_net_emcy_subscriber_t *subs;

		/* double in size on each realloc */
		sz = 2 * net->emcy_subs.sz * sizeof(*subs);
		subs = realloc(net->emcy_subs.subs, sz);
		if (!subs) {
			ilerr__set("Subscribers re-allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		net->emcy_subs.subs = subs;
		net->emcy_subs.sz = sz;
	}

	net->emcy_subs.subs[slot].id = id;
	net->emcy_subs.subs[slot].cb = cb;
	net->emcy_subs.subs[slot].ctx = ctx;

	r = slot;

unlock:
	osal_mutex_unlock(net->emcy_subs.lock);

	return r;
}

void il_net_base__emcy_unsubscribe(il_net_t *net, int slot)
{
	osal_mutex_lock(net->emcy_subs.lock);

	/* skip out of range slot */
	if (slot >= net->emcy_subs.sz)
		goto unlock;

	net->emcy_subs.subs[slot].id = 0;
	net->emcy_subs.subs[slot].cb = NULL;
	net->emcy_subs.subs[slot].ctx = NULL;

unlock:
	osal_mutex_unlock(net->emcy_subs.lock);
}

int il_net_base__init(il_net_t *net, const il_net_opts_t *opts)
{
	int r;

	/* initialize */
	net->timeout_rd = opts->timeout_rd;
	net->timeout_wr = opts->timeout_wr;

	/* initialize network lock */
	net->lock = osal_mutex_create();
	if (!net->lock) {
		ilerr__set("Network lock allocation failed");
		return IL_ENOMEM;
	}

	/* initialize network state */
	net->state_lock = osal_mutex_create();
	if (!net->state_lock) {
		ilerr__set("Network state lock allocation failed");
		r = IL_ENOMEM;
		goto cleanup_lock;
	}

	net->state = IL_NET_STATE_OPERATIVE;

	/* initialize statusword update subscribers */
	net->sw_subs.subs = calloc(SW_SUBS_SZ_DEF, sizeof(*net->sw_subs.subs));
	if (!net->sw_subs.subs) {
		ilerr__set("Network statusword subscribers allocation failed");
		r = IL_ENOMEM;
		goto cleanup_state_lock;
	}

	net->sw_subs.lock = osal_mutex_create();
	if (!net->sw_subs.lock) {
		ilerr__set("Network statusword lock allocation failed");
		r = IL_ENOMEM;
		goto cleanup_sw_subs_subs;
	}

	net->sw_subs.sz = SW_SUBS_SZ_DEF;

	/* initialize emcy update subscribers */
	net->emcy_subs.subs = calloc(EMCY_SUBS_SZ_DEF,
				     sizeof(*net->emcy_subs.subs));
	if (!net->emcy_subs.subs) {
		ilerr__set("Network emergency subscribers allocation failed");
		r = IL_ENOMEM;
		goto cleanup_sw_subs_lock;
	}

	net->emcy_subs.lock = osal_mutex_create();
	if (!net->emcy_subs.lock) {
		ilerr__set("Network emergency lock allocation failed");
		r = IL_ENOMEM;
		goto cleanup_emcy_subs_subs;
	}

	net->emcy_subs.sz = EMCY_SUBS_SZ_DEF;

	return 0;

cleanup_emcy_subs_subs:
	free(net->emcy_subs.subs);

cleanup_sw_subs_lock:
	osal_mutex_destroy(net->sw_subs.lock);

cleanup_sw_subs_subs:
	free(net->sw_subs.subs);

cleanup_state_lock:
	osal_mutex_destroy(net->state_lock);

cleanup_lock:
	osal_mutex_destroy(net->lock);

	return r;
}

void il_net_base__deinit(il_net_t *net)
{
	osal_mutex_destroy(net->emcy_subs.lock);
	free(net->emcy_subs.subs);

	osal_mutex_destroy(net->sw_subs.lock);
	free(net->sw_subs.subs);

	osal_mutex_destroy(net->state_lock);

	osal_mutex_destroy(net->lock);
}

il_net_state_t il_net_base__state_get(il_net_t *net)
{
	il_net_state_t state;

	osal_mutex_lock(net->state_lock);
	state = net->state;
	osal_mutex_unlock(net->state_lock);

	return state;
}

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

int il_net__write(il_net_t *net, uint16_t id, uint32_t address, const void *buf,
		  size_t sz, int confirmed)
{
	return net->ops->_write(net, id, address, buf, sz, confirmed);
}

int il_net__read(il_net_t *net, uint16_t id, uint32_t address, void *buf,
		 size_t sz)
{
	return net->ops->_read(net, id, address, buf, sz);
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

il_net_prot_t il_net_prot_get(il_net_t *net)
{
	return net->prot;
}

il_net_state_t il_net_state_get(il_net_t *net)
{
	return net->ops->state_get(net);
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
#endif
	default:
		ilerr__set("Unsupported network protocol");
		return NULL;
	}
}
