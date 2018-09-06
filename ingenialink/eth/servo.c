
#include "servo.h"

#include <string.h>

#include "ingenialink/err.h"
#include "ingenialink/base/servo.h"
#include "ingenialink/registers.h"

/*******************************************************************************
 * Private
 ******************************************************************************/

static int not_supported(void)
{
	ilerr__set("Functionality not supported");

	return IL_ENOTSUP;
}


/*******************************************************************************
 * Public
 ******************************************************************************/

static il_servo_t *il_eth_servo_create(il_net_t *net, uint16_t id,
				       const char *dict)
{
	return NULL;
}


/** ETH servo operations. */
const il_servo_ops_t il_eth_servo_ops = {
    /* public */
	.create = il_eth_servo_create,
};