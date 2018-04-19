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

#include "osal/clock.h"

#include "ingenialink/err.h"
#include "ingenialink/base/net.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Process asynchronous statusword messages.
 *
 * @param [in] this
 *	E-USB Network.
 * @param [in] frame
 *	IngeniaLink frame.
 */
static void process_statusword(il_eusb_net_t *this, il_eusb_frame_t *frame)
{
	uint32_t address;

	address = il_eusb_frame__get_address(frame);

	if (address == STATUSWORD_ADDRESS) {
		il_net_sw_subscriber_lst_t *subs;
		int i;
		uint8_t id;
		uint16_t sw;

		subs = &this->net.sw_subs;

		id = il_eusb_frame__get_id(frame);
		sw = __swap_be_16(*(uint16_t *)il_eusb_frame__get_data(frame));

		osal_mutex_lock(subs->lock);

		for (i = 0; i < subs->sz; i++) {
			if (subs->subs[i].id == id && subs->subs[i].cb) {
				void *ctx;

				ctx = subs->subs[i].ctx;
				subs->subs[i].cb(ctx, sw);

				break;
			}
		}

		osal_mutex_unlock(subs->lock);
	}
}

/**
 * Process asynchronous emergency messages.
 *
 * @param [in] this
 *	E-USB Network.
 * @param [in] frame
 *	IngeniaLink frame.
 */
static void process_emcy(il_eusb_net_t *this, il_eusb_frame_t *frame)
{
	uint32_t address;

	address = il_eusb_frame__get_address(frame);

	if (address == EMCY_ADDRESS) {
		il_net_emcy_subscriber_lst_t *subs;
		int i;
		uint8_t id;
		uint32_t code;

		subs = &this->net.emcy_subs;

		id = il_eusb_frame__get_id(frame);
		code = __swap_be_32(
			*(uint32_t *)il_eusb_frame__get_data(frame));

		osal_mutex_lock(subs->lock);

		for (i = 0; i < subs->sz; i++) {
			if (subs->subs[i].id == id && subs->subs[i].cb) {
				void *ctx;

				ctx = subs->subs[i].ctx;
				subs->subs[i].cb(ctx, code);

				break;
			}
		}

		osal_mutex_unlock(subs->lock);
	}
}

/**
 * Process synchronous messages.
 *
 * @param [in] this
 *	E-USB Network.
 * @param [in] frame
 *	IngeniaLink frame.
 */
static void process_sync(il_eusb_net_t *this, il_eusb_frame_t *frame)
{
	il_eusb_net_sync_t *sync = &this->sync;

	osal_mutex_lock(sync->lock);

	if (!sync->complete) {
		uint8_t id = il_eusb_frame__get_id(frame);
		uint32_t address = il_eusb_frame__get_address(frame);
		size_t sz = il_eusb_frame__get_sz(frame);

		if (((sync->id == id) || (sync->id == 0)) &&
		    (sync->address == address) && (sync->sz >= sz)) {
			void *data = il_eusb_frame__get_data(frame);

			memcpy(sync->buf, data, sz);

			sync->complete = 1;
			osal_cond_signal(sync->cond);
		}
	}

	osal_mutex_unlock(sync->lock);
}

/**
 * Process reception buffer.
 *
 * @param [in] this
 *	E-USB Network.
 * @param [in] rbuf
 *	Reception buffer.
 * @param [in, out] cnt
 *	Buffer contents size.
 * @param [in, out] frame
 *	IngeniaLink frame.
 */
static void process_rbuf(il_eusb_net_t *this, uint8_t *rbuf, size_t *cnt,
			 il_eusb_frame_t *frame)
{
	size_t i;

	for (i = 0; *cnt; i++) {
		int r;

		(*cnt)--;

		/* push to the frame (and update its state) */
		r = il_eusb_frame__push(frame, rbuf[i]);
		if (r < 0) {
			/* likely garbage, reset keeping current */
			il_eusb_frame__reset(frame);
			(void)il_eusb_frame__push(frame, rbuf[i]);

			continue;
		}

		/* validate */
		if (frame->state == IL_EUSB_FRAME_STATE_COMPLETE) {
			if (il_eusb_frame__is_resp(frame)) {
				process_statusword(this, frame);
				process_emcy(this, frame);
				process_sync(this, frame);
			}

			il_eusb_frame__reset(frame);
		}
	}
}

/**
 * Listener thread.
 *
 * @param [in] args
 *	E-USB Network (il_eusb_net_t *).
 */
int listener(void *args)
{
	il_eusb_net_t *this = args;

	il_eusb_frame_t frame = IL_EUSB_FRAME_INIT_DEF;

	uint8_t rbuf[IL_EUSB_FRAME_MAX_SZ];
	size_t rbuf_cnt = 0;

	while (!this->stop) {
		int r;
		size_t rbuf_free, rbuf_added;

		/* process current buffer content */
		process_rbuf(this, rbuf, &rbuf_cnt, &frame);

		/* read more bytes */
		rbuf_free = sizeof(rbuf) - rbuf_cnt;

		r = ser_read(this->ser, &rbuf[rbuf_cnt], rbuf_free,
			     &rbuf_added);
		if (r == SER_EEMPTY) {
			r = ser_read_wait(this->ser);
			if (r == SER_ETIMEDOUT)
				continue;
			else if (r < 0)
				goto err;
		} else if ((r < 0) || ((r == 0) && (rbuf_added == 0))) {
			goto err;
		} else {
			rbuf_cnt += rbuf_added;
		}
	}

	ser_close(this->ser);

	return 0;

err:
	ser_close(this->ser);
	il_net__state_set(&this->net, IL_NET_STATE_FAULTY);

	return IL_EFAIL;
}

/**
 * Monitor event callback.
 */
void on_ser_evt(void *ctx, ser_dev_evt_t evt, const ser_dev_t *dev)
{
	il_eusb_net_dev_mon_t *this = to_eusb_mon(ctx);

	if (evt == SER_DEV_EVT_ADDED)
		this->on_evt(this->ctx, IL_NET_DEV_EVT_ADDED, dev->path);
	else
		this->on_evt(this->ctx, IL_NET_DEV_EVT_REMOVED, dev->path);
}

/**
 * Destroy network.
 *
 * @param [in] ctx
 *	Context (il_net_t *).
 */
static void enet_destroy(void *ctx)
{
	il_eusb_net_t *this = ctx;

	if (il_net_state_get(&this->net) != IL_NET_STATE_DISCONNECTED) {
		this->stop = 1;
		osal_thread_join(this->listener, NULL);
	}

	osal_cond_destroy(this->sync.cond);
	osal_mutex_destroy(this->sync.lock);

	ser_destroy(this->ser);

	il_net_base__deinit(&this->net);

	free(this);
}

/**
 * Read (non-threadsafe).
 *
 * @param [in] this
 *	E-USB Network.
 * @param [in] id
 *	Expected node id (0 to match any).
 * @param [in] address
 *	Expected address.
 * @param [out] buf
 *	Data output buffer.
 * @param [in] sz
 *	Data buffer size.
 *
 * @returns
 *	0 on success, error code otherwise.
 */
static int net_read(il_eusb_net_t *this, uint8_t id, uint32_t address,
		    void *buf, size_t sz)
{
	int r;
	il_eusb_frame_t frame;

	/* register synchronous transfer */
	osal_mutex_lock(this->sync.lock);

	this->sync.id = id;
	this->sync.address = address;
	this->sync.buf = buf;
	this->sync.sz = sz;
	this->sync.complete = 0;

	/* send synchronous read petition */
	il_eusb_frame__init(&frame, id, address, NULL, 0);

	r = ser_write(this->ser, frame.buf, frame.sz, NULL);
	if (r < 0) {
		ilerr__ser(r);
		goto unlock;
	}

	osal_mutex_unlock(this->sync.lock);

	/* wait for response */
	osal_mutex_lock(this->sync.lock);

	if (!this->sync.complete) {
		r = osal_cond_wait(this->sync.cond, this->sync.lock,
				   this->net.timeout_rd);
		if (r == OSAL_ETIMEDOUT) {
			ilerr__set("Reception timed out");
			r = IL_ETIMEDOUT;
		} else if (r < 0) {
			ilerr__set("Reception failed");
			r = IL_EFAIL;
		}
	} else {
		r = 0;
	}

unlock:
	this->sync.complete = 1;

	osal_mutex_unlock(this->sync.lock);

	return r;
}

/*******************************************************************************
 * Implementation: Internal
 ******************************************************************************/

static void il_eusb_net__retain(il_net_t *net)
{
	il_eusb_net_t *this = to_eusb_net(net);

	il_utils__refcnt_retain(this->refcnt);
}

static void il_eusb_net__release(il_net_t *net)
{
	il_eusb_net_t *this = to_eusb_net(net);

	il_utils__refcnt_release(this->refcnt);
}

static int il_eusb_net__read(il_net_t *net, uint16_t id, uint32_t address,
			     void *buf, size_t sz)
{
	il_eusb_net_t *this = to_eusb_net(net);

	int r;

	if (il_net_state_get(&this->net) != IL_NET_STATE_CONNECTED) {
		ilerr__set("Network is not connected");
		return IL_ESTATE;
	}

	osal_mutex_lock(this->net.lock);

	r = net_read(this, (uint8_t)id, address, buf, sz);

	osal_mutex_unlock(this->net.lock);

	return r;
}

static int il_eusb_net__write(il_net_t *net, uint16_t id, uint32_t address,
			      const void *buf, size_t sz, int confirmed)
{
	il_eusb_net_t *this = to_eusb_net(net);

	int r;
	il_eusb_frame_t frame;

	if (il_net_state_get(&this->net) != IL_NET_STATE_CONNECTED) {
		ilerr__set("Network is not connected");
		return IL_ESTATE;
	}

	osal_mutex_lock(this->net.lock);

	/* write */
	il_eusb_frame__init(&frame, (uint8_t)id, address, buf, sz);

	r = ser_write(this->ser, frame.buf, frame.sz, NULL);
	if (r < 0) {
		ilerr__ser(r);
		goto unlock;
	}

	/* read back if confirmed */
	if (confirmed) {
		void *buf_;

		buf_ = malloc(sz);
		if (!buf_) {
			ilerr__set("Buffer allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		r = net_read(this, (uint8_t)id, address, buf_, sz);
		if (r == 0) {
			if (memcmp(buf, buf_, sz) != 0) {
				ilerr__set("Write failed (content mismatch)");
				r = IL_EIO;
			}
		}

		free(buf_);
	}

unlock:
	osal_mutex_unlock(this->net.lock);

	return r;
}

/*******************************************************************************
 * Implementation: Public
 ******************************************************************************/

static il_net_t *il_eusb_net_create(const il_net_opts_t *opts)
{
	il_eusb_net_t *this;
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

	this->net.ops = &il_eusb_net_ops;
	this->net.prot = IL_NET_PROT_EUSB;

	/* setup refcnt */
	this->refcnt = il_utils__refcnt_create(enet_destroy, this);
	if (!this->refcnt)
		goto cleanup_net;

	/* initialize synchronous transfers context */
	this->sync.lock = osal_mutex_create();
	if (!this->sync.lock) {
		ilerr__set("Network sync lock allocation failed");
		goto cleanup_refcnt;
	}

	this->sync.cond = osal_cond_create();
	if (!this->sync.cond) {
		ilerr__set("Network sync condition allocation failed");
		goto cleanup_sync_lock;
	}

	this->sync.complete = 1;

	/* allocate serial port */
	this->ser = ser_create();
	if (!this->ser) {
		ilerr__set("Serial port allocation failed (%s)",
			   sererr_last());
		goto cleanup_sync_cond;
	}

	/* connect */
	this->sopts.port = il_net_port_get(&this->net);
	this->sopts.baudrate = BAUDRATE_DEF;
	this->sopts.timeouts.rd = SER_POLL_TIMEOUT;
	this->sopts.timeouts.wr = opts->timeout_wr;

	r = il_net_connect(&this->net);
	if (r < 0)
		goto cleanup_ser;

	return &this->net;

cleanup_ser:
	ser_destroy(this->ser);

cleanup_sync_cond:
	osal_cond_destroy(this->sync.cond);

cleanup_sync_lock:
	osal_mutex_destroy(this->sync.lock);

cleanup_refcnt:
	il_utils__refcnt_destroy(this->refcnt);

cleanup_net:
	il_net_base__deinit(&this->net);

cleanup_this:
	free(this);

	return NULL;
}

static void il_eusb_net_destroy(il_net_t *net)
{
	il_eusb_net_t *this = to_eusb_net(net);

	il_utils__refcnt_release(this->refcnt);
}

static int il_eusb_net_connect(il_net_t *net)
{
	int r, i;
	uint8_t val;
	il_net_state_t state;

	il_eusb_net_t *this = to_eusb_net(net);

	/* check state, proceed only if not connected */
	state = il_net_state_get(&this->net);
	if (state == IL_NET_STATE_CONNECTED) {
		ilerr__set("Network already connected");
		return IL_EALREADY;
	} else if (state == IL_NET_STATE_FAULTY) {
		/* free resources if faulty */
		this->stop = 1;
		osal_thread_join(this->listener, NULL);

		il_net__state_set(&this->net, IL_NET_STATE_DISCONNECTED);
	}

	/* open port */
	r = ser_open(this->ser, &this->sopts);
	if (r < 0) {
		ilerr__set("Serial port open failed (%s)", sererr_last());
		return IL_EFAIL;
	}

	/* QUIRK: drive may not be operative immediately */
	osal_clock_sleep_ms(INIT_WAIT_TIME);

	/* connect (so that binary can be enabled) */
	il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);

	/* send ascii message to force binary */
	r = ser_write(this->ser, MSG_A2B, sizeof(MSG_A2B) - 1, NULL);
	if (r < 0) {
		ilerr__set("Binary configuration failed (%s)", sererr_last());
		goto close_ser;
	}

	/* send the same message twice in binary (will flush) */
	val = 1;
	for (i = 0; i < BIN_FLUSH; i++) {
		r = il_eusb_net__write(&this->net, 0, UARTCFG_BIN_ADDRESS, &val,
				       sizeof(val), 0);
		if (r < 0)
			goto close_ser;
	}

	/* start listener thread */
	this->stop = 0;

	this->listener = osal_thread_create(listener, this);
	if (!this->listener) {
		ilerr__set("Listener thread creation failed");
		goto close_ser;
	}

	return 0;

close_ser:
	ser_close(this->ser);
	il_net__state_set(&this->net, IL_NET_STATE_DISCONNECTED);

	return IL_EFAIL;
}

static void il_eusb_net_disconnect(il_net_t *net)
{
	il_eusb_net_t *this = to_eusb_net(net);

	if (il_net_state_get(&this->net) != IL_NET_STATE_DISCONNECTED) {
		this->stop = 1;
		osal_thread_join(this->listener, NULL);

		il_net__state_set(&this->net, IL_NET_STATE_DISCONNECTED);
	}
}

static il_net_servos_list_t *il_eusb_net_servos_list_get(
	il_net_t *net, il_net_servos_on_found_t on_found, void *ctx)
{
	il_eusb_net_t *this = to_eusb_net(net);

	int r;
	uint8_t id;
	il_eusb_frame_t frame;

	il_net_servos_list_t *lst = NULL;
	il_net_servos_list_t *prev;

	/* check network state */
	if (il_net_state_get(net) != IL_NET_STATE_CONNECTED) {
		ilerr__set("Network is not connected");
		return NULL;
	}

	osal_mutex_lock(this->net.lock);

	/* register synchronous transfer */
	osal_mutex_lock(this->sync.lock);

	this->sync.id = 0;
	this->sync.address = UARTCFG_ID_ADDRESS;
	this->sync.buf = &id;
	this->sync.sz = sizeof(id);
	this->sync.complete = 0;

	il_eusb_frame__init(&frame, 0, UARTCFG_ID_ADDRESS, NULL, 0);

	/* QUIRK: ignore first run, as on cold-boot firmware may issue
	 * improperly formatted binary messages, leading to no servos found.
	 */
	r = ser_write(this->ser, frame.buf, frame.sz, NULL);
	if (r < 0) {
		ilerr__ser(r);
		goto sync_unlock;
	}

	osal_mutex_unlock(this->sync.lock);
	osal_mutex_lock(this->sync.lock);

	while (r == 0) {
		if (this->sync.complete)
			this->sync.complete = 0;
		else
			r = osal_cond_wait(this->sync.cond, this->sync.lock,
					   SCAN_TIMEOUT);
	}

	/* second try */
	this->sync.complete = 0;

	r = ser_write(this->ser, frame.buf, frame.sz, NULL);
	if (r < 0) {
		ilerr__ser(r);
		goto sync_unlock;
	}

	osal_mutex_unlock(this->sync.lock);
	osal_mutex_lock(this->sync.lock);

	while (r == 0) {
		if (this->sync.complete) {
			this->sync.complete = 0;

			/* allocate new list entry */
			prev = lst;
			lst = malloc(sizeof(*lst));
			if (!lst) {
				il_net_servos_list_destroy(prev);
				break;
			}

			lst->next = prev;
			lst->id = id;

			if (on_found)
				on_found(ctx, id);
		} else {
			r = osal_cond_wait(this->sync.cond, this->sync.lock,
					   SCAN_TIMEOUT);
		}
	}

sync_unlock:
	osal_mutex_unlock(this->sync.lock);

	osal_mutex_unlock(this->net.lock);

	return lst;
}

static il_net_dev_mon_t *il_eusb_net_dev_mon_create(void)
{
	il_eusb_net_dev_mon_t *this;

	/* allocate monitor */
	this = malloc(sizeof(*this));
	if (!this) {
		ilerr__set("Monitor allocation failed");
		return NULL;
	}

	this->mon.ops = &il_eusb_net_dev_mon_ops;
	this->running = 0;

	return &this->mon;
}

static void il_eusb_net_dev_mon_destroy(il_net_dev_mon_t *mon)
{
	il_eusb_net_dev_mon_t *this = to_eusb_mon(mon);

	il_net_dev_mon_stop(mon);

	free(this);
}

static int il_eusb_net_dev_mon_start(
	il_net_dev_mon_t *mon, il_net_dev_on_evt_t on_evt, void *ctx)
{
	il_eusb_net_dev_mon_t *this = to_eusb_mon(mon);

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

static void il_eusb_net_dev_mon_stop(il_net_dev_mon_t *mon)
{
	il_eusb_net_dev_mon_t *this = to_eusb_mon(mon);

	if (this->running) {
		ser_dev_monitor_stop(this->smon);
		this->running = 0;
	}
}

il_net_dev_list_t *il_eusb_net_dev_list_get()
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
			goto out;
		}

		lst->next = prev;

		/* store port */
		strncpy(lst->port, ser_dev->dev.path, sizeof(lst->port));
	}

out:
	ser_dev_list_destroy(ser_devs);

	return lst;
}

/** E-USB network operations. */
const il_net_ops_t il_eusb_net_ops = {
	/* internal */
	._retain = il_eusb_net__retain,
	._release = il_eusb_net__release,
	._state_set = il_net_base__state_set,
	._read = il_eusb_net__read,
	._write = il_eusb_net__write,
	._sw_subscribe = il_net_base__sw_subscribe,
	._sw_unsubscribe = il_net_base__sw_unsubscribe,
	._emcy_subscribe = il_net_base__emcy_subscribe,
	._emcy_unsubscribe = il_net_base__emcy_unsubscribe,
	/* public */
	.create = il_eusb_net_create,
	.destroy = il_eusb_net_destroy,
	.connect = il_eusb_net_connect,
	.disconnect = il_eusb_net_disconnect,
	.state_get = il_net_base__state_get,
	.servos_list_get = il_eusb_net_servos_list_get,
};

/** E-USB network device monitor operations. */
const il_net_dev_mon_ops_t il_eusb_net_dev_mon_ops = {
	.create = il_eusb_net_dev_mon_create,
	.destroy = il_eusb_net_dev_mon_destroy,
	.start = il_eusb_net_dev_mon_start,
	.stop = il_eusb_net_dev_mon_stop,
};
