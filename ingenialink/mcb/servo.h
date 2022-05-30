#ifndef MCB_SERVO_H
#define MCB_SERVO_H

#include "../servo.h"

/** PDS default timeout (ms). */
#define PDS_TIMEOUT		1000

/** Flags position offset in statusword. */
#define FLAGS_SW_POS		10

/** Number of retries to reset fault state **/
#define FAULT_RESET_RETRIES		1


/** IngeniaLink servo. */
typedef struct il_mcb_servo {
	/** Servo (parent). */
	il_servo_t servo;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
} il_mcb_servo_t;

/** Obtain MCB servo from parent. */
#define to_mcb_servo(ptr) container_of(ptr, struct il_mcb_servo, servo)

#endif
