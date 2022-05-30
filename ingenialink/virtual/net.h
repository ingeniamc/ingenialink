#ifndef VIRTUAL_NET_H_
#define VIRTUAL_NET_H_

#include "../net.h"

/** VIRTUAL network. */
typedef struct il_virtual_net {
	/** Network (parent). */
	il_net_t net;
	/** Reference counter. */
	il_utils_refcnt_t *refcnt;
} il_virtual_net_t;

/** Obtain VIRTUAL Network from parent. */
#define to_virtual_net(ptr) container_of(ptr, struct il_virtual_net, net)

#endif
