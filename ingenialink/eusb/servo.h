#ifndef EUSB_SERVO_H
#define EUSB_SERVO_H

#include "../servo.h"

/** Minimum servo id. */
#define SERVOID_MIN		1

/** Maximum servo id. */
#define SERVOID_MAX		127

/** PDS default timeout (ms). */
#define PDS_TIMEOUT		1000

/** Flags position offset in statusword. */
#define FLAGS_SW_POS		10

/*
 * Constants associated to types of velocity and position feedbacks.
 *
 * References:
 *	http://doc.ingeniamc.com/display/i14402/0x2310+-+Feedbacks
 */

#define DIGITAL_HALLS_CONSTANT	6
#define ANALOG_HALLS_CONSTANT	4096
#define ANALOG_INPUT_CONSTANT	4096
#define SINCOS_CONSTANT		1024
#define PWM_CONSTANT		65535
#define RESOLVER_CONSTANT	65535

/** Relative voltage range. */
#define VOLT_REL_RANGE		32767

/** Radians range. */
#define RAD_RANGE		65535

/** IngeniaLink servo. */
typedef struct il_eusb_servo {
	/** Servo (parent). */
	il_servo_t servo;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
} il_eusb_servo_t;

/** Obtain E-USB servo from parent. */
#define to_eusb_servo(ptr) container_of(ptr, struct il_eusb_servo, servo)

#endif
