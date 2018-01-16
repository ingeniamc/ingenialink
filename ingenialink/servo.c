/*
 * MIT License
 *
 * Copyright (c) 2017 Ingenia-CAT S.L.
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
#include "mc.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "public/ingenialink/const.h"
#include "public/ingenialink/dict.h"
#include "ingenialink/err.h"
#include "ingenialink/utils.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

/**
 * Obtain register (pre-defined or from dictionary).
 *
 * @param [in] dict
 *	Dictionary.
 * @param [in] reg_pdef
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [out] reg
 *	Where register will be stored.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int get_reg(il_dict_t *dict, const il_reg_t *reg_pdef,
		   const char *id, const il_reg_t **reg)
{
	int r;

	/* obtain register (predefined or from dictionary) */
	if (reg_pdef) {
		*reg = reg_pdef;
	} else {
		if (!dict) {
			ilerr__set("No dictionary loaded");
			return IL_EFAIL;
		}

		r = il_dict_reg_get(dict, id, reg);
		if (r < 0)
			return r;
	}

	return 0;
}

/**
 * Raw read.
 *
 * @param [in] servo
 *	Servo.
 * @param [in] reg_pdef
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] dtype
 *	Expected data type.
 * @param [out] buf
 *	Where data will be stored.
 * @param [in] sz
 *	Buffer size.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int raw_read(il_servo_t *servo, const il_reg_t *reg_pdef,
		     const char *id, il_reg_dtype_t dtype, void *buf, size_t sz)
{
	int r;
	const il_reg_t *reg;

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg_pdef, id, &reg);
	if (r < 0)
		return r;

	/* verify register properties */
	if ((dtype != IL_REG_DTYPE_RAW) && (reg->dtype != dtype)) {
		ilerr__set("Unexpected register data type");
		return IL_EINVAL;
	}

	if (reg->access == IL_REG_ACCESS_WO) {
		ilerr__set("Register is write-only");
		return IL_EACCESS;
	}

	return il_net__read(servo->net, servo->id, reg->address, buf, sz,
			    servo->timeout);
}

/**
 * Raw write.
 *
 * @param [in] servo
 *	Servo.
 * @param [in] reg_pdef
 *	Pre-defined register.
 * @param [in] id
 *	Register ID.
 * @param [in] dtype
 *	Expected data type.
 * @param [in] data
 *	Data buffer.
 * @param [in] sz
 *	Data buffer size.
 * @param [in] confirmed
 *	Confirm write.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int raw_write(il_servo_t *servo, const il_reg_t *reg_pdef,
		     const char *id, il_reg_dtype_t dtype, const void *data,
		     size_t sz, int confirmed)
{
	int r, confirmed_;
	const il_reg_t *reg;

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg_pdef, id, &reg);
	if (r < 0)
		return r;

	/* verify register properties */
	if ((dtype != IL_REG_DTYPE_RAW) && (reg->dtype != dtype)) {
		ilerr__set("Unexpected register data type");
		return IL_EINVAL;
	}

	if (reg->access == IL_REG_ACCESS_RO) {
		ilerr__set("Register is read-only");
		return IL_EACCESS;
	}

	/* skip confirmation on write-only registers */
	confirmed_ = (reg->access == IL_REG_ACCESS_WO) ? 0 : confirmed;

	return il_net__write(servo->net, servo->id, reg->address, data, sz,
			     confirmed_, servo->timeout);
}

/**
 * Decode the PDS state.
 *
 * @param [in] sw
 *	Statusword value.
 *
 * @return
 *	PDS state (IL_SERVO_STATE_NRDY if unknown).
 */
static il_servo_state_t pds_state_decode(uint16_t sw)
{
	if ((sw & IL_MC_PDS_STA_NRTSO_MSK) == IL_MC_PDS_STA_NRTSO)
		return IL_SERVO_STATE_NRDY;
	else if ((sw & IL_MC_PDS_STA_SOD_MSK) == IL_MC_PDS_STA_SOD)
		return IL_SERVO_STATE_DISABLED;
	else if ((sw & IL_MC_PDS_STA_RTSO_MSK) == IL_MC_PDS_STA_RTSO)
		return IL_SERVO_STATE_RDY;
	else if ((sw & IL_MC_PDS_STA_SO_MSK) == IL_MC_PDS_STA_SO)
		return IL_SERVO_STATE_ON;
	else if ((sw & IL_MC_PDS_STA_OE_MSK) == IL_MC_PDS_STA_OE)
		return IL_SERVO_STATE_ENABLED;
	else if ((sw & IL_MC_PDS_STA_QSA_MSK) == IL_MC_PDS_STA_QSA)
		return IL_SERVO_STATE_QSTOP;
	else if ((sw & IL_MC_PDS_STA_FRA_MSK) == IL_MC_PDS_STA_FRA)
		return IL_SERVO_STATE_FAULTR;
	else if ((sw & IL_MC_PDS_STA_F_MSK) == IL_MC_PDS_STA_F)
		return IL_SERVO_STATE_FAULT;

	return IL_SERVO_STATE_NRDY;
}

/**
 * Statusword update callback
 *
 * @param [in] ctx
 *	Context (servo_t *).
 * @param [in] sw
 *	Statusword value.
 */
static void sw_update(void *ctx, uint16_t sw)
{
	il_servo_t *servo = ctx;

	osal_mutex_lock(servo->sw.lock);

	if (servo->sw.value != sw) {
		servo->sw.value = sw;
		osal_cond_broadcast(servo->sw.changed);
	}

	osal_mutex_unlock(servo->sw.lock);
}

/**
 * Obtain the current statusword value.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 *
 * @return
 *	Statusword value.
 */
static uint16_t sw_get(il_servo_t *servo)
{
	uint16_t sw;

	osal_mutex_lock(servo->sw.lock);
	sw = servo->sw.value;
	osal_mutex_unlock(servo->sw.lock);

	return sw;
}

/**
 * Wait until the statusword changes its value.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in, out] sw
 *	Current statusword value, where next value will be stored.
 * @param [in, out] timeout
 *	Timeout (ms), if positive will be updated with remaining ms.
 *
 * @return
 *	0 on success, error code otherwise.
 */
static int sw_wait_change(il_servo_t *servo, uint16_t *sw, int *timeout)
{
	int r = 0;
	osal_timespec_t start = { 0, 0 }, end, diff;

	/* obtain start time */
	if (*timeout > 0) {
		if (osal_clock_gettime(&start) < 0) {
			ilerr__set("Could not obtain system time");
			return IL_EFAIL;
		}
	}

	/* wait for change */
	osal_mutex_lock(servo->sw.lock);

	if (servo->sw.value == *sw) {
		r = osal_cond_wait(servo->sw.changed, servo->sw.lock, *timeout);
		if (r == OSAL_ETIMEDOUT) {
			ilerr__set("Operation timed out");
			r = IL_ETIMEDOUT;
			goto out;
		} else if (r < 0) {
			ilerr__set("Statusword wait change failed");
			r = IL_EFAIL;
			goto out;
		}
	}

	*sw = servo->sw.value;

out:
	/* update timeout */
	if ((*timeout > 0) && (r == 0)) {
		/* obtain end time */
		if (osal_clock_gettime(&end) < 0) {
			ilerr__set("Could not obtain system time");
			r = IL_EFAIL;
			goto unlock;
		}

		/* compute difference */
		if ((end.ns - start.ns) < 0) {
			diff.s = end.s - start.s - 1;
			diff.ns = end.ns - start.ns + OSAL_CLOCK_NANOSPERSEC;
		} else {
			diff.s = end.s - start.s;
			diff.ns = end.ns - start.ns;
		}

		/* update timeout */
		*timeout -= diff.s * 1000 + diff.ns / OSAL_CLOCK_NANOSPERMSEC;
		if (*timeout <= 0) {
			ilerr__set("Operation timed out");
			r = IL_ETIMEDOUT;
		}
	}

unlock:
	osal_mutex_unlock(servo->sw.lock);

	return r;
}

/**
 * Wait until the statusword has the requested value
 *
 * @note
 *	The timeout is not an absolute timeout, but an interval timeout.
 *
 * @param [in] servo
 *	IngeniaLink servo.
 * @param [in] msk
 *	Statusword mask.
 * @param [in] val
 *	Statusword value.
 * @param [in] timeout
 *	Timeout (ms).
 */
static int sw_wait_value(il_servo_t *servo, uint16_t msk, uint16_t val,
			 int timeout)
{
	int r = 0;
	uint16_t result;

	/* wait until the flag changes to the requested state */
	osal_mutex_lock(servo->sw.lock);

	do {
		result = servo->sw.value & msk;
		if (result != val) {
			r = osal_cond_wait(servo->sw.changed, servo->sw.lock,
					   timeout);
			if (r == OSAL_ETIMEDOUT) {
				ilerr__set("Operation timed out");
				r = IL_ETIMEDOUT;
			} else if (r < 0) {
				ilerr__set("Statusword wait change failed");
				r = IL_EFAIL;
			}
		}
	} while ((result != val) && (r == 0));

	osal_mutex_unlock(servo->sw.lock);

	return r;
}

/**
 * State change monitor, used to push state changes to external subscriptors.
 *
 * @param [in] args
 *	Arguments (il_servo_t *).
 */
static int state_subs_monitor(void *args)
{
	il_servo_t *servo = args;
	uint16_t sw;

	sw = sw_get(servo);

	while (!servo->state_subs.stop) {
		int timeout;
		size_t i;
		il_servo_state_t state;
		int flags;

		/* wait for change */
		timeout = STATE_SUBS_TIMEOUT;
		if (sw_wait_change(servo, &sw, &timeout) < 0)
			continue;

		/* obtain state/flags */
		state = pds_state_decode(sw);
		flags = (int)(sw >> FLAGS_SW_POS);

		/* notify all subscribers */
		osal_mutex_lock(servo->state_subs.lock);

		for (i = 0; i < servo->state_subs.sz; i++) {
			void *ctx;

			if (!servo->state_subs.subs[i].cb)
				continue;

			ctx = servo->state_subs.subs[i].ctx;
			servo->state_subs.subs[i].cb(ctx, state, flags);
		}

		osal_mutex_unlock(servo->state_subs.lock);
	}

	return 0;
}

/**
 * Emergencies callback.
 *
 */
static void on_emcy(void *ctx, uint32_t code)
{
	il_servo_t *servo = ctx;
	il_servo_emcy_t *emcy = &servo->emcy;

	osal_mutex_lock(emcy->lock);

	/* if full, drop oldest item */
	if (!CIRC_SPACE(emcy->head, emcy->tail, EMCY_QUEUE_SZ))
		emcy->tail = (emcy->tail + 1) & (EMCY_QUEUE_SZ - 1);

	/* push emergency and notify */
	emcy->queue[emcy->head] = code;
	emcy->head = (emcy->head + 1) & (EMCY_QUEUE_SZ - 1);

	osal_cond_signal(emcy->not_empty);

	osal_mutex_unlock(emcy->lock);
}

/**
 * Emergencies monitor, used to notify about emergencies to external
 * subscriptors.
 *
 * @param [in] args
 *	Arguments (il_servo_t *).
 */
static int emcy_subs_monitor(void *args)
{
	il_servo_t *servo = args;
	il_servo_emcy_t *emcy = &servo->emcy;
	il_servo_emcy_subscriber_lst_t *emcy_subs = &servo->emcy_subs;

	while (!emcy_subs->stop) {
		int r;

		/* wait until emcy queue not empty */
		osal_mutex_lock(emcy->lock);

		r = osal_cond_wait(emcy->not_empty, emcy->lock,
				   EMCY_SUBS_TIMEOUT);

		if (r == 0) {
			/* process all available emergencies */
			while (CIRC_CNT(emcy->head, emcy->tail, emcy->sz)) {
				size_t i;
				uint32_t code;

				code = emcy->queue[emcy->tail];
				emcy->tail = (emcy->tail + 1) & (emcy->sz - 1);

				osal_mutex_unlock(emcy->lock);

				/* notify all subscribers */
				osal_mutex_lock(emcy_subs->lock);

				for (i = 0; i < emcy_subs->sz; i++) {
					void *ctx;

					if (!emcy_subs->subs[i].cb)
						continue;

					ctx = emcy_subs->subs[i].ctx;
					emcy_subs->subs[i].cb(ctx, code);
				}

				osal_mutex_unlock(emcy_subs->lock);
				osal_mutex_lock(emcy->lock);
			}
		}

		osal_mutex_unlock(emcy->lock);
	}

	return 0;
}

/**
 * Destroy servo instance.
 *
 * @param [in] ctx
 *	Context (il_servo_t *).
 */
static void servo_destroy(void *ctx)
{
	il_servo_t *servo = ctx;

	servo->emcy_subs.stop = 1;
	(void)osal_thread_join(servo->emcy_subs.monitor, NULL);
	osal_mutex_destroy(servo->emcy_subs.lock);
	free(servo->emcy_subs.subs);

	il_net__emcy_unsubscribe(servo->net, servo->emcy.slot);
	osal_cond_destroy(servo->emcy.not_empty);
	osal_mutex_destroy(servo->emcy.lock);

	servo->state_subs.stop = 1;
	(void)osal_thread_join(servo->state_subs.monitor, NULL);
	osal_mutex_destroy(servo->state_subs.lock);
	free(servo->state_subs.subs);

	il_net__sw_unsubscribe(servo->net, servo->sw.slot);
	osal_cond_destroy(servo->sw.changed);
	osal_mutex_destroy(servo->sw.lock);

	osal_mutex_destroy(servo->units.lock);

	if (servo->dict)
		il_dict_destroy(servo->dict);

	il_net__release(servo->net);

	free(servo);
}

/*******************************************************************************
 * Internal
 ******************************************************************************/

void il_servo__retain(il_servo_t *servo)
{
	il_utils__refcnt_retain(servo->refcnt);
}

void il_servo__release(il_servo_t *servo)
{
	il_utils__refcnt_release(servo->refcnt);
}

/*******************************************************************************
 * Public
 ******************************************************************************/

il_servo_t *il_servo_create(il_net_t *net, uint8_t id, const char *dict,
			    int timeout)
{
	int r;

	il_servo_t *servo;
	uint16_t sw;

	assert(net);

	/* validate node id */
	if ((id < SERVOID_MIN) || (id > SERVOID_MAX)) {
		ilerr__set("Servo id out of range");
		return NULL;
	}

	/* allocate servo */
	servo = malloc(sizeof(*servo));
	if (!servo) {
		ilerr__set("Servo allocation failed");
		return NULL;
	}

	/* load dictionary (optional) */
	if (dict) {
		servo->dict = il_dict_create(dict);
		if (!servo->dict)
			goto cleanup_servo;
	} else {
		servo->dict = NULL;
	}

	/* initialize, setup refcnt */
	servo->net = net;
	il_net__retain(servo->net);
	servo->id = id;
	servo->timeout = timeout;

	servo->refcnt = il_utils__refcnt_create(servo_destroy, servo);
	if (!servo->refcnt)
		goto cleanup_dict;


	/* obtain current operation mode */
	r = il_servo_mode_get(servo, &servo->mode);
	if (r < 0)
		goto cleanup_refcnt;

	/* configure units */
	servo->units.lock = osal_mutex_create();
	if (!servo->units.lock) {
		ilerr__set("Units lock allocation failed");
		goto cleanup_refcnt;
	}

	r = il_servo_units_update(servo);
	if (r < 0)
		goto cleanup_units_lock;

	servo->units.torque = IL_UNITS_TORQUE_NATIVE;
	servo->units.pos = IL_UNITS_POS_NATIVE;
	servo->units.vel = IL_UNITS_VEL_NATIVE;
	servo->units.acc = IL_UNITS_ACC_NATIVE;

	/* configure statusword subscription */
	servo->sw.lock = osal_mutex_create();
	if (!servo->sw.lock) {
		ilerr__set("Statusword subscriber lock allocation failed");
		goto cleanup_units_lock;
	}

	servo->sw.changed = osal_cond_create();
	if (!servo->sw.changed) {
		ilerr__set("Statusword subscriber condition allocation failed");
		goto cleanup_sw_lock;
	}

	servo->sw.value = 0;

	r = il_net__sw_subscribe(servo->net, servo->id, sw_update, servo);
	if (r < 0)
		goto cleanup_sw_changed;

	servo->sw.slot = r;

	/* trigger update (with manual read) */
	(void)il_servo_raw_read_u16(servo, &IL_REG_STS_WORD, NULL, &sw);

	/* configute external state subscriptors */
	servo->state_subs.subs = calloc(STATE_SUBS_SZ_DEF,
					sizeof(*servo->state_subs.subs));
	if (!servo->state_subs.subs) {
		ilerr__set("State subscribers allocation failed");
		goto cleanup_sw_subscribe;
	}

	servo->state_subs.sz = STATE_SUBS_SZ_DEF;

	servo->state_subs.lock = osal_mutex_create();
	if (!servo->state_subs.lock) {
		ilerr__set("State subscription lock allocation failed");
		goto cleanup_state_subs_subs;
	}

	servo->state_subs.stop = 0;

	servo->state_subs.monitor = osal_thread_create(state_subs_monitor,
						       servo);
	if (!servo->state_subs.monitor) {
		ilerr__set("State change monitor could not be created");
		goto cleanup_state_subs_lock;
	}

	/* configure emergency subscription */
	servo->emcy.lock = osal_mutex_create();
	if (!servo->emcy.lock) {
		ilerr__set("Emergency subscriber lock allocation failed");
		goto cleanup_state_subs_monitor;
	}

	servo->emcy.not_empty = osal_cond_create();
	if (!servo->emcy.not_empty) {
		ilerr__set("Emergency subscriber condition allocation failed");
		goto cleanup_emcy_lock;
	}

	servo->emcy.head = 0;
	servo->emcy.tail = 0;
	servo->emcy.sz = EMCY_QUEUE_SZ;

	r = il_net__emcy_subscribe(servo->net, servo->id, on_emcy, servo);
	if (r < 0)
		goto cleanup_emcy_not_empty;

	servo->emcy.slot = r;

	/* configure external emergency subscriptors */
	servo->emcy_subs.subs = calloc(EMCY_SUBS_SZ_DEF,
				       sizeof(*servo->emcy_subs.subs));
	if (!servo->emcy_subs.subs) {
		ilerr__set("Emergency subscribers allocation failed");
		goto cleanup_emcy_subscribe;
	}

	servo->emcy_subs.sz = EMCY_SUBS_SZ_DEF;

	servo->emcy_subs.lock = osal_mutex_create();
	if (!servo->emcy_subs.lock) {
		ilerr__set("Emergency subscription lock allocation failed");
		goto cleanup_emcy_subs_subs;
	}

	servo->emcy_subs.stop = 0;
	servo->emcy_subs.monitor = osal_thread_create(emcy_subs_monitor, servo);
	if (!servo->emcy_subs.monitor) {
		ilerr__set("Emergency monitor could not be created");
		goto cleanup_emcy_subs_lock;
	}

	return servo;

cleanup_emcy_subs_lock:
	osal_mutex_destroy(servo->emcy_subs.lock);

cleanup_emcy_subs_subs:
	free(servo->emcy_subs.subs);

cleanup_emcy_subscribe:
	il_net__emcy_unsubscribe(servo->net, servo->emcy.slot);

cleanup_emcy_not_empty:
	osal_cond_destroy(servo->emcy.not_empty);

cleanup_emcy_lock:
	osal_mutex_destroy(servo->emcy.lock);

cleanup_state_subs_monitor:
	servo->state_subs.stop = 1;
	(void)osal_thread_join(servo->state_subs.monitor, NULL);

cleanup_state_subs_lock:
	osal_mutex_destroy(servo->state_subs.lock);

cleanup_state_subs_subs:
	free(servo->state_subs.subs);

cleanup_sw_subscribe:
	il_net__sw_unsubscribe(servo->net, servo->sw.slot);

cleanup_sw_changed:
	osal_cond_destroy(servo->sw.changed);

cleanup_sw_lock:
	osal_mutex_destroy(servo->sw.lock);

cleanup_units_lock:
	osal_mutex_destroy(servo->units.lock);

cleanup_refcnt:
	il_utils__refcnt_destroy(servo->refcnt);

cleanup_dict:
	if (servo->dict)
		il_dict_destroy(servo->dict);

cleanup_servo:
	il_net__release(servo->net);
	free(servo);

	return NULL;
}

void il_servo_destroy(il_servo_t *servo)
{
	assert(servo);

	il_utils__refcnt_release(servo->refcnt);
}

int il_servo_lucky(il_net_t **net, il_servo_t **servo, const char *dict)
{
	il_net_dev_list_t *devs, *dev;
	il_net_servos_list_t *servo_ids, *servo_id;

	assert(net);
	assert(servo);

	/* scan all available network devices */
	devs = il_net_dev_list_get();
	il_net_dev_list_foreach(dev, devs) {
		*net = il_net_create(dev->port);
		if (!*net)
			continue;

		/* try to connect to any available servo */
		servo_ids = il_net_servos_list_get(*net, NULL, NULL);
		il_net_servos_list_foreach(servo_id, servo_ids) {
			*servo = il_servo_create(*net, servo_id->id, dict,
						 IL_SERVO_TIMEOUT_DEF);
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

void il_servo_state_get(il_servo_t *servo, il_servo_state_t *state, int *flags)
{
	uint16_t sw;

	assert(servo);
	assert(state);
	assert(flags);

	sw = sw_get(servo);

	*state = pds_state_decode(sw);
	*flags = (int)(sw >> FLAGS_SW_POS);
}

int il_servo_state_subscribe(il_servo_t *servo,
			     il_servo_state_subscriber_cb_t cb, void *ctx)
{
	int r = 0;
	int slot;

	assert(servo);
	assert(cb);

	osal_mutex_lock(servo->state_subs.lock);

	/* look for the first empty slot */
	for (slot = 0; slot < (int)servo->state_subs.sz; slot++) {
		if (!servo->state_subs.subs[slot].cb)
			break;
	}

	/* increase array if no space left */
	if (slot == (int)servo->state_subs.sz) {
		size_t sz;
		il_servo_state_subscriber_t *subs;

		/* double in size on each realloc */
		sz = 2 * servo->state_subs.sz * sizeof(*subs);
		subs = realloc(servo->state_subs.subs, sz);
		if (!subs) {
			ilerr__set("Subscribers re-allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		servo->state_subs.subs = subs;
		servo->state_subs.sz = sz;
	}

	servo->state_subs.subs[slot].cb = cb;
	servo->state_subs.subs[slot].ctx = ctx;

	r = slot;

unlock:
	osal_mutex_unlock(servo->state_subs.lock);

	return r;
}

void il_servo_state_unsubscribe(il_servo_t *servo, int slot)
{
	assert(servo);

	osal_mutex_lock(servo->state_subs.lock);

	/* skip out of range slot */
	if (slot >= (int)servo->state_subs.sz)
		return;

	servo->state_subs.subs[slot].cb = NULL;
	servo->state_subs.subs[slot].ctx = NULL;

	osal_mutex_unlock(servo->state_subs.lock);
}

int il_servo_emcy_subscribe(il_servo_t *servo, il_servo_emcy_subscriber_cb_t cb,
			    void *ctx)
{
	int r = 0;
	int slot;

	assert(servo);
	assert(cb);

	osal_mutex_lock(servo->emcy_subs.lock);

	/* look for the first empty slot */
	for (slot = 0; slot < (int)servo->emcy_subs.sz; slot++) {
		if (!servo->emcy_subs.subs[slot].cb)
			break;
	}

	/* increase array if no space left */
	if (slot == (int)servo->emcy_subs.sz) {
		size_t sz;
		il_servo_emcy_subscriber_t *subs;

		/* double in size on each realloc */
		sz = 2 * servo->emcy_subs.sz * sizeof(*subs);
		subs = realloc(servo->emcy_subs.subs, sz);
		if (!subs) {
			ilerr__set("Subscribers re-allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		servo->emcy_subs.subs = subs;
		servo->emcy_subs.sz = sz;
	}

	servo->emcy_subs.subs[slot].cb = cb;
	servo->emcy_subs.subs[slot].ctx = ctx;

	r = slot;

unlock:
	osal_mutex_unlock(servo->emcy_subs.lock);

	return r;
}

void il_servo_emcy_unsubscribe(il_servo_t *servo, int slot)
{
	assert(servo);

	osal_mutex_lock(servo->emcy_subs.lock);

	/* skip out of range slot */
	if (slot >= (int)servo->emcy_subs.sz)
		return;

	servo->emcy_subs.subs[slot].cb = NULL;
	servo->emcy_subs.subs[slot].ctx = NULL;

	osal_mutex_unlock(servo->emcy_subs.lock);
}

il_dict_t *il_servo_dict_get(il_servo_t *servo)
{
	assert(servo);

	return servo->dict;
}

int il_servo_dict_load(il_servo_t *servo, const char *dict)
{
	assert(servo);

	if (servo->dict) {
		ilerr__set("Dictionary already loaded");
		return IL_EALREADY;
	}

	servo->dict = il_dict_create(dict);
	if (!servo->dict)
		return IL_EFAIL;

	return 0;
}

int il_servo_name_get(il_servo_t *servo, char *name, size_t sz)
{
	int r;

	assert(name);

	if (sz < IL_SERVO_NAME_SZ) {
		ilerr__set("Insufficient name buffer size");
		return IL_ENOMEM;
	}

	r = raw_read(servo, &IL_REG_DRIVE_NAME, NULL, IL_REG_DTYPE_RAW,
		     name, sz);
	if (r < 0)
		return r;

	name[IL_SERVO_NAME_SZ - 1] = '\0';

	return 0;
}

int il_servo_name_set(il_servo_t *servo, const char *name)
{
	size_t sz;
	char name_[IL_SERVO_NAME_SZ] = { '\0' };

	assert(name);

	/* clip name to the maximum size */
	sz = MIN(strlen(name), IL_SERVO_NAME_SZ - 1);
	memcpy(name_, name, sz);

	return raw_write(servo, &IL_REG_DRIVE_NAME, NULL, IL_REG_DTYPE_RAW,
			 name_, sizeof(name_) - 1, 1);
}

int il_servo_info_get(il_servo_t *servo, il_servo_info_t *info)
{
	int r;
	size_t i;

	assert(info);

	r = il_servo_raw_read_u32(servo, &IL_REG_ID_SERIAL, NULL,
				  &info->serial);
	if (r < 0)
		return r;

	r = il_servo_name_get(servo, info->name, sizeof(info->name));
	if (r < 0)
		return r;

	memset(info->sw_version, 0, sizeof(info->sw_version));
	r = raw_read(servo, &IL_REG_SW_VERSION, NULL, IL_REG_DTYPE_RAW,
		     info->sw_version, sizeof(info->sw_version) - 1);
	if (r < 0)
		return r;

	memset(info->hw_variant, 0, sizeof(info->hw_variant));
	r = raw_read(servo, &IL_REG_HW_VARIANT, NULL, IL_REG_DTYPE_RAW,
		     info->hw_variant, sizeof(info->hw_variant) - 1);
	if (r < 0)
		return r;

	/* FIX: hardware variant may not be present in all devices. If not
	 * present it may contain random non-printable characters, so make
	 * it null.
	 */
	for (i = 0; i < sizeof(info->hw_variant); i++) {
		if (info->hw_variant[i] != '\0' &&
		    !isprint((int)info->hw_variant[i])) {
			memset(info->hw_variant, 0, sizeof(info->hw_variant));
			break;
		}
	}

	r = il_servo_raw_read_u32(servo, &IL_REG_ID_PROD_CODE, NULL,
				  &info->prod_code);
	if (r < 0)
		return r;

	return il_servo_raw_read_u32(servo, &IL_REG_ID_REVISION, NULL,
				     &info->revision);
}

int il_servo_store_all(il_servo_t *servo)
{
	return il_servo_raw_write_u32(servo, &IL_REG_STORE_ALL, NULL,
				      ILK_SIGNATURE_STORE, 0);
}

int il_servo_store_comm(il_servo_t *servo)
{
	return il_servo_raw_write_u32(servo, &IL_REG_STORE_COMM, NULL,
				      ILK_SIGNATURE_STORE, 0);
}

int il_servo_store_app(il_servo_t *servo)
{
	return il_servo_raw_write_u32(servo, &IL_REG_STORE_APP, NULL,
				      ILK_SIGNATURE_STORE, 0);
}

int il_servo_units_update(il_servo_t *servo)
{
	int r;
	uint32_t rated_torque, pos_res, vel_res, ppitch;

	assert(servo);

	r = il_servo_raw_read_u32(servo, &IL_REG_RATED_TORQUE, NULL,
				  &rated_torque);
	if (r < 0)
		return r;

	r = il_servo_position_res_get(servo, &pos_res);
	if (r < 0)
		return r;

	r = il_servo_velocity_res_get(servo, &vel_res);
	if (r < 0)
		return r;

	r = il_servo_raw_read_u32(servo, &IL_REG_MOTPARAM_PPITCH, NULL,
				  &ppitch);
	if (r < 0)
		return r;

	servo->cfg.rated_torque = (double)rated_torque;
	servo->cfg.pos_res = (double)pos_res;
	servo->cfg.vel_res = (double)vel_res;
	servo->cfg.acc_res = servo->cfg.pos_res;
	servo->cfg.ppitch = (double)ppitch / 1000000;

	return 0;
}

double il_servo_units_factor(il_servo_t *servo, const il_reg_t *reg)
{
	double factor;

	assert(servo);
	assert(reg);

	osal_mutex_lock(servo->units.lock);

	switch (reg->phy) {
	case IL_REG_PHY_TORQUE:
		switch (servo->units.torque) {
		case IL_UNITS_TORQUE_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_TORQUE_MN:
			factor = servo->cfg.rated_torque / 1000000.;
			break;
		case IL_UNITS_TORQUE_N:
			factor = servo->cfg.rated_torque / 1000.;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_POS:
		switch (servo->units.pos) {
		case IL_UNITS_POS_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_POS_REV:
			factor = 1. / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_RAD:
			factor = 2. * M_PI / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_DEG:
			factor = 360. / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_UM:
			factor = 1000000. * servo->cfg.ppitch /
				 servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_MM:
			factor = 1000. * servo->cfg.ppitch / servo->cfg.pos_res;
			break;
		case IL_UNITS_POS_M:
			factor = 1. * servo->cfg.ppitch / servo->cfg.pos_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_VEL:
		switch (servo->units.vel) {
		case IL_UNITS_VEL_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_VEL_RPS:
			factor = 1. / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_RPM:
			factor = 60. / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_RAD_S:
			factor = 2. * M_PI / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_DEG_S:
			factor = 360. / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_UM_S:
			factor = 1000000. * servo->cfg.ppitch /
				 servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_MM_S:
			factor = 1000. * servo->cfg.ppitch / servo->cfg.vel_res;
			break;
		case IL_UNITS_VEL_M_S:
			factor = 1. * servo->cfg.ppitch / servo->cfg.vel_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_ACC:
		switch (servo->units.acc) {
		case IL_UNITS_ACC_NATIVE:
			factor = 1.;
			break;
		case IL_UNITS_ACC_REV_S2:
			factor = 1. / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_RAD_S2:
			factor = 2. * M_PI / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_DEG_S2:
			factor = 360. / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_UM_S2:
			factor = 1000000. * servo->cfg.ppitch /
				 servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_MM_S2:
			factor = 1000. * servo->cfg.ppitch / servo->cfg.acc_res;
			break;
		case IL_UNITS_ACC_M_S2:
			factor = 1. * servo->cfg.ppitch / servo->cfg.acc_res;
			break;
		default:
			factor = 1.;
			break;
		}

		break;
	case IL_REG_PHY_VOLT_REL:
		factor = 1. / VOLT_REL_RANGE;
		break;
	case IL_REG_PHY_RAD:
		factor = 2. * M_PI / RAD_RANGE;
		break;
	default:
		factor = 1.;
		break;
	}

	osal_mutex_unlock(servo->units.lock);

	return factor;
}

il_units_torque_t il_servo_units_torque_get(il_servo_t *servo)
{
	il_units_torque_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.torque;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_torque_set(il_servo_t *servo, il_units_torque_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.torque = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_pos_t il_servo_units_pos_get(il_servo_t *servo)
{
	il_units_pos_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.pos;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_pos_set(il_servo_t *servo, il_units_pos_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.pos = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_vel_t il_servo_units_vel_get(il_servo_t *servo)
{
	il_units_vel_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.vel;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_vel_set(il_servo_t *servo, il_units_vel_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.vel = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_acc_t il_servo_units_acc_get(il_servo_t *servo)
{
	il_units_acc_t units;

	assert(servo);

	osal_mutex_lock(servo->units.lock);
	units = servo->units.acc;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_units_acc_set(il_servo_t *servo, il_units_acc_t units)
{
	assert(servo);

	osal_mutex_lock(servo->units.lock);
	servo->units.acc = units;
	osal_mutex_unlock(servo->units.lock);
}

int il_servo_raw_read_u8(il_servo_t *servo, const il_reg_t *reg, const char *id,
			 uint8_t *buf)
{
	return raw_read(servo, reg, id, IL_REG_DTYPE_U8, buf, sizeof(*buf));
}

int il_servo_raw_read_s8(il_servo_t *servo, const il_reg_t *reg, const char *id,
			 int8_t *buf)
{
	return raw_read(servo, reg, id, IL_REG_DTYPE_S8, buf, sizeof(*buf));
}

int il_servo_raw_read_u16(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint16_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U16, buf, sizeof(*buf));
	if (r == 0)
		*buf = __swap_16(*buf);

	return r;
}

int il_servo_raw_read_s16(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int16_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S16, buf, sizeof(*buf));
	if (r == 0)
		*buf = (int16_t)__swap_16(*buf);

	return r;
}

int il_servo_raw_read_u32(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint32_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U32, buf, sizeof(*buf));
	if (r == 0)
		*buf = __swap_32(*buf);

	return r;
}

int il_servo_raw_read_s32(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int32_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S32, buf, sizeof(*buf));
	if (r == 0)
		*buf = (int32_t)__swap_32(*buf);

	return r;
}

int il_servo_raw_read_u64(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint64_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U64, buf, sizeof(*buf));
	if (r == 0)
		*buf = __swap_64(*buf);

	return r;
}

int il_servo_raw_read_s64(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int64_t *buf)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S64, buf, sizeof(*buf));
	if (r == 0)
		*buf = (int64_t)__swap_64(*buf);

	return r;
}

int il_servo_read(il_servo_t *servo, const il_reg_t *reg, const char *id,
		  double *buf)
{
	int r;

	const il_reg_t *reg_;

	uint8_t u8_v;
	uint16_t u16_v;
	uint32_t u32_v;
	uint64_t u64_v;
	int8_t s8_v;
	int16_t s16_v;
	int32_t s32_v;

	int64_t buf_;

	assert(servo);
	assert(buf);

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	/* read */
	switch (reg_->dtype) {
	case IL_REG_DTYPE_U8:
		r = il_servo_raw_read_u8(servo, reg_, NULL, &u8_v);
		buf_ = (int64_t)u8_v;
		break;
	case IL_REG_DTYPE_S8:
		r = il_servo_raw_read_s8(servo, reg_, NULL, &s8_v);
		buf_ = (int64_t)s8_v;
		break;
	case IL_REG_DTYPE_U16:
		r = il_servo_raw_read_u16(servo, reg_, NULL, &u16_v);
		buf_ = (int64_t)u16_v;
		break;
	case IL_REG_DTYPE_S16:
		r = il_servo_raw_read_s16(servo, reg_, NULL, &s16_v);
		buf_ = (int64_t)s16_v;
		break;
	case IL_REG_DTYPE_U32:
		r = il_servo_raw_read_u32(servo, reg_, NULL, &u32_v);
		buf_ = (int64_t)u32_v;
		break;
	case IL_REG_DTYPE_S32:
		r = il_servo_raw_read_s32(servo, reg_, NULL, &s32_v);
		buf_ = (int64_t)s32_v;
		break;
	case IL_REG_DTYPE_U64:
		r = il_servo_raw_read_u64(servo, reg_, NULL, &u64_v);
		buf_ = (int64_t)u64_v;
		break;
	case IL_REG_DTYPE_S64:
		r = il_servo_raw_read_s64(servo, reg_, NULL, &buf_);
		break;
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}

	if (r < 0)
		return r;

	/* store converted value to buffer */
	*buf = buf_ * il_servo_units_factor(servo, reg_);

	return 0;
}

int il_servo_raw_write_u8(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, uint8_t val, int confirm)
{
	return raw_write(servo, reg, id, IL_REG_DTYPE_U8, &val, sizeof(val),
			 confirm);
}

int il_servo_raw_write_s8(il_servo_t *servo, const il_reg_t *reg,
			  const char *id, int8_t val, int confirm)
{
	return raw_write(servo, reg, id, IL_REG_DTYPE_S8, &val, sizeof(val),
			 confirm);
}

int il_servo_raw_write_u16(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, uint16_t val, int confirm)
{
	uint16_t val_;

	val_ = __swap_16(val);

	return raw_write(servo, reg, id, IL_REG_DTYPE_U16, &val_, sizeof(val_),
			 confirm);
}

int il_servo_raw_write_s16(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, int16_t val, int confirm)
{
	int16_t val_;

	val_ = (int16_t)__swap_16(val);

	return raw_write(servo, reg, id, IL_REG_DTYPE_S16, &val_, sizeof(val_),
			 confirm);
}

int il_servo_raw_write_u32(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, uint32_t val, int confirm)
{
	uint32_t val_;

	val_ = __swap_32(val);

	return raw_write(servo, reg, id, IL_REG_DTYPE_U32, &val_, sizeof(val_),
			 confirm);
}

int il_servo_raw_write_s32(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, int32_t val, int confirm)
{
	int32_t val_;

	val_ = (int32_t)__swap_32(val);

	return raw_write(servo, reg, id, IL_REG_DTYPE_S32, &val_, sizeof(val_),
			 confirm);
}

int il_servo_raw_write_u64(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, uint64_t val, int confirm)
{
	uint64_t val_;

	val_ = __swap_64(val);

	return raw_write(servo, reg, id, IL_REG_DTYPE_U64, &val_, sizeof(val_),
			 confirm);
}

int il_servo_raw_write_s64(il_servo_t *servo, const il_reg_t *reg,
			   const char *id, int64_t val, int confirm)
{
	int64_t val_;

	val_ = (int64_t)__swap_64(val);

	return raw_write(servo, reg, id, IL_REG_DTYPE_S64, &val_, sizeof(val_),
			 confirm);
}

int il_servo_write(il_servo_t *servo, const il_reg_t *reg, const char *id,
		   double val, int confirm)
{
	int r;

	const il_reg_t *reg_;
	int64_t val_;

	assert(servo);

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg, id, &reg_);
	if (r < 0)
		return r;

	/* convert to native units */
	val_ = (int64_t)(val / il_servo_units_factor(servo, reg_));

	/* write using the appropriate native type */
	switch (reg_->dtype) {
	case IL_REG_DTYPE_U8:
		return il_servo_raw_write_u8(servo, reg_, NULL, (uint8_t)val_,
					     confirm);
	case IL_REG_DTYPE_S8:
		return il_servo_raw_write_s8(servo, reg_, NULL, (int8_t)val_,
					     confirm);
	case IL_REG_DTYPE_U16:
		return il_servo_raw_write_u16(servo, reg_, NULL, (uint16_t)val_,
					      confirm);
	case IL_REG_DTYPE_S16:
		return il_servo_raw_write_s16(servo, reg_, NULL, (int16_t)val_,
					      confirm);
	case IL_REG_DTYPE_U32:
		return il_servo_raw_write_u32(servo, reg_, NULL, (uint32_t)val_,
					      confirm);
	case IL_REG_DTYPE_S32:
		return il_servo_raw_write_s32(servo, reg_, NULL, (int32_t)val_,
					      confirm);
	case IL_REG_DTYPE_U64:
		return il_servo_raw_write_u64(servo, reg_, NULL, (uint64_t)val_,
					      confirm);
	case IL_REG_DTYPE_S64:
		return il_servo_raw_write_s64(servo, reg_, NULL, (int64_t)val_,
					      confirm);
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}
}

int il_servo_disable(il_servo_t *servo)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;
	int timeout = PDS_TIMEOUT;

	assert(servo);

	sw = sw_get(servo);

	do {
		state = pds_state_decode(sw);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_fault_reset(servo);
			if (r < 0)
				return r;

			sw = sw_get(servo);
		/* check state and command action to reach disabled */
		} else if (state != IL_SERVO_STATE_DISABLED) {
			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, IL_MC_PDS_CMD_DV, 1);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, &sw, &timeout);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_DISABLED);

	return 0;
}

int il_servo_switch_on(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;
	int timeout_ = timeout;

	assert(servo);

	sw = sw_get(servo);

	do {
		state = pds_state_decode(sw);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_fault_reset(servo);
			if (r < 0)
				return r;

			sw = sw_get(servo);
		/* check state and command action to reach switch on */
		} else if (state != IL_SERVO_STATE_ON) {
			if (state == IL_SERVO_STATE_NRDY)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_SERVO_STATE_DISABLED)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_SERVO_STATE_RDY)
				cmd = IL_MC_PDS_CMD_SO;
			else if (state == IL_SERVO_STATE_ENABLED)
				cmd = IL_MC_PDS_CMD_DO;
			else
				cmd = IL_MC_PDS_CMD_DV;

			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, cmd, 1);
			if (r < 0)
				return r;

			/* wait for state change */
			r = sw_wait_change(servo, &sw, &timeout_);
			if (r < 0)
				return r;
		}
	} while (state != IL_SERVO_STATE_ON);

	return 0;
}

int il_servo_enable(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, cmd;
	il_servo_state_t state;
	int timeout_ = timeout;

	assert(servo);

	sw = sw_get(servo);

	do {
		state = pds_state_decode(sw);

		/* try fault reset if faulty */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_fault_reset(servo);
			if (r < 0)
				return r;

			sw = sw_get(servo);
		/* check state and command action to reach enabled */
		} else if ((state != IL_SERVO_STATE_ENABLED) ||
			   !(sw & IL_MC_SW_IANGLE)) {
			if (state == IL_SERVO_STATE_NRDY)
				cmd = IL_MC_PDS_CMD_DV;
			else if (state == IL_SERVO_STATE_DISABLED)
				cmd = IL_MC_PDS_CMD_SD;
			else if (state == IL_SERVO_STATE_RDY)
				cmd = IL_MC_PDS_CMD_SOEO;
			else
				cmd = IL_MC_PDS_CMD_EO;

			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, cmd, 1);
			if (r < 0)
				return r;

			/* wait for state change */
			r = sw_wait_change(servo, &sw, &timeout_);
			if (r < 0)
				return r;
		}
	} while ((state != IL_SERVO_STATE_ENABLED) || !(sw & IL_MC_SW_IANGLE));

	return 0;
}

int il_servo_fault_reset(il_servo_t *servo)
{
	int r;
	uint16_t sw;
	il_servo_state_t state;
	int timeout = PDS_TIMEOUT;

	assert(servo);

	sw = sw_get(servo);

	do {
		state = pds_state_decode(sw);

		/* check if faulty, if so try to reset (0->1) */
		if ((state == IL_SERVO_STATE_FAULT) ||
		    (state == IL_SERVO_STATE_FAULTR)) {
			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, 0, 1);
			if (r < 0)
				return r;

			r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD,
						   NULL, IL_MC_PDS_CMD_FR, 1);
			if (r < 0)
				return r;

			/* wait until statusword changes */
			r = sw_wait_change(servo, &sw, &timeout);
			if (r < 0)
				return r;
		}
	} while ((state == IL_SERVO_STATE_FAULT) ||
		 (state == IL_SERVO_STATE_FAULTR));

	return 0;
}

int il_servo_mode_get(il_servo_t *servo, il_servo_mode_t *mode)
{
	int r;
	int8_t code;

	assert(mode);

	r = il_servo_raw_read_s8(servo, &IL_REG_OP_MODE_DISP, NULL, &code);
	if (r < 0)
		return r;

	switch (code) {
	case ILK_OP_MODE_OLV:
		*mode = IL_SERVO_MODE_OLV;
		break;
	case ILK_OP_MODE_OLS:
		*mode = IL_SERVO_MODE_OLS;
		break;
	case ILK_OP_MODE_PP:
		*mode = IL_SERVO_MODE_PP;
		break;
	case ILK_OP_MODE_VEL:
		*mode = IL_SERVO_MODE_VEL;
		break;
	case ILK_OP_MODE_PV:
		*mode = IL_SERVO_MODE_PV;
		break;
	case ILK_OP_MODE_PT:
		*mode = IL_SERVO_MODE_PT;
		break;
	case ILK_OP_MODE_HOMING:
		*mode = IL_SERVO_MODE_HOMING;
		break;
	case ILK_OP_MODE_IP:
		*mode = IL_SERVO_MODE_IP;
		break;
	case ILK_OP_MODE_CSP:
		*mode = IL_SERVO_MODE_CSP;
		break;
	case ILK_OP_MODE_CSV:
		*mode = IL_SERVO_MODE_CSV;
		break;
	case ILK_OP_MODE_CST:
		*mode = IL_SERVO_MODE_CST;
		break;
	default:
		ilerr__set("Unknown operation mode: %d", code);
		return IL_EINVAL;
	}

	return 0;
}

int il_servo_mode_set(il_servo_t *servo, il_servo_mode_t mode)
{
	int r;
	int8_t code;

	switch (mode) {
	case IL_SERVO_MODE_OLV:
		code = ILK_OP_MODE_OLV;
		break;
	case IL_SERVO_MODE_OLS:
		code = ILK_OP_MODE_OLS;
		break;
	case IL_SERVO_MODE_PP:
		code = ILK_OP_MODE_PP;
		break;
	case IL_SERVO_MODE_VEL:
		code = ILK_OP_MODE_VEL;
		break;
	case IL_SERVO_MODE_PV:
		code = ILK_OP_MODE_PV;
		break;
	case IL_SERVO_MODE_PT:
		code = ILK_OP_MODE_PT;
		break;
	case IL_SERVO_MODE_HOMING:
		code = ILK_OP_MODE_HOMING;
		break;
	case IL_SERVO_MODE_IP:
		code = ILK_OP_MODE_IP;
		break;
	case IL_SERVO_MODE_CSP:
		code = ILK_OP_MODE_CSP;
		break;
	case IL_SERVO_MODE_CSV:
		code = ILK_OP_MODE_CSV;
		break;
	case IL_SERVO_MODE_CST:
		code = ILK_OP_MODE_CST;
		break;
	default:
		ilerr__set("Invalid mode");
		return IL_EINVAL;
	}

	r = il_servo_raw_write_s8(servo, &IL_REG_OP_MODE, NULL, code, 1);
	if (r < 0)
		return r;

	servo->mode = mode;

	return 0;
}

int il_servo_ol_voltage_get(il_servo_t *servo, double *voltage)
{
	return il_servo_read(servo, &IL_REG_OL_VOLTAGE, NULL, voltage);
}

int il_servo_ol_voltage_set(il_servo_t *servo, double voltage)
{
	return il_servo_write(servo, &IL_REG_OL_VOLTAGE, NULL, voltage, 1);
}

int il_servo_ol_frequency_get(il_servo_t *servo, double *freq)
{
	return il_servo_read(servo, &IL_REG_OL_FREQUENCY, NULL, freq);
}

int il_servo_ol_frequency_set(il_servo_t *servo, double freq)
{
	return il_servo_write(servo, &IL_REG_OL_FREQUENCY, NULL, freq, 1);
}

int il_servo_homing_start(il_servo_t *servo)
{
	return il_servo_raw_write_u16(
			servo, &IL_REG_CTL_WORD, NULL,
			IL_MC_HOMING_CW_START | IL_MC_PDS_CMD_EO, 1);
}

int il_servo_homing_wait(il_servo_t *servo, int timeout)
{
	int r;
	uint16_t sw, state;
	int timeout_ = timeout;

	assert(servo);

	sw = sw_get(servo);

	do {
		state = sw & IL_MC_HOMING_STA_MSK;

		if (state == IL_MC_HOMING_STA_INPROG) {
			r = sw_wait_change(servo, &sw, &timeout_);
			if (r < 0)
				return r;
		}
	} while (state == IL_MC_HOMING_STA_INPROG);

	if (state == IL_MC_HOMING_STA_SUCCESS)
		return 0;

	/* report failures */
	if (state == IL_MC_HOMING_STA_INT)
		ilerr__set("Homing procedure is interrupted or not started");
	else if (state == IL_MC_HOMING_STA_ATT)
		ilerr__set("Homing is attained, but target is not reached");
	else if (state == IL_MC_HOMING_STA_ERR_VNZ)
		ilerr__set("Homing error occurred, velocity is not zero");
	else if (state == IL_MC_HOMING_STA_ERR_VZ)
		ilerr__set("Homing error occurred, velocity is zero");

	return IL_EFAIL;
}

int il_servo_torque_get(il_servo_t *servo, double *torque)
{
	return il_servo_read(servo, &IL_REG_TORQUE_ACT, NULL, torque);
}

int il_servo_torque_set(il_servo_t *servo, double torque)
{
	return il_servo_write(servo, &IL_REG_TORQUE_TGT, NULL, torque, 1);
}

int il_servo_position_get(il_servo_t *servo, double *pos)
{
	return il_servo_read(servo, &IL_REG_POS_ACT, NULL, pos);
}

int il_servo_position_set(il_servo_t *servo, double pos, int immediate,
			  int relative, int sp_timeout)
{
	int r;
	uint16_t cmd;
	il_servo_state_t state;
	int flags;

	/* send position */
	r = il_servo_write(servo, &IL_REG_POS_TGT, NULL, pos, 1);
	if (r < 0)
		return r;

	/* wait for SP ack if enabled and in PP */
	il_servo_state_get(servo, &state, &flags);

	if ((state == IL_SERVO_STATE_ENABLED) &&
	    (servo->mode == IL_SERVO_MODE_PP)) {
		/* new set-point (0->1) */
		cmd = IL_MC_PDS_CMD_EO;
		r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD, NULL,
					   cmd, 1);
		if (r < 0)
			return r;

		/* wait set-point ack clear */
		r = sw_wait_value(servo, IL_MC_PP_SW_SPACK, 0, sp_timeout);
		if (r < 0)
			return r;

		/* set-point */
		cmd |= IL_MC_PP_CW_NEWSP;

		if (immediate)
			cmd |= IL_MC_PP_CW_IMMEDIATE;

		if (relative)
			cmd |= IL_MC_PP_CW_REL;

		r = il_servo_raw_write_u16(servo, &IL_REG_CTL_WORD, NULL,
					   cmd, 1);
		if (r < 0)
			return r;

		/* wait set-point ack */
		r = sw_wait_value(servo, IL_MC_PP_SW_SPACK, IL_MC_PP_SW_SPACK,
				  sp_timeout);
		if (r < 0)
			return r;
	}

	return 0;
}

int il_servo_position_res_get(il_servo_t *servo, uint32_t *res)
{
	int r;
	uint8_t fb, ppoles, turnbits;
	uint32_t incrs, revs;

	assert(servo);
	assert(res);

	r = il_servo_raw_read_u8(servo, &IL_REG_FB_POS_SENSOR, NULL, &fb);
	if (r < 0)
		return r;

	switch (fb) {
	case ILK_POS_SENSOR_DIGITAL_ENCODER:
		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_ENC_INCR, NULL,
					  &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_MOTOR_REVS, NULL,
					  &revs);
		if (r < 0)
			return r;

		*res = incrs / revs;
		break;

	case ILK_POS_SENSOR_DIGITAL_HALLS:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, NULL,
					 &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * DIGITAL_HALLS_CONSTANT;
		break;

	case ILK_POS_SENSOR_ANALOG_HALLS:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, NULL,
					 &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * ANALOG_HALLS_CONSTANT;
		break;

	case ILK_POS_SENSOR_ANALOG_INPUT:
		r = il_servo_raw_read_u8(servo, &IL_REG_PAIR_POLES, NULL,
					 &ppoles);
		if (r < 0)
			return r;

		*res = ppoles * ANALOG_INPUT_CONSTANT;
		break;

	case ILK_POS_SENSOR_SINCOS:
		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_ENC_INCR, NULL,
					  &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(servo, &IL_REG_PRES_MOTOR_REVS, NULL,
					  &revs);
		if (r < 0)
			return r;

		*res = (incrs / revs) * SINCOS_CONSTANT;
		break;

	case ILK_POS_SENSOR_PWM:
		*res = PWM_CONSTANT;
		break;

	case ILK_POS_SENSOR_RESOLVER:
		*res = RESOLVER_CONSTANT;
		break;

	case ILK_POS_SENSOR_SSI:
		r = il_servo_raw_read_u8(servo, &IL_REG_SSI_STURNBITS, NULL,
					 &turnbits);
		if (r < 0)
			return r;

		*res = 2 << turnbits;
		break;

	default:
		*res = 1;
	}

	return 0;
}

int il_servo_velocity_get(il_servo_t *servo, double *vel)
{
	return il_servo_read(servo, &IL_REG_VEL_ACT, NULL, vel);
}

int il_servo_velocity_set(il_servo_t *servo, double vel)
{
	return il_servo_write(servo, &IL_REG_VEL_TGT, NULL, vel, 1);
}

int il_servo_velocity_res_get(il_servo_t *servo, uint32_t *res)
{
	int r;
	uint8_t fb;
	uint32_t incrs, revs;

	assert(servo);
	assert(res);

	r = il_servo_raw_read_u8(servo, &IL_REG_FB_VEL_SENSOR, NULL, &fb);
	if (r < 0)
		return r;

	switch (fb) {
	case ILK_VEL_SENSOR_POS:
		r = il_servo_position_res_get(servo, res);
		break;

	case ILK_VEL_SENSOR_TACHOMETER:
		r = il_servo_raw_read_u32(servo, &IL_REG_VRES_ENC_INCR, NULL,
					  &incrs);
		if (r < 0)
			return r;

		r = il_servo_raw_read_u32(servo, &IL_REG_VRES_MOTOR_REVS, NULL,
					  &revs);
		if (r < 0)
			return r;

		*res = incrs / revs;
		break;

	default:
		*res = 1;
	}

	return 0;
}

int il_servo_wait_reached(il_servo_t *servo, int timeout)
{
	assert(servo);

	/* wait until target reached */
	return sw_wait_value(servo, IL_MC_SW_TR, IL_MC_SW_TR, timeout);
}
