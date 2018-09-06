
#ifndef ETH_SERVO_H
#define ETH_SERVO_H

#include "../servo.h"

/** PDS default timeout (ms). */
#define PDS_TIMEOUT		1000

/** Flags position offset in statusword. */
#define FLAGS_SW_POS		10


/** IngeniaLink servo. */
typedef struct il_eth_servo {
	/** Servo (parent). */
	il_servo_t servo;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
} il_eth_servo_t;

/** Obtain ETH servo from parent. */
#define to_eth_servo(ptr) container_of(ptr, struct il_eth_servo, servo)

#endif