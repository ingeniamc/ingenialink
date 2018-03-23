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
#include "frame.h"

#include <string.h>

#include "ingenialink/err.h"
#include "ingenialink/base/net.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Compute CRC of the given buffer.
 *
 * @param [in] buf
 *	Buffer.
 * @param [in] sz
 *	Buffer size (bytes).
 *
 * @return
 *	CRC.
 */
static uint16_t crc_calc(const uint8_t *buf, size_t sz)
{
	size_t pos, i;
	uint16_t result = 0;

	for (pos = 0; pos < sz; pos++) {
		result ^= (uint16_t)buf[pos] << 8;

		for (i = 0; i < 8; i++) {
			if ((result & 0x8000) != 0)
				result = (result << 1) ^ MCB_CRC_POLY;
			else
				result <<= 1;
		}
	}

	return result;
}

static int net_send(il_mcb_net_t *this, uint16_t address, const void *data,
		    size_t sz)
{
	int finished = 0;
	uint8_t cmd;
	size_t pending_sz = sz;

	cmd = sz ? MCB_CMD_WRITE : MCB_CMD_READ;

	(void)ser_flush(this->ser, SER_QUEUE_ALL);

	while (!finished) {
		int r;
		uint8_t frame[MCB_FRAME_SZ], pending;
		uint16_t hdr, crc;
		size_t chunk_sz;

		/* header */
		pending = (pending_sz > MCB_DATA_SZ) ? 1 : 0;

		hdr = (address << MCB_ADDR_POS) | (cmd << MCB_CMD_POS) |
		      (pending << MCB_PENDING_POS);
		*(uint16_t *)&frame[MCB_HDR_H] = hdr;

		/* data */
		chunk_sz = MIN(pending_sz, MCB_DATA_SZ);
		if (chunk_sz)
			memcpy(&frame[MCB_DATA_POS], data, chunk_sz);

		memset(&frame[MCB_DATA_POS + chunk_sz], 0,
		       MCB_DATA_SZ - chunk_sz);

		il_mcb_frame__swap(frame, sizeof(frame));

		/* crc */
		crc = crc_calc(frame, MCB_PAYLOAD_SZ);
		*(uint16_t *)&frame[MCB_CRC_H] = __swap_le_16(crc);

		/* send frame */
		r = ser_write(this->ser, frame, sizeof(frame), NULL);
		if (r < 0)
			return ilerr__ser(r);

		/* update pending */
		pending_sz -= chunk_sz;
		if (!pending_sz)
			finished = 1;
	}

	return 0;
}

static int net_recv(il_mcb_net_t *this, uint16_t address, uint8_t *buf,
		    size_t sz)
{
	int finished = 0;
	size_t pending_sz = sz;

	while (!finished) {
		uint8_t frame[MCB_FRAME_SZ];
		size_t block_sz = 0;
		uint16_t crc, hdr;

		/* read next frame */
		while (block_sz < MCB_FRAME_SZ) {
			int r;
			size_t chunk_sz;

			r = ser_read(this->ser, &frame[block_sz],
				     sizeof(frame) - block_sz, &chunk_sz);
			if (r == SER_EEMPTY) {
				r = ser_read_wait(this->ser);
				if (r < 0)
					return ilerr__ser(r);
			} else if (r < 0) {
				return ilerr__ser(r);
			} else {
				block_sz += chunk_sz;
			}
		}

		/* process frame: validate CRC, address, ACK */
		crc = *(uint16_t *)&frame[MCB_CRC_H];
		crc = __swap_le_16(crc);
		if (crc_calc(frame, MCB_PAYLOAD_SZ) != crc) {
			ilerr__set("Communications error (CRC mismatch)");
			return IL_EIO;
		}

		il_mcb_frame__swap(frame, sizeof(frame));

		hdr = *(uint16_t *)&frame[MCB_HDR_H];

		if (((hdr & MCB_CMD_MSK) >> MCB_CMD_POS) != MCB_CMD_ACK) {
			uint32_t err;

			err = __swap_be_32(*(uint32_t *)&frame[MCB_DATA_POS]);

			ilerr__set("Communications error (NACK -> %08x)", err);
			return IL_EIO;
		}

		if (((hdr & MCB_ADDR_MSK) >> MCB_ADDR_POS) != address) {
			ilerr__set("Communications error (bad address)");
			return IL_EIO;
		}

		if (!pending_sz) {
			finished = 1;
		} else {
			size_t data_sz;

			/* store data */
			data_sz = MIN(pending_sz, MCB_DATA_SZ);
			memcpy(buf, &frame[MCB_DATA_POS], data_sz);
			buf += data_sz;

			/* update pending size */
			pending_sz -= data_sz;
			if (!pending_sz) {
				if (hdr & MCB_PENDING_MSK) {
					ilerr__set("Unexpected pending data");
					return IL_EIO;
				}

				finished = 1;
			}
		}
	}

	return 0;
}

/**
 * Monitor event callback.
 */
static void on_ser_evt(void *ctx, ser_dev_evt_t evt, const ser_dev_t *dev)
{
	il_mcb_net_dev_mon_t *this = ctx;

	if (evt == SER_DEV_EVT_ADDED)
		this->on_evt(this->ctx, IL_NET_DEV_EVT_ADDED, dev->path);
	else
		this->on_evt(this->ctx, IL_NET_DEV_EVT_REMOVED, dev->path);
}

/**
 * Destroy MCB network.
 *
 * @param [in] ctx
 *	Context (il_net_t *).
 */
static void mcb_net_destroy(void *ctx)
{
	il_mcb_net_t *this = ctx;

	if (il_net_state_get(&this->net) != IL_NET_STATE_DISCONNECTED)
		ser_close(this->ser);

	ser_destroy(this->ser);

	il_net_base__deinit(&this->net);

	free(this);
}

/*******************************************************************************
 * Implementation: Internal
 ******************************************************************************/

static void il_mcb_net__retain(il_net_t *net)
{
	il_mcb_net_t *this = to_mcb_net(net);

	il_utils__refcnt_retain(this->refcnt);
}

static void il_mcb_net__release(il_net_t *net)
{
	il_mcb_net_t *this = to_mcb_net(net);

	il_utils__refcnt_release(this->refcnt);
}

static int il_mcb_net__read(il_net_t *net, uint16_t id, uint32_t address,
			    void *buf, size_t sz)
{
	il_mcb_net_t *this = to_mcb_net(net);

	int r;

	(void)id;

	osal_mutex_lock(this->net.lock);

	r = net_send(this, (uint16_t)address, NULL, 0);
	if (r < 0)
		goto unlock;

	r = net_recv(this, (uint16_t)address, buf, sz);

unlock:
	osal_mutex_unlock(this->net.lock);

	return r;
}

static int il_mcb_net__write(il_net_t *net, uint16_t id, uint32_t address,
			     const void *buf, size_t sz, int confirmed)
{
	il_mcb_net_t *this = to_mcb_net(net);

	int r;

	(void)id;
	(void)confirmed;

	osal_mutex_lock(this->net.lock);

	r = net_send(this, (uint16_t)address, buf, sz);
	if (r < 0)
		goto unlock;

	r = net_recv(this, (uint16_t)address, NULL, 0);

unlock:
	osal_mutex_unlock(this->net.lock);

	return r;
}

/*******************************************************************************
 * Implementation: Public
 ******************************************************************************/

static il_net_t *il_mcb_net_create(const il_net_opts_t *opts)
{
	il_mcb_net_t *this;
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

	this->net.ops = &il_mcb_net_ops;
	this->net.prot = IL_NET_PROT_MCB;

	/* setup refcnt */
	this->refcnt = il_utils__refcnt_create(mcb_net_destroy, this);
	if (!this->refcnt)
		goto cleanup_net;

	/* allocate serial port */
	this->ser = ser_create();
	if (!this->ser) {
		ilerr__set("Serial port allocation failed (%s)", sererr_last());
		goto cleanup_refcnt;
	}

	/* open serial port */
	this->sopts.port = il_net_port_get(&this->net);
	this->sopts.baudrate = BAUDRATE_DEF;
	this->sopts.timeouts.rd = opts->timeout_rd;
	this->sopts.timeouts.wr = opts->timeout_wr;

	r = il_net_connect(&this->net);
	if (r < 0)
		goto cleanup_ser;

	return &this->net;

cleanup_ser:
	ser_destroy(this->ser);

cleanup_refcnt:
	il_utils__refcnt_destroy(this->refcnt);

cleanup_net:
	il_net_base__deinit(&this->net);

cleanup_this:
	free(this);

	return NULL;
}

static void il_mcb_net_destroy(il_net_t *net)
{
	il_mcb_net_t *this = to_mcb_net(net);

	il_utils__refcnt_release(this->refcnt);
}

static int il_mcb_net_connect(il_net_t *net)
{
	int r;
	il_mcb_net_t *this = to_mcb_net(net);

	if (il_net_state_get(&this->net) == IL_NET_STATE_CONNECTED) {
		ilerr__set("Network already connected");
		return IL_EALREADY;
	}

	r = ser_open(this->ser, &this->sopts);
	if (r < 0) {
		ilerr__set("Serial port open failed (%s)", sererr_last());
		return IL_EFAIL;
	}

	il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);

	return 0;
}

static void il_mcb_net_disconnect(il_net_t *net)
{
	il_mcb_net_t *this = to_mcb_net(net);

	if (il_net_state_get(&this->net) != IL_NET_STATE_DISCONNECTED) {
		ser_close(this->ser);
		il_net__state_set(&this->net, IL_NET_STATE_DISCONNECTED);
	}
}

static il_net_servos_list_t *il_mcb_net_servos_list_get(
	il_net_t *net, il_net_servos_on_found_t on_found, void *ctx)
{
	int r;
	uint32_t vid;
	il_net_servos_list_t *lst;

	/* try to read the vendor id register to see if a servo is alive */
	r = il_net__read(net, 1, VENDOR_ID_ADDR, &vid, sizeof(vid));
	if (r < 0)
		return NULL;

	/* create list with one element (id=1) */
	lst = malloc(sizeof(*lst));
	if (!lst)
		return NULL;

	lst->next = NULL;
	lst->id = 1;

	if (on_found)
		on_found(ctx, 1);

	return lst;
}

static il_net_dev_mon_t *il_mcb_net_dev_mon_create(void)
{
	il_mcb_net_dev_mon_t *this;

	this = malloc(sizeof(*this));
	if (!this) {
		ilerr__set("Monitor allocation failed");
		return NULL;
	}

	this->mon.ops = &il_mcb_net_dev_mon_ops;
	this->running = 0;

	return &this->mon;
}

static void il_mcb_net_dev_mon_destroy(il_net_dev_mon_t *mon)
{
	il_mcb_net_dev_mon_t *this = to_mcb_mon(mon);

	il_net_dev_mon_stop(mon);

	free(this);
}

static int il_mcb_net_dev_mon_start(il_net_dev_mon_t *mon,
				    il_net_dev_on_evt_t on_evt,
				    void *ctx)
{
	il_mcb_net_dev_mon_t *this = to_mcb_mon(mon);

	if (this->running) {
		ilerr__set("Monitor already running");
		return IL_EALREADY;
	}

	/* store context and bring up monitor */
	this->ctx = ctx;
	this->on_evt = on_evt;
	this->smon = ser_dev_monitor_init(on_ser_evt, this);
	if (!this->smon) {
		ilerr__set("Network device monitor allocation failed (%s)",
			   sererr_last());
		return IL_EFAIL;
	}

	this->running = 1;

	return 0;
}

static void il_mcb_net_dev_mon_stop(il_net_dev_mon_t *mon)
{
	il_mcb_net_dev_mon_t *this = to_mcb_mon(mon);

	if (this->running) {
		ser_dev_monitor_stop(this->smon);
		this->running = 0;
	}
}

il_net_dev_list_t *il_mcb_net_dev_list_get()
{
	il_net_dev_list_t *lst = NULL;
	il_net_dev_list_t *prev;

	ser_dev_list_t *ser_devs;
	ser_dev_list_t *ser_dev;

	/* obtain all serial ports */
	ser_devs = ser_dev_list_get();
	if (!ser_devs)
		return NULL;

	/* create the device list */
	ser_dev_list_foreach(ser_dev, ser_devs) {
		/* allocate new list entry */
		prev = lst;
		lst = malloc(sizeof(*lst));
		if (!lst) {
			il_net_dev_list_destroy(prev);
			break;
		}

		lst->next = prev;

		/* store port */
		strncpy(lst->port, ser_dev->dev.path, sizeof(lst->port));
	}

	ser_dev_list_destroy(ser_devs);

	return lst;
}

/** MCB network operations. */
const il_net_ops_t il_mcb_net_ops = {
	/* internal */
	._retain = il_mcb_net__retain,
	._release = il_mcb_net__release,
	._state_set = il_net_base__state_set,
	._read = il_mcb_net__read,
	._write = il_mcb_net__write,
	._sw_subscribe = il_net_base__sw_subscribe,
	._sw_unsubscribe = il_net_base__sw_unsubscribe,
	._emcy_subscribe = il_net_base__emcy_subscribe,
	._emcy_unsubscribe = il_net_base__emcy_unsubscribe,
	/* public */
	.create = il_mcb_net_create,
	.destroy = il_mcb_net_destroy,
	.connect = il_mcb_net_connect,
	.disconnect = il_mcb_net_disconnect,
	.state_get = il_net_base__state_get,
	.servos_list_get = il_mcb_net_servos_list_get,
};

/** MCB network device monitor operations. */
const il_net_dev_mon_ops_t il_mcb_net_dev_mon_ops = {
	.create = il_mcb_net_dev_mon_create,
	.destroy = il_mcb_net_dev_mon_destroy,
	.start = il_mcb_net_dev_mon_start,
	.stop = il_mcb_net_dev_mon_stop,
};
