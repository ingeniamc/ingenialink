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
#include "ingenialink/base/net.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Destroy VIRTUAL network.
 *
 * @param [in] ctx
 *	Context (il_net_t *).
 */
static void virtual_net_destroy(void *ctx)
{
	il_virtual_net_t *this = ctx;

	il_net_base__deinit(&this->net);

	free(this);
}

/*******************************************************************************
 * Implementation: Internal
 ******************************************************************************/

static void il_virtual_net__retain(il_net_t *net)
{
	il_virtual_net_t *this = to_virtual_net(net);

	il_utils__refcnt_retain(this->refcnt);
}

static void il_virtual_net__release(il_net_t *net)
{
	il_virtual_net_t *this = to_virtual_net(net);

	il_utils__refcnt_release(this->refcnt);
}

static int il_virtual_net__read(il_net_t *net, uint16_t id, uint32_t address,
				void *buf, size_t sz)
{
	(void)net;
	(void)id;
	(void)address;
	(void)buf;
	(void)sz;

	return 0;
}

static int il_virtual_net__write(il_net_t *net, uint16_t id, uint32_t address,
				 const void *buf, size_t sz, int confirmed)
{
	(void)net;
	(void)id;
	(void)address;
	(void)buf;
	(void)sz;
	(void)confirmed;

	return 0;
}

/*******************************************************************************
 * Implementation: Public
 ******************************************************************************/

static il_net_t *il_virtual_net_create(const il_net_opts_t *opts)
{
	il_virtual_net_t *this;
	int r;

	this = calloc(1, sizeof(*this));
	if (!this) {
		ilerr__set("Network allocation failed");
		return NULL;
	}

	/* initialize parent */
	r = il_net_base__init(&this->net, opts);
	if (r < 0)
		goto cleanup_this;

	this->net.ops = &il_virtual_net_ops;
	this->net.prot = IL_NET_PROT_VIRTUAL;

	/* setup refcnt */
	this->refcnt = il_utils__refcnt_create(virtual_net_destroy, this);
	if (!this->refcnt)
		goto cleanup_net;

	r = il_net_connect(&this->net);
	if (r < 0)
		goto cleanup_refcnt;

	return &this->net;

cleanup_refcnt:
	il_utils__refcnt_destroy(this->refcnt);

cleanup_net:
	il_net_base__deinit(&this->net);

cleanup_this:
	free(this);

	return NULL;
}

static void il_virtual_net_destroy(il_net_t *net)
{
	il_virtual_net_t *this = to_virtual_net(net);

	il_utils__refcnt_release(this->refcnt);
}

static int il_virtual_net_connect(il_net_t *net)
{
	il_virtual_net_t *this = to_virtual_net(net);

	if (il_net_state_get(&this->net) == IL_NET_STATE_CONNECTED) {
		ilerr__set("Network already connected");
		return IL_EALREADY;
	}

	il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);

	return 0;
}

static void il_virtual_net_disconnect(il_net_t *net)
{
	il_virtual_net_t *this = to_virtual_net(net);

	if (il_net_state_get(&this->net) != IL_NET_STATE_DISCONNECTED)
		il_net__state_set(&this->net, IL_NET_STATE_DISCONNECTED);
}

static il_net_servos_list_t *il_virtual_net_servos_list_get(
	il_net_t *net, il_net_servos_on_found_t on_found, void *ctx)
{
	(void)net;
	(void)on_found;
	(void)ctx;

	return NULL;
}

static il_net_dev_mon_t *il_virtual_net_dev_mon_create(void)
{
	ilerr__set("Functionality not supported");

	return NULL;
}

static void il_virtual_net_dev_mon_destroy(il_net_dev_mon_t *mon)
{
	(void)mon;
}

static int il_virtual_net_dev_mon_start(il_net_dev_mon_t *mon,
					il_net_dev_on_evt_t on_evt,
					void *ctx)
{
	(void)mon;
	(void)on_evt;
	(void)ctx;

	return IL_ENOTSUP;
}

static void il_virtual_net_dev_mon_stop(il_net_dev_mon_t *mon)
{
	(void)mon;
}

il_net_dev_list_t *il_virtual_net_dev_list_get()
{
	return NULL;
}

/** VIRTUAL network operations. */
const il_net_ops_t il_virtual_net_ops = {
	/* internal */
	._retain = il_virtual_net__retain,
	._release = il_virtual_net__release,
	._state_set = il_net_base__state_set,
	._read = il_virtual_net__read,
	._write = il_virtual_net__write,
	._sw_subscribe = il_net_base__sw_subscribe,
	._sw_unsubscribe = il_net_base__sw_unsubscribe,
	._emcy_subscribe = il_net_base__emcy_subscribe,
	._emcy_unsubscribe = il_net_base__emcy_unsubscribe,
	/* public */
	.create = il_virtual_net_create,
	.destroy = il_virtual_net_destroy,
	.connect = il_virtual_net_connect,
	.disconnect = il_virtual_net_disconnect,
	.state_get = il_net_base__state_get,
	.servos_list_get = il_virtual_net_servos_list_get,
};

/** VIRTUAL network device monitor operations. */
const il_net_dev_mon_ops_t il_virtual_net_dev_mon_ops = {
	.create = il_virtual_net_dev_mon_create,
	.destroy = il_virtual_net_dev_mon_destroy,
	.start = il_virtual_net_dev_mon_start,
	.stop = il_virtual_net_dev_mon_stop,
};
