#include "../servo.h"

#include "ingenialink/err.h"
#include <windows.h>

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
		   const char *id, const il_reg_t **reg, uint8_t subnode)
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

		r = il_dict_reg_get(dict, id, reg, subnode);
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
		    const char *id, il_reg_dtype_t dtype, void *buf, size_t sz, uint8_t subnode)
{
	int r;
	const il_reg_t *reg;

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg_pdef, id, &reg, subnode);
	if (r < 0)
		return r;

	/* verify register properties */
	if (reg->dtype != dtype) {
		ilerr__set("Unexpected register data type");
		return IL_EINVAL;
	}

	if (reg->access == IL_REG_ACCESS_WO) {
		ilerr__set("Register is write-only");
		return IL_EACCESS;
	}
	Sleep(2);
	return il_net__read(servo->net, servo->id, reg->subnode, reg->address, buf, sz);
}

/**
 * Raw write.
 *
 * @param [in] servo
 *	Servo.
 * @param [in] reg
 *	Pre-defined register.
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
static int raw_write(il_servo_t *servo, const il_reg_t *reg,
		     il_reg_dtype_t dtype, const void *data, size_t sz,
		     int confirmed, uint16_t extended)
{
	int confirmed_;

	/* verify register properties */
	if (reg->dtype != dtype) {
		ilerr__set("Unexpected register data type");
		return IL_EINVAL;
	}

	if (reg->access == IL_REG_ACCESS_RO) {
		ilerr__set("Register is read-only");
		return IL_EACCESS;
	}

	/* skip confirmation on write-only registers */
	confirmed_ = (reg->access == IL_REG_ACCESS_WO) ? 0 : confirmed;

	return il_net__write(servo->net, servo->id, reg->subnode, reg->address, data, sz,
			     confirmed_, extended);
}

static int raw_wait_write(il_servo_t *servo, const il_reg_t *reg,
		     il_reg_dtype_t dtype, const void *data, size_t sz,
		     int confirmed, uint16_t extended)
{
	int confirmed_;

	/* verify register properties */
	if (reg->dtype != dtype) {
		ilerr__set("Unexpected register data type");
		return IL_EINVAL;
	}

	if (reg->access == IL_REG_ACCESS_RO) {
		ilerr__set("Register is read-only");
		return IL_EACCESS;
	}

	/* skip confirmation on write-only registers */
	confirmed_ = (reg->access == IL_REG_ACCESS_WO) ? 0 : confirmed;

	return il_net__wait_write(servo->net, servo->id, reg->subnode, reg->address, data, sz,
			     confirmed_, extended);
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
	// printf("%d\n", servo->sw.value);
	osal_mutex_unlock(servo->sw.lock);
}

/**
 * State change monitor, used to push state changes to external subscriptors.
 *
 * @param [in] args
 *	Arguments (il_servo_t *).
 */
static int state_subs_monitor(void *args)
{
	// Init internal variables
	uint16_t sw;
	uint8_t subnode = 1;
	il_servo_t *servo = args;
	// Status word register
	il_reg_t status_word_register = {
		.subnode = subnode,
		.address = 0x0011,
		.dtype = IL_REG_DTYPE_U16,
		.access = IL_REG_ACCESS_RW,
		.phy = IL_REG_PHY_NONE,
		.range = {
			.min.u16 = 0,
			.max.u16 = UINT16_MAX
		},
		.labels = NULL,
		.enums = NULL,
		.enums_count = 0
	};
	il_servo_state_t states[5] = { IL_SERVO_STATE_NRDY, IL_SERVO_STATE_NRDY, IL_SERVO_STATE_NRDY, IL_SERVO_STATE_NRDY, IL_SERVO_STATE_NRDY };
	Sleep(200);
	while (servo->state_subs.kill != 1) {
		if (servo->state_subs.stop != 1) {
			for (uint8_t i = 0; i < servo->subnodes; i++) {
				subnode = i + 1;
				status_word_register.subnode = subnode;
				osal_mutex_lock(servo->sw.lock);
				int r = il_servo_raw_read_u16(servo, &status_word_register, NULL, &sw);
				if (r < 0) {
					osal_mutex_unlock(servo->sw.lock);
				}
				else {
					if (servo->sw.value != sw) {
						servo->sw.value = sw;
						osal_cond_broadcast(servo->sw.changed);
					}
					osal_mutex_unlock(servo->sw.lock);
					/* obtain state/flags */
					il_servo_state_t current_state;
					int flags;
					servo->ops->_state_decode(sw, &current_state, &flags);
					if (current_state != states[i]) {
						states[i] = current_state;
						/* notify all subscribers */
						size_t sz;
						osal_mutex_lock(servo->state_subs.lock);
						for (sz = 0; sz < servo->state_subs.sz; sz++) {
							void *ctx;
							if (!servo->state_subs.subs[sz].cb)
								continue;
							ctx = servo->state_subs.subs[sz].ctx;
							servo->state_subs.subs[sz].cb(ctx, current_state, flags, subnode);
						}

						osal_mutex_unlock(servo->state_subs.lock);
					}
				}
			}
			Sleep(200);
		}
		Sleep(200);
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

/*******************************************************************************
 * Base implementation
 ******************************************************************************/

int il_servo_base__init(il_servo_t *servo, il_net_t *net, uint16_t id,
			const char *dict)
{
	int r;

	/* initialize */
	servo->net = net;
	servo->id = id;

	//il_net__retain(servo->net);

	/* load dictionary (optional) */
	if (dict) {
		servo->dict = il_dict_create(dict);
		if (!servo->dict) {
			r = IL_EFAIL;
			goto cleanup_net;
		}
	} else {
		servo->dict = NULL;
	}

	/* configure units */
	servo->units.lock = osal_mutex_create();
	if (!servo->units.lock) {
		ilerr__set("Units lock allocation failed");
		r = IL_EFAIL;
		goto cleanup_dict;
	}

	servo->units.torque = IL_UNITS_TORQUE_NATIVE;
	servo->units.pos = IL_UNITS_POS_NATIVE;
	servo->units.vel = IL_UNITS_VEL_NATIVE;
	servo->units.acc = IL_UNITS_ACC_NATIVE;

	/* configure statusword subscription */
	servo->sw.lock = osal_mutex_create();
	if (!servo->sw.lock) {
		ilerr__set("Statusword subscriber lock allocation failed");
		r = IL_EFAIL;
		goto cleanup_units_lock;
	}

	servo->sw.changed = osal_cond_create();
	if (!servo->sw.changed) {
		ilerr__set("Statusword subscriber condition allocation failed");
		r = IL_EFAIL;
		goto cleanup_sw_lock;
	}

	servo->sw.value = 0;

	r = il_net__sw_subscribe(servo->net, servo->id, sw_update, servo);
	if (r < 0)
		goto cleanup_sw_changed;

	servo->sw.slot = r;

	/* configute external state subscriptors */
	servo->state_subs.subs = calloc(STATE_SUBS_SZ_DEF,
		sizeof(*servo->state_subs.subs));
	if (!servo->state_subs.subs) {
		ilerr__set("State subscribers allocation failed");
		r = IL_EFAIL;
		goto cleanup_sw_subscribe;
	}

	servo->state_subs.sz = STATE_SUBS_SZ_DEF;

	servo->state_subs.lock = osal_mutex_create();
	if (!servo->state_subs.lock) {
		ilerr__set("State subscription lock allocation failed");
		r = IL_EFAIL;
		goto cleanup_state_subs_subs;
	}

	servo->state_subs.kill = 0;
	servo->state_subs.stop = 1;

	servo->state_subs.monitor = osal_thread_create_(state_subs_monitor,
		servo);
	if (!servo->state_subs.monitor) {
		ilerr__set("State change monitor could not be created");
		r = IL_EFAIL;
		goto cleanup_state_subs_lock;
	}

	/* configure emergency subscription */
	servo->emcy.lock = osal_mutex_create();
	if (!servo->emcy.lock) {
		ilerr__set("Emergency subscriber lock allocation failed");
		r = IL_EFAIL;
		goto cleanup_state_subs_monitor;
	}

	servo->emcy.not_empty = osal_cond_create();
	if (!servo->emcy.not_empty) {
		ilerr__set("Emergency subscriber condition allocation failed");
		r = IL_EFAIL;
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
		r = IL_EFAIL;
		goto cleanup_emcy_subscribe;
	}

	servo->emcy_subs.sz = EMCY_SUBS_SZ_DEF;

	servo->emcy_subs.lock = osal_mutex_create();
	if (!servo->emcy_subs.lock) {
		ilerr__set("Emergency subscription lock allocation failed");
		r = IL_EFAIL;
		goto cleanup_emcy_subs_subs;
	}

	servo->emcy_subs.stop = 0;
	servo->emcy_subs.monitor = osal_thread_create_(emcy_subs_monitor, servo);
	if (!servo->emcy_subs.monitor) {
		ilerr__set("Emergency monitor could not be created");
		r = IL_EFAIL;
		goto cleanup_emcy_subs_lock;
	}

	return 0;

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
	servo->state_subs.kill = 1;
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

cleanup_dict:
	if (servo->dict)
		il_dict_destroy(servo->dict);

cleanup_net:
	il_net__release(servo->net);

	return r;
}

void il_servo_base__deinit(il_servo_t *servo)
{
	servo->emcy_subs.stop = 1;
	osal_mutex_destroy(servo->emcy_subs.lock);
	free(servo->emcy_subs.subs);

	il_net__emcy_unsubscribe(servo->net, servo->emcy.slot);
	osal_cond_destroy(servo->emcy.not_empty);
	osal_mutex_destroy(servo->emcy.lock);

	servo->state_subs.stop = 1;
	servo->state_subs.kill = 1;
	(void)osal_thread_join(servo->state_subs.monitor, NULL);
	osal_mutex_destroy(servo->state_subs.lock);
	free(servo->state_subs.subs);

	il_net__sw_unsubscribe(servo->net, servo->sw.slot);
	osal_cond_destroy(servo->sw.changed);
	osal_mutex_destroy(servo->sw.lock);

	osal_mutex_destroy(servo->units.lock);

	if (servo->dict)
		il_dict_destroy(servo->dict);

	//il_net__release(servo->net);
}

void il_servo_base__state_get(il_servo_t *servo, il_servo_state_t *state,
			      int *flags, uint8_t subnode)
{
	uint16_t sw;
	il_reg_t status_word_register = {
		.subnode = subnode,
		.address = 0x0011,
		.dtype = IL_REG_DTYPE_U16,
		.access = IL_REG_ACCESS_RW,
		.phy = IL_REG_PHY_NONE,
		.range = {
			.min.u16 = 0,
			.max.u16 = UINT16_MAX
		},
		.labels = NULL,
		.enums = NULL,
		.enums_count = 0
	};
	osal_mutex_lock(servo->sw.lock);
	int r = il_servo_raw_read_u16(servo, &status_word_register, NULL, &sw);
	osal_mutex_unlock(servo->sw.lock);

	servo->ops->_state_decode(sw, state, flags);
}

int il_servo_base__state_subscribe(il_servo_t *servo,
				   il_servo_state_subscriber_cb_t cb, void *ctx)
{
	int r = 0;
	int slot;

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

void il_servo_base__state_unsubscribe(il_servo_t *servo, int slot)
{
	osal_mutex_lock(servo->state_subs.lock);

	/* skip out of range slot */
	if (slot >= (int)servo->state_subs.sz)
		return;

	servo->state_subs.subs[slot].cb = NULL;
	servo->state_subs.subs[slot].ctx = NULL;

	osal_mutex_unlock(servo->state_subs.lock);
}

int il_servo_base__emcy_subscribe(il_servo_t *servo,
				  il_servo_emcy_subscriber_cb_t cb, void *ctx)
{
	int r = 0;
	int slot;

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

void il_servo_base__emcy_unsubscribe(il_servo_t *servo, int slot)
{
	osal_mutex_lock(servo->emcy_subs.lock);

	/* skip out of range slot */
	if (slot >= (int)servo->emcy_subs.sz)
		return;

	servo->emcy_subs.subs[slot].cb = NULL;
	servo->emcy_subs.subs[slot].ctx = NULL;

	osal_mutex_unlock(servo->emcy_subs.lock);
}

il_dict_t *il_servo_base__dict_get(il_servo_t *servo)
{
	return servo->dict;
}

int il_servo_base__dict_load(il_servo_t *servo, const char *dict)
{
	if (servo->dict) {
		ilerr__set("Dictionary already loaded");
		return IL_EALREADY;
	}

	servo->dict = il_dict_create(dict);
	if (!servo->dict)
		return IL_EFAIL;

	return 0;
}

il_units_torque_t il_servo_base__units_torque_get(il_servo_t *servo)
{
	il_units_torque_t units;

	osal_mutex_lock(servo->units.lock);
	units = servo->units.torque;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_base__units_torque_set(il_servo_t *servo, il_units_torque_t units)
{
	osal_mutex_lock(servo->units.lock);
	servo->units.torque = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_pos_t il_servo_base__units_pos_get(il_servo_t *servo)
{
	il_units_pos_t units;

	osal_mutex_lock(servo->units.lock);
	units = servo->units.pos;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_base__units_pos_set(il_servo_t *servo, il_units_pos_t units)
{
	osal_mutex_lock(servo->units.lock);
	servo->units.pos = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_vel_t il_servo_base__units_vel_get(il_servo_t *servo)
{
	il_units_vel_t units;

	osal_mutex_lock(servo->units.lock);
	units = servo->units.vel;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_base__units_vel_set(il_servo_t *servo, il_units_vel_t units)
{
	osal_mutex_lock(servo->units.lock);
	servo->units.vel = units;
	osal_mutex_unlock(servo->units.lock);
}

il_units_acc_t il_servo_base__units_acc_get(il_servo_t *servo)
{
	il_units_acc_t units;

	osal_mutex_lock(servo->units.lock);
	units = servo->units.acc;
	osal_mutex_unlock(servo->units.lock);

	return units;
}

void il_servo_base__units_acc_set(il_servo_t *servo, il_units_acc_t units)
{
	osal_mutex_lock(servo->units.lock);
	servo->units.acc = units;
	osal_mutex_unlock(servo->units.lock);
}

int il_servo_base__raw_read_u8(il_servo_t *servo, const il_reg_t *reg,
			       const char *id, uint8_t *buf, uint8_t subnode)
{
	return raw_read(servo, reg, id, IL_REG_DTYPE_U8, buf, sizeof(*buf), subnode);
}

int il_servo_base__raw_read_s8(il_servo_t *servo, const il_reg_t *reg,
			       const char *id, int8_t *buf, uint8_t subnode)
{
	return raw_read(servo, reg, id, IL_REG_DTYPE_S8, buf, sizeof(*buf), subnode);
}

int il_servo_base__raw_read_u16(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint16_t *buf, uint8_t subnode)
{
	int r;
	// printf("before read \n");
	r = raw_read(servo, reg, id, IL_REG_DTYPE_U16, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = __swap_be_16(*buf);

	// printf("after read \n");

	return r;
}

int il_servo_base__raw_read_s16(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int16_t *buf, uint8_t subnode)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S16, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = (int16_t)__swap_be_16(*buf);

	return r;
}

int il_servo_base__raw_read_u32(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint32_t *buf, uint8_t subnode)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U32, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = __swap_be_32(*buf);

	return r;
}

int il_servo_base__raw_read_str(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint32_t *buf, uint8_t subnode)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_STR, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = __swap_be_32(*buf);

	return r;
}

int il_servo_base__raw_read_s32(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int32_t *buf, uint8_t subnode)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S32, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = (int32_t)__swap_be_32(*buf);

	return r;
}

int il_servo_base__raw_read_u64(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint64_t *buf, uint8_t subnode)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_U64, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = __swap_be_64(*buf);

	return r;
}

int il_servo_base__raw_read_s64(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int64_t *buf, uint8_t subnode)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_S64, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = (int64_t)__swap_be_64(*buf);

	return r;
}

int il_servo_base__raw_read_float(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, float *buf, uint8_t subnode)
{
	int r;

	r = raw_read(servo, reg, id, IL_REG_DTYPE_FLOAT, buf, sizeof(*buf), subnode);
	if (r == 0)
		*buf = __swap_be_float(*buf);

	return r;
}

int il_servo_base__read(il_servo_t *servo, const il_reg_t *reg, const char *id,
			double *buf, uint8_t subnode)
{
	int r;

	const il_reg_t *reg_;

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

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	/* read */
	switch (reg_->dtype) {
	case IL_REG_DTYPE_U8:
		r = il_servo_raw_read_u8(servo, reg_, NULL, &u8_v, subnode);
		buf_ = (float)u8_v;
		break;
	case IL_REG_DTYPE_S8:
		r = il_servo_raw_read_s8(servo, reg_, NULL, &s8_v, subnode);
		buf_ = (float)s8_v;
		break;
	case IL_REG_DTYPE_U16:
		r = il_servo_raw_read_u16(servo, reg_, NULL, &u16_v, subnode);
		buf_ = (float)u16_v;
		break;
	case IL_REG_DTYPE_S16:
		r = il_servo_raw_read_s16(servo, reg_, NULL, &s16_v, subnode);
		buf_ = (float)s16_v;
		break;
	case IL_REG_DTYPE_U32:
		r = il_servo_raw_read_u32(servo, reg_, NULL, &u32_v, subnode);
		buf_ = (double)u32_v;
		break;
	case IL_REG_DTYPE_S32:
		r = il_servo_raw_read_s32(servo, reg_, NULL, &s32_v, subnode);
		buf_ = (float)s32_v;
		break;
	case IL_REG_DTYPE_U64:
		r = il_servo_raw_read_u64(servo, reg_, NULL, &u64_v, subnode);
		buf_ = (float)u64_v;
		break;
	case IL_REG_DTYPE_S64:
		r = il_servo_raw_read_s64(servo, reg_, NULL, &s64_v, subnode);
		buf_ = (float)s64_v;
		break;
	case IL_REG_DTYPE_FLOAT:
		r = il_servo_raw_read_float(servo, reg_, NULL, &float_v, subnode);
		buf_ = (double)float_v;
		break;
	case IL_REG_DTYPE_STR:
		r = il_servo_raw_read_str(servo, reg_, NULL, &u32_str_v, subnode);
		buf_ = (float)u32_str_v;
		break;
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}

	if (r < 0)
		return r;

	/* store converted value to buffer: NOT used right now */
	//*buf = buf_ * il_servo_units_factor(servo, reg_);
	*buf = buf_;

	return 0;
}

int il_servo_base__raw_write_u8(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint8_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.u8) || (val > reg->range.max.u8)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	return raw_write(servo, reg_, IL_REG_DTYPE_U8, &val, sizeof(val),
			 confirm, extended, subnode);
}

int il_servo_base__raw_write_s8(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int8_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.s8) || (val > reg->range.max.s8)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	return raw_write(servo, reg_, IL_REG_DTYPE_S8, &val, sizeof(val),
			 confirm, extended, subnode);
}

int il_servo_base__raw_write_u16(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint16_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	uint16_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.u16) || (val > reg->range.max.u16)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}


	val_ = __swap_be_16(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_U16, &val_, sizeof(val_),
			 confirm, extended, subnode);
}

int il_servo_base__raw_write_s16(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, int16_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	int16_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.s16) || (val > reg->range.max.s16)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	val_ = (int16_t)__swap_be_16(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_S16, &val_, sizeof(val_),
			 confirm, extended, subnode);
}

int il_servo_base__raw_write_u32(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint32_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	uint32_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.u32) || (val > reg->range.max.u32)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	val_ = __swap_be_32(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_U32, &val_, sizeof(val_),
			 confirm, extended, subnode);
}

int il_servo_base__raw_wait_write_u32(il_servo_t *servo, const il_reg_t *reg,
	const char *id, uint32_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	uint32_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.u32) || (val > reg->range.max.u32)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	val_ = __swap_be_32(val);

	return raw_wait_write(servo, reg_, IL_REG_DTYPE_U32, &val_, sizeof(val_),
		confirm, extended, subnode);
}

int il_servo_base__raw_write_s32(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, int32_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	int32_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.s32) || (val > reg->range.max.s32)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	val_ = (int32_t)__swap_be_32(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_S32, &val_, sizeof(val_),
			 confirm, extended, subnode);
}

int il_servo_base__raw_write_u64(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint64_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	uint64_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.u64) || (val > reg->range.max.u64)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	val_ = __swap_be_64(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_U64, &val_, sizeof(val_),
			 confirm, extended, subnode);
}

int il_servo_base__raw_write_s64(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, int64_t val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	int64_t val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	if (reg != NULL) {
		if ((val < reg->range.min.s64) || (val > reg->range.max.s64)) {
			ilerr__set("Value out of range");
			return IL_EINVAL;
		}
	}

	val_ = (int64_t)__swap_be_64(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_S64, &val_, sizeof(val_),
			 confirm, extended, subnode);
}

int il_servo_base__raw_write_float(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, float val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;
	float val_;
	const il_reg_t *reg_;

	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	val_ = __swap_be_float(val);

	return raw_write(servo, reg_, IL_REG_DTYPE_FLOAT, &val_,
			 sizeof(val_), confirm, extended, subnode);
}

int il_servo_base__write(il_servo_t *servo, const il_reg_t *reg, const char *id,
			 double val, int confirm, uint16_t extended, uint8_t subnode)
{
	int r;

	const il_reg_t *reg_;
	double val_;

	/* obtain register (predefined or from dictionary) */
	r = get_reg(servo->dict, reg, id, &reg_, subnode);
	if (r < 0)
		return r;

	/* convert to native units: NOT used right now */
	/*val_ = val / il_servo_units_factor(servo, reg_);*/
	val_ = val;

	/* write using the appropriate native type */
	switch (reg_->dtype) {
	case IL_REG_DTYPE_U8:
		return il_servo_raw_write_u8(servo, reg_, NULL, (uint8_t)val_,
					     confirm, extended);
	case IL_REG_DTYPE_S8:
		return il_servo_raw_write_s8(servo, reg_, NULL, (int8_t)val_,
					     confirm, extended);
	case IL_REG_DTYPE_U16:
		return il_servo_raw_write_u16(servo, reg_, NULL, (uint16_t)val_,
					      confirm, extended);
	case IL_REG_DTYPE_S16:
		return il_servo_raw_write_s16(servo, reg_, NULL, (int16_t)val_,
					      confirm, extended);
	case IL_REG_DTYPE_U32:
		return il_servo_raw_write_u32(servo, reg_, NULL, (uint32_t)val_,
					      confirm, extended);
	case IL_REG_DTYPE_S32:
		return il_servo_raw_write_s32(servo, reg_, NULL, (int32_t)val_,
					      confirm, extended);
	case IL_REG_DTYPE_U64:
		return il_servo_raw_write_u64(servo, reg_, NULL, (uint64_t)val_,
					      confirm, extended);
	case IL_REG_DTYPE_S64:
		return il_servo_raw_write_s64(servo, reg_, NULL, (int64_t)val_,
					      confirm, extended);
	case IL_REG_DTYPE_FLOAT:
		return il_servo_raw_write_float(servo, reg_, NULL, (float)val_,
						confirm, extended);
	default:
		ilerr__set("Unsupported register data type");
		return IL_EINVAL;
	}
}
