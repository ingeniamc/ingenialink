#ifndef INGENIALINK_BASE_SERVO_H_
#define INGENIALINK_BASE_SERVO_H_

#include "public/ingenialink/servo.h"

int il_servo_base__init(il_servo_t *servo, il_net_t *net, uint16_t id,
			const char *dict);
void il_servo_base__deinit(il_servo_t *servo);

void il_servo_base__state_get(il_servo_t *servo, il_servo_state_t *state,
			      int *flags);

int il_servo_base__state_subscribe(il_servo_t *servo,
				   il_servo_state_subscriber_cb_t cb,
				   void *ctx);

void il_servo_base__state_unsubscribe(il_servo_t *servo, int slot);

int il_servo_base__emcy_subscribe(il_servo_t *servo,
				  il_servo_emcy_subscriber_cb_t cb, void *ctx);

void il_servo_base__emcy_unsubscribe(il_servo_t *servo, int slot);

il_dict_t *il_servo_base__dict_get(il_servo_t *servo);

int il_servo_base__dict_load(il_servo_t *servo, const char *dict);

il_units_torque_t il_servo_base__units_torque_get(il_servo_t *servo);

void il_servo_base__units_torque_set(il_servo_t *servo,
				     il_units_torque_t units);

il_units_pos_t il_servo_base__units_pos_get(il_servo_t *servo);

void il_servo_base__units_pos_set(il_servo_t *servo, il_units_pos_t units);

il_units_vel_t il_servo_base__units_vel_get(il_servo_t *servo);

void il_servo_base__units_vel_set(il_servo_t *servo, il_units_vel_t units);

il_units_acc_t il_servo_base__units_acc_get(il_servo_t *servo);

void il_servo_base__units_acc_set(il_servo_t *servo, il_units_acc_t units);

int il_servo_base__raw_read_u8(il_servo_t *servo, const il_reg_t *reg,
			       const char *id, uint8_t *buf);

int il_servo_base__raw_read_s8(il_servo_t *servo, const il_reg_t *reg,
			       const char *id, int8_t *buf);

int il_servo_base__raw_read_u16(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint16_t *buf);

int il_servo_base__raw_read_s16(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int16_t *buf);

int il_servo_base__raw_read_u32(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint32_t *buf);

int il_servo_base__raw_read_str(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint32_t *buf);

int il_servo_base__raw_read_s32(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int32_t *buf);

int il_servo_base__raw_read_u64(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint64_t *buf);

int il_servo_base__raw_read_s64(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int64_t *buf);

int il_servo_base__raw_read_float(il_servo_t *servo, const il_reg_t *reg,
				  const char *id, float *buf);

int il_servo_base__read(il_servo_t *servo, const il_reg_t *reg, const char *id,
			double *buf);

int il_servo_base__raw_write_u8(il_servo_t *servo, const il_reg_t *reg,
				const char *id, uint8_t val, int confirm);

int il_servo_base__raw_write_s8(il_servo_t *servo, const il_reg_t *reg,
				const char *id, int8_t val, int confirm);

int il_servo_base__raw_write_u16(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint16_t val, int confirm);

int il_servo_base__raw_write_s16(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, int16_t val, int confirm);

int il_servo_base__raw_write_u32(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint32_t val, int confirm);

int il_servo_base__raw_wait_write_u32(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint32_t val, int confirm);

int il_servo_base__raw_write_s32(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, int32_t val, int confirm);

int il_servo_base__raw_write_u64(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, uint64_t val, int confirm);

int il_servo_base__raw_write_s64(il_servo_t *servo, const il_reg_t *reg,
				 const char *id, int64_t val, int confirm);

int il_servo_base__raw_write_float(il_servo_t *servo, const il_reg_t *reg,
				   const char *id, float val, int confirm);

int il_servo_base__write(il_servo_t *servo, const il_reg_t *reg, const char *id,
			 double val, int confirm);

#endif

