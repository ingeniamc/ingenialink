
#ifndef ECAT_SERVO_H
#define ECAT_SERVO_H

#include "../servo.h"

/** PDS default timeout (ms). */
#define PDS_TIMEOUT		1000

/** Flags position offset in statusword. */
#define FLAGS_SW_POS		10

/** Number of retries to reset fault state **/
#define FAULT_RESET_RETRIES		20


/** IngeniaLink servo. */
typedef struct il_ecat_servo {
	/** Servo (parent). */
	il_servo_t servo;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
} il_ecat_servo_t;

/** Obtain ECAT servo from parent. */
#define to_ecat_servo(ptr) container_of(ptr, struct il_ecat_servo, servo)

#endif